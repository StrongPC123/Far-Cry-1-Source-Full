// XTMemDC.h interface for the CXTMemDC class.
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XTMEMDC_H__)
#define __XTMEMDC_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
// Summary: CXTMemDC is a CDC derived class.  CXTMemDC is an extension of CDC that
//			helps eliminate screen flicker when windows are resized, by painting
//			to an off screen bitmap.  The class then uses CDC::BitBlt to copy the
//			bitmap back into the current device context after all items have been
//			painted.
class _XT_EXT_CLASS CXTMemDC : public CDC  
{
public:
    DECLARE_DYNAMIC(CXTMemDC);
    
	// Input:	pDC - A Pointer to the current device context. 
	//			rect - Represents the size of the area to paint.
	//			clrColor - An RGB value that represents the background color of the area to paint.
	//			Defaults to COLOR_3DFACE.  Pass in a value of -1 to disable background
	//			painting.
	// Summary:	Constructs a CXTMemDC object.
    CXTMemDC(CDC* pDC,const CRect& rect,COLORREF clrColor=xtAfxData.clr3DFace);

    // Summary: Destroys a CXTMemDC object, handles cleanup and de-allocation.
    virtual ~CXTMemDC();

	// Summary: This member function is called to set the valid flag to false so the
	//			offscreen device context will not be drawn.
	void Discard();

    // Summary: This member function gets content from the given DC.
	void FromDC();

	// Returns: A reference to the CBitmap object associated with the memory device context.
    // Summary:	This member function retrieves a reference to the CBitmap object
	//			associated with the memory device context. 
	CBitmap& GetBitmap();
	
protected:
    
	CDC*	m_pDC;			// Saves the CDC passed in constructor.
	BOOL	m_bValid;		// Flag used for autodraw in destructor.
	CRect	m_rc;			// Rectangle of the drawing area.
	CBitmap m_bitmap;		// Offscreen bitmap.
	HBITMAP m_hOldBitmap;	// Original GDI object.
	
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE CBitmap& CXTMemDC::GetBitmap() {
	return m_bitmap;
}
AFX_INLINE void CXTMemDC::Discard() {
	m_bValid = FALSE;
}
AFX_INLINE void CXTMemDC::FromDC() {
	BitBlt(0, 0, m_rc.Width(), m_rc.Height(), m_pDC, m_rc.left, m_rc.top, SRCCOPY);            
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTMEMDC_H__)