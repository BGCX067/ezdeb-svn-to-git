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
#include<time.h>
#include<ctype.h>
#include <syslog.h>
#include <stdarg.h>


FILE *flog;
extern char logfile[256];

#include "dhisd.h"
#include "misc.h"
#include "ddb.h"


void strtolower(char *s) {
	
	while(*s!='\0') { *s = tolower(*s); s++; }
}


void off_nl(char *s) {

	while(*s!='\0' && *s!='\n' && *s!='\r') s++;
	*s='\0';
	return;
}

char *line_ptr(int idx,char *buff) {

	int i;
	static char m[1];


	idx--;
	m[0]='\0';

	while((*buff==' ' || *buff=='\t') && *buff!='\0' && *buff!='\n')
		buff++;
	if(*buff=='\0' || *buff=='\n') return(m);
		
	for(i=0;i<idx;i++) {
		while(*buff!=' ' && *buff!='\t' && *buff!='\0' &&
			*buff!='\n') buff++;
		if(*buff=='\0' || *buff=='\n') return(m);
		while((*buff==' ' || *buff=='\t') && *buff!='\0' && *buff!='\n')
			buff++;
		if(*buff=='\0' || *buff=='\n') return(m);
	}
	return(buff);
}

char *line_entry(int idx,char *buff) {

	static char b2[1024],*pb2;
	int i;

	idx--;
	b2[0]='\0';
	pb2=b2;

	while((*buff==' ' || *buff=='\t') && *buff!='\0' && *buff!='\n')
		buff++;
	if(*buff=='\0' || *buff=='\n') return(b2);
		
	for(i=0;i<idx;i++) {
		while(*buff!=' ' && *buff!='\t' && *buff!='\0' &&
			*buff!='\n') buff++;
		if(*buff=='\0' || *buff=='\n') return(b2);
		while((*buff==' ' || *buff=='\t') && *buff!='\0' && *buff!='\n')
			buff++;
		if(*buff=='\0' || *buff=='\n') return(b2);
	}
	while(*buff!=' ' && *buff!='\t' && *buff!='\0' && *buff!='\n') {
		*pb2 = *buff;
		buff++;
		pb2++;
	}
	*pb2 = '\0';
	return(b2);
}

int msg_log(const char *msg) {

	time_t	tt;
	char buff[256];

	DSYSLOG(1,(LOG_DEBUG,"msg_log(): Logging %s\n",msg));

	flog=fopen(logfile,"a");
	if(flog==NULL) return(0);
	tt=time(NULL);
	strcpy(buff,ctime(&tt));
	off_nl(buff);
	strcat(buff," : ");
	strcat(buff,msg);
	
#ifdef	WITH_MYSQL
	sql_log(buff);
#endif
	
	strcat(buff,"\n");
	fputs(buff,flog);
	fclose(flog);
	return(1);
}

/* R3 hashing function */
int r3_pass_encrypt(unsigned char *pass,int n) {
	
	int res=0;
	
	while(*pass!='\0') 
		res += (*pass++ * (n%256));
	res += n+ 8 + (n%100);
	return(res);
}

