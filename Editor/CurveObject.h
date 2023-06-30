// CCurveObject : header file
//
// CurveObject Class, data representaion of the curve. 
// Functionality :
//		1. Setting, retrieving and moving knots.
//      2. Curve calculation
//		3. HitTesting 
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


#ifndef _JANSSENS_JOHAN_KNOT_H_
#define _JANSSENS_JOHAN_KNOT_H_

//Knot Class
class CKnot : public CObject
{
public :

	UINT  x;
	UINT  y;
	DWORD dwData;

public :

	CKnot() : x(0), y(0), dwData(true) {}	//Constructors	
	CKnot(int ptX, int ptY) : 
			x(ptX), y(ptY), dwData(0)  {}

	void SetPoint(CPoint pt)  {	x = pt.x; y = pt.y;}	//Setting
	void SetData(DWORD data)  { dwData = data;}

	void GetPoint(LPPOINT pt) { pt->x = x; pt->y = y;}	//Extraction

	//Operator overloading
	void operator = (CKnot knot)   {x = knot.x; y = knot.y; dwData  = knot.dwData; }
	void operator = (CPoint point) {x = point.x; y = point.y;}

	bool operator != (CPoint point)
	{
		bool b;
		((x != point.x) && (y != point.y)) ? b = true : b = false;
		return b;
	}

	void Serialize(CArchive& ar)
	{
		if(ar.IsStoring())
			ar << x << y << dwData;
		else
			ar >> x >> y >> dwData;
	}
};

/////////////////////////////////////////////////////////////////////////////
#endif // _JANSSENS_JOHAN_KNOT_H_


#ifndef _JANSSENS_JOHAN_CURVEOBJECT_H_
#define _JANSSENS_JOHAN_CURVEOBJECT_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CurveObjectDef.h"

class CXmlArchive;

class CCurveObject  : public CObject
{
DECLARE_SERIAL( CCurveObject )
public:
	CCurveObject();
	virtual ~CCurveObject();

	//Create Curve Object
	BOOL CreateCurve(LPCTSTR strName, CRect rcClipRec, UINT nSmoothing = 1, DWORD dwFlags = NULL);
	
	void Serialize( CXmlArchive &xmlAr );

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCurveObject)
	public:
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL
	
	//Curve functions
	void  HitTest(CPoint ptHit, LPHITINFO pHitInfo);
	BOOL  PtOnKnot (CPoint ptHit, UINT nInterval, UINT*  iIndex);
	BOOL  PtOnCurve(CPoint ptHit, UINT nInterval, POINT* ptCurve);
	
	UINT GetCurveY(UINT ptX);

	//void GetCurveInfo(LPCURVEINFO tagInfo);
	//void SetCurveInfo(CURVEINFO   tagInfo);

	//Knot functions
	UINT InsertKnot (CPoint ptCntKnot);					//Operations				
	BOOL MoveKnot	(CPoint ptMoveTo, UINT nIndex);

	BOOL RemoveKnot		(UINT nIndex);					
	BOOL RemoveAllKnots	();

	CKnot* FindNearKnot (CPoint pt, UINT nDirection);	//Searching

	CKnot* GetKnot(UINT nIndex);						//Access

	CKnot* GetHeadKnot();								//Retrieval
	CKnot* GetTailKnot();

	CKnot* GetNextKnot(UINT nIndex);					//Irritation
	CKnot* GetPrevKnot(UINT nIndex);

	UINT GetKnotCount();								//Status


public :

	//Curve Settings
	BOOL m_bParabolic;
	BOOL m_bAveraging;

	BOOL m_bDiscreetY;
	UINT m_nSmoothing;

	//Curve Bounding Rect
	CRect m_rcClipRect;	
	
	//Curve Name
	CString m_strName;

protected:

		
private:

	CObArray m_arrKnots;					//Knot Object Array
	BOOL  m_bIsValid;						//Curve Object Is Initialised
	inline BOOL IsValid() { return m_bIsValid;};

//	static CCurveDllImpl	m_dllImpl;		//Wrapper for Curve Dll
	
};

////////////////////////////////////////////////////////////////////////////
#endif // _JANSSENS_JOHAN_CURVEOBJECT_H_
