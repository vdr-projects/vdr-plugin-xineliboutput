/*
 * playlist.h: Media player playlist
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef __XINELIBOUTPUT_PLAYLIST_H
#define __XINELIBOUTPUT_PLAYLIST_H

#include <vdr/tools.h>  // cString, cListObject, cList<>
#include <vdr/thread.h> // cMutex


//
// cPlaylistItem
//
 
class cPlaylistItem : public cListObject 
{
 private:
  cPlaylistItem();

  virtual int Compare(const cListObject &ListObject) const;

 public:
  cPlaylistItem(const char *filename); /* file name with full path */
  cPlaylistItem(const char *filename,  /* file name without path */
		const char *path,
		const char *title = NULL, 
		int position = -1);
  
  cString Filename; /* file name and full path */

  // Metaingo (ID3 etc.)
  cString Track;
  cString Artist;
  cString Album;
    
  // position in playlist (if given in playlist file)
  int Position;
};


//
// cPlaylistChangeNotify interface
//

class cPlaylistChangeNotify 
{
 public:
  virtual void PlaylistChanged(const cPlaylistItem *Item) = 0;

  virtual ~cPlaylistChangeNotify() {}
};


//
// cPlaylist
//


class cID3Scanner;

class cPlaylist : protected cList<cPlaylistItem> 
{
 private:

  cMutex         m_Lock;
  cString        m_Name;    // playlist (or folder) name
  cString        m_Folder;  // path to "root" of playlist
  cPlaylistItem *m_Current; // now playing

  enum { ePlaylist, eImplicit } m_Origin;

  cPlaylistChangeNotify *m_Menu;
  cID3Scanner *m_Scanner;
 
 protected:

  bool StoreCache(void);
  bool ReadCache(void);

  int ReadPlaylist(const char *PlaylistFile);
  int ScanFolder(const char *FolderName, 
		 bool Recursive = false,
		 bool (config_t::*Filter)(const char *) = &config_t::IsAudioFile);

  friend class cID3Scanner;
  friend class cPlaylistReader;
  void PlaylistChanged(const cPlaylistItem *Item);
  cPlaylistItem *Last(void) { return cList<cPlaylistItem>::Last(); }

 public:

  cPlaylist();
  virtual ~cPlaylist();

  const cString& Name(void) const { return m_Name; }

  // listen for changes in playlist
  void Listen(cPlaylistChangeNotify *Menu = NULL);

  // read playlist from file or create playlist from directory tree 
  bool Read(const char *PlaylistFile, bool Recursive = false);
  void StartScanner(void);
  void Sort(void);
  int  Count(void) const;

  // access/iterate playlist items
  cPlaylistItem *First(void) { return Next(NULL); }
  cPlaylistItem *Next(const cPlaylistItem *i);

  // get/set current (now playing) item
  cPlaylistItem *Current(void);
  void           SetCurrent(cPlaylistItem *current);
  cPlaylistItem *Next(void);
  cPlaylistItem *Prev(void);
};


#endif // __XINELIBOUTPUT_PLAYLIST_H
