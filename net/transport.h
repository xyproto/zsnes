/*
Copyright (C) 1997-2008 ZSNES Team ( zsKnight, _Demo_, pagefault, Nach )

http://www.zsnes.com
http://sourceforge.net/projects/zsnes
https://zsnes.bountysource.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* Everything netplay needs from the machine it is running on.
 *
 * The session state machine talks to this and nothing else, so porting
 * netplay to another platform is a matter of writing one of these rather than
 * finding the socket calls scattered through the protocol. unix/net_transport.c
 * is the BSD sockets implementation. */

#ifndef ZSNES_NET_TRANSPORT_H
#define ZSNES_NET_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

/* A socket handle. Unix makes this a file descriptor; another platform is
   free to make it an index into whatever it keeps. */
typedef int NetSocket;

enum { NET_SOCKET_NONE = -1 };

/* Somewhere to remember a peer that sent us a datagram, without the session
   machine needing to know what an address looks like. */
typedef struct {
    unsigned char opaque[32];
    unsigned len;
} NetAddr;

/* Open a non-blocking socket with the options netplay wants already set.
   Returns NET_SOCKET_NONE on failure. */
NetSocket net_open(int udp);
void net_close(NetSocket s);

/* Apply netplay's usual options to a socket that arrived from elsewhere -
   one an asynchronous connect produced, say. */
void net_adopt(NetSocket s, int udp);

int net_bind_any(NetSocket s, uint16_t port);
int net_listen(NetSocket s, int backlog);
NetSocket net_accept(NetSocket s);

/* Wait for readiness: >0 ready, 0 timed out, <0 failed. */
int net_wait(NetSocket s, int want_write, int timeout_ms);
int net_wait_until(NetSocket s, int want_write, uint64_t deadline_ms);

/* Whether a connect that was still in progress has since succeeded. */
int net_connect_pending_ok(NetSocket s);

/* Single datagram or stream chunk; negative on failure. */
long net_send(NetSocket s, void const* buf, size_t len);
long net_recv(NetSocket s, void* buf, size_t len);
long net_recv_from(NetSocket s, void* buf, size_t len, NetAddr* from);
long net_send_to(NetSocket s, void const* buf, size_t len, NetAddr const* to);

/* Point a datagram socket at one peer, so plain send and recv work. */
int net_connect_addr(NetSocket s, NetAddr const* a);

/* Whole buffer, within a deadline. Non-zero on success. */
int net_send_all(NetSocket s, void const* buf, size_t len, int timeout_ms);
int net_recv_all(NetSocket s, void* buf, size_t len, int timeout_ms);

/* A monotonic millisecond clock, and something unpredictable enough for a
   session token. */
uint64_t net_now_ms(void);
uint32_t net_random_u32(void);

/* Resolving a name and connecting to it can block for seconds, so it happens
   off the emulator's thread: start one, poll it, abandon it. Only one runs at
   a time. Poll returns NET_CONNECT_PENDING until it is finished, and hands
   over the socket exactly once. */
enum { NET_CONNECT_PENDING = 0,
    NET_CONNECT_READY = 1,
    NET_CONNECT_FAILED = -1 };

int net_connect_start(char const* host, uint16_t port, int udp);
int net_connect_poll(NetSocket* out);
void net_connect_cancel(void);

/* The address other machines would reach this one at, asked of a STUN server
   in the background. Empty until it is known, and never fatal if it is not. */
void net_extip_start(void);
int net_extip_busy(void);
char const* net_extip(void);

#endif
