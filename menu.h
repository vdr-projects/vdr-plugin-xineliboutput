/*
 * menu.h: Main Menu
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef __XINELIB_MENU_H
#define __XINELIB_MENU_H

#include <vdr/menuitems.h>

#include "config.h" // eMainMenuMode

class cXinelibDevice;

class cMenuXinelib : public cMenuSetupPage
{
  private:
    cXinelibDevice *m_Dev;

    int compression;
    int autocrop;
    int overscan;
    int novideo;

    // Hotkeys
    enum { hkInit, hkSeen, hkNone } hotkey_state;

    virtual eOSState ProcessHotkey(eKeys Key);

    cOsdItem *audio_ctrl_compress;

    cOsdItem *ctrl_autocrop;
    cOsdItem *ctrl_overscan;
    cOsdItem *ctrl_novideo;

  protected:
    virtual void Store(void);

  public:
    cMenuXinelib(cXinelibDevice *Dev);
    virtual ~cMenuXinelib();
    virtual eOSState ProcessKey(eKeys Key);

    static cOsdMenu *CreateMenuBrowseFiles(cXinelibDevice *Dev, eMainMenuMode mode, bool Queue=true);
};

#endif //__XINELIB_SETUP_MENU_H
