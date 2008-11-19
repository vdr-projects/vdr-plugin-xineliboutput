/*
 * vo_props.h: Extended video-out capabilities and properties
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef XINELIBOUTPUT_VO_PROPS_H_
#define XINELIBOUTPUT_VO_PROPS_H_


/* output can scale OSD */
#define VO_CAP_OSDSCALING  0x01000000

/* enable/disable OSD scaling */
#define VO_PROP_OSD_SCALING 0x1001
/* OSD width */
#define VO_PROP_OSD_WIDTH   0x1002
/* OSD height */
#define VO_PROP_OSD_HEIGHT  0x1003


#endif /* XINELIBOUTPUT_VO_PROPS_H_ */