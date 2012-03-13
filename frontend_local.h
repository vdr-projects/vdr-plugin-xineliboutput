/*
 * frontend_local.h:
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef __XINELIB_FRONTEND_LOCAL_H
#define __XINELIB_FRONTEND_LOCAL_H

#include "frontend.h"

//----------------------------- cXinelibLocal --------------------------------

extern "C" {
  typedef struct frontend_s frontend_t;
}

class cXinelibLocal : public cXinelibThread
{

  public:
    cXinelibLocal(const char *frontend_name);
    virtual ~cXinelibLocal();

  protected:
    virtual void Action(void);

  public:

    // Data transfer
    virtual int     Play(const uchar *buf, int len, eStreamId StreamId = sidVdr);
    virtual void    OsdCmd(void *cmd);
    virtual int64_t GetSTC();

    // Playback files
    virtual bool EndOfStreamReached(void);

    // Image grabbing
    virtual uchar *GrabImage(int &Size, bool Jpeg, int Quality,
			     int SizeX, int SizeY);

    // Configuration
    virtual void ConfigureWindow(int fullscreen, int width, int height,
				 int modeswitch, const char *modeline,
				 int aspect, int scale_video);
    virtual void ConfigureDecoder(int pes_buffers);

    virtual int  SupportsTrueColorOSD(void);

  protected:

    // Playback control
    virtual int  Xine_Control(const char *cmd);

  protected:

    // Frontend access
    frontend_t *load_frontend(const char *fe_name);

    // Data
    void       *h_fe_lib;
    frontend_t *fe;
    cRwLock     m_feLock;
    bool        m_bReconfigRequest;
};


#endif // __XINELIB_FRONTEND_LOCAL_H
