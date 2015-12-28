# -
# Copyright (c) 1998-2008 DHIS, Dynamic Host Information System
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

CC=gcc

MYSQL_INCLUDE=	-I/usr/local/include/mysql -I/usr/local/mysql/include \
		-I/usr/local/mysql/include/mysql

MYSQL_LIBDIR=	-L/usr/local/lib/mysql -L/usr/local/mysql/lib \
		-L/usr/local/mysql/lib/mysql

WFLAGS=-Wall -Wstrict-prototypes -Wpointer-arith \
        -Wno-sign-compare -Wno-traditional -Wreturn-type -Wswitch -Wshadow \
        -Wwrite-strings -Wextra -funsigned-char 

# Possible CFLAGS: -g -O3 -DDONT_FORK
#

CFLAGS= -O3 $(WFLAGS) -I. -I/usr/local/include # $(MYSQL_INCLUDE) -DWITH_MYSQL=1
LFLAGS= -L/usr/local/lib # $(MYSQL_LIBDIR)

# For System V (Solaris) add -lnsl and -lsocket
# For MySQL support add -lmysqlclient
LIBS=-lgmp # -lmysqlclient #-lnsl -lsocket 

INSTALLBIN=/usr/local/sbin
INSTALLCONF=/usr/local/etc
INSTALLRC=/usr/local/etc/rc.d
LOGDIR=/var/log/dhis
PIDDIR=/var/run/dhis
CP=cp
OWNER=dhis
GROUP=dhis
MKDIR=mkdir -p
CHOWN=chown
CHGRP=chgrp
CHMOD=chmod


OBJS=qrc.o misc.o nsupdate.o ddb.o network.o online.o 
RM=rm -f

all: dhisd dhisgenkeys

dhisd:	$(OBJS) main.o
	$(CC) $(LFLAGS) -o dhisd $(OBJS) $(LIBS) main.o

dhisgenkeys: qrc.o genkeys.o
	$(CC) $(LFLAGS) -o dhisgenkeys qrc.o $(LIBS) genkeys.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

genkeys.o: genkeys.c
	$(CC) $(CFLAGS) -c genkeys.c

install: dhisd  dhisgenkeys
	strip dhisd dhisgenkeys
	$(MKDIR) $(INSTALLBIN)
	$(MKDIR) $(LOGDIR)
	$(MKDIR) $(PIDDIR)
	$(CP) dhisd $(INSTALLBIN)
	$(CP) dhisgenkeys $(INSTALLBIN)
	$(CHOWN) dhis $(INSTALLBIN)/dhisd $(INSTALLBIN)/dhisgenkeys $(LOGDIR) $(PIDDIR)
	$(CHGRP) dhis $(INSTALLBIN)/dhisd $(INSTALLBIN)/dhisgenkeys $(LOGDIR) $(PIDDIR)
	$(CHMOD) 770 $(INSTALLBIN)/dhisd $(INSTALLBIN)/dhisgenkeys $(LOGDIR) $(PIDDIR)
	$(CHMOD) u+s $(INSTALLBIN)/dhisd
	$(CP) dhis.db.sample dhisd.conf.sample $(INSTALLCONF)
	$(CHOWN) dhis $(INSTALLCONF)/dhis.db.sample $(INSTALLCONF)/dhisd.conf.sample
	$(CHGRP) dhis $(INSTALLCONF)/dhis.db.sample $(INSTALLCONF)/dhisd.conf.sample
	$(CHMOD) 640 $(INSTALLCONF)/dhis.db.sample $(INSTALLCONF)/dhisd.conf.sample
	$(CP) dhisd.rc $(INSTALLRC)/dhisd
	$(CHOWN) dhis $(INSTALLRC)/dhisd
	$(CHGRP) dhis $(INSTALLRC)/dhisd
	$(CHMOD) 755 $(INSTALLRC)/dhisd

clean:
	$(RM) dhisgenkeys *.core core dhisd *.o *~

