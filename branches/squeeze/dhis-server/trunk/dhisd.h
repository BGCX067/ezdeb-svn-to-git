/*-
 * Copyright (c) 1998-2008 DHIS, Dynamic Host Information System
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<time.h>
#include<sys/time.h>
#include<sys/types.h>
#include<netdb.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<arpa/nameser.h>
#include<resolv.h>
#include<sys/utsname.h>
#include<unistd.h>
#include<signal.h>
#include<sys/signal.h>
#include<sys/wait.h>
#include<syslog.h>
// #include<varargs.h>
#include<gmp.h>

#define	DHISD_CONF_FILE		"/usr/local/etc/dhisd.conf"
#define	DHISD_DB_FILE		"/usr/local/etc/dhis.db"

#define	DHISD_PID		"/var/run/dhis/dhisd.pid"
#define	DHISD_LOG		"/var/log/dhis/dhisd.log"

#define	DHISD_PORT	58800

#define	DHIS_VERSION		55	/* Current version */
#define	DHIS_RELEASE		55	/* Current release */
#define	DHIS_R4			40	/* R4 starting version */
#define	DHIS_MIN_VERSION	30	/* Minimum required version to run */

#define	KA_OFFLINE		200	/* timeout for R3 clients */
#define	PARSE_TIMEOUT		10	/* frequency of parse  in sec */
#define	MIN_NEXT_CHECK		60	/* mim allowed refresh */
#define	MAX_NEXT_CHECK		3600 	/* max allowed refresh secs */
#define	CHECK_FAILS		3	/* maximum check fails */
#define NEXT_CHECK      	60      /* default refresh */

#define	MAX_HOSTNAME	64
#define	MAX_PASS	16

/* R3 messages */
#define	R3_ONLINE_REQ	0x00000311
#define	R3_OFFLINE_REQ	0x00000312


/* R4 messages */
#define	R4_ECHO_REQ	0x00000411
#define	R4_ECHO_ACK	0x00000412
#define	R4_AUTH_REQ	0x00000421
#define	R4_AUTH_DENY	0x00000422
#define	R4_AUTH_ACK	0x00000423
#define	R4_AUTH_SX	0x00000424
#define	R4_AUTH_SY	0x00000425
#define	R4_CHECK_REQ	0x00000441
#define	R4_CHECK_ACK	0x00000442
#define	R4_OFFLINE_REQ	0x00000451

/* R5 messages */
#define	ECHO_REQ	0x00000511
#define	ECHO_ACK	0x00000512
#define	AUTH_REQ	0x00000521
#define	AUTH_DENY	0x00000522
#define	AUTH_ACK	0x00000523
#define	AUTH_SX		0x00000524
#define	AUTH_SY		0x00000525
#define	R51_AUTH_ACK	0x00000526
#define	CHECK_REQ	0x00000541
#define	CHECK_ACK	0x00000542
#define	OFFLINE_REQ	0x00000551

extern int debug_level;

#define DSYSLOG(d,m)	\
  do {			\
    if (debug_level>=d)	\
      syslog m;		\
  } while(0)


