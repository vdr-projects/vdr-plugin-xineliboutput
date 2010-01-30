/*
 * xvdr_metronom.h:
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef XVDR_METRONOM_H
#define XVDR_METRONOM_H


#define XVDR_METRONOM_LAST_VO_PTS  0x1001


typedef struct xvdr_metronom_s xvdr_metronom_t;

struct xvdr_metronom_s {
  /* xine-lib metronom interface */
  metronom_t     metronom;

  /* management interface */
  void (*set_cb)      (xvdr_metronom_t *,
                       void (*cb) (void *, uint, uint),
                       void *);
  void (*reset_frames)(xvdr_metronom_t *);
  void (*dispose)     (xvdr_metronom_t *);

  void (*set_trickspeed)(xvdr_metronom_t *, int);

  /* accumulated frame data */
  volatile uint video_frames;
  volatile uint audio_frames;

  /* private data */

#ifdef XVDR_METRONOM_COMPILE

  /* original metronom */
  metronom_t    *orig_metronom;
  xine_stream_t *stream;

  /* callback */
  void *handle;
  void (*frame_decoded)(void *handle, uint video_count, uint audio_count);

  int     trickspeed;    /* current trick speed */
  int64_t last_vo_pts;   /* last displayed video frame PTS */
#endif
};

xvdr_metronom_t *xvdr_metronom_init(xine_stream_t *);


#endif /* XVDR_METRONOM_H */
