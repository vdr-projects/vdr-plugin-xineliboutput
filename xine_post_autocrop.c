/*
 * Copyright (C) 2006 the xine project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * $Id$
 *
 * autocrop video filter by Petri Hintukainen 25/03/2006
 *
 * Automatically crop 4:3 letterbox frames to 16:9
 * 
 * based on expand.c
 *
 *
 * TODO: 
 *  - more reliable border detection, including channel logo detection
 *  - OSD re-positioning (?)
 *
 */


#ifdef HAVE_CONFIG_H 
#include "config.h"     /* ARCH_X86 */
#endif

#include <stdint.h>

#include <xine/xine_internal.h>
#include <xine/post.h>

/*
 *  Configuration
 */

/*#define USE_CROP         / * Crop frame in video_out instead of copying */
/*#define MARK_FRAME       Draw markers on detected boundaries */
/*#define ENABLE_64BIT 1   Force using of 64-bit routines */
/*#undef __MMX__           Disable MMX */
/*#undef __SSE__           Disable SSE */
/*#define FILTER2          Tighter Y-filter */

# if defined(__SSE__)
#  warning Compiling with SSE support
#  include <xmmintrin.h>
# elif defined(__MMX__)
#  warning Compiling with MMX support
#  include <mmintrin.h>
# endif

#if defined(__WORDSIZE)
#  if __WORDSIZE == 64
#    warning Compiling with 64-bit integer support
#    define ENABLE_64BIT (sizeof(int) > 32)
#  endif
#endif

/*#define TRACE       printf*/
#define TRACE(x...)   do {} while(0)
#define INFO          printf


/*
 * Constants
 */ 

#define YNOISEFILTER    (0xE0U)
#define YSHIFTUP        (0x05U)
#define UVBLACK         (0x80U)
#define UVSHIFTUP       (0x03U)
#define UVNOISEFILTER   (0xF8U)

/* YV12 */
#define YNOISEFILTER32  (YNOISEFILTER  * 0x01010101U)
#define YSHIFTUP32      (YSHIFTUP      * 0x01010101U)
#define UVBLACK32       (UVBLACK       * 0x01010101U)
#define UVSHIFTUP32     (UVSHIFTUP     * 0x01010101U)
#define UVNOISEFILTER32 (UVNOISEFILTER * 0x01010101U)

#define YNOISEFILTER64  (YNOISEFILTER  * UINT64_C(0x0101010101010101))
#define YSHIFTUP64      (YSHIFTUP      * UINT64_C(0x0101010101010101))
#define UVBLACK64       (UVBLACK       * UINT64_C(0x0101010101010101))
#define UVSHIFTUP64     (UVSHIFTUP     * UINT64_C(0x0101010101010101))
#define UVNOISEFILTER64 (UVNOISEFILTER * UINT64_C(0x0101010101010101))

/* YUY2 */
/* TODO: should use normal/inverse order based on endianess */
#if 0
#define YUY2BLACK32       (UVBLACK       * 0x00010001U)
#define YUY2SHIFTUP32     (UVSHIFTUP     * 0x00010001U)
#define YUY2NOISEFILTER32 ((YNOISEFILTER * 0x01000100U)|(UVNOISEFILTER * 0x00010001U))
#else
#define YUY2BLACK32       (UVBLACK       * 0x01000100U)
#define YUY2SHIFTUP32     (UVSHIFTUP     * 0x01000100U)
#define YUY2NOISEFILTER32 ((YNOISEFILTER * 0x00010001U)|(UVNOISEFILTER * 0x01000100U))
#endif

#define YUY2BLACK64       (YUY2BLACK32       * UINT64_C(0x0000000100000001))
#define YUY2SHIFTUP64     (YUY2SHIFTUP32     * UINT64_C(0x0000000100000001))
#define YUY2NOISEFILTER64 (YUY2NOISEFILTER32 * UINT64_C(0x0000000100000001))

#ifdef FILTER2
/* tighter Y-filter: original black threshold is 0x1f ; here it is 0x1f - 0x0b = 0x14 */
# define YUY2SHIFTUP32  ((UVSHIFTUP    * 0x00010001U)|(YSHIFTUP      * 0x01000100U))
# undef __SSE__
#endif


#define START_TIMER_INIT         (25) /* 1 second, unit: frames */
#define HEIGHT_LIMIT_LIFETIME (60*25) /* 1 minute, unit: frames */

#define LOGOSKIP (frame->width/4)  /* skip logo (Y, top-left or top-right quarter) */

/*
 * Plugin
 */

typedef struct autocrop_parameters_s {
  int    enable_autodetect;
  int    enable_subs_detect;
  int    soft_start;
  int    stabilize;
} autocrop_parameters_t;

START_PARAM_DESCR(autocrop_parameters_t)
PARAM_ITEM(POST_PARAM_TYPE_BOOL, enable_autodetect, NULL, 0, 1, 0,
  "enable automatic border detecton")
PARAM_ITEM(POST_PARAM_TYPE_BOOL, enable_subs_detect, NULL, 0, 1, 0,
  "enable automatic subtitle detecton")
PARAM_ITEM(POST_PARAM_TYPE_BOOL, soft_start, NULL, 0, 1, 0,
  "enable soft start of cropping")
PARAM_ITEM(POST_PARAM_TYPE_BOOL, stabilize, NULL, 0, 1, 0,
  "stabilize cropping to 14:9, 16:9, (16:9+subs), 20:9, (20:9+subs)")
END_PARAM_DESCR(autocrop_param_descr)


typedef struct autocrop_post_plugin_s
{
  post_plugin_t  post_plugin;

  xine_post_in_t parameter_input;

  /* setup */
  int autodetect;
  int subs_detect;
  int soft_start;
  int stabilize;

  /* Current cropping status */
  int cropping_active; 

  /* Detected bars */
  int start_line;       
  int end_line;
  int crop_total;

  /* Previously detected bars
     - eliminate jumping if there is some noise at bar boundaries: 
       don't change cropped area unless it has been stable for
       some time */
  int prev_start_line;
  int prev_end_line;

  /* Delayed start for cropping */
  int start_timer;
  int stabilize_timer;

  /* Last seen frame */
  int     prev_height;
  int     prev_width;
  int64_t prev_pts;

  /* eliminate jumping when when there are subtitles inside bottom bar:
     - when cropping is active and one frame has larger end_line 
       than previous, we enlarge frame.
     - after this, cropping is not resetted to previous value unless 
       bottom bar has been empty for certain time */
  int height_limit_active;  /* true if detected possible subtitles in bottom area */
  int height_limit;         /* do not crop bottom above this value (bottom of subtitles) */
  int height_limit_timer;   /* counter how many following frames must have black 
			       bottom bar until returning to full cropping
			       (used to reset height_limit when there are no subtitles) */
} autocrop_post_plugin_t;


/*
 * Black bar detection
 *
 *  Detect black lines with simple noise filtering.
 *  Line is "black" if Y-valus are less than 0x20 and 
 *  U/V values inside range 0x7d...0x84.
 *  ~ 32 first and last pixels are not checked.
 *
 */

static int blank_line_Y_C(uint8_t *data, int length);
static int blank_line_UV_C(uint8_t *data, int length);
static int blank_line_YUY2_C(uint8_t *data, int length);
#if defined(ENABLE_64BIT)
static int blank_line_Y_C64(uint8_t *data, int length);
static int blank_line_UV_C64(uint8_t *data, int length);
static int blank_line_YUY2_C64(uint8_t *data, int length);
#endif
#if defined(__MMX__)
static int blank_line_Y_mmx(uint8_t *data, int length);
static int blank_line_UV_mmx(uint8_t *data, int length);
static int blank_line_YUY2_mmx(uint8_t *data, int length);
#endif
#if defined(__SSE__)
static int blank_line_Y_sse(uint8_t *data, int length);
static int blank_line_UV_sse(uint8_t *data, int length);
static int blank_line_YUY2_sse(uint8_t *data, int length);
#endif

static int blank_line_Y_INIT(uint8_t *data, int length);
static int blank_line_UV_INIT(uint8_t *data, int length);
static int blank_line_YUY2_INIT(uint8_t *data, int length);

static void autocrop_init_mm_accel(void);

int (*blank_line_Y)(uint8_t *data, int length)  = blank_line_Y_INIT;
int (*blank_line_UV)(uint8_t *data, int length) = blank_line_UV_INIT;
int (*blank_line_YUY2)(uint8_t *data, int length) = blank_line_YUY2_INIT;

static int blank_line_Y_C(uint8_t *data, int length)
{
  uint32_t *data32 = (uint32_t*)((((long int)data) + 32 + 3) & (~3)), r = 0;

  length -= 64; /* skip borders (2 x 32 pixels) */
  length /= 4;  /* 4 bytes / loop */

#ifdef FILTER2
  while(length) {
    /* shiftdown needs saturated unsigned element-wise substraction, available only in MMX ...*/
    /* -> use shiftup and looser noise filter for same result. */
    /*    this needs special handling for large values */
    r = r | data32[--length]; /* this catches large values (0xf9 : 0xf9+0x7=0x100 === black) */
    r = r | (data32[length] + YSHIFTUP32); /* this catches small walues (0x1d...0x1f) */
  }
#else
  while(length)
    r = r | data32[--length];
#endif

  return !(r & YNOISEFILTER32);
}

static int blank_line_UV_C(uint8_t *data, int length)
{
  uint32_t *data32 = (uint32_t*)((((long int)data) + 16 + 3) & (~3));
  uint32_t r1 = 0, r2 = 0;

  length -= 32; /* skip borders (2 x 32 pixels, 2 pix/byte) */
  length /= 4;  /* 2 x 4 bytes / loop */

  while(length>0) {
    r1 = r1 | ((data32[--length] + UVSHIFTUP32) ^ UVBLACK32);
    r2 = r2 | ((data32[--length] + UVSHIFTUP32) ^ UVBLACK32);
  }
  return !((r1|r2) & (UVNOISEFILTER32));
}

#if defined(ENABLE_64BIT)
static int blank_line_Y_C64(uint8_t *data, int length)
{
  uint64_t *data64 = (uint64_t*)((((long int)data) + 32 + 7) & (~7)), r = 0;

  length -= 64; /* skip borders (2 x 32 pixels) */
  length /= 8;  /* 8 bytes / loop */

#ifdef FILTER2
  while(length) {
    r = r | data64[--length];
    r = r | (data64[length] + YSHIFTUP64);
  }
#else
  while(length)
    r = r | data64[--length];
#endif

  return !(r & YNOISEFILTER64);
}
#endif

#if defined(ENABLE_64BIT)
static int blank_line_UV_C64(uint8_t *data, int length)
{
  uint64_t *data64 = (uint64_t*)((((long int)data) + 16 + 7) & (~7));
  uint64_t r1 = UINT64_C(0), r2 = UINT64_C(0);

  length -= 32; /* skip borders (2x32 pixels, 2 pix/byte) */
  length /= 8;  /* 2 x 8 bytes / loop */

  while(length>0) {
    r1 = r1 | ((data64[--length] + UVSHIFTUP64) ^ UVBLACK64);
    r2 = r2 | ((data64[--length] + UVSHIFTUP64) ^ UVBLACK64);
  }
  return !((r1|r2) & (UVNOISEFILTER64));
}
#endif


#if defined(__MMX__)
typedef union {
  uint32_t u32[2];
  __m64    m64;
} __attribute__((__aligned__ (8))) __m64_wrapper;
#endif

#if defined(__MMX__)
 int blank_line_Y_mmx(uint8_t *data, int length)
{
#ifdef FILTER2
  static const __m64_wrapper mask   = {{YNOISEFILTER32, YNOISEFILTER32}};
  static const __m64_wrapper gshift = {{YSHIFTUP32,     YSHIFTUP32}};
  register __m64 sum, sum2, shift = gshift.m64, val;
#else
  static const __m64_wrapper mask  = {{YNOISEFILTER32, YNOISEFILTER32}};
  register __m64 sum;
#endif
  __m64 *data64 = (__m64*)(((long int)(data + 32 + 7)) & (~7));

  /*sum = _m_pxor(sum,sum);*/
  __asm__("pxor %0,%0" : "=y"(sum));

  length -= 64; /* skip borders (2 x 32 pixels) */
  length /= 8;  /* 8 bytes / loop */

#ifdef FILTER2
  __asm__("pxor %0,%0" : "=y"(sum2));
  while(length) {
    val  = data64[--length];
    sum  = _m_por(sum, val);
    sum2 = _m_por(sum2, _m_paddb(val, shift));
  }
  sum = _m_por(sum, sum2);
#else
  while(length)
    sum = _m_por(sum, data64[--length]);
#endif

  sum = _m_pand(sum, mask.m64);
  return 0 == _m_to_int(_m_packsswb(sum, sum));
}
#endif

#if defined(__MMX__)
static int blank_line_UV_mmx(uint8_t *data, int length)
{
  static const __m64_wrapper gm_03 = {{UVSHIFTUP32,     UVSHIFTUP32}};
  static const __m64_wrapper gm_f8 = {{UVNOISEFILTER32, UVNOISEFILTER32}};
  static const __m64_wrapper gm_80 = {{UVBLACK32,       UVBLACK32}};
  __m64 *data64 = (__m64*)(((long int)(data) + 16 + 7) & (~7));
  register __m64 sum1, sum2, m_03, /*m_f8,*/ m_80;

  /*sum1 = _m_pxor(sum1, sum1); sum1 = _mm_setzero_si64(); */
  /*sum2 = _m_pxor(sum2, sum2); sum2 = _mm_setzero_si64(); */
  __asm__("pxor %0,%0" : "=y"(sum1));
  __asm__("pxor %0,%0" : "=y"(sum2));

  m_03 = gm_03.m64;
  /*m_f8 = gm_f8.m64;*/
  m_80 = gm_80.m64;

  length -= 32; /* skip borders (2 x 32 pixels, 2pix/byte) */
  length /= 8;  /* 8 bytes / vector */

  do {
    /* process two 8-byte vectors */
    sum1 = _m_por(sum1,
		  /* grab every byte that is not black (x ^ 0x80 != 0) */
		  _m_pxor(
			  /* filter noise: U/V of each "black" pixel should be 0x7d..0x84 
			     -> each black pixel should be 0x80 after (x+3) & 0xf8 */
			  /*_m_pand(*/
				  /* each black pixel should be 0x80..0x87 after adding 3 */
				  _m_paddb(
					     data64[length-1], 
					     m_03)/*,
				  m_f8)*/,
			  m_80));
    sum2 = _m_por(sum2, 
		  _m_pxor( 
			  /*_m_pand( */
				  _m_paddb( 
					     data64[length-2], 
					     m_03)/*, 
				  m_f8)*/,
			    m_80));
    length -= 2;
  } while(length>0);
  
  /* combine two result vectors (or), filter noise (and) */
  sum1 = _m_pand(_m_por(sum1,
			sum2), 
		 gm_f8.m64);
  /* result vector of black line is 0 */
  return 0 == _m_to_int(_m_packsswb(sum1, sum1));
}
#endif

#if defined(__SSE__)
typedef  union {
  uint32_t u32[4];
  __m128   m128;
} __attribute((__aligned__ (16))) __m128_wrapper;
#endif

#if defined(__SSE__)
static int blank_line_Y_sse(uint8_t *data, int length)
{
  static const __m128_wrapper gmask = {{YNOISEFILTER32, YNOISEFILTER32,
					YNOISEFILTER32, YNOISEFILTER32}};
  __m128 *data128 = (__m128*)(((long int)(data) + 32 + 15) & (~15));
  register __m128 sum1, sum2, zero, mask;

  length -= 64; /* skip borders (2 x 32 pixels) */
  length /= 16; /* 16 bytes / loop */

  _mm_prefetch(data128+length-1, _MM_HINT_NTA);
  _mm_prefetch(data128+length-3, _MM_HINT_NTA);

  /* 
   * Process in two paraller loops, one 16 byte vector / each sub-loop 
   * - grabs bytes with value larger than treshold
   */

  zero = _mm_setzero_ps();
  mask = gmask.m128;
  sum1 = zero;
  sum2 = zero;

  do {
    _mm_prefetch(data128+length-5, _MM_HINT_NTA);
    sum1 = _mm_or_ps(sum1, data128[--length]);
    sum2 = _mm_or_ps(sum2, data128[--length]);
  } while(length>0);

  return 0x0f == _mm_movemask_ps(_mm_cmpeq_ps(_mm_and_ps(_mm_or_ps(sum1, 
								   sum2), 
							 gmask.m128), 
					      _mm_setzero_ps()));
}
#endif

#if defined(__SSE__)
static int blank_line_UV_sse(uint8_t *data, int length)
{
  uint8_t *top = data + length - 1;
  do {
    _mm_prefetch(top,    _MM_HINT_NTA);
    _mm_prefetch(top-32, _MM_HINT_NTA);
    _mm_prefetch(top-64, _MM_HINT_NTA);
    _mm_prefetch(top-72, _MM_HINT_NTA);
    top -= 128;
  } while(top >= data);

  return blank_line_UV_mmx(data, length);
}
#endif

static int blank_line_YUY2_C(uint8_t *data, int length)
{
  uint32_t *data32 = (uint32_t*)((((long int)data) + 64 + 3) & (~3));
  uint32_t r1 = 0, r2 = 0;

  length -= 128; /* skip borders (2 x 32 pixels, 2 bytes/pixel) */
  length /= 4;   /* 2 x 4 bytes / loop */

  while(length) {
    r1 = r1 | ((data32[--length] + YUY2SHIFTUP32) ^ YUY2BLACK32);
    r2 = r2 | ((data32[--length] + YUY2SHIFTUP32) ^ YUY2BLACK32);
  }
  return !((r1|r2) & YUY2NOISEFILTER32);
}

#if defined(ENABLE_64BIT)
static int blank_line_YUY2_C64(uint8_t *data, int length)
{
  uint64_t *data64 = (uint32_t*)((((long int)data) + 64 + 7) & (~7));
  uint64_t r1 = 0, r2 = 0;

  length -= 128; /* skip borders (2 x 32 pixels, 2 bytes/pixel) */
  length /= 8;   /* 2 x 8 bytes / loop */

  while(length) {
    r1 = r1 | ((data64[--length] + YUY2SHIFTUP64) ^ YUY2BLACK64);
    r2 = r2 | ((data64[--length] + YUY2SHIFTUP64) ^ YUY2BLACK64);
  }
  return !((r1|r2) & YUY2NOISEFILTER64);
}
#endif

#if defined(__MMX__)
static int blank_line_YUY2_mmx(uint8_t *data, int length)
{
  /* not implemented */

# if !defined(ENABLE_64BIT)
  return blank_line_YUY2_C(data, length);
# else
  return blank_line_YUY2_C64(data, length);
# endif
}
#endif

#if defined(__SSE__)
static int blank_line_YUY2_sse(uint8_t *data, int length)
{
  uint8_t *top = data + length - 1;
  do {
    _mm_prefetch(top,    _MM_HINT_NTA);
    _mm_prefetch(top-32, _MM_HINT_NTA);
    _mm_prefetch(top-64, _MM_HINT_NTA);
    _mm_prefetch(top-72, _MM_HINT_NTA);
    top -= 128;
  } while(top >= data);

  return blank_line_YUY2_mmx(data, length);
}
#endif

static void autocrop_init_mm_accel(void)
{
  blank_line_Y  = blank_line_Y_C;
  blank_line_UV = blank_line_UV_C;
  blank_line_YUY2 = blank_line_YUY2_C;

#if defined(__SSE__)
  if(xine_mm_accel() & MM_ACCEL_X86_SSE) {
    INFO("autocrop_init_mm_accel: using SSE\n");
    blank_line_Y  = blank_line_Y_sse;
    blank_line_UV = blank_line_UV_sse;
    blank_line_YUY2 = blank_line_YUY2_sse;
    return;
  }
# endif
#if defined(ENABLE_64BIT)
  if(ENABLE_64BIT) {
    INFO("autocrop_init_mm_accel: using 64-bit integer operations\n");
    blank_line_Y  = blank_line_Y_C64;
    blank_line_UV = blank_line_UV_C64;
    blank_line_YUY2 = blank_line_YUY2_C64;
    return;
  }
#endif
#if defined(__MMX__)
  if(xine_mm_accel() & MM_ACCEL_X86_MMX) {
    /* mmx not faster than normal x64 (?) */
    INFO("autocrop_init_mm_accel: using MMX\n");
    blank_line_Y  = blank_line_Y_mmx;
    blank_line_UV = blank_line_UV_mmx;
    blank_line_YUY2 = blank_line_YUY2_mmx;
    return;
  }
#endif
  INFO("autocrop_init_mm_accel: no compatible acceleration methods found\n");
}

static int blank_line_Y_INIT(uint8_t *data, int length)
{
  autocrop_init_mm_accel();
  return (*blank_line_Y)(data, length);
}

static int blank_line_UV_INIT(uint8_t *data, int length)
{
  autocrop_init_mm_accel();
  return (*blank_line_UV)(data, length);
}

static int blank_line_YUY2_INIT(uint8_t *data, int length)
{
  autocrop_init_mm_accel();
  return (*blank_line_YUY2)(data, length);
}

/*
 * Analyze frame
 *  - if frame needs cropping set crop_top & crop_bottom
 */

#ifdef MARK_FRAME
int dbg_top=0, dbg_bottom=0;
#endif

static int analyze_frame_yv12(vo_frame_t *frame, int *crop_top, int *crop_bottom)
{
  int y;
  int ypitch = frame->pitches[0];
  int upitch = frame->pitches[1];
  int vpitch = frame->pitches[2];
  uint8_t *ydata = frame->base[0];
  uint8_t *udata = frame->base[1];
  uint8_t *vdata = frame->base[2];
  int max_crop = (frame->height / 4) / 2; /* 4:3 --> 16:9 */

  /* from top -> down */
  ydata += 8 * ypitch;  /* skip 8 first lines */
  udata += 4 * upitch;
  vdata += 4 * vpitch;
  for(y = 8; y <= max_crop   *2 /* *2 = 20:9+subs -> 16:9 */ ; y += 2) {
    if(  ! ( blank_line_UV(udata,                (frame->width-LOGOSKIP)/2) ||
	     blank_line_UV(udata+LOGOSKIP/2,     (frame->width-LOGOSKIP)/2)    ) ||
	 ! ( blank_line_UV(vdata,                (frame->width-LOGOSKIP)/2) ||
	     blank_line_UV(vdata+LOGOSKIP/2,     (frame->width-LOGOSKIP)/2)    ) ||
	 ! ( blank_line_Y( ydata,                (frame->width-LOGOSKIP)  ) ||
	     blank_line_Y( ydata+LOGOSKIP,       (frame->width-LOGOSKIP)  )    ) ||
	 ! ( blank_line_Y( ydata+ypitch,         (frame->width-LOGOSKIP)  ) ||
	     blank_line_Y( ydata+ypitch+LOGOSKIP,(frame->width-LOGOSKIP)  )    )) {
      break;
    } else {
      ydata += 2 * ypitch;
      udata += upitch;
      vdata += vpitch;
    }
  }
  *crop_top = y>8 ? y : 0;

  /* from bottom -> up */
  ydata = frame->base[0] + ((frame->height-4)   -1 ) * ypitch;
  udata = frame->base[1] + ((frame->height-4)/2 -1 ) * upitch;
  vdata = frame->base[2] + ((frame->height-4)/2 -1 ) * vpitch;
  for(y = frame->height - 5; y >= frame->height-max_crop; y -=2 ) {
    if( ! blank_line_Y(ydata,        frame->width) ||
	! blank_line_Y(ydata-ypitch, frame->width) ||  
	! blank_line_UV(udata,       frame->width/2) ||
	! blank_line_UV(vdata,       frame->width/2)) {
      break;
    } else {
      ydata -= 2*ypitch;
      udata -= upitch;
      vdata -= vpitch;
    }
  }
  *crop_bottom = y;

  /* test for black in center - don't crop if frame is empty */
  if(*crop_top >= max_crop*2 && *crop_bottom <= frame->height-max_crop) {
    ydata = frame->base[0] + (frame->height/2)*ypitch;
    udata = frame->base[1] + (frame->height/4)*upitch;
    vdata = frame->base[2] + (frame->height/4)*vpitch;
    if( blank_line_Y(ydata,        frame->width) &&
	blank_line_Y(ydata-ypitch, frame->width) &&  
	blank_line_UV(udata,       frame->width/2) &&
	blank_line_UV(vdata,       frame->width/2)) {
      TRACE("not cropping black frame\n");
#if 0
      *crop_top = 0;
      *crop_bottom = frame->height - 1;
#else
      return 0;
#endif
    }
  }
  return 1;
}

static int analyze_frame_yuy2(vo_frame_t *frame, int *crop_top, int *crop_bottom)
{
  int y;
  int pitch = frame->pitches[0];
  uint8_t *data = frame->base[0];
  int max_crop = (frame->height / 4) / 2; /* 4:3 --> 16:9 */

  /* from top -> down */
  data += 6 * pitch;  /* skip 6 first lines */
  for(y = 6; y <= max_crop  *2 /* *2 = 20:9+subs -> 16:9 */ ; y ++)
    if(  ! ( blank_line_YUY2(data,            (frame->width-LOGOSKIP)*2) ||
	     blank_line_YUY2(data+2*LOGOSKIP, (frame->width-LOGOSKIP)*2))) 
      break;
    else 
      data += pitch;

  *crop_top = y;

  /* from bottom -> up */
  data = frame->base[0] + ((frame->height-4)   -1 ) * pitch;
  for(y = frame->height - 5; y >= frame->height-max_crop; y -- )
    if( ! blank_line_YUY2(data,  frame->width * 2))
      break;
    else 
      data -= pitch;
  
  *crop_bottom = y;
  
  /* test for black in center - don't crop if frame is empty */
  if(*crop_top >= max_crop*2 && *crop_bottom <= frame->height-max_crop) {
    data = frame->base[0] + (frame->height/2)*pitch;
    if( blank_line_YUY2(data, frame->width * 2)) {
      TRACE("not cropping black frame\n");
#if 0
      *crop_top = 0;
      *crop_bottom = frame->height - 1;
#else
      return 0;
#endif
    }
  }

  return 1;
}

static void analyze_frame(vo_frame_t *frame, int *crop_top, int *crop_bottom)
{
  post_video_port_t *port = (post_video_port_t *)frame->port;
  autocrop_post_plugin_t *this = (autocrop_post_plugin_t *)port->post;
  int result;

  if(frame->format == XINE_IMGFMT_YV12) 
    result = analyze_frame_yv12(frame, crop_top, crop_bottom);
  else /*if(frame->format == XINE_IMGFMT_YUY2)*/
    result = analyze_frame_yuy2(frame, crop_top, crop_bottom);

#if defined(__MMX__)
  _mm_empty();
#endif

  if(!result)
    return;

#ifdef MARK_FRAME
  dbg_top = *crop_top; dbg_bottom = *crop_bottom;
#endif

  if(this->stabilize) {
    int bottom = frame->height - *crop_bottom;
    int wide = 0;

    /* bottom bar size */
    if(bottom < frame->height/32) {
      TRACE("bottom: %d ->  4:3       ", *crop_bottom);
      *crop_bottom = frame->height - 1;  /* no cropping */
    } else if(bottom < frame->height*3/32) {
      TRACE("bottom: %d -> 14:9 (%d) ", *crop_bottom, frame->height * 15 / 16 - 1);
      *crop_bottom = frame->height * 15 / 16 - 1; /* 14:9 */
    } else if(bottom < frame->height*3/16) {
      TRACE("bottom: %d -> 16:9 (%d) ", *crop_bottom, frame->height * 7 / 8 - 1);
      *crop_bottom = frame->height * 7 / 8 - 1;   /* 16:9 */
      wide = 1;
    } else {
      TRACE("bottom: %d -> 20:9 (%d) ", *crop_bottom, frame->height * 3 / 4 - 1);
      *crop_bottom = frame->height * 3 / 4 - 1;   /* 20:9 */
      wide = 2;
    }

    /* top bar size */
    if(*crop_top < frame->height/32) {
      TRACE("top:    %3d ->  4:3      \n", *crop_top);
      *crop_top = 0;        /* no cropping */
    } else if(*crop_top < frame->height*3/32) {
      TRACE("top:    %3d -> 14:9 (%d)\n", *crop_top, frame->height / 16);
      *crop_top = frame->height / 16; /* 14:9 */
    } else if(*crop_top < frame->height*3/16 || wide) {
      TRACE("top:    %3d -> 16:9 (%d)\n", *crop_top, frame->height / 8);
      *crop_top = frame->height / 8;   /* 16:9 */
    } else { 
      TRACE("top:    %3d -> 20:9 (%d)\n", *crop_top, frame->height / 4);
      *crop_top = frame->height / 4;   /* 20:9 */
      wide++;
    }
    switch(wide) {
    case 3: *crop_top -= frame->height / 8;
            if(*crop_top < 0) 
	      *crop_top = 0;
            TRACE("        wide -> center top\n");
    case 2: *crop_bottom += frame->height / 8;
            if(*crop_bottom >= frame->height) 
	      *crop_bottom = frame->height-1;
            TRACE("        wide -> center bottom\n");
    }

  } else {

    if(*crop_top > (frame->height/8  *2))  /* *2 --> 20:9 -> 16:9 + subtitles */
      *crop_top = frame->height/8  *2 ;
    if(*crop_bottom < (frame->height*7/8))
      *crop_bottom = frame->height*7/8;

    if(*crop_top > (frame->height/8)) {
      /* if wider than 16:9, prefer cropping top if subtitles are inside bottom bar */
      if(*crop_top + (frame->height - *crop_bottom) > frame->height/4) {
	int diff = *crop_top + (frame->height - *crop_bottom) - frame->height/4;
	diff &= ~1;
	TRACE("balance: %d,%d -> %d,%d\n",
	      *crop_top, *crop_bottom, 
	      *crop_top, *crop_bottom + diff);
#if 0
        /* this moves image to top (crop only top) */ 
        *crop_bottom += diff;
#endif
#if 0
        /* this moves image to center */ 
	/* may cause problems with subtitles ... */
	*crop_top -= diff;
#endif
#if 1
        /* this moves image to center when there are no 
	   detected subtitles inside bottom bar */
	if(this->height_limit_active) {
	  int reserved = this->height_limit - *crop_bottom;
	  if(reserved>0) {
	    *crop_bottom += reserved;
	    diff -= reserved;
	  }
	}
	*crop_top -= diff;
#endif
#if 0
	/* do nothing - image will be centered in video out.
	   - problems with subtitles using unscaled OSD */
#endif
      }
    }

    /* stay inside frame and forget very small bars */
    if(*crop_top <= 8)
      *crop_top = 0;
    if(*crop_bottom >= (frame->height-6))
      *crop_bottom = frame->height;
 
    if(*crop_top < frame->height/12 || *crop_bottom > frame->height*11/12) {
      /* Small bars -> crop only detected borders */
     if(*crop_top || *crop_bottom < frame->height-1) {
	TRACE("Small bars -> <16:9 : start_line = %d end_line = %d (%s%d t%d)\n", 
	      *crop_top, *crop_bottom,
	      this->height_limit_active ? "height limit " : "",
	      this->height_limit,
	      this->height_limit_active ? this->height_limit_timer : 0);
      }
    } else {
      /* Large bars -> crop to 16:9 */
      TRACE("Large bars -> 16:9  : start_line = %d end_line = %d (%s%d t%d)\n", 
	    *crop_top, *crop_bottom, 
	    this->height_limit_active ? "height limit " : "",
	    this->height_limit,
	    this->height_limit_active ? this->height_limit_timer : 0);
      if(*crop_top < frame->height / 8)
	*crop_top = frame->height / 8;
      if(*crop_bottom < frame->height * 7 / 8)
	*crop_bottom = frame->height * 7 / 8;
    }
  }

  /* adjust start and stop to even lines */
  (*crop_top)    = (*crop_top)        & (~1);
  (*crop_bottom) = (*crop_bottom + 1) & (~1);
}

#ifdef MARK_FRAME
static void mark_frame_yv12(autocrop_post_plugin_t *this,
			    vo_frame_t *frame, int *crop_top, int *crop_bottom)
{
  int ypitch = frame->pitches[0];
  int upitch = frame->pitches[1];
  int vpitch = frame->pitches[2];
  uint8_t *ydata = frame->base[0];
  uint8_t *udata = frame->base[1];
  uint8_t *vdata = frame->base[2];

  /* draw markers to detected boundaries and expected boundaries */
  if(*crop_top > 4 && *crop_top < 200) {
    ydata = frame->base[0] + ((*crop_top)-2)*ypitch;
    udata = frame->base[1] + ((*crop_top)/2 -1)*upitch;
    memset(ydata, 0xff, frame->width/10);
    memset(ydata+ypitch, 0xff, frame->width/10);
    memset(udata, 0xff, frame->width/2/10);
    
    if(dbg_top < *crop_top) dbg_top = *crop_top;
    ydata = frame->base[0] + ((dbg_top - *crop_top))*ypitch;
    udata = frame->base[1] + ((dbg_top - *crop_top)/2)*upitch;
    memset(ydata, 0x80, frame->width/2);
    memset(ydata+ypitch, 0x80, frame->width/2);
    memset(udata, 0xff, frame->width/2/2);
  }
  if(*crop_bottom > 300) {
    if(*crop_bottom < frame->height - 2) {
      ydata = frame->base[0] + (*crop_bottom + 2)*ypitch;
      udata = frame->base[1] + ((*crop_bottom)/2)*upitch;
      memset(ydata, 0xff, frame->width);
      memset(ydata+ypitch, 0xff, frame->width);
      memset(udata, 0xff, frame->width/2);
    }
    if(dbg_bottom - *crop_top - 2 < frame->height) {
      ydata = frame->base[0] + (dbg_bottom - *crop_top - 2)*ypitch;
      udata = frame->base[1] + (dbg_bottom - *crop_top - 2)/2*upitch;
      memset(ydata, 0x80, frame->width/2);
      memset(ydata+ypitch, 0xff, frame->width/2);
      memset(udata, 0xff, frame->width/2/2);
    }
  }
  if(frame->height > 500) {
    /* TODO: use frame height instead of assuming 576 ... -> 72 */
    vdata = frame->base[2] + ((72-*crop_top)/2)*vpitch;
vdata = frame->base[2] + (72/2)*vpitch;
    memset(vdata, 0xff, frame->width/2);
    vdata = frame->base[2] + ((frame->height-72+(576-*crop_bottom))/2)*vpitch;
vdata = frame->base[2] + ((frame->height-72)/2)*vpitch;
    memset(vdata, 0xff, frame->width/2);
  }
}
#endif

/* 
 * crop frame by copying 
 */
#ifndef USE_CROP
static int crop_copy_yv12(vo_frame_t *frame, xine_stream_t *stream)
{
  post_video_port_t *port = (post_video_port_t *)frame->port;
  autocrop_post_plugin_t *this = (autocrop_post_plugin_t *)port->post;
  vo_frame_t *new_frame;
    
  int y, result;
  int yp = frame->pitches[0], yp2;
  int up = frame->pitches[1], up2;
  int vp = frame->pitches[2], vp2;
  uint8_t *ydata = frame->base[0], *ydata2;
  uint8_t *udata = frame->base[1], *udata2;
  uint8_t *vdata = frame->base[2], *vdata2;

  int   new_height;
  float new_ratio;

  /* top bar */
  ydata += this->start_line * yp;
  udata += (this->start_line/2) * up;
  vdata += (this->start_line/2) * vp;

  new_height = this->end_line - this->start_line;
  new_ratio  = 12.0/9.0 * ((float)frame->height / (float)new_height);

  new_frame = port->original_port->get_frame(port->original_port,
					     frame->width, new_height, 
					     new_ratio, frame->format, 
					     frame->flags | VO_BOTH_FIELDS);

  /* ??? */
  frame->ratio = new_frame->ratio;

  _x_post_frame_copy_down(frame, new_frame);

  yp2 = new_frame->pitches[0];
  up2 = new_frame->pitches[1];
  vp2 = new_frame->pitches[2];
  ydata2 = new_frame->base[0];
  udata2 = new_frame->base[1];
  vdata2 = new_frame->base[2];

  /*
    TODO:
    save channel logo mask (from top)
    - no changes in 3 sec -> next time crop it out ...
  */

  for(y=0; y < new_height/2; y++) {
    xine_fast_memcpy(ydata2, ydata, frame->width);
    ydata += yp;
    ydata2 += yp2;
    xine_fast_memcpy(ydata2, ydata, frame->width);
    ydata += yp;
    ydata2 += yp2;
    xine_fast_memcpy(udata2, udata, frame->width/2);
    udata += up;
    udata2 += up2;
    xine_fast_memcpy(vdata2, vdata, frame->width/2);
    vdata += vp;
    vdata2 += vp2;
  }

#ifdef MARK_FRAME
  mark_frame_yv12(this, new_frame, &this->start_line, &this->end_line);
#endif

  result = new_frame->draw(new_frame, stream);
  _x_post_frame_copy_up(frame, new_frame);
  new_frame->free(new_frame);
  
  return result;
}

static int crop_copy_yuy2(vo_frame_t *frame, xine_stream_t *stream)
{
  post_video_port_t *port = (post_video_port_t *)frame->port;
  autocrop_post_plugin_t *this = (autocrop_post_plugin_t *)port->post;
  vo_frame_t *new_frame;
    
  int y, result;
  int p = frame->pitches[0], p2;
  uint8_t *data = frame->base[0], *data2;

  int   new_height;
  float new_ratio;

  /* top bar */
  data += this->start_line * p;

  new_height = this->end_line - this->start_line;
  new_ratio = 12.0/9.0 * ((float)frame->height / (float)new_height);
  new_frame = port->original_port->get_frame(port->original_port,
					     frame->width, new_height, 
					     new_ratio, frame->format, 
					     frame->flags | VO_BOTH_FIELDS);
  /* ??? */
  frame->ratio = new_frame->ratio;

  _x_post_frame_copy_down(frame, new_frame);

  p2 = new_frame->pitches[0];
  data2 = new_frame->base[0];

  for(y=0; y < new_height; y++) {
    xine_fast_memcpy(data2, data, frame->width);
    data += p;
    data2 += p2;
  }

  result = new_frame->draw(new_frame, stream);
  _x_post_frame_copy_up(frame, new_frame);
  new_frame->free(new_frame);
  
  return result;
}

#endif

/*
 * crop frame without copying 
 */
#ifdef USE_CROP
static int crop_nocopy(vo_frame_t *frame, xine_stream_t *stream)
{
  post_video_port_t *port = (post_video_port_t *)frame->port;
  autocrop_post_plugin_t *this = (autocrop_post_plugin_t *)port->post;
  int skip;
  double new_ratio = frame->ratio;
  int new_height;

  if(this->cropping_active) {
    frame->crop_top += this->start_line;
    frame->crop_bottom += (frame->height + 1 - this->end_line);

    TRACE("crop_nocopy: top ->%d bottom ->%d\n", frame->crop_top, frame->crop_bottom);

    new_height = this->end_line - this->start_line;
    new_ratio  = 12.0/9.0 * ((float)frame->height / (float)new_height);
  }

  _x_post_frame_copy_down(frame, frame->next);

  /* ??? */
  frame->ratio = new_ratio;

  skip = frame->next->draw(frame->next, stream);
  TRACE("crop: top %d, bottom %d\n", frame->crop_top, frame->crop_bottom);
  _x_post_frame_copy_up(frame, frame->next);

  return skip;
}
#endif

/*
 *    Frame handling
 */

static int autocrop_draw(vo_frame_t *frame, xine_stream_t *stream)
{
  post_video_port_t *port = (post_video_port_t *)frame->port;
  autocrop_post_plugin_t *this = (autocrop_post_plugin_t *)port->post;
  int result;
  int detected_start, detected_end;

  if(!this->autodetect) {
    this->start_line = frame->height/8;
    this->end_line   = frame->height*7/8;
    this->crop_total = frame->height/4;
#ifdef USE_CROP
    return crop_nocopy(frame, stream);
#else
    if(frame->format == XINE_IMGFMT_YV12)
      return crop_copy_yv12(frame, stream);
    else /*if(frame->format == XINE_IMGFMT_YUY2)*/
      return crop_copy_yuy2(frame, stream);
#endif
  }

  /* use pts jumps to track stream changes (and seeks) */
  if(frame->pts > 0) {
    if(this->prev_pts>0) {
      int64_t dpts = frame->pts - this->prev_pts;
      if(dpts < INT64_C(-30*90000) || dpts > INT64_C(30*90000)) { /* 30 sec */
	if(this->height_limit_active) {
	  this->height_limit_timer = START_TIMER_INIT;
	  TRACE("short pts jump resetted height limit");
	}
      }
      if(dpts < INT64_C(-30*60*90000) || dpts > INT64_C(30*60*90000)) { /* 30 min */ 
	this->cropping_active = 0;
	TRACE("long pts jump resetted cropping");
      }
    }
    this->prev_pts = frame->pts;
  }

  /* reset ? */
  if(! this->cropping_active) {
    this->prev_start_line = 0;
    this->prev_end_line = frame->height;
    this->start_timer = START_TIMER_INIT;
    this->prev_pts = -1;
    if(this->height_limit_active) {
      TRACE("height limit reset (no cropping)");
    }
    this->height_limit_active = 0;
    this->height_limit = frame->height;
  }

  /* only 4:3 YV12 frames are cropped */
  if(frame->ratio != 4.0/3.0 || (frame->format != XINE_IMGFMT_YV12 && 
				 frame->format != XINE_IMGFMT_YUY2)) {
    this->cropping_active = 0;
    
  } else if(frame->bad_frame) {

  /* check for letterbox borders only from I-frames */
  } else if(frame->picture_coding_type == 1/*XINE_PICT_I_TYPE*/) {

    analyze_frame(frame, &this->start_line, &this->end_line);

    /* ignore very small bars */
    if(this->start_line > 10 || this->end_line < frame->height - 10) 
      this->cropping_active = 1;
    else 
      this->cropping_active = 0;
    
    this->prev_height = frame->height;
    this->prev_width = frame->width;

    /* no change unless same values for several frames */
    if(this->stabilize &&
       (this->start_line != this->prev_start_line ||
	this->end_line != this->prev_end_line)) {
      if(this->stabilize_timer)
	this->stabilize_timer--;
      else
	this->stabilize_timer = 4;
      if(this->stabilize_timer) {
	TRACE("stabilize start_line: %d -> %d, end_line %d -> %d\n", 
	      this->start_line, this->prev_start_line,
	      this->end_line, this->prev_end_line);
	this->start_line = this->prev_start_line;
	this->end_line = this->prev_end_line;
      }
    }

  } else {
    /* reset when format changes */
    if(frame->height != this->prev_height)
      this->cropping_active = 0;
    if(frame->width != this->prev_width)
      this->cropping_active = 0;
  }

  /* update timers */
  if(this->start_timer)
    this->start_timer--;

  if(this->height_limit_timer) {
    if (! --this->height_limit_timer) {
      this->height_limit_active = 0;
      this->height_limit = frame->height;
      TRACE("height limit timer expired");
    }
  } 

  /* no cropping to be done ? */
  if (/*frame->bad_frame ||*/ !this->cropping_active || this->start_timer>0) {
    _x_post_frame_copy_down(frame, frame->next);
    result = frame->next->draw(frame->next, stream);
    _x_post_frame_copy_up(frame, frame->next);
    return result;
  }

  /* "soft start" and border stabilization */
  detected_start = this->start_line;
  detected_end = this->end_line;
  if(this->soft_start) {
    if(this->prev_start_line != this->start_line) {
      int diff = this->prev_start_line - this->start_line;
      if(diff < -4) diff = -4;
      else if(diff > 4) diff = 4;
      else diff = 0;
      this->start_line = this->prev_start_line - diff;
    }
    if(this->prev_end_line != this->end_line) {
      int diff = this->prev_end_line - this->end_line;
      if(diff < -4) diff = -4;
      else if(diff > 4) diff = 4;
      else diff = 0;
      this->end_line = this->prev_end_line - diff;
    }
  }

  /* handle fixed subtitles inside bottom bar */
  if(this->subs_detect) {
    if(abs(this->prev_start_line - this->start_line) > 5 ) {
      /* reset height limit if top bar changes */
      TRACE("height limit reset, top bar moved from %d -> %d\n", 
	    this->prev_start_line, this->start_line);
      this->height_limit_active = 0;
      this->height_limit = frame->height;
      this->height_limit_timer = 0;
    }
    if (this->end_line > this->prev_end_line) {
      if(!this->height_limit_active || 
	 this->height_limit < this->end_line) {
	/* start or increase height limit */
	TRACE("height limit %d -> %d (%d secs)\n", 
	      this->height_limit, this->end_line, 
	      HEIGHT_LIMIT_LIFETIME/25);
	this->height_limit = this->end_line;
	this->height_limit_timer = HEIGHT_LIMIT_LIFETIME;
	this->height_limit_active = 1;
      }
      if(this->height_limit_active &&
	 this->height_limit_timer < HEIGHT_LIMIT_LIFETIME/4) {
	/* keep heigh limit timer running */
	TRACE("height_limit_timer increment (still needed)\n");
	this->height_limit_timer = HEIGHT_LIMIT_LIFETIME/2;
      }
    }
  }

  this->prev_start_line = this->start_line;
  this->prev_end_line = this->end_line;

  if(this->subs_detect) {
    if(this->height_limit_active) {
      /* apply height limit */
      if(this->end_line < this->height_limit)
	this->end_line = this->height_limit;
    } else {
      this->height_limit = frame->height;
    }
  }

  /*
   * do cropping 
   *  - using frame->crop_... does not seem to work with my xv and xine-lib-1.1.1. 
   *  - maybe cropping could be done by adjusting y/u/v data pointers 
   *    and height of frame ?
   *    -> no time-consuming copying 
   */
#ifdef USE_CROP
  result = crop_nocopy(frame, stream);
#else
  if(frame->format == XINE_IMGFMT_YV12)
    result = crop_copy_yv12(frame, stream);
  else /*if(frame->format == XINE_IMGFMT_YUY2)*/
    result = crop_copy_yuy2(frame, stream);
#endif
  this->crop_total = this->start_line + frame->height - this->end_line;

  /* forget stabilized values */
  this->start_line = detected_start;
  this->end_line = detected_end; 

  return result;
}

#ifdef USE_CROP
static vo_frame_t *autocrop_get_frame(xine_video_port_t *port_gen, 
				       uint32_t width, uint32_t height, 
				       double ratio, int format, int flags)
{
  post_video_port_t      *port = (post_video_port_t *)port_gen;
  post_plugin_t          *this_gen = port->post;
  autocrop_post_plugin_t *this = (autocrop_post_plugin_t *)this_gen;
  vo_frame_t             *frame;
  
  _x_post_rewire(this_gen);
  
  if (ratio <= 0.0)
    if(height > 1)
      ratio = (double)width / (double)height;
  
  if (this->cropping_active &&
      ratio == 4.0/3.0 && (format == XINE_IMGFMT_YV12 ||
			   format == XINE_IMGFMT_YUY2)) {
    int new_height = this->end_line+2 - this->start_line;
    float new_ratio = 12.0/9.0 * ((float)height / (float)new_height);

    frame = port->original_port->get_frame(port->original_port,
					   width, height, 
					   new_ratio, format, flags);
    _x_post_inc_usage(port);
    frame = _x_post_intercept_video_frame(frame, port);
    
    frame->ratio = ratio;
    return frame;
  }
  
  return port->original_port->get_frame(port->original_port,
					width, height, 
					ratio, format, flags);
}
#endif

static int autocrop_intercept_frame(post_video_port_t *port, vo_frame_t *frame)
{
  autocrop_post_plugin_t *this = (autocrop_post_plugin_t *)port->post;

  /* Crop only SDTV YV12 4:3 frames ... */
  int intercept = ((frame->format == XINE_IMGFMT_YV12 || 
		    frame->format == XINE_IMGFMT_YUY2) &&
		   frame->ratio == 4.0/3.0 &&
		   frame->width  >= 480 && frame->width  <= 768 &&
		   frame->height >= 288 && frame->height <= 576);
  if(!intercept) {
    this->height_limit_active = 0;
    this->crop_total = 0;
    this->cropping_active = 0;
  }

  return intercept;
}

static int autocrop_intercept_ovl(post_video_port_t *port)
{
  autocrop_post_plugin_t *this = (autocrop_post_plugin_t *)port->post;

  if (!this->cropping_active) 
    return 0;

  return 1;
}

static int32_t autocrop_overlay_add_event(video_overlay_manager_t *this_gen, void *event_gen)
{
  post_video_port_t      *port  = _x_post_ovl_manager_to_port(this_gen);
  autocrop_post_plugin_t *this  = (autocrop_post_plugin_t *)port->post;
  video_overlay_event_t  *event = (video_overlay_event_t *)event_gen;
  int caps;

  if(this->cropping_active && this->crop_total>10) {
    if (event->event_type == OVERLAY_EVENT_SHOW) {
      switch (event->object.object_type) {
      case 0:
	/* regular subtitle */
	/* Subtitle overlays must be coming somewhere inside xine engine */

	caps = port->stream->video_out->get_capabilities (port->stream->video_out);
#ifdef USE_CROP
	if(caps & VO_CAP_CROP) {
	  if(! event->object.overlay->unscaled || !(caps & VO_CAP_UNSCALED_OVERLAY)) {
	    event->object.overlay->y -= this->crop_total;
	  }
	} else {
	  /* object is moved crop_top amount in video_out */
	  if(event->object.overlay->unscaled && (caps & VO_CAP_UNSCALED_OVERLAY)) {
	    /* cancel incorrect move that will be done in video_out */
	    event->object.overlay->y += this->start_line;
	  } else {
	    /* move crop_bottom pixels up */
	    event->object.overlay->y -= (this->crop_total - this->start_line);
	  }
	}

	/* when using cropping overlays are moved in video_out */
	INFO("autocrop_overlay_add_event: subtitle event untouched\n");
#else
	/* when cropping here subtitles coming from inside of xine must be re-positioned */
	if(! event->object.overlay->unscaled || !(caps & VO_CAP_UNSCALED_OVERLAY)) {
	  event->object.overlay->y -= this->crop_total;
	  INFO("autocrop_overlay_add_event: subtitle event moved up\n");
	}
#endif
	break;
      case 1:
	/* menu overlay */
	/* All overlays coming from VDR have this type */

#ifdef USE_CROP
	if(!event->object.overlay->unscaled) {
	    event->object.overlay->y += this->start_line;//crop_total;
	} else {
	  caps = port->stream->video_out->get_capabilities (port->stream->video_out);
	  if(!(caps & VO_CAP_UNSCALED_OVERLAY))
	    event->object.overlay->y += this->start_line;//crop_total;
	}
#endif

	break;
      }
    }
  }

  return port->original_manager->add_event(port->original_manager, event_gen);
}


/*
 *    Parameter functions
 */

static xine_post_api_descr_t *autocrop_get_param_descr(void)
{
  return &autocrop_param_descr;
}

static int autocrop_set_parameters(xine_post_t *this_gen, void *param_gen)
{
  autocrop_post_plugin_t *this = (autocrop_post_plugin_t *)this_gen;
  autocrop_parameters_t *param = (autocrop_parameters_t *)param_gen;

  this->autodetect  = param->enable_autodetect;
  this->subs_detect = param->enable_subs_detect;  
  this->soft_start  = param->soft_start;
  this->stabilize   = param->stabilize;
  TRACE("autocrop_set_parameters: "
	"auto=%d  subs=%d  soft=%d  stabilize=%d\n",
	this->autodetect, this->subs_detect,
	this->soft_start, this->stabilize);
  return 1;
}

static int autocrop_get_parameters(xine_post_t *this_gen, void *param_gen)
{
  autocrop_post_plugin_t *this = (autocrop_post_plugin_t *)this_gen;
  autocrop_parameters_t *param = (autocrop_parameters_t *)param_gen;
  
  TRACE("autocrop_get_parameters: "
	"auto=%d  subs=%d  soft=%d  stabilize=%d\n",
	this->autodetect, this->subs_detect,
	this->soft_start, this->stabilize);
  param->enable_autodetect  = this->autodetect;
  param->enable_subs_detect = this->subs_detect;
  param->soft_start         = this->soft_start;
  param->stabilize          = this->stabilize;
  return 1;
}

static char *autocrop_get_help(void) {
  return _("The autocrop plugin is meant to take 4:3 letterboxed frames and "
           "convert them to 16:9 by removing black bars on the top and bottom "
	   "of the frame.\n"
           "\n"
           "Parameters\n"
           "  enable_autodetect:  Enable automatic letterbox detection\n"
           "  enable_subs_detect: Enable automatic subtitle detection inside bottom bar\n"
           "  soft_start:         Enable soft start of cropping\n"
           "  stabilize:          Stabilize cropping to\n"
	   "                      14:9, 16:9, (16:9+subs), 20:9, (20:9+subs)\n"
           "\n"
         );
}


/*
 *    Open/close
 */

static void autocrop_dispose(post_plugin_t *this_gen)
{
  if (_x_post_dispose(this_gen)) 
    free(this_gen);
}

static post_plugin_t *autocrop_open_plugin(post_class_t *class_gen, 
					    int inputs,
					    xine_audio_port_t **audio_target,
					    xine_video_port_t **video_target)
{
  if (video_target && video_target[ 0 ]) {
    autocrop_post_plugin_t *this = (autocrop_post_plugin_t*)xine_xmalloc(sizeof(autocrop_post_plugin_t));
    post_in_t           *input;
    post_out_t          *output;
    post_video_port_t   *port;
    xine_post_in_t      *input_param;

    static xine_post_api_t post_api =
      { autocrop_set_parameters, autocrop_get_parameters, 
	autocrop_get_param_descr, autocrop_get_help };

    if (this) {
      _x_post_init(&this->post_plugin, 0, 1);

      port = _x_post_intercept_video_port(&this->post_plugin, 
					  video_target[ 0 ], 
					  &input, &output);

      input->xine_in.name   = "video in";
      output->xine_out.name = "video out";

      port->intercept_ovl          = autocrop_intercept_ovl;
      port->new_manager->add_event = autocrop_overlay_add_event;
      port->intercept_frame        = autocrop_intercept_frame;
#ifdef USE_CROP
      port->new_port.get_frame     = autocrop_get_frame;
#endif
      port->new_frame->draw        = autocrop_draw;

      this->post_plugin.xine_post.video_input[ 0 ] = &port->new_port;
      this->post_plugin.dispose = autocrop_dispose;

      input_param       = &this->parameter_input;
      input_param->name = "parameters";
      input_param->type = XINE_POST_DATA_PARAMETERS;
      input_param->data = &post_api;
#if XINE_VERSION_CODE >= 10102
      xine_list_push_back(this->post_plugin.input, input_param);
#else
      xine_list_append_content(this->post_plugin.input, input_param);
#endif
      this->cropping_active = 0;
      this->autodetect  = 1;
      this->subs_detect = 1;
      this->soft_start  = 1;
      this->stabilize   = 1;
      this->start_line  = 0;
      this->end_line    = 576;

      this->prev_start_line = 0;
      this->prev_end_line = 576;

      return &this->post_plugin;
    }
  }

  return NULL;
}


/*
 *    Plugin class
 */

static char *autocrop_get_identifier(post_class_t *class_gen)
{
  return "autocrop";
}

static char *autocrop_get_description(post_class_t *class_gen)
{
  return "Crop letterboxed 4:3 video to 16:9";
}

static void autocrop_class_dispose(post_class_t *class_gen)
{
  free(class_gen);
}

static void *autocrop_init_plugin(xine_t *xine, void *data)
{
  post_class_t *class = (post_class_t*)malloc(sizeof(post_class_t));
  
  if(class) {
    class->open_plugin     = autocrop_open_plugin;
    class->get_identifier  = autocrop_get_identifier;
    class->get_description = autocrop_get_description;
    class->dispose         = autocrop_class_dispose;
  }

  return class;
}


static post_info_t info = { XINE_POST_TYPE_VIDEO_FILTER };

const plugin_info_t xine_plugin_info[] __attribute__((visibility("default"))) =
{
  /* type, API, "name", version, special_info, init_function */  
  { PLUGIN_POST, 9, "autocrop", XINE_VERSION_CODE, &info, &autocrop_init_plugin },
  { PLUGIN_NONE, 0, "", 0, NULL, NULL }
};
