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
#include "network.h"
#include "ddb.h"
#include "online.h"
#include "qrc.h"
#include "misc.h"

int udp_sock=-1;
int udp_port=DHISD_PORT;
char dhisd_bind_address[32]="0.0.0.0";

extern online_t *onbase;


/*
 * msg_size_by_opcode() - Returns the size of a structure of opcode type
 *
 */
int msg_size_by_opcode(int opcode) {

	switch(opcode) {
	case(ECHO_REQ): return(sizeof(echo_req_t));
	case(ECHO_ACK): return(sizeof(echo_ack_t));
	case(AUTH_REQ): return(sizeof(auth_req_t));
	case(AUTH_ACK): return(sizeof(auth_ack_t));
	case(R51_AUTH_ACK): return(sizeof(auth_ack_51_t));
	case(AUTH_DENY): return(sizeof(auth_deny_t));
	case(AUTH_SX): return(sizeof(auth_sendx_t));
	case(AUTH_SY): return(sizeof(auth_sendy_t));
	case(CHECK_REQ): return(sizeof(check_req_t));
	case(CHECK_ACK): return(sizeof(check_ack_t));
	case(OFFLINE_REQ): return(sizeof(offline_req_t));

	case(R4_ECHO_REQ): return(sizeof(r4_echo_req_t));
	case(R4_ECHO_ACK): return(sizeof(r4_echo_ack_t));
	case(R4_AUTH_REQ): return(sizeof(r4_auth_req_t));
	case(R4_AUTH_ACK): return(sizeof(r4_auth_ack_t));
	case(R4_AUTH_DENY): return(sizeof(r4_auth_deny_t));
	case(R4_AUTH_SX): return(sizeof(r4_auth_sendx_t));
	case(R4_AUTH_SY): return(sizeof(r4_auth_sendy_t));
	case(R4_CHECK_REQ): return(sizeof(r4_check_req_t));
	case(R4_CHECK_ACK): return(sizeof(r4_check_ack_t));
	case(R4_OFFLINE_REQ): return(sizeof(r4_offline_req_t));

	case(R3_ONLINE_REQ): return(sizeof(r3_online_req_t));
	case(R3_OFFLINE_REQ): return(sizeof(r3_offline_req_t));
	default:	return(0);
	}
}

/*
 * swap_int() - Swaps the byte order of an integer 
 *
 */
void swap_int(int *n) {

        unsigned char *p,a,b,c,d;
        p=(unsigned char *)n;
        a=*p++;b=*p++;c=*p++;d=*p;
        p=(unsigned char *)n;
        *p++ = d;*p++ = c;*p++ = b;*p = a;
}

/*
 * swap_msg() - Calls swap_int to n members of the message 
 *
 */
void swap_msg(int *m,int n) {

        int i;
        for(i=0;i<n;i++) {
                swap_int(m);
                m++;
        }
}

/*
 * little_entian() - Checks if the system is little endian. 
 *                   Returns 1 if so or 0 if not 
 *
 */
int little_endian(void) {

        int a=1;
        unsigned char *p;
        p=(unsigned char *)&a;
        if((int)p[0]==1) return(1);
        return(0);
}

/* 
 * convert_message() - Converts a message to big/little endian as required 
 *
 */
void convert_message(msg_t *p,int mode) {

	int opcode=0;

	if(mode==1)  	opcode=p->hdr.opcode;
	swap_msg((int *)&(p->hdr),4);
	if(mode==2) opcode=p->hdr.opcode;

        if(opcode >= ECHO_REQ ) 
		swap_int((int *)&(p->hdr.hostid));

	
	switch(opcode) {
	

	case(ECHO_REQ): break;
	case(ECHO_ACK): { echo_ack_t *p2; p2=(echo_ack_t *)p;
			  swap_int((int *)&(p2->oserial));
			  break;
			}

	case(AUTH_REQ):  { auth_req_t *p2; p2=(auth_req_t *)p;
			  swap_int((int *)&(p2->refresh));
			  break;
			}
	case(AUTH_ACK):  { auth_ack_t *p2; p2=(auth_ack_t *)p;
			  swap_int((int *)&(p2->sid));
			  break;
			}
	case(R51_AUTH_ACK):  { auth_ack_51_t *p2; p2=(auth_ack_51_t *)p;
			  swap_int((int *)&(p2->sid));
			  break;
			}
	case(AUTH_DENY): break;
	case(AUTH_SX): { auth_sendx_t *p2; p2=(auth_sendx_t *)p;
			  break;
			}
			 
	case(AUTH_SY): break;

	case(CHECK_REQ):  { check_req_t *p2; p2=(check_req_t *)p;
			  swap_int((int *)&(p2->next_check));
			  break;
			}

	case(CHECK_ACK):  { check_ack_t *p2; p2=(check_ack_t *)p;
			  swap_int((int *)&(p2->sid));
			  break;
			}

	case(OFFLINE_REQ):  { offline_req_t *p2; p2=(offline_req_t *)p;
			  swap_int((int *)&(p2->sid));
			  break;
			}

	/* R4 messages */
	case(R4_ECHO_REQ): break;
	case(R4_ECHO_ACK): { r4_echo_ack_t *p2; p2=(r4_echo_ack_t *)p;
			  swap_int((int *)&(p2->oserial));
			  break;
			}

	case(R4_AUTH_REQ):  { r4_auth_req_t *p2; p2=(r4_auth_req_t *)p;
			  swap_int((int *)&(p2->id));
			  break;
			}
	case(R4_AUTH_ACK):  { r4_auth_ack_t *p2; p2=(r4_auth_ack_t *)p;
			  swap_int((int *)&(p2->sid));
			  break;
			}
	case(R4_AUTH_DENY): break;
	case(R4_AUTH_SX): { r4_auth_sendx_t *p2; p2=(r4_auth_sendx_t *)p;
			  swap_int((int *)&(p2->id));
			  break;
			}
			 
	case(R4_AUTH_SY): break;

	case(R4_CHECK_REQ):  { r4_check_req_t *p2; p2=(r4_check_req_t *)p;
			  swap_int((int *)&(p2->next_check));
			  break;
			}

	case(R4_CHECK_ACK):  { r4_check_ack_t *p2; p2=(r4_check_ack_t *)p;
			  swap_int((int *)&(p2->sid));
			  break;
			}

	case(R4_OFFLINE_REQ):  { r4_offline_req_t *p2; p2=(r4_offline_req_t *)p;
			  swap_int((int *)&(p2->sid));
			  break;
			}


	/* R3 messages */
	case(R3_OFFLINE_REQ):  { r3_offline_req_t *p2; p2=(r3_offline_req_t *)p;
			  swap_int((int *)&(p2->id));
			  swap_int((int *)&(p2->pass));
			  break;
			}
	case(R3_ONLINE_REQ):  { r3_online_req_t *p2; p2=(r3_online_req_t *)p;
			  swap_int((int *)&(p2->id));
			  swap_int((int *)&(p2->pass));
			  break;
			}

	}
	return;
}

int get_serial(void) { static int s=0; return(++s); }

/*
 * net_init() - Initialises the socket descriptor for receiving
 *              UDP connections
 *
 * Updates:   udp_sock only
 *
 * Returns:   0 on success, 1 on error
 *
 */

int net_init(void) {

        struct sockaddr_in sa;


        /* Create UDP socket */
        udp_sock=socket(AF_INET,SOCK_DGRAM,0);
        if(udp_sock<0) return(1);

        /* Bind the UDP socket */
        sa.sin_family=AF_INET;
        sa.sin_port=htons(udp_port);
        sa.sin_addr.s_addr=inet_addr(dhisd_bind_address);
        if(bind(udp_sock,(struct sockaddr *)&sa,sizeof(struct sockaddr_in)))
        { 
                close(udp_sock);
                return(1);
        }

        /* UDP socket is ready to receive messages */
        return(0);
}

/*
 * net_close() - Closes sockets associated with UDP incoming ports
 *
 * Updates:   udp_sock only
 *
 * Returns:   0
 *
 */
int net_close(void) {

        close(udp_sock);
        return(0);
}

/* 
 * net_check_message() - Returns 1 if there is a message to be read or 0
 *                           otherwise.
 *
 */
int net_check_message(void) {

   	fd_set readfds;
        struct timeval tv;

        /* Prepare for select */
        FD_ZERO(&readfds);
        FD_SET(udp_sock,&readfds);
        tv.tv_sec=0;
        tv.tv_usec=0;

        /* Check for new messages */
        if(select(udp_sock+1,&readfds,NULL,NULL,&tv)==-1) return(0);
        if(!FD_ISSET(udp_sock,&readfds)) return(0);
	return(1);
}

/* 
 * net_read_message() - Reads a message into *p and returns length read
 *			or 0 on error.
 *
 */
int net_read_message(msg_t *p,int *from) {

	int n;
	socklen_t sl;
	struct sockaddr_in sa;

        /* Read message */
        sl=sizeof(struct sockaddr_in);
        n=recvfrom(udp_sock,p,MAX_MSG,0,(struct sockaddr *)&sa,&sl);
	if(n<=0 || n >MAX_MSG) return(0);

	DSYSLOG(1,(LOG_DEBUG,"net_read_message(): Message arrived from %s\n",
		inet_ntoa(sa.sin_addr)));
	
	/* Convert to big endian if necessary */
	if(little_endian()) convert_message(p,2);
	memcpy(from,&sa.sin_addr,sizeof(struct in_addr));

	if(p->hdr.version>=54)
		p->hdr.rport=ntohs(sa.sin_port);

	
	return(n);
}

/* 
 * net_write_message() - Writes a message from *p and returns the number of
 *                       bytes sent or 0 on error.
 *
 */
int net_write_message(msg_t *p,int toaddr,int toport) {

 	struct sockaddr_in sa;
	int len;
	int r;
	struct in_addr sai;

	sai.s_addr=toaddr;
	DSYSLOG(1,(LOG_DEBUG,"net_write_message(): Sending Message to %s\n",
		inet_ntoa(sai)));

	p->hdr.version=DHIS_VERSION;
	p->hdr.serial=get_serial();
	p->hdr.rport=udp_port;
	
        /* set destination */
        sa.sin_family=AF_INET;
        sa.sin_port=htons(toport);
        sa.sin_addr.s_addr=toaddr;

	/* Get message size */
	len=msg_size_by_opcode(p->hdr.opcode);

	/* Convert to big endian if necessary */
	if(little_endian()) convert_message(p,1);

        /* Send message request */
        r=sendto(udp_sock,(unsigned char *)p,len,0,(struct sockaddr *)&sa,
		sizeof(struct sockaddr_in));

	/* Convert back just in case */
	if(little_endian()) convert_message(p,2);

	return(r);

}


int net_do_dgram(msg_t msg,int from) {
	
	DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Processing packet [OPCODE=%x] started\n",
			   msg.hdr.opcode));
	
	/* First lets take care of DHIS R3 compatible messages */
	
	if(msg.hdr.version<DHIS_MIN_VERSION) return(0);
	
	if(msg.hdr.version<DHIS_R4) { /* DHIS R3 here */
		
		char *p;
		r3_online_req_t *m;	/* Same as offline_req anyway */	
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Message is in R3 format\n"));
		
		m=(r3_online_req_t *)&msg;
		if(msg.hdr.opcode!=R3_ONLINE_REQ && msg.hdr.opcode!=R3_OFFLINE_REQ) {
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Message opcode is unsupported. Discarded.\n"));
			
			return(0);
		}
		
		p=db_password(m->id);
		if(p==NULL) { 
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): HostID does not match database.\n"));
			
			return(0);
		}
		
		if(r3_pass_encrypt((unsigned char *)p,msg.hdr.rport)!=m->pass) {
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Security: Invalid password. Discarded.\n"));
			
			return(0);
		}
		if(msg.hdr.opcode==R3_ONLINE_REQ) {
			
			if(db_is_locked(m->id)) {
				DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Online Broadcast: But record is locked!.\n"));
			}
			else {
				DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Online Broadcast: callind on_update().\n"));
				
				on_update(m->id,from,0,msg.hdr.version,0);
			}
		}
		if(msg.hdr.opcode==R3_OFFLINE_REQ) {
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Offline Broadcast: callind on_delete().\n"));
			
			on_delete(m->id);
		}
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): End of R3 message processing.\n"));
		
		return(1);
	} /* end R3 */
	
	/* Now lets process R4 messages */
	
	
	if(msg.hdr.version>=40 && msg.hdr.version < 50)
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Message is in R4 format\n"));
	
	if(msg.hdr.opcode==R4_OFFLINE_REQ) {
		r4_offline_req_t *mp;
		online_t *op;
		mp=(r4_offline_req_t *)&msg;
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): R4_OFFLINE_REQ received\n"));
		
		op=onbase;
		while(op!=NULL) { 
			if(op->sid==mp->sid && op->addr==from) break;
			op=op->next;
		}
		if(op!=NULL) {
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Calling on_delete() for %d\n",op->id));
			
			on_delete(op->id);
		}
		else {
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Security: Invalid addr or sid. Discarded.\n"));
		}
		return(1);
	}
	if(msg.hdr.opcode==R4_ECHO_REQ) {
		r4_echo_ack_t m;
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): R4_ECHO_REQ received. Sending ECHO_ACK.\n"));
		
		m.hdr.opcode=R4_ECHO_ACK;
		m.oserial=msg.hdr.serial;
		net_write_message((msg_t *)&m,from,msg.hdr.rport);
		return(1);
	}
	
	if(msg.hdr.opcode==R4_CHECK_ACK) {
		online_t *p;
		r4_check_ack_t *mp;
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Received R4_CHECK_ACK.\n"));
		
		mp=(r4_check_ack_t *)&msg;
		p=onbase;	
		while(p!=NULL) {
			if(p->addr==from) break;
			p=p->next;
		} 
		if(p==NULL) return(0);
		if(mp->sid!=p->sid) return(0); 
		on_update(p->id,from,msg.hdr.rport,msg.hdr.version,0);
		p->ka=time(NULL) + NEXT_CHECK;
		p->check_fails=0;
		return(1);
	}
	
	if(msg.hdr.opcode==R4_AUTH_SX) {
		struct dhis_rec *dbp;
		r4_auth_sendx_t *mp;
		mpz_t x;
		char buff[1024];
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Received R4_AUTH_SX.\n"));
		
		mp=(r4_auth_sendx_t *)&msg;
		dbp=get_rec(mp->id);
		
		if(dbp==NULL) {
			r4_auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending R4_AUTH_DENY. Invalid ID.\n"));
			
			m.hdr.opcode=R4_AUTH_DENY;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(0);
		}
		if(!dbp->status) {
			r4_auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending R4_AUTH_DENY. Locked.\n"));
			
			m.hdr.opcode=R4_AUTH_DENY;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(0);
		}
		
		if(dbp->atype!=AQRC || dbp->xstage==0) {
			r4_auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending R4_AUTH_DENY. Not in X waiting mode.\n"));
			
			m.hdr.opcode=R4_AUTH_DENY;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(0);
		}
		mpz_init(x);
		memcpy(buff,mp->x,200);
		buff[200]='\0';
		mpz_set_str(x,buff,10);
		if(mpz_cmp(x,dbp->x)) {
			r4_auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending R4_AUTH_DENY. X and X do not match.\n"));
			
			m.hdr.opcode=R4_AUTH_DENY;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			if(dbp->xstage) mpz_clear(dbp->x);
			dbp->xstage=0;
			mpz_clear(x);
			return(0);
		} else {
			r4_auth_ack_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending R4_AUTH_ACK. X and X match.\n"));
			
			mpz_clear(x);
			mpz_clear(dbp->x);
			dbp->xstage=0;
			m.hdr.opcode=R4_AUTH_ACK;
			if((m.sid=on_update(dbp->hostid,from,msg.hdr.rport,msg.hdr.version,0))!=0) {
				net_write_message((msg_t *)&m,from,msg.hdr.rport);
				return(1);
			} else return(0);
		}
	}
	
	if(msg.hdr.opcode==R4_AUTH_REQ) {
		struct dhis_rec *dbp;
		r4_auth_req_t *mp;
		mp=(r4_auth_req_t *)&msg;
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Received R4_AUTH_REQ for %d\n",mp->id));
		
		dbp=get_rec(mp->id);
		if(dbp==NULL) {
			r4_auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending R4_AUTH_DENY. Invalid ID.\n"));
			
			m.hdr.opcode=R4_AUTH_DENY;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(0);
		}
		if(!dbp->status) {
			r4_auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending R4_AUTH_DENY. Locked.\n"));
			
			m.hdr.opcode=R4_AUTH_DENY;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(0);
		}
		mp->pass[MAX_PASS-1]='\0';
		if(dbp->atype==APASS) {
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Password Authentication.\n"));
			
			if(strcmp(mp->pass,dbp->hostpass)) {
				r4_auth_deny_t m;
				
				DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending R4_AUTH_DENY. Invalid Password.\n"));
				
				m.hdr.opcode=R4_AUTH_DENY;
				net_write_message((msg_t *)&m,from,msg.hdr.rport);
				return(0);
			} else {
				r4_auth_ack_t m;
				
				DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending R4_AUTH_ACK.\n"));
				
				m.hdr.opcode=R4_AUTH_ACK;
				if((m.sid=on_update(dbp->hostid,from,msg.hdr.rport,msg.hdr.version,0))!=0) {
					net_write_message((msg_t *)&m,from,msg.hdr.rport);
					return(1);
				} else return(0);
			}
		}
		if(dbp->atype==AQRC) { 
			r4_auth_sendy_t m;
			mpz_t y;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): QRC Authentication.\n"));
			
			if(dbp->xstage==1) { dbp->xstage=0; mpz_clear(dbp->x);}
			m.hdr.opcode=R4_AUTH_SY;
			dbp->xstage=1;
			mpz_init(dbp->x);
			qrc_genx(dbp->x,dbp->mauthn);
			mpz_init(y);
			qrc_geny(y,dbp->x,dbp->mauthn);
			qrc_fill_str(y,m.y,200);
			mpz_clear(y);
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending R4_AUTH_SY.\n"));
			
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(1);
		}
		return(0);
		
	}
	
	/* Finally process R5 messages */
	
	
	if(msg.hdr.version>=50)
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Message is in R5 format\n"));
	
	if(msg.hdr.opcode==OFFLINE_REQ) {
		offline_req_t *mp;
		online_t *op;
		mp=(offline_req_t *)&msg;
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): OFFLINE_REQ received\n"));
		
		op=onbase;
		while(op!=NULL) { 
			if(op->sid==mp->sid && op->addr==from) break;
			op=op->next;
		}
		if(op!=NULL) { 
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Calling on_delete() for %d\n",op->id));
			
			on_delete(op->id);
		} 
		else {
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Security: Invalid addr or sid. Discarded.\n"));
		}
		return(1);
	}
	
	if(msg.hdr.opcode==ECHO_REQ) {
		echo_ack_t m;
		online_t *p;
		echo_req_t *mp;
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): ECHO_REQ received. Sending ECHO_ACK.\n"));
		
		m.hdr.opcode=ECHO_ACK;
		m.oserial=msg.hdr.serial;
		m.hdr.hostid=msg.hdr.hostid;
		
		mp=(echo_req_t *)&msg;
		p=onbase;
		while(p!=NULL) {
			if(p->id==mp->hdr.hostid) break;
			p=p->next;
		}
		
		if(p==NULL) {
			if(msg.hdr.version>=54) 
				m.oserial=0;	
		} else {
			if(p->client_refresh && from!=p->addr) 
				m.oserial=0;	
		}

		net_write_message((msg_t *)&m,from,msg.hdr.rport);

		if(p==NULL) return(0);

		if(p->client_refresh && from==p->addr) {
			on_update(p->id,from,msg.hdr.rport,msg.hdr.version,0);
			p->ka=time(NULL) + p->refresh;
			p->check_fails=0;
		}
		return(1);
	}
	
	if(msg.hdr.opcode==CHECK_ACK) {
		online_t *p;
		check_ack_t *mp;
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Received CHECK_ACK.\n"));
		
		mp=(check_ack_t *)&msg;
		p=onbase;
		while(p!=NULL) {
			if(p->addr==from && p->id==mp->hdr.hostid) break;
			p=p->next;
		} 
		if(p==NULL) return(0);
		if(mp->sid!=p->sid) return(0); 
		on_update(p->id,from,msg.hdr.rport,msg.hdr.version,0);
		p->ka=time(NULL) + p->refresh;
		p->check_fails=0;
		return(1);
	}
	
	if(msg.hdr.opcode==AUTH_SX) {
		struct dhis_rec *dbp;
		auth_sendx_t *mp;
		mpz_t x;
		char buff[1024];
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Received AUTH_SX.\n"));
		
		mp=(auth_sendx_t *)&msg;
		dbp=get_rec(mp->hdr.hostid);
		if(dbp==NULL) {
			auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending AUTH_DENY. Invalid ID.\n"));
			
			m.hdr.opcode=AUTH_DENY;
			m.hdr.hostid=msg.hdr.hostid;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(0);
		}
		if(!dbp->status) {
			auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending AUTH_DENY. Locked.\n"));
			
			m.hdr.opcode=AUTH_DENY;
			m.hdr.hostid=msg.hdr.hostid;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(0);
		}
		
		if(dbp->atype!=AQRC || dbp->xstage==0) {
			auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending AUTH_DENY. Not in X waiting mode.\n"));
			
			m.hdr.opcode=AUTH_DENY;
			m.hdr.hostid=msg.hdr.hostid;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(0);
		}
		mpz_init(x);
		memcpy(buff,mp->x,200);
		buff[200]='\0';
		mpz_set_str(x,buff,10);
		if(mpz_cmp(x,dbp->x)) {
			auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending AUTH_DENY. X and X do not match.\n"));
			
			m.hdr.opcode=AUTH_DENY;
			m.hdr.hostid=msg.hdr.hostid;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			if(dbp->xstage) mpz_clear(dbp->x);
			dbp->xstage=0;
			mpz_clear(x);
			return(0);
		} else {
			auth_ack_t m;
			auth_ack_51_t m51;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending AUTH_ACK. X and X match.\n"));
			
			mpz_clear(x);
			mpz_clear(dbp->x);
			dbp->xstage=0;
			if(msg.hdr.version>=51) {
				m51.hdr.opcode=R51_AUTH_ACK;
				m51.hdr.hostid=msg.hdr.hostid;
				m51.raddr=from;
				if((m51.sid=on_update(dbp->hostid,from,msg.hdr.rport,msg.hdr.version,dbp->refresh))!=0) {
					net_write_message((msg_t *)&m51,from,msg.hdr.rport);
					return(1);
				} else return(0);
				
			} else {
				m.hdr.opcode=AUTH_ACK;
				m.hdr.hostid=msg.hdr.hostid;
				if((m.sid=on_update(dbp->hostid,from,msg.hdr.rport,
									msg.hdr.version,dbp->refresh))!=0) {
					net_write_message((msg_t *)&m,from,msg.hdr.rport);
				return(1); } else return(0);
			}
		}
	}
	
	if(msg.hdr.opcode==AUTH_REQ) {
		struct dhis_rec *dbp;
		auth_req_t *mp;
		mp=(auth_req_t *)&msg;
		
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Received AUTH_REQ for %d\n",mp->hdr.hostid));
		DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Refresh rate is set to %d\n",mp->refresh));
		
		dbp=get_rec(mp->hdr.hostid);
		if(dbp==NULL) {
			auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending AUTH_DENY. Invalid ID.\n"));
			
			m.hdr.opcode=AUTH_DENY;
			m.hdr.hostid=msg.hdr.hostid;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(0);
		}
		if(!dbp->status) {
			auth_deny_t m;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending AUTH_DENY. Locked.\n"));
			
			m.hdr.opcode=AUTH_DENY;
			m.hdr.hostid=msg.hdr.hostid;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(0);
		}
		mp->pass[MAX_PASS-1]='\0';
		if(dbp->atype==APASS) {
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Password Authentication.\n"));
			
			if(strcmp(mp->pass,dbp->hostpass)) {
				auth_deny_t m;
				
				DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending AUTH_DENY. Invalid Password.\n"));
				m.hdr.opcode=AUTH_DENY;
				m.hdr.hostid=msg.hdr.hostid;
				net_write_message((msg_t *)&m,from,msg.hdr.rport);
				return(0);
			} else {
				auth_ack_t m;
				auth_ack_51_t m51;
				
				DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending AUTH_ACK.\n"));
				
				dbp->refresh=mp->refresh;
				if(msg.hdr.version>=51) {
					m51.hdr.opcode=R51_AUTH_ACK;
					m51.hdr.hostid=msg.hdr.hostid;
					m51.raddr=from;
					if((m51.sid=on_update(dbp->hostid,from,msg.hdr.rport,
										  msg.hdr.version,dbp->refresh))!=0) {
                        net_write_message((msg_t *)&m51,from,msg.hdr.rport);
					return(1); } else return(0);
				} else {
					m.hdr.opcode=AUTH_ACK;
					m.hdr.hostid=msg.hdr.hostid;
					if((m.sid=on_update(dbp->hostid,from,msg.hdr.rport,
										msg.hdr.version,dbp->refresh))!=0) {
                        net_write_message((msg_t *)&m,from,msg.hdr.rport);
					return(1); } else return(0);
				}
			}
		}
		if(dbp->atype==AQRC) {
			auth_sendy_t m;
			mpz_t y;
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): QRC Authentication.\n"));
			
			dbp->refresh=mp->refresh;
			if(dbp->xstage==1) { dbp->xstage=0; mpz_clear(dbp->x);}
			m.hdr.opcode=AUTH_SY;
			dbp->xstage=1;
			mpz_init(dbp->x);
			qrc_genx(dbp->x,dbp->mauthn);
			mpz_init(y);
			qrc_geny(y,dbp->x,dbp->mauthn);
			qrc_fill_str(y,m.y,200);
			mpz_clear(y);
			
			DSYSLOG(1,(LOG_DEBUG,"do_dgram(): Sending AUTH_SY.\n"));
			
			m.hdr.hostid=msg.hdr.hostid;
			net_write_message((msg_t *)&m,from,msg.hdr.rport);
			return(1);
		}
		return(0);
		
	}
	
	
	
	return(1);
}



