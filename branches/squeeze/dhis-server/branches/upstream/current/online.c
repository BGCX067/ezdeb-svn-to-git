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
#include "nsupdate.h"
#include "online.h"
#include "network.h"
#include "misc.h"
#include "ddb.h"

#include <string.h>
#include <time.h>

online_t *onbase=NULL;

int on_add(int id,int addr,int port,int vers,int refresh) {

	online_t *p;

        int proto;
        if(vers<30) return 0;
        if(vers<40) proto=3;
        else if(vers<50) proto=4;
        else proto=5;

	p=onbase;
	if(p!=NULL)
	while(p->next!=NULL) p=p->next;
	if(p==NULL) {
		onbase=(online_t *)malloc(sizeof(online_t));
		p=onbase;
	} else {
		p->next=(online_t *)malloc(sizeof(online_t));
		p=p->next;
	}
	if(p==NULL) return(0);
	p->next=NULL;
	p->id=id;
	p->addr=addr;
	if(proto>=4) p->port=port;
	else p->port=DHISD_PORT;
	p->proto=proto;
	p->check_fails=0;
	p->sid=0;
	if(vers>=54) p->client_refresh=1;
	else p->client_refresh=0;

	if(refresh>=MIN_NEXT_CHECK && refresh <=MAX_NEXT_CHECK) {
		p->refresh=refresh;
	}
	else {

		if(refresh)
			DSYSLOG(1,(LOG_DEBUG,"on_add(): Refresh rate out of limits\n"));

		p->refresh=NEXT_CHECK;
	}
	if(p->proto>=4)
	p->ka=time(NULL) + p->refresh;
	else
	p->ka=time(NULL);

	srandom(time(NULL));
	while(!(p->sid=random()));
	
	DSYSLOG(1,(LOG_DEBUG,"on_add(): Marking Host %d Online\n",p->id));
	
	mark_online(p->id,p->addr);

	return(p->sid);
}

int on_delete(int id) {

	online_t *p1,*p2;

	p2=onbase;
	p1=onbase;
	while(p1!=NULL) {
		if(p1->id==id) break;
		p2=p1;
		p1=p1->next;
	}

	if(p1==NULL) return(0);

	if(p1==onbase) onbase=p1->next;
	else p2->next=p1->next;

	DSYSLOG(1,(LOG_DEBUG,"on_delete(): Marking Host %d Offline\n",p1->id));
	mark_offline(p1->id,p1->addr);
	free(p1);
	return(1);
}

int on_update(int id,int addr,int port,int vers,int refresh) {

	online_t *p;
	int oaddr;


	int proto; 
	if(vers<30) return 0;
	else if(vers<40) proto=3;
	else if(vers<50) proto=4;
	else proto=5;

	p=onbase;
	while(p!=NULL) {
		if(p->id==id) break;
		p=p->next;
	}

	if(p==NULL) {
		return(on_add(id,addr,port,vers,refresh));
	}

	if(proto>=4) {
	p->ka=time(NULL)+p->refresh;
	p->check_fails=0;
	}
	else
	p->ka=time(NULL);

	if(p->addr==addr) return(p->sid);
	
	DSYSLOG(1,(LOG_DEBUG,"on_update(): Updating host %d \n",p->id));

	oaddr= p->addr;
	p->addr=addr;
	mark_update(p->id,p->addr,oaddr);

	return(p->sid);
}

int on_parse(void) {

	online_t *p1,*p2;
	time_t t = time(NULL);

	DSYSLOG(2,(LOG_DEBUG,"on_parse(): Executing.\n"));

	p2=onbase;
	p1=onbase;
	while(p1!=NULL) {


		p2=p1->next;
		if(db_is_locked(p1->id)) {
			on_delete(p1->id);
			p1=p2;
			continue;
		}
		if(p1->proto==3) 
		if((t  - p1->ka) > KA_OFFLINE)
			on_delete(p1->id);
		if(p1->proto==4 || p1->proto ==5) {
		if(p1->check_fails >= CHECK_FAILS)
			on_delete(p1->id);
		else {
			if (p1->proto == 4) {
			if(p1->ka < t) {
			r4_check_req_t m;

			DSYSLOG(1,(LOG_DEBUG,"on_parse(): Sending R4_CHECK_REQ to %d\n",
			     p1->id)); 

			m.hdr.opcode=R4_CHECK_REQ;
			m.next_check=p1->refresh;
			net_write_message((msg_t *)&m,p1->addr,p1->port);
			p1->check_fails++;
			} /* end if ka */
			} /* end proto */

			if(p1->proto==5) {
			if(p1->ka < t) {
			  if(p1->client_refresh) {
				p1->ka=time(NULL)+p1->refresh;
			  } else {
				check_req_t m;

				DSYSLOG(1,(LOG_DEBUG,"on_parse(): Sending CHECK_REQ to %d\n",p1->id));

				m.hdr.opcode=CHECK_REQ;
				m.hdr.hostid=p1->id;
				m.next_check=p1->refresh;
				net_write_message((msg_t *)&m,p1->addr,p1->port);
			  }
				p1->check_fails++;
			} /* end if ka */
			} /* end proto 5 */
		}}
		p1=p2;
	}
	return(1);
}

void on_free(void) {

	online_t *op;

	op=onbase;
	while(op!=NULL) {
		mark_offline(op->id,op->addr);
		op=op->next;
		free(onbase);
		onbase=op;
	}
}



void mark_online(int id,int addr) {

	struct in_addr sa;
        char addrs[32];
	char str[512];
	struct dhis_rec *dp;
        sa.s_addr=addr;
        strcpy(addrs,inet_ntoa((struct in_addr)sa));

	dp=get_rec(id);
        DSYSLOG(1,(LOG_DEBUG,"mark_online() Got record from databse"));

	if(dp==NULL) return;
	if(!dp->status) return;

        DSYSLOG(1,(LOG_DEBUG,"mark_online() %d %s\n",id,addrs));

	dns_update(NSUPDATE_DEL,dp->hostname,"");
	dns_update(NSUPDATE_ADD,dp->hostname,addrs);

	sprintf(str,"-> online (%d) %s [%s]",dp->hostid,dp->hostname,addrs);
	msg_log(str);

#ifdef WITH_MYSQL
	sql_update_lastlogin(dp->hostid);
#endif

	if(dp->oncmd!=NULL) 
	if(dp->oncmd[0]!='\0')
		fork_cmd(dp->oncmd,id,addrs);
        
	return;
}
	
void mark_offline(int id,int addr) {

        struct in_addr sa;
        char addrs[32];
	char str[512];
        struct dhis_rec *dp;

        sa.s_addr=addr;
        strcpy(addrs,inet_ntoa((struct in_addr)sa));

	dp=get_rec(id);
        if(dp==NULL) return;

        DSYSLOG(1,(LOG_DEBUG,"mark_offline() %d %s\n",id,addrs));

	dns_update(NSUPDATE_DEL,dp->hostname,"");
	dns_update(NSUPDATE_ADD,dp->hostname,DYN_IP);

	sprintf(str,"-> offline (%d) %s [%s]",dp->hostid,dp->hostname,addrs);
	msg_log(str);

        if(dp->offcmd!=NULL) 
        if(dp->offcmd[0]!='\0')
        	fork_cmd(dp->offcmd,id,addrs);
        
        return;
}

void mark_update(int id,int addr,int oaddr) {

        struct in_addr sa,osa;
        char addrs[32],oaddrs[32];
	char str[512];
        struct dhis_rec *dp;

        sa.s_addr=addr;
        osa.s_addr=oaddr;
        strcpy(addrs,inet_ntoa((struct in_addr)sa));
        strcpy(oaddrs,inet_ntoa((struct in_addr)osa));

	dp=get_rec(id);

        if(dp==NULL) return;
        if(!dp->status) return;

        DSYSLOG(1,(LOG_DEBUG,"mark_update() %d %s -> %s\n",id,oaddrs,addrs));

	dns_update(NSUPDATE_DEL,dp->hostname,"");
	dns_update(NSUPDATE_ADD,dp->hostname,addrs);

	sprintf(str,"-> update (%d) %s [%s -> %s]",dp->hostid,dp->hostname,
		oaddrs,addrs);
	msg_log(str);

        if(dp->offcmd!=NULL) 
        if(dp->offcmd[0]!='\0')
        	fork_cmd(dp->offcmd,id,addrs);

	sleep(2);	// Wait a little bit

        if(dp->oncmd!=NULL)
        if(dp->oncmd[0]!='\0')
                fork_cmd(dp->oncmd,id,addrs);

        return;
}

void fork_cmd(char *cmd,int id,char *addr) {

        char line[1024];
        char path[1024];

        strcpy(path,line_entry(1,cmd));
        sprintf(line,"%s %d %s %s",path,id,addr,line_ptr(2,cmd));

        if(!fork()) {
                system(line);
                exit(0);
        }
        return;
}


