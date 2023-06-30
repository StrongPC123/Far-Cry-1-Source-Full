/*=============================================================================
	inv_cmap.cpp :
	Copyright (c) 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "CImage.h"

static int bcenter, gcenter, rcenter;
static long gdist, rdist, cdist;
static long cbinc, cginc, crinc;
static unsigned long *gdp, *rdp, *cdp;
static unsigned char *grgbp, *rrgbp, *crgbp;
static long gstride, rstride;
static long rx, gx, bx;
static long rxsqr, gxsqr, bxsqr;
static long rcolormax, gcolormax, bcolormax;
static int cindex;

static void maxfill (unsigned long *, long, long, long);
static int redloop (void);
static int greenloop (int);
static int blueloop (int);

/*
* Here's the idea:  scan from the "center" of each cell "out"
* until we hit the "edge" of the cell -- that is, the point
* at which some other color is closer -- and stop.  In 1-D,
* this is simple:
* 	for i := here to max do
 * 		if closer then buffer[i] = this color
 * 		else break
 * 	repeat above loop with i := here-1 to min by -1
 *
 * In 2-D, it's trickier, because along a "scan-line", the
 * region might start "after" the "center" point.  A picture
 * might clarify:
 *		 |    ...
 *               | ...	.
 *              ...    	.
 *           ... |      .
 *          .    +     	.
 *           .          .
 *            .         .
 *             .........
 *
 * The + marks the "center" of the above region.  On the top 2
 * lines, the region "begins" to the right of the "center".
 *
 * Thus, we need a loop like this:
 * 	detect := false
 * 	for i := here to max do
 * 		if closer then
 * 			buffer[..., i] := this color
 * 			if !detect then
 * 				here = i
 * 				detect = true
 * 		else
 * 			if detect then
 * 				break
 *
 * Repeat the above loop with i := here-1 to min by -1.  Note that
 * the "detect" value should not be reinitialized.  If it was
 * "true", and center is not inside the cell, then none of the
 * cell lies to the left and this loop should exit
 * immediately.
 *
 * The outer loops are similar, except that the "closer" test
 * is replaced by a call to the "next in" loop; its "detect"
 * value serves as the test.  (No assignment to the buffer is
 * done, either.)
 *
 * Each time an outer loop starts, the "here", "min", and
 * "max" values of the next inner loop should be
 * re-initialized to the center of the cell, 0, and cube size,
 * respectively.  Otherwise, these values will carry over from
 * one "call" to the inner loop to the next.  This tracks the
 * edges of the cell and minimizes the number of
 * "unproductive" comparisons that must be made.
 *
 * Finally, the inner-most loop can have the "if !detect"
 * optimized out of it by splitting it into two loops: one
 * that finds the first color value on the scan line that is
 * in this cell, and a second that fills the cell until
 * another one is closer:
 *  	if !detect then	    {needed for "down" loop}
 * 	    for i := here to max do
 * 		if closer then
 * 			buffer[..., i] := this color
 * 			detect := true
 * 			break
 *	for i := i+1 to max do
 *		if closer then
 * 			buffer[..., i] := this color
 * 		else
 * 			break
 *
 * In this implementation, each level will require the
 * following variables.  Variables labelled (l) are local to each
 * procedure.  The ? should be replaced with r, g, or b:
 *  	cdist:	    	The distance at the starting point.
 * 	?center:	The value of this component of the color
 *  	c?inc:	    	The initial increment at the ?center position.
 * 	?stride:	The amount to add to the buffer
 * 			pointers (dp and rgbp) to get to the
 * 			"next row".
 * 	min(l):		The "low edge" of the cell, init to 0
 * 	max(l):		The "high edge" of the cell, init to
 * 			colormax-1
 * 	detect(l):    	True if this row has changed some
 * 	    	    	buffer entries.
 *  	i(l): 	    	The index for this row.
 *  	?xx:	    	The accumulated increment value.
 *
 *  	here(l):    	The starting index for this color.  The
 *  	    	    	following variables are associated with here,
 *  	    	    	in the sense that they must be updated if here
 *  	    	    	is changed.
 *  	?dist:	    	The current distance for this level.  The
 *  	    	    	value of dist from the previous level (g or r,
 *  	    	    	for level b or g) initializes dist on this
 *  	    	    	level.  Thus gdist is associated with here(b)).
 *  	?inc:	    	The initial increment for the row.
 *  	?dp:	    	Pointer into the distance buffer.  The value
 *  	    	    	from the previous level initializes this level.
 *  	?rgbp:	    	Pointer into the rgb buffer.  The value
 *  	    	    	from the previous level initializes this level.
 *
 * The blue and green levels modify 'here-associated' variables (dp,
 * rgbp, dist) on the green and red levels, respectively, when here is
 * changed.
 */

void shInverseColormap (int colors, SRGBPixel *colormap,
  int rbits, int gbits, int bbits, byte *&rgbmap, unsigned long *dist_buf)
{
  int rnbits = 8 - rbits;
  int gnbits = 8 - gbits;
  int bnbits = 8 - bbits;

  rcolormax = 1 << rbits;
  gcolormax = 1 << gbits;
  bcolormax = 1 << bbits;
  rx = 1 << rnbits;
  gx = 1 << gnbits;
  bx = 1 << bnbits;
  rxsqr = 1 << (2 * rnbits);
  gxsqr = 1 << (2 * gnbits);
  bxsqr = 1 << (2 * bnbits);

  /* Compute "strides" for accessing the arrays. */
  gstride = bcolormax;
  rstride = gcolormax * bcolormax;

  bool free_dist_buf = false;
  if (!dist_buf)
  {
    free_dist_buf = true;
    dist_buf = new unsigned long [rcolormax * gcolormax * bcolormax];
  }
  maxfill (dist_buf, rcolormax, gcolormax, bcolormax);

  // Allocate inverse colormap if not already done
  if (!rgbmap)
    rgbmap = new byte [rcolormax * gcolormax * bcolormax];

  for (cindex = 0; cindex < colors; cindex++)
  {
    /*
     * Distance formula is
     * (red - map[0])^2 + (green - map[1])^2 + (blue - map[2])^2
     *
     * Because of quantization, we will measure from the center of
     * each quantized "cube", so blue distance is
     * 	(blue + x/2 - map[2])^2,
     * where x = 2^(8 - bits).
     * The step size is x, so the blue increment is
     * 	2*x*blue - 2*x*map[2] + 2*x^2
     *
     * Now, b in the code below is actually blue/x, so our
     * increment will be 2*(b*x^2 + x^2 - x*map[2]).  For
     * efficiency, we will maintain this quantity in a separate variable
     * that will be updated incrementally by adding 2*x^2 each time.
     */

    /* The initial position is the cell containing the colormap
     * entry.  We get this by quantizing the colormap values.
     */
    rcenter = colormap [cindex].red   >> rnbits;
    gcenter = colormap [cindex].green >> gnbits;
    bcenter = colormap [cindex].blue  >> bnbits;

    rdist = colormap [cindex].red   - (rcenter * rx + rx / 2);
    gdist = colormap [cindex].green - (gcenter * gx + gx / 2);
    cdist = colormap [cindex].blue  - (bcenter * bx + bx / 2);
    cdist = rdist * rdist + gdist * gdist + cdist * cdist;

    crinc = 2 * ((rcenter + 1) * rxsqr - (colormap [cindex].red   * rx));
    cginc = 2 * ((gcenter + 1) * gxsqr - (colormap [cindex].green * gx));
    cbinc = 2 * ((bcenter + 1) * bxsqr - (colormap [cindex].blue  * bx));

    /* Array starting points. */
    cdp = dist_buf + rcenter * rstride + gcenter * gstride + bcenter;
    crgbp = rgbmap + rcenter * rstride + gcenter * gstride + bcenter;

    (void) redloop ();
  }

  if (free_dist_buf)
    delete [] dist_buf;
}

/* redloop -- loop up and down from red center. */
static int redloop ()
{
  int detect;
  int r;
  int first;
  long txsqr = rxsqr + rxsqr;
  static long rxx;

  detect = 0;

  /* Basic loop up. */
  for (r = rcenter, rdist = cdist, rxx = crinc,
    rdp = cdp, rrgbp = crgbp, first = 1;
    r < rcolormax;
    r++, rdp += rstride, rrgbp += rstride,
    rdist += rxx, rxx += txsqr, first = 0)
  {
    if (greenloop (first))
      detect = 1;
    else if (detect)
      break;
  }

  /* Basic loop down. */
  for (r = rcenter - 1, rxx = crinc - txsqr, rdist = cdist - rxx,
    rdp = cdp - rstride, rrgbp = crgbp - rstride, first = 1;
    r >= 0;
    r--, rdp -= rstride, rrgbp -= rstride,
    rxx -= txsqr, rdist -= rxx, first = 0)
  {
    if (greenloop (first))
      detect = 1;
    else if (detect)
      break;
  }

  return detect;
}

/* greenloop -- loop up and down from green center. */
static int greenloop (int restart)
{
  int detect;
  int g;
  int first;
  long txsqr = gxsqr + gxsqr;
  static int here, min, max;
  static long ginc, gxx, gcdist;        /* "gc" variables maintain correct */
  static unsigned long *gcdp;           /* values for bcenter position, */
  static unsigned char *gcrgbp;         /* despite modifications by blueloop */

  /* to gdist, gdp, grgbp. */

  if (restart)
  {
    here = gcenter;
    min = 0;
    max = gcolormax - 1;
    ginc = cginc;
  }

  detect = 0;

  /* Basic loop up. */
  for (g = here, gcdist = gdist = rdist, gxx = ginc,
    gcdp = gdp = rdp, gcrgbp = grgbp = rrgbp, first = 1;
    g <= max;
    g++, gdp += gstride, gcdp += gstride, grgbp += gstride, gcrgbp += gstride,
    gdist += gxx, gcdist += gxx, gxx += txsqr, first = 0)
  {
    if (blueloop (first))
    {
      if (!detect)
      {
        /* Remember here and associated data! */
        if (g > here)
        {
          here = g;
          rdp = gcdp;
          rrgbp = gcrgbp;
          rdist = gcdist;
          ginc = gxx;
        }
        detect = 1;
      }
    }
    else if (detect)
      break;
  }

  /* Basic loop down. */
  for (g = here - 1, gxx = ginc - txsqr, gcdist = gdist = rdist - gxx,
    gcdp = gdp = rdp - gstride, gcrgbp = grgbp = rrgbp - gstride,
    first = 1;
    g >= min;
    g--, gdp -= gstride, gcdp -= gstride, grgbp -= gstride, gcrgbp -= gstride,
    gxx -= txsqr, gdist -= gxx, gcdist -= gxx, first = 0)
  {
    if (blueloop (first))
    {
      if (!detect)
      {
        /* Remember here! */
        here = g;
        rdp = gcdp;
        rrgbp = gcrgbp;
        rdist = gcdist;
        ginc = gxx;
        detect = 1;
      }
    }
    else if (detect)
      break;
  }

  return detect;
}

/* blueloop -- loop up and down from blue center. */
static int blueloop (int restart)
{
  int detect;
  register unsigned long *dp;
  register unsigned char *rgbp;
  register unsigned long bdist, bxx;
  register int b, i = cindex;
  register long txsqr = bxsqr + bxsqr;
  register int lim;
  static int here, min, max;
  static long binc;

  if (restart)
  {
    here = bcenter;
    min = 0;
    max = bcolormax - 1;
    binc = cbinc;
  }

  detect = 0;

  /* Basic loop up. */
  /* First loop just finds first applicable cell. */
  for (b = here, bdist = gdist, bxx = binc, dp = gdp, rgbp = grgbp, lim = max;
    b <= lim;
    b++, dp++, rgbp++,
    bdist += bxx, bxx += txsqr)
  {
    if (*dp > bdist)
    {
      /* Remember new 'here' and associated data! */
      if (b > here)
      {
        here = b;
        gdp = dp;
        grgbp = rgbp;
        gdist = bdist;
        binc = bxx;
      }
      detect = 1;
      break;
    }
  }
  /* Second loop fills in a run of closer cells. */
  for (;
    b <= lim;
    b++, dp++, rgbp++,
    bdist += bxx, bxx += txsqr)
  {
    if (*dp > bdist)
    {
      *dp = bdist;
      *rgbp = i;
    }
    else
    {
      break;
    }
  }

  /* Basic loop down. */
  /* Do initializations here, since the 'find' loop might not get executed. */
  lim = min;
  b = here - 1;
  bxx = binc - txsqr;
  bdist = gdist - bxx;
  dp = gdp - 1;
  rgbp = grgbp - 1;
  /* The 'find' loop is executed only if we didn't already find something. */
  if (!detect)
    for (;
      b >= lim;
      b--, dp--, rgbp--,
      bxx -= txsqr, bdist -= bxx)
    {
      if (*dp > bdist)
      {
        /* Remember here! */
        /* No test for b against here necessary because b < here by definition. */
        here = b;
        gdp = dp;
        grgbp = rgbp;
        gdist = bdist;
        binc = bxx;
        detect = 1;
        break;
      }
    }
  /* The 'update' loop. */
  for (;
    b >= lim;
    b--, dp--, rgbp--,
    bxx -= txsqr, bdist -= bxx)
  {
    if (*dp > bdist)
    {
      *dp = bdist;
      *rgbp = i;
    }
    else
      break;
  }

  /* If we saw something, update the edge trackers. */

  return detect;
}

static void maxfill (unsigned long *buffer, long rside, long gside, long bside)
{
  register unsigned long maxv = ~0UL;
  register long i;
  register unsigned long *bp;

  for (i = rside * gside * bside, bp = buffer; i > 0; i--, bp++)
    *bp = maxv;
}
