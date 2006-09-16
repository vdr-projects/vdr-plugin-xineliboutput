/*
 * xine_frontend.c:
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef XINE_VERSION_CODE
#  define XINE_VERSION_CODE (XINE_MAJOR_VERSION*10000 + \
                             XINE_MINOR_VERSION*100 + \
                             XINE_SUB_VERSION)
#endif

#define LOG_MODULENAME "[vdr-fe]    "
#include "logdefs.h"

#include "xine/post.h"

#ifdef FE_STANDALONE
  /* next two symbols are dynamically linked from input plugin */
  int SysLogLevel __attribute__((visibility("default"))) = 2; /* errors and info, no debug */
  int LogToSysLog __attribute__((visibility("default"))) = 0; /* log to syslog instead of console */

  static int verbose_xine_log = 0;
#else
  extern int SysLogLevel; /* vdr tools.c */
  int LogToSysLog __attribute__((visibility("default"))) = 1;    /* dynamically linked from input plugin */
#endif

/* from vdr_input_plugin: */
typedef struct {
  input_plugin_t           input_plugin;
  vdr_input_plugin_funcs_t f;
  /* ... */
} vdr_input_plugin_t;


/* 
 * logging 
 */

#if !defined(XINELIBOUTPUT_DEBUG_STDOUT) && \
    !defined(XINELIBOUTPUT_DEBUG_STDERR)

#undef x_syslog

static void x_syslog(int level, const char *fmt, ...)
{
  va_list argp;
  char buf[512];
  va_start(argp, fmt);
  vsnprintf(buf, 512, fmt, argp);
  if(!LogToSysLog) {
    printf(LOG_MODULENAME "%s\n", buf);
  } else {
    syslog(level, "[%ld] " LOG_MODULENAME "%s", syscall(__NR_gettid), buf);
  }
  va_end(argp);
}

#endif

/*
 * detect input plugin 
 */

static int find_input(fe_t *this)
{
  if(!this->input) {
    if(!this->stream || !this->stream->input_plugin ||
       !this->stream->input_plugin->input_class || this->playback_finished) {
      LOGMSG("find_input: stream not initialized or playback finished !");
      usleep(100*1000);
      return 0;
    }
    if(strcmp(this->stream->input_plugin->input_class->get_identifier(
	      this->stream->input_plugin->input_class),
              "xvdr")) {
      LOGMSG("find_input: current xine input plugin is not xvdr !");
      /*usleep(100*1000);*/
      return 0;
    }
    this->input = this->stream->input_plugin;
  }
  return 1;
}

static void *fe_control(void *fe_handle, const char *cmd);

/*
 * xine callbacks
 */

static double fe_dest_pixel_aspect(fe_t *this, double video_pixel_aspect,
				   int video_width, int video_height)
{
  int new_cropping = 0;
  double result = 1.0;

  if(!this->scale_video) {

    /*#warning what to do if scaling disabled ???*/

    /*return video_pixel_aspect;*/
  }

  switch(this->aspect) {
    /* Auto */
    default: {
               double correction = 
		 ((double)video_width/(double)video_height) /
		 ((double)this->width/(double)this->height);

               result = video_pixel_aspect * correction;
               if(result > (16.9/9.0 * (double)this->height/
			    (double)this->width))
	         result = (16.0/9.0 * (double)this->height/
			   (double)this->width);
	       break;
             }
    /* Default */
    case 1:  result = this->display_ratio; break;

    /* 4:3 */
    case 2:  result = (4.0/3.0 * (double)this->height/(double)this->width); break;
    /* 16:9 */
    case 3:  result = (16.0/9.0 * (double)this->height/(double)this->width); break;
    /* 16:10 */
    case 4:  result = (16.0/10.0 * (double)this->height/(double)this->width); break;
    /* Pan&Scan */
    case 5: {
      double aspect_diff /*= video_pixel_aspect - 1.0*/;
	      /* TODO */
	      /* does not work (?) */
aspect_diff=(video_pixel_aspect*(double)video_width/(double)video_height) - 4.0 / 3.0;
              if ((aspect_diff < 0.05) && (aspect_diff > -0.05)) {
		result = (4.0/3.0 * (double)this->height/(double)this->width);
		/*LOGDBG("diff: %f", aspect_diff);*/
		new_cropping = 1;
	      } else {
		result = (16.0/9.0 * (double)this->height/(double)this->width);
	      }
	      /*result = (4.0/3.0 * (double)this->height/(double)this->width);*/
	      break;
            }
    /* center cut-out */
    case 6: {
/*#warning center cut-out mode not implemented*/
      break;
    }

  }
#if 0
  if(this->cropping && !new_cropping) {
    LOGDBG("pan&scan CROP OFF");
    xine_set_param(this->stream, XINE_PARAM_VO_CROP_LEFT, 0);
    xine_set_param(this->stream, XINE_PARAM_VO_CROP_TOP, 72);
    xine_set_param(this->stream, XINE_PARAM_VO_CROP_RIGHT, 0);
    xine_set_param(this->stream, XINE_PARAM_VO_CROP_BOTTOM, 72);
    this->cropping = 0;
  }
  if(!this->cropping && new_cropping) {
    LOGDBG("pan&scan CROP ON");
    /*** Should set unscaled osd (or top & bottom will be cropped off) */
    xine_set_param(this->stream, XINE_PARAM_VO_CROP_LEFT, 0);
    xine_set_param(this->stream, XINE_PARAM_VO_CROP_TOP, 0);
    xine_set_param(this->stream, XINE_PARAM_VO_CROP_RIGHT, 0);
    xine_set_param(this->stream, XINE_PARAM_VO_CROP_BOTTOM, 0);
    this->cropping = 1;
  }
#endif

  return result;
}

static void fe_frame_output_cb (void *data,
				int video_width, int video_height, 
				double video_pixel_aspect,
				int *dest_x, int *dest_y, 
				int *dest_width, int *dest_height, 
				double *dest_pixel_aspect,
				int *win_x, int *win_y) 
{
  fe_t *this = (fe_t *)data;

  if (!this)
    return;

  *dest_width  = this->width;
  *dest_height = this->height;
  *dest_x = 0;
#ifndef HAVE_XV_FIELD_ORDER
  *dest_y = 0 + this->field_order;
#else
  *dest_y = 0;
#endif

#if 1
  if(!this->scale_video) {
    if(video_height < this->height) {
      *dest_height = video_height;
      *dest_y = (this->height - video_height) / 2;
    }
    if(video_width < this->width) {
      *dest_width  = video_width;
      *dest_x = (this->width - video_width) / 2;
    }
  }
#endif

  *win_x = this->xpos;
  *win_y = this->ypos;  
  
  *dest_pixel_aspect = fe_dest_pixel_aspect(this, video_pixel_aspect,
					    video_width, video_height);

#if 0
  if(this->cropping) {
    *dest_pixel_aspect = *dest_pixel_aspect * (16.0/9.0)/(4.0/3.0);
    *dest_y = *dest_y - 72;
    *dest_height = *dest_height + 144;
  }
#endif

#if 0
  /* video_out cropping works better */
  if(this->overscan) {
    int crop_x = this->overscan * this->width  / 100 / 2;
    int crop_y = this->overscan * this->height / 100 / 2;
    *dest_x -= crop_x;
    *dest_y -= crop_y;
    *dest_width  += crop_x;
    *dest_height += crop_y;
  }
#endif

  if(!this->stream)
    return;

  _x_stream_info_set(this->stream, XINE_STREAM_INFO_VIDEO_RATIO, 
		     (int)(10000.0*video_pixel_aspect * 
			   ((double)video_width)/((double)video_height)));
  if(this->video_width  != video_width ||
     this->video_height != video_height) {
    xine_event_t event;
    xine_format_change_data_t framedata;
    event.type = XINE_EVENT_FRAME_FORMAT_CHANGE;
    event.stream = this->stream;
    event.data = &framedata;
    event.data_length = sizeof(framedata);
    framedata.width  = video_width;
    framedata.height = video_height;
    framedata.aspect = 0; /* TODO */
    framedata.pan_scan = 0;
    xine_event_send(this->stream, &event);
    this->video_width = video_width;
    this->video_height = video_height;

    /* trigger forced redraw to make cropping changes effective */
    xine_set_param(this->stream, XINE_PARAM_VO_ZOOM_X, 100);      
  }
}

static void xine_event_cb (void *user_data, const xine_event_t *event) 
{
  fe_t *this = (fe_t *)user_data;

  switch (event->type) {
    /* in local mode: vdr stream / slave stream ; in remote mode: vdr stream only */
    case XINE_EVENT_UI_PLAYBACK_FINISHED:
      LOGMSG("xine_event_cb: XINE_EVENT_UI_PLAYBACK_FINISHED");
      if(this) {
	if(event->stream == this->stream)
	  this->playback_finished = 1;
      } else {
	LOGMSG("xine_event_cb: NO USER DATA !");
      }
      break;
  }
}

/*
 * fe_xine_init
 *
 * initialize xine engine, initialize audio and video ports, setup stream
 */

#define MONO          0
#define STEREO        1
#define HEADPHONES    2
#define SURROUND21    3
#define SURROUND3     4
#define SURROUND4     5
#define SURROUND41    6
#define SURROUND5     7
#define SURROUND51    8
#define SURROUND6     9
#define SURROUND61    10
#define SURROUND71    11
#define A52_PASSTHRU  12 

static void configure_audio_out(fe_t *this, const char *audio_driver, const char *audio_port)
{
  /* define possible speaker arrangements */
  /* xine audio_alsa_out.c */
  static char *speaker_arrangement[] = 
    {"Mono 1.0", "Stereo 2.0", "Headphones 2.0", "Stereo 2.1",
     "Surround 3.0", "Surround 4.0", "Surround 4.1", 
     "Surround 5.0", "Surround 5.1", "Surround 6.0",
     "Surround 6.1", "Surround 7.1", "Pass Through", NULL};
  
  if(!strcmp("alsa", audio_driver) && strlen(audio_port) > 0) {
    
    this->xine->config->register_string(this->xine->config,
					"audio.device.alsa_default_device",
					"default",
					"device used for mono output",
					"xine will use this alsa device "
					"to output mono sound.\n"
					"See the alsa documentation "
					"for information on alsa devices.",
					10, NULL,
					NULL);
    this->xine->config->register_string(this->xine->config,
					"audio.device.alsa_front_device",
					"plug:front:default",
					"device used for stereo output",
					"xine will use this alsa device "
					"to output stereo sound.\n"
					"See the alsa documentation "
					"for information on alsa devices.",
					10, NULL,
					NULL);
    this->xine->config->register_string(this->xine->config,
					"audio.device.alsa_surround51_device",
					"plug:surround51:0",
					"device used for 5.1-channel output",
					"xine will use this alsa device to output "
					"5 channel plus LFE (5.1) surround sound.\n"
					"See the alsa documentation for information "
					"on alsa devices.",
					10,  NULL,
					NULL);
    this->xine->config->register_string(this->xine->config,
					"audio.device.alsa_passthrough_device",
					"iec958:AES0=0x6,AES1=0x82,AES2=0x0,AES3=0x2",
					"device used for 5.1-channel output",
					"xine will use this alsa device to output "
					"undecoded digital surround sound. This can "
					"be used be external surround decoders.\nSee the "
					"alsa documentation for information on alsa "
					"devices.",
					10, NULL,
					NULL);
    this->xine->config->register_enum
      (this->xine->config,
       "audio.output.speaker_arrangement",
       STEREO,
       speaker_arrangement,
       "speaker arrangement",
       "Select how your speakers are arranged, "
       "this determines which speakers xine uses for sound output. "
       "The individual values are:\n\n"
       "Mono 1.0: You have only one speaker.\n"
       "Stereo 2.0: You have two speakers for left and right channel.\n"
       "Headphones 2.0: You use headphones.\n"
       "Stereo 2.1: You have two speakers for left and right channel, and one "
       "subwoofer for the low frequencies.\n"
       "Surround 3.0: You have three speakers for left, right and rear channel.\n"
       "Surround 4.0: You have four speakers for front left and right and rear "
       "left and right channels.\n"
       "Surround 4.1: You have four speakers for front left and right and rear "
       "left and right channels, and one subwoofer for the low frequencies.\n"
       "Surround 5.0: You have five speakers for front left, center and right and "
       "rear left and right channels.\n"
       "Surround 5.1: You have five speakers for front left, center and right and "
       "rear left and right channels, and one subwoofer for the low frequencies.\n"
       "Surround 6.0: You have six speakers for front left, center and right and "
       "rear left, center and right channels.\n"
       "Surround 6.1: You have six speakers for front left, center and right and "
       "rear left, center and right channels, and one subwoofer for the low frequencies.\n"
       "Surround 7.1: You have seven speakers for front left, center and right, "
       "left and right and rear left and right channels, and one subwoofer for the "
       "low frequencies.\n"
       "Pass Through: Your sound system will receive undecoded digital sound from xine. "
       "You need to connect a digital surround decoder capable of decoding the "
       "formats you want to play to your sound card's digital output.",
       10, NULL,
       NULL);

    this->xine->config->update_string(this->xine->config,
				      "audio.device.alsa_front_device",
				      audio_port);
    this->xine->config->update_string(this->xine->config,
				      "audio.device.alsa_default_device",
				      audio_port);
    this->xine->config->update_string(this->xine->config,
				      "audio.device.alsa_surround51_device",
				      audio_port);
    if(strstr(audio_port, "iec") ||
       strstr(audio_port, "spdif")) {
      this->xine->config->update_string(this->xine->config,
					"audio.device.alsa_passthrough_device",
					audio_port);
      this->xine->config->update_num(this->xine->config,
				     "audio.output.speaker_arrangement",
				     A52_PASSTHRU);
    } else {
      this->xine->config->update_num(this->xine->config,
				     "audio.output.speaker_arrangement",
				     strstr(audio_port, "surround") ? SURROUND51 : STEREO);
    }
  }

  if(!strcmp("oss", audio_driver) && !strncmp("/dev/dsp", audio_port, 8)) {
    int num = 0;
    sscanf(audio_port+8,"%d",&num);
    this->xine->config->update_num(this->xine->config,"audio.device.oss_device_num",num);
  }
}

static int fe_xine_init(frontend_t *this_gen, const char *audio_driver, 
			const char *audio_port,
			const char *video_driver, 
			int pes_buffers, int priority, 
			const char *static_post_plugins)
{
  fe_t *this = (fe_t*)this_gen;
  post_plugins_t *posts = NULL;

  if(!this)
    return 0;

  /*
   * init xine engine
   */

  if(this->xine)
    this->fe.xine_exit(this_gen);
  
  this->stream          = NULL;
  this->video_port      = NULL;
  this->audio_port      = NULL;
  this->input           = NULL;

  /* create a new xine and load config file */
  this->xine = xine_new();
  if(!this->xine)
    return 0;

#ifdef FE_STANDALONE
  this->xine->verbosity = verbose_xine_log;
#else
  this->xine->verbosity = (SysLogLevel>2);
#endif

  /*xine_register_log_cb(this->xine, xine_log_cb, this);*/

  /* TODO: use different config file ? (vdr conf.dir/xine/config_vdr ?) */
  sprintf(this->configfile, "%s%s", xine_get_homedir(), 
	  "/.xine/config_xineliboutput");
  
  xine_config_load (this->xine, this->configfile);

  /*this->xine->config->update_num(this->xine->config, "video.device.xv_double_buffer", 1); */
  /*this->xine->config->update_num(this->xine->config, "engine.buffers.video_num_buffers", 
                                   pes_buffers); */
  /*#warning update ??? add TYPE_UNKNOWN ... ? like when xine reads config file ...?*/
  this->xine->config->register_num (this->xine->config,
				    "engine.buffers.video_num_buffers",
				    500,
				    "number of video buffers",
				    "The number of video buffers "
				    "(each is 8k in size) "
				    "xine uses in its internal queue. "
				    "Higher values "
				    "mean smoother playback for unreliable "
				    "inputs, but "
				    "also increased latency and memory "
				    "consumption.",
				    20, NULL, NULL);

  xine_init (this->xine);

  this->xine->config->update_num(this->xine->config,"video.device.xv_double_buffer", 1);
  this->xine->config->update_num(this->xine->config,"engine.buffers.video_num_buffers", pes_buffers);

  this->playback_finished = 0;

  /* create video port */
  
  this->video_port = xine_open_video_driver(this->xine,
					    video_driver,
					    this->xine_visual_type,
					    (void *) &(this->vis));
  if(!this->video_port) {
    LOGMSG("fe_xine_init: xine_open_video_driver(\"%s\") failed",
	   video_driver?video_driver:"(NULL)"); 
    xine_exit(this->xine);
    this->xine = NULL;
    return 0;
  }

  this->video_port_none = NULL;
  
  /* re-configure display size (DirectFB driver changes display mode in init) */
  if(this->update_display_size)
    this->update_display_size(this_gen);

  /* create audio port */

  if(audio_driver && audio_port) 
    configure_audio_out(this, audio_driver, audio_port);

  if(audio_driver && !strcmp(audio_driver, "auto"))
    this->audio_port = xine_open_audio_driver (this->xine, NULL, NULL);
  if(audio_driver && !strcmp(audio_driver, "none")) {
    this->audio_port = _x_ao_new_port (this->xine, NULL, 1);
    this->audio_port->set_property(this->audio_port, AO_PROP_DISCARD_BUFFERS, 1);
  } else
    this->audio_port = xine_open_audio_driver (this->xine, audio_driver, NULL);
  
  if(!this->audio_port && (audio_driver && !!strcmp(audio_driver, "none"))) {
    LOGMSG("fe_xine_init: xine_open_audio_driver(\"%s%s%s\") failed",
	   audio_driver?audio_driver:"(NULL)", 
	   audio_port?":":"", audio_port?audio_port:""); 
  }

  this->audio_port_none = NULL;

  /* create stream */

  this->stream = xine_stream_new(this->xine, this->audio_port, this->video_port);

  if(!this->stream) {
    LOGMSG("fe_xine_init: xine_stream_new failed"); 

    if(this->audio_port)
      xine_close_audio_driver(this->xine, this->audio_port);
    this->audio_port = NULL;
    xine_close_video_driver(this->xine, this->video_port);
    this->video_port = NULL;
    xine_exit(this->xine);
    this->xine = NULL;

    return 0;
  }

  /* event handling */

  this->event_queue = xine_event_new_queue (this->stream);
  xine_event_create_listener_thread (this->event_queue, xine_event_cb, this);

  if(!this->event_queue)
    LOGMSG("fe_xine_init: xine_event_new_queue failed");

  /* misc. config */

  this->priority = priority;
  this->pes_buffers = pes_buffers;

  posts = this->postplugins = malloc(sizeof(post_plugins_t));
  memset(posts, 0, sizeof(post_plugins_t));
  posts->xine = this->xine;
  posts->audio_port = this->audio_port;
  posts->video_port = this->video_port;
  posts->stream = this->stream;

  if(static_post_plugins && *static_post_plugins) {
    int i;
    LOGDBG("static post plugins (from command line): %s", static_post_plugins);
    posts->static_post_plugins = strdup(static_post_plugins);
    vpplugin_parse_and_store_post(posts, posts->static_post_plugins);
    applugin_parse_and_store_post(posts, posts->static_post_plugins);

    for(i=0; i<posts->post_audio_elements_num; i++)
      if(posts->post_audio_elements[i])
	posts->post_audio_elements[i]->enable = 2;
    for(i=0; i<posts->post_video_elements_num; i++)
      if(posts->post_video_elements[i])
	posts->post_video_elements[i]->enable = 2;
    posts->post_video_enable = 1;
    posts->post_audio_enable = 1;
  }

  this->video_width = this->video_height = 0;

  return 1;
}

/*
 * fe_xine_open
 *
 * open xine stream
 */

static int fe_xine_open(frontend_t *this_gen, const char *mrl)
{
  fe_t *this = (fe_t*)this_gen;
  int result = 0;
  char url[1024] = "";

  if(!this)
    return 0;

  this->input = NULL;
  this->playback_finished = 1;

  if(!mrl)
    mrl = "xvdr://";

  sprintf(url, "%s#nocache;demux:mpeg_block", mrl);

  result = xine_open(this->stream, url);

  if(!result) {
    LOGMSG("fe_xine_open: xine_open(\"%s\") failed", mrl);
    return 0;
  }

#if 0
  /* priority */
  {
    struct sched_param temp;
    temp.sched_priority = this->priority;
    if(!pthread_setschedparam(this->stream->demux_thread, SCHED_RR, &temp))
      LOGERR("pthread_setschedparam(demux_thread, SCHED_RR, %d) failed", 
	     temp.sched_priority);
    if(!pthread_setschedparam(this->stream->video_thread, SCHED_RR, &temp))
      LOGERR("pthread_setschedparam(video_thread, SCHED_RR, %d) failed", 
	     temp.sched_priority);
    if(!pthread_setschedparam(this->stream->audio_thread, SCHED_RR, &temp))
      LOGERR("pthread_setschedparam(audio_thread, SCHED_RR, %d) failed", 
	     temp.sched_priority);
  }
#endif

#if 0
  this->xine->config->update_num(this->xine->config,
				 "video.output.xv_double_buffer",
				 1);
#endif
  this->xine->config->update_num(this->xine->config,
				 "engine.buffers.video_num_buffers",
				 this->pes_buffers);  
  return result;
}

/*
 * post plugin handling
 *
 */

#define POST_AUDIO_VIS  0
#define POST_AUDIO      1
#define POST_VIDEO      2
#define POST_VIDEO_PIP  3


static void init_dummy_ports(fe_t *this, int on)
{
  if(!on) {
    if(this->postplugins->slave_stream)
      LOGMSG("ERROR: init_dummy_ports(false) called while port is still in use !");
    
    if(this->audio_port_none)
      xine_close_audio_driver(this->xine, this->audio_port_none);
    this->audio_port_none = NULL;
    if(this->video_port_none)
      xine_close_video_driver(this->xine, this->video_port_none);
    this->video_port_none = NULL;
  } else {
    if(! this->audio_port_none)
      this->audio_port_none = _x_ao_new_port (this->xine, NULL, 1); 
    this->audio_port_none->set_property(this->audio_port_none, AO_PROP_DISCARD_BUFFERS, 1);
    /*LOGMSG("initialized dummy audio port %x", this->audio_port_none);*/
#if 0
    if(! this->video_port_none)
      this->video_port_none =
	_x_vo_new_port(this->xine, 
		       _x_load_video_output_plugin(this->xine, "none", 
						   XINE_VISUAL_TYPE_NONE, NULL),
		       1);
    this->video_port_none->set_property(this->video_port_none, VO_PROP_DISCARD_FRAMES, 1);
#endif
  }
}

static void fe_post_unwire(fe_t *this)
{
  xine_post_out_t  *vo_source = xine_get_video_source(this->stream);
  xine_post_out_t  *ao_source = xine_get_audio_source(this->stream);

  if(this->postplugins->slave_stream) {
    LOGDBG("unwiring slave stream post plugins");

    init_dummy_ports(this, 1);
    (void) xine_post_wire_audio_port(ao_source, this->audio_port_none);
    /*(void) xine_post_wire_video_port(vo_source, this->video_port_none);*/

    vo_source = xine_get_video_source(this->postplugins->slave_stream);
    ao_source = xine_get_audio_source(this->postplugins->slave_stream);
    (void) xine_post_wire_video_port(vo_source, this->video_port);
    (void) xine_post_wire_audio_port(ao_source, this->audio_port);

  } else {
    LOGDBG("unwiring post plugins");
    init_dummy_ports(this, 0);
    (void) xine_post_wire_video_port(vo_source, this->video_port);
    (void) xine_post_wire_audio_port(ao_source, this->audio_port);   
  }
}

static void fe_post_rewire(fe_t *this)
{
  LOGDBG("re-wiring post plugins");
  vpplugin_rewire_posts(this->postplugins);
  applugin_rewire_posts(this->postplugins);
}

static void fe_post_unload(fe_t *this)
{
  LOGDBG("unloading post plugins");
  vpplugin_unload_post(this->postplugins, NULL);
  applugin_unload_post(this->postplugins, NULL);
}

static int fe_post_close(fe_t *this, const char *name, int which)
{
  post_plugins_t *posts = this->postplugins;
  int result = 0;

  if(!this)
    return 0;

  if(name && !strcmp(name, "AudioVisualization")) {
    name = NULL;
    which = POST_AUDIO_VIS;
  }
  if(name && !strcmp(name, "Pip")) {
    name = NULL;
    which = POST_VIDEO_PIP;
  }

  /* by name */
  if(name) {
    LOGDBG("closing post plugin: %s", name);
    if(applugin_unload_post(posts, name)) {
      /*LOGDBG("  * rewiring audio");*/
      applugin_rewire_posts(posts);
      return 1;
    }
    if(vpplugin_unload_post(posts, name)) {
      /*LOGDBG("  * rewiring video");*/
      vpplugin_rewire_posts(posts);
      return 1;
    }
    return 0;
  }

  /* by type */
  if(which == POST_AUDIO_VIS || which < 0) { /* audio visualization */
    if(posts->post_vis_elements_num && 
       posts->post_vis_elements &&   
       posts->post_vis_elements[0]) {
      LOGDBG("Closing audio visualization post plugins");
      if(applugin_unload_post(posts, posts->post_vis_elements[0]->name)) {
	/*LOGDBG("  * rewiring audio");*/
	applugin_rewire_posts(posts);
	result = 1;
      }
    }
  }

  if(which == POST_AUDIO || which < 0) { /* audio effect(s) */
    LOGDBG("Closing audio post plugins");
    if(applugin_disable_post(posts, NULL)) {
      /*LOGDBG("  * rewiring audio");*/
      applugin_rewire_posts(posts);
      result = 1;
    }
  }
  if(which == POST_VIDEO || which < 0) { /* video effect(s) */
    LOGDBG("Closing video post plugins");
    if(vpplugin_unload_post(posts, NULL)) {
      /*LOGDBG("  * rewiring video");*/
      vpplugin_rewire_posts(posts);
      result = 1;
    }
  }

  if(which == POST_VIDEO_PIP || which < 0) { /* Picture-In-Picture */
    if(posts->post_pip_elements_num && 
       posts->post_pip_elements &&   
       posts->post_pip_elements[0]) {
      LOGDBG("Closing PIP (mosaico) post plugins");
      if(vpplugin_unload_post(posts, "mosaico")) {
	/*LOGDBG("  * rewiring video");*/
	vpplugin_rewire_posts(posts);
	result = 1;
      }
    }
  }

  return result;
}

static int fe_post_open(fe_t *this, const char *name, const char *args)
{
  post_plugins_t *posts = this->postplugins;
  char initstr[1024];
  int found = 0;

  if(!this || !this->xine || !this->stream)
    return 0;

  /* pip */
  if(!strcmp(name, "Pip")) {
    posts->post_pip_enable = 1;
    name = "mosaico";
    if(!posts->post_pip_elements ||
       !posts->post_vis_elements[0] ||
       !posts->post_vis_elements[0]->enable)
      LOGMSG("enabling picture-in-picture (\"%s\") post plugin", initstr);
  }

  if(args)
    sprintf(initstr, "%s:%s", name, args);
  else
    strcpy(initstr, name);

  LOGDBG("opening post plugin: %s", initstr);

  /* close old audio visualization plugin */
  if(!strcmp(name,"goom") || !strcmp(name,"oscope") || 
     !strcmp(name,"fftscope") || !strcmp(name,"fftgraph")) {

    /* close if changed */
    if(posts->post_vis_elements_num && 
       posts->post_vis_elements &&   
       posts->post_vis_elements[0] &&
       strcmp(name, posts->post_vis_elements[0]->name)) {

      fe_post_close(this, NULL, POST_AUDIO_VIS);
    }

    posts->post_vis_enable = 1;
  }

  if(vpplugin_enable_post(posts, initstr, &found)) {
    posts->post_video_enable = 1;
    vpplugin_rewire_posts(posts);
    return 1;
  }
  if(!found && applugin_enable_post(posts, initstr, &found)) {
    posts->post_audio_enable = 1;
    applugin_rewire_posts(posts);
    return 1;
  }

  if(!found)
    LOGERR("Can't load post plugin %s", name);
  else
    LOGDBG("Post plugin %s loaded and wired", name);
    
  return 0;
}

static int fe_xine_play(frontend_t *this_gen) 
{
  fe_t *this = (fe_t*)this_gen;
  vdr_input_plugin_t *input_vdr;

  if(!this)
    return 0;

  fe_post_rewire(this);

  this->input = NULL;
  this->playback_finished = xine_play(this->stream, 0, 0) ? 0 : 1;

  if(!this->input && !find_input(this))
    return -1;

  input_vdr = (vdr_input_plugin_t *)this->input;
  input_vdr->f.xine_input_event = this->keypress;
  input_vdr->f.fe_control = fe_control;
  input_vdr->f.fe_handle  = (void*)this;

  if(this->playback_finished)
    LOGMSG("Error playing xvdr:// !");

  return !this->playback_finished;
}

static int fe_xine_stop(frontend_t *this_gen) 
{
  fe_t *this = (fe_t*)this_gen;

  if(!this)
    return 0;

  this->input = NULL;
  this->playback_finished = 1;

  xine_stop(this->stream);

  fe_post_unwire(this);

  return 1;
}

static void fe_xine_close(frontend_t *this_gen) 
{
  fe_t *this = (fe_t*)this_gen;

  if(!this)
    return;

  if (this && this->xine) {
#ifndef FE_STANDALONE
    if(this->input) {
      vdr_input_plugin_t *input_vdr;
      input_vdr = (vdr_input_plugin_t *)this->input;
      input_vdr->f.xine_input_event = NULL;
    }
#endif

    fe_xine_stop(this_gen);

    fe_post_unload(this);

    xine_close(this->stream);
    if(this->postplugins->pip_stream) 
      xine_close(this->postplugins->pip_stream);
  }
}

static void fe_xine_exit(frontend_t *this_gen) 
{
  fe_t *this = (fe_t*)this_gen;

  if (this && this->xine) {

    if(this->input || !this->playback_finished)
      fe_xine_close(this_gen);
    fe_post_unload(this);

    xine_config_save (this->xine, this->configfile);
    if(this->event_queue)
      xine_event_dispose_queue(this->event_queue);
    this->event_queue = NULL;

    if(this->stream)
      xine_dispose(this->stream);
    this->stream = NULL;

    if(this->postplugins->pip_stream) 
      xine_dispose(this->postplugins->pip_stream);
    this->postplugins->pip_stream = NULL;

    if(this->postplugins->slave_stream) 
      xine_dispose(this->postplugins->slave_stream);
    this->postplugins->slave_stream = NULL;

    if(this->audio_port)
      xine_close_audio_driver(this->xine, this->audio_port);
    this->audio_port = NULL;

    init_dummy_ports(this, 0);

    if(this->video_port)
      xine_close_video_driver(this->xine, this->video_port);
    this->video_port = NULL;

    if(this->postplugins->static_post_plugins)
      free(this->postplugins->static_post_plugins);
    free(this->postplugins);
    this->postplugins = NULL;

    xine_exit(this->xine);
    this->xine = NULL;
  }
}

static void fe_free(frontend_t *this_gen) 
{
  fe_t *this = (fe_t*)this_gen;

  if (this) {
    if(this->display)
      this->fe.fe_display_close(this_gen);
    free(this);
  }
}

static int fe_is_finished(frontend_t *this_gen, int slave_stream)
{
  fe_t *this = (fe_t*)this_gen;

  if(!this || this->playback_finished)
    return 1;
  
  if(slave_stream) {
    if(!this->postplugins->slave_stream || this->slave_playback_finished)
      return 1;
  }
  
  return 0;
}

/************************** hooks to input plugin ****************************/

#ifndef FE_STANDALONE

static int xine_control(frontend_t *this_gen, const char *cmd)
{
  fe_t *this = (fe_t*)this_gen;
  vdr_input_plugin_t *input_vdr;

  if(!this->input && !find_input(this))
    return -1;

  input_vdr = (vdr_input_plugin_t *)this->input;
  return input_vdr->f.push_input_control(this->input, cmd);
}

static int xine_osd_command(frontend_t *this_gen, struct osd_command_s *cmd) {
  fe_t *this = (fe_t*)this_gen;
  vdr_input_plugin_t *input_vdr;

  if(!this->input && !find_input(this))
    return -1;

  input_vdr = (vdr_input_plugin_t *)this->input;
  return input_vdr->f.push_input_osd(this->input, cmd);
}

static int xine_queue_pes_packet(frontend_t *this_gen, const char *data, int len)
{
  fe_t *this = (fe_t*)this_gen;
  vdr_input_plugin_t *input_vdr;

  if(!this->input && !find_input(this))
    return 0/*-1*/;

#if 0
  if(len<6) {
    LOGMSG("xine_queue_pes_packet: len == %d, too short!", len);
    abort();
  }
  /* must contain single pes packet and nothing else */
  if(data[0] || data[1] || (data[2] != 1)) {
    static int counter=0;
    counter++;
    LOGMSG("xine_queue_pes_packet: packet not starting with 00 00 01 \n"
	   "    packet #%d, size=%d : %02x %02x %02x %02x %02x %02x\n",
	   counter, len,
	   data[0], data[1], data[2], data[3], data[4], data[5]);
    abort();
  }
#endif

  input_vdr = (vdr_input_plugin_t *)this->input;
  return input_vdr->f.push_input_write(this->input, data, len);
}

#else /* #ifndef FE_STANDALONE */

static void process_xine_keypress(input_plugin_t *input, 
				  const char *map, const char *key,
				  int repeat, int release)
{
  /* from UI --> input plugin --> vdr */
  LOGDBG("Keypress: %s %s %s %s", 
	 map, key, repeat?"Repeat":"", release?"Release":"");
  if(input) {
    vdr_input_plugin_t *input_vdr = (vdr_input_plugin_t *)input;
    if(input_vdr->f.input_control) {
      input_vdr->f.input_control(input, map, key, repeat, release);
    } else {
      LOGMSG("Keypress --- NO HANDLER SET");
    }
  } else {
    LOGMSG("Keypress --- NO PLUGIN FOUND");
  }
}

#endif /* #ifndef FE_STANDALONE */

/*
 *  Control messages from input plugin
 */
static void *fe_control(void *fe_handle, const char *cmd)
{
  fe_t *this = (fe_t*)fe_handle;
  post_plugins_t *posts;

  /*LOGDBG("fe_control(\"%s\")", cmd);*/

  if(!cmd || !this) {
    LOGMSG("fe_control(0x%lx,0x%lx) : invalid argument", 
	   (unsigned long int)fe_handle, (unsigned long int)cmd);
    return NULL;
  }

  posts = this->postplugins;

  if(!posts) {
    LOGMSG("fe_control : this->posts == NULL");
    return NULL;
  }

  if(!strncmp(cmd, "SLAVE CLOSED", 16)) {
    /*LOGMSG("fe_control : slave closed");*/
    if(posts->slave_stream)
      fe_control(fe_handle, "SLAVE 0x0\r\n");
    init_dummy_ports(this, 0);

  } else if(!strncmp(cmd, "SLAVE 0x", 8)) {
    unsigned long pt;
    if(1 == sscanf(cmd, "SLAVE 0x%lx", &pt)) {
      xine_stream_t *slave_stream = (xine_stream_t*)pt;
      if(posts->slave_stream != slave_stream) {

	fe_post_unwire(this);

	if(posts->slave_stream) {
	  /*xine_post_out_t  *vo_source = xine_get_video_source(posts->slave_stream);*/
	  xine_post_out_t  *ao_source = xine_get_audio_source(posts->slave_stream);
	  LOGMSG("unwiring slave stream from output");
	  /*(void) xine_post_wire_video_port(vo_source, this->video_port_none);*/
	  (void) xine_post_wire_audio_port(ao_source, this->audio_port_none);
	}

	posts->slave_stream = slave_stream;

	if(posts->slave_stream)
	  fe_post_unwire(this);

	fe_post_rewire(this);
      }
      this->slave_playback_finished = (slave_stream==NULL);
    }

  } else if(!strncmp(cmd, "ENDOFSTREAM", 11)) {
    if(posts->slave_stream)
      this->slave_playback_finished = 1;

  } else if(!strncmp(cmd, "SUBSTREAM ", 10)) {
    unsigned int pid;
    int x, y, w, h;
    if(5 == sscanf(cmd, "SUBSTREAM 0x%x %d %d %d %d", &pid, &x, &y, &w, &h)) {
      char mrl[256];
      if(!posts->pip_stream)
	posts->pip_stream = xine_stream_new(this->xine, 
					    this->audio_port, 
					    this->video_port);
      LOGMSG("  PIP %d: %dx%d @ (%d,%d)", pid & 0x0f, w, h, x, y);
      LOGMSG("create pip stream done");
      sprintf(mrl, "xvdr:slave://0x%lx#nocache;demux:mpeg_block",
	      (unsigned long int)this);
      if(!xine_open(posts->pip_stream, mrl) ||
	 !xine_play(posts->pip_stream, 0, 0)) {
	LOGERR("  pip stream open/play failed");
      } else {
	char params[256];
	sprintf(params, "pip_num=1,x=%d,y=%d,w=%d,h=%d", x,y,w,h);
	fe_post_open(this, "Pip", params);
	return posts->pip_stream;
      }
    }
    fe_post_close(this, NULL, POST_VIDEO_PIP);
    if(posts->pip_stream) {
      xine_close(posts->pip_stream);
      xine_dispose(posts->pip_stream);
      posts->pip_stream = NULL;
    }
    return NULL;
    
  } else if(!strncmp(cmd, "POST ", 5)) {
    char *name = strdup(cmd+5), *args = name, *pt;

    if(NULL != (pt=strchr(name, '\r')))
      *pt = 0;
    if(NULL != (pt=strchr(name, '\n')))
      *pt = 0;

    while(*args && *args != ' ') /* skip name */
      args++;
    if(*args /*== ' '*/)
      *args++ = 0;

    while(*args && *args == ' ')  /* skip whitespace between name and args */
      args++;

    if(!strncmp(args, "On", 2)) {
      args += 2;
      while(*args == ' ') 
	args++;
      /*LOGDBG("  POST: %s On \"%s\"", name, args);*/
      fe_post_open(this, name, args);
    } else if(!strncmp(args, "Off", 3)) {
      /*LOGDBG("  POST: %s Off (name len=%d), name => int = %d", name, strlen(name), atoi(name));*/
      if(strlen(name) == 1)
	fe_post_close(this, NULL, atoi(name));
      else
	fe_post_close(this, name, -1);
    } else {
      LOGMSG("fe_control: POST: unknown command %s", cmd);
    }
    free(name);
    return NULL;

  } else if(!strncmp(cmd, "GRAB ", 5)) {
    int quality, width, height, jpeg, size=0;
    jpeg = !strncmp(cmd+5,"JPEG",4);
    if(3 == sscanf(cmd+5+4, "%d %d %d", &quality, &width, &height)) {
      grab_data_t *result = (grab_data_t*)malloc(sizeof(grab_data_t));   
      result->data = this->fe.grab((frontend_t*)this, &size, 
				   jpeg, quality, width, height);
      if(result->data && (result->size=size)>0)
	return result;
      free(result->data);
      free(result);
    }

  } else if(!strncmp(cmd, "OVERSCAN ", 9)) {
    int overscan;
    if(1 == sscanf(cmd+9, "%d", &overscan)) {
      int crop_x = overscan * this->width  / 100 / 2;
      int crop_y = overscan * this->height / 100 / 2;
      this->overscan = overscan;
      xine_set_param(this->stream, XINE_PARAM_VO_CROP_LEFT,   crop_x);
      xine_set_param(this->stream, XINE_PARAM_VO_CROP_TOP,    crop_y);
      xine_set_param(this->stream, XINE_PARAM_VO_CROP_RIGHT,  crop_x);
      xine_set_param(this->stream, XINE_PARAM_VO_CROP_BOTTOM, crop_y);
      /* trigger forced redraw to make changes effective */
      xine_set_param(this->stream, XINE_PARAM_VO_ZOOM_X, 100);      
    }
  }
  

  return NULL;
}

/*
 * --- RgbToJpeg -------------------------------------------------------------
 *
 *     source: vdr-1.3.42, tools.c
 *     modified to accept YUV data
 *
 *  TODO: remote version: send to ctrl stream
 *        - move to xine_input_vdr ?
 */

#define JPEGCOMPRESSMEM 500000

typedef struct tJpegCompressData_s {
  int size;
  unsigned char *mem;
} tJpegCompressData;

static void JpegCompressInitDestination(j_compress_ptr cinfo)
{
  tJpegCompressData *jcd = (tJpegCompressData *)cinfo->client_data;
  if (jcd) {
     cinfo->dest->free_in_buffer = jcd->size = JPEGCOMPRESSMEM;
     cinfo->dest->next_output_byte = jcd->mem = 
       (unsigned char *)malloc(jcd->size);
     }
}

static boolean JpegCompressEmptyOutputBuffer(j_compress_ptr cinfo)
{
  tJpegCompressData *jcd = (tJpegCompressData *)cinfo->client_data;
  if (jcd) {
     int Used = jcd->size;
     jcd->size += JPEGCOMPRESSMEM;
     jcd->mem = (unsigned char *)realloc(jcd->mem, jcd->size);
     if (jcd->mem) {
        cinfo->dest->next_output_byte = jcd->mem + Used;
        cinfo->dest->free_in_buffer = jcd->size - Used;
        return TRUE;
        }
     }
  return FALSE;
}

static void JpegCompressTermDestination(j_compress_ptr cinfo)
{
  tJpegCompressData *jcd = (tJpegCompressData *)cinfo->client_data;
  if (jcd) {
     int Used = cinfo->dest->next_output_byte - jcd->mem;
     if (Used < jcd->size) {
        jcd->size = Used;
        jcd->mem = (unsigned char *)realloc(jcd->mem, jcd->size);
        }
     }
}

static char *fe_grab(frontend_t *this_gen, int *size, int jpeg, 
		     int quality, int width, int height)
{
  struct jpeg_destination_mgr jdm;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  tJpegCompressData jcd;

  fe_t *this = (fe_t*)this_gen;
  vo_frame_t *frame;

#ifndef PPM_SUPPORTED
  if(!jpeg) {
    LOGMSG("fe_grab: PPM grab not implemented");
    return 0;
  }
#else
  /* #warning TODO: convert to RGB PPM */
#endif

  if(!this->input && !find_input(this))
    return 0;

  LOGDBG("fe_grab: grabbing %s %d %dx%d", 
	 jpeg ? "JPEG" : "PNM", quality, width, height);

  if (quality < 0 || quality > 100)
    quality = 100; /* -1 defaults to 100 */

  this->stream->xine->port_ticket->acquire(this->stream->xine->port_ticket, 0);
  frame = this->stream->video_out->get_last_frame (this->stream->video_out);
  if(frame)
    frame->lock(frame);
  this->stream->xine->port_ticket->release(this->stream->xine->port_ticket, 0);

  if(!frame)
    return NULL;

  /* #warning TODO: no scaling implemented */
  if(width != frame->width)
    width = frame->width;
  if(height != frame->height)
    height = frame->height;

  /* Compress JPEG */

  jdm.init_destination = JpegCompressInitDestination;
  jdm.empty_output_buffer = JpegCompressEmptyOutputBuffer;
  jdm.term_destination = JpegCompressTermDestination;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  cinfo.dest = &jdm;

  cinfo.client_data = &jcd;
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  /*cinfo.in_color_space = JCS_RGB;*/
  cinfo.in_color_space = JCS_YCbCr;

  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE);

  switch (frame->format) {
    case XINE_IMGFMT_YV12: {
      JSAMPARRAY pp[3];
      JSAMPROW *rpY = (JSAMPROW*)malloc(sizeof(JSAMPROW) * height);
      JSAMPROW *rpU = (JSAMPROW*)malloc(sizeof(JSAMPROW) * height);
      JSAMPROW *rpV = (JSAMPROW*)malloc(sizeof(JSAMPROW) * height);
      int k;
      cinfo.raw_data_in = TRUE;
      jpeg_set_colorspace(&cinfo, JCS_YCbCr);
      cinfo.comp_info[0].h_samp_factor =
      cinfo.comp_info[0].v_samp_factor = 2;
      cinfo.comp_info[1].h_samp_factor =
      cinfo.comp_info[1].v_samp_factor =
      cinfo.comp_info[2].h_samp_factor =
      cinfo.comp_info[2].v_samp_factor = 1;
      jpeg_start_compress(&cinfo, TRUE);
      
      for (k = 0; k < height; k+=2) {
	rpY[k]   = frame->base[0] + k*frame->pitches[0];
	rpY[k+1] = frame->base[0] + (k+1)*frame->pitches[0];
	rpU[k/2] = frame->base[1] + (k/2)*frame->pitches[1];
	rpV[k/2] = frame->base[2] + (k/2)*frame->pitches[2];
      }
      for (k = 0; k < height; k+=2*DCTSIZE) {
	pp[0] = &rpY[k];
	pp[1] = &rpU[k/2];
	pp[2] = &rpV[k/2];
	jpeg_write_raw_data(&cinfo, pp, 2*DCTSIZE);
      }
      free(rpY);
      free(rpU);
      free(rpV);
      break;
    }
    case XINE_IMGFMT_YUY2: {
      JSAMPROW *rp = (JSAMPROW*)malloc(sizeof(JSAMPROW) * height);
      int rs, k;
      jpeg_start_compress(&cinfo, TRUE);
      rs = frame->pitches[0];
      for (k = 0; k < height; k++)
	rp[k] = frame->base[0] + k*rs;
      jpeg_write_scanlines(&cinfo, rp, height);
      free(rp);
      break;
    }
#if 0
    case XINE_IMGFMT_RGB: {
      JSAMPROW rp[height];
      int rs, k;
      cinfo.in_color_space = JCS_RGB;
      jpeg_start_compress(&cinfo, TRUE);
      rs = frame->pitches[0];
      for (k = 0; k < height; k++)
	rp[k] = frame->base[0] + k*rs;
      jpeg_write_scanlines(&cinfo, rp, height);
      break;
    }
#endif
    default:
      LOGMSG("fe_grab: grabbing failed (unsupported image format %d)", 
	     frame->format);
      break;
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  
  frame->free(frame);

  *size = jcd.size;
  return (char*) jcd.mem;
}


#ifdef FE_STANDALONE

/* VDR discovery protocol code */
#include "xine_frontend_vdrdiscovery.c"
/* LIRC forwarding code */
#include "xine_frontend_lirc.c"
/* frontend main() */
#include "xine_frontend_main.c"


#endif /* #ifdef FE_STANDALONE */

