/*
 * i18n.h: Internationalization
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#ifndef __XINELIBOUTPUT_I18N_H
#define __XINELIBOUTPUT_I18N_H

#include <vdr/i18n.h>

extern const tI18nPhrase Phrases[];

#ifndef trNOOP
#  define trNOOP(s) (s)
#endif

#ifndef trVDR
#  define trVDR(s) tr(s)
#endif

#endif //__XINELIBOUTPUT_I18N_H
