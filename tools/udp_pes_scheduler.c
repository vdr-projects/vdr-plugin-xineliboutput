/*
 * udp_pes_scheduler.h: PES scheduler for UDP/RTP streams
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS 
#include <inttypes.h>

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/unistd.h>

#include <vdr/config.h>
#include <vdr/tools.h>

#include "../logdefs.h"

#include "udp_buffer.h"
#include "pes.h"

#include "udp_pes_scheduler.h"

#include "../xine_input_vdr_net.h" // frame headers and constants
#include "../config.h"             // configuration data

#include "cxsocket.h"
#include "sap.h"  // SAP  - Session Announcement Protocol
#include "sdp.h"  // SDP  - Session Description Protocol
#include "rtcp.h" // RTCP

//----------------------- cTimePts ------------------------------------------

#include <time.h>

cTimePts::cTimePts(void)
{
  m_Paused     = false;
  m_Multiplier = 90000;
  m_Monotonic  = false;

#if _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK)
  struct timespec resolution;   

  if(clock_getres(CLOCK_MONOTONIC, &resolution)) {
    LOGERR("cTimePts: clock_getres(CLOCK_MONOTONIC) failed");
  } else {
    LOGDBG("cTimePts: clock_gettime(CLOCK_MONOTONIC): clock resolution %d us",
	   ((int)resolution.tv_nsec) / 1000);

    if( resolution.tv_sec == 0 && resolution.tv_nsec <= 1000000 ) {
      struct timespec tp;
      if(clock_gettime(CLOCK_MONOTONIC, &tp)) {
	LOGERR("cTimePts: clock_gettime(CLOCL_MONOTONIC) failed");
      } else {
	LOGDBG("cTimePts: using monotonic clock");
	m_Monotonic = true;
      }
    }
  }
#else
#  warning Posix monotonic clock not available
#endif

  Set();
}

int64_t cTimePts::Now(void)
{
  if(m_Paused)
    return begin;

  struct timeval t;

#if _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK)
  if(m_Monotonic) {
    struct timespec tp;

    if(clock_gettime(CLOCK_MONOTONIC, &tp)) {
      LOGERR("cTimePts: clock_gettime(CLOCK_MONOTONIC) failed");
      return -1;
    }

    t.tv_sec  = tp.tv_sec;
    t.tv_usec = tp.tv_nsec/1000;

  } else if (gettimeofday(&t, NULL)) {
    LOGERR("cTimePts: gettimeofday() failed");
    return -1;
  }
#else
  if (gettimeofday(&t, NULL)) {
    LOGERR("cTimePts: gettimeofday() failed");
    return -1;
  }
#endif

  t.tv_sec -= tbegin.tv_sec;
  if(t.tv_usec < tbegin.tv_usec) {
    t.tv_sec--;
    t.tv_usec += 1000000;
  }
  t.tv_usec -= tbegin.tv_usec;
  
  return ( ((int64_t)t.tv_sec) * ((int64_t)m_Multiplier) + 
	   ((int64_t)t.tv_usec) * INT64_C(90) / INT64_C(1000) +
	   begin) & MAX_SCR;

}

void cTimePts::Set(int64_t Pts)
{
  begin = Pts;

#if _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK)
  if(m_Monotonic) {
    struct timespec tp;

    if(!clock_gettime(CLOCK_MONOTONIC, &tp)) {
      tbegin.tv_sec  = tp.tv_sec;
      tbegin.tv_usec = tp.tv_nsec/1000;
      return;
    }

    LOGERR("cTimePts: clock_gettime(CLOCL_MONOTONIC) failed");
    m_Monotonic = false;
  }
#endif

  gettimeofday(&tbegin, NULL);
}

void cTimePts::Pause(void)
{
  Set(Now());
  m_Paused = true;
}

void cTimePts::Resume(void)
{
  m_Paused = false;
}

void cTimePts::TrickSpeed(int Multiplier)
{
  if(Multiplier < 0)
    m_Multiplier = 90000 / (-Multiplier);
  else if(Multiplier > 0)
    m_Multiplier = 90000 * Multiplier;
  else
    LOGERR("cTimePts::SetSpeed: Multiplier=%d", Multiplier);
}

//----------------------- cUdpScheduler -------------------------------------

#ifdef LOG_RESEND
#  define LOGRESEND LOGDBG
#else
#  define LOGRESEND(x...)
#endif

#ifdef LOG_SCR
#  define LOGSCR  LOGDBG
#else
#  define LOGSCR(x...)
#endif


const int MAX_QUEUE_SIZE      = 64;       // ~ 65 ms with typical DVB stream
const int MAX_LIVE_QUEUE_SIZE = (64+60);  // ~ 100 ms with typical DVB stream
const int HARD_LIMIT          = (4*1024); // ~ 40 Mbit/s === 4 Mb/s

// initial burst length after seek (500ms = ~13 video frames)
const int64_t INITIAL_BURST_TIME  = (int64_t)(45000); // pts units (90kHz)

// assume seek when when pts difference between two frames exceeds this (1.5 seconds)
const int64_t JUMP_LIMIT_TIME = (int64_t)(3*90000/2); // pts units (90kHz)

const int RTCP_MIN_INTERVAL = 45000; // max. twice in second

typedef enum {
  eScrDetect,
  eScrFromAudio,
  eScrFromPS1,
  eScrFromVideo
} ScrSource_t;

#ifdef LOG_SCR
int data_sent;   /* in current time interval, bytes */
int frames_sent; /* in current time interval */
int frame_rate;  /* pes frames / second */
int prev_frames;
#endif

cUdpScheduler::cUdpScheduler()
{

  // Scheduler data

  current_audio_vtime = 0;
  current_video_vtime = 0;
  MasterClock.Set(INT64_C(0));

#ifdef LOG_SCR
  data_sent = 0; 
  frames_sent = 0;
  frame_rate = 2000;
  prev_frames = 200;
#endif

  last_delay_time = 0;

  // RTP

  srandom(time(NULL) ^ getpid());

  m_ssrc = random();
  LOGDBG("RTP SSRC: 0x%08x", m_ssrc);
  m_LastRtcpTime = 0;
  m_Frames = 0;
  m_Octets = 0;
  RtpScr.Set((int64_t)random());

  m_fd_rtp = -1;
  m_fd_rtcp = -1;
  m_fd_sap = -1;

  // Queuing

  int i;
  for(i=0; i<MAX_UDP_HANDLES; i++)
    m_Handles[i] = -1;

  m_BackLog = new cUdpBackLog;

  m_QueueNextSeq = 0;
  m_QueuePending = 0;

  // Thread

  m_Running = 1;

  Start();
}

cUdpScheduler::~cUdpScheduler()
{
  if(m_fd_rtcp >= 0 || m_fd_rtp >= 0) {
    Send_SAP(false);
    close(m_fd_rtp);
    m_fd_rtp = -1;
    close(m_fd_rtcp);
    m_fd_rtcp = -1;
  }
  close(m_fd_sap);
  m_fd_sap = -1;

  m_Lock.Lock();
  m_Running = 0;
  m_Cond.Broadcast();
  m_Lock.Unlock();

  Cancel(3);

  delete m_BackLog;
}

bool cUdpScheduler::AddRtp(void) 
{
  cMutexLock ml(&m_Lock);

  int fd_rtp = -1, fd_rtcp = -1;
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(xc.remote_rtp_port);
  sin.sin_addr.s_addr = inet_addr(xc.remote_rtp_addr);


  if(m_fd_rtp >= 0) {
    LOGERR("cUdpScheduler::AddHandle: RTP socket already open !");
    Send_SAP(false);
    close(m_fd_rtp);
    m_fd_rtp = -1;
  }

  if(m_fd_rtcp >= 0) {
    LOGERR("cUdpScheduler::AddHandle: RTCP socket already open !");
    close(m_fd_rtcp);
    m_fd_rtcp = -1;
  }

  /* need new ssrc */
  m_ssrc = random();
  LOGDBG("RTP SSRC: 0x%08x", m_ssrc);

  //
  // RTP
  //
  if((fd_rtp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    LOGERR("socket() failed (UDP/RTP multicast)");
    return false;
  }

  // Set buffer sizes
  set_socket_buffers(fd_rtp, KILOBYTE(256), 2048);

  // Set multicast socket options
  if(set_multicast_options(fd_rtp, xc.remote_rtp_ttl)) {
    close(fd_rtp);
    return false;
  }

  // Connect to multicast address
  if(connect(fd_rtp, (struct sockaddr *)&sin, sizeof(sin)) == -1 && 
     errno != EINPROGRESS) {
    LOGERR("connect(fd_rtp) failed. Address=%s, port=%d",
	   xc.remote_rtp_addr, xc.remote_rtp_port);
    close(fd_rtp);
    return false;
  }

  // Set to non-blocking mode
  if(fcntl (fd_rtp, F_SETFL, 
	    fcntl (fd_rtp, F_GETFL) | O_NONBLOCK) == -1) 
    LOGERR("can't put multicast socket in non-blocking mode");      
  
  
  //
  // RTCP
  //
  if((fd_rtcp = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    LOGERR("socket() failed (RTCP multicast)");
      
  /* RTCP port (RFC 1889) */
  if(xc.remote_rtp_port & 1)
    sin.sin_port = htons(xc.remote_rtp_port - 1);
  else
    sin.sin_port = htons(xc.remote_rtp_port + 1);
      
  set_socket_buffers(fd_rtcp, 16384, 16384);
  if(set_multicast_options(fd_rtcp, xc.remote_rtp_ttl)) {
    close(fd_rtcp);
    fd_rtcp = -1;
  }
      
  else if(connect(fd_rtcp, (struct sockaddr *)&sin, sizeof(sin))==-1 && 
	  errno != EINPROGRESS) {
    LOGERR("connect(fd_rtcp) failed. Address=%s, port=%d",
	   xc.remote_rtp_addr, xc.remote_rtp_port +
	   (xc.remote_rtp_port&1)?-1:1);
    close(fd_rtcp);
    fd_rtcp = -1;
  }

  // Set to non-blocking mode
  else if(fcntl (fd_rtcp, F_SETFL, 
		 fcntl (fd_rtcp, F_GETFL) | O_NONBLOCK) == -1) 
    LOGERR("can't put multicast socket in non-blocking mode");

  // Finished

  if(!AddHandle(fd_rtp)) {
    LOGERR("cUdpScheduler::AddHandle(fd_rtp) failed");
  }

  m_fd_rtp = fd_rtp;
  m_fd_rtcp = fd_rtcp;
	
  Send_SAP(true);

  return true;
}

bool cUdpScheduler::AddHandle(int fd) 
{
  cMutexLock ml(&m_Lock);

  int i;

  for(i=0; i<MAX_UDP_HANDLES; i++)
    if(m_Handles[i] < 0 || m_Handles[i] == fd) {
      m_Handles[i] = fd;

      /* query socket send buffer size */
      m_wmem[i] = 0x10000; /* default to 64k */
      socklen_t l = sizeof(int);
      if(getsockopt(m_Handles[i], SOL_SOCKET, SO_SNDBUF, &m_wmem[i], &l))
	LOGERR("getsockopt(SO_SNDBUF) failed");
      m_wmem[i] /= 2; /* man 7 socket */

      m_Cond.Broadcast();

      return true;
    }

  return false;
}

void cUdpScheduler::RemoveRtp(void) 
{
  cMutexLock ml(&m_Lock);

  if(m_fd_rtp >= 0 || m_fd_rtcp >= 0) {
    Send_SAP(false);

    RemoveHandle(m_fd_rtp);
    
    close(m_fd_rtp);
    m_fd_rtp = -1;
    close(m_fd_rtcp);
    m_fd_rtcp = -1;
    close(m_fd_sap);
    m_fd_sap = -1;
  }
}

void cUdpScheduler::RemoveHandle(int fd) 
{
  cMutexLock ml(&m_Lock);

  int i;
  for(i=0; i<MAX_UDP_HANDLES; i++)
    if(m_Handles[i] == fd)
      break;

  for(; i<MAX_UDP_HANDLES-1; i++)
    m_Handles[i] = m_Handles[i+1];

  m_Handles[MAX_UDP_HANDLES-1] = -1;

  if(m_Handles[0] < 0) {
    // No clients left ...

    // Flush all buffers
    m_QueueNextSeq = 0;
    m_QueuePending = 0;
    delete m_BackLog; 
    m_BackLog = new cUdpBackLog;

    m_Frames = 0;
    m_Octets = 0;
  }
}

bool cUdpScheduler::Poll(int TimeoutMs, bool Master)
{
  cMutexLock ml(&m_Lock);

  m_Master = Master;

  if(m_Handles[0] < 0) {
    // no clients, so we can eat all data we are given ...
    return true;
  }
  
  int limit = m_Master ? MAX_QUEUE_SIZE : MAX_LIVE_QUEUE_SIZE;
  if(m_QueuePending >= limit) {
    uint64_t WaitEnd = cTimeMs::Now();
    if(TimeoutMs >= 0)
      WaitEnd += (uint64_t)TimeoutMs;

    while(cTimeMs::Now() < WaitEnd &&
	  m_Running &&
	  m_QueuePending >= limit)	
      m_Cond.TimedWait(m_Lock, 5);
  }

  return m_QueuePending < limit;
}

bool cUdpScheduler::Flush(int TimeoutMs)
{
  cMutexLock ml(&m_Lock);

  if(m_Handles[0] < 0)
    return true;
  
  if(m_QueuePending > 0) {
    uint64_t WaitEnd = cTimeMs::Now();
    if(TimeoutMs >= 0)
      WaitEnd += (uint64_t)TimeoutMs;

    while(cTimeMs::Now() < WaitEnd &&
	  m_Running &&
	  m_QueuePending > 0)
      m_Cond.TimedWait(m_Lock, 5);
  }
  return m_QueuePending == 0;
}

void cUdpScheduler::Clear(void)
{
  cMutexLock ml(&m_Lock);

  m_BackLog->Clear(m_QueuePending);

  m_QueuePending = 0;
  m_Cond.Broadcast();
}

void cUdpScheduler::Pause(bool On)
{
  cMutexLock ml(&m_Lock);

  if(On)
    MasterClock.Pause();
  else
    MasterClock.Resume();
}

void cUdpScheduler::TrickSpeed(int Multiplier)
{
  cMutexLock ml(&m_Lock);

#ifdef LOG_SCR
  if(Multiplier == 1 || Multiplier == -1)
    LOGMSG("UDP clock --> normal");
  else if(Multiplier < 0)
    LOGMSG("UDP clock --> %dx", -Multiplier);
  else
    LOGMSG("UDP clock --> 1/%d", Multiplier);
#endif

  MasterClock.TrickSpeed(Multiplier);
}

bool cUdpScheduler::Queue(uint64_t StreamPos, const uchar *Data, int Length) 
{
  cMutexLock ml(&m_Lock);

  if(m_Handles[0] < 0) 
    return true;

  int limit = m_Master ? MAX_QUEUE_SIZE : MAX_LIVE_QUEUE_SIZE;
  if(m_QueuePending >= limit)
    return false;

  m_BackLog->MakeFrame(StreamPos, Data, Length); 
  m_QueuePending++;

  m_Cond.Broadcast();

  return true;
}

int cUdpScheduler::calc_elapsed_vtime(int64_t pts, bool Audio) 
{
  int64_t diff = 0;

  if(!Audio /*Video*/) {
    /* #warning TODO: should be possible to use video pts too (if ac3 or muted ...) */
    diff = pts - current_video_vtime;
    if(diff < 0) diff = -diff;
    if(diff > JUMP_LIMIT_TIME) { // 1 s (must be > GOP)
      // RESET
#ifdef LOG_SCR
      LOGDBG("cUdpScheduler RESET (Video jump %lld->%lld)",
	     current_audio_vtime, pts);
      data_sent = frames_sent = 0;
#endif
      //diff = 0;
      current_video_vtime = pts;
      return -1;
    }
    current_video_vtime = pts;
    
  } else if(Audio) {
    diff = pts - current_audio_vtime;
    if(diff < 0) diff = -diff;
    if(diff > JUMP_LIMIT_TIME) { // 1 sec
      // RESET
#ifdef LOG_SCR
      LOGDBG("cUdpScheduler RESET (Audio jump %lld->%lld)",
	     current_audio_vtime, pts);
      data_sent = frames_sent = 0;
#endif
      //diff = 0;
      current_audio_vtime = pts;

      // Use audio pts for sync (audio has constant and increasing intervals)
      MasterClock.Set(current_audio_vtime + INITIAL_BURST_TIME);
      
      return -1;
    }
    current_audio_vtime = pts;
  }
 
#ifdef LOG_SCR
  if(diff && Audio) {
    frame_rate = (int)(90000*frames_sent/(int)diff);
    LOGDBG("rate %d kbit/s (%d frames/s)",
	   (int)(90*data_sent/((int)diff)*8), frame_rate);
    prev_frames = frames_sent;
    data_sent = frames_sent = 0;
  }
#endif
  
  return (int) diff;
}

void cUdpScheduler::Send_RTCP(void)
{
  if(m_fd_rtcp < 0)
    return;

  uint64_t scr = RtpScr.Now();

  if(scr > (m_LastRtcpTime + RTCP_MIN_INTERVAL)) {
    uint8_t frame[2048], *content = frame;
    char hostname[64] = "";
    rtcp_packet_t  *msg = (rtcp_packet_t *)content;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    gethostname(hostname, 63);

    // SR (Sender report)
    msg->hdr.raw[0] = 0x81;     // RTP version = 2, Report count = 1 */
    msg->hdr.ptype  = RTCP_SR;
    msg->hdr.length = htons(6); // length 6 dwords

    msg->sr.ssrc     = htonl(m_ssrc);
    msg->sr.ntp_sec  = htonl(tv.tv_sec + 0x83AA7E80);
    msg->sr.ntp_frac = htonl((uint32_t)((double)tv.tv_usec*(double)(1LL<<32)*1.0e-6));
    msg->sr.rtp_ts   = htonl((uint32_t)(scr    & 0xffffffff));
    msg->sr.psent    = htonl((uint32_t)(m_Frames & 0xffffffff));
    msg->sr.osent    = htonl((uint32_t)(m_Octets & 0xffffffff));

    content += sizeof(rtcp_common_t) + sizeof(rtcp_sr_t);
    msg = (rtcp_packet_t *)content;

    // SDES
    msg->hdr.raw[0] = 0x81;       // RTP version = 2, Report count = 1 */
    msg->hdr.ptype  = RTCP_SDES;  
    msg->hdr.count  = 1;

    msg->sdes.ssrc   = m_ssrc;
    msg->sdes.item[0].type   = RTCP_SDES_CNAME;
    sprintf(msg->sdes.item[0].data, "VDR@%s:%d%c%c%c", 
	    hostname[0] ? hostname : xc.remote_rtp_addr,
	    xc.remote_rtp_port, 0, 0, 0);
    msg->sdes.item[0].length = strlen(msg->sdes.item[0].data);
    msg->hdr.length = htons(1 + 1 + ((msg->sdes.item[0].length - 2) + 3) / 4); 
    
    content += sizeof(rtcp_common_t) + 4*ntohs(msg->hdr.length);
    msg = (rtcp_packet_t *)content;

    // Send
#ifndef LOG_RTCP
    (void) send(m_fd_rtcp, frame, content - frame, 0);
#else
    LOGMSG("RTCP send (%d)", send(m_fd_rtcp, frame, content - frame, 0));
    for(int i=0; i<content-frame; i+=16) 
      LOGMSG("%02X %02X %02X %02X %02X %02X %02X %02X  "
	     "%02X %02X %02X %02X %02X %02X %02X %02X  "
	     "  %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c",
	     frame[i+0],frame[i+1],frame[i+2],frame[i+3],
	     frame[i+4],frame[i+5],frame[i+6],frame[i+7],
	     frame[i+8],frame[i+9],frame[i+10],frame[i+11],
	     frame[i+12],frame[i+13],frame[i+14],frame[i+15],
	     frame[i+0],frame[i+1],frame[i+2],frame[i+3],
	     frame[i+4],frame[i+5],frame[i+6],frame[i+7],
	     frame[i+8],frame[i+9],frame[i+10],frame[i+11],
	     frame[i+12],frame[i+13],frame[i+14],frame[i+15]);
#endif

    m_LastRtcpTime = scr;
  }
}

#include <sys/ioctl.h>
#include <net/if.h>

static uint32_t get_local_address(int fd, char *ip_address)
{
  uint32_t local_addr = 0;
  struct ifconf conf;
  struct ifreq buf[3];
  unsigned int n;

  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);

  if(!getsockname(fd, (struct sockaddr *)&sin, &len)) {
    local_addr = sin->sin_addr.s_addr;

  } else {
    //LOGERR("getsockname failed");

    // scan network interfaces

    conf.ifc_len = sizeof(buf);
    conf.ifc_req = buf;
    memset(buf, 0, sizeof(buf));
    
    errno = 0;
    if(ioctl(fd, SIOCGIFCONF, &conf) < 0)
      LOGERR("can't obtain socket local address");
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
    ip2txt(local_addr, 0, ip_address);

  return local_addr;  
}

void cUdpScheduler::Send_SAP(bool Announce)
{
  if(xc.remote_rtp_sap && m_fd_rtp >= 0) {
    char ip[20] = "";
    uint32_t local_addr = get_local_address(m_fd_rtp, ip);
    
    if(local_addr) {
      const char *sdp_descr = vdr_sdp_description(ip,
						  2001,
						  xc.listen_port,
						  xc.remote_rtp_addr,
						  m_ssrc,
						  xc.remote_rtp_port,
						  xc.remote_rtp_ttl);
#if 1
      /* store copy of SDP data */
      if(m_fd_sap < 0) {
	FILE *fp = fopen("/video/xineliboutput.sdp", "w");
	fprintf(fp, "%s", sdp_descr);
	fclose(fp);
      }
#endif
      sap_pdu_t *pdu = sap_create_pdu(local_addr,
				      Announce, 
				      (m_ssrc >> 16 | m_ssrc) & 0xffff,
				      "application/sdp",
				      sdp_descr);
      
      if(!sap_send_pdu(&m_fd_sap, pdu, 0))
	LOGERR("SAP/SDP announce failed");
      free(pdu);
    }
  }
}

void cUdpScheduler::Schedule(const uchar *Data, int Length)
{
  bool Audio=false, Video=false;
  int64_t pts = pes_extract_pts(Data, Length, Audio, Video);
  int elapsed = pts>0 ? calc_elapsed_vtime(pts, Audio) : 0;

#ifdef LOG_SCR
  if(elapsed > 0)
    LOGMSG("PTS: %lld  (%s) elapsed %d ms (PID %02x)", 
	   pts, Video?"Video":Audio?"Audio":"?", elapsed/90, Data[3]);
#endif

  if(elapsed > 0 && Audio/*Video*/) {
    int64_t now = MasterClock.Now();
    if(now > current_audio_vtime && (now - current_audio_vtime)>JUMP_LIMIT_TIME) {
#ifdef LOG_SCR
      LOGMSG("cUdpScheduler MasterClock init (was in past)");
      elapsed = -1;
#endif
      MasterClock.Set(current_audio_vtime + INITIAL_BURST_TIME);
    } else if(now < current_audio_vtime && (current_audio_vtime-now)>JUMP_LIMIT_TIME) {
#ifdef LOG_SCR
      LOGMSG("cUdpScheduler MasterClock init (was in future)");
      elapsed = -1;
#endif
      MasterClock.Set(current_audio_vtime + INITIAL_BURST_TIME);
    } else if(!last_delay_time) {
      // first burst done, no delay yet ???
      // (queue up to xxx bytes first)
    } else {
      if(current_audio_vtime > now) {
	int delay_ms = (int)(current_audio_vtime - now)/90;
#ifdef LOG_SCR
	LOGDBG("cUdpScheduler sleeping %d ms "
	       "(time reference: %s, beat interval %d ms)",
	       delay_ms, (Audio?"Audio PTS":"Video PTS"), elapsed);
#endif
	if(delay_ms > 3) {
	  //LOGMSG("sleep %d ms (%d f)", delay_ms, prev_frames);
	  CondWait.Wait(delay_ms);
	}
      }
    }
    last_delay_time = now;
  }
  
#if 0
  static int win = 0;
  static int64_t prev;

  if(data_sent == 0 || elapsed < 0) {
    win = 0;
    prev = MasterClock.Now();
  }
  win ++;
  int mrate = 3*frame_rate/2;
  if(mrate < 100) mrate = 100;
  if(mrate > 2000) mrate = 2000;
  if(MasterClock.Now() - prev >= win*90000 / frame_rate) {
    LOGMSG("sleep:3");
    CondWait.Wait(3);
  }
#endif

#ifdef LOG_SCR
  data_sent += Length;
  frames_sent ++;
#endif
}

void cUdpScheduler::Action(void)
{
#if 0
  {
    // Request real-time scheduling
    sched_param temp;
    temp.sched_priority = 2;
    
    if (!pthread_setschedparam(pthread_self(), SCHED_RR, &temp)) {
      LOGMSG("cUdpScheduler priority set successful SCHED_RR %d [%d,%d]",
	     temp.sched_priority,
	     sched_get_priority_min(SCHED_RR),
	     sched_get_priority_max(SCHED_RR));
    } else {
      LOGMSG("cUdpScheduer: Can't set priority to SCHED_RR %d [%d,%d]",
	     temp.sched_priority,
	     sched_get_priority_min(SCHED_RR),
	     sched_get_priority_max(SCHED_RR));
    }
  }
#endif

  /* UDP Scheduler needs high priority */
  SetPriority(-5);
  (void)nice(-5);
  errno = 0;

  m_Lock.Lock();

  while(m_Running) {
	
    if(m_Handles[0] < 0) {
      m_Cond.TimedWait(m_Lock, 5000); 
      continue;
    }

    // Wait until we have outgoing data in queue
    if(m_QueuePending <= 0) {
      m_Cond.TimedWait(m_Lock, 100); 
      if(m_QueuePending <= 0) {
	// Still nothing...
	// Send padding frame once in 100ms so clients can detect 
	// possible missing frames and server shutdown
	static unsigned char padding[] = {0x00,0x00,0x01,0xBE,0x00,0x02,0xff,0xff};
	int prevseq = (m_QueueNextSeq + UDP_BUFFER_SIZE - 1) & UDP_BUFFER_MASK;
	stream_rtp_header_impl_t *frame = m_BackLog->Get(prevseq);
	if(frame) {
	  int prevlen = m_BackLog->PayloadSize(prevseq);
	  uint64_t pos = ntohll(frame->hdr_ext.pos) + prevlen - 8;
	  m_BackLog->MakeFrame(pos, padding, 8);
	} else
	  m_BackLog->MakeFrame(0, padding, 8);
	m_QueuePending++;
      }
      continue; // to check m_Running
    }

    // Take next frame from queue
    stream_rtp_header_impl_t *frame = m_BackLog->Get(m_QueueNextSeq);
    int PayloadSize  = m_BackLog->PayloadSize(m_QueueNextSeq);
    int UdpPacketLen = PayloadSize + sizeof(stream_udp_header_t);
    int RtpPacketLen = PayloadSize + sizeof(stream_rtp_header_impl_t);

    m_QueueNextSeq = (m_QueueNextSeq + 1) & UDP_BUFFER_MASK;
    m_QueuePending--;

    m_Cond.Broadcast();
 
    m_Lock.Unlock();

    // Schedule frame
    if(m_Master)
      Schedule(frame->payload, PayloadSize);

    // Need some bandwidth limit for ex. sequence of still frames when 
    // moving cutting marks very fast (no audio or PTS available) 
#if 1
    // hard limit for used bandwidth:
    // - ~1 frames/ms & 8kb/ms -> 8mb/s -> ~ 80 Mbit/s ( / client)
    // - max burst 15 frames or 30kb
    static int cnt = 0, bytes = 0;
    static uint64_t dbg_timer = cTimeMs::Now();
    static int dbg_bytes = 0;
    cnt++; 
    bytes += PayloadSize;
    if(cnt>=15 && bytes >= 30000) {
      CondWait.Wait(4);
      dbg_bytes += bytes;
      cnt = 0; 
      bytes = 0;
      if(dbg_timer+60000 <= cTimeMs::Now()) {
# if 0
	LOGDBG("UDP rate: %4d Kbps (queue %d)", dbg_bytes/(60*1024/8),
	       m_QueuePending);
# endif
	dbg_bytes = 0;
	dbg_timer = cTimeMs::Now();
      }
    }
#endif

    /* tag frame with ssrc and timestamp */
    frame->rtp_hdr.ts   = htonl((uint32_t)(RtpScr.Now() & 0xffffffff));
    frame->rtp_hdr.ssrc = htonl(m_ssrc);

    /* deliver to all active sockets */
    for(int i=0; i<MAX_UDP_HANDLES && m_Handles[i]>=0; i++) {

      //
      // use TIOCOUTQ ioctl instead of poll/select.
      // - poll/select for UDP/RTP may return true even when queue 
      //   is (almost) full
      // - kernel silently drops frames it cant send
      // -> poll() + send() just causes frames to be dropped
      //
      int size = 0;
      if(!ioctl(m_Handles[i], TIOCOUTQ, &size)) {
	if(size >= (m_wmem[i] - 2*RtpPacketLen)) {
	  LOGMSG("cUdpScheduler: kernel transmit queue > ~%dkb (max %dkb) ! (master=%d)", 
		 (m_wmem[i] - 2*RtpPacketLen)/1024, m_wmem[i]/1024, m_Master);
	  CondWait.Wait(2);
	}
      }	else {
	if(m_QueuePending > (MAX_QUEUE_SIZE-5))
	  LOGDBG("cUdpScheduler: kernel transmit queue > ~30kb ! (master=%d ; Queue=%d)", 
		 m_Master, m_QueuePending);
	CondWait.Wait(2);
      }

      if(m_Handles[i] == m_fd_rtp) {
	if(send(m_Handles[i], frame, RtpPacketLen, 0) <= 0)
	  LOGERR("cUdpScheduler: UDP/RTP send() failed !");
      } else {
	/* UDP: send without rtp header */
	if(send(m_Handles[i], 
		((uint8_t*)frame) + sizeof(stream_rtp_header_impl_t) - sizeof(stream_udp_header_t), 
		UdpPacketLen, 0) <= 0)
	  LOGERR("cUdpScheduler: UDP send() failed !");
      }
    }

    m_Lock.Lock();
    m_Frames ++;
    m_Octets += PayloadSize;
    if(m_fd_rtcp >= 0 && (m_Frames & 0xff) == 1) { // every 256th frame
      Send_RTCP();
#if 0
      if((m_Frames & 0xff00) == 0) // every 65536th frame (~ 2 min)
	Send_SAP();
#else
      if((m_Frames & 0x0300) == 0) // every 1024th frame (~ 2...4 sec)
	Send_SAP();
#endif
    }
  }
  
  m_Lock.Unlock();
}

void cUdpScheduler::ReSend(int fd, uint64_t Pos, int Seq1, int Seq2) 
{
  if(fd < 0) /* no re-send for RTP */
    return;

  char udp_ctrl[64] = {0};
  ((stream_udp_header_t *)udp_ctrl)->seq = (uint16_t)(-1);
  ((stream_udp_header_t *)udp_ctrl)->pos = (uint64_t)(-1);

  // Handle buffer wrap
  if(Seq1 > Seq2)
    Seq2 += UDP_BUFFER_SIZE;

  cMutexLock ml(&m_Lock); // keeps also scheduler thread suspended ...

  if(Seq2-Seq1 > 64) {
    LOGDBG("cUdpScheduler::ReSend: requested range too large (%d-%d)",
	   Seq1, Seq2);

    sprintf((udp_ctrl+sizeof(stream_udp_header_t)),
	    "UDP MISSING %d-%d %" PRIu64,
	    Seq1, (Seq2 & UDP_BUFFER_MASK), Pos);
    send(fd, udp_ctrl, 64, 0);
    return;
  }

  // re-send whole range
  for(; Seq1 <= Seq2; Seq1++) {
      
    // Wait if kernel queue is full
    int size = 0;
    if(!ioctl(fd, TIOCOUTQ, &size))
      if(size > ((0x10000)/2 - 2048)) { // assume 64k kernel buffer
	LOGDBG("cUdpScheduler::ReSend: kernel transmit queue > ~30kb !");
	cCondWait::SleepMs(2);
      }
    
    stream_rtp_header_impl_t *frame = m_BackLog->Get(Seq1);
      
    if(frame) {
      if(ntohull(frame->hdr_ext.pos) - Pos < 100000) {
	send(fd, 
	     ((uint8_t*)frame) + sizeof(stream_rtp_header_impl_t) - sizeof(stream_udp_header_t), 
	     m_BackLog->PayloadSize(Seq1) + sizeof(stream_udp_header_t), 
	     0);
	LOGRESEND("cUdpScheduler::ReSend: %d (%d bytes) @%lld sent", 
		  Seq1, m_BackLog->PayloadSize(Seq1), Pos);
	Pos = ntohull(frame->hdr_ext.pos) + m_BackLog->PayloadSize(Seq1);
	continue;
      } else {
	// buffer has been lost long time ago...
	LOGRESEND("cUdpScheduler::ReSend: Requested position does not match "
	       "(%lld ; has %lld)", Pos, ntohll(frame->hdr_ext.pos));
      }
    } else {
      LOGRESEND("cUdpScheduler::ReSend: %d @%lld missing", Seq1, Pos);
    }

    // buffer has been lost - send packet missing info

    LOGRESEND("cUdpScheduler::ReSend: missing %d-%d @%d (hdr 0x%llx 0x%x)",
	   Seq1, Seq1, Pos,
	   ((stream_udp_header_t *)udp_ctrl)->pos,
	   ((stream_udp_header_t *)udp_ctrl)->seq);

    int Seq0 = Seq1;
    for(; Seq1 <= Seq2; Seq1++) {
      stream_rtp_header_impl_t *frame = m_BackLog->Get(Seq1+1);
      if(frame && (ntohull(frame->hdr_ext.pos) - Pos < 100000))
	break;
    }

    sprintf((udp_ctrl+sizeof(stream_udp_header_t)),
	    "UDP MISSING %d-%d %" PRIu64,
	    Seq0, (Seq1 & UDP_BUFFER_MASK), Pos);

    send(fd, udp_ctrl, 64, 0);
  }
}
