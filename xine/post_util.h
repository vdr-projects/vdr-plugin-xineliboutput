/*
 * post_util.h: post plugin utility functions
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#if POST_PLUGIN_IFACE_VERSION < 9
# warning  POST_PLUGIN_IFACE_VERSION < 9 not supported !
#endif
#if POST_PLUGIN_IFACE_VERSION > 10
# warning  POST_PLUGIN_IFACE_VERSION > 10 not supported !
#endif

/*
 * class util prototypes
 */

static void *init_plugin(xine_t *xine, const void *data);
#if POST_PLUGIN_IFACE_VERSION < 10
static char *get_identifier(post_class_t *class_gen);
static char *get_description(post_class_t *class_gen);
#endif
#if XINE_VERSION_CODE <= 10209
static void  class_dispose(post_class_t *class_gen);
#endif

/* required from plugin: */
static post_plugin_t *open_plugin(post_class_t *class_gen, int inputs,
                                  xine_audio_port_t **audio_target,
                                  xine_video_port_t **video_target);

/*
 * plugin util prototypes
 */

static int   dispatch_draw(vo_frame_t *frame, xine_stream_t *stream);
static int   intercept_frame_yuy(post_video_port_t *port, vo_frame_t *frame);
static int   post_draw(vo_frame_t *frame, xine_stream_t *stream);
#ifdef ENABLE_SLICED
static void  dispatch_slice(vo_frame_t *vo_img, uint8_t **src);
#endif

/* required from plugin: */
static vo_frame_t    *got_frame(vo_frame_t *frame);
static void           draw_internal(vo_frame_t *frame, vo_frame_t *new_frame);


/*
 * class utils
 */

static void *init_plugin(xine_t *xine, const void *data)
{
  static const post_class_t post_class = {
    .open_plugin     = open_plugin,
#if POST_PLUGIN_IFACE_VERSION < 10
    .get_identifier  = get_identifier,
    .get_description = get_description,
#else
    .identifier      = PLUGIN_ID,
    .description     = PLUGIN_DESCR,
#endif
#if XINE_VERSION_CODE <= 10209
    .dispose         = class_dispose,
#else
    .dispose         = NULL,
#endif
  };
  return (void *)&post_class;
}

#if POST_PLUGIN_IFACE_VERSION < 10
static char *get_identifier(post_class_t *class_gen)
{
  return PLUGIN_ID;
}

static char *get_description(post_class_t *class_gen)
{
  return PLUGIN_DESCR;
}
#endif
#if XINE_VERSION_CODE <= 10209
static void class_dispose(post_class_t *class_gen)
{
  (void)class_gen;
}
#endif

/*
 * plugin utils
 */

#ifdef ENABLE_SLICED
static void dispatch_slice(vo_frame_t *vo_img, uint8_t **src)
{
  if (vo_img->next->proc_slice) {
    _x_post_frame_copy_down(vo_img, vo_img->next);
    vo_img->next->proc_slice(vo_img->next, src);
    _x_post_frame_copy_up(vo_img, vo_img->next);
  }
}
#endif

static int dispatch_draw(vo_frame_t *frame, xine_stream_t *stream)
{
  int skip;
  _x_post_frame_copy_down(frame, frame->next);
  skip = frame->next->draw(frame->next, stream);
  _x_post_frame_copy_up(frame, frame->next);
  return skip;
}

static int intercept_frame_yuy(post_video_port_t *port, vo_frame_t *frame)
{
  return (frame->format == XINE_IMGFMT_YV12 || frame->format == XINE_IMGFMT_YUY2);
}

static int post_draw(vo_frame_t *frame, xine_stream_t *stream)
{
  vo_frame_t *new_frame;
  int skip;

  if (frame->bad_frame)
    return dispatch_draw(frame, stream);

  new_frame = got_frame(frame);

  if (!new_frame)
    return dispatch_draw(frame, stream);

  _x_post_frame_copy_down(frame, new_frame);

  draw_internal(frame, new_frame);

  skip = new_frame->draw(new_frame, stream);
  _x_post_frame_copy_up(frame, new_frame);
  new_frame->free(new_frame);

  return skip;
}

