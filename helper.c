/*
 *  helper.c - setuid helper program for authbind
 *
 *  authbind is Copyright (C) 1998 Ian Jackson
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 * 
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef CONFIGDIR
# define CONFIGDIR "/etc/authbind"
#endif

static const char *rcsid="$Id: helper.c,v 1.5 1998-08-30 19:52:38 ian Exp $";

static void exiterrno(int xerrno) {
  exit(xerrno > 0 && xerrno < 128 ? xerrno : ENOSYS);
}

static void perrorfail(const char *msg) {
  int xerrno = errno;
  fprintf(stderr, "libauthbind's helper: %s: %s\n", msg, strerror(xerrno));
  exiterrno(xerrno);
}

static void badusage(void) {
  fprintf(stderr, "libauthbind's helper: bad usage\n (%s)\n", rcsid);
  exit(EINVAL);
}

static struct sockaddr_in saddr;

static void authorised(void) {
  if (bind(0, (struct sockaddr *) &saddr, sizeof(saddr))) {
    perrorfail("libauthbind's helper: bind() failed");

  } else {
    _exit(0);
  }
}

int main(int argc, const char *const *argv) {
  uid_t uid;
  char fnbuf[100];
  char *ep;
  const char *np;
  unsigned long addr, port, haddr, thaddr, thmask;
  unsigned int hport, a1,a2,a3,a4, alen,pmin,pmax;
  int nchar;
  FILE *file;

  if (argc != 3) badusage(); 
  addr= strtoul(argv[1],&ep,16); if (*ep || addr&~0x0ffffffffUL) badusage();
  port= strtoul(argv[2],&ep,16); if (*ep || port&~0x0ffffUL) badusage();
  hport= htons(port);

  if (hport >= IPPORT_RESERVED/2) {
     _exit(EPERM);
  }

  if (chdir(CONFIGDIR)) {
    perrorfail("chdir " CONFIGDIR);
  }

  fnbuf[sizeof(fnbuf)-1] = 0;
  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = port;
  saddr.sin_addr.s_addr = addr;

  snprintf(fnbuf,sizeof(fnbuf)-1, "byport/%u", hport);
  if (access(fnbuf, X_OK) == 0) {
    authorised();
  }

  if (errno != ENOENT) {
    fprintf(stderr, "libauthbind's helper: error checking %s: %s\n", fnbuf,
      strerror(errno));
    exiterrno(errno);
  }

  np = inet_ntoa(saddr.sin_addr);
  assert(np);

  snprintf(fnbuf, sizeof(fnbuf)-1, "byaddr/%s:%u", np, hport);
  if (access(fnbuf, X_OK) == 0) {
    authorised();
  }

  if (errno != ENOENT) {
    fprintf(stderr, "libauthbind's helper: error checking %s: %s\n", fnbuf,
      strerror(errno));
    exiterrno(errno);
  }

  uid = getuid();
  if (uid == (uid_t)-1) {
    perrorfail("getuid");
  }

  snprintf(fnbuf, sizeof(fnbuf)-1, "byuid/%lu", (unsigned long) uid);

  file = fopen(fnbuf,"r");
  if (file == NULL) {
    fprintf(stderr, "libauthbind's helper: error opening %s: %s\n", fnbuf,
      strerror(errno));
    exiterrno(errno == ENOENT ? EPERM : errno);
  }

  haddr = ntohl(addr);

  while (fgets(fnbuf, sizeof(fnbuf)-1, file)) {
    nchar= -1;
    sscanf(fnbuf," %u.%u.%u.%u/%u:%u,%u %n",
	   &a1,&a2,&a3,&a4,&alen,&pmin,&pmax,&nchar);
    if (nchar != strlen(fnbuf) ||
	alen>32 || pmin&~0x0ffff || pmax&~0x0ffff ||
	a1&~0x0ff || a2&~0xff || a3&~0x0ff || a4&~0x0ff)
      continue;
    
    if (hport<pmin || hport>pmax) continue;

    thaddr= (a1<<24)|(a2<<16)|(a3<<8)|(a4);
    thmask= 0x0ffffffffUL<<(32-alen);
    if ((haddr&thmask) != thaddr) continue;
    authorised();
  }

  if (ferror(file)) {
    perrorfail("read per-uid file");
  }

  _exit(ENOENT);
}
