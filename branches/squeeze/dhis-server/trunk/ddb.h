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


#include <stdio.h>
#include <time.h>

#define APASS           1
#define AQRC            2

struct dhis_rec {
	int hostid;
	char hostname[128];
	char hostpass[32];
	char authn[4][128];
	int status;
	char oncmd[256];
	char offcmd[256];

	/* DHISD specific parameters */
	int atype;
	mpz_t mauthn;
	int xstage;
	int refresh;
	mpz_t x;

	struct dhis_rec *next;
};
	
#define	DB_FILE	1		// Default database type if mysql is not enabled

struct dhis_rec *new_rec(void);
void free_rec(struct dhis_rec *);
struct dhis_rec *get_rec(int);

int db_reload(void);
void db_free(void);

char *db_password(int);
int db_is_locked(int);

int file_db_reload(void);

#ifdef WITH_MYSQL
#define	DB_MYSQL 2
int sql_login(void);
void sql_logout(void);
int sql_log(char *);
int sql_update_lastlogin(int);
int sql_db_reload(void);
#endif

