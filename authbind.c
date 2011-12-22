/*
 *  authbind.c - main invoker program
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "authbind.h"

static const char *rcsid="$Id: authbind.c,v 1.5 1998-08-30 19:12:00 ian Exp $";

static void printusage(FILE *f) {
  if (fprintf(f,
	      "usage:       authbind [<options>] <program> <arg> <arg> ...\n"
	      "options:     --deep    --depth <levels>\n"
	      "version:     " MAJOR_VER "." MINOR_VER "  %s\n",
	      rcsid) == EOF) { perror("printf usage"); exit(-1); }
}

static void usageerror(const char *msg) {
  fprintf(stderr, "usage error: %s\n", msg);
  printusage(stderr);
  exit(-1);
}

static void mustsetenv(const char *var, const char *val) {
  if (setenv(var, val, 1)) {
    perror("authbind: setenv");
    exit(-1);
  }
}

int main(int argc, char *const *argv) {
  const char *expreload, *authbindlib, *preload;
  char *newpreload, *ep;
  char buf[50];
  unsigned long depth;

  depth= 1;
  while (argc>1 && argv[1][0]=='-') {
    argc--; argv++;
    if (!argv[0][1]) break;
    if (!strcmp("--deep",argv[0])) { depth= 0; }
    else if (!strcmp("--depth",argv[0])) {
      if (argc<=1) usageerror("--depth requires a value");
      argc--; argv++;
      depth= strtoul(argv[0],&ep,10);
      if (*ep || depth<=0 || depth>100) usageerror("--depth value is not valid");
    } else if (!strcmp("--help",argv[0]) || !strcmp("--help",argv[0])) {
      printusage(stdout);
      exit(0);
    }
  }

  if (argc < 2) {
    usageerror("need program name");
  }

  authbindlib = getenv(AUTHBINDLIB_VAR);
  if (authbindlib == NULL) {
    authbindlib = LIBAUTHBIND;
    mustsetenv(AUTHBINDLIB_VAR, authbindlib);
  }
    
  expreload = getenv(PRELOAD_VAR);
  if (expreload != NULL) {
    /* XXX Need to check for NULL return value */
    newpreload = malloc(strlen(expreload) + strlen(authbindlib) + 2);
    strcpy(newpreload, expreload);
    strcat(newpreload, ":");
    strcat(newpreload, authbindlib);
    preload = newpreload;

  } else {
    preload = authbindlib;
  }

  mustsetenv(PRELOAD_VAR, preload);

  /* XX Mac OSX hack */
  mustsetenv("DYLD_FORCE_FLAT_NAMESPACE", "1");

  if (depth > 1) {
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "%ld", depth-1);
    mustsetenv(AUTHBIND_LEVELS_VAR, buf);

  } else if (depth == 0) {
    mustsetenv(AUTHBIND_LEVELS_VAR, "y");

  } else {
    assert(depth==1);
  }

  execvp(argv[1], argv+1);
  perror(argv[1]);
  exit(-1);
}
