/* 
 * Reed Solomon Encoder/Decoder 
 *
 * Copyright Henry Minsky (hqm@alum.mit.edu) 1991-2009
 *
 * This software library is licensed under terms of the GNU GENERAL
 * PUBLIC LICENSE
 *
 * RSCODE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RSCODE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rscode.  If not, see <http://www.gnu.org/licenses/>.

 * Commercial licensing is available under a separate license, please
 * contact author for details.
 *
 * Source code is available at http://rscode.sourceforge.net
 */

#include <stdio.h>
#include <ctype.h>
#include "ecc.h"

/* Encoder parity bytes */
int pBytes[MAXDEG];

/* Decoder syndrome bytes */
int synBytes[MAXDEG];

/* generator polynomial */
int genPoly[MAXDEG*2];

int DEBUG = FALSE;

static void
compute_genpoly (int nbytes, int genpoly[]);

/* Initialize lookup tables, polynomials, etc. */
void
initialize_ecc ()
{
  /* Initialize the galois field arithmetic tables */
    init_galois_tables();

    /* Compute the encoder generator polynomial */
    compute_genpoly(RS_ECC_NPARITY, genPoly);
}

void
zero_fill_from (unsigned char buf[], int from, int to)
{
  int i;
  for (i = from; i < to; i++) buf[i] = 0;
}

/* debugging routines */
void
print_parity (void)
{
#ifdef NEVER
  int i;
  printf("Parity Bytes: ");
  for (i = 0; i < RS_ECC_NPARITY; i++) 
    printf("[%d]:%x, ",i,pBytes[i]);
  printf("\n");
#endif
}


void
print_syndrome (void)
{
#ifdef NEVER
  int i;
  printf("Syndrome Bytes: ");
  for (i = 0; i < RS_ECC_NPARITY; i++) 
    printf("[%d]:%x, ",i,synBytes[i]);
  printf("\n");
#endif
}

/* Append the parity bytes onto the end of the message */
void
build_codeword (unsigned char msg[], int nbytes, unsigned char dst[])
{
  int i;
	
  for (i = 0; i < nbytes; i++) dst[i] = msg[i];
	
  for (i = 0; i < RS_ECC_NPARITY; i++) {
    dst[i+nbytes] = pBytes[RS_ECC_NPARITY-1-i];
  }
}
	
/**********************************************************
 * Reed Solomon Decoder 
 *
 * Computes the syndrome of a codeword. Puts the results
 * into the synBytes[] array.
 */
 
void
decode_data(unsigned char data[], int nbytes)
{
  int i, j, sum;
  for (j = 0; j < RS_ECC_NPARITY;  j++) {
    sum	= 0;
    for (i = 0; i < nbytes; i++) {
      sum = data[i] ^ gmult(gexp[j+1], sum);
    }
    synBytes[j]  = sum;
  }
}


/* Check if the syndrome is zero */
int
check_syndrome (void)
{
 int i, nz = 0;
 for (i =0 ; i < RS_ECC_NPARITY; i++) {
  if (synBytes[i] != 0) {
      nz = 1;
      break;
  }
 }
 return nz;
}


void
debug_check_syndrome (void)
{	
#ifdef NEVER
  int i;
	
  for (i = 0; i < 3; i++) {
    printf(" inv log S[%d]/S[%d] = %d\n", i, i+1, 
	   glog[gmult(synBytes[i], ginv(synBytes[i+1]))]);
  }
#endif
}


/* Create a generator polynomial for an n byte RS code. 
 * The coefficients are returned in the genPoly arg.
 * Make sure that the genPoly array which is passed in is 
 * at least n+1 bytes long.
 */

static void
compute_genpoly (int nbytes, int genpoly[])
{
  int i, tp[MAXDEG], tp1[MAXDEG];
	
  /* multiply (x + a^n) for n = 1 to nbytes */

  zero_poly(tp1);
  tp1[0] = 1;

  for (i = 1; i <= nbytes; i++) {
    zero_poly(tp);
    tp[0] = gexp[i];		/* set up x+a^n */
    tp[1] = 1;
	  
    mult_polys(genpoly, tp, tp1);
    copy_poly(tp1, genpoly);
  }
}

/* Simulate a LFSR with generator polynomial for n byte RS code. 
 * Pass in a pointer to the data array, and amount of data. 
 *
 * The parity bytes are deposited into pBytes[], and the whole message
 * and parity are copied to dest to make a codeword.
 * 
 */

void
encode_data (unsigned char msg[], int nbytes, unsigned char dst[])
{
  int i, LFSR[RS_ECC_NPARITY+1],dbyte, j;
	
  for(i=0; i < RS_ECC_NPARITY+1; i++) LFSR[i]=0;

  for (i = 0; i < nbytes; i++) {
    dbyte = msg[i] ^ LFSR[RS_ECC_NPARITY-1];
    for (j = RS_ECC_NPARITY-1; j > 0; j--) {
      LFSR[j] = LFSR[j-1] ^ gmult(genPoly[j], dbyte);
    }
    LFSR[0] = gmult(genPoly[0], dbyte);
  }

  for (i = 0; i < RS_ECC_NPARITY; i++) 
    pBytes[i] = LFSR[i];
	
  build_codeword(msg, nbytes, dst);
}

