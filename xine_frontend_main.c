/*
 * Simple main() routine for stand-alone frontends.
 *
 * See the main source file 'xineliboutput.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 *
 */

#include "features.h"

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>
#include <getopt.h>
#include <signal.h>

#include <xine.h>  /* xine_get_version */

#include "logdefs.h"

#include "xine_input_vdr_mrl.h"
#include "xine_frontend.h"
#include "tools/vdrdiscovery.h"
#include "xine_frontend_lirc.h"

/* next symbol is dynamically linked from input plugin */
int SysLogLevel __attribute__((visibility("default"))) = SYSLOGLEVEL_INFO; /* errors and info, no debug */


/* static data */
pthread_t kbd_thread;
struct termios tm, saved_tm;
volatile int terminate_key_pressed = 0;


static int read_key(void)
{
  unsigned char ch;
  int err;
  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;

  errno = 0;
  if(1 == (err=poll(&pfd, 1, 50))) {

    if (1 == (err = read(STDIN_FILENO, &ch, 1)))
      return (int)ch;

    if (err < 0)
      LOGERR("read_key: read(stdin) failed");
    else
      LOGERR("read_key: read(stdin) failed: no stdin");
    return -2;

  } else if(err < 0 && errno != EINTR) {
    LOGERR("read_key: poll(stdin) failed");
    return -2;
  }

  return -1;
}

static uint64_t read_key_seq(void)
{
  /* from vdr, remote.c */
  uint64_t k = 0;
  int key1;
  
  if ((key1 = read_key()) >= 0) {
    k = key1;
    if (key1 == 0x1B) {
      // Start of escape sequence
      if ((key1 = read_key()) >= 0) {
	k <<= 8;
	k |= key1 & 0xFF;
	switch (key1) {
	  case 0x4F: // 3-byte sequence
	    if ((key1 = read_key()) >= 0) {
	      k <<= 8;
	      k |= key1 & 0xFF;
	    }
	    break;
	  case 0x5B: // 3- or more-byte sequence
	    if ((key1 = read_key()) >= 0) {
	      k <<= 8;
	      k |= key1 & 0xFF;
	      switch (key1) {
	        case 0x31 ... 0x3F: // more-byte sequence
	        case 0x5B: // strange, may apparently occur
		  do {
		    if ((key1 = read_key()) < 0)
		      break; // Sequence ends here
		    k <<= 8;
		    k |= key1 & 0xFF;
		  } while (key1 != 0x7E);
		  break;
	      }
	    }
	    break;
	}
      }
    }
  }
  if(key1==-2)
    return 0xffff;
  return k;
}

static void *kbd_receiver_thread(void *fe_gen) 
{
  frontend_t *fe = (frontend_t*)fe_gen;
  uint64_t code = 0;
  char str[64];

  system("setterm -cursor off");
  system("setterm -blank off");

  /* Set stdin to deliver keypresses without buffering whole lines */
  tcgetattr(STDIN_FILENO, &saved_tm);
  if (tcgetattr(STDIN_FILENO, &tm) == 0) {
    tm.c_iflag = 0;
    tm.c_lflag &= ~(ICANON | ECHO);
    tm.c_cc[VMIN] = 0;
    tm.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &tm);
  }

  do {
    alarm(0);
    errno = 0;
    code = read_key_seq();
    alarm(3); /* watchdog */
    if(code == 0)
      continue;
    if(code == 27) {
      fe->send_event(fe, "QUIT");
      break;
    }
#if defined(XINELIBOUTPUT_FE_TOGGLE_FULLSCREEN) || defined(INTERPRET_LIRC_KEYS)
    if(code == 'f' || code == 'F') {
      fe->send_event(fe, "TOGGLE_FULLSCREEN");
      continue;
    } else if(code == 'd' || code == 'D') {
      fe->send_event(fe, "TOGGLE_DEINTERLACE");
      continue;
    } else
#endif
    if(code == 0xffff) 
      break;

    snprintf(str, sizeof(str), "%016" PRIX64, code);
    fe->send_input_event(fe, "KBD", str, 0, 0);

  } while(fe->xine_is_finished(fe, 0) != FE_XINE_EXIT && code != 0xffff);
  
  alarm(0);
  LOGDBG("Keyboard thread terminated");
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_tm);
  system("setterm -cursor on");

  pthread_exit(NULL);
  return NULL; /* never reached */
}

static void *slave_receiver_thread(void *fe_gen)
{
  frontend_t *fe = (frontend_t*)fe_gen;
  char str[128], *pt;

  tcgetattr(STDIN_FILENO, &saved_tm);

  do {

    errno = 0;
    str[0] = 0;

    if(!fgets(str, sizeof(str), stdin))
      break;

    if(NULL != (pt = strchr(str, '\r')))
      *pt = 0;
    if(NULL != (pt = strchr(str, '\n')))
      *pt = 0;

    if(!strncasecmp(str, "QUIT", 4)) {
      fe->send_event(fe, "QUIT");
      break;

    } else if(!strncasecmp(str, "FULLSCREEN", 10)) {
      fe->send_event(fe, "TOGGLE_FULLSCREEN");

    } else if(!strncasecmp(str, "DEINTERLACE ", 12)) {
      fe->send_event(fe, str);

    } else if(!strncasecmp(str, "HITK ", 5)) {
      fe->send_input_event(fe, NULL, str+5, 0, 0);

    } else {
      LOGMSG("Unknown slave mode command: %s", str);
    }

  } while(fe->xine_is_finished(fe, 0) != FE_XINE_EXIT);
  
  LOGDBG("Slave mode receiver terminated");
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_tm);

  pthread_exit(NULL);
  return NULL; /* never reached */
}

static void kbd_stop(void)
{
  void *p;

  pthread_cancel (kbd_thread);
  pthread_join (kbd_thread, &p);

  tcsetattr(STDIN_FILENO, TCSANOW, &saved_tm);
  system("setterm -cursor on");
}

static void SignalHandler(int signum)
{
  if (signum != SIGPIPE)
    terminate_key_pressed = 1;

  signal(signum, SignalHandler);
}

static char *strcatrealloc(char *dest, const char *src)
{
  size_t l;

  if (!src || !*src) 
    return dest;

  l = (dest ? strlen(dest) : 0) + strlen(src) + 1;
  if(dest) {
    dest = (char *)realloc(dest, l);
    strcat(dest, src);
  } else {
    dest = (char*)malloc(l);
    strcpy(dest, src);
  } 
  return dest;
}

static const char help_str[] = 
"When server address is not given, server is searched from local network.\n"
"If server is not found, localhost (127.0.0.1) is used as default.\n\n"
    "   --help                        Show (this) help message\n"
    "   --audio=audiodriver[:device]  Select audio driver and optional port\n"
    "   --video=videodriver[:device]  Select video driver and optional port\n"
#ifndef IS_FBFE
    "   --display=displayaddress      X11 display address\n"
    "   --wid=id                      Use existing X11 window\n"
#endif
    "   --aspect=[auto|4:3|16:9|16:10|default]\n"
    "                                 Display aspect ratio\n"
    "                                 Use script to control HW aspect ratio:\n"
    "                                   --aspect=auto:path_to_script\n"
    "   --fullscreen                  Fullscreen mode\n"
#ifdef HAVE_XRENDER
    "   --hud                         Head Up Display OSD mode\n"
#endif
    "   --width=x                     Video window width\n"
    "   --height=x                    Video window height\n"
    "   --noscaling                   Disable all video scaling\n"
    "   --post=name[:arg=val[,arg=val]] Load and use xine post plugin(s)\n"
    "                                 examples:\n"
    "                                 --post=upmix\n"
    "                                 --post=upmix;tvtime:enabled=1,cheap_mode=1\n"
    "   --lirc[=devicename]           Use lirc input device\n"
    "                                 Optional lirc socket name can be given\n"
    "   --verbose                     Verbose debug output\n"
    "   --silent                      Silent mode (report only errors)\n"
    "   --syslog                      Write all output to system log\n"
    "   --nokbd                       Disable keyboard input\n"
    "   --daemon                      Run as daemon (disable keyboard,\n"
    "                                 log to syslog and fork to background)\n"
    "   --slave                       Enable slave mode (read commands from stdin)\n"
    "   --reconnect                   Automatically reconnect when connection has been lost\n"
    "   --tcp                         Use TCP transport\n"
    "   --udp                         Use UDP transport\n"
    "   --rtp                         Use RTP transport\n\n"
    "                                 If no transport options are given, transports\n"
    "                                 are tried in following order:\n"
    "                                 local pipe, rtp, udp, tcp\n\n";

static const char short_options[] = "HL:A:V:d:a:fDw:h:P:vslkbtur";

static const struct option long_options[] = {
  { "help",       no_argument,       NULL, 'H' },
  { "audio",      required_argument, NULL, 'A' },
  { "video",      required_argument, NULL, 'V' },
  { "display",    required_argument, NULL, 'd' },
  { "wid",        required_argument, NULL, 'W' },
  { "aspect",     required_argument, NULL, 'a' },
  { "fullscreen", no_argument,       NULL, 'f' },
  { "hud",        no_argument,       NULL, 'D' },
  { "width",      required_argument, NULL, 'w' },
  { "height",     required_argument, NULL, 'h' },
  { "noscaling",  no_argument,       NULL, 'n' },
  { "post",       required_argument, NULL, 'P' },
  { "lirc",       optional_argument, NULL, 'L' },

  { "verbose", no_argument,  NULL, 'v' },
  { "silent",  no_argument,  NULL, 's' },
  { "syslog",  no_argument,  NULL, 'l' },
  { "nokbd",   no_argument,  NULL, 'k' },
  { "daemon",  no_argument,  NULL, 'b' },
  { "slave",   no_argument,  NULL, 'S' },
  
  { "reconnect", no_argument,  NULL, 'R' },
  { "tcp",       no_argument,  NULL, 't' },
  { "udp",       no_argument,  NULL, 'u' },
  { "rtp",       no_argument,  NULL, 'r' },
  { NULL }
};
 
#define PRINTF(x...) do { if(SysLogLevel>1) printf(x); } while(0)

int main(int argc, char *argv[])
{
  char *mrl = NULL, *gdrv = NULL, *adrv = NULL, *adev = NULL;
  int ftcp = 0, fudp = 0, frtp = 0, reconnect = 0, firsttry = 1;
  int fullscreen = 0, hud = 0, width = 720, height = 576;
  int scale_video = 1, aspect = 1;
  int daemon_mode = 0, nokbd = 0, slave_mode = 0;
  char *video_port = NULL;
  int window_id = -1;
  int xmajor, xminor, xsub;
  int err, c;
  int xine_finished = FE_XINE_RUNNING;
  frontend_t *fe = NULL;
  extern const fe_creator_f fe_creator;
  char *static_post_plugins = NULL;
  char *lirc_dev = NULL;
  char *aspect_controller = NULL;
  int repeat_emu = 0;
  char *exec_name = argv[0];

  LogToSysLog = 0;

  if(strrchr(argv[0],'/'))
    exec_name = strrchr(argv[0],'/')+1;

  xine_get_version(&xmajor, &xminor, &xsub);  
  printf("%s %s  (build with xine-lib %d.%d.%d, using xine-lib %d.%d.%d)\n\n",
	 exec_name,
	 FE_VERSION_STR,
	 XINE_MAJOR_VERSION, XINE_MINOR_VERSION, XINE_SUB_VERSION,
	 xmajor, xminor, xsub);

  /* Parse arguments */
  while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
    switch (c) {
    default:
    case 'H': printf("\nUsage: %s [options] [" MRL_ID "[+udp|+tcp|+rtp]:[//host[:port]]] \n"
		     "\nAvailable options:\n", exec_name);
              printf("%s", help_str);
	      list_xine_plugins(NULL, SysLogLevel>2);
	      exit(0);
    case 'A': adrv = strdup(optarg);
              adev = strchr(adrv, ':');
	      if(adev)
	        *(adev++) = 0;
	      PRINTF("Audio driver: %s\n",adrv);
	      if(adev)
		PRINTF("Audio device: %s\n",adev);
	      break;
    case 'V': gdrv = strdup(optarg);
              if(strchr(gdrv, ':')) {
		video_port = strchr(gdrv, ':');
		*video_port = 0;
		video_port++;
		PRINTF("Video port: %s\n",video_port);
	      }
              PRINTF("Video driver: %s\n",gdrv);
              break;
#ifndef IS_FBFE
    case 'W': window_id = atoi(optarg);
              break;
    case 'd': video_port = strdup(optarg);
              break;
#endif
    case 'a': if(!strncmp(optarg, "auto", 4))
                aspect = 0;
              if(!strncmp(optarg, "4:3", 3))
		aspect = 2;
	      if(!strncmp(optarg, "16:9", 4))
		aspect = 3;
	      if(!strncmp(optarg, "16:10", 5))
		aspect = 4;
              if(aspect == 0 && optarg[4] == ':')
                aspect_controller = strdup(optarg+5);
	      PRINTF("Aspect ratio: %s\n", 
		     aspect==0?"Auto":aspect==2?"4:3":aspect==3?"16:9":
		     aspect==4?"16:10":"Default");
              if(aspect_controller)
                PRINTF("Using %s to switch aspect ratio\n", 
                       aspect_controller);
	      break;
    case 'f': fullscreen=1;
              PRINTF("Fullscreen mode\n");
	      break;
    case 'D': hud=1;
#ifdef HAVE_XRENDER
              PRINTF("HUD OSD mode\n");
#else
              PRINTF("HUD OSD not supported\n");
#endif
              break;
    case 'w': width = atoi(optarg);
              PRINTF("Width: %d\n", width);
	      break;
    case 'h': height = atoi(optarg);
              PRINTF("Height: %d\n", height);
	      break;
    case 'n': scale_video = 0;
              PRINTF("Video scaling disabled\n");
	      break;
    case 'P': if(static_post_plugins)
                strcatrealloc(static_post_plugins, ";");
              static_post_plugins = strcatrealloc(static_post_plugins, optarg);
	      PRINTF("Post plugins: %s\n", static_post_plugins);
	      break;
    case 'L': lirc_dev = optarg ? : strdup("/dev/lircd");
              if(strstr((char*)lirc_dev, ",repeatemu")) {
		*strstr((char*)lirc_dev, ",repeatemu") = 0;
		repeat_emu = 1;
              }
              PRINTF("LIRC device:  %s%s\n", lirc_dev,
		     repeat_emu?", emulating key repeat":"");
	      break;
    case 'v': SysLogLevel = (SysLogLevel<SYSLOGLEVEL_DEBUG) ? SYSLOGLEVEL_DEBUG : SysLogLevel+1;
	      PRINTF("Verbose mode\n");
	      break;
    case 's': SysLogLevel = 1;
	      PRINTF("Silent mode\n");
	      break;
    case 'S': slave_mode = 1;
	      PRINTF("Slave mode\n");
	      break;
    case 'l': LogToSysLog = 1;
              openlog(exec_name, LOG_PID|LOG_CONS, LOG_USER);
	      break;
    case 'k': nokbd = 1;
              PRINTF("Keyboard input disabled\n");
	      break;
    case 'b': nokbd = daemon_mode = 1;
              PRINTF("Keyboard input disabled\n");
	      break;
    case 'R': reconnect = 1;
              PRINTF("Automatic reconnection enabled\n");
	      break;
    case 't': ftcp = 1;
              PRINTF("Protocol: TCP\n");
	      break;
    case 'u': fudp = 1;
              PRINTF("Protocol: UDP\n");
	      break;
    case 'r': frtp = 1;
              PRINTF("Protocol: RTP\n");
	      break;
    case 1:   printf("arg 1 (%s)\n", long_options[optind].name); exit(0);
    }
  }

  if (optind < argc) {
    mrl = strdup(argv[optind]);
    PRINTF("VDR Server: %s\n", mrl);
    while (++optind < argc)
      printf("Unknown argument: %s\n", argv[optind]);
  }

  PRINTF("\n");

#if 1
  /* backward compability */
  if(mrl && ( !strncmp(mrl, MRL_ID ":tcp:",  MRL_ID_LEN+5) ||
	      !strncmp(mrl, MRL_ID ":udp:",  MRL_ID_LEN+5) ||
	      !strncmp(mrl, MRL_ID ":rtp:",  MRL_ID_LEN+5) ||
	      !strncmp(mrl, MRL_ID ":pipe:", MRL_ID_LEN+6)))
    mrl[5] = '+';
#endif

  /* If server address not given, try to find server automatically */
  if(!mrl ||
     !strcmp(mrl, MRL_ID ":") ||
     !strcmp(mrl, MRL_ID "+tcp:") ||
     !strcmp(mrl, MRL_ID "+udp:") ||
     !strcmp(mrl, MRL_ID "+rtp:") ||
     !strcmp(mrl, MRL_ID "+pipe:")) {
    char address[1024] = "";
    int port = -1;
    PRINTF("VDR server not given, searching ...\n");
    if(udp_discovery_find_server(&port, &address[0])) {
      PRINTF("Found VDR server: host %s, port %d\n", address, port);
      if(mrl) {
	char *tmp = mrl;
	mrl = NULL;
	if(asprintf(&mrl, "%s//%s:%d", tmp, address, port) < 0) {
	  free(tmp);
	  return -1;
        }
      } else
	if(asprintf(&mrl, MRL_ID "://%s:%d", address, port) < 0)
          return -1;
    } else {
      PRINTF("---------------------------------------------------------------\n"
	     "WARNING: MRL not given and server not found from local network.\n"
	     "         Trying to connect to default port on local host.\n"
	     "---------------------------------------------------------------\n");
      mrl = strdup(MRL_ID "://127.0.0.1");
    }
  }

  if(mrl && 
     strncmp(mrl, MRL_ID ":", MRL_ID_LEN+1) && 
     strncmp(mrl, MRL_ID "+", MRL_ID_LEN+1)) {
    char *mrl2 = mrl;
    PRINTF("WARNING: MRL does not start with \'" MRL_ID ":\' (%s)", mrl);
    if(asprintf(&mrl, MRL_ID "://%s", mrl) < 0)
      return -1;
    free(mrl2);
  }

  {
    char *tmp = NULL, *mrl2 = mrl;
    if(frtp && !strstr(mrl, "rtp:"))
      tmp = strdup(MRL_ID "+rtp:");
    else if(fudp && !strstr(mrl, "udp:"))
      tmp = strdup(MRL_ID "+udp:");
    else if(ftcp && !strstr(mrl, "tcp:"))
      tmp = strdup(MRL_ID "+tcp:");
    if(tmp) {
      mrl = strcatrealloc(tmp, strchr(mrl, '/'));
      free(mrl2);
    }
  }

  if(daemon_mode) {
    PRINTF("Entering daemon mode\n\n");
    if (daemon(1, 0) == -1) {
      fprintf(stderr, "%s: %m\n", exec_name);
      LOGERR("daemon() failed");
      return -2;
    }
  }

  /* Create front-end */
  fe = (*fe_creator)();
  if(!fe) {
    fprintf(stderr, "Error initializing frontend\n");
    return -3;
  }

  /* Initialize display */
  if(!fe->fe_display_open(fe, width, height, fullscreen, hud, 0, 
			  "", aspect, NULL, video_port, scale_video, 0,
			  aspect_controller, window_id)) {
    fprintf(stderr, "Error opening display\n");
    fe->fe_free(fe);
    return -4;
  }

  /* Initialize xine */
  if(!fe->xine_init(fe, adrv, adev, gdrv, 250, static_post_plugins)) {
    fprintf(stderr, "Error initializing xine\n");
    fe->fe_free(fe);
    list_xine_plugins(NULL, SysLogLevel>2);
    return -5;
  }
  if(SysLogLevel>2)
    list_xine_plugins(fe, SysLogLevel>2);

  /* signal handlers */

  if (signal(SIGHUP,  SignalHandler) == SIG_IGN) signal(SIGHUP,  SIG_IGN);
  if (signal(SIGINT,  SignalHandler) == SIG_IGN) signal(SIGINT,  SIG_IGN);
  if (signal(SIGTERM, SignalHandler) == SIG_IGN) signal(SIGTERM, SIG_IGN);
  if (signal(SIGPIPE, SignalHandler) == SIG_IGN) signal(SIGPIPE, SIG_IGN);

  /* Start LIRC forwarding */
  lirc_start(fe, lirc_dev, repeat_emu);
  
  PRINTF("\n\nPress Esc to exit\n\n");
  
  /* Start keyboard listener thread */
  if(!nokbd) {
    if ((err = pthread_create (&kbd_thread,
			       NULL, 
			       slave_mode ? slave_receiver_thread : kbd_receiver_thread, 
			       (void*)fe)) != 0) {
      fprintf(stderr, "Can't create new thread for keyboard (%s)\n", 
	      strerror(err));
    }
    sleep(1);
  }
  
  do {

    if(!firsttry) {
      PRINTF("Connection to server lost. Reconnecting after two seconds...\n");
      sleep(2);
      PRINTF("Reconnecting...\n");
    }

    /* Connect to VDR xineliboutput server */
    if(!fe->xine_open(fe, mrl)) {
      /*print_xine_log(((fe_t *)fe)->xine);*/
      if(!firsttry) {
	PRINTF("Error opening %s\n", mrl);
	continue;
      }
      fprintf(stderr, "Error opening %s\n", mrl);
      lirc_stop();
      if(!nokbd) kbd_stop();
      fe->fe_free(fe);
      return -6;
    }
  
    if(!fe->xine_play(fe)) {
      /*print_xine_log(((fe_t *)fe)->xine);*/
      /*printf("Error playing %s//%s:%s\n", argv[1], host, port);*/
      if(!firsttry) {
	PRINTF("Error playing %s\n", argv[1]);
	continue;
      }
      fprintf(stderr, "Error playing %s\n", argv[1]);
      lirc_stop();
      if(!nokbd) kbd_stop();
      fe->fe_free(fe);
      return -7;
    }
  
    /* Main loop */

    sleep(2);  /* give input_vdr some time to establish connection */

    fflush(stdout);
    fflush(stderr);

    while(!terminate_key_pressed && fe->fe_run(fe) && 
	  (FE_XINE_RUNNING == (xine_finished = fe->xine_is_finished(fe,0))))
      ;

    fe->xine_close(fe);
    firsttry = 0;

  } while(!terminate_key_pressed && xine_finished != FE_XINE_EXIT && reconnect);

  /* Clean up */

  PRINTF("Terminating...\n");

  fe->send_event(fe, "QUIT");

  lirc_stop();
  if(!nokbd) kbd_stop();

  fe->fe_free(fe); 
  return xine_finished==FE_XINE_EXIT ? 0 : 1;
}
