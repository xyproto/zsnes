// ZSNES netplay relay server.
//
// Two clients connect over TLS, present the same room code + password, and
// the server pairs them and forwards INPUT frames between them. The first
// client into a room is assigned the host role (player 1); the second is
// the client (player 2). When either side disconnects, the room is torn
// down and the surviving peer is informed.
//
// Wire protocol (all integers big-endian, fixed-size payloads NUL-padded):
//
//	Stream prefix (sent once per direction immediately after TLS handshake):
//	  bytes  "ZNP1"   protocol identifier
//
//	Frame:
//	  u16   type
//	  u16   length        (bytes of payload that follow; max FrameMaxPayload)
//	  u8[]  payload
//
//	Types:
//	  0x0001  CLIENT_HELLO   u32 proto, u8 mode, char room[16], char pass[32], char nick[16]
//	  0x0002  SERVER_HELLO   u32 proto, u8 role (1=host/P1, 2=client/P2), char room[16]
//	  0x0003  PEER_READY     char peer_nick[16]
//	  0x0004  SERVER_ERROR   utf-8 message
//	  0x0010  INPUT          u32 magic ("NETP"), u32 seq, u32 joy, u32 crc
//	  0x0020  PING           u64 ts
//	  0x0021  PONG           u64 ts_echo
//	  0x00FF  BYE            (empty)
package main

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/sha256"
	"crypto/subtle"
	"crypto/tls"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/binary"
	"encoding/hex"
	"encoding/pem"
	"errors"
	"flag"
	"fmt"
	"io"
	"log/slog"
	"math/big"
	"net"
	"os"
	"sync"
	"time"
)

// Protocol constants. Keep in sync with linux/netplay.h.
const (
	StreamPrefix     = "ZNP1"
	ProtocolVersion  = uint32(1)
	FrameMaxPayload  = 4096
	RoomCodeLen      = 16
	PasswordLen      = 32
	NicknameLen      = 16
	InputPayloadSize = 16
	InputMagic       = uint32(0x4E455450) // "NETP"

	FrameClientHello = uint16(0x0001)
	FrameServerHello = uint16(0x0002)
	FramePeerReady   = uint16(0x0003)
	FrameServerError = uint16(0x0004)
	FrameInput       = uint16(0x0010)
	FramePing        = uint16(0x0020)
	FramePong        = uint16(0x0021)
	FrameBye         = uint16(0x00FF)

	HelloModeCreate = uint8(1)
	HelloModeJoin   = uint8(2)

	RoleHost   = uint8(1)
	RoleClient = uint8(2)

	HandshakeTimeout = 10 * time.Second
	IdleTimeout      = 60 * time.Second
	WriteTimeout     = 5 * time.Second
)

type frame struct {
	typ     uint16
	payload []byte
}

type peer struct {
	conn net.Conn
	nick [NicknameLen]byte
	role uint8
	out  chan frame // frames the relay writes to this peer
	done chan struct{}
}

type room struct {
	mu     sync.Mutex
	code   string
	auth   []byte // sha256(password); empty means open
	host   *peer
	client *peer
}

type server struct {
	mu       sync.Mutex
	rooms    map[string]*room
	log      *slog.Logger
	password []byte // optional global password (sha256). zero-length if disabled.
}

func main() {
	addr := flag.String("addr", ":7845", "listen address")
	certFile := flag.String("cert", "", "TLS certificate (PEM). Auto-generated self-signed if empty.")
	keyFile := flag.String("key", "", "TLS private key (PEM). Auto-generated if cert is empty.")
	insecure := flag.Bool("insecure", false, "disable TLS (plain TCP — testing only)")
	password := flag.String("password", "", "shared password required from all clients (optional)")
	logLevelArg := flag.String("log-level", "info", "log level: debug|info|warn|error")
	flag.Parse()

	var lvl slog.Level
	if err := lvl.UnmarshalText([]byte(*logLevelArg)); err != nil {
		fmt.Fprintf(os.Stderr, "invalid -log-level %q: %v\n", *logLevelArg, err)
		os.Exit(2)
	}
	log := slog.New(slog.NewTextHandler(os.Stderr, &slog.HandlerOptions{Level: lvl}))

	srv := &server{
		rooms: make(map[string]*room),
		log:   log,
	}
	if *password != "" {
		h := sha256.Sum256([]byte(*password))
		srv.password = h[:]
	}

	var listener net.Listener
	if *insecure {
		log.Warn("starting in INSECURE mode — connections are unencrypted")
		l, err := net.Listen("tcp", *addr)
		if err != nil {
			log.Error("listen failed", "err", err)
			os.Exit(1)
		}
		listener = l
	} else {
		cert, pin, err := loadOrGenerateCert(*certFile, *keyFile)
		if err != nil {
			log.Error("certificate load failed", "err", err)
			os.Exit(1)
		}
		tlsCfg := &tls.Config{
			Certificates: []tls.Certificate{cert},
			MinVersion:   tls.VersionTLS12,
			CipherSuites: []uint16{
				tls.TLS_AES_128_GCM_SHA256,
				tls.TLS_AES_256_GCM_SHA384,
				tls.TLS_CHACHA20_POLY1305_SHA256,
				tls.TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
				tls.TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
				tls.TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305,
				tls.TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305,
			},
		}
		l, err := tls.Listen("tcp", *addr, tlsCfg)
		if err != nil {
			log.Error("tls listen failed", "err", err)
			os.Exit(1)
		}
		listener = l
		fmt.Fprintf(os.Stderr, "ZSNES netplay server listening on %s (TLS)\n", *addr)
		fmt.Fprintf(os.Stderr, "SPKI SHA-256 pin: %s\n", pin)
		fmt.Fprintf(os.Stderr, "Clients should set ZSNES_NETPLAY_PIN to this value to verify the server.\n")
	}
	defer listener.Close()
	log.Info("listening", "addr", *addr, "tls", !*insecure)

	for {
		conn, err := listener.Accept()
		if err != nil {
			if errors.Is(err, net.ErrClosed) {
				return
			}
			log.Warn("accept failed", "err", err)
			continue
		}
		go srv.handle(conn)
	}
}

// loadOrGenerateCert returns the server certificate plus the hex SHA-256 of
// the certificate's SubjectPublicKeyInfo (used for client-side pinning).
func loadOrGenerateCert(certPath, keyPath string) (tls.Certificate, string, error) {
	if certPath != "" && keyPath != "" {
		c, err := tls.LoadX509KeyPair(certPath, keyPath)
		if err != nil {
			return tls.Certificate{}, "", err
		}
		leaf, err := x509.ParseCertificate(c.Certificate[0])
		if err != nil {
			return tls.Certificate{}, "", err
		}
		c.Leaf = leaf
		return c, spkiPin(leaf), nil
	}

	priv, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		return tls.Certificate{}, "", err
	}
	serial, err := rand.Int(rand.Reader, new(big.Int).Lsh(big.NewInt(1), 128))
	if err != nil {
		return tls.Certificate{}, "", err
	}
	now := time.Now()
	tmpl := x509.Certificate{
		SerialNumber:          serial,
		Subject:               pkix.Name{CommonName: "zsnes-netplay-relay"},
		NotBefore:             now.Add(-time.Hour),
		NotAfter:              now.AddDate(5, 0, 0),
		KeyUsage:              x509.KeyUsageDigitalSignature | x509.KeyUsageKeyEncipherment,
		ExtKeyUsage:           []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth},
		BasicConstraintsValid: true,
		IsCA:                  false,
		DNSNames:              []string{"localhost"},
		IPAddresses:           []net.IP{net.IPv4(127, 0, 0, 1), net.IPv6loopback},
	}
	der, err := x509.CreateCertificate(rand.Reader, &tmpl, &tmpl, &priv.PublicKey, priv)
	if err != nil {
		return tls.Certificate{}, "", err
	}
	leaf, err := x509.ParseCertificate(der)
	if err != nil {
		return tls.Certificate{}, "", err
	}
	cert := tls.Certificate{
		Certificate: [][]byte{der},
		PrivateKey:  priv,
		Leaf:        leaf,
	}
	_ = pem.Encode // keep import live in case the user wants to dump it later
	return cert, spkiPin(leaf), nil
}

func spkiPin(cert *x509.Certificate) string {
	sum := sha256.Sum256(cert.RawSubjectPublicKeyInfo)
	return hex.EncodeToString(sum[:])
}

func (s *server) handle(raw net.Conn) {
	remote := raw.RemoteAddr().String()
	logger := s.log.With("remote", remote)
	defer raw.Close()

	if tc, ok := raw.(*tls.Conn); ok {
		_ = tc.SetDeadline(time.Now().Add(HandshakeTimeout))
		if err := tc.Handshake(); err != nil {
			logger.Debug("tls handshake failed", "err", err)
			return
		}
		_ = tc.SetDeadline(time.Time{})
	}

	if err := raw.SetReadDeadline(time.Now().Add(HandshakeTimeout)); err != nil {
		logger.Debug("set deadline failed", "err", err)
		return
	}

	// Stream prefix.
	prefix := make([]byte, len(StreamPrefix))
	if _, err := io.ReadFull(raw, prefix); err != nil {
		logger.Debug("prefix read failed", "err", err)
		return
	}
	if string(prefix) != StreamPrefix {
		logger.Debug("bad stream prefix", "got", fmt.Sprintf("%q", prefix))
		_ = writeFrame(raw, FrameServerError, []byte("bad stream prefix"))
		return
	}
	if _, err := raw.Write([]byte(StreamPrefix)); err != nil {
		logger.Debug("prefix write failed", "err", err)
		return
	}

	// CLIENT_HELLO.
	f, err := readFrame(raw)
	if err != nil {
		logger.Debug("hello read failed", "err", err)
		return
	}
	if f.typ != FrameClientHello {
		logger.Debug("expected CLIENT_HELLO", "got", f.typ)
		_ = writeFrame(raw, FrameServerError, []byte("expected CLIENT_HELLO"))
		return
	}
	hello, err := parseClientHello(f.payload)
	if err != nil {
		logger.Debug("hello parse failed", "err", err)
		_ = writeFrame(raw, FrameServerError, []byte("malformed hello"))
		return
	}
	if hello.proto != ProtocolVersion {
		_ = writeFrame(raw, FrameServerError, fmt.Appendf(nil, "unsupported protocol %d", hello.proto))
		return
	}
	if hello.mode != HelloModeCreate && hello.mode != HelloModeJoin {
		_ = writeFrame(raw, FrameServerError, []byte("invalid mode"))
		return
	}
	if hello.room == "" {
		_ = writeFrame(raw, FrameServerError, []byte("empty room code"))
		return
	}

	// Server-wide password gate.
	if len(s.password) != 0 {
		ph := sha256.Sum256([]byte(hello.password))
		if subtle.ConstantTimeCompare(ph[:], s.password) != 1 {
			_ = writeFrame(raw, FrameServerError, []byte("authentication required"))
			return
		}
	}

	_ = raw.SetDeadline(time.Time{})

	p, peerForReady, err := s.joinRoom(hello, raw, logger)
	if err != nil {
		_ = writeFrame(raw, FrameServerError, []byte(err.Error()))
		return
	}
	defer s.leaveRoom(hello.room, p)

	// Inform this client of its assigned role and room.
	if err := writeFrame(raw, FrameServerHello, encodeServerHello(p.role, hello.room)); err != nil {
		logger.Debug("server hello write failed", "err", err)
		return
	}
	if peerForReady != nil {
		// Both sides are now present — exchange peer-ready (nicknames).
		_ = writeFrame(p.conn, FramePeerReady, peerForReady.nick[:])
		select {
		case peerForReady.out <- frame{typ: FramePeerReady, payload: append([]byte{}, p.nick[:]...)}:
		case <-peerForReady.done:
		}
	}

	logger.Info("client paired", "room", hello.room, "role", p.role)
	s.runRelay(p, hello.room, logger)
}

type clientHello struct {
	proto    uint32
	mode     uint8
	room     string
	password string
	nickname [NicknameLen]byte
}

func parseClientHello(b []byte) (clientHello, error) {
	const want = 4 + 1 + RoomCodeLen + PasswordLen + NicknameLen
	if len(b) != want {
		return clientHello{}, fmt.Errorf("hello: have %d bytes, want %d", len(b), want)
	}
	h := clientHello{}
	h.proto = binary.BigEndian.Uint32(b[0:4])
	h.mode = b[4]
	h.room = trimNul(b[5 : 5+RoomCodeLen])
	h.password = trimNul(b[5+RoomCodeLen : 5+RoomCodeLen+PasswordLen])
	copy(h.nickname[:], b[5+RoomCodeLen+PasswordLen:])
	return h, nil
}

func encodeServerHello(role uint8, roomCode string) []byte {
	out := make([]byte, 4+1+RoomCodeLen)
	binary.BigEndian.PutUint32(out[0:4], ProtocolVersion)
	out[4] = role
	copy(out[5:], []byte(roomCode))
	return out
}

func trimNul(b []byte) string {
	for i, c := range b {
		if c == 0 {
			return string(b[:i])
		}
	}
	return string(b)
}

func (s *server) joinRoom(h clientHello, conn net.Conn, _ *slog.Logger) (*peer, *peer, error) {
	s.mu.Lock()
	r, exists := s.rooms[h.room]
	switch {
	case !exists && h.mode == HelloModeCreate:
		r = &room{code: h.room}
		if h.password != "" {
			ph := sha256.Sum256([]byte(h.password))
			r.auth = ph[:]
		}
		s.rooms[h.room] = r
	case !exists && h.mode == HelloModeJoin:
		s.mu.Unlock()
		return nil, nil, errors.New("no such room")
	}
	s.mu.Unlock()

	r.mu.Lock()
	defer r.mu.Unlock()

	// Verify per-room password.
	if len(r.auth) != 0 {
		ph := sha256.Sum256([]byte(h.password))
		if subtle.ConstantTimeCompare(ph[:], r.auth) != 1 {
			return nil, nil, errors.New("bad room password")
		}
	} else if h.password != "" {
		return nil, nil, errors.New("room is open, password not accepted")
	}

	p := &peer{
		conn: conn,
		out:  make(chan frame, 64),
		done: make(chan struct{}),
	}
	copy(p.nick[:], h.nickname[:])

	switch {
	case r.host == nil:
		p.role = RoleHost
		r.host = p
		return p, nil, nil
	case r.client == nil:
		p.role = RoleClient
		r.client = p
		return p, r.host, nil
	default:
		return nil, nil, errors.New("room is full")
	}
}

func (s *server) leaveRoom(code string, p *peer) {
	s.mu.Lock()
	r, ok := s.rooms[code]
	if !ok {
		s.mu.Unlock()
		return
	}
	s.mu.Unlock()

	r.mu.Lock()
	var peerOther *peer
	switch p {
	case r.host:
		r.host = nil
		peerOther = r.client
	case r.client:
		r.client = nil
		peerOther = r.host
	}
	empty := r.host == nil && r.client == nil
	r.mu.Unlock()

	if peerOther != nil {
		select {
		case peerOther.out <- frame{typ: FrameBye}:
		case <-peerOther.done:
		}
	}

	if empty {
		s.mu.Lock()
		delete(s.rooms, code)
		s.mu.Unlock()
	}
}

func (s *server) peerOf(code string, p *peer) *peer {
	s.mu.Lock()
	r, ok := s.rooms[code]
	s.mu.Unlock()
	if !ok {
		return nil
	}
	r.mu.Lock()
	defer r.mu.Unlock()
	if p == r.host {
		return r.client
	}
	return r.host
}

func (s *server) runRelay(self *peer, roomCode string, log *slog.Logger) {
	writerDone := make(chan struct{})
	go func() {
		defer close(writerDone)
		for {
			select {
			case f, ok := <-self.out:
				if !ok {
					return
				}
				if err := writeFrameTimeout(self.conn, f.typ, f.payload, WriteTimeout); err != nil {
					log.Debug("write failed", "err", err)
					return
				}
				if f.typ == FrameBye {
					return
				}
			case <-self.done:
				return
			}
		}
	}()

	// Main loop reads frames from self and forwards them to peer.
	for {
		_ = self.conn.SetReadDeadline(time.Now().Add(IdleTimeout))
		f, err := readFrame(self.conn)
		if err != nil {
			log.Debug("read closed", "err", err)
			break
		}
		switch f.typ {
		case FrameInput:
			if len(f.payload) != InputPayloadSize {
				continue
			}
			// Cheap header check — must carry the NETP magic, which keeps
			// random TCP probes from being relayed.
			if binary.BigEndian.Uint32(f.payload[0:4]) != InputMagic {
				continue
			}
			s.forward(roomCode, self, f)
		case FramePing:
			// Reply locally with PONG, do not forward.
			select {
			case self.out <- frame{typ: FramePong, payload: append([]byte{}, f.payload...)}:
			case <-self.done:
			}
		case FrameBye:
			goto done
		default:
			// Forward everything else opaquely so future versions stay extensible,
			// after a size sanity check.
			if len(f.payload) <= FrameMaxPayload {
				s.forward(roomCode, self, f)
			}
		}
	}
done:
	// Signal the writer to drain pending out-frames then exit. close(self.done)
	// also tells forward()/leaveRoom that further sends to self.out are futile.
	close(self.done)
	<-writerDone
}

func (s *server) forward(roomCode string, from *peer, f frame) {
	other := s.peerOf(roomCode, from)
	if other == nil {
		return
	}
	select {
	case other.out <- frame{typ: f.typ, payload: append([]byte(nil), f.payload...)}:
	case <-other.done:
	default:
		// Buffer is full — drop. Input is sent every ~17ms; the next one will
		// arrive shortly and lockstep recovery is the client's job.
	}
}

// Frame I/O.

func readFrame(r io.Reader) (frame, error) {
	var hdr [4]byte
	if _, err := io.ReadFull(r, hdr[:]); err != nil {
		return frame{}, err
	}
	typ := binary.BigEndian.Uint16(hdr[0:2])
	length := binary.BigEndian.Uint16(hdr[2:4])
	if int(length) > FrameMaxPayload {
		return frame{}, fmt.Errorf("frame too large: %d", length)
	}
	payload := make([]byte, length)
	if length > 0 {
		if _, err := io.ReadFull(r, payload); err != nil {
			return frame{}, err
		}
	}
	return frame{typ: typ, payload: payload}, nil
}

func writeFrame(w io.Writer, typ uint16, payload []byte) error {
	if len(payload) > FrameMaxPayload {
		return fmt.Errorf("frame too large: %d", len(payload))
	}
	var hdr [4]byte
	binary.BigEndian.PutUint16(hdr[0:2], typ)
	binary.BigEndian.PutUint16(hdr[2:4], uint16(len(payload)))
	if _, err := w.Write(hdr[:]); err != nil {
		return err
	}
	if len(payload) > 0 {
		if _, err := w.Write(payload); err != nil {
			return err
		}
	}
	return nil
}

func writeFrameTimeout(c net.Conn, typ uint16, payload []byte, d time.Duration) error {
	_ = c.SetWriteDeadline(time.Now().Add(d))
	defer c.SetWriteDeadline(time.Time{})
	return writeFrame(c, typ, payload)
}

// pem is referenced to keep the import live for future cert-dump tooling.
var _ = pem.Encode
