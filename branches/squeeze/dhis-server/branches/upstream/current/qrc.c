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

#include "qrc.h"

char gauthp[2][256];
char gauthq[2][256];
char gauthn[4][256];

/* qrc_random() - Generates a random integer of n digits
 *                n may be up to 1024
 */
void qrc_random(mpz_t x,int n) {

        char buff[1024],temp[128];
        static int seed=0;

        if(!seed) { seed++; srandom(time(NULL)); }
        memset(buff,0,256);
        memset(temp,0,128);

        do {
                sprintf(temp,"%lu",random());
                strcat(buff,temp);

        } while(strlen(buff) < n);
        buff[n]='\0';

        mpz_set_str(x,buff,10);
        return;
}

/* qrc_genkey() - Generates an integer of 100 digits being congruent
 *                to 3 mod 4
 *
 */

void qrc_genkey(mpz_t k) {

        int flag=1;

        do {


        mpz_t a,b;

        /* Get a prime number */
        do qrc_random(k,100); while(!mpz_probab_prime_p(k,5));

        /* Now see if it is congruent to 3 mod 4 */
        mpz_init(a);mpz_init(b);
        mpz_set_ui(a,4);
        mpz_mod(b,k,a);
        mpz_set_ui(a,3);
        if(!mpz_cmp(a,b)) flag=0;
        mpz_clear(a);
        mpz_clear(b);

        } while(flag);

}
void gen_qrc(void) {

        mpz_t p,q,n;

        mpz_init(p);
        mpz_init(q);
        mpz_init(n);

        qrc_genkey(p);
        show_key(p,2,1);
        qrc_genkey(q);
        show_key(q,2,2);
        mpz_mul(n,p,q);
        show_key(n,4,3);
        mpz_clear(p);
        mpz_clear(q);
        mpz_clear(n);
        return;
}

void show_key(mpz_t key,int n,int variab) {

        char buff[1024];
        char chunk[128];
        char *cp;
        int i;

        mpz_get_str(buff,10,key);

        cp=buff;
        for(i=0;i<n;i++) {
                memcpy(chunk,cp,50);
                chunk[50]='\0';
                if(variab==1) strcpy(gauthp[i],chunk);
                if(variab==2) strcpy(gauthq[i],chunk);
                if(variab==3) strcpy(gauthn[i],chunk);
                cp+=50;
        }
}

/* qrc_genx() - Geretates a random x relatively prime to n
 *
 */
void qrc_genx(mpz_t x,mpz_t n) {

        int i;
        mpz_t t;

        i=mpz_sizeinbase(n,10);         /* Get size of n and take 1 */
        i--;

        mpz_init(t);

        do {                            /* Generate x of n-1 digits */
                qrc_random(x,i);
                qrc_geny(t,x,n);        /* square it modulo n to get */
                mpz_set(x,t);           /* quadractic residue */
                mpz_gcd(t,x,n);

        } while(mpz_cmp_ui(t,1));       /* test relative primeness */

        mpz_clear(t);
}

/* qrc_geny() - y is the quadractic residue given by x^2 mod n
 *
 */
void qrc_geny(mpz_t y,mpz_t x,mpz_t n) {

        mpz_powm_ui(y,x,2,n);
}
/* qrc_sqrty() - Calculates the square root of y mod k using a^((k+1)/4))mod k
 *
 */

void qrc_sqrty(mpz_t s,mpz_t y,mpz_t k) {

        mpz_t t1,t2,t3;

        mpz_init(t1);
        mpz_init(t2);
        mpz_init(t3);
        mpz_set(t1,k);
        mpz_set_ui(t3,4);
        mpz_add_ui(t2,t1,1);            /* t2 = k+1 */
        mpz_divexact(t1,t2,t3);         /* t1 = t2/4 */
        mpz_powm(s,y,t1,k);
        mpz_clear(t1);
        mpz_clear(t2);
        mpz_clear(t3);
}
/* qrc_crt() - Applies the Chinese remainder theorem and calculates x
 *
 */
void qrc_crt(mpz_t x,mpz_t xp,mpz_t xq,mpz_t p,mpz_t q) {

        mpz_t s,t,g1,g2;
        mpz_t temp;

        mpz_init(s);
        mpz_init(t);
        mpz_init(g1);
        mpz_init(g2);
        mpz_init(temp);

        /* Use Euclid's theorem to find s and t */
        mpz_gcdext(g1,s,t,q,p);

        mpz_mul(temp,xp,s);     /* Do g1 = x1.s.q */
        mpz_mul(g1,temp,q);

        mpz_mul(temp,xq,t);     /* Do g2 = x2.t.p */
        mpz_mul(g2,temp,p);

        mpz_add(x,g1,g2);       /* Do x = g1 + g2 */

        mpz_clear(temp);
        mpz_clear(s);
        mpz_clear(t);
        mpz_clear(g1);
        mpz_clear(g2);

}

/* qrc_fill_str() - This function fills a buffer pointed by str
 *                  with n digits of x. Adds 0's to the left if
 *                  required.
 */
void qrc_fill_str(mpz_t x, char *str,int n) {

        int i,j;
        char buff[1024];
        char buff2[1024];
        char *cp1,*cp2;

        i=mpz_sizeinbase(x,10);         /* Get size of x */
        j=n-i;                          /* j = number of 0's to add */
        if(j<0) return;

        buff[0]='\0';
        for(i=0;i<j;i++) strcat(buff,"0");      /* Place 0's */

        mpz_get_str(buff2,10,x);                /* Add x */
        strcat(buff,buff2);

        /* Now copy n digits to str */
        cp1=str;
        cp2=buff;
        for(i=0;i<n;i++)
                *cp1++ = *cp2++;
        return;
}

