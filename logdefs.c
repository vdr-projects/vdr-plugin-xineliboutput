/*
 * logdefs.c: Logging and debug output
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#include "logdefs.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdarg.h>

#ifndef __APPLE__
#  include <linux/unistd.h> /* syscall(__NR_gettid) */
#endif

extern int LogToSysLog; /* xine_frontend.c, log to syslog instead of console */

void x_syslog(int level, const char *module, const char *fmt, ...)
{ 
  va_list argp;
  char buf[512];

  va_start(argp, fmt);
  vsnprintf(buf, 512, fmt, argp);
  buf[sizeof(buf)-1] = 0;

#ifndef __APPLE__
  if(!LogToSysLog) {
    fprintf(stderr,"[%ld] %s%s\n", (long int)syscall(__NR_gettid), module, buf);
  } else {
    syslog(level, "[%ld] %s%s", (long int)syscall(__NR_gettid), module, buf);
  }
#else
  if(!LogToSysLog) {
    fprintf(stderr, "%s%s\n", module, buf);
  } else {
    syslog(level, "%s%s", module, buf);
  }
#endif

  va_end(argp);
}

