/*
 * xine_osd_command.h:
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef __XINE_OSD_COMMAND_H_
#define __XINE_OSD_COMMAND_H_

#include <stdint.h>

#ifndef PACKED
#  define PACKED  __attribute__((packed))
#endif
#ifndef ALIGNED
#  define ALIGNED __attribute__((aligned))
#endif

#define MAX_OSD_OBJECT 50

#if defined __cplusplus
extern "C" {
#endif

typedef enum  {
  OSD_Nop         = 0,    /* Do nothing ; used to initialize delay_ms counter */
  OSD_Size        = 1,    /* Set size of VDR OSD area (usually 720x576) */
  OSD_Set_RLE     = 2,    /* Create/update OSD window. Data is rle-compressed. */
  OSD_Close       = 5,    /* Close OSD window */
  OSD_Commit      = 7,    /* All OSD areas have been updated, commit changes to display */
  OSD_Flush       = 8,    /* Flush all pending OSD operations immediately */
  OSD_VideoWindow = 9,    /* Set video window inside OSD */
  OSD_Set_HDMV    = 10,   /* Create/update OSD window. Data is RLE compressed. */
  OSD_Set_LUT8    = 11,   /* Create/update OSD window. Data is uncompressed. */
  OSD_Set_ARGB    = 12,   /* Create/update OSD window. Data is uncompressed. */
  OSD_Set_ARGBRLE = 13,   /* Create/update OSD window. Data is RLE compressed. */
} osd_command_id_t;

#define OSDFLAG_YUV_CLUT        0x01 /* palette is in YUV format */
#define OSDFLAG_REFRESH         0x02 /* OSD data refresh for new config, clients, etc. - no changes in bitmap */
#define OSDFLAG_UNSCALED        0x04 /* xine-lib unscaled (hardware) blending                   */
#define OSDFLAG_UNSCALED_LOWRES 0x08 /* unscaled blending when video resolution < .95 * 720x576 */

#define OSDFLAG_TOP_LAYER       0x10 /* window is part of top layer OSD */

typedef struct osd_clut_s {
  union {
    uint8_t cb  /*: 8*/;
    uint8_t g;
  } PACKED;
  union {
    uint8_t cr  /*: 8*/;
    uint8_t b;
  } PACKED;
  union {
    uint8_t y   /*: 8*/;
    uint8_t r;
  } PACKED;
  uint8_t alpha /*: 8*/;
} PACKED osd_clut_t; /* from xine, alphablend.h */

typedef struct osd_rle_elem_s {
  uint16_t len;
  uint16_t color;
} PACKED osd_rle_elem_t; /* from xine */

typedef struct osd_rect_s {
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
} PACKED osd_rect_t;

typedef struct osd_command_s {
  uint8_t  size;     /* size of osd_command_t struct */

  uint8_t  cmd;      /* osd_command_id_t */

  uint8_t  wnd;      /* OSD window handle */
  uint8_t  layer;    /* OSD layer */

  int64_t  pts;      /* execute at given pts */
  uint32_t delay_ms; /* execute 'delay_ms' ms after previous command (for same window). */

  uint16_t x;         /* window position, x */
  uint16_t y;         /* window position, y */
  uint16_t w;         /* window width */
  uint16_t h;         /* window height */

  uint32_t datalen;   /* size of image data, in bytes */
  uint32_t num_rle;
  union {
    osd_rle_elem_t  *data; /* RLE compressed image */
    uint8_t         *raw_data;
    uint64_t         dummy01;
  } PACKED;
  uint32_t colors;         /* palette size */
  union {
    osd_clut_t      *palette;  /* palette (YCrCb) */
    uint64_t         dummy02;
  } PACKED;

  osd_rect_t dirty_area;
  uint8_t    flags;
  uint8_t    scaling;

} PACKED osd_command_t ALIGNED;

#define ctt_assert(e) ((void)sizeof(char[1 - 2*!(e)]))
static inline void ctt_assert_cmd_size(void) {
  ctt_assert(sizeof(struct osd_command_s) < 0x100);
}

# define hton_osdcmd(cmdP) \
  do { \
    cmdP.pts      = priv_htonll(cmdP.pts);           \
    cmdP.delay_ms = htonl (cmdP.delay_ms);           \
    cmdP.x        = htons (cmdP.x);                  \
    cmdP.y        = htons (cmdP.y);                  \
    cmdP.w        = htons (cmdP.w);                  \
    cmdP.h        = htons (cmdP.h);                  \
    cmdP.datalen  = htonl (cmdP.datalen);            \
    cmdP.num_rle  = htonl (cmdP.num_rle);            \
    cmdP.colors   = htonl (cmdP.colors);             \
    cmdP.dirty_area.x1 = htons(cmdP.dirty_area.x1);  \
    cmdP.dirty_area.y1 = htons(cmdP.dirty_area.y1);  \
    cmdP.dirty_area.x2 = htons(cmdP.dirty_area.x2);  \
    cmdP.dirty_area.y2 = htons(cmdP.dirty_area.y2);  \
  } while(0)

# define ntoh_osdcmd(cmdP) \
  do { \
    cmdP.pts      = priv_ntohll(cmdP.pts);           \
    cmdP.delay_ms = ntohl (cmdP.delay_ms);           \
    cmdP.x        = ntohs (cmdP.x);                  \
    cmdP.y        = ntohs (cmdP.y);                  \
    cmdP.w        = ntohs (cmdP.w);                  \
    cmdP.h        = ntohs (cmdP.h);                  \
    cmdP.datalen  = ntohl (cmdP.datalen);            \
    cmdP.num_rle  = ntohl (cmdP.num_rle);            \
    cmdP.colors   = ntohl (cmdP.colors);             \
    cmdP.dirty_area.x1 = ntohs(cmdP.dirty_area.x1);  \
    cmdP.dirty_area.y1 = ntohs(cmdP.dirty_area.y1);  \
    cmdP.dirty_area.x2 = ntohs(cmdP.dirty_area.x2);  \
    cmdP.dirty_area.y2 = ntohs(cmdP.dirty_area.y2);  \
  } while(0)

#if defined __cplusplus
}
#endif

#endif /*__XINE_OSD_COMMAND_H_*/
