// CCurveObjectDef : header file
//
// CCurveObjectDef file, defenition file for the CurveObject Class 
//      
// Copyright Johan Janssens, 2001 (jjanssens@mail.ru)
// Feel free to use and distribute. May not be sold for profit. 
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name is included.
// If the source code in this file is used in any commercial application
// then acknowledgement must be made to the author of this file
// 
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage of buiness that this
// product may cause
//
// Please use and enjoy. Please let me know of any bugs/mods/improvements
// that you have found/implemented and I will fix/incorporate them into 
// this file 

#ifndef _JANSSENS_JOHAN_CURVEOBJECT_DEFENITIONS_H_
#define _JANSSENS_JOHAN_CURVEOBJECT_DEFENITIONS_H_

//////////////////////////////////////////////////
//Knot Identifiers
#define KNOT_RIGHT	0x0001	
#define KNOT_LEFT	0x0002

///////////////////////////////////////////////////
//Curve Flags
#define CURVE_PARABOLIC	0x00000001
#define CURVE_SMOOTHING 0x00000002
#define CURVE_DISCREETY	0x00000004

//////////////////////////////////////////////////
//New WM_NCHITTEST Mouse Position Codes
#define HTUSER		0x1000		
#define HTCANVAS	HTUSER + 1
#define HTCURVE		HTUSER + 2
#define HTKNOT		HTUSER + 3

//////////////////////////////////////////////////
//Hit Info Structure

typedef struct tagHITINFOSTRUCT {

	POINT  ptHit;		//Cursor position
	WORD	wHitCode;	//Hit Code 
	UINT	nKnotIndex;	//Index of knot			HTKNOT
	POINT	ptCurve;	//Exact point on curve	HTCURVE
} HITINFO, FAR *LPHITINFO;

//////////////////////////////////////////////////
//Curve Info Structure

typedef struct tagCURVEINFOSTRUCT {

	LPCTSTR lpszName;
	RECT	rcClip;
	DWORD	dwFlags;
	UINT	nSmoothing;
		
} CURVEINFO, FAR *LPCURVEINFO;

//////////////////////////////////////////////////
// Bit Operators
#define BITSET(dw, bit)		(((dw) & (bit)) != 0L)

/////////////////////////////////////////////////////////////////////////////
#endif // _JANSSENS_JOHAN_CURDVEOBJECT_DEFENITIONS_H_