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

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#include "nsupdate.h"

int dns_update(int opcode,const char *hostname,const char *ipaddr) {
	
	int fildes[2];
	int childpid;
	int ret_code=0; 
	char str[1024];
	
	if(opcode==NSUPDATE_ADD) {
        	sprintf(str,"update add %s. %d IN A %s\n\n",hostname,DYN_TTL,ipaddr);
	} 
	else {
		if(opcode==NSUPDATE_DEL)
			sprintf(str,"update delete %s. IN A\n\n",hostname);
		else 
			return(0);
	}

	if(pipe(fildes)) return(0);

	write(fildes[1],str,strlen(str)+1);

	childpid=fork();
	
	if(!childpid) { // I am the child 

		// Place the pipe read in stdin and close pipe write
		if(fildes[0]!=0) close(0);
        	if(fildes[0]!=0) dup2(fildes[0],0);
        	close(fildes[1]);

		// Redirect stdout and stderr to NULL
		close(1);
		close(2);
		open("/dev/null",O_WRONLY,0666);
		open("/dev/null",O_WRONLY,0666);

		ret_code=execlp(NSUPDATE_CMD,NSUPDATE_CMD,"-d",NULL);
		if(ret_code) exit(1); else exit(0);

	} else {	// I am the parent, feed the child and wait for it to finish

		close(fildes[0]);
		close(fildes[1]);
		if(waitpid(childpid,&ret_code,0)==-1) return(0);
		return(1);
	}

}

