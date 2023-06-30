// CCurveWnd : header file
//
// CurveWnd Class, visual representaion of the curve. 
// Functionality :
//		1. Draw Knots, Curve
//      2. Handle mouse and keyboard input 
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

#ifndef _JANSSENS_JOHAN_CURVEWND_H_
#define _JANSSENS_JOHAN_CURVEWND_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CurveObject.h"

class CCurveWnd : public CWnd
{
public:
	CCurveWnd();
	virtual ~CCurveWnd();

	//Create Window
	BOOL Create(LPCTSTR strName, const RECT &rect, CWnd* pWndParent, UINT nID, BOOL CreateCurveObj = true);

	//Knot functions
	UINT GetActiveKnot();
	void SetActiveKnot(UINT nIndex);

	//Inline CurveObject Access function
	CCurveObject* GetCurveObject();
	void          SetCurveObject(CCurveObject* pObject, BOOL bRedraw = FALSE);

protected:


	HITINFO* m_pHitInfo;	  //Hit Info Structure

	UINT   m_nActiveKnot; //Index of active knot object;
	int	   m_iKnotRadius; 

	BOOL  m_bTracking;  //Tracking Knots

	//Creation helper functions
	void CreateCurveObject(CString strCurve);
	
	//Drawing helper functions
	void DrawWindow(CDC* pDC);
	void DrawGrid  (CDC* pDC);
	void DrawCurve (CDC* pDC);
	void DrawKnots (CDC* pDC);

	//Hit Testing helper functions
	WORD  HitTest  	  (CPoint ptHit);

	//Tracking support helper functions
	void StartTracking();
	void TrackKnot(CPoint point);
	void StopTracking();

	//{{AFX_MSG(CCurveWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private :

	CCurveObject* m_pCurve;	//CurveObject associate with this window

};

//////////////////////////////////////////////////////////////////////////////
#endif // _JANSSENS_JOHAN_CURVEWND_H_
