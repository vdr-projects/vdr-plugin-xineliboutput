/*
 * xine_fbfe_frontend.c: Simple front-end, framebuffer functions
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#include <inttypes.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#if defined(__linux__)
# include <linux/kd.h>
#endif

#define LOG_MODULENAME "[vdr-fbfe]  "
#include "logdefs.h"

#include "xine_frontend_internal.h"


/*
 * data
 */

typedef struct fbfe_s {

  /* function pointers / base class */
  union {
    frontend_t fe;  /* generic frontend */
    fe_t       x;   /* xine frontend */
  };

  /* stored original handlers */
  int (*fe_xine_init)(frontend_t *this_gen, const char *audio_driver, 
		      const char *audio_port,
		      const char *video_driver, 
		      int pes_buffers,
		      const char *static_post_plugins);

  /* display */
/*char   *modeline;*/
  int     fd_tty;

  /* frontend */
  uint8_t fullscreen : 1;
/*uint8_t vmode_switch : 1;*/

} fbfe_t;

static void fbfe_update_display_size(fe_t *this_gen)
{
  fbfe_t *this = (fbfe_t*)this_gen;
  if(this->fullscreen && this->x.video_port) {
    this->x.width  = this->x.video_port->get_property(this->x.video_port, 
						      VO_PROP_WINDOW_WIDTH);
    this->x.height = this->x.video_port->get_property(this->x.video_port, 
						      VO_PROP_WINDOW_HEIGHT);
    LOGDBG("Framebuffer size after initialization: %dx%d",
	   this->x.width, this->x.height);
  }
}

/*
 * update_DFBARGS
 *
 * (optionally) add fbdev option to DFBARGS environment variable
 */
static void update_DFBARGS(const char *fb_dev)
{
  const char *env_old = getenv("DFBARGS");
  char *env_new = NULL;

  if (env_old) {
    char *env_tmp = strdup(env_old);
    char *head    = strstr(env_tmp, "fbdev=");

    if (head) {
      char *tail = strchr(head, ',');
      if(head == env_tmp)
	head = NULL;
      else
	*head = 0;
      asprintf(&env_new, "%sfbdev=%s%s",
	       head ? env_tmp : "", fb_dev, tail ? tail : "");
    } else {
      asprintf(&env_new, "fbdev=%s%s%s", fb_dev, env_tmp ? "," : "", env_tmp ?: "");
    }
    free(env_tmp);

    LOGMSG("replacing environment variable DFBARGS with %s (original was %s)", 
	   env_new, env_old);

  } else {
    asprintf(&env_new, "fbdev=%s", fb_dev);

    LOGMSG("setting environment variable DFBARGS to %s", env_new);
  }

  setenv("DFBARGS", env_new, 1);
  free(env_new);
}

/*
 * fbfe_display_open
 */
static int fbfe_display_open(frontend_t *this_gen, int width, int height, int fullscreen, int hud,
			     int modeswitch, const char *modeline, int aspect,
			     fe_keypress_f keyfunc, const char *video_port,
			     int scale_video, int field_order,
			     const char *aspect_controller, int window_id) 
{
  fbfe_t *this = (fbfe_t*)this_gen;

  if(!this)
    return 0;

  if(this->fd_tty >= 0)
    this->fe.fe_display_close(this_gen);

  if(keyfunc) {
    this->x.keypress = keyfunc;
    this->x.keypress("KBD", "");
  }

  LOGDBG("fbfe_display_open(width=%d, height=%d, fullscreen=%d, display=%s)",
	 width, height, fullscreen, video_port);

  this->x.xpos          = 0;
  this->x.ypos          = 0;
  this->x.width         = width;
  this->x.height        = height;
  this->x.aspect        = aspect;
/*this->x.cropping      = 0;*/
  this->x.field_order   = field_order;
  this->x.scale_video   = scale_video;
  this->x.overscan      = 0;
  this->x.display_ratio = 1.0;
  this->x.aspect_controller = aspect_controller ? strdup(aspect_controller) : NULL;

  this->fullscreen      = fullscreen;
/*this->vmode_switch    = modeswitch;*/
/*this->modeline        = strdup(modeline ?: "");*/

  /* setup xine FB visual */
  this->x.xine_visual_type = XINE_VISUAL_TYPE_FB;
  this->x.vis_fb.frame_output_cb = this->x.frame_output_handler;
  this->x.vis_fb.user_data = this;

  /* select framebuffer device ? */
  if(video_port && !strncmp(video_port, "/dev/", 5))
    this->x.video_port_name = strdup(video_port);
  else
    this->x.video_port_name = NULL;

  /* set console to graphics mode */
#if defined(KDSETMODE) && defined(KD_GRAPHICS)
  if (isatty(STDIN_FILENO))
    this->fd_tty = dup(STDIN_FILENO);
  else
    this->fd_tty = open("/dev/tty", O_RDWR);
  
  if(this->fd_tty < 0)
    LOGERR("fbfe_display_open: error opening /dev/tty");
  else if (ioctl(this->fd_tty, KDSETMODE, KD_GRAPHICS) == -1)
    LOGERR("fbfe_display_open: failed to set /dev/tty to graphics mode");
#else
# warning No support for console graphics mode
  this->fd_tty = -1;
#endif

  return 1;
}

/*
 * fbfe_display_config
 *
 * configure windows
 */
static int fbfe_display_config(frontend_t *this_gen, int width, int height, int fullscreen, 
			       int modeswitch, const char *modeline, int aspect, 
			       int scale_video, int field_order) 
{
  fbfe_t *this = (fbfe_t*)this_gen;

  if(!this)
    return 0;

  this->x.width       = width;
  this->x.height      = height;
  this->x.aspect      = aspect;
  this->x.scale_video = scale_video;
  this->x.field_order = field_order;
  this->fullscreen    = fullscreen;
/*this->vmode_switch  = modeswitch;*/
#if 0
  if(!modeswitch && strcmp(modeline, this->modeline)) {
    free(this->modeline);
    this->modeline = strdup(modeline ?: "");
    /* #warning XXX TODO - switch vmode */
  }
#endif

  return 1;
}

static void fbfe_interrupt(frontend_t *this_gen) 
{
  /* stop fbfe_run() */
}

static int fbfe_run(frontend_t *this_gen) 
{
  fbfe_t *this = (fbfe_t*)this_gen;

  if(this && this->x.playback_finished)
    return !this->x.playback_finished;

  /* just sleep 500ms */
  select(0, NULL, NULL, NULL, &(struct timeval){ .tv_sec = 0, .tv_usec = 500*1000 }); 

  return !(!this || this->x.playback_finished);
}

static void fbfe_display_close(frontend_t *this_gen) 
{
  fbfe_t *this = (fbfe_t*)this_gen;

  if (!this)
    return;

  if (this->x.xine)
    this->fe.xine_exit(this_gen);

  if (this->fd_tty >= 0) {
#if defined(KDSETMODE) && defined(KD_TEXT)
    if(ioctl(this->fd_tty, KDSETMODE, KD_TEXT) == -1)
      LOGERR("fbfe_display_close: failed to set /dev/tty to text mode");
#endif
    close(this->fd_tty);
    this->fd_tty = -1;
  }

  free(this->x.video_port_name);
  this->x.video_port_name = NULL;

  free(this->x.aspect_controller);
  this->x.aspect_controller = NULL;
#if 0
  free(this->modeline);
  this->modeline = NULL;
#endif
}

static int fbfe_xine_init(frontend_t *this_gen, const char *audio_driver, 
			  const char *audio_port,
			  const char *video_driver, 
			  int pes_buffers,
			  const char *static_post_plugins)
{
  fbfe_t *this = (fbfe_t*)this_gen;

  if (video_driver && !strcmp(video_driver, "DirectFB"))
    update_DFBARGS(this->x.video_port_name);

  return this->fe_xine_init(this_gen, audio_driver, audio_port,
			    video_driver, pes_buffers, static_post_plugins);
}

static frontend_t *fbfe_get_frontend(void)
{
  fbfe_t *this = calloc(1, sizeof(fbfe_t));

  this->fd_tty = -1;

  init_fe((fe_t*)this);

  this->fe.fe_display_open   = fbfe_display_open;
  this->fe.fe_display_config = fbfe_display_config;
  this->fe.fe_display_close  = fbfe_display_close;

  this->fe.fe_run       = fbfe_run;
  this->fe.fe_interrupt = fbfe_interrupt;

  this->x.update_display_size_cb = fbfe_update_display_size;

  /* override */
  this->fe_xine_init  = this->fe.xine_init;
  this->fe.xine_init  = fbfe_xine_init;

  return (frontend_t*)this;
}

/* ENTRY POINT */
const fe_creator_f fe_creator __attribute__((visibility("default"))) = fbfe_get_frontend;


