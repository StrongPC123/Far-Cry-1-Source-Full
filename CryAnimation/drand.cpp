/*-------------------------------------------------------------------------*/
/*  Uniform random numbers: Multiplicative congruential generator for      */
/*                          uniformly distributed random numbers on (0, 1) */
/*                                                                         */
/*  Modul m = 2**54,  Multiplier a  =  2.783.377.640.906.189               */
/*  Period  = 2**52,                =  a1 * 2**27 + a2                     */
/*                    where  a1 = 20.737.779, a2 = 59.760.077              */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/* The multiplier a of the generator was the winner of a search among one  */
/* million possible multipliers near to the well-known golden section      */ 
/* factor f = .5*(sqrt(5)-1)*2^52. The multiplier a was choosen because of */
/* its good lattice properties up to dimension 8 measured by both Beyer    */ 
/* quotients (Afflerbach/Grothe,1985, Computing 35, 269-276) and           */ 
/* standardized values of the spectral test according to Fishman (1990),   */ 
/* Math.Comput. 54,331-344. The search was carried out by J.H. Ahrens(1992).*/ 
/* The generator (denoted as krand) was implemented and tested as described*/ 
/* in Stadlober/Kremer (1992), Lecture Notes in Econ.Math.Systems 374,     */ 
/* 154-162.                                                                */
/* The values of the measures from dimension 2 to dimension 8 are:         */
/* Beyer-quotients:  .87, .71, .72, .73, .66, .82, .80;  Minimum = .6626   */
/* Spectral-test:    .87, .76, .79, .78, .75, .73, .69;  Minimum = .6870   */
/*                                                                         */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/* The seed-values z_hi and z_lo has to be defined as follows:             */                
/* printf("Input seed-high (0 ... 2^27 - 1)  -->   ");                     */
/* scanf("%ld", &z_hi);                                                    */
/* printf("Input seed-low  (0 ... 2^25 - 1)  -->   ");                     */
/* scanf("%ld", &z_lo); z_lo = z_lo*4L + 1L;                               */
/*-------------------------------------------------------------------------*/

#include "stdafx.h"
//#include <math.h>

unsigned int drand_z_hi = ((0x9DC59E12) & ((1<<27)-1)); // 8456922526314162853725694434834
unsigned int drand_z_lo = ((0x7E46BBD3) & ((1<<25)-1)) * 4l + 1l; // 462159607207911770990826022692
double drand()
{
	unsigned int   i_x;
	double              d_x;

	union ux {
			 double  d;
			 short   i[4];
		 } x;

	x.d = (double)(drand_z_lo) * 59760077.0;  x.i[3] -= 0X01b0;
	i_x = (unsigned int)x.d;
	drand_z_hi = (drand_z_hi * 59760077L + drand_z_lo * 20737779L + i_x) & 0X07FFFFFFL;
	d_x = x.d -= (double)i_x;            x.i[3] += 0X01b0;
	drand_z_lo = (unsigned int)x.d;
	x.d = (double)drand_z_hi + d_x;           x.i[3] -= 0X01b0;

	assert (x.d > 0 && x.d < 1);
	return (x.d);
}


static int irand_sx = 1, irand_sy = 10000, irand_sz = 3000;
/* This function implements the three
 * congruential generators.
 */
 
int ranwh()
{
	int r, s;

	/*  irand_sx = irand_sx * 171 mod 30269 */
	r = irand_sx/177;
	s = irand_sx - 177 * r;
	irand_sx = 171 * s - 2 * r;
	if( irand_sx < 0 )
		irand_sx += 30269;


	/* irand_sy = irand_sy * 172 mod 30307 */
	r = irand_sy/176;
	s = irand_sy - 176 * r;
	irand_sy = 172 * s - 35 * r;
	if( irand_sy < 0 )
		irand_sy += 30307;

	/* irand_sz = 170 * irand_sz mod 30323 */
	r = irand_sz/178;
	s = irand_sz - 178 * r;
	irand_sz = 170 * s - 63 * r;
	if( irand_sz < 0 )
		irand_sz += 30323;
	/* The results are in static irand_sx, irand_sy, irand_sz. */
	return 0;
}

int irand ()
{
	ranwh();
	return irand_sx*irand_sy+irand_sz;
}
