# Makefile for authbind
# 
# authbind is Copyright (C) 1998 Ian Jackson
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
#
# $Id: Makefile,v 1.10 2007-09-23 19:22:24 ian Exp $

prefix=/usr/local
#prefix=/Users/tj/local/authbind

bin_dir=$(prefix)/bin
lib_dir=$(prefix)/lib/authbind
libexec_dir=$(lib_dir)

share_dir=$(prefix)/share
man_dir=$(share_dir)/man
man1_dir=$(man_dir)/man1
man8_dir=$(man_dir)/man8

etc_dir=/etc/authbind
#etc_dir=/Users/tj/local/authbind/etc/authbind

INSTALL_USER=root
INSTALL_GROUP=wheel
#INSTALL_USER=tj
#INSTALL_GROUP=staff

INSTALL_FILE	?= install -o $(INSTALL_USER) -g $(INSTALL_GROUP) -m 644 
INSTALL_PROGRAM ?= install -o $(INSTALL_USER) -g $(INSTALL_GROUP) -m 755 -s
INSTALL_DIR	?= install -o $(INSTALL_USER) -g $(INSTALL_GROUP) -m 755 -d
STRIP		?= strip

# Note that it's necessary to build for both architectures, in order to
# interoperate with executables of either arch.  For example, the Mac OSX
# Android SDK provides i386 executables, which will fail from dyld failures
# from loading the libauthbind.dylib library if that library is only built
# for e.g. the x86_64 arch.
ARCH=-arch x86_64
OSX_CFLAGS=-flat_namespace
OSX_LDFLAGS=$(ARCH) -dynamiclib -dynamic -flat_namespace

OPTIMISE=-O2
LDFLAGS=$(OSX_LDFLAGS)
LIBS=-ldl
CFLAGS=$(ARCH) -g $(OPTIMISE) -D_REENTRANT \
 -Wall -Wwrite-strings -Wpointer-arith -Wimplicit -Wnested-externs \
 -Wmissing-prototypes -Wstrict-prototypes $(OSX_CFLAGS)

CPPFLAGS=-DMAJOR_VER='"$(MAJOR)"' \
 -DMINOR_VER='"$(MINOR)"' \
 -DLIBAUTHBIND='"$(lib_dir)/$(LIBRARY)$(LIBEXT)"' \
 -DHELPER='"$(libexec_dir)/helper"' \
 -DCONFIGDIR='"$(etc_dir)"' \
 -D_GNU_SOURCE \
 -DPRELOAD_VAR='"DYLD_INSERT_LIBRARIES"'

LIBRARY=libauthbind
LIBEXT=.dylib
EXES=authbind helper
OBJS=authbind.o helper.o

TARGETS=$(EXES) $(LIBRARY)

MANPAGES_1=authbind.1
MANPAGES_8=authbind-helper.8

all: $(TARGETS)

install: $(TARGETS) install-man
	mkdir -p $(bin_dir)
	$(INSTALL_PROGRAM) authbind $(bin_dir)/.
	chmod a+x $(bin_dir)/authbind
	mkdir -p $(libexec_dir)
	$(INSTALL_PROGRAM) helper $(libexec_dir)/.
	chmod u+s $(libexec_dir)/helper
	mkdir -p $(lib_dir)
	$(INSTALL_FILE) $(LIBRARY)$(LIBEXT) $(lib_dir)/.
#	$(STRIP) --strip-unneeded $(lib_dir)/$(LIBTARGET)
#	ln -sf $(LIBTARGET) $(lib_dir)/$(LIBCANON)
	$(INSTALL_DIR) $(etc_dir) $(etc_dir)/byport $(etc_dir)/byaddr $(etc_dir)/byuid

install-man: $(MANPAGES_1) $(MANPAGES_8)
	$(INSTALL_DIR) $(man1_dir) $(man8_dir)
	$(INSTALL_FILE) $(MANPAGES_1) $(man1_dir)/.
	$(INSTALL_FILE) $(MANPAGES_8) $(man8_dir)/.

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

authbind: authbind.o
	$(CC) -o $@ $<

helper: helper.o
	$(CC) -o $@ $<

$(LIBRARY):
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@$(LIBEXT) $@.c $(LIBS)

clean distclean:
	$(RM) $(TARGETS) *.o *~ ./#*# *.bak *.new core
	$(RM) $(LIBRARY)$(LIBEXT) *.core
	$(RM) -r $(LIBRARY)$(LIBEXT).dSYM/
