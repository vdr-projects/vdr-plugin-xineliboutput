/*
 * rle.c: RLE utils
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef __FreeBSD__
#include <sys/types.h>
#endif

#include "osd_command.h"

#include "rle.h"


#undef  MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/*
 * rle_compress()
 *
 */
unsigned rle_compress(osd_rle_elem_t **rle_data, const uint8_t *data, unsigned w, unsigned h)
{
  osd_rle_elem_t rle, *rle_p = 0, *rle_base;
  unsigned x, y, num_rle = 0, rle_size = 8128;
  const uint8_t *c;

  rle_p = (osd_rle_elem_t*)malloc(4*rle_size);
  rle_base = rle_p;

  for (y = 0; y < h; y++) {
    rle.len = 0;
    rle.color = 0;
    c = data + y * w;
    for (x = 0; x < w; x++, c++) {
      if (rle.color != *c) {
        if (rle.len) {
          if ( (num_rle + h-y+1) > rle_size ) {
            rle_size *= 2;
            rle_base = (osd_rle_elem_t*)realloc( rle_base, 4*rle_size );
            rle_p = rle_base + num_rle;
          }
          *rle_p++ = rle;
          num_rle++;
        }
        rle.color = *c;
        rle.len = 1;
      } else {
        rle.len++;
      }
    }
    *rle_p++ = rle;
    num_rle++;
  }

  *rle_data = rle_base;
  return num_rle;
}

/*
 * rle_recompress_net()
 *
 * recompress RLE-compressed OSD using variable sized RLE codewords
*/
unsigned rle_recompress_net(uint8_t *raw, osd_rle_elem_t *data, unsigned elems)
{
  uint8_t *raw0 = raw;
  unsigned i;

  for (i = 0; i < elems; i++) {
    uint16_t len   = data[i].len;
    uint16_t color = data[i].color;
    if (len >= 0x80) {
      *(raw++) = (len>>8) | 0x80;
      *(raw++) = (len & 0xff);
    } else {
      *(raw++) = (len & 0x7f);
    }
    *(raw++) = color;
  }

  return (raw - raw0);
}

/*
 * rle_scale_nearest()
 *
 * - Simple nearest-neighbour scaling for RLE-compressed image
 * - fast scaling in compressed form without decompression
 */
osd_rle_elem_t *rle_scale_nearest(const osd_rle_elem_t *old_rle, int *rle_elems,
                                   unsigned w, unsigned h, unsigned new_w, unsigned new_h)
{
  #define FACTORBASE      0x100
  #define FACTOR2PIXEL(f) ((f)>>8)
  #define SCALEX(x) FACTOR2PIXEL(factor_x*(x))
  #define SCALEY(y) FACTOR2PIXEL(factor_y*(y))

  unsigned old_w = w, old_h = h;
  unsigned old_y = 0, new_y = 0;
  unsigned factor_x = FACTORBASE*new_w/old_w;
  unsigned factor_y = FACTORBASE*new_h/old_h;
  unsigned rle_size = MAX(8128, *rle_elems * new_h/h ); /* guess ... */
  unsigned num_rle  = 0;
  osd_rle_elem_t *new_rle = (osd_rle_elem_t*)malloc(sizeof(osd_rle_elem_t)*rle_size);
  osd_rle_elem_t *new_rle_start = new_rle;

  /* we assume rle elements are breaked at end of line */
  while (old_y < old_h) {
    unsigned elems_current_line = 0;
    unsigned old_x = 0, new_x = 0;

    while (old_x < old_w) {
      unsigned new_x_end = SCALEX(old_x + old_rle->len);

      if (new_x_end > new_w) {
        new_x_end = new_w;
      }

      new_rle->len   = new_x_end - new_x;
      new_rle->color = old_rle->color;

      old_x += old_rle->len;
      old_rle++; /* may be incremented to last element + 1 (element is not accessed anymore) */

      if (new_rle->len > 0) {
        new_x += new_rle->len;
        new_rle++;

        num_rle++;
        elems_current_line++;

        if ( (num_rle + 1) >= rle_size ) {
          rle_size *= 2;
          new_rle_start = (osd_rle_elem_t*)realloc( new_rle_start, 4*rle_size);
          new_rle = new_rle_start + num_rle;
        }
      }
    }
    if (new_x < new_w)
      (new_rle-1)->len += new_w - new_x;
    old_y++;
    new_y++;

    if (factor_y > FACTORBASE) {
      /* scale up -- duplicate current line ? */
      int dup = SCALEY(old_y) - new_y;

      /* if no lines left in (old) rle, copy all lines still missing from new */
      if (old_y == old_h)
        dup = new_h - new_y - 1;

      while (dup-- && (new_y+1<new_h)) {
        osd_rle_elem_t *prevline;
        unsigned n;
        if ( (num_rle + elems_current_line + 1) >= rle_size ) {
          rle_size *= 2;
          new_rle_start = (osd_rle_elem_t*)realloc( new_rle_start, 4*rle_size);
          new_rle = new_rle_start + num_rle;
        }

        /* duplicate previous line */
        prevline = new_rle - elems_current_line;
        for (n = 0; n < elems_current_line; n++) {
          *new_rle++ = *prevline++;
          num_rle++;
        }
        new_y++;
      }

    } else if (factor_y < FACTORBASE) {
      /* scale down -- drop next line ? */
      unsigned skip = new_y - SCALEY(old_y);
      if (old_y == old_h-1) {
        /* one (old) line left ; don't skip it if new rle is not complete */
        if (new_y < new_h)
          skip = 0;
      }
      while (skip-- &&
             old_y<old_h /* rounding error may add one line, filter it out */) {
        for (old_x = 0; old_x < old_w;) {
          old_x += old_rle->len;
          old_rle++;
        }
        old_y++;
      }
    }
  }

  *rle_elems = num_rle;
  return new_rle_start;
}

/*
 * Compress ARGB overlays
 */

static uint8_t *write_u32(uint8_t *rle_data, uint32_t color)
{
  *rle_data++ = color >> 24;
  *rle_data++ = color >> 16;
  *rle_data++ = color >> 8;
  *rle_data++ = color;
  return rle_data;
}

static uint8_t *write_rle_argb(uint8_t *rle_data,
                               uint32_t color, unsigned len)
{
  uint8_t alpha = color >> 24;

  if (alpha && len < 2) {
    /* single non-transparent pixel, write as plain */
    unsigned i;
    for (i = 0; i < len; i++) {
      rle_data = write_u32(rle_data, color);
    }
    return rle_data;
  }

  /* rle code marker */
  *rle_data++ = 0;

  if (!alpha) {
    /* transparent */
    if (len < 64) {
      *rle_data++ = len;
    } else {
      *rle_data++ = 0x40 | ((len >> 8) & 0x3f);
      *rle_data++ = len & 0xff;
    }
  } else {
    if (len < 64) {
      *rle_data++ = 0x80 | len;
    } else {
      *rle_data++ = 0x80 | 0x40 | ((len >> 8) & 0x3f);
      *rle_data++ = len & 0xff;
    }

    rle_data = write_u32(rle_data, color);
  }

  return rle_data;
}

size_t rle_compress_argbrle(uint8_t **rle_data, const uint32_t *data,
                            unsigned w, unsigned h, int *num_rle)
{
  unsigned y;
  size_t   rle_size = 0;
  uint8_t *rle = NULL;

  *rle_data = NULL;
  *num_rle = 0;

  assert(w > 0);       /* avoid overreading data */
  assert(w <= 0x3fff); /* larger value does not fit in codeword */

  if (w < 1 || h < 1)
    return 0;

  for (y = 0; y < h; y++) {

    /* grow buffer ? */
    if ((ssize_t)(rle_size - ((const uint8_t *)rle - *rle_data)) < w * 4 * 4) {
      size_t used = (const uint8_t *)rle - *rle_data;
      rle_size = rle_size < 1 ? w*h/16 : rle_size*2;
      *rle_data = realloc(*rle_data, rle_size);
      rle = *rle_data + used;
    }

    /* compress line */
    uint32_t color = *data;
    uint32_t len   = 1;
    unsigned x     = 1;

    for (x = 1; x < w; x++) {
      if (data[x] == color) {
        len++;
      } else {
        rle = write_rle_argb(rle, color, len);
        (*num_rle)++;
        color = data[x];
        len   = 1;
      }
    }

    if (len) {
      rle = write_rle_argb(rle, color, len);
      (*num_rle)++;
    }

    /* end of line marker */
    rle = write_rle_argb(rle, 0, 0);
    (*num_rle)++;
    data += w;
  }

  return (rle - *rle_data);
}

/*
 * Uncompress ARGB RLE
 */

static const uint8_t *read_u32_argb(const uint8_t *rle_data, uint32_t *color)
{
  *color = (rle_data[0] << 24) |
           (rle_data[1] << 16) |
           (rle_data[2] << 8) |
           (rle_data[3]);
  return rle_data + 4;
}

static const uint8_t *decode_len(const uint8_t *rle_data, uint32_t *len)
{
  uint8_t byte = *rle_data++;
  if (!(byte & 0x40))
    *len = byte & 0x3f;
  else
    *len = ((byte & 0x3f) << 8) | *rle_data++;
  return rle_data;
}

int rle_uncompress_argbrle(uint32_t *dst,
                           unsigned w, unsigned h, unsigned stride,
                           const uint8_t *rle_data, unsigned num_rle, size_t rle_size)
{
  unsigned rle_count = 0, x = 0, y = 0;
  const uint8_t *end = rle_data + rle_size;

  while (y < h) {

    if (rle_data >= end || rle_count >= num_rle) {
      return -1 - (rle_data >= end);
    }
    rle_count++;

    /* decode RLE element */

    if (*rle_data) {
      /* plain dword */
      rle_data = read_u32_argb(rle_data, dst + x);
      x++;

    } else {
      /* compressed */
      rle_data++; /* skip marker */

      uint32_t dw, len;
      if (!(*rle_data & 0x80)) {
        /* transparent */
        rle_data = decode_len(rle_data, &len);
        if (x + len > w)
          return -9999;

        if (len) {
          memset(dst + x, 0, len * sizeof(uint32_t));
        } else {
          /* end of line marker (00 00) */
          if (x < w-1) {
            memset(dst + x, 0, (w - x - 1) * 4);
          }
          x = 0;
          dst += stride;
          y++;
        }
      } else {
        /* color */
        unsigned i;
        rle_data = decode_len(rle_data, &len);
        if (x+len > w)
          return -9999;

        rle_data = read_u32_argb(rle_data, &dw);
        for (i = 0; i < len; i++)
          dst[x + i] = dw;
      }
      x += len;
    }

    if (x > w) {
      return -99;
    }
  }

  if (rle_count != num_rle)
    return -100000 - (num_rle - rle_count);

  return rle_count;
}

/*
 * encode single HDMV PG rle element
 */
static uint8_t *write_rle_hdmv(uint8_t *rle_data, unsigned color, unsigned len)
{
  /* short non-transparent sequences are uncompressed */
  if (color && len < 4) {
    unsigned i;
    for (i = 0; i < len; i++) {
      *rle_data++ = color;
    }
    return rle_data;
  }

  /* rle code marker */
  *rle_data++ = 0;

  if (!color) {
    /* transparent */
    if (len < 64) {
      *rle_data++ = len;
    } else {
      *rle_data++ = 0x40 | ((len >> 8) & 0x3f);
      *rle_data++ = len & 0xff;
    }
  } else {
    if (len < 64) {
      *rle_data++ = 0x80 | len;
    } else {
      *rle_data++ = 0x80 | 0x40 | ((len >> 8) & 0x3f);
      *rle_data++ = len & 0xff;
    }
    *rle_data++ = color;
  }

  return rle_data;
}

/*
 * compress LUT8 image using HDMV PG compression algorithm
 */
size_t rle_compress_hdmv(uint8_t **rle_data, const uint8_t *data, unsigned w, unsigned h, int *num_rle)
{
  unsigned y;
  size_t   rle_size = 0;
  uint8_t *rle = NULL;

  assert(w > 0);       /* avoid overreading data */
  assert(w <= 0x3fff); /* larger value does not fit in codeword */

  *rle_data = NULL;
  *num_rle = 0;

  if (w < 1 || h < 1)
    return 0;

  for (y = 0; y < h; y++) {

    /* grow buffer ? */
    if ((ssize_t)(rle_size - (rle - *rle_data)) < w * 4) {
      size_t used = rle - *rle_data;
      rle_size = rle_size < 1 ? w*h/16 : rle_size*2;
      *rle_data = realloc(*rle_data, rle_size);
      rle = *rle_data + used;
    }

    /* compress line */
    unsigned color = *data;
    unsigned len   = 1;
    unsigned x     = 1;

    for (x = 1; x < w; x++) {
      if (data[x] == color) {
        len++;
      } else {
        rle = write_rle_hdmv(rle, color, len);
        (*num_rle)++;
        color = data[x];
        len   = 1;
      }
    }

    if (len) {
      rle = write_rle_hdmv(rle, color, len);
      (*num_rle)++;
    }

    /* end of line marker */
    rle = write_rle_hdmv(rle, 0, 0);
    (*num_rle)++;
    data += w;
  }

  return rle - *rle_data;
}

int rle_uncompress_hdmv(osd_rle_elem_t **data,
                        unsigned w, unsigned h,
                        const uint8_t *rle_data, unsigned num_rle, size_t rle_size)
{
  unsigned rle_count = 0, x = 0, y = 0;
  osd_rle_elem_t *rlep = calloc(2*num_rle, sizeof(osd_rle_elem_t));
  const uint8_t *end = rle_data + rle_size;

  *data = rlep;

  /* convert to xine-lib rle format */
  while (y < h) {

    if (rle_data >= end || rle_count >= 2*num_rle) {
      free(*data);
      *data = NULL;
      return -1 - (rle_data >= end);
    }

    /* decode RLE element */
    unsigned byte = *rle_data++;
    if (byte) {
      rlep->color = byte;
      rlep->len   = 1;
    } else {
      byte = *rle_data++;
      if (!(byte & 0x80)) {
        rlep->color = 0;
        if (!(byte & 0x40))
          rlep->len = byte & 0x3f;
        else
          rlep->len = ((byte & 0x3f) << 8) | *rle_data++;
      } else {
        if (!(byte & 0x40))
          rlep->len = byte & 0x3f;
        else
          rlep->len = ((byte & 0x3f) << 8) | *rle_data++;
        rlep->color = *rle_data++;
      }
    }

    /* move to next element */
    if (rlep->len > 0) {

      if (rlep->len == 1 && x && rlep[-1].color == rlep->color) {
        rlep[-1].len++;
        x++;
      } else {
        x += rlep->len;
        rlep++;
        rle_count++;
      }

      if (x > w) {
        return -9999;
      }

    } else {
      /* end of line marker (00 00) */
      if (x < w-1) {
        //return -1-rlep->color - (w-x);
        rlep->len = w - x;
        rlep->color = 0xff;
        rlep++;
        rle_count++;
      }
      x = 0;
      y++;
    }
  }

  return rle_count;
}

/*
 * uncompress LUT8 RLE to surfaces
 */

void rle_uncompress_lut8(uint8_t *dst,
                         unsigned w, unsigned h, unsigned stride,
                         const struct osd_rle_elem_s *rle_data, unsigned num_rle)
{
  unsigned i, pixelcounter = 0;
  unsigned idx = 0, line = 0;

  for(i = 0; i < num_rle; ++i) {
    uint8_t  color = (rle_data + i)->color;
    unsigned len   = (rle_data + i)->len;
    unsigned j;

    for (j = 0; j < len; ++j) {
      if (pixelcounter >= w) {
        idx += stride - pixelcounter;
        pixelcounter = 0;
        if (++line >= h)
          return;
      }
      dst[idx] = color;
      ++idx;
      ++pixelcounter;
    }
  }
}

void rle_palette_to_argb(uint32_t *argb, const struct osd_clut_s *palette, unsigned entries)
{
  unsigned i;
  for (i = 0; i < entries; i++) {
    argb[i] = (palette[i].alpha << 24) |
              (palette[i].r     << 16) |
              (palette[i].g     << 8 ) |
              (palette[i].b          );
  }
}

void rle_palette_to_rgba(uint32_t *rgba, const struct osd_clut_s *palette, unsigned entries)
{
  unsigned i;
  for (i = 0; i < entries; i++) {
    rgba[i] = (palette[i].r     << 24) |
              (palette[i].g     << 16) |
              (palette[i].b     << 8 ) |
              (palette[i].alpha      );
  }
}

static void rle_uncompress_u32(uint32_t *dst,
                               unsigned w, unsigned h, unsigned stride,
                               const struct osd_rle_elem_s *rle_data, unsigned num_rle,
                               uint32_t *lut)
{
  unsigned i, pixelcounter = 0;
  unsigned idx = 0, line = 0;

  for(i = 0; i < num_rle; ++i) {
    uint32_t color = lut[(rle_data + i)->color];
    unsigned len   = (rle_data + i)->len;
    unsigned j;

    for (j = 0; j < len; ++j) {
      if (pixelcounter >= w) {
        idx += stride - pixelcounter;
        pixelcounter = 0;
        if (++line >= h)
          return;
      }
      dst[idx] = color;
      ++idx;
      ++pixelcounter;
    }
  }
}

void rle_uncompress_argb(uint32_t *dst,
                         unsigned w, unsigned h, unsigned stride,
                         const struct osd_rle_elem_s *rle_data, unsigned num_rle,
                         const struct osd_clut_s *palette, unsigned palette_entries)
{
  uint32_t lut[256] = {0};

  if (palette_entries > 256)
    return;

  rle_palette_to_argb(lut, palette, palette_entries);

  rle_uncompress_u32(dst, w, h, stride, rle_data, num_rle, lut);
}

void rle_uncompress_rgba(uint32_t *dst,
                         unsigned w, unsigned h, unsigned stride,
                         const struct osd_rle_elem_s *rle_data, unsigned num_rle,
                         const struct osd_clut_s *palette, unsigned palette_entries)
{
  uint32_t lut[256] = {0};

  if (palette_entries > 256)
    return;

  rle_palette_to_rgba(lut, palette, palette_entries);

  rle_uncompress_u32(dst, w, h, stride, rle_data, num_rle, lut);
}
