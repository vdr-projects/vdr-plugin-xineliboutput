/*
 * h264.h: H.264 bitstream decoding
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef _XINELIBOUTPUT_H264_H_
#define _XINELIBOUTPUT_H264_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "mpeg.h"

#define NAL_SPS     0x07 /* Sequence Parameter Set */
#define NAL_AUD     0x09 /* Access Unit Delimiter */
#define NAL_END_SEQ 0x10 /* End of Sequence */


#if defined(__i386__) || defined(__x86_64__)
#  define IS_NAL_SPS(buf)     (*(uint32_t*)(buf) == 0x07010000U)
#  define IS_NAL_AUD(buf)     (*(uint32_t*)(buf) == 0x09010000U)
#  define IS_NAL_END_SEQ(buf) (*(uint32_t*)(buf) == 0x10010000U)
#else
#  define IS_NAL_SPS(buf)     ((buf)[0] == 0 && (buf)[1] == 0 && (buf)[2] == 1 && (buf)[3] == NAL_SPS)
#  define IS_NAL_AUD(buf)     ((buf)[0] == 0 && (buf)[1] == 0 && (buf)[2] == 1 && (buf)[3] == NAL_AUD)
#  define IS_NAL_END_SEQ(buf) ((buf)[0] == 0 && (buf)[1] == 0 && (buf)[2] == 1 && (buf)[3] == NAL_END_SEQ)
#endif


typedef struct {
  int width;
  int height;
  mpeg_rational_t pixel_aspect;
  /* ... */
} h264_sps_data_t;

/*
 * input: start of NAL SPS (without 00 00 01 07)
 */
int h264_parse_sps(const uint8_t *buf, int len, h264_sps_data_t *sps);

/*
 * input: start of H.264 video data (not PES)
 */
int  h264_get_picture_type(const uint8_t *buf, int len);

/*
 * input: start of H.264 video data (not PES)
 */
int  h264_get_video_size(const uint8_t *buf, int len, video_size_t *size);


#ifdef __cplusplus
} /* extern "C" { */
#endif


#endif /* _XINELIBOUTPUT_H264_H_ */
