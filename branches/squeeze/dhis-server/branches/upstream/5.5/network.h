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

#define	MAX_MSG	256

typedef struct {
		int opcode;
		int serial;
		int version;
		int rport;
		} header4_t;

typedef struct {
		int opcode;
		int serial;
		int version;
		int rport;
		int hostid;
		} header_t;

typedef struct { header_t hdr;
		unsigned char buff[MAX_MSG-sizeof(header_t)];
	       } msg_t;

/* R3 messages */
typedef struct { header4_t hdr; int id; int pass; } r3_online_req_t;
typedef struct { header4_t hdr; int id; int pass; } r3_offline_req_t;

/* R4 messages */
typedef struct { header4_t hdr; } r4_echo_req_t;
typedef struct { header4_t hdr; int oserial; } r4_echo_ack_t;
typedef struct { header4_t hdr; int id; char pass[16]; } r4_auth_req_t;
typedef struct { header4_t hdr; } r4_auth_deny_t;
typedef struct { header4_t hdr; int sid; } r4_auth_ack_t;
typedef struct { header4_t hdr; int id; char x[200]; } r4_auth_sendx_t;
typedef struct { header4_t hdr; char y[200]; } r4_auth_sendy_t;
typedef struct { header4_t hdr; int next_check; } r4_check_req_t;
typedef struct { header4_t hdr; int sid; } r4_check_ack_t;
typedef struct { header4_t hdr; int sid; } r4_offline_req_t;

/* R5 messages */
typedef struct { header_t hdr; } echo_req_t;
typedef struct { header_t hdr; int oserial; } echo_ack_t;
typedef struct { header_t hdr; char pass[16];int refresh; } auth_req_t;
typedef struct { header_t hdr; } auth_deny_t;
typedef struct { header_t hdr; int sid; } auth_ack_t;
typedef struct { header_t hdr; int sid; int raddr; } auth_ack_51_t;
typedef struct { header_t hdr; char x[200]; } auth_sendx_t;
typedef struct { header_t hdr; char y[200]; } auth_sendy_t;
typedef struct { header_t hdr; int next_check; } check_req_t;
typedef struct { header_t hdr; int sid; } check_ack_t;
typedef struct { header_t hdr; int sid; } offline_req_t;

int msg_size_by_opcode(int);
void swap_int(int *);
void swap_msg(int *,int);
int little_endian(void);
void convert_message(msg_t *,int);
int get_serial(void);
int net_init(void);
int net_close(void);
int net_check_message(void);
int net_read_message(msg_t *,int *);
int net_write_message(msg_t *,int,int);
int net_do_dgram(msg_t,int);

