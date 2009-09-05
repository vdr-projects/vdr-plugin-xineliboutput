/*
 * frontend.c:
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <vdr/config.h>
#include <vdr/tools.h>
#include <vdr/plugin.h>

#include "logdefs.h"
#include "config.h"
#include "frontend.h"
#include "device.h"

#include "tools/pes.h"
#include "tools/mpeg.h"
#include "tools/h264.h"
#include "tools/general_remote.h"
#include "tools/iso639.h"

//#define LOG_CONTROL_MESSAGES
//#define XINELIBOUTPUT_LOG_KEYS

#ifndef STARTUP_IMAGE_FILE
#  define  STARTUP_IMAGE_FILE "/usr/share/vdr/xineliboutput/logo.mpv"
#endif
#ifndef STARTUP_MAX_SIZE
#  define STARTUP_MAX_SIZE (256*1024)
#endif

//----------------------------- cXinelibThread --------------------------------

//
// keyboard control handler
//

/*static*/
void cXinelibThread::KeypressHandler(const char *keymap, const char *key, 
				     bool repeat, bool release)
{
#ifdef XINELIBOUTPUT_LOG_KEYS
  static FILE *flog = fopen("/video/keys.log","w");
  if (flog) {
    fprintf(flog,"KEY %s %s %d %d\n",keymap,key,repeat,release); fflush(flog);
  }
#endif

  TRACE("keypress_handler: " << (keymap?keymap:"") << " " << key);

  if(!key)
    return;

  if(keymap) {
    cRemote *item = Remotes.First();
    while(item) {
      if(!strcmp(item->Name(), keymap)) {
	// dirty... but only way to support learning ...
	((cGeneralRemote*)item)->Put(key, repeat, release);
	return;
      }
      item = Remotes.Next(item);
    }
    cGeneralRemote *r = new cGeneralRemote(keymap);
    if(*key)
      r->Put(key, repeat, release);
  } else {
    cRemote::Put(cKey::FromString(key));
  }
}

#include <vdr/status.h>
class cFrontendStatusMonitor : public cStatus {
  private:
    bool& m_SpuLangAuto;
  public:
    cFrontendStatusMonitor(bool& SpuLangAuto) : m_SpuLangAuto(SpuLangAuto) {};
    virtual void SetSubtitleTrack(int /*Index*/, const char * const */*Tracks*/) { m_SpuLangAuto = false; }
};

void cXinelibThread::InfoHandler(const char *info)
{
  char *pmap = strdup(info), *map = pmap, *pt;

  if(NULL != (pt=strchr(map, '\r')))
    *pt = 0;

  if(!strncmp(info, "TRACKMAP SPU", 12)) {
    int CurrentTrack = ttXSubtitleAuto;
    map += 12;
    while(*map) {
      bool Current = false;
      while(*map == ' ') map++;
      if(*map == '*') {
	Current = true;
	map++;
	if (*map == '-') {
	  CurrentTrack = atoi(map);
	  while (*map && *map != ' ') map++;
	  continue;
	}
      }
      if(*map >= '0' && *map <= '9') {
	int id = atoi(map);
	while(*map && *map != ':') map++;
	if(*map == ':') map++;
	char *lang = map;
	while(*map && *map != ' ') map++;
	if(*map == ' ') { *map = 0; map++; };
	cXinelibDevice::Instance().SetAvailableTrack(ttSubtitle, id, id+1, iso639_2_to_iso639_1(lang) ?: *cString::sprintf("%03d", id+1));
	if (Current)
	  CurrentTrack = id;
      }
    }
    if (CurrentTrack == ttXSubtitleAuto)
      cXinelibDevice::Instance().EnsureSubtitleTrack();
    else if (CurrentTrack == ttXSubtitleNone)
      cXinelibDevice::Instance().SetCurrentSubtitleTrack(ttNone, true);
    else
      cXinelibDevice::Instance().SetCurrentSubtitleTrack(eTrackType(CurrentTrack+ttSubtitleFirst), true);
  }

  else if(!strncmp(info, "TRACKMAP AUDIO", 14)) {
    map += 14;
    cXinelibDevice::Instance().ClrAvailableTracks();
    while(*map) {
      bool Current = false;
      while(*map == ' ') map++;
      if(*map == '*') {
	Current = true;
	map++;
      }
      int id = atoi(map);
      while(*map && *map != ':') map++;
      if(*map == ':') map++;
      char *lang = map;
      while(*map && *map != ' ') map++;
      if(*map == ' ') { *map = 0; map++; };
      cXinelibDevice::Instance().SetAvailableTrack(ttDolby, id, ttDolby+id, iso639_2_to_iso639_1(lang) ?: cString::sprintf("%03d", id+1));
      if(Current)
	cXinelibDevice::Instance().SetCurrentAudioTrack((eTrackType)(ttDolby+id));
    }
  }

  else if(!strncmp(info, "METAINFO", 8)) {
    map += 8;
    while(*map) {
      while(*map == ' ') map++;
      char *next = strstr(map, "=@");
      if(!next)
	break;
      *next = 0;
      next += 2;
      char *end =  strstr(next, "@");
      if(!end)
	break;
      *end = 0;

      if(!strcmp(map, "title"))
	cXinelibDevice::Instance().SetMetaInfo(miTitle, next);
      if(!strcmp(map, "tracknumber"))
        cXinelibDevice::Instance().SetMetaInfo(miTracknumber, next);
      if(!strcmp(map, "album"))
	cXinelibDevice::Instance().SetMetaInfo(miAlbum, next);
      if(!strcmp(map, "artist"))
	cXinelibDevice::Instance().SetMetaInfo(miArtist, next);
      map = end+1;
    }
  }

  else if(!strncmp(info, "DVDBUTTONS ", 11)) {
    map += 11;
    while(*map == ' ') map++;
    cXinelibDevice::Instance().SetMetaInfo(miDvdButtons, map);
  }

  else if(!strncmp(info, "TITLE ", 6)) {
    map += 6;
    while(*map == ' ') map++;
    cXinelibDevice::Instance().SetMetaInfo(miTitle, map);
  }

  else if(!strncmp(info, "DVDTITLE ", 9)) {
    map += 9;
    while(*map == ' ') map++;
    cXinelibDevice::Instance().SetMetaInfo(miDvdTitleNo, map);
    if (*map == '0')  // DVD Menu, set spu track to 0
      cXinelibDevice::Instance().SetCurrentSubtitleTrack(ttSubtitleFirst);
  }

  else if (!strncmp(info, "WINDOW ", 7)) {
    int w, h;
    map += 7;
    while(*map == ' ') map++;
    if (2 == sscanf(map, "%dx%d", &w, &h)) {
      xc.osd_width_auto  = w;
      xc.osd_height_auto = h;
    }
  }

  free(pmap);
}

cXinelibThread::cXinelibThread(const char *Description) : cThread(Description)
{
  TRACEF("cXinelibThread::cXinelibThread");

  m_Volume = 255;
  m_bReady = false;
  m_bNoVideo = true;
  m_bLiveMode = true; /* can't be replaying when there is no output device */
  m_StreamPos = 0;
  m_LastClearPos = 0;
  m_Frames = 0;
  m_bEndOfStreamReached = false;
  m_bPlayingFile = false;
  m_StatusMonitor = NULL;
}

cXinelibThread::~cXinelibThread()
{
  TRACEF("cXinelibThread::~cXinelibThread");

  Cancel(3);

  if (m_StatusMonitor)
    DELETENULL(m_StatusMonitor);
}

//
// Thread control
//

bool cXinelibThread::IsReady(void)
{
  LOCK_THREAD;
  return m_bReady;
}

//
// Playback control
//

void cXinelibThread::SetVolume(int NewVolume)
{
  m_Volume = NewVolume;
  cString str = cString::sprintf("VOLUME %d%s", NewVolume * 100 / 255, 
				 xc.sw_volume_control ? " SW" : "");
  Xine_Control(str);
}

void cXinelibThread::TrickSpeed(int Speed)
{
  TRACEF("cXinelibThread::TrickSpeed");

  Xine_Control("TRICKSPEED", Speed);
}

void cXinelibThread::SetLiveMode(bool LiveModeOn)
{
  TRACEF("cXinelibThread::SetLiveMode");

  Lock();
  if(m_bLiveMode == LiveModeOn) {
    Unlock();
    return;
  }
  m_bLiveMode = LiveModeOn;
  Unlock();

  Xine_Control("LIVE", m_bLiveMode ? 1 : 0);
}

void cXinelibThread::SetStillMode(bool StillModeOn)
{
  TRACEF("cXinelibThread::SetStillMode");
  Xine_Control("STILL", StillModeOn ? 1 : 0);
}

void cXinelibThread::SetNoVideo(bool bVal)
{
  TRACEF("cXinelibThread::SetNoVideo");

  Lock();
  if(m_bNoVideo == bVal) {
    Unlock();
    return;
  }
  m_bNoVideo = bVal;
  Unlock();

  Xine_Control("NOVIDEO", m_bNoVideo ? 1 : 0);

  char *opts = NULL;
  if(xc.audio_vis_goom_opts[0] && !strcmp(xc.audio_visualization, "goom"))
    opts = xc.audio_vis_goom_opts;

  if(m_bNoVideo && strcmp(xc.audio_visualization, "none")) {
    ConfigurePostprocessing(xc.audio_visualization, true, opts);
  } else {
    ConfigurePostprocessing("AudioVisualization", false, NULL);
  }
}

void cXinelibThread::AudioStreamChanged(bool ac3, int StreamId)
{
  TRACEF("cXinelibThread::AudioStreamChanged");
  if(ac3)
    Xine_Control("AUDIOSTREAM AC3", StreamId);
  else
    Xine_Control("AUDIOSTREAM", StreamId);
}

void cXinelibThread::SetSubtitleTrack(eTrackType Track)
{
  TRACEF("cXinelibThread::SetSubtitleTrack");
  cString buf = cString::sprintf("SPUSTREAM %d%s", 
				 Track==ttNone ? ttXSubtitleNone  : (Track - ttSubtitleFirst), 
				 m_SpuLangAuto ? " auto" : "");
  Xine_Control(buf);
}

void cXinelibThread::Clear(void)
{
  TRACEF("cXinelibThread::Clear");

  char buf[128];

  {
    LOCK_THREAD;

    if (m_StreamPos == m_LastClearPos) {
      //LOGDBG("cXinelibThread::Clear(): double Clear() ignored");
      return;
    }
    m_LastClearPos = m_StreamPos;

    snprintf(buf, sizeof(buf), "DISCARD %" PRId64 " %d", m_StreamPos, m_Frames);
  }

  /* Send to control stream and data stream. If message is sent only to
   * control stream, and it is delayed, engine flush will be skipped.
   */
  Xine_Control(buf);
  Xine_Control_Sync(buf);
}

bool cXinelibThread::Flush(int TimeoutMs) 
{
  TRACEF("cXinelibThread::Flush");

  return Xine_Control("FLUSH", TimeoutMs) <= 0;
}

int cXinelibThread::Poll(cPoller& Poller, int TimeoutMs) 
{
  TRACEF("cXinelibThread::Poll");

  if(!m_bReady) {
    if(TimeoutMs>0)
      cCondWait::SleepMs(TimeoutMs);
    if(!m_bReady)
      return 0;
  }

  int n = Xine_Control("POLL", TimeoutMs);

  return max(n, 0);
}

//
// Data transfer
//

int cXinelibThread::Play_PES(const uchar *data, int len)
{
  Lock();
  m_StreamPos += len;
  m_Frames++;
  /*m_bEndOfStreamReached = false;*/
  Unlock();
  return len;
}

//
// Stream conversions
//

// Convert MPEG1 PES headers to MPEG2 PES headers

int cXinelibThread::Play_Mpeg1_PES(const uchar *data1, int len)
{
  if(!data1[0] && !data1[1] && data1[2] == 0x01 && len>7 && /* header sync bytes */
     ( IS_VIDEO_PACKET(data1) || IS_AUDIO_PACKET(data1)) && /* video / audio / ps1 stream */
     ((data1[6] & 0xC0) != 0x80) &&                         /* really mpeg1 pes */
     (len == ((data1[4]<<8) | data1[5]) + 6)) {             /* whole PES packet and nothing else */
    uchar *data2 = new uchar[len+64];
    int i1=0, i2=0, r=0;
    
    data2[i2++]=data1[i1++];  // 00 (sync)  
    data2[i2++]=data1[i1++];  // 00 (sync)
    data2[i2++]=data1[i1++];  // 01 (sync)
    data2[i2++]=data1[i1++];  // stream ID
    data2[i2++]=data1[i1++];  // len hi
    data2[i2++]=data1[i1++];  // len lo
    
    // skip stuffing
    while ((data1[i1] & 0x80) == 0x80) 
      i1++;

    if ((data1[i1] & 0xc0) == 0x40) {
      // skip STD_buffer_scale, STD_buffer_size
      i1 += 2;
    }
    
    if(len<i1+5) return len;
    
    data2[i2++] = 0x80;
    
    if ((data1[i1] & 0xf0) == 0x20) { 
      /* PTS */
      data2[i2++] = 0x80;
      data2[i2++] = 5;
      data2[i2++] = data1[i1++] & 0x0E;
      data2[i2++] = data1[i1++] & 0xFF;
      data2[i2++] = data1[i1++] & 0xFE;
      data2[i2++] = data1[i1++] & 0xFF;
      data2[i2++] = data1[i1++] & 0xFE;
    }
    else if ((data1[i1] & 0xf0) == 0x30) { 
      /* PTS & DTS */
      data2[i2++] = 0x80|0x40;
      data2[i2++] = 10;
      data2[i2++] = data1[i1++] & 0x0E;
      data2[i2++] = data1[i1++] & 0xFF;
      data2[i2++] = data1[i1++] & 0xFE;
      data2[i2++] = data1[i1++] & 0xFF;
      data2[i2++] = data1[i1++] & 0xFE;
      
      data2[i2++] = data1[i1++] & 0x0E;
      data2[i2++] = data1[i1++] & 0xFF;
      data2[i2++] = data1[i1++] & 0xFE;
      data2[i2++] = data1[i1++] & 0xFF;
      data2[i2++] = data1[i1++] & 0xFE;
    } else {
      i1++;
      data2[i2++] = 0; /* no pts, no dts */
      data2[i2++] = 0; /* header len */
    }
    
    int newlen = ((data1[4]<<8) | data1[5]) + (i2-i1), loops=0;
    data2[4] = ((newlen)&0xff00)>>8;
    data2[5] = ((newlen)&0xff);
    if(len-i1 > 0) {
      memcpy(data2+i2, data1+i1, len-i1);
      cPoller p;
      while(!Poll(p,100) && loops++ < 10) {
	LOGDBG("Play_Mpeg1_PES: poll failed");
      }
      r = Play_PES(data2,newlen+6);
    } 
  
    delete data2;
    return r==newlen+6 ? ((data1[4]<<8)|data1[5])+6 : 0;
  }
  return len; // nothing useful found ...
}

// Pack elementary MPEG stream to PES

bool cXinelibThread::Play_Mpeg2_ES(const uchar *data, int len, int streamID)
{
  static uchar hdr_vid[] = {0x00,0x00,0x01,0xe0, 0x00,0x00,0x80,0x00,0x00}; /* mpeg2 */
  static uchar hdr_pts[] = {0x00,0x00,0x01,0xe0, 0x00,0x08,0x80,0x80,
                            0x05,0x00,0x00,0x00, 0x00,0x00}; /* mpeg2 */
  static uchar seq_end[] = {0x00,0x00,0x01,0xe0, 0x00,0x07,0x80,0x00,
			    0x00,
			    0x00,0x00,0x01,0xB7}; /* mpeg2 */
  int todo = len, done = 0, hdrlen = 9/*sizeof(hdr)*/;
  uchar *frame = new uchar[PES_CHUNK_SIZE+32];
  cPoller p;
  bool h264 = IS_NAL_AUD(data);

  hdr_pts[3] = (uchar)streamID;
  Poll(p, 100);
  Play_PES(hdr_pts, sizeof(hdr_pts));

  hdr_vid[3] = (uchar)streamID;
  while(todo) {
    int blocklen = todo;
    if(blocklen > ((PES_CHUNK_SIZE - hdrlen) & 0xfffc))
      blocklen = (PES_CHUNK_SIZE - hdrlen) & 0xfffc;
    hdr_vid[4] = ((blocklen+3)&0xff00)>>8;
    hdr_vid[5] = ((blocklen+3)&0xff);

    memcpy(frame, hdr_vid, hdrlen);
    memcpy(frame+hdrlen, data+done, blocklen);

    done += blocklen;
    todo -= blocklen;

    Poll(p, 100);

    if(blocklen+hdrlen != Play_PES(frame,blocklen+hdrlen)) {      
      delete frame;
      return false;
    }
  }

  // append sequence end code to video 
  if((streamID & 0xF0) == 0xE0) { 
    seq_end[3] = (uchar)streamID;
    seq_end[12] = h264 ? NAL_END_SEQ : 0xB7;
    Poll(p, 100);
    Play_PES(seq_end, sizeof(seq_end));
  }

  delete[] frame;
  return true;
}

//
// Built-in still images
//

bool cXinelibThread::QueueBlankDisplay(void)
{
  TRACEF("cXinelibThread::BlankDisplay");
  Xine_Control_Sync("BLANK");
  return true;
}

bool cXinelibThread::BlankDisplay(void)
{
  TRACEF("cXinelibThread::BlankDisplay");

  bool r = QueueBlankDisplay();
  for(int i=0; i<5 && !Flush(100); i++)
    ;
  return r;
}

bool cXinelibThread::LogoDisplay(void)
{
  TRACEF("cXinelibThread::LogoDisplay");
  
  cString Path;
  int     fd = -1;

  if(Setup.FileName()) {
    cString SetupPath = Setup.FileName();
    const char *end = strrchr(SetupPath, '/');
    if(end) {
      SetupPath.Truncate(end - (const char *)SetupPath);
      fd = open(Path=cString::sprintf("%s/plugins/xineliboutput/logo.mpv", *SetupPath), O_RDONLY);
    }
  }
  
  if(fd<0)
    fd = open(Path=STARTUP_IMAGE_FILE, O_RDONLY);
  
  if(fd >= 0) {
    uint8_t *data = (uint8_t*)malloc(STARTUP_MAX_SIZE);
    int datalen = read(fd, data, STARTUP_MAX_SIZE);
    if(datalen == STARTUP_MAX_SIZE) {
      LOGMSG("WARNING: custom startup image %s too large", *Path);
    } else if(datalen<=0) {
      LOGERR("error reading custom startup image %s", *Path);
    } else {
      LOGMSG("using custom startup image %s", *Path);
      bool r = Play_Mpeg2_ES(data, datalen, VIDEO_STREAM);
      free(data);
      for(int i=0; i<5 && !Flush(100); i++)
	;
      return r;
    }
    free(data);
    close(fd);
  }
  
  /* use default image */
  extern const unsigned char v_mpg_vdrlogo[];     // vdrlogo_720x576.c
  extern const int v_mpg_vdrlogo_length;

  bool r = Play_Mpeg2_ES(v_mpg_vdrlogo, v_mpg_vdrlogo_length, VIDEO_STREAM);
  for(int i=0; i<5 && !Flush(100); i++)
    ;
  return r;
}

bool cXinelibThread::NoSignalDisplay(void)
{
  TRACEF("cXinelibThread::NoSignalDisplay");

  extern const unsigned char v_mpg_nosignal[];     // nosignal_720x576.c
  extern const int v_mpg_nosignal_length;

  bool r = Play_Mpeg2_ES(v_mpg_nosignal, v_mpg_nosignal_length, VIDEO_STREAM);
  for(int i=0; i<5 && !Flush(100); i++)
    ;
  return r;
}

//
// Xine Control
//

int cXinelibThread::Xine_Control(const char *cmd, int p1)
{
  char buf[128];
  if(snprintf(buf, sizeof(buf), "%s %d", cmd, p1) >= (int)sizeof(buf)) {
    LOGMSG("Xine_Control %s: message too long !", cmd);
    return 0;
  }
  //buf[sizeof(buf)-1] = 0;
  return Xine_Control(buf);
}

int cXinelibThread::Xine_Control(const char *cmd, int64_t p1)
{
  char buf[128];
  if(snprintf(buf, sizeof(buf), "%s %" PRId64, cmd, p1) >= (int)sizeof(buf)) {
    LOGMSG("Xine_Control %s: message too long !", cmd);
    return 0;
  }
  //buf[sizeof(buf)-1] = 0;
  return Xine_Control(buf);
}

int cXinelibThread::Xine_Control(const char *cmd, const char *p1)
{
  char buf[1024];
  if(snprintf(buf, sizeof(buf), "%s %s", cmd, p1) >= (int)sizeof(buf)) {
    LOGMSG("Xine_Control %s: message too long !", cmd);
    return 0;
  }
  //buf[sizeof(buf)-1] = 0;
  return Xine_Control(buf);
}

bool cXinelibThread::PlayFile(const char *FileName, int Position, 
			      bool LoopPlay, ePlayMode PlayMode,
			      int TimeoutMs)
{
  TRACEF("cXinelibThread::PlayFile");

  char vis[256];

  switch(PlayMode) {
    case pmVideoOnly:
      LOGDBG("cXinelibThread::PlayFile: Video from file, audio from VDR");
      strcpy(vis, "Video");
      break;
    case pmAudioOnly:
      LOGDBG("cXinelibThread::PlayFile: Audio from file, video from VDR");
      strcpy(vis, "Audio");
      break;
    case pmAudioOnlyBlack:
      //LOGDBG("cXinelibThread::PlayFile: Audio from file, no video");
      strcpy(vis, "none");
      break;
    case pmAudioVideo:
    default:
      if(xc.audio_vis_goom_opts[0] && !strcmp(xc.audio_visualization, "goom"))
	snprintf(vis, sizeof(vis), "%s:%s", xc.audio_visualization, xc.audio_vis_goom_opts);
      else
	strn0cpy(vis, xc.audio_visualization, sizeof(vis));
      vis[sizeof(vis)-1] = 0;
      break;
  }

  char buf[4096];
  m_bEndOfStreamReached = false;
  if(snprintf(buf, sizeof(buf), "PLAYFILE %s %d %s %s",
	      LoopPlay ? "Loop" : "", Position, vis, FileName ? FileName : "")
     >= 4096) {
    LOGMSG("PlayFile: message too long !");
    return 0;
  }

  if(FileName) {
    Lock();
    m_FileName = FileName;
    m_bPlayingFile = true;
    m_SpuLangAuto = true;
    if (m_StatusMonitor)
      DELETENULL(m_StatusMonitor);
    m_StatusMonitor = new cFrontendStatusMonitor(m_SpuLangAuto);
    Unlock();
  }

  int result = PlayFileCtrl(buf, TimeoutMs);

  if(!FileName || result != 0) {
    Lock();
    m_bPlayingFile = false;
    m_FileName = NULL;
    if (m_StatusMonitor)
      DELETENULL(m_StatusMonitor);
    Unlock();
  } else {
    if(xc.extsub_size >= 0)
      Xine_Control("EXTSUBSIZE", xc.extsub_size);

    // set preferred subtitle language 
    if (Setup.DisplaySubtitles) {
      const char *langs = I18nLanguageCode(Setup.SubtitleLanguages[0]);
      if (langs) {
	char lang1[5];
	strn0cpy(lang1, langs, 4); /* truncate */
	const char *spu_lang = iso639_1_to_iso639_2(lang1);
	LOGMSG("Preferred SPU language: %s (%s)", lang1, spu_lang);
	if (spu_lang && spu_lang[0] && spu_lang[1] && !spu_lang[2])
	  Xine_Control(cString::sprintf("SPUSTREAM %s", spu_lang));
      }
    } else {
      LOGMSG("Preferred SPU language: (none)");
      Xine_Control(cString::sprintf("SPUSTREAM %d", ttXSubtitleNone));
    }
  }

  return Running() && !result;
}


//
// Configuration
//

void cXinelibThread::Configure(void)
{
    ConfigurePostprocessing(xc.deinterlace_method, xc.audio_delay, 
			    xc.audio_compression, xc.audio_equalizer,
			    xc.audio_surround, xc.speaker_type);
    ConfigureVideo(xc.hue, xc.saturation, xc.brightness, xc.sharpness, xc.noise_reduction, xc.contrast, xc.overscan, xc.vo_aspect_ratio);
    ConfigurePostprocessing("upmix",     xc.audio_upmix ? true : false, NULL);
    ConfigurePostprocessing("autocrop",  xc.autocrop  ? true : false, 
			    xc.AutocropOptions());
    ConfigurePostprocessing("swscale", xc.swscale ? true : false, 
			    xc.SwScaleOptions());
    ConfigurePostprocessing("pp", xc.ffmpeg_pp ? true : false, 
			    xc.FfmpegPpOptions());
    ConfigurePostprocessing("unsharp",xc.unsharp ? true : false, 
                            xc.UnsharpOptions());
    ConfigurePostprocessing("denoise3d",xc.denoise3d ? true : false, 
                            xc.Denoise3dOptions());

#ifdef ENABLE_TEST_POSTPLUGINS
    ConfigurePostprocessing("headphone", xc.headphone ? true : false, NULL);
#endif

    Xine_Control(cString::sprintf("SCR %s %d", 
				  xc.live_mode_sync ? "Sync"    : "NoSync",
				  xc.scr_tuning     ? xc.scr_hz : 90000));
}

int cXinelibThread::ConfigurePostprocessing(const char *deinterlace_method, 
					    int audio_delay, 
					    int audio_compression, 
					    const int *audio_equalizer,
					    int audio_surround,
					    int speaker_type) 
{
  char buf[1024];
  int r = true;

  if(strcmp(deinterlace_method, "tvtime")) 
    r = ConfigurePostprocessing("tvtime", false, NULL) && r;

  r = Xine_Control("DEINTERLACE", deinterlace_method) && r;
  r = Xine_Control("AUDIODELAY", audio_delay) && r;
  r = Xine_Control("AUDIOCOMPRESSION", audio_compression) && r;
  r = Xine_Control("AUDIOSURROUND", audio_surround) && r;
  r = Xine_Control("SPEAKERS", speaker_type) && r;
  sprintf(buf,"EQUALIZER %d %d %d %d %d %d %d %d %d %d",
	  audio_equalizer[0],audio_equalizer[1],
	  audio_equalizer[2],audio_equalizer[3],
	  audio_equalizer[4],audio_equalizer[5],
	  audio_equalizer[6],audio_equalizer[7],
	  audio_equalizer[8],audio_equalizer[9]);
  r = Xine_Control(buf) && r;

  if(m_bNoVideo && strcmp(xc.audio_visualization, "none")) {
    char *opts = NULL;
    if(xc.audio_vis_goom_opts[0] && !strcmp(xc.audio_visualization, "goom"))
      opts = xc.audio_vis_goom_opts;
    //fe->post_open(fe, xc.audio_visualization, NULL);
    r = ConfigurePostprocessing(xc.audio_visualization, true, opts) && r;
  } else {
    //fe->post_close(fe, NULL, 0);
    r = ConfigurePostprocessing("AudioVisualization", false, NULL) && r;
  }

  if(!strcmp(deinterlace_method, "tvtime")) 
    r = ConfigurePostprocessing("tvtime", true, xc.deinterlace_opts) && r;

  return r;
}

int cXinelibThread::ConfigurePostprocessing(const char *name, bool on, const char *args)
{
  char buf[1024];  
  int l;

  if(on) 
    l = snprintf(buf, sizeof(buf), "POST %s On %s", (name&&*name)?name:"0", args?args:"");
  else 
    // 0 - audio vis.
    // 1 - audio post
    // 2 - video post
    //return fe->post_close(fe, name, -1);
    l = snprintf(buf, sizeof(buf), "POST %s Off", (name&&*name)?name:"0");

  if(l >= (int)sizeof(buf)) {
    LOGMSG("ConfigurePostprocessing %s: message too long !", name);
    return 0;
  }
  //buf[sizeof(buf)-1] = 0;

  return Xine_Control(buf);
}

int cXinelibThread::ConfigureVideo(int hue, int saturation, 
				   int brightness, int sharpness,
				   int noise_reduction, int contrast,
				   int overscan, int vo_aspect_ratio)
{
  char cmd[128];
  Xine_Control("OVERSCAN", overscan);
  snprintf(cmd, sizeof(cmd),
	   "VIDEO_PROPERTIES %d %d %d %d %d %d %d", 
	  hue, saturation, brightness, sharpness, noise_reduction, contrast, vo_aspect_ratio);
  return Xine_Control(cmd);
}

//
// Playback files
//

bool cXinelibThread::EndOfStreamReached(void) 
{
  LOCK_THREAD;
  bool r = m_bEndOfStreamReached;
  return r;
}


