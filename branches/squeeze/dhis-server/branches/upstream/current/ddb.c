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

#include "dhisd.h"
#include "qrc.h"
#include "ddb.h"
#include "misc.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<syslog.h>
#include<gmp.h>
#include<ctype.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<signal.h>
#include<netdb.h>
#include<time.h>
#include<sys/time.h>
#include<sys/utsname.h>
#include<unistd.h>
#include <sys/file.h>
#include<fcntl.h>

#ifdef	WITH_MYSQL
#include<mysql.h>
#endif

int db_mode=DB_FILE;

extern char database_file[256];
extern char conf_file[256];

struct dhis_rec *dBase=NULL;
#ifdef WITH_MYSQL
char sql_server[128];
char sql_user[128];
char sql_password[128];
char sql_dbase[128];

MYSQL *conn;      /* Our connection with SQL Server. */
MYSQL_RES *res;
MYSQL_ROW row;
int db_logged_in=0;
#endif

struct dhis_rec *new_rec(void) {

	struct dhis_rec *dp;
	dp=(struct dhis_rec *)malloc(sizeof(struct dhis_rec));
	if(dp==NULL) return(NULL);

	memset(dp,0,sizeof(struct dhis_rec));

	dp->status=1;
	dp->next=NULL;
	return(dp);
}

void free_rec(struct dhis_rec *dp) {
	
	if(dp->atype==AQRC) mpz_clear(dp->mauthn);
	free(dp);
}


struct dhis_rec *get_rec(int rid) {
	
	struct dhis_rec *r; r=dBase;
	
	if(dBase==NULL) {
		if(!db_reload()) {
#ifdef WITH_MYSQL
			sql_logout();
			sql_login();
#endif
			if(!db_reload()) return NULL;
		}
		if(dBase==NULL) return NULL;
	}

	while(r!=NULL) {
		if(r->hostid==rid) return r;
		r=r->next;
	}
	return NULL;
}

int db_reload(void) {

#ifdef WITH_MYSQL
	if(db_mode==DB_MYSQL) {
		return (sql_db_reload());
	}
#endif
	
	return (file_db_reload());
}

void db_free(void) {
	struct dhis_rec *r=dBase;
	while(dBase!=NULL) {
		r=dBase;
		dBase=dBase->next;
		r->next=NULL; 
		free_rec(r);
	}
	dBase=NULL;
	return;
}

char *db_password(int id) {

	static char pass[32];
	struct dhis_rec *dp;

	dp=get_rec(id);
	if(dp==NULL) return NULL;
	strcpy(pass,dp->hostpass);
	return pass;
}

int db_is_locked(int id) {

	struct dhis_rec *dp;
	dp=get_rec(id);
	if(dp==NULL) return 1;
	if(dp->status) return(0);
	else return(1);
}



int file_db_reload(void)  {
		
	FILE *fp;
	char str[1024];
	struct dhis_rec *rec;
	char keyn[1024];
	
	db_free();
		
	DSYSLOG(0,(LOG_INFO,"db_reload(): Loading DHIS database %s\n",database_file));
		
	fp=fopen(database_file,"r");
	
	if(fp==NULL) return 0;
	
	while(fgets(str,1024,fp)!=NULL) {
			off_nl(str);
			if(strchr(line_entry(1,str),';')==NULL)
				if(!strcmp(line_entry(2,str),"{")) {
					char cmd[64];
					char *cp;
					rec=new_rec();
					if(rec==NULL) { fclose(fp); return(0); }
					rec->hostid=atoi(line_entry(1,str));
					keyn[0]='\0';
					rec->refresh=NEXT_CHECK;
					do {
						if(fgets(str,1024,fp)==NULL) 
						{
							fclose(fp);
							free_rec(rec);
							return(0);
						}
						off_nl(str);
						
						strcpy(cmd,line_entry(1,str));
						strtolower(cmd);
						
						if(!strcmp(cmd,"hostname")) {
							strcpy(rec->hostname,line_entry(2,str));
							if((cp=strchr(rec->hostname,';'))!=NULL)
                                *cp='\0';
						}
						
						if(!strcmp(cmd,"hostpass")) {
							strcpy(rec->hostpass,line_entry(2,str));
							rec->atype=APASS;
							
							if((cp=strchr(rec->hostpass,';'))!=NULL)
                                *cp='\0';
							
						}
						
						if(!strcmp(cmd,"oncmd")) {
							strcpy(rec->oncmd,line_ptr(2,str));
							if((cp=strchr(rec->oncmd,';'))!=NULL)
                                *cp='\0';
						}
						if(!strcmp(cmd,"offcmd")) {
							strcpy(rec->offcmd,line_ptr(2,str));
							if((cp=strchr(rec->offcmd,';'))!=NULL)
                                *cp='\0';
						}
						
						if(!strcmp(cmd,"authn")) {
							strcat(keyn,line_entry(2,str));
							if((cp=strchr(keyn,';'))!=NULL)
                                *cp='\0';
							rec->atype=AQRC;
						}
						
					} while(strcmp(line_entry(1,str),"}"));
					
					if(!rec->atype) 
						free_rec(rec); 
					else
						if(rec->atype==AQRC && keyn[0]=='\0') 
								free_rec(rec); 
						else {
							if(rec->atype==AQRC) {
								mpz_init(rec->mauthn);
								mpz_set_str(rec->mauthn,keyn,10);
							}
							if(dBase==NULL) dBase=rec; 
							else { rec->next=dBase; dBase=rec; }
						}
					
					DSYSLOG(2,(LOG_DEBUG,"db_reload(): Read record %d\n",rec->hostid));
					
				} /* End if */
		} /* End while */
		fclose(fp);
		
		DSYSLOG(0,(LOG_INFO,"db_reload(): Database loaded\n"));
		
		return(1);
}

#ifdef WITH_MYSQL

int sql_login(void) {

	if(db_logged_in) return 1;
	if(sql_server[0]=='\0' || sql_user[0]=='\0' || sql_password[0]=='\0' || sql_dbase[0]=='\0') 
		return 0;

	conn = mysql_init(NULL);
	
	if (!mysql_real_connect(conn, sql_server,sql_user,sql_password,sql_dbase,0,NULL,0))
        return 0;
	
	db_logged_in=1;
	return 1;
}

void sql_logout(void) {
	
	if(!db_logged_in) return;
	db_logged_in=0;
	mysql_close(conn);
	return;
}


int sql_log(char *lstr) {
	
	char str[1024];
	if(!sql_login()) return 0;
	sprintf(str,"insert into DHISServerLog values(\"%s\")",lstr);
	if (mysql_query(conn,str)) return(1);
	res = mysql_use_result(conn);
	mysql_free_result(res);
	return(0);
}

int sql_update_lastlogin(int hostid) {
	
	char str[1024*8];
	if(!sql_login()) return 0;
	sprintf(str,"update DHISHost set DHISLastLogin=%ld where DHISHostID=%d",time(NULL),hostid);
	if (mysql_query(conn,str)) return(1);
	res = mysql_use_result(conn);
	mysql_free_result(res);
	return(0);
}	

int sql_db_reload(void) {
	
	char str[1024];
	struct dhis_rec *dp=NULL;
		
	db_free();
	if(!sql_login()) return 0;
	
	DSYSLOG(0,(LOG_INFO,"db_reload(): Loading DHIS SQL database\n"));
	
	sprintf(str,"select DHISHostID,DHISHostName,DHISHostPassword,DHISAuthN0,DHISAuthN1,DHISAuthN2,DHISAuthN3,DHISOnlineCmd,DHISOfflineCmd,DHISStatus from DHISHost");
	if (mysql_query(conn,str)) 
		return 0;
	
	res = mysql_use_result(conn);
	
	while((row = mysql_fetch_row(res)) != NULL) {
		
        	dp=new_rec();
        	if(dp==NULL) { 
			mysql_free_result(res);
			return 0;
		}
		
        	dp->hostid=atoi(row[0]);
		
		dp->atype=APASS;
		strcpy(dp->hostname,row[1]);
		strcpy(dp->hostpass,row[2]);
		if(dp->hostpass[0]!='\0') dp->atype=APASS;
		strcpy(dp->authn[0],row[3]);
		strcpy(dp->authn[1],row[4]);
		strcpy(dp->authn[2],row[5]);
		strcpy(dp->authn[3],row[6]);
		if(strlen(dp->authn[0])>10)  dp->atype=AQRC;
		strcpy(dp->oncmd,row[7]);
		strcpy(dp->offcmd,row[8]);
		dp->status=atoi(row[9]);

		dp->refresh=NEXT_CHECK;
		
        	if(dp->atype==AQRC) {
			char xauthn[1024];
			strcpy(xauthn,"");
			strcat(xauthn,dp->authn[0]);
			strcat(xauthn,dp->authn[1]);
			strcat(xauthn,dp->authn[2]);
			strcat(xauthn,dp->authn[3]);
			mpz_init(dp->mauthn);
			mpz_set_str(dp->mauthn,xauthn,10);
		}
		if(dBase==NULL) dBase=dp; 
		else { dp->next=dBase; dBase=dp; }
		
	}
	
	mysql_free_result(res);
		
	DSYSLOG(0,(LOG_INFO,"db_reload(): Database loaded\n"));
	return 1;
}

#endif

