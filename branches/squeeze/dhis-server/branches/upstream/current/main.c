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

#include<string.h>
#include<stdio.h>

#include "dhisd.h"
#include "online.h"
#include "network.h"
#include "ddb.h"
#include "qrc.h"
#include "misc.h"

/* Locally defined prototypes */
void sig_parse(int);
void sig_hup(int);
void sig_usr1(int);
void sig_usr2(int);
void sig_void(int);
void sig_chld(int);
void sig_term(int);
void usage(char *);


int do_reload=0;
int do_parse=0;
int debug_level = 0;

char logfile[256];
char pid_file[256];
char database_file[256];
char conf_file[256];

extern int udp_port;
extern char dhisd_bind_address[32];
extern int db_mode;

#ifdef	WITH_MYSQL
char sql_server[128];
char sql_user[128];
char sql_password[128];
char sql_dbase[128];
#endif


int read_conf(void) {

	FILE *fp;
	char str[256];

	DSYSLOG(1,(LOG_DEBUG,"read_conf(): Reading configuration File\n")); 

#ifdef	WITH_MYSQL
	sql_server[0]='\0';
	sql_user[0]='\0';
	sql_password[0]='\0';
	sql_dbase[0]='\0';
#endif
	
	fp=fopen(conf_file,"r");
	if(fp==NULL) return 0;
	while(fgets(str,255,fp)!=NULL) {
		char command[256];
		off_nl(str);
		strcpy(command,line_entry(1,str));
		strtolower(command);
		if(!strcmp(command,"bindaddress"))
			strncpy(dhisd_bind_address,line_entry(2,str),31);	
		if(!strcmp(command,"pidfile"))
			strncpy(pid_file,line_entry(2,str),255);	
		if(!strcmp(command,"logfile"))
			strncpy(logfile,line_entry(2,str),255);	
		if(!strcmp(command,"dbfile"))
			strncpy(database_file,line_entry(2,str),255);	
		if(!strcmp(command,"bindport"))
			udp_port=atoi(line_entry(2,str));		
#ifdef	WITH_MYSQL
		if(!strcmp(command,"mysqlserver"))
			strncpy(sql_server,line_entry(2,str),127);
		if(!strcmp(command,"mysqluser"))
			strncpy(sql_user,line_entry(2,str),127);
		if(!strcmp(command,"mysqlpassword"))
			strncpy(sql_password,line_entry(2,str),127);
		if(!strcmp(command,"mysqldbase"))
			strncpy(sql_dbase,line_entry(2,str),127);
#endif
	}
	fclose(fp);
	return 1;
}

void sig_parse(int s) {

	if(s!=SIGALRM) return;
	do_parse=1;
	signal(SIGALRM,sig_parse);
	alarm(PARSE_TIMEOUT);
}

void sig_hup(int s) {

	if(s!=SIGHUP) return;
	DSYSLOG(0,(LOG_INFO,"sig_hup(): Reload HUP Received\n"));
	do_reload=1;
	signal(SIGHUP,sig_hup);
}

void sig_void(int s) { 

	if(s!=SIGCHLD) return;	/* Just so that cc doesn't complain */
	return;
}

void sig_usr1(int s) {
	if(s!=SIGUSR1) return;
	debug_level++;
	syslog(LOG_DEBUG,"debug level %d\n", debug_level);
	signal(SIGUSR1,sig_usr1);
}

void sig_usr2(int s) {
	if(s!=SIGUSR2) return;
	debug_level = 0;
	syslog(LOG_DEBUG,"debug level %d\n", debug_level);
	signal(SIGUSR2,sig_usr2);
}


void sig_chld(int s) {

	if(s!=SIGCHLD) return;
	int r;
	wait(&r);
	signal(SIGCHLD,sig_chld);
}

void sig_term(int s) {

	if(s!=SIGTERM && s!=SIGKILL && s!=SIGINT && s!=SIGQUIT) return;
	signal(SIGCHLD,sig_void);
	on_free();
	net_close();
 	msg_log("SIGTERM received. Exiting ...");
	unlink(pid_file);
	exit(0);
}


void usage(char *s) {

	fprintf(stderr,"Syntax: %s [-D] [-P pid_file] [-l log_file] [-c config_file]\n",s); 

#ifdef	WITH_MYSQL
	fprintf(stderr,"        [-d dbfile|mysql] [-b ipv4_interface_address] [-p udp_port] \n");
#else
	fprintf(stderr,"        [-d dbfile] [-b ipv4_interface_address] [-p udp_port] \n");
#endif
	
	exit(0);
}

int main(int argc,char *argv[]) {

	FILE *fp;
	char str[128];
	extern char *optarg;
	extern int optreset;
	extern int optind;
	int c;

	strcpy(logfile,DHISD_LOG);
	strcpy(pid_file,DHISD_PID);
	strcpy(database_file,DHISD_DB_FILE);
	strcpy(conf_file,DHISD_CONF_FILE);

	while((c=getopt(argc,argv,"Dhp:P:l:b:d:c:")) !=EOF) {
		switch(c) {
			case('c'):  strcpy(conf_file,optarg);break;
		}
	}

	read_conf();
	optreset=1;
	optind=1;

	while((c=getopt(argc,argv,"Dhp:P:l:b:d:c:")) !=EOF) {
	switch(c) {
	case('l'):strcpy(logfile,optarg);break;
	case('P'):strcpy(pid_file,optarg);break;
	case('p'):udp_port=atoi(optarg);break;
	case('b'):strcpy(dhisd_bind_address,optarg);break;
	case('d'):strcpy(database_file,optarg);break;
	case('D'):debug_level++;break;
	case('h'): usage(argv[0]);
	case('c'):  strcpy(conf_file,optarg);break;
	default: usage(argv[0]);
	}
	}

	
#ifdef WITH_MYSQL	
	if(!strcmp(database_file,"mysql")) db_mode=DB_MYSQL;
	if(db_mode==DB_MYSQL && (sql_server[0]=='\0' || sql_user[0]=='\0' 
           || sql_password[0]=='\0' || sql_dbase[0]=='\0')) {
		syslog(LOG_ERR,"Unable to read all SQL login parameters. Cannot start server! Exiting ...");	
		fprintf(stderr,"Unable to read all SQL login parameters. Cannot start server! Exiting ...\n");

		exit(255);
	}				
#endif

#ifndef	DONT_FORK
	setsid();
	if(fork()) _exit(0);
#endif
	
	if(!db_reload()) {
#ifdef	WITH_MYSQL
		if(db_mode==DB_MYSQL) {

		// If MySQL read failed we may need to wait for the mysql server
		// to start. We try 3 times with 15 second intervals.
		int count; 
		for(count=0;count<3;count++) {
			sleep(15);
			if(db_reload()) break;
		}
		if(count==3) {	
		  syslog(LOG_ERR,"SQL database read failed: Please double-check your login data.");
		  exit(255);
		}

		} else {
#endif
		  syslog(LOG_ERR,"Unable to load database. Exiting ...");	
		  exit(255);
#ifdef	WITH_MYSQL
		}
#endif
	}


	if(net_init()) {
		syslog(LOG_ERR,"Unable to initialise network");

		DSYSLOG(1,(LOG_DEBUG,"main(): Failed to initialise network\n")); 

		exit(255);
	}

#ifndef	DONT_FORK
	close(0);
	close(1);
	close(2);
#endif

	unlink(pid_file);
	fp=fopen(pid_file,"w");
	if(fp!=NULL) {
		fprintf(fp,"%d",(int)getpid());
		fclose(fp);
	}

	

	sprintf(str,"Datagram Server Started [%d]",(int)getpid());
	msg_log(str);
	syslog(LOG_INFO,"DHIS Server Started");

	signal(SIGUSR1,sig_usr1);
	signal(SIGUSR2,sig_usr2);
	signal(SIGTERM,sig_term);
	signal(SIGKILL,sig_term);
	signal(SIGINT,sig_term);
	signal(SIGQUIT,sig_term);
	signal(SIGHUP,sig_hup);
	signal(SIGALRM,sig_parse);
	signal(SIGCHLD,sig_chld);
	alarm(PARSE_TIMEOUT);


	for(;;) {
		msg_t msg;
		int from;
		if(net_check_message())
		  if(net_read_message(&msg,&from)) 
			net_do_dgram(msg,from);
		
		if(do_reload) {
			db_reload();
			do_reload=0;
		}
		
		if(do_parse) {
			on_parse();
			do_parse=0;
		}
		usleep(100000);		// Sleep for 100 ms
	}
}

