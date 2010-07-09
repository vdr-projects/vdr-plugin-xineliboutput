/*
 * Copyright (C) 2000-2005 the xine project
 *
 * Copyright (C) 2009 Petri Hintukainen <phintuka@users.sourceforge.net>
 *
 * This file is part of xine, a free video player.
 *
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 * Input plugin for BluRay discs / images
 *
 * Requires libbluray from git://git.videolan.org/libbluray.git
 * Tested with revision 2010-07-09
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* asprintf: */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <libbluray/bluray.h>

#define LOG_MODULE "input_bluray"
#define LOG_VERBOSE

#define LOG

#define LOGMSG(x...)  xine_log (this->stream->xine, XINE_LOG_MSG, "input_bluray: " x);

#define XINE_ENGINE_INTERNAL

#ifdef HAVE_CONFIG_H
# include "xine_internal.h"
# include "input_plugin.h"
#else
# include <xine/xine_internal.h>
# include <xine/input_plugin.h>
#endif

#ifndef EXPORTED
#  define EXPORTED __attribute__((visibility("default")))
#endif

#ifndef MIN
# define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
# define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define ALIGNED_UNIT_SIZE 6144
#define PKT_SIZE          192
#define TICKS_IN_MS       45

typedef struct {

  input_class_t   input_class;

  xine_t         *xine;

  xine_mrl_t    **xine_playlist;
  int             xine_playlist_size;

  /* config */
  char           *mountpoint;
  char           *keyfile;
  char           *device;

} bluray_input_class_t;

typedef struct {
  input_plugin_t        input_plugin;

  xine_stream_t        *stream;
  xine_event_queue_t   *event_queue;

  bluray_input_class_t *class;

  char                 *mrl;
  char                 *disc_root;
  char                 *disc_name;

  BLURAY               *bdh;

  int                num_titles;
  int                current_title;
  BLURAY_TITLE_INFO *title_info;
  int                current_clip;

  uint32_t       cap_seekable;

} bluray_input_plugin_t;

static int get_current_chapter(bluray_input_plugin_t *this)
{
  uint64_t offset = bd_tell(this->bdh);
  int      chapter;

  /* search for next chapter mark */
  for (chapter = 0; chapter < this->title_info->chapter_count; chapter++) {
    if (this->title_info->chapters[chapter].offset > offset)
      break;
  }

  return MAX(0, chapter - 1);
}

static void update_stream_info(bluray_input_plugin_t *this)
{
  /* set stream info */

  _x_stream_info_set(this->stream, XINE_STREAM_INFO_DVD_ANGLE_COUNT,    this->title_info->angle_count);
  _x_stream_info_set(this->stream, XINE_STREAM_INFO_DVD_ANGLE_NUMBER,   bd_get_current_angle(this->bdh));
  _x_stream_info_set(this->stream, XINE_STREAM_INFO_HAS_CHAPTERS,       this->title_info->chapter_count > 0);
  _x_stream_info_set(this->stream, XINE_STREAM_INFO_DVD_CHAPTER_COUNT,  this->title_info->chapter_count);
  _x_stream_info_set(this->stream, XINE_STREAM_INFO_DVD_CHAPTER_NUMBER, get_current_chapter(this) + 1);
}

static int open_title (bluray_input_plugin_t *this, int title)
{
  if (bd_select_title(this->bdh, title) <= 0) {
    LOGMSG("bd_select_title(%d) failed\n", title);
    return 0;
  }

  this->current_title = title;

  if (this->title_info)
    bd_free_title_info(this->title_info);
  this->title_info = bd_get_title_info(this->bdh, this->current_title);

#ifdef LOG
  int ms = this->title_info->duration / INT64_C(90000);
  lprintf("Opened playlist %d. Length %"PRId64" bytes / %02d:%02d:%02d.%03d\n",
          title, bd_get_title_size(this->bdh),
          ms / 3600000, (ms % 3600000 / 60000), (ms % 60000) / 1000, ms % 1000);
#endif

  /* set title */

  xine_ui_data_t udata;
  xine_event_t uevent = {
    .type = XINE_EVENT_UI_SET_TITLE,
    .stream = this->stream,
    .data = &udata,
    .data_length = sizeof(udata)
  };
  if (this->disc_name && this->disc_name[0])
    udata.str_len = snprintf(udata.str, sizeof(udata.str), "%s, Title %d/%d", this->disc_name, title, this->num_titles);
  else
    udata.str_len = snprintf(udata.str, sizeof(udata.str), "Title %d/%d", title, this->num_titles);
  xine_event_send(this->stream, &uevent);

  _x_meta_info_set(this->stream, XINE_META_INFO_TITLE, udata.str);

  /* calculate and set stream rate */

  uint64_t rate = bd_get_title_size(this->bdh) * UINT64_C(8) // bits
                  * INT64_C(90000)
                  / (uint64_t)(this->title_info->duration);
  _x_stream_info_set(this->stream, XINE_STREAM_INFO_BITRATE, rate);

#ifdef LOG
  int krate = (int)(rate / UINT64_C(1024));
  int s = this->title_info->duration / 90000;
  int h = s / 3600; s -= h*3600;
  int m = s / 60;   s -= m*60;
  int f = this->title_info->duration % 90000; f = f * 1000 / 90000;
  LOGMSG("BluRay stream: length:  %d:%d:%d.%03d\n"
         "               bitrate: %d.%03d Mbps\n\n",
         h, m, s, f, krate/1024, (krate%1024)*1000/1024);
#endif

  /* set stream info */

  _x_stream_info_set(this->stream, XINE_STREAM_INFO_DVD_TITLE_COUNT,  this->num_titles);
  _x_stream_info_set(this->stream, XINE_STREAM_INFO_DVD_TITLE_NUMBER, this->current_title);

  update_stream_info(this);

  return 1;
}

static void stream_reset(bluray_input_plugin_t *this)
{
  this->cap_seekable = 0;

  xine_set_param(this->stream, XINE_PARAM_FINE_SPEED, XINE_FINE_SPEED_NORMAL);
  this->stream->demux_plugin->seek(this->stream->demux_plugin, 0, 0, 1);
  _x_demux_control_start(this->stream);

  this->cap_seekable = INPUT_CAP_SEEKABLE;
}

static void handle_events(bluray_input_plugin_t *this)
{
  if (!this->event_queue)
    return;

  xine_event_t *event;
  while (NULL != (event = xine_event_get(this->event_queue))) {

    if (!this->bdh || !this->title_info) {
      xine_event_free(event);
      return;
    }

    switch (event->type) {

      case XINE_EVENT_INPUT_LEFT:
        lprintf("XINE_EVENT_INPUT_LEFT: next title\n");
        open_title(this, MAX(0, this->current_title-1));
        stream_reset(this);
        break;

      case XINE_EVENT_INPUT_RIGHT:
        lprintf("XINE_EVENT_INPUT_RIGHT: previous title\n");
        open_title(this, MIN(this->num_titles, this->current_title+1));
        stream_reset(this);
        break;

      case XINE_EVENT_INPUT_NEXT: {
        int chapter = get_current_chapter(this) + 1;

        lprintf("XINE_EVENT_INPUT_NEXT: next chapter\n");

        if (chapter >= this->title_info->chapter_count) {
          if (this->current_title < this->num_titles - 1) {
            open_title(this, this->current_title + 1);
            stream_reset(this);
          }
        } else {
          bd_seek_chapter(this->bdh, chapter);
          update_stream_info(this);
          stream_reset(this);
        }
        break;
      }

      case XINE_EVENT_INPUT_PREVIOUS: {
        int chapter = get_current_chapter(this) - 1;

        lprintf("XINE_EVENT_INPUT_PREVIOUS: previous chapter\n");

        if (chapter < 0 && this->current_title > 0) {
          open_title(this, this->current_title - 1);
          stream_reset(this);
        } else {
          chapter = MAX(0, chapter);
          bd_seek_chapter(this->bdh, chapter);
          update_stream_info(this);
          stream_reset(this);
        }
        break;
      }

      case XINE_EVENT_INPUT_ANGLE_NEXT: {
        unsigned curr_angle = bd_get_current_angle(this->bdh);
        unsigned angle      = MIN(8, curr_angle + 1);
        lprintf("XINE_EVENT_INPUT_ANGLE_NEXT: set angle %d --> %d\n", curr_angle, angle);
        bd_seamless_angle_change(this->bdh, angle);
        _x_stream_info_set(this->stream, XINE_STREAM_INFO_DVD_ANGLE_NUMBER, bd_get_current_angle(this->bdh));
        break;
      }

      case XINE_EVENT_INPUT_ANGLE_PREVIOUS: {
        unsigned curr_angle = bd_get_current_angle(this->bdh);
        unsigned angle      = curr_angle ? curr_angle - 1 : 0;
        lprintf("XINE_EVENT_INPUT_ANGLE_PREVIOUS: set angle %d --> %d\n", curr_angle, angle);
        bd_seamless_angle_change(this->bdh, angle);
        _x_stream_info_set(this->stream, XINE_STREAM_INFO_DVD_ANGLE_NUMBER, bd_get_current_angle(this->bdh));
        break;
      }
    }

    xine_event_free(event);
  }
}

/*
 * xine plugin interface
 */

static uint32_t bluray_plugin_get_capabilities (input_plugin_t *this_gen)
{
  bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;
  return this->cap_seekable  |
         INPUT_CAP_BLOCK     |
         INPUT_CAP_AUDIOLANG |
         INPUT_CAP_SPULANG   |
         INPUT_CAP_CHAPTERS;
}

static off_t bluray_plugin_read (input_plugin_t *this_gen, char *buf, off_t len)
{
  bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;

  if (!this || !this->bdh || len < 0)
    return -1;

  handle_events(this);

  off_t result = bd_read (this->bdh, (unsigned char *)buf, len);

  if (result < 0)
    LOGMSG("bd_read() failed: %s (%d of %d)\n", strerror(errno), (int)result, (int)len);

#if 0
  if (buf[4] != 0x47) {
    LOGMSG("bd_read(): invalid data ? [%02x %02x %02x %02x %02x ...]\n",
          buf[0], buf[1], buf[2], buf[3], buf[4]);
    return 0;
  }
#endif

  return result;
}

static buf_element_t *bluray_plugin_read_block (input_plugin_t *this_gen, fifo_buffer_t *fifo, off_t todo)
{
  buf_element_t *buf = fifo->buffer_pool_alloc (fifo);

  if (todo > (off_t)buf->max_size)
    todo = buf->max_size;

  if (todo > ALIGNED_UNIT_SIZE)
    todo = ALIGNED_UNIT_SIZE;

  if (todo > 0) {
    bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;

    buf->size = bluray_plugin_read(this_gen, (char*)buf->mem, todo);
    buf->type = BUF_DEMUX_BLOCK;

    if (buf->size > 0) {
      buf->extra_info->input_time = 0;
      buf->extra_info->total_time = this->title_info->duration / 90000;
      return buf;
    }
  }

  buf->free_buffer (buf);
  return NULL;
}

static off_t bluray_plugin_seek (input_plugin_t *this_gen, off_t offset, int origin)
{
  bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;

  if (!this || !this->bdh)
    return -1;

  /* convert relative seeks to absolute */

  if (origin == SEEK_CUR) {
    offset = bd_tell(this->bdh) + offset;
  }
  else if (origin == SEEK_END) {
    if (offset < bd_get_title_size(this->bdh))
      offset = bd_get_title_size(this->bdh) - offset;
    else
      offset = 0;
  }

  lprintf("bluray_plugin_seek() seeking to %lld\n", (long long)offset);

  return bd_seek (this->bdh, offset);
}

static off_t bluray_plugin_seek_time (input_plugin_t *this_gen, int time_offset, int origin)
{
  bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;

  if (!this || !this->bdh || !this->title_info)
    return -1;

  /* convert relative seeks to absolute */

  if (origin == SEEK_CUR) {
    time_offset += this_gen->get_current_time(this_gen);
  }
  else if (origin == SEEK_END) {
    int duration = this->title_info->duration / 90;
    if (time_offset < duration)
      time_offset = duration - time_offset;
    else
      time_offset = 0;
  }

  lprintf("bluray_plugin_seek_time() seeking to %d.%03ds\n", time_offset / 1000, time_offset % 1000);

  return bd_seek_time(this->bdh, time_offset * INT64_C(90));
}

static off_t bluray_plugin_get_current_pos (input_plugin_t *this_gen)
{
  bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;

  return this->bdh ? bd_tell(this->bdh) : 0;
}

static int bluray_plugin_get_current_time (input_plugin_t *this_gen)
{
  bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;

  return this->bdh ? bd_tell_time(this->bdh) / 90 : -1;
}

static off_t bluray_plugin_get_length (input_plugin_t *this_gen)
{
  bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;

  return this->bdh ? bd_get_title_size(this->bdh) : -1;
}

static uint32_t bluray_plugin_get_blocksize (input_plugin_t *this_gen)
{
  return ALIGNED_UNIT_SIZE;
}

static const char* bluray_plugin_get_mrl (input_plugin_t *this_gen)
{
  bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;

  return this->mrl;
}

static int bluray_plugin_get_optional_data (input_plugin_t *this_gen, void *data, int data_type)
{
  bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;

  if (!this || !this->stream || !data)
    return INPUT_OPTIONAL_UNSUPPORTED;

  switch (data_type) {

    case INPUT_OPTIONAL_DATA_DEMUXER:
#ifdef HAVE_CONFIG_H
      *(const char **)data = "mpeg-ts";
#else
      *(const char **)data = "mpeg-ts-hdmv";
#endif
      return INPUT_OPTIONAL_SUCCESS;

    /*
     * audio track language:
     * - channel number can be mpeg-ts PID (0x1100 ... 0x11ff)
     */
    case INPUT_OPTIONAL_DATA_AUDIOLANG:
      if (this->title_info) {
        int               channel = *((int *)data);
        BLURAY_CLIP_INFO *clip    = &this->title_info->clips[this->current_clip];

        if (channel < clip->audio_stream_count) {
          memcpy(data, clip->audio_streams[channel].lang, 4);
          lprintf("INPUT_OPTIONAL_DATA_AUDIOLANG: %02d [pid 0x%04x]: %s\n",
                  channel, clip->audio_streams[channel].pid, clip->audio_streams[channel].lang);

          return INPUT_OPTIONAL_SUCCESS;
        }
        /* search by pid */
        int i;
        for (i = 0; i < clip->audio_stream_count; i++) {
          if (channel == clip->audio_streams[i].pid) {
            memcpy(data, clip->audio_streams[i].lang, 4);
            lprintf("INPUT_OPTIONAL_DATA_AUDIOLANG: pid 0x%04x -> ch %d: %s\n",
                    channel, i, clip->audio_streams[i].lang);

            return INPUT_OPTIONAL_SUCCESS;
          }
        }
      }
      return INPUT_OPTIONAL_UNSUPPORTED;

    /*
     * SPU track language:
     * - channel number can be mpeg-ts PID (0x1200 ... 0x12ff)
     */
    case INPUT_OPTIONAL_DATA_SPULANG:
      if (this->title_info) {
        int               channel = *((int *)data);
        BLURAY_CLIP_INFO *clip    = &this->title_info->clips[this->current_clip];

        if (channel < clip->pg_stream_count) {
          memcpy(data, clip->pg_streams[channel].lang, 4);
          lprintf("INPUT_OPTIONAL_DATA_SPULANG: %02d [pid 0x%04x]: %s\n",
                  channel, clip->pg_streams[channel].pid, clip->pg_streams[channel].lang);

          return INPUT_OPTIONAL_SUCCESS;
        }
        /* search by pid */
        int i;
        for (i = 0; i < clip->pg_stream_count; i++) {
          if (channel == clip->pg_streams[i].pid) {
            memcpy(data, clip->pg_streams[i].lang, 4);
            lprintf("INPUT_OPTIONAL_DATA_SPULANG: pid 0x%04x -> ch %d: %s\n",
                    channel, i, clip->pg_streams[i].lang);

            return INPUT_OPTIONAL_SUCCESS;
          }
        }
      }
      return INPUT_OPTIONAL_UNSUPPORTED;

    default:
      return DEMUX_OPTIONAL_UNSUPPORTED;
    }

  return INPUT_OPTIONAL_UNSUPPORTED;
}

static void bluray_plugin_dispose (input_plugin_t *this_gen)
{
  bluray_input_plugin_t *this = (bluray_input_plugin_t *) this_gen;

  if (this->event_queue)
    xine_event_dispose_queue(this->event_queue);

  if (this->title_info)
    bd_free_title_info(this->title_info);

  if (this->bdh)
    bd_close(this->bdh);

  free (this->mrl);
  free (this->disc_root);
  free (this->disc_name);

  free (this);
}

static int parse_mrl(const char *mrl_in, char **path, int *title, int *chapter)
{
  if (strncasecmp (mrl_in, "bluray:", 7))
    return -1;

  char *mrl = strdup(mrl_in);

  /* title[.chapter] given ? parse and drop it */
  if (mrl[strlen(mrl)-1] != '/') {
    char *end = strrchr(mrl, '/');
    if (end && end[1]) {
      if (sscanf(end, "/%d.%d", title, chapter) < 1)
        *title = -1;
      else
        *end = 0;
    }
  }
  lprintf(" -> title %d, chapter %d, mrl \'%s\'\n", *title, *chapter, mrl);

  if (!strcasecmp (mrl, "bluray:") ||
      !strcasecmp (mrl, "bluray:/") ||
      !strcasecmp (mrl, "bluray://") ||
      !strcasecmp (mrl, "bluray:///")) {

    /* default device */
    *path = NULL;
 
  } else if (!strncasecmp (mrl, "bluray:/", 8)) {

    /* strip extra slashes */
    char *start = mrl + 7;
    while (start[0] == '/' && start[1] == '/')
      start++;

    *path = strdup(start);

    _x_mrl_unescape(*path);

    lprintf("non-defaut mount point \'%s\'\n", *path);

  } else {
    lprintf("invalid mrl \'%s\'\n", mrl);
    free(mrl);
    return 0;
  }

  free(mrl);

  return 1;
}

static int bluray_plugin_open (input_plugin_t *this_gen)
{
  bluray_input_plugin_t *this    = (bluray_input_plugin_t *) this_gen;
  int                    title   = -1;
  int                    chapter = 0;

  lprintf("bluray_plugin_open\n");

  /* validate and parse mrl */

  if (!parse_mrl(this->mrl, &this->disc_root, &title, &chapter))
    return -1;

  if (!this->disc_root)
    this->disc_root = strdup(this->class->mountpoint);

  /* open libbluray */

  /* replace ~/ in keyfile path */
  char *keyfile = NULL;
  if (this->class->keyfile && !strncmp(this->class->keyfile, "~/", 2))
    if (asprintf(&keyfile, "%s/%s", xine_get_homedir(), this->class->keyfile + 2) < 0)
      keyfile = NULL;
  /* open */
  if (! (this->bdh = bd_open (this->disc_root, keyfile ?: this->class->keyfile))) {
    LOGMSG("bd_open(\'%s\') failed: %s\n", this->disc_root, strerror(errno));
    free(keyfile);
    return -1;
  }
  free(keyfile);
  lprintf("bd_open(\'%s\') OK\n", this->disc_root);

  /* load title list */

  this->num_titles = bd_get_titles(this->bdh, TITLES_RELEVANT);
  LOGMSG("%d titles\n", this->num_titles);

  if (this->num_titles < 1)
    return -1;

  /* select title */

  /* if title was not in mrl, find the main title */
  if (title < 0) {
    int i, duration = 0;
    for (i = 0; i < this->num_titles; i++) {
      BLURAY_TITLE_INFO *info = bd_get_title_info(this->bdh, i);
      if (info->duration > duration) {
        duration = info->duration;
        title = i;
      }
      bd_free_title_info(info);
    }
  }

  /* get disc name */

  if (strcmp(this->disc_root, this->class->mountpoint)) {
    char *t = strrchr(this->disc_root, '/');
    if (!t[1])
      while (t > this->disc_root && t[-1] != '/') t--;
    else
      while (t[0] == '/') t++;
    this->disc_name = strdup(t);
    char *end = this->disc_name + strlen(this->disc_name) - 1;
    if (*end ==  '/')
      *end = 0;
  }

  /* open */

  if (open_title(this, title) <= 0 &&
      open_title(this, 0) <= 0)
    return -1;

  /* jump to chapter */

  if (chapter > 0) {
    chapter = MAX(0, MIN(this->title_info->chapter_count, chapter) - 1);
    bd_seek_chapter(this->bdh, chapter);
    _x_stream_info_set(this->stream, XINE_STREAM_INFO_DVD_CHAPTER_NUMBER, chapter + 1);
  }

  return 1;
}

static input_plugin_t *bluray_class_get_instance (input_class_t *cls_gen, xine_stream_t *stream,
                                                  const char *mrl)
{
  bluray_input_plugin_t *this;

  lprintf("bluray_class_get_instance\n");

  if (strncasecmp (mrl, "bluray:", 7))
    return NULL;

  this = (bluray_input_plugin_t *) calloc(1, sizeof (bluray_input_plugin_t));

  this->stream = stream;
  this->class  = (bluray_input_class_t*)cls_gen;
  this->mrl    = strdup(mrl);

  this->cap_seekable = INPUT_CAP_SEEKABLE;

  this->input_plugin.open               = bluray_plugin_open;
  this->input_plugin.get_capabilities   = bluray_plugin_get_capabilities;
  this->input_plugin.read               = bluray_plugin_read;
  this->input_plugin.read_block         = bluray_plugin_read_block;
  this->input_plugin.seek               = bluray_plugin_seek;
  this->input_plugin.seek_time          = bluray_plugin_seek_time;
  this->input_plugin.get_current_pos    = bluray_plugin_get_current_pos;
  this->input_plugin.get_current_time   = bluray_plugin_get_current_time;
  this->input_plugin.get_length         = bluray_plugin_get_length;
  this->input_plugin.get_blocksize      = bluray_plugin_get_blocksize;
  this->input_plugin.get_mrl            = bluray_plugin_get_mrl;
  this->input_plugin.get_optional_data  = bluray_plugin_get_optional_data;
  this->input_plugin.dispose            = bluray_plugin_dispose;
  this->input_plugin.input_class        = cls_gen;

  this->event_queue = xine_event_new_queue (this->stream);

  return &this->input_plugin;
}

/*
 * plugin class
 */

static void mountpoint_change_cb(void *data, xine_cfg_entry_t *cfg)
{
  bluray_input_class_t *this = (bluray_input_class_t *) data;

  this->mountpoint = cfg->str_value;
}

static void device_change_cb(void *data, xine_cfg_entry_t *cfg)
{
  bluray_input_class_t *this = (bluray_input_class_t *) data;

  this->device = cfg->str_value;
}

static void keyfile_change_cb(void *data, xine_cfg_entry_t *cfg)
{
  bluray_input_class_t *this = (bluray_input_class_t *) data;

  this->keyfile = cfg->str_value;
}

static void free_xine_playlist(bluray_input_class_t *this)
{
  if (this->xine_playlist) {
    int i;
    for (i = 0; i < this->xine_playlist_size; i++) {
      MRL_ZERO(this->xine_playlist[i]);
      free(this->xine_playlist[i]);
    }
    free(this->xine_playlist);
    this->xine_playlist = NULL;
  }

  this->xine_playlist_size = 0;
}

static const char *bluray_class_get_description (input_class_t *this_gen)
{
  return _("BluRay input plugin");
}

static const char *bluray_class_get_identifier (input_class_t *this_gen)
{
  return "bluray";
}

static char **bluray_class_get_autoplay_list (input_class_t *this_gen, int *num_files)
{
  static char *autoplay_list[] = { "bluray:/", NULL };

  *num_files = 1;

  return autoplay_list;
}

xine_mrl_t **bluray_class_get_dir(input_class_t *this_gen, const char *filename, int *nFiles)
{
  bluray_input_class_t *this = (bluray_input_class_t*) this_gen;
  char *path = NULL;
  int title = -1, chapter = -1, i, num_titles;

  lprintf("bluray_class_get_dir(%s)\n", filename);

  free_xine_playlist(this);

  if (filename)
    parse_mrl(filename, &path, &title, &chapter);

  BLURAY *bdh    = bd_open(path?:this->mountpoint, NULL);

  if (bdh) {
    num_titles = bd_get_titles(bdh, TITLES_RELEVANT);

    if (num_titles > 0) {

      this->xine_playlist_size = num_titles;
      this->xine_playlist      = calloc(this->xine_playlist_size + 1, sizeof(xine_mrl_t*));

      for (i = 0; i < num_titles; i++) {
        //BLURAY_TITLE_INFO *info = bd_get_title_info(bd, title);

        this->xine_playlist[i] = calloc(1, sizeof(xine_mrl_t));

        if (asprintf(&this->xine_playlist[i]->origin, "bluray:/%s", path?:"") < 0)
          this->xine_playlist[i]->origin = NULL;
        if (asprintf(&this->xine_playlist[i]->mrl,    "bluray:/%s/%d", path?:"", i) < 0)
          this->xine_playlist[i]->mrl = NULL;
        this->xine_playlist[i]->type = mrl_dvd;
        //this->xine_playlist[i]->size = info->duration;

        //bd_free_title_info(info);
      }
    }
  }

  bd_close(bdh);

  free(path);

  if (nFiles)
    *nFiles = this->xine_playlist_size;

  return this->xine_playlist;
}

static int bluray_class_eject_media (input_class_t *this_gen)
{
#if 0
  bluray_input_class_t *this = (bluray_input_class_t*) this_gen;

  return media_eject_media (this->xine, this->device);
#endif
  return 1;
}

static void bluray_class_dispose (input_class_t *this_gen)
{
  bluray_input_class_t *this   = (bluray_input_class_t *) this_gen;
  config_values_t      *config = this->xine->config;

  free_xine_playlist(this);

  config->unregister_callback(config, "media.bluray.mountpoint");
  config->unregister_callback(config, "media.bluray.device");
  config->unregister_callback(config, "media.bluray.keyfile");

  free (this);
}

static void *bluray_init_plugin (xine_t *xine, void *data)
{
  config_values_t      *config = xine->config;
  bluray_input_class_t *this   = (bluray_input_class_t *) calloc(1, sizeof (bluray_input_class_t));

  this->xine = xine;

  this->input_class.get_instance       = bluray_class_get_instance;
  this->input_class.get_identifier     = bluray_class_get_identifier;
  this->input_class.get_description    = bluray_class_get_description;
  this->input_class.get_dir            = bluray_class_get_dir;
  this->input_class.get_autoplay_list  = bluray_class_get_autoplay_list;
  this->input_class.dispose            = bluray_class_dispose;
  this->input_class.eject_media        = bluray_class_eject_media;

  this->mountpoint = config->register_filename(config, "media.bluray.mountpoint",
                                               "/mnt/bluray", XINE_CONFIG_STRING_IS_DIRECTORY_NAME,
                                               _("BluRay mount point"),
                                               _("Default mount location for BluRay discs."),
                                               0, mountpoint_change_cb, (void *) this);
  this->device = config->register_filename(config, "media.bluray.device",
                                           "/dev/dvd", XINE_CONFIG_STRING_IS_DIRECTORY_NAME,
                                           _("device used for BluRay playback"),
                                           _("The path to the device "
                                             "which you intend to use for playing BluRy discs."),
                                           0, device_change_cb, (void *) this);
  this->keyfile = config->register_filename(config, "media.bluray.keyfile",
                                            "~/.xine/aacskeys.bin", XINE_CONFIG_STRING_IS_DIRECTORY_NAME,
                                            _("AACS key file"),
                                            _("Location of libaacs key file."),
                                            0, keyfile_change_cb, (void *) this);

  return this;
}

/*
 * exported plugin catalog entry
 */

const plugin_info_t xine_plugin_info[] EXPORTED = {
  /* type, API, "name", version, special_info, init_function */
  { PLUGIN_INPUT | PLUGIN_MUST_PRELOAD, 17, "BLURAY", XINE_VERSION_CODE, NULL, bluray_init_plugin },
  { PLUGIN_NONE, 0, "", 0, NULL, NULL }
};
