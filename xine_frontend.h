/*
 * xine_frontend.h:
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef _XINE_FRONTEND_H
#define _XINE_FRONTEND_H

#ifdef __cplusplus
extern "C" {
#endif

#define FE_VERSION_STR  XINELIBOUTPUT_VERSION /*"1.0.0pre1"*/

typedef void (*fe_keypress_f)(const char *keymap, const char *name);

typedef struct frontend_config_s frontend_config_t;
typedef struct frontend_s frontend_t;

struct osd_command_s;

#if 0
struct frontend_config_s {
  /* Display */
  int width;
  int height;
  int fullscreen;
  int modeswitch;
  char *modeline;
  int aspect;

  char *video_port;

  int scale_video;
  int field_order;

  fe_keypress_f keypresshandler;

  /* Xine engine */
  char *audio_driver;
  char *audio_port;
  char *video_driver;
  int pes_buffers;
  int priority;
};
#endif

/* xine_is_finished return values */
#define FE_XINE_RUNNING 0
#define FE_XINE_ERROR  -1
#define FE_XINE_EXIT    1

/* return values */
#define FE_OK     1
#define FE_ERROR  0

struct frontend_s {
  /* Display */
  int (*fe_display_open)(frontend_t*,
                         int xpos, int ypos,
                         int winwidth, int winheight,
                         int fullscreen, int hud, int modeswitch, const char *modeline,
                         int aspect, fe_keypress_f keypresshandler,
                         int no_x_kbd, int gui_hotkeys,
                         const char *video_port,
                         int scale_video, int field_order,
                         const char *aspect_controller, int window_id);
  int  (*fe_display_config)(frontend_t *,
                            int xpos, int ypos,
                            int width, int height,
                            int fullscreen,
                            int modeswitch, const char *modeline,
                            int aspect, int scale_video, int field_order);
  void (*fe_display_close)(frontend_t*);

  /* Xine engine */
  int  (*xine_init)(frontend_t*,
                    const char *audio_driver,
                    const char *audio_port,
                    const char *video_driver,
                    int pes_buffers,
                    const char *static_post,
                    const char *config_file);
  int  (*xine_open)(frontend_t*, const char *mrl);
  int  (*xine_play)(frontend_t*);
  int  (*xine_stop)(frontend_t*);
  void (*xine_close)(frontend_t*);
  void (*xine_exit)(frontend_t*);

  void (*shutdown_init)(frontend_t*, const char *cmd, int timeout);

  /* Execution control */
  int  (*fe_run)(frontend_t*);
  void (*fe_interrupt)(frontend_t*);
  void (*fe_free)(frontend_t*);

  int  (*xine_is_finished)(frontend_t*, int slave_stream);

  /* Data transfer VDR -> frontend/xine */
  int  (*xine_osd_command)(frontend_t*, struct osd_command_s *cmd);
  int  (*xine_control)(frontend_t*, const char *cmd);
  int  (*xine_queue_pes_packet)(frontend_t*, const char *data, int len);

  char *(*grab)(frontend_t*, int *size, int jpeg, int quality,
                int width, int height);

  /* events from frontend -> xine/vdr */
  int  (*send_event)(frontend_t *fe, const char *data);
  int  (*send_input_event)(frontend_t *fe,
                           const char *map, const char *key,
                           int repeat, int release);
#if 0
  frontend_config_t config;
#endif
};

typedef frontend_t *(*fe_creator_f)(void);

void list_xine_plugins(frontend_t *fe, int verbose);


#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* _XINE_FRONTEND_H */

