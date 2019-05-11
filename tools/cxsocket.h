/*
 * cxsocket.h: Socket wrapper classes
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef __CXSOCKET_H
#define __CXSOCKET_H

#include <inttypes.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifdef __FreeBSD__
#include <netinet/in.h>
#endif
#include <unistd.h> // close()

#include "../logdefs.h"

#define CLOSESOCKET(fd) do { if(fd>=0) { ::close(fd); fd=-1; } } while(0)

class cxSocket {
 private:
  int m_fd;

  cxSocket(const cxSocket& s) ;//{ m_fd = s.m_fd>=0 ? dup(s.m_fd) : -1; }
  cxSocket &operator=(const cxSocket &S)
    ;// { close(); m_fd = S.m_fd >= 0 ? dup(S.m_fd) : -1; return *this; };

 public:

  typedef enum {
    estSTREAM = SOCK_STREAM,
    estDGRAM  = SOCK_DGRAM
  } eSockType;

  cxSocket() : m_fd(-1) {}
  cxSocket(eSockType type) : m_fd(::socket(PF_INET, (int)type, 0)) {}

  ~cxSocket() { CLOSESOCKET(m_fd); }

  //operator int ()  const { return Handle(); }
  //operator bool () const { return open(); }
  //bool operator==(const cxSocket &s) { return m_fd == s.m_fd; }

  int  handle(bool take_ownership=false)
       { int r=m_fd; if(take_ownership) m_fd=-1; return r; }
  void set_handle(int h) { if(h != m_fd) {close(); m_fd = h;} }
  bool create(eSockType type) { close(); return (m_fd=::socket(PF_INET, (int)type, 0)) >= 0; }
  bool open(void)   const { return m_fd >= 0; }
  void close(void)        { CLOSESOCKET(m_fd); }

  ssize_t send(const void *buf, size_t size, int flags=0, 
	       const struct sockaddr *to = NULL, socklen_t tolen = 0);
  ssize_t recv(void *buf, size_t size, int flags = 0,
	       struct sockaddr *from = NULL, socklen_t *fromlen = NULL);
  ssize_t sendfile(int fd_file, off_t *offset, size_t count);

  ssize_t read(void *buffer, size_t size, int timeout_ms = -1);
  ssize_t write(const void *buffer, size_t size, int timeout_ms = -1);

  ssize_t printf(const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
  ssize_t write_str(const char *str, int timeout_ms=-1, int len=0)
  { return write(str, len ?: strlen(str), timeout_ms); }
  ssize_t write_cmd(const char *str, int len=0)
  { return write(str, len ?: strlen(str), 10); }

/* readline return value:
 *   <0        : failed
 *   >=maxsize : buffer overflow
 *   >=0       : if errno = EAGAIN -> line is not complete (there was timeout)
 *               if errno = 0      -> succeed
 *               (return value 0 indicates empty line "\r\n")
 */
  ssize_t readline(char *buf, int bufsize, int timeout=0, int bufpos=0);

  bool set_buffers(int Tx, int Rx);
  bool set_multicast(int ttl);
  bool set_blocking(bool state);
  bool set_cork(bool state);
  bool flush_cork(void) { return set_nodelay(true); };
  bool set_nodelay(bool state);
  ssize_t tx_buffer_size(void);
  ssize_t tx_buffer_free(void);
  int getsockname(struct sockaddr *name, socklen_t *namelen);
  int getpeername(struct sockaddr *name, socklen_t *namelen);


  bool connect(struct sockaddr *addr, socklen_t len);
  bool connect(const char *ip, int port);

  uint32_t get_local_address(char *ip_address);

  static char *ip2txt(uint32_t ip, unsigned int port, char *str);
};


#include <vdr/tools.h>

class cxPoller : public cPoller {
  public:
    cxPoller(cxSocket& Sock, bool Out=false) : cPoller(Sock.handle(), Out) {};

    cxPoller(cxSocket* Socks, int count, bool Out=false) 
    {
      for(int i=0; i<count; i++)
	Add(Socks[i].handle(), Out);
    }
};

//
// Set socket buffers
//
static inline void set_socket_buffers(int s, int txbuf, int rxbuf)
{
  int max_buf = txbuf;
  /*while(max_buf) {*/
    errno = 0;
    if(setsockopt(s, SOL_SOCKET, SO_SNDBUF, &max_buf, sizeof(int))) {
      LOGERR("setsockopt(SO_SNDBUF,%d) failed", max_buf);
      /*max_buf >>= 1;*/
    }
    /*else {*/
      int tmp = 0;
      int len = sizeof(int);
      errno = 0;
      if(getsockopt(s, SOL_SOCKET, SO_SNDBUF, &tmp, (socklen_t*)&len)) {
	LOGERR("getsockopt(SO_SNDBUF,%d) failed", max_buf);
	/*break;*/
      } else if(tmp != max_buf) {
	LOGDBG("setsockopt(SO_SNDBUF): got %d bytes", tmp);
	/*max_buf >>= 1;*/
	/*continue;*/
      }
    /*}*/
  /*}*/

  max_buf = rxbuf;
  if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &max_buf, sizeof(int)) < 0) {
    LOGERR("setsockopt(SO_RCVBUF,%d) failed", max_buf);
  }
}

//
// Connect data socket to client (take address from fd_control)
//
static inline int sock_connect(int fd_control, int port, int type)
{
  union {
    struct sockaddr    sa;
    struct sockaddr_in in;
  } addr;
  socklen_t len = sizeof(addr);
  int             s, one = 1;

  if (getpeername(fd_control, &addr.sa, &len) < 0) {
    LOGERR("sock_connect: getpeername failed");
    return -1;
  }

  uint32_t tmp = ntohl(addr.in.sin_addr.s_addr);
  LOGMSG("Client address: %d.%d.%d.%d", 
	 ((tmp>>24)&0xff), ((tmp>>16)&0xff), 
	 ((tmp>>8)&0xff), ((tmp)&0xff));

#if 0
  if ((h = gethostbyname(tmp)) == NULL) {
    LOGDBG("sock_connect: unable to resolve host name", tmp);
  }
#endif

  if ((s = socket(PF_INET, type, 
		  type==SOCK_DGRAM?IPPROTO_UDP:IPPROTO_TCP)) < 0) {
    LOGERR("sock_connect: failed to create socket");
    return -1;
  }

  // Set socket buffers: large send buffer, small receive buffer
  set_socket_buffers(s, KILOBYTE(256), 2048);

  if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0)
    LOGERR("sock_connect: setsockopt(SO_REUSEADDR) failed");

  addr.in.sin_family = AF_INET;
  addr.in.sin_port   = htons(port);
  
  if (connect(s, &addr.sa, sizeof(addr)) < 0 &&
      errno != EINPROGRESS) {
    LOGERR("connect() failed");
    CLOSESOCKET(s);
  }

  if (fcntl (s, F_SETFL, fcntl (s, F_GETFL) | O_NONBLOCK) == -1) {
    LOGERR("can't put socket in non-blocking mode");
    CLOSESOCKET(s);
    return -1;
  }

  return s;
}

#endif // __CXSOCKET_H
