/*
 * xine_frontend_lirc.h:
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef XINE_FRONTEND_LIRC_H
#define XINE_FRONTEND_LIRC_H

#include "xine_frontend.h"

void lirc_start(fe_t *fe, char *lirc_dev, int repeat_emu);
void lirc_stop(void);

#endif /* XINE_FRONTEND_LIRC_H */
