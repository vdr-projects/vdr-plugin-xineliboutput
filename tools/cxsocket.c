/*
 * cxsocket.c: Socket wrapper classes
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <vdr/config.h>
#include <vdr/tools.h>

#include "../logdefs.h"

#include "cxsocket.h"

bool cxSocket::connect(struct sockaddr *addr, socklen_t len)
{
  return ::connect(m_fd, addr, len) == 0;
}

bool cxSocket::connect(const char *ip, int port)
{
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = inet_addr(ip);

  return connect((struct sockaddr *)&sin, sizeof(sin));
}

bool cxSocket::set_blocking(bool state)
{
  int flags = fcntl (m_fd, F_GETFL);

  if(flags == -1) {
    LOGERR("cxSocket::SetBlocking: fcntl(F_GETFL) failed");
    return false;
  }

  flags = state ? (flags&(~O_NONBLOCK)) : (flags|O_NONBLOCK);

  if(fcntl (m_fd, F_SETFL, flags) == -1) {
    LOGERR("cxSocket::SetBlocking: fcntl(F_SETFL) failed");
    return false;
  }

  return true;
}

bool cxSocket::set_buffers(int Tx, int Rx)
{
  int max_buf = Tx;
  /*while(max_buf) {*/
    errno = 0;
    if(setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, &max_buf, sizeof(int))) {
      LOGERR("cxSocket: setsockopt(SO_SNDBUF,%d) failed", max_buf);
      /*max_buf >>= 1;*/
    }
    /*else {*/
      int tmp = 0;
      int len = sizeof(int);
      errno = 0;
      if(getsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, &tmp, (socklen_t*)&len)) {
	LOGERR("cxSocket: getsockopt(SO_SNDBUF,%d) failed", max_buf);
	/*break;*/
      } else if(tmp != max_buf) {
	LOGDBG("cxSocket: setsockopt(SO_SNDBUF): got %d bytes", tmp);
	/*max_buf >>= 1;*/
	/*continue;*/
      }
    /*}*/
  /*}*/

  max_buf = Rx;
  setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, &max_buf, sizeof(int));

  return true;
}

bool cxSocket::set_multicast(int ttl)
{
  int iReuse = 1, iLoop = 1, iTtl = ttl;

  errno = 0;

  if(setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, 
		&iReuse, sizeof(int)) < 0) {
    LOGERR("cxSocket: setsockopt(SO_REUSEADDR) failed");
    return false;
  }  

  if(setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_TTL, 
		&iTtl, sizeof(int))) {
    LOGERR("cxSocket: setsockopt(IP_MULTICAST_TTL) failed");
    return false;
  }
      
  if(setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_LOOP,
		&iLoop, sizeof(int))) {
    LOGERR("cxSocket: setsockopt(IP_MULTICAST_LOOP) failed");
    return false;
  }

  return true;
}

ssize_t cxSocket::send(const void *buf, size_t size, int flags,
		       const struct sockaddr *to, socklen_t tolen)
{
  return ::sendto(m_fd, buf, size, flags, to, tolen);
}

ssize_t cxSocket::recv(void *buf, size_t size, int flags,
		       struct sockaddr *from, socklen_t *fromlen)
{
  return ::recvfrom(m_fd, buf, size, flags, from, fromlen);
}

ssize_t cxSocket::write(const void *buffer, size_t size, 
			      int timeout_ms)
{
  ssize_t written = (ssize_t)size;
  const unsigned char *ptr = (const unsigned char *)buffer;
  cPoller poller(m_fd, true);

  while (size > 0) {
    errno = 0;
    if(!poller.Poll(timeout_ms)) {
      LOGERR("cxSocket::write: poll() failed");
      return written-size;
    }

    errno = 0;
    ssize_t p = ::write(m_fd, ptr, size);

    if (p <= 0) {
      if (errno == EINTR || errno == EAGAIN) {
	LOGDBG("cxSocket::write: EINTR during write(), retrying");
	continue;
      }
      LOGERR("cxSocket::write: write() error");
      return p;
    }

    ptr  += p;
    size -= p;    
  }

  return written;
}

ssize_t cxSocket::read(void *buffer, size_t size, int timeout_ms)
{
  ssize_t missing = (ssize_t)size;
  unsigned char *ptr = (unsigned char *)buffer;
  cPoller poller(m_fd);

  while (missing > 0) {

    if(!poller.Poll(timeout_ms)) {
      LOGERR("cxSocket::read: poll() failed at %d/%d", (int)(size-missing), (int)size);
      return size-missing;
    }

    errno = 0;
    ssize_t p = ::read(m_fd, ptr, missing);

    if (p <= 0) {
      if (errno == EINTR || errno == EAGAIN) {
	LOGDBG("cxSocket::read: EINTR/EAGAIN during read(), retrying");
	continue;
      }
      LOGERR("cxSocket::read: read() error at %d/%d", (int)(size-missing), (int)size);
      return size-missing;
    }

    ptr  += p;
    missing -= p;    
  }

  return size;
}

ssize_t cxSocket::printf(const char *fmt, ...)
{
  va_list argp;
  char buf[256];
  int r;

  va_start(argp, fmt);
  r = vsnprintf(buf, sizeof(buf), fmt, argp);
  if(r<0)
    LOGERR("cxSocket::printf: vsnprintf failed");
  else if(r >= (int)sizeof(buf))
    LOGMSG("cxSocket::printf: vsnprintf overflow (%20s)", buf);
  else
    return write(buf, r);
  
  return (ssize_t)-1;
}

/* readline return value:
 *   <0        : failed
 *   >=maxsize : buffer overflow
 *   >=0       : if errno = EAGAIN -> line is not complete (there was timeout)
 *               if errno = 0      -> succeed
 *               (return value 0 indicates empty line "\r\n")
 */
ssize_t cxSocket::readline(char *buf, int bufsize, int timeout, int bufpos)
{
  int n = -1, cnt = bufpos;
  cPoller p(m_fd);

  do {
    if(timeout>0 && !p.Poll(timeout)) {
      errno = EAGAIN;
      return cnt;
    }

    while((n = ::read(m_fd, buf+cnt, 1)) == 1) {
      buf[++cnt] = 0;

      if( cnt > 1 && buf[cnt - 2] == '\r' && buf[cnt - 1] == '\n') {
	cnt -= 2;
	buf[cnt] = 0;
	errno = 0;
	return cnt;
      }

      if( cnt >= bufsize) {
	LOGMSG("cxSocket::readline: too long control message (%d bytes): %20s", cnt, buf);
	errno = 0;
	return bufsize;
      }
    }

    /* connection closed ? */
    if (n == 0) {
      LOGMSG("cxSocket::readline: disconnected");
      if(errno == EAGAIN)
	errno = ENOTCONN;
      return -1;
    }

  } while (timeout>0 && n<0 && errno == EAGAIN);

  if(errno == EAGAIN)
    return cnt;

  LOGERR("cxSocket::readline: read failed");
  return n;
}

#include <sys/ioctl.h>
#include <net/if.h>

uint32_t cxSocket::get_local_address(char *ip_address)
{
  uint32_t local_addr = 0;
  struct ifconf conf;
  struct ifreq buf[3];
  unsigned int n;

  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);

  if(!getsockname(m_fd, (struct sockaddr *)&sin, &len)) {
    local_addr = sin.sin_addr.s_addr;

  } else {
    //LOGERR("getsockname failed");

    // scan network interfaces

    conf.ifc_len = sizeof(buf);
    conf.ifc_req = buf;
    memset(buf, 0, sizeof(buf));
    
    errno = 0;
    if(ioctl(m_fd, SIOCGIFCONF, &conf) < 0)
      LOGERR("cxSocket: can't obtain socket local address");
    else {
      for(n=0; n<conf.ifc_len/sizeof(struct ifreq); n++) {
	struct sockaddr_in *in = (struct sockaddr_in *) &buf[n].ifr_addr;
# if 0
	uint32_t tmp = ntohl(in->sin_addr.s_addr);
	LOGMSG("Local address %6s %d.%d.%d.%d", 
	       conf.ifc_req[n].ifr_name,
	       ((tmp>>24)&0xff), ((tmp>>16)&0xff), 
	       ((tmp>>8)&0xff), ((tmp)&0xff));
# endif
	if(n==0 || local_addr == htonl(INADDR_LOOPBACK)) 
	  local_addr = in->sin_addr.s_addr;
	else
	  break;
      }
    }
  }

  if(!local_addr)
    LOGERR("No local address found");

  if(ip_address)
    cxSocket::ip2txt(local_addr, 0, ip_address);

  return local_addr;  
}

char *cxSocket::ip2txt(uint32_t ip, unsigned int port, char *str)
{
  // inet_ntoa is not thread-safe (?)
  if(str) {
    unsigned int iph =(unsigned int)ntohl(ip);
    unsigned int porth =(unsigned int)ntohs(port);
    if(!porth)
      sprintf(str, "%d.%d.%d.%d", 
	      ((iph>>24)&0xff), ((iph>>16)&0xff), 
	      ((iph>>8)&0xff), ((iph)&0xff));
    else
      sprintf(str, "%u.%u.%u.%u:%u", 
	      ((iph>>24)&0xff), ((iph>>16)&0xff), 
	      ((iph>>8)&0xff), ((iph)&0xff),
	      porth);
  }
  return str;
}
