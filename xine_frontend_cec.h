/*
 * xine_frontend_cec.h:
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef XINE_FRONTEND_CEC_H
#define XINE_FRONTEND_CEC_H

struct frontend_s;

void cec_start(struct frontend_s *fe, int hdmi_port, int dev_type);
void cec_stop(void);

#endif /* XINE_FRONTEND_CEC_H */
