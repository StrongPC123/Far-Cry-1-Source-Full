/* NeuQuant Neural-Net Quantization Algorithm Interface
 * ----------------------------------------------------
 *
 * Copyright (c) 1994 Anthony Dekker
 *
 * NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994.
 * See "Kohonen neural networks for optimal colour quantization"
 * in "Network: Computation in Neural Systems" Vol. 5 (1994) pp 351-367.
 * for a discussion of the algorithm.
 * See also  http://www.acm.org/~dekker/NEUQUANT.HTML
 *
 * Any party obtaining a copy of these files from the author, directly or
 * indirectly, is granted, free of charge, a full and unrestricted irrevocable,
 * world-wide, paid up, royalty-free, nonexclusive right and license to deal
 * in this software and documentation files (the "Software"), including without
 * limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.
 */


#include <stdio.h>


#define netsize		256			/* number of colours used */


/* For 256 colours, fixed arrays need 8kb, plus space for the image
   ---------------------------------------------------------------- */


/* four primes near 500 - assume no image has a length so large */
/* that it is divisible by all four primes */
#define prime1		499
#define prime2		491
#define prime3		487
#define prime4		503

#define minpicturebytes	(3*prime4)		/* minimum size for input image */


/* Initialise network in range (0,0,0) to (255,255,255) and set parameters
   ----------------------------------------------------------------------- */
void initnet(unsigned char *thepic, int len, int sample);
		
/* Unbias network to give byte values 0..255 and record position i to prepare for sort
   ----------------------------------------------------------------------------------- */
void unbiasnet();	/* can edit this function to do output of colour map */

/* Output colour map
   ----------------- */
void writecolourmap(unsigned char *p);

/* Insertion sort of network and building of netindex[0..255] (to do after unbias)
   ------------------------------------------------------------------------------- */
void inxbuild();

/* Search for BGR values 0..255 (after net is unbiased) and return colour index
   ---------------------------------------------------------------------------- */
int inxsearch(register int b, register int g, register int r);

/* Main Learning Loop
   ------------------ */
void learn();

/* Program Skeleton
   ----------------
   	[select samplefac in range 1..30]
   	pic = (unsigned char*) malloc(3*width*height);
   	[read image from input file into pic]
	initnet(pic,3*width*height,samplefac);
	learn();
	unbiasnet();
	[write output image header, using writecolourmap(f),
	possibly editing the loops in that function]
	inxbuild();
	[write output image using inxsearch(b,g,r)]		*/
