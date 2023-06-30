// XTResizeRect.h: interface for the CXTResizeRect class.
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//--------------------------------------------------------------------
// Based on the resizable classes created by Torben B. Haagh. Used by permission.
// http://www.codeguru.com/dialog/torbenResizeDialog.shtml
//--------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////

#if !defined(__XTRESIZERECT_H__)
#define __XTRESIZERECT_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: Float data type used by resizing windows.
typedef float XT_RESIZE;

//////////////////////////////////////////////////////////////////////
// Summary: XT_RESIZERECT structure is a stand alone structure class.  It defines
//			the coordinates of the upper-left and lower-right corners of a rectangle.
struct XT_RESIZERECT
{
	XT_RESIZE left;		// Specifies the x-coordinate of the upper-left corner of a rectangle.
	XT_RESIZE top;		// Specifies the y-coordinate of the upper-left corner of a rectangle.
	XT_RESIZE right;	// Specifies the x-coordinate of the lower-right corner of a rectangle.
	XT_RESIZE bottom;	// Specifies the y-coordinate of the lower-right corner of a rectangle.
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTResizeRect is an XT_RESIZERECT structure derived class.  The 
//			CXTResizeRect class is similar to an XT_RESIZERECT structure.  CXTResizeRect
//			also includes member functions to manipulate CXTResizeRect objects and
//			XT_RESIZERECT structures.
class _XT_EXT_CLASS CXTResizeRect: public XT_RESIZERECT
{
public:

    // Summary: Constructs a CXTResizeRect object.
	CXTResizeRect();

	// Input:	rc - Refers to the RECT structure with the coordinates for CXTResizeRect.
    // Summary:	Constructs a CXTResizeRect object.
	CXTResizeRect(const RECT& rc);

	// Input:	rrc - Refers to the XT_RESIZERECT structure with the coordinates for 
	//			CXTResizeRect.
    // Summary:	Constructs a CXTResizeRect object.
	CXTResizeRect(const XT_RESIZERECT& rrc);

	// Input:	l - Specifies the left position of CXTResizeRect.
	//			t - Specifies the top of CXTResizeRect.
	//			r - Specifies the right position of CXTResizeRect.
	//			b - Specifies the bottom of CXTResizeRect.
    // Summary:	Constructs a CXTResizeRect object.
	CXTResizeRect(XT_RESIZE l,XT_RESIZE t,XT_RESIZE r,XT_RESIZE b);

	// Input:	rc - Refers to a source rectangle. It can be a RECT or CRect.
	// Returns: A reference to a CXTResizeRect object.
	// Summary:	This operator copies the dimensions of a rectangle to CXTResizeRect.
	CXTResizeRect& operator = (const RECT& rc);

	// Input:	rrc - Refers to a source rectangle. It can be a XT_RESIZERECT or CXTResizeRect.
	// Returns: A reference to a CXTResizeRect object.
	// Summary:	This operator copies the dimensions of a rectangle to CXTResizeRect.
	CXTResizeRect& operator = (const XT_RESIZERECT& rrc);

	// Input:	rrc - Points to an XT_RESIZERECT structure or a CXTResizeRect object that
	//			contains the number of units to inflate each side of CXTResizeRect.
	// Returns: A reference to a CXTResizeRect object.
	// Summary:	This operator adds the specified offsets to CXTResizeRect or inflates
	//			CXTResizeRect. 
	CXTResizeRect& operator += (const XT_RESIZERECT& rrc);

	// Input:	rrc - Points to an XT_RESIZERECT structure or a CXTResizeRect object that
	//			contains the number of units to inflate each side of the return value.
	// Returns: The resulting CXTResizeRect object.
	// Summary:	This operator adds the given offsets to CRect or inflates CRect. 
	CXTResizeRect operator + (const XT_RESIZERECT& rrc);

	// Input:	rrc - Contains an XT_RESIZERECT or a CXTResizeRect.
	// Returns: A CXTResizeRect that is the intersection of CXTResizeRect and 'rrc'.  The 
	//			intersection is the largest rectangle that is contained in both rectangles.
	// Summary:	This operator creates the intersection of CXTResizeRect and a rectangle,
	//			and returns the resulting CXTResizeRect.  
	CXTResizeRect operator & (const XT_RESIZERECT& rrc);

	// Input:	rrc - Refers to a source rectangle. It can be an XT_RESIZERECT or a CXTResizeRect.
	// Returns: true if equal, otherwise returns false.
	// Summary:	This operator determines whether CXTResizeRect is equal to a rectangle.
	bool operator == (const XT_RESIZERECT& rrc);

	// Input:	rrc - Refers to a source rectangle. It can be an XT_RESIZERECT or a CXTResizeRect.
	// Returns: false if equal, otherwise returns true.
	// Summary:	This operator determines whether CXTResizeRect is not equal to a rectangle.
	bool operator != (const XT_RESIZERECT& rrc);

	// Summary: This operator converts a CXTResizeRect to a CRect.  When you use this
	//			function, you do not need the address-of (&) operator.  This operator
	//			will be automatically used when you pass a CXTResizeRect object to
	//			a function that expects a CRect.
	operator CRect();

	// Returns: true if normalized, otherwise returns false.
	// Summary:	This member function determines if CXTResizeRect is normalized.  
	bool IsNormalized();

	// Returns: The width of a CXTResizeRect.
	// Summary:	This member function calculates the width of a CXTResizeRect by subtracting
	//			the left value from the right value.  The resulting value can be negative.
	XT_RESIZE Width();

	// Returns: The height of a CXTResizeRect.
	// Summary:	This member function calculates the height of a CXTResizeRect by subtracting
	//			the top value from the bottom value.  The resulting value can be negative.
	XT_RESIZE Height();
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE CXTResizeRect::CXTResizeRect(const RECT& rc) {
	(operator =)(rc);
}
AFX_INLINE CXTResizeRect::CXTResizeRect(const XT_RESIZERECT& rrc) {
	(operator =)(rrc);
}
AFX_INLINE CXTResizeRect CXTResizeRect::operator + (const XT_RESIZERECT& rrc) { 
	return CXTResizeRect(left + rrc.left, top + rrc.top, right + rrc.right, bottom += rrc.bottom); 
}
AFX_INLINE bool CXTResizeRect::IsNormalized() {
	return ((left <= right) && (top <= bottom));
}
AFX_INLINE bool CXTResizeRect::operator == (const XT_RESIZERECT& rrc) { 
	return left==rrc.left && top==rrc.top && right==rrc.right && bottom==rrc.bottom; 
}
AFX_INLINE bool CXTResizeRect::operator != (const XT_RESIZERECT& rrc) { 
	return !operator==(rrc); 
}
AFX_INLINE CXTResizeRect::operator CRect() { 
	return CRect((int) left, (int) top, (int) right, (int) bottom); 
}
AFX_INLINE XT_RESIZE CXTResizeRect::Width() { 
	return right-left; 
}
AFX_INLINE XT_RESIZE CXTResizeRect::Height() { 
	return bottom-top; 
}

//////////////////////////////////////////////////////////////////////

//:Associate with "CXTResizeRect"

//////////////////////////////////////////////////////////////////////
#define SZ_RESIZE(x)	CXTResizeRect(0,0,x,x)  // Resize.
#define SZ_REPOS(x)		CXTResizeRect(x,x,x,x)  // Reposition.
#define SZ_HORRESIZE(x)	CXTResizeRect(0,0,x,0)  // Horizontal resize.
#define SZ_HORREPOS(x)	CXTResizeRect(x,0,x,0)  // Horizontal reposition.
#define SZ_VERRESIZE(x)	CXTResizeRect(0,0,0,x)  // Vertical resize.
#define SZ_VERREPOS(x)	CXTResizeRect(0,x,0,x)  // Vertical reposition.

//////////////////////////////////////////////////////////////////////

#endif // !defined(__XTRESIZERECT_H__)