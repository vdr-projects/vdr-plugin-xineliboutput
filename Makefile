#
# Makefile for a Video Disk Recorder plugin
#
# See the main source file 'xineliboutput.c' for copyright information and
# how to reach the author.
#
# $Id$
#

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.

PLUGIN = xineliboutput

_default: all


# check for Apple Darwin
ARCH_APPLE_DARWIN = no
ifeq ($(shell gcc -dumpmachine | grep -q 'apple-darwin' && echo "1" || echo "0"), 1)
    ARCH_APPLE_DARWIN = yes
endif

#
# Override configuration here or in ../../../Make.config
#

USE_ICONV = yes
#NOSIGNAL_IMAGE_FILE=/usr/share/vdr/xineliboutput/nosignal.mpv
#STARTUP_IMAGE_FILE=/usr/share/vdr/xineliboutput/logodisplay.mpv
CONFIGURE_OPTS =


###
### The version number of this plugin (taken from the main source file):
###

VERSION = $(shell grep 'static const char \*VERSION *=' $(PLUGIN).c | cut -d'"' -f2)


###
### The C++ compiler and options:
###

CXX      ?= g++
CC       ?= gcc 
OPTFLAGS ?= 

ifeq ($(ARCH_APPLE_DARWIN), yes)
    CXXFLAGS   ?= -O3 -pipe -Wall -Woverloaded-virtual -fPIC -g -fno-common -bundle -flat_namespace -undefined suppress
    CFLAGS     ?= -O3 -pipe -Wall -fPIC -g -fno-common -bundle -flat_namespace -undefined suppress
    LDFLAGS_SO ?= -fvisibility=hidden
else
    CXXFLAGS   ?= -O3 -pipe -Wall -Woverloaded-virtual -fPIC -g 
    CFLAGS     ?= -O3 -pipe -Wall -fPIC -g
    LDFLAGS_SO ?= -shared -fvisibility=hidden 
endif
LIBS_VDR ?= 

###
### The directory environment:
###

VDRDIR  ?= ../../..
LIBDIR  ?= ../../lib
TMPDIR  ?= /tmp
BINDIR  ?= /usr/bin
DESTDIR ?= /

INSTALL ?= install

VDRINCDIR ?= $(VDRDIR)/include

###
### Allow user defined options to overwrite defaults:
###

-include $(VDRDIR)/Make.config
-include Make.config


###
### check for VDR
###

ifeq ($(ARCH_APPLE_DARWIN), yes)
    VDRVERSION = $(shell sed -ne '/define VDRVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)
    APIVERSION = $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)
else
    VDRVERSION = $(shell sed -ne '/define VDRVERSION/ { s/^.*"\(.*\)".*$$/\1/; p }' $(VDRDIR)/config.h)
    APIVERSION = $(shell sed -ne '/define APIVERSION/ { s/^.*"\(.*\)".*$$/\1/; p }' $(VDRDIR)/config.h)
endif

ifeq ($(strip $(VDRVERSION)),)
    $(warning ********************************************************)
    $(warning VDR not detected ! VDR plugin will not be compiled.     )
    $(warning ********************************************************)
    XINELIBOUTPUT_VDRPLUGIN = no
    CONFIGURE_OPTS += --disable-vdr
else
    ifeq ($(strip $(APIVERSION)),)
        $(warning VDR APIVERSION missing, using VDRVERSION $(VDRVERSION) )
        APIVERSION = $(VDRVERSION)
    endif
    XINELIBOUTPUT_VDRPLUGIN = yes
    CONFIGURE_OPTS += --add-cflags=-I$(VDRDIR)
endif


###
### run configure script
###

config.mak: Makefile configure
	@echo Running configure
	./configure --cc=$(CC) --cxx=$(CXX) $(CONFIGURE_OPTS)
-include config.mak

###
### The name of the distribution archive:
###

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)


###
### The name of executable and libraries
###

VDRSXFE              = vdr-sxfe
VDRFBFE              = vdr-fbfe
VDRPLUGIN_SXFE       = lib$(PLUGIN)-sxfe.so
VDRPLUGIN_FBFE       = lib$(PLUGIN)-fbfe.so
XINEINPUTVDR         = xineplug_inp_xvdr.so
XINEPOSTAUTOCROP     = xineplug_post_autocrop.so
XINEPOSTSWSCALE      = xineplug_post_swscale.so
XINEPOSTAUDIOCHANNEL = xineplug_post_audiochannel.so

###
### which programs and libs to build
###

VDRSXFE_EXEC =
VDRFBFE_EXEC =
VDRPLUGIN_SO =
VDRPLUGIN_SXFE_SO =
VDRPLUGIN_FBFE_SO =
XINEINPUTVDR_SO =
XINEPOSTAUTOCROP_SO =
XINEPOSTSWSCALE_SO =
XINEPOSTAUDIOCHANNEL_SO =

ifeq ($(XINELIBOUTPUT_X11), yes)
    VDRSXFE_EXEC = $(VDRSXFE)
    ifeq ($(XINELIBOUTPUT_VDRPLUGIN), yes)
        VDRPLUGIN_SXFE_SO = $(VDRPLUGIN_SXFE)
    endif
endif
ifeq ($(XINELIBOUTPUT_FB), yes)
    VDRFBFE_EXEC = $(VDRFBFE)
    ifeq ($(XINELIBOUTPUT_VDRPLUGIN), yes)
        VDRPLUGIN_FBFE_SO = $(VDRPLUGIN_FBFE)
    endif
endif
ifeq ($(XINELIBOUTPUT_XINEPLUGIN), yes)
    XINEINPUTVDR_SO = $(XINEINPUTVDR)
    XINEPOSTAUTOCROP_SO = $(XINEPOSTAUTOCROP)
    XINEPOSTSWSCALE_SO = $(XINEPOSTSWSCALE)
    XINEPOSTAUDIOCHANNEL_SO = $(XINEPOSTAUDIOCHANNEL)
endif
ifeq ($(XINELIBOUTPUT_VDRPLUGIN), yes)
    VDRPLUGIN_SO = libvdr-$(PLUGIN).so
endif


###
### Includes and Defines (add further entries here):
###

INCLUDES  += -I$(VDRINCDIR)
LIBS_X11  += -L/usr/X11R6/lib -lXv -lXext

ifeq ($(ARCH_APPLE_DARWIN), yes)
    INCLUDES  += -I/sw/include
    LIBDIRS   += -L/sw/lib
    LIBS      += $(LIBDIRS) -liconv
else
    LIBS      += -lrt
endif

DEFINES   += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"' \
             -D_REENTRANT -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 \
	     -DXINELIBOUTPUT_VERSION='"$(VERSION)"'

ifeq ($(XINELIBOUTPUT_XINEPLUGIN), yes)
    CFLAGS += $(shell (pkg-config libxine --atleast-version=1.1.90 && pkg-config libxine --cflags) || xine-config --cflags) 
endif

ifeq ($(USE_ICONV), yes)
  DEFINES += -DUSE_ICONV=1
endif
ifdef NOSIGNAL_IMAGE_FILE
  DEFINES += -DNOSIGNAL_IMAGE_FILE='"$(NOSIGNAL_IMAGE_FILE)"'
endif
ifdef STARTUP_IMAGE_FILE
  DEFINES += -DSTARTUP_IMAGE_FILE='"$(STARTUP_IMAGE_FILE)"'
endif


###
### The object files (add further files here):
###

ifeq ($(XINELIBOUTPUT_VDRPLUGIN), yes)
  OBJS = $(PLUGIN).o device.o frontend.o osd.o config.o menu.o setup_menu.o \
         i18n.o menuitems.o media_player.o equalizer.o \
         frontend_local.o frontend_svr.o \
         tools/cxsocket.o tools/udp_pes_scheduler.o \
         tools/backgroundwriter.o tools/playlist.o tools/http.o \
         tools/vdrdiscovery.o tools/time_pts.o tools.o \
         tools/metainfo_menu.o logdefs.o
  OBJS_MPG  = black_720x576.o nosignal_720x576.o vdrlogo_720x576.o
else
  OBJS = 
  OBJS_MPG = 
endif

ifeq ($(XINELIBOUTPUT_X11), yes)
  OBJS_SXFE_SO = xine_sxfe_frontend.o xine/post.o
  OBJS_SXFE = xine_sxfe_frontend_standalone.o xine/post.o tools/vdrdiscovery.o logdefs.o \
              xine_frontend_lirc.o
else
  OBJS_SXFE_SO = 
  OBJS_SXFE = 
endif

ifeq ($(XINELIBOUTPUT_FB), yes)
  OBJS_FBFE_SO = xine_fbfe_frontend.o xine/post.o
  OBJS_FBFE = xine_fbfe_frontend_standalone.o xine/post.o tools/vdrdiscovery.o logdefs.o \
              xine_frontend_lirc.o
else
  OBJS_FBFE_SO = 
  OBJS_FBFE = 
endif


###
### Implicit rules:
###

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $(CFLAGS_VDR) -o $@ $<


###
### Dependencies:
###

MAKEDEP = g++ -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.c) $(OBJS_SXFE_SO:%.o=%.c) $(OBJS_FBFE_SO:%.o=%.c) > $@

-include $(DEPFILE)

DEFINES += -Wall


###
### Rules:
###

mpg2c: mpg2c.c
	$(CC) mpg2c.c -o $@

black_720x576.c: mpg2c black_720x576.mpg
	@./mpg2c black black_720x576.mpg black_720x576.c
nosignal_720x576.c: mpg2c nosignal_720x576.mpg
	@./mpg2c nosignal nosignal_720x576.mpg nosignal_720x576.c
vdrlogo_720x576.c: mpg2c vdrlogo_720x576.mpg
	@./mpg2c vdrlogo vdrlogo_720x576.mpg vdrlogo_720x576.c

xine_input_vdr.o: xine_input_vdr.c xine_input_vdr.h xine_osd_command.h nosignal_720x576.c logdefs.h
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<
xine_input_http.o: xine_input_http.c
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<
xine_post_autocrop.o: xine_post_autocrop.c
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<
xine_post_swscale.o: xine_post_swscale.c
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<
xine_post_audiochannel.o: xine_post_audiochannel.c
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<

logdefs.o: logdefs.c logdefs.h
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<
xine_frontend_lirc.o: xine_frontend_lirc.c xine_frontend_lirc.h
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<
xine/post.o: xine/post.c xine/post.h
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<
tools/vdrdiscovery.o: tools/vdrdiscovery.c tools/vdrdiscovery.h
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<

xine_sxfe_frontend.o: xine_sxfe_frontend.c
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<
xine_fbfe_frontend.o: xine_fbfe_frontend.c
	$(CC) $(CFLAGS) -c $(DEFINES) $(INCLUDES) $(OPTFLAGS) -o $@ $<

xine_sxfe_frontend_standalone.o: xine_sxfe_frontend.c xine_frontend.c \
		xine_frontend.h xine_input_vdr.h xine_osd_command.h \
		xine/post.h logdefs.h xine_frontend_main.c xine_frontend_lirc.h \
		xineliboutput.c tools/vdrdiscovery.h
	$(CC) $(CFLAGS) -c $(DEFINES) -DFE_STANDALONE $(INCLUDES) $(OPTFLAGS) xine_sxfe_frontend.c -o $@
xine_fbfe_frontend_standalone.o: xine_fbfe_frontend.c xine_frontend.c \
		xine_frontend.h xine_input_vdr.h xine_osd_command.h \
		xine/post.h logdefs.h xine_frontend_main.c xine_frontend_lirc.h \
		xineliboutput.c tools/vdrdiscovery.h
	$(CC) $(CFLAGS) -c $(DEFINES) -DFE_STANDALONE $(INCLUDES) $(OPTFLAGS) xine_fbfe_frontend.c -o $@

### Internationalization (I18N):

PODIR     = po
LOCALEDIR ?= $(VDRDIR)/locale
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmsgs  = $(addprefix $(LOCALEDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.c)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --msgid-bugs-address='<phintuka@users.sourceforge.net>' -o $@ $^

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(I18Nmsgs): $(LOCALEDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: i18n
i18n: $(I18Nmsgs)

###
### targets
###

XINELIBOUTPUT_INSTALL_MSG =  \
	    $(warning *********************** xineliboutput ***************************) \
	    $(warning  Xine plugins and frontends will not be installed automatically. ) \
	    $(warning  To install files execute "make install" in                      ) \
	    $(warning  $(shell echo `pwd`)) \
	    $(warning *****************************************************************) \

install : XINELIBOUTPUT_INSTALL_MSG =

all: $(VDRPLUGIN_SO) $(VDRPLUGIN_SXFE_SO) $(VDRPLUGIN_FBFE_SO) \
	    $(VDRSXFE_EXEC) $(VDRFBFE_EXEC) $(XINEINPUTVDR_SO) \
	    $(XINEPOSTAUTOCROP_SO) $(XINEPOSTSWSCALE_SO) \
	    $(XINEPOSTAUDIOCHANNEL_SO) i18n
	$(XINELIBOUTPUT_INSTALL_MSG)

frontends: $(VDRSXFE_EXEC) $(VDRFBFE_EXEC) $(XINEINPUTVDR_SO) \
	    $(XINEPOSTAUTOCROP_SO) $(XINEPOSTSWSCALE_SO) \
	    $(XINEPOSTAUDIOCHANNEL_SO)

.PHONY: all

#
# VDR plugin
#
ifeq ($(XINELIBOUTPUT_VDRPLUGIN), yes)
$(VDRPLUGIN_SO): $(OBJS) $(OBJS_MPG)
	$(CXX) $(CXXFLAGS) $(LDFLAGS_SO) $(OBJS) $(OBJS_MPG) $(LIBS) $(LIBS_VDR) -o $@
	@-rm -rf $(LIBDIR)/$@.$(APIVERSION)
	@cp $@ $(LIBDIR)/$@.$(APIVERSION)
endif

#
# vdr-sxfe
#
ifeq ($(XINELIBOUTPUT_X11), yes)
$(VDRPLUGIN_SXFE_SO): $(OBJS_SXFE_SO)
	$(CC) $(CFLAGS) $(LDFLAGS_SO) $(OBJS_SXFE_SO) $(LIBS_X11) $(LIBS_XINE) -o $@
	@-rm -rf $(LIBDIR)/$(VDRPLUGIN_SXFE_SO).$(VERSION)
	@cp $@ $(LIBDIR)/$(VDRPLUGIN_SXFE_SO).$(VERSION)
$(VDRSXFE): $(OBJS_SXFE)
	$(CC) -g $(OBJS_SXFE) $(LIBS_X11) $(LIBS_XINE) -o $@
endif

#
# vdr-fbfe
#
ifeq ($(XINELIBOUTPUT_FB), yes)
$(VDRPLUGIN_FBFE_SO): $(OBJS_FBFE_SO)
	$(CC) $(CFLAGS) $(LDFLAGS_SO) $(OBJS_FBFE_SO) $(LIBS_XINE) -o $@
	@-rm -rf $(LIBDIR)/$(VDRPLUGIN_FBFE_SO).$(VERSION)
	@cp $@ $(LIBDIR)/$(VDRPLUGIN_FBFE_SO).$(VERSION)
$(VDRFBFE): $(OBJS_FBFE)
	$(CC) -g $(OBJS_FBFE) $(LIBS_XINE) -o $@
endif

#
# xine plugins
#
ifeq ($(XINELIBOUTPUT_XINEPLUGIN), yes)
$(XINEINPUTVDR_SO): xine_input_vdr.o
	$(CC) $(CFLAGS) $(LDFLAGS_SO) xine_input_vdr.o $(LIBS_XINE) -o $@
$(XINEPOSTAUTOCROP_SO): xine_post_autocrop.o
	$(CC) $(CFLAGS) $(LDFLAGS_SO) xine_post_autocrop.o $(LIBS_XINE) -o $@
$(XINEPOSTSWSCALE_SO): xine_post_swscale.o
	$(CC) $(CFLAGS) $(LDFLAGS_SO) xine_post_swscale.o $(LIBS_XINE) -o $@
$(XINEPOSTAUDIOCHANNEL_SO): xine_post_audiochannel.o
	$(CC) $(CFLAGS) $(LDFLAGS_SO) xine_post_audiochannel.o $(LIBS_XINE) -o $@
endif

install: all
ifeq ($(XINELIBOUTPUT_XINEPLUGIN), yes)
	@mkdir -p $(DESTDIR)/$(XINEPLUGINDIR)/post
	@echo Installing $(DESTDIR)/$(XINEPLUGINDIR)/$(XINEINPUTVDR)
	@-rm -rf $(DESTDIR)/$(XINEPLUGINDIR)/$(XINEINPUTVDR)
	@$(INSTALL) -m 0644 $(XINEINPUTVDR) $(DESTDIR)/$(XINEPLUGINDIR)/$(XINEINPUTVDR)
	@echo Installing $(DESTDIR)/$(XINEPLUGINDIR)/post/$(XINEPOSTAUTOCROP)
	@-rm -rf $(DESTDIR)/$(XINEPLUGINDIR)/post/$(XINEPOSTAUTOCROP)
	@$(INSTALL) -m 0644 $(XINEPOSTAUTOCROP) $(DESTDIR)/$(XINEPLUGINDIR)/post/$(XINEPOSTAUTOCROP)
	@echo Installing $(DESTDIR)/$(XINEPLUGINDIR)/post/$(XINEPOSTSWSCALE)
	@-rm -rf $(DESTDIR)/$(XINEPLUGINDIR)/post/$(XINEPOSTSWSCALE)
	@$(INSTALL) -m 0644 $(XINEPOSTSWSCALE) $(DESTDIR)/$(XINEPLUGINDIR)/post/$(XINEPOSTSWSCALE)
	@echo Installing $(DESTDIR)/$(XINEPLUGINDIR)/post/$(XINEPOSTAUDIOCHANNEL)
	@-rm -rf $(DESTDIR)/$(XINEPLUGINDIR)/post/$(XINEPOSTAUDIOCHANNEL)
	@$(INSTALL) -m 0644 $(XINEPOSTAUDIOCHANNEL) $(DESTDIR)/$(XINEPLUGINDIR)/post/$(XINEPOSTAUDIOCHANNEL)
endif
ifeq ($(XINELIBOUTPUT_FB), yes)
	@echo Installing $(DESTDIR)/$(BINDIR)/vdr-fbfe
	@mkdir -p $(DESTDIR)/$(BINDIR)
	@-rm -rf $(DESTDIR)/$(BINDIR)/vdr-fbfe
	@$(INSTALL) -m 0755 vdr-fbfe $(DESTDIR)/$(BINDIR)/vdr-fbfe
endif
ifeq ($(XINELIBOUTPUT_X11), yes)
	@echo Installing $(DESTDIR)/$(BINDIR)/vdr-sxfe
	@mkdir -p $(DESTDIR)/$(BINDIR)
	@-rm -rf $(DESTDIR)/$(BINDIR)/vdr-sxfe
	@$(INSTALL) -m 0755 vdr-sxfe $(DESTDIR)/$(BINDIR)/vdr-sxfe
endif

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz --exclude=CVS -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz


clean:
	@-rm -f $(DEPFILE) *.so* *.o *.tgz core* *~ *.flc *.bak \
		tools/*.o tools/*~ tools/*.flc xine/*.o xine/*~ \
		xine/*.flc $(VDR_FBFE) $(VDR_SXFE) mpg2c black_720x576.c \
		nosignal_720x576.c vdrlogo_720x576.c vdr-sxfe vdr-fbfe \
		$(PODIR)/*.mo $(PODIR)/*.pot \
		features.h config.mak configure.log

