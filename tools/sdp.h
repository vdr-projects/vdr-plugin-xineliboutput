/*
 * sdp.h: RFC2974 Session Description Protocol (SDP)
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef XINELIBOUTPUT_SDP_H_
#define XINELIBOUTPUT_SDP_H_


#define SDP_MIME_TYPE  "application/sdp"


static char *vdr_sdp_description(const char *vdr_ip, 
				 int vdr_svdrp_port,
				 int vdr_xineliboutput_port,
				 const char *rtp_ip,
				 uint32_t rtp_ssrc,
				 int rtp_port, 
				 int rtp_ttl)
{
  static uint8_t s_serial = 0;
  static char   *s_data = NULL;
  static char    s_hostname[257] = {0};

  uint64_t serial = (time(NULL) << 2) + ((s_serial++) & 0x03);

  if(!s_hostname[0])
    gethostname(s_hostname, 256);

  free(s_data);

  asprintf(&s_data,
	   /*** session ***/
	   /* version    */        "v=0"
	   /* origin     */ "\r\n" "o=%s %u %"PRIu64" IN IP4 %s"
	   /* name       */ "\r\n" "s=%s@%s (multicast %s:%d)"
	   /* opt:info   */ //"\r\n" "i=vdr-xineliboutput primary device output"
	   /* time       */ "\r\n" "t=0 0"

	   /*** data stream(s) ***/
	   /* connection */ "\r\n" "c=IN IP4 %s/%d"
	   /*            */ "\r\n" "a=recvonly"
	   /*            */ "\r\n" "a=type:broadcast"
	   /*            */ "\r\n" "a=x-plgroup:vdr"
	   /* media      */ "\r\n" "m=video %d RTP/AVP 96"
	   /*            */ "\r\n" "a=rtpmap:96 MP2P/90000"
	   /* media      */ /*"\r\n" "m=video %d udp MP2P"*/
	   /*            */ /*"\r\n" "a=mux:ps"*/
	   /*            */ /*"\r\n" "a=packetformat:RAW"*/
#if 0
	   /*** rtsp control port ***/
	   /* connection */ "\r\n" "c=IN IP4 %s"
	   /* media      */ "\r\n" "m=control %d tcp/http rtsp"
#endif
	   /*** xineliboutput control port ***/
	   /* connection */ "\r\n" "c=IN IP4 %s"
	   /* media      */ "\r\n" "m=control %d tcp x-vdr-xineliboutput"

	   /*** SVDRP control port ***/
	   /* connection */ "\r\n" "c=IN IP4 %s"
	   /* media      */ "\r\n" "m=control %d tcp x-svdrp"

	   /* origin */
	   , "vdr", rtp_ssrc, serial, vdr_ip

	   /* name */ 
	   , "vdr", s_hostname, rtp_ip, rtp_port

	   /* video/mp2p udp/rtp */
	   , rtp_ip, rtp_ttl
	   , rtp_port
#if 0
	   /* tcp/http control/rtsp */
	   , vdr_ip
	   , vdr_xineliboutput_port
#endif
	   /* tcp control/x-vdr-xineliboutput */
	   , vdr_ip
	   , vdr_xineliboutput_port

	   /* tcp control/x-svdrp */
	   , vdr_ip
	   , vdr_svdrp_port
	   );

  return s_data;
}


#endif  /* XINELIBOUTPUT_SDP_H_ */
