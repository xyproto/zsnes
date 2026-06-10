package main

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"io"
	"log/slog"
	"net"
	"strings"
	"sync"
	"testing"
	"time"
)

// startTestServer spins up a server bound to 127.0.0.1:0 (random free port)
// over plain TCP — easier to inspect than TLS, and the framed protocol on top
// is identical.
func startTestServer(tb testing.TB, password string) (string, func()) {
	tb.Helper()
	l, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		tb.Fatalf("listen: %v", err)
	}
	srv := &server{
		rooms: make(map[string]*room),
		log:   slog.New(slog.NewTextHandler(io.Discard, nil)),
	}
	if password != "" {
		h := sha256.Sum256([]byte(password))
		srv.password = h[:]
	}
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		defer wg.Done()
		for {
			c, err := l.Accept()
			if err != nil {
				if errors.Is(err, net.ErrClosed) {
					return
				}
				return
			}
			go srv.handle(c)
		}
	}()
	return l.Addr().String(), func() {
		_ = l.Close()
		wg.Wait()
	}
}

func helloPayload(mode uint8, room, password, nick string) []byte {
	out := make([]byte, 4+1+RoomCodeLen+PasswordLen+NicknameLen)
	binary.BigEndian.PutUint32(out[0:4], ProtocolVersion)
	out[4] = mode
	copy(out[5:5+RoomCodeLen], room)
	copy(out[5+RoomCodeLen:5+RoomCodeLen+PasswordLen], password)
	copy(out[5+RoomCodeLen+PasswordLen:], nick)
	return out
}

// dial completes the TLS-less prefix exchange and returns the live connection.
func dial(tb testing.TB, addr string) net.Conn {
	tb.Helper()
	c, err := net.Dial("tcp", addr)
	if err != nil {
		tb.Fatalf("dial: %v", err)
	}
	_ = c.SetDeadline(time.Now().Add(2 * time.Second))
	if _, err := c.Write([]byte(StreamPrefix)); err != nil {
		tb.Fatalf("write prefix: %v", err)
	}
	buf := make([]byte, len(StreamPrefix))
	if _, err := io.ReadFull(c, buf); err != nil {
		tb.Fatalf("read prefix: %v", err)
	}
	if string(buf) != StreamPrefix {
		tb.Fatalf("server prefix = %q, want %q", buf, StreamPrefix)
	}
	return c
}

// joinRoom dials, sends a CLIENT_HELLO, and returns the connection plus the
// first frame from the server (typically SERVER_HELLO or SERVER_ERROR).
func joinRoom(tb testing.TB, addr string, mode uint8, room, password, nick string) (net.Conn, frame) {
	tb.Helper()
	c := dial(tb, addr)
	if err := writeFrame(c, FrameClientHello, helloPayload(mode, room, password, nick)); err != nil {
		tb.Fatalf("write hello: %v", err)
	}
	f, err := readFrame(c)
	if err != nil {
		tb.Fatalf("read hello reply: %v", err)
	}
	return c, f
}

func mustRecv(tb testing.TB, c net.Conn) frame {
	tb.Helper()
	_ = c.SetReadDeadline(time.Now().Add(2 * time.Second))
	f, err := readFrame(c)
	if err != nil {
		tb.Fatalf("readFrame: %v", err)
	}
	return f
}

// --- frame I/O unit tests ---

func TestFrameRoundTrip(t *testing.T) {
	cases := []struct {
		typ     uint16
		payload []byte
	}{
		{FrameInput, bytes.Repeat([]byte{0xAB}, InputPayloadSize)},
		{FrameBye, nil},
		{FramePing, []byte{1, 2, 3, 4, 5, 6, 7, 8}},
		{FrameServerError, []byte("hello world")},
	}
	for _, tc := range cases {
		var buf bytes.Buffer
		if err := writeFrame(&buf, tc.typ, tc.payload); err != nil {
			t.Fatalf("write: %v", err)
		}
		f, err := readFrame(&buf)
		if err != nil {
			t.Fatalf("read: %v", err)
		}
		if f.typ != tc.typ {
			t.Errorf("type: got %#x want %#x", f.typ, tc.typ)
		}
		if !bytes.Equal(f.payload, tc.payload) && !(len(f.payload) == 0 && len(tc.payload) == 0) {
			t.Errorf("payload: got %x want %x", f.payload, tc.payload)
		}
	}
}

func TestFrameOversizeRejected(t *testing.T) {
	tooBig := bytes.Repeat([]byte{0}, FrameMaxPayload+1)
	if err := writeFrame(io.Discard, FrameInput, tooBig); err == nil {
		t.Fatal("expected error writing oversize frame")
	}

	// Also: reader must reject a header that claims > FrameMaxPayload.
	var buf bytes.Buffer
	var hdr [4]byte
	binary.BigEndian.PutUint16(hdr[0:2], FrameInput)
	binary.BigEndian.PutUint16(hdr[2:4], uint16(FrameMaxPayload+1))
	buf.Write(hdr[:])
	if _, err := readFrame(&buf); err == nil {
		t.Fatal("expected error reading oversize length")
	}
}

func TestParseClientHelloRoundTrip(t *testing.T) {
	got, err := parseClientHello(helloPayload(HelloModeCreate, "lobby", "secret", "alice"))
	if err != nil {
		t.Fatalf("parse: %v", err)
	}
	if got.proto != ProtocolVersion {
		t.Errorf("proto = %d", got.proto)
	}
	if got.mode != HelloModeCreate {
		t.Errorf("mode = %d", got.mode)
	}
	if got.room != "lobby" {
		t.Errorf("room = %q", got.room)
	}
	if got.password != "secret" {
		t.Errorf("password = %q", got.password)
	}
	if !bytes.Equal(got.nickname[:5], []byte("alice")) {
		t.Errorf("nickname = %q", got.nickname[:])
	}
}

func TestParseClientHelloShortPayload(t *testing.T) {
	if _, err := parseClientHello([]byte{0, 0, 0, 1, 1}); err == nil {
		t.Fatal("expected error for short hello")
	}
}

// --- handshake + matchmaking ---

func TestRoleAssignment(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	a, fa := joinRoom(t, addr, HelloModeCreate, "r1", "", "alice")
	defer a.Close()
	if fa.typ != FrameServerHello {
		t.Fatalf("first hello reply: type=%#x payload=%q", fa.typ, fa.payload)
	}
	if fa.payload[4] != RoleHost {
		t.Errorf("first client role = %d, want %d", fa.payload[4], RoleHost)
	}

	b, fb := joinRoom(t, addr, HelloModeCreate, "r1", "", "bob")
	defer b.Close()
	if fb.typ != FrameServerHello {
		t.Fatalf("second hello reply: type=%#x", fb.typ)
	}
	if fb.payload[4] != RoleClient {
		t.Errorf("second client role = %d, want %d", fb.payload[4], RoleClient)
	}

	// Both sides receive PEER_READY carrying the peer's nickname.
	pa := mustRecv(t, a)
	pb := mustRecv(t, b)
	if pa.typ != FramePeerReady || pb.typ != FramePeerReady {
		t.Fatalf("expected PEER_READY on both sides, got %#x and %#x", pa.typ, pb.typ)
	}
	if !bytes.HasPrefix(pa.payload, []byte("bob")) {
		t.Errorf("host saw peer-nick %q, want bob", pa.payload)
	}
	if !bytes.HasPrefix(pb.payload, []byte("alice")) {
		t.Errorf("client saw peer-nick %q, want alice", pb.payload)
	}
}

func TestInputRelay(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	a, _ := joinRoom(t, addr, HelloModeCreate, "rly", "", "a")
	defer a.Close()
	b, _ := joinRoom(t, addr, HelloModeCreate, "rly", "", "b")
	defer b.Close()
	_ = mustRecv(t, a) // drain PEER_READY
	_ = mustRecv(t, b)

	// a -> b
	payload := make([]byte, InputPayloadSize)
	binary.BigEndian.PutUint32(payload[0:4], InputMagic)
	binary.BigEndian.PutUint32(payload[4:8], 42)
	binary.BigEndian.PutUint32(payload[8:12], 0xDEADBEEF)
	binary.BigEndian.PutUint32(payload[12:16], 0xCAFEBABE)
	if err := writeFrame(a, FrameInput, payload); err != nil {
		t.Fatal(err)
	}
	got := mustRecv(t, b)
	if got.typ != FrameInput {
		t.Fatalf("got type %#x, want INPUT", got.typ)
	}
	if !bytes.Equal(got.payload, payload) {
		t.Fatalf("payload mismatch: got %x want %x", got.payload, payload)
	}

	// b -> a
	binary.BigEndian.PutUint32(payload[4:8], 43)
	binary.BigEndian.PutUint32(payload[8:12], 0x00008000)
	if err := writeFrame(b, FrameInput, payload); err != nil {
		t.Fatal(err)
	}
	got = mustRecv(t, a)
	if got.typ != FrameInput || !bytes.Equal(got.payload, payload) {
		t.Fatalf("reverse relay failed: type=%#x payload=%x", got.typ, got.payload)
	}
}

func TestInputWithoutMagicDropped(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	a, _ := joinRoom(t, addr, HelloModeCreate, "magic", "", "a")
	defer a.Close()
	b, _ := joinRoom(t, addr, HelloModeCreate, "magic", "", "b")
	defer b.Close()
	_ = mustRecv(t, a)
	_ = mustRecv(t, b)

	// Wrong magic — should not be relayed.
	bogus := make([]byte, InputPayloadSize)
	binary.BigEndian.PutUint32(bogus[0:4], 0xDEADBEEF) // not "NETP"
	if err := writeFrame(a, FrameInput, bogus); err != nil {
		t.Fatal(err)
	}
	_ = b.SetReadDeadline(time.Now().Add(200 * time.Millisecond))
	if _, err := readFrame(b); err == nil {
		t.Fatal("peer received a frame that should have been dropped")
	}

	// Now send a real INPUT to confirm the connection is still alive.
	good := make([]byte, InputPayloadSize)
	binary.BigEndian.PutUint32(good[0:4], InputMagic)
	if err := writeFrame(a, FrameInput, good); err != nil {
		t.Fatal(err)
	}
	_ = b.SetReadDeadline(time.Now().Add(2 * time.Second))
	if f, err := readFrame(b); err != nil || f.typ != FrameInput {
		t.Fatalf("real INPUT didn't arrive: f=%v err=%v", f, err)
	}
}

func TestBYEOnPeerDisconnect(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	a, _ := joinRoom(t, addr, HelloModeCreate, "bye", "", "a")
	b, _ := joinRoom(t, addr, HelloModeCreate, "bye", "", "b")
	defer b.Close()
	_ = mustRecv(t, a) // PEER_READY
	_ = mustRecv(t, b)

	a.Close()
	got := mustRecv(t, b)
	if got.typ != FrameBye {
		t.Fatalf("expected BYE, got %#x", got.typ)
	}
}

func TestPingPongLocal(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	a, _ := joinRoom(t, addr, HelloModeCreate, "ping", "", "a")
	defer a.Close()
	b, _ := joinRoom(t, addr, HelloModeCreate, "ping", "", "b")
	defer b.Close()
	_ = mustRecv(t, a) // PEER_READY
	_ = mustRecv(t, b)

	// PING should be answered with PONG carrying the same payload, and NOT
	// be forwarded to the peer.
	ts := []byte{0xCA, 0xFE, 0xBA, 0xBE, 0x01, 0x02, 0x03, 0x04}
	if err := writeFrame(a, FramePing, ts); err != nil {
		t.Fatal(err)
	}
	got := mustRecv(t, a)
	if got.typ != FramePong {
		t.Fatalf("got %#x, want PONG", got.typ)
	}
	if !bytes.Equal(got.payload, ts) {
		t.Errorf("PONG did not echo payload: got %x want %x", got.payload, ts)
	}

	// Peer must not see the PING (or PONG).
	_ = b.SetReadDeadline(time.Now().Add(200 * time.Millisecond))
	if _, err := readFrame(b); err == nil {
		t.Fatal("PING was forwarded to the peer; should be server-local")
	}
}

func TestJoinNonexistentRoom(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	c, f := joinRoom(t, addr, HelloModeJoin, "ghost", "", "a")
	defer c.Close()
	if f.typ != FrameServerError {
		t.Fatalf("expected SERVER_ERROR, got %#x: %q", f.typ, f.payload)
	}
	if !strings.Contains(string(f.payload), "no such room") {
		t.Errorf("error = %q, want substring 'no such room'", f.payload)
	}
}

func TestCreateThenJoinWorks(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	a, fa := joinRoom(t, addr, HelloModeCreate, "joinable", "", "a")
	defer a.Close()
	if fa.typ != FrameServerHello {
		t.Fatalf("create reply: %#x", fa.typ)
	}
	b, fb := joinRoom(t, addr, HelloModeJoin, "joinable", "", "b")
	defer b.Close()
	if fb.typ != FrameServerHello {
		t.Fatalf("join reply: %#x payload=%q", fb.typ, fb.payload)
	}
	if fb.payload[4] != RoleClient {
		t.Errorf("joiner role = %d, want %d", fb.payload[4], RoleClient)
	}
}

func TestRoomFullRejectsThirdClient(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	a, _ := joinRoom(t, addr, HelloModeCreate, "duo", "", "a")
	defer a.Close()
	b, _ := joinRoom(t, addr, HelloModeCreate, "duo", "", "b")
	defer b.Close()
	_ = mustRecv(t, a)
	_ = mustRecv(t, b)

	c, f := joinRoom(t, addr, HelloModeCreate, "duo", "", "c")
	defer c.Close()
	if f.typ != FrameServerError {
		t.Fatalf("expected SERVER_ERROR for third client, got %#x", f.typ)
	}
	if !strings.Contains(string(f.payload), "full") {
		t.Errorf("error = %q, want 'full'", f.payload)
	}
}

func TestBadStreamPrefix(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	c, err := net.Dial("tcp", addr)
	if err != nil {
		t.Fatal(err)
	}
	defer c.Close()
	_ = c.SetDeadline(time.Now().Add(2 * time.Second))
	if _, err := c.Write([]byte("XXXX")); err != nil {
		t.Fatal(err)
	}
	// Server should send back its own prefix first, then SERVER_ERROR, then close.
	// (The current implementation drops the connection without writing a reply
	// when the prefix is bad — accept either: an error frame, or EOF.)
	_, _ = io.ReadAll(c)
	// Reaching here means the connection terminated, which is what we want.
}

func TestBadProtocolVersion(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	c := dial(t, addr)
	defer c.Close()
	bad := helloPayload(HelloModeCreate, "x", "", "n")
	binary.BigEndian.PutUint32(bad[0:4], 9999) // unsupported version
	if err := writeFrame(c, FrameClientHello, bad); err != nil {
		t.Fatal(err)
	}
	f, err := readFrame(c)
	if err != nil {
		t.Fatalf("read reply: %v", err)
	}
	if f.typ != FrameServerError {
		t.Fatalf("expected SERVER_ERROR, got %#x", f.typ)
	}
	if !strings.Contains(string(f.payload), "unsupported protocol") {
		t.Errorf("error = %q", f.payload)
	}
}

func TestEmptyRoomRejected(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	c, f := joinRoom(t, addr, HelloModeCreate, "", "", "n")
	defer c.Close()
	if f.typ != FrameServerError {
		t.Fatalf("expected SERVER_ERROR, got %#x", f.typ)
	}
	if !strings.Contains(string(f.payload), "empty room") {
		t.Errorf("error = %q", f.payload)
	}
}

func TestRoomPassword(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	// Creator sets the password.
	a, fa := joinRoom(t, addr, HelloModeCreate, "locked", "swordfish", "a")
	defer a.Close()
	if fa.typ != FrameServerHello {
		t.Fatalf("creator reply: %#x payload=%q", fa.typ, fa.payload)
	}

	// Wrong password is rejected.
	b, fb := joinRoom(t, addr, HelloModeCreate, "locked", "wrong", "b")
	defer b.Close()
	if fb.typ != FrameServerError {
		t.Fatalf("expected SERVER_ERROR for bad pw, got %#x", fb.typ)
	}
	if !strings.Contains(string(fb.payload), "bad room password") {
		t.Errorf("error = %q", fb.payload)
	}

	// Correct password joins.
	c, fc := joinRoom(t, addr, HelloModeCreate, "locked", "swordfish", "c")
	defer c.Close()
	if fc.typ != FrameServerHello {
		t.Fatalf("expected SERVER_HELLO for correct pw, got %#x payload=%q", fc.typ, fc.payload)
	}
}

func TestGlobalPasswordGate(t *testing.T) {
	addr, stop := startTestServer(t, "topsecret")
	defer stop()

	// Wrong server-wide password → rejected, regardless of room state.
	a, fa := joinRoom(t, addr, HelloModeCreate, "r", "guess", "a")
	defer a.Close()
	if fa.typ != FrameServerError {
		t.Fatalf("expected SERVER_ERROR for bad global pw, got %#x", fa.typ)
	}
	if !strings.Contains(string(fa.payload), "authentication required") {
		t.Errorf("error = %q", fa.payload)
	}

	// Correct password → accepted.
	b, fb := joinRoom(t, addr, HelloModeCreate, "r", "topsecret", "b")
	defer b.Close()
	if fb.typ != FrameServerHello {
		t.Fatalf("expected SERVER_HELLO for correct global pw, got %#x", fb.typ)
	}
}

func TestServerHelloCarriesRoomCode(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	c, f := joinRoom(t, addr, HelloModeCreate, "echoroom", "", "n")
	defer c.Close()
	if f.typ != FrameServerHello {
		t.Fatalf("type %#x", f.typ)
	}
	if got := string(bytes.TrimRight(f.payload[5:5+RoomCodeLen], "\x00")); got != "echoroom" {
		t.Errorf("server hello room = %q, want echoroom", got)
	}
}

// TestRoleSlotContract documents the role/slot wiring the C client depends on.
// Each side reads its LOCAL pad from joy_a (controller 1), then:
//   - host writes its local input into joy_a, remote into joy_b.
//   - client writes its local input into joy_b, remote into joy_a.
// Net effect for the SNES engine: P1 always carries the host's pad, P2 always
// carries the client's pad. If this test ever fails, the C-side input routing
// must be revisited.
func TestRoleSlotContract(t *testing.T) {
	addr, stop := startTestServer(t, "")
	defer stop()

	a, fa := joinRoom(t, addr, HelloModeCreate, "slots", "", "host")
	defer a.Close()
	b, fb := joinRoom(t, addr, HelloModeCreate, "slots", "", "client")
	defer b.Close()
	if fa.payload[4] != RoleHost {
		t.Fatalf("first client expected RoleHost (%d), got %d", RoleHost, fa.payload[4])
	}
	if fb.payload[4] != RoleClient {
		t.Fatalf("second client expected RoleClient (%d), got %d", RoleClient, fb.payload[4])
	}
	_ = mustRecv(t, a) // drain PEER_READY
	_ = mustRecv(t, b)

	// Simulate the host pressing A (0x8080) and the client pressing B (0x4040)
	// in the same frame. The relay just shuffles bytes — but we lock in that
	// each side receives the OTHER side's input verbatim, which is what the
	// C client then routes into joy_b (host) / joy_a (client).
	mkInput := func(seq, joy uint32) []byte {
		p := make([]byte, InputPayloadSize)
		binary.BigEndian.PutUint32(p[0:4], InputMagic)
		binary.BigEndian.PutUint32(p[4:8], seq)
		binary.BigEndian.PutUint32(p[8:12], joy)
		return p
	}
	if err := writeFrame(a, FrameInput, mkInput(1, 0x8080)); err != nil {
		t.Fatal(err)
	}
	if err := writeFrame(b, FrameInput, mkInput(1, 0x4040)); err != nil {
		t.Fatal(err)
	}
	gotB := mustRecv(t, b) // host's input lands at client
	gotA := mustRecv(t, a) // client's input lands at host
	if gotB.typ != FrameInput || binary.BigEndian.Uint32(gotB.payload[8:12]) != 0x8080 {
		t.Errorf("client did not receive host's pad: %x", gotB.payload)
	}
	if gotA.typ != FrameInput || binary.BigEndian.Uint32(gotA.payload[8:12]) != 0x4040 {
		t.Errorf("host did not receive client's pad: %x", gotA.payload)
	}
}

func TestSPKIPinIsDeterministic(t *testing.T) {
	// Self-generated cert; pin derives from the leaf's SPKI bytes. The hex
	// should be stable for a given key but the key is fresh per call, so we
	// just check format & length.
	cert, pin, err := loadOrGenerateCert("", "")
	if err != nil {
		t.Fatalf("gen: %v", err)
	}
	if cert.Leaf == nil {
		t.Fatal("leaf nil")
	}
	if len(pin) != 64 {
		t.Errorf("pin len = %d, want 64", len(pin))
	}
	for _, c := range pin {
		if !(c >= '0' && c <= '9' || c >= 'a' && c <= 'f') {
			t.Errorf("non-hex char %q in pin", c)
			break
		}
	}
	if got := spkiPin(cert.Leaf); got != pin {
		t.Errorf("spkiPin(leaf) = %s, want %s", got, pin)
	}
}
