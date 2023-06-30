// XTSplitterDock.h interface for the CXTSplitterRowDock class.
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

#if !defined(__XTSPLITTERDOCK_H__)
#define __XTSPLITTERDOCK_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// class forwards.

class CXTDockBar;

//////////////////////////////////////////////////////////////////////
// Summary: CXTSplitterDock is a stand alone base class.  It is used to create a
//			CXTSplitterDock class object.
class _XT_EXT_CLASS CXTSplitterDock
{
public:

	// Input:	pBar - A pointer to a valid CXTDockBar object.
	//			rcCurrent - A reference to a valid CRect object.
	//			bar - Type of splitter bar, either XT_SPLITTER_VERT or XT_SPLITTER_HORZ.
	//			nPos - Index into a control bar array.
    // Summary:	Constructs a CXTSplitterDock object.
	CXTSplitterDock(CXTDockBar *pBar,const CRect &rcCurrent,const int nType,const int nPos);

	// Summary: Destroys a CXTSplitterDock object, handles clean up and de-allocation.
	virtual ~CXTSplitterDock();
	
	/////////////////////////////////////////////////////////////////////////
	// Current Size and Position
	/////////////////////////////////////////////////////////////////////////

	CRect  m_rcCurrent; // Current size of the splitter rect.
	CRect  m_rcTrack;   // Current size of the tracking rect.
	CPoint m_ptCurrent; // Represents the current cursor position.

	/////////////////////////////////////////////////////////////////////////
	// Width of the Row
	/////////////////////////////////////////////////////////////////////////

	int m_nStartHeight;		// Original height of the splitter.
	int m_nCurrentHeight;	// Current height of the splitter.
	int m_nMaxHeight;		// Maximum height for the splitter.
	int m_nMinHeight;		// Minimum height for the splitter.

	/////////////////////////////////////////////////////////////////////////
	// Vertical or Horizontal
	/////////////////////////////////////////////////////////////////////////

	int m_nType;			// Type of splitter bar either, XT_SPLITTER_VERT or XT_SPLITTER_HORZ.
	int m_nControlBar;		// Index into a control bar array.
	CXTDockBar*	m_pDockBar; // Pointer to the dockbar.

	// Input:	point - Represents the current cursor position.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function is called to move the splitter during tracking
	//			operations. 
	virtual bool Move(CPoint point);

	// Input:	point - Represents the current cursor position.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function is called during tracking operations. 
	virtual bool StartTrack(CPoint point);

	// Input:	point - Represents the current cursor position.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function is called to terminate the tracking operation.
	virtual bool EndTrack(CPoint point);

	// Returns: TRUE if the splitter is horizontal, otherwise returns FALSE.
	// Summary:	This member function is called to determine if the splitter is horizontal.
	BOOL IsHorz();

	// Input:	point - A reference to a valid CPoint object.
	// Returns: TRUE if the cursor position specified by 'point' falls inside of the splitter
	//			area, otherwise returns FALSE.
	// Summary:	This member function is called to determine if the cursor position
	//			is inside of the splitter area. 
	BOOL HitTest(const CPoint &point);

	// Input:	nLength - New size for the splitter.
	// Summary:	This member function is called to set the length for the splitter.
	void SetLength(int nLength);

	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function is called to display the tracking rect when the
	//			splitter is moved. 
	bool OnInvertTracker();

	// Input:	pDC - A CDC pointer to a valid device context.
	// Summary:	This member function is called to draw the splitter during paint operations.
	void Draw(CDC *pDC);
	
protected:

	// Summary: This member function is called internally to set the new height for
	//			the splitter bar based on the current control bar size.
	virtual void SetNewHeight();

	// Summary: This member function is called internally to calculate the new height
	//			for the splitter bar based on the current control bar size.
	virtual void CalcMaxMinHeight();
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTSplitterRowDock is a CXTSplitterDock derived class.  It is used to
//			create a CXTSplitterRowDock class object.
class _XT_EXT_CLASS CXTSplitterRowDock : public CXTSplitterDock  
{
public:

	// Input:	pBar - A pointer to a valid CXTDockBar object.
	//			rcCurrent - A reference to a valid CRect object.
	//			nType - Type of splitter bar, either XT_SPLITTER_VERT or XT_SPLITTER_HORZ.
	//			nPos - Index into a control bar array.
    // Summary:	Constructs a CXTSplitterRowDock object.
	CXTSplitterRowDock(CXTDockBar *pBar,const CRect &rcCurrent,const int nType,const int nPos);

	// Input:	point - Represents the current cursor position.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function is called to move the splitter during tracking
	//			operations. 
	virtual bool Move(CPoint point);

protected:

	// Summary: This member function is called internally to set the new height for
	//			the splitter bar based on the current control bar size.
	virtual void SetNewHeight();

	// Summary: This member function is called internally to calculate the new height
	//			for the splitter bar based on the current control bar size.
	virtual void CalcMaxMinHeight();
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE BOOL CXTSplitterDock::IsHorz() {
	return (m_nType == XT_SPLITTER_HORZ);
}
AFX_INLINE BOOL CXTSplitterDock::HitTest(const CPoint &point) {
	return m_rcCurrent.PtInRect(point);
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTSPLITTERDOCK_H__)