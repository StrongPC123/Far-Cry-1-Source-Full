// WOUTER
// - made it compile as C++
// - fixed /0 bug with small image sizes


/* NeuQuant Neural-Net Quantization Algorithm
 * ------------------------------------------
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


#include "neuquant.h"
#include <assert.h>


/* Network Definitions
   ------------------- */
   
#define maxnetpos	(netsize-1)
#define netbiasshift	4			/* bias for colour values */
#define ncycles		100			/* no. of learning cycles */

/* defs for freq and bias */
#define intbiasshift    16			/* bias for fractions */
#define intbias		(((int) 1)<<intbiasshift)
#define gammashift  	10			/* gamma = 1024 */
#define gamma   	(((int) 1)<<gammashift)
#define betashift  	10
#define beta		(intbias>>betashift)	/* beta = 1/1024 */
#define betagamma	(intbias<<(gammashift-betashift))

/* defs for decreasing radius factor */
#define initrad		(netsize>>3)		/* for 256 cols, radius starts */
#define radiusbiasshift	6			/* at 32.0 biased by 6 bits */
#define radiusbias	(((int) 1)<<radiusbiasshift)
#define initradius	(initrad*radiusbias)	/* and decreases by a */
#define radiusdec	30			/* factor of 1/30 each cycle */ 

/* defs for decreasing alpha factor */
#define alphabiasshift	10			/* alpha starts at 1.0 */
#define initalpha	(((int) 1)<<alphabiasshift)
int alphadec;					/* biased by 10 bits */

/* radbias and alpharadbias used for radpower calculation */
#define radbiasshift	8
#define radbias		(((int) 1)<<radbiasshift)
#define alpharadbshift  (alphabiasshift+radbiasshift)
#define alpharadbias    (((int) 1)<<alpharadbshift)


/* Types and Global Variables
   -------------------------- */
   
static unsigned char *thepicture;		/* the input image itself */
static int lengthcount;				/* lengthcount = H*W*3 */

static int samplefac;				/* sampling factor 1..30 */


typedef int pixel[4];				/* BGRc */
static pixel network[netsize];			/* the network itself */

static int netindex[256];			/* for network lookup - really 256 */

static int bias [netsize];			/* bias and freq arrays for learning */
static int freq [netsize];
static int radpower[initrad];			/* radpower for precomputation */


/* Initialise network in range (0,0,0) to (255,255,255) and set parameters
   ----------------------------------------------------------------------- */

void initnet(unsigned char *thepic, int len, int sample)	
{
	register int i;
	register int *p;
	
	thepicture = thepic;
	lengthcount = len;
	samplefac = sample;
	
	for (i=0; i<netsize; i++) {
		p = network[i];
		p[0] = p[1] = p[2] = (i << (netbiasshift+8))/netsize;
		freq[i] = intbias/netsize;	/* 1/netsize */
		bias[i] = 0;
	}
}

	
/* Unbias network to give byte values 0..255 and record position i to prepare for sort
   ----------------------------------------------------------------------------------- */

void unbiasnet()
{
	int i,j,temp;

	for (i=0; i<netsize; i++) {
		for (j=0; j<3; j++) {
			/* OLD CODE: network[i][j] >>= netbiasshift; */
			/* Fix based on bug report by Juergen Weigert jw@suse.de */
			temp = (network[i][j] + (1 << (netbiasshift - 1))) >> netbiasshift;
			if (temp > 255) temp = 255;
			network[i][j] = temp;
		}
		network[i][3] = i;			/* record colour no */
	}
}


/* Output colour map
   ----------------- */

void writecolourmap(unsigned char *p)
{
	int i,j;

	for (j=0; j<netsize; j++) 
		for (i=0; i<3; i++) 
			*p++ = network[j][i];
}


/* Insertion sort of network and building of netindex[0..255] (to do after unbias)
   ------------------------------------------------------------------------------- */

void inxbuild()
{
	register int i,j,smallpos,smallval;
	register int *p,*q;
	int previouscol,startpos;

	previouscol = 0;
	startpos = 0;
	for (i=0; i<netsize; i++) {
		p = network[i];
		smallpos = i;
		smallval = p[1];			/* index on g */
		/* find smallest in i..netsize-1 */
		for (j=i+1; j<netsize; j++) {
			q = network[j];
			if (q[1] < smallval) {		/* index on g */
				smallpos = j;
				smallval = q[1];	/* index on g */
			}
		}
		q = network[smallpos];
		/* swap p (i) and q (smallpos) entries */
		if (i != smallpos) {
			j = q[0];   q[0] = p[0];   p[0] = j;
			j = q[1];   q[1] = p[1];   p[1] = j;
			j = q[2];   q[2] = p[2];   p[2] = j;
			j = q[3];   q[3] = p[3];   p[3] = j;
		}
		/* smallval entry is now in position i */
		if (smallval != previouscol) {
			netindex[previouscol] = (startpos+i)>>1;
			for (j=previouscol+1; j<smallval; j++) netindex[j] = i;
			previouscol = smallval;
			startpos = i;
		}
	}
	netindex[previouscol] = (startpos+maxnetpos)>>1;
	for (j=previouscol+1; j<256; j++) netindex[j] = maxnetpos; /* really 256 */
}


/* Search for BGR values 0..255 (after net is unbiased) and return colour index
   ---------------------------------------------------------------------------- */

int inxsearch(int b, int g, int r)
{
	register int i,j,dist,a,bestd;
	register int *p;
	int best;

	bestd = 1000;		/* biggest possible dist is 256*3 */
	best = -1;
	i = netindex[g];	/* index on g */
	j = i-1;		/* start at netindex[g] and work outwards */

	while ((i<netsize) || (j>=0)) {
		if (i<netsize) {
			p = network[i];
			dist = p[1] - g;		/* inx key */
			if (dist >= bestd) i = netsize;	/* stop iter */
			else {
				i++;
				if (dist<0) dist = -dist;
				a = p[0] - b;   if (a<0) a = -a;
				dist += a;
				if (dist<bestd) {
					a = p[2] - r;   if (a<0) a = -a;
					dist += a;
					if (dist<bestd) {bestd=dist; best=p[3];}
				}
			}
		}
		if (j>=0) {
			p = network[j];
			dist = g - p[1]; /* inx key - reverse dif */
			if (dist >= bestd) j = -1; /* stop iter */
			else {
				j--;
				if (dist<0) dist = -dist;
				a = p[0] - b;   if (a<0) a = -a;
				dist += a;
				if (dist<bestd) {
					a = p[2] - r;   if (a<0) a = -a;
					dist += a;
					if (dist<bestd) {bestd=dist; best=p[3];}
				}
			}
		}
	}
	return(best);
}


/* Search for biased BGR values
   ---------------------------- */

int contest(int b, int g, int r)
{
	/* finds closest neuron (min dist) and updates freq */
	/* finds best neuron (min dist-bias) and returns position */
	/* for frequently chosen neurons, freq[i] is high and bias[i] is negative */
	/* bias[i] = gamma*((1/netsize)-freq[i]) */

	register int i,dist,a,biasdist,betafreq;
	int bestpos,bestbiaspos,bestd,bestbiasd;
	register int *p,*f, *n;

	bestd = ~(((int) 1)<<31);
	bestbiasd = bestd;
	bestpos = -1;
	bestbiaspos = bestpos;
	p = bias;
	f = freq;

	for (i=0; i<netsize; i++) {
		n = network[i];
		dist = n[0] - b;   if (dist<0) dist = -dist;
		a = n[1] - g;   if (a<0) a = -a;
		dist += a;
		a = n[2] - r;   if (a<0) a = -a;
		dist += a;
		if (dist<bestd) {bestd=dist; bestpos=i;}
		biasdist = dist - ((*p)>>(intbiasshift-netbiasshift));
		if (biasdist<bestbiasd) {bestbiasd=biasdist; bestbiaspos=i;}
		betafreq = (*f >> betashift);
		*f++ -= betafreq;
		*p++ += (betafreq<<gammashift);
	}
	freq[bestpos] += beta;
	bias[bestpos] -= betagamma;
	return(bestbiaspos);
}


/* Move neuron i towards biased (b,g,r) by factor alpha
   ---------------------------------------------------- */

void altersingle(int alpha, int i, int b, int g, int r)
{
	register int *n;

	n = network[i];				/* alter hit neuron */
	*n -= (alpha*(*n - b)) / initalpha;
	n++;
	*n -= (alpha*(*n - g)) / initalpha;
	n++;
	*n -= (alpha*(*n - r)) / initalpha;
}


/* Move adjacent neurons by precomputed alpha*(1-((i-j)^2/[r]^2)) in radpower[|i-j|]
   --------------------------------------------------------------------------------- */

void alterneigh(int rad, int i, int b, int g, int r)
{
	register int j,k,lo,hi,a;
	register int *p, *q;

	lo = i-rad;   if (lo<-1) lo=-1;
	hi = i+rad;   if (hi>netsize) hi=netsize;

	j = i+1;
	k = i-1;
	q = radpower;
	while ((j<hi) || (k>lo)) {
		a = (*(++q));
		if (j<hi) {
			p = network[j];
			*p -= (a*(*p - b)) / alpharadbias;
			p++;
			*p -= (a*(*p - g)) / alpharadbias;
			p++;
			*p -= (a*(*p - r)) / alpharadbias;
			j++;
		}
		if (k>lo) {
			p = network[k];
			*p -= (a*(*p - b)) / alpharadbias;
			p++;
			*p -= (a*(*p - g)) / alpharadbias;
			p++;
			*p -= (a*(*p - r)) / alpharadbias;
			k--;
		}
	}
}


/* Main Learning Loop
   ------------------ */

void learn()
{
	register int i,j,b,g,r;
	int radius,rad,alpha,step,delta,samplepixels;
	register unsigned char *p;
	unsigned char *lim;

	alphadec = 30 + ((samplefac-1)/3);
	p = thepicture;
	lim = thepicture + lengthcount;
	samplepixels = lengthcount/(3*samplefac);
	
	//if(samplepixels<ncycles) samplepixels = ncycles;
	//assert(lengthcount>=ncycles);
	
	delta = samplepixels/ncycles;
	
	if(!delta) delta = 1; // WOUTER: I guess noone ever tested with small images (mipmaps)
	
	alpha = initalpha;
	radius = initradius;
	
	rad = radius >> radiusbiasshift;
	if (rad <= 1) rad = 0;
	for (i=0; i<rad; i++) 
		radpower[i] = alpha*(((rad*rad - i*i)*radbias)/(rad*rad));
	
	//fprintf(stderr,"beginning 1D learning: initial radius=%d\n", rad);

	if ((lengthcount%prime1) != 0) step = 3*prime1;
	else {
		if ((lengthcount%prime2) !=0) step = 3*prime2;
		else {
			if ((lengthcount%prime3) !=0) step = 3*prime3;
			else step = 3*prime4;
		}
	}
	
	i = 0;
	while (i < samplepixels) {
		b = p[0] << netbiasshift;
		g = p[1] << netbiasshift;
		r = p[2] << netbiasshift;
		j = contest(b,g,r);

		altersingle(alpha,j,b,g,r);
		if (rad) alterneigh(rad,j,b,g,r);   /* alter neighbours */

		p += step;
		while (p >= lim) p -= lengthcount;	// wouter: was: if (p >= lim)
	
		i++;
		if (i%delta == 0) {	
			alpha -= alpha / alphadec;
			radius -= radius / radiusdec;
			rad = radius >> radiusbiasshift;
			if (rad <= 1) rad = 0;
			for (j=0; j<rad; j++) 
				radpower[j] = alpha*(((rad*rad - j*j)*radbias)/(rad*rad));
		}
	}
	//fprintf(stderr,"finished 1D learning: final alpha=%f !\n",((float)alpha)/initalpha);
}
