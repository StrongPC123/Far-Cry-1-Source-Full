// CCurveObject : implementaion file
//
// CurveObject Class, data representaion of the curve. 
// Functionality :
//		1. Setting, retrieving and moving knots.
//      2. Calculation the curve point 
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

#include "stdafx.h"
#include "CurveObject.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//CCurveDllImpl CCurveObject::m_dllImpl;	//Initialise CCurveDllImpl

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL(CCurveObject, CObject, 1)

CCurveObject::CCurveObject()
{
	m_bIsValid = false;	//Curve Object not yet created
	
	//Initialise members
	m_bParabolic = false;
	m_bAveraging = false;

	m_bDiscreetY = false;

	m_nSmoothing = 1;
}

CCurveObject::~CCurveObject()
{
	//clean up
	RemoveAllKnots();
}

//////////////////////////////////////////////////////////////////////
//Curve Funtions
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//Curve Creation

BOOL CCurveObject::CreateCurve(LPCTSTR strName, CRect rcClipRect, UINT nSmoothing, DWORD dwFlags)
{			
	//Normalise ViewPort Rect
	rcClipRect.NormalizeRect();

	if(!rcClipRect.IsRectNull()) 
	{
		m_strName    = strName;		//Set Curve Name
		m_rcClipRect = rcClipRect;	//Set Cliprect

		//Add Head Knot
		CPoint ptHead;
		ptHead.x = rcClipRect.left;	
		ptHead.y = rcClipRect.bottom;

		CKnot* pKnotHead = new CKnot;
		pKnotHead->SetPoint(ptHead);
		m_arrKnots.Add(pKnotHead);

		//Add Tail Knot
		CPoint ptTail;
		ptTail.x = rcClipRect.right;
		ptTail.y = rcClipRect.top;
		
		CKnot* pKnotTail = new CKnot;
		pKnotTail->SetPoint(ptTail);
		m_arrKnots.Add(pKnotTail);

		m_bIsValid = true;
	}

	else 
		m_bIsValid = false;
	
	return m_bIsValid;
}

//////////////////////////////////////////////////////////////////////
//Curve Setting

/*void CCurveObject::SetCurveInfo(CURVEINFO tagInfo)
{
 
}

void CCurveObject::GetCurveInfo(LPCURVEINFO tagInfo)
{
 
}*/

//////////////////////////////////////////////////////////////////////
//Curve Calculation

UINT CCurveObject::GetCurveY(UINT ptX)
{	
	ASSERT(IsValid());	//Verify Curve Object 
	
	std::vector<CPoint> ptArray;
	
	for(int pos = 0; pos < m_arrKnots.GetSize(); pos++)
	{
		CKnot* Knot = GetKnot(pos);
		
		CPoint pt;
		pt.x = Knot->x;
		pt.y = Knot->y;
		ptArray.push_back(pt);
	}
		
	double iY = 0;//CCurveDllImpl::Interpolate(&ptArray, ptX, m_bParabolic, m_bAveraging, m_nSmoothing);	

	//Clip To Tail Knot																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																								
	CKnot* pTail = GetTailKnot();
	iY = __max(iY, (double)pTail->y);

	CKnot* pHead = GetHeadKnot();
	iY = __min(iY, (double)(pHead->y - 1));

	if(m_bDiscreetY) {

		//Clip Curve To Next Knot
		CKnot* pKnotNear = 
			FindNearKnot(CPoint(ptX, iY), KNOT_RIGHT);
		iY = __max(iY, pKnotNear->y);
	}

	return iY;
}

//////////////////////////////////////////////////////////////////////
//Curve Hit Testing

void CCurveObject::HitTest(CPoint point, LPHITINFO pHitInfo)
{	
	UINT  iIndex;	//Index of Knot			HTKNOT
	POINT ptCurve;	//Exact point on curve	HTCURVE

	pHitInfo->ptHit = point; //Set cursor position
 
	//if(PtOnKnot(point, 3, &iIndex))		
	if(PtOnKnot(point, 1000, &iIndex))
	{
		pHitInfo->wHitCode   = HTKNOT;
		pHitInfo->nKnotIndex = iIndex;
	}
	//else if(PtOnCurve(point, 0, &ptCurve))	
	else if(PtOnCurve(point, 1000, &ptCurve))	
	{
		pHitInfo->wHitCode = HTCURVE;
		pHitInfo->ptCurve  = ptCurve;
	}
	else pHitInfo->wHitCode = HTCANVAS;
}

BOOL CCurveObject::PtOnCurve(CPoint ptHit, UINT nInterval, POINT* pt)
{
	ASSERT(IsValid());	//Verify Curve Object 

	if(GetKnotCount() == -1)
		return FALSE;	//Curve Has No Knots
	
	int iY, iX;

	BOOL b = FALSE; 
	
	for(int i = 0; i<5; i++)
	{
		int iXdiff;

		if(i != 0) 
			(i%2 != 0) ? iXdiff = (+1)*((i+1)/2) : iXdiff = (-1)*(i/2);
		else iXdiff = 0;

		iX = ptHit.x + iXdiff;
		iY = GetCurveY(iX);
		
		if((iY > ptHit.y - 2) && (iY < ptHit.y + 2)) {
			b = TRUE;
			break;
		}
	}

	//Return curve exact point
	if(b) {	
		pt->x = iX;
		pt->y = iY;
	}
	
	return b;
}

BOOL CCurveObject::PtOnKnot(CPoint ptHit, UINT nInterval, UINT* nIndex)
{
	ASSERT(IsValid());	//Verify Curve Object 

	if(GetKnotCount() == -1)
		return FALSE;	//Curve Has No Knots
	
	int iIndex = - 1;

	for(int pos = 0; pos <= GetKnotCount(); pos++)
	{
		CKnot* pKnot = GetKnot(pos);

		CRect rcKnot;
		rcKnot.left   = pKnot->x - nInterval;
		rcKnot.right  = pKnot->x + nInterval;
		rcKnot.top    = pKnot->y - nInterval;
		rcKnot.bottom = pKnot->y + nInterval;

		if(rcKnot.PtInRect(ptHit)) {
			iIndex = pos;
			break;
		}
	}

	if(iIndex != -1) {
		*nIndex = iIndex; 
		return TRUE;
	}
	else return FALSE;
}	


//////////////////////////////////////////////////////////////////////
//Knot Functions
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//Operations

UINT CCurveObject::InsertKnot(CPoint ptCntKnot)
{
	ASSERT(IsValid());	//Verify Curve Object 
	
	//Create new Knot object
	CKnot* pKnot = new CKnot;
	pKnot->SetPoint(ptCntKnot);
	
	int iIndex = 0;
	

	std::vector<double> arrValues;
	double dValueToSeek = ptCntKnot.x;

	for(int pos = 0; pos <= GetKnotCount(); pos++)
	{
		CKnot* pKnot = GetKnot(pos);
		arrValues.push_back(pKnot->x);
	}

	//CCurveDllImpl dllCurve;
	double dIndex = 0;//CCurveDllImpl::IndexOfClosestValue(&arrValues, dValueToSeek) - 1;

	double dValue = arrValues[(int)dIndex];
	if(dValue < dValueToSeek) 
		dIndex += 1;
	
	iIndex = (int) dIndex;
	m_arrKnots.InsertAt(iIndex, pKnot);

	return iIndex;
}

BOOL CCurveObject::MoveKnot(CPoint ptMoveTo, UINT nIndex)
{	
	VERIFY(IsValid());	//Verify Curve Object 
	
	CPoint ptNext, ptPrev;
	
	CKnot* pKnotNext = GetNextKnot(nIndex);
	CKnot* pKnotPrev = GetPrevKnot(nIndex);

	//Restrict x movemnt to Viewport boundaries
	ptMoveTo.x = __min((int)ptMoveTo.x, (int) m_rcClipRect.right);
	ptMoveTo.x = __max((int)ptMoveTo.x, (int) m_rcClipRect.left);

	//Restrict x movement to next/prev knot
	ptMoveTo.x = __min((int)ptMoveTo.x, (int)(pKnotNext->x) - 1);
	ptMoveTo.x = __max((int)ptMoveTo.x, (int)(pKnotPrev->x) + 1);

	//Restrict y movement to Viewport boundaries
	ptMoveTo.y = __min((int)ptMoveTo.y, (int) m_rcClipRect.bottom);
	ptMoveTo.y = __max((int)ptMoveTo.y, (int) m_rcClipRect.top);

	//Restrict y movement (discreet y)
	if(m_bDiscreetY) {
		ptMoveTo.y = __max((int) ptMoveTo.y, (int)pKnotNext->y);
		ptMoveTo.y = __min((int) ptMoveTo.y, (int)pKnotPrev->y);
	}

	CKnot* pKnot = GetKnot(nIndex);
	
	BOOL b = (*pKnot != ptMoveTo);
	*pKnot = ptMoveTo;

	return b;
}

BOOL CCurveObject::RemoveKnot(UINT nIndex)
{
	ASSERT(IsValid());	//Verify Curve Object 

	BOOL b = TRUE;
	
	if(nIndex > GetKnotCount())
		b = FALSE;
	else 
	{
		CKnot *pKnot = (CKnot*) m_arrKnots.GetAt(nIndex);
		delete pKnot;

		m_arrKnots.RemoveAt(nIndex);
	}
	
	return b;
}

BOOL CCurveObject::RemoveAllKnots()
{
	ASSERT(IsValid());	//Verify Curve Object

	BOOL b = TRUE;

	for(int pos = 0; pos < m_arrKnots.GetSize(); pos++)
	{
		CKnot* pKnot = (CKnot*) m_arrKnots.GetAt(pos);
		delete pKnot;
	}

	m_arrKnots.RemoveAll();
	
	return b;
}

///////////////////////////////////////////////////////////////////////
//Searching

CKnot* CCurveObject::FindNearKnot(CPoint pt, UINT nDirection)
{	
	ASSERT(IsValid());	//Verify Curve Object 

	std::vector<double> arrValues;
	double dValueToSeek = pt.x;

	for(int pos = 0; pos < m_arrKnots.GetSize(); pos++)
	{
		CKnot* pKnot = (CKnot*) m_arrKnots.GetAt(pos);
		arrValues.push_back(pKnot->x);
	}

	//CCurveDllImpl dllCurve;
	double dIndex = 0;//dllCurve.IndexOfClosestValue(&arrValues, dValueToSeek) - 1;

	double dValue = arrValues[(int)dIndex];
	if((dValue <= dValueToSeek) && (nDirection == KNOT_RIGHT))
		dIndex += 1;
	if((dValue >= dValueToSeek) && (nDirection == KNOT_LEFT))
		dIndex -= 1;

	if(dIndex > GetKnotCount())
		dIndex = GetKnotCount();

	CKnot* pKnotNear = GetKnot((int) dIndex);
	return pKnotNear;

}



///////////////////////////////////////////////////////////////////////
//Retrieval/Iteration

CKnot* CCurveObject::GetHeadKnot()
{
	ASSERT(IsValid());	//Verify Curve Object 

	CKnot* pKnot; //Knot Object
	
	int iIndex = m_arrKnots.GetUpperBound();

	if(iIndex != -1)
	{
		pKnot = (CKnot*) m_arrKnots.GetAt(0);
		ASSERT(pKnot != NULL);
	}

	else pKnot = NULL;

	return pKnot;
}

CKnot* CCurveObject::GetTailKnot()
{
	ASSERT(IsValid());	//Verify Curve Object 

	CKnot* pKnot; //Knot Object
	
	int iIndex = m_arrKnots.GetUpperBound();
	
	if(iIndex != -1) 
	{
		pKnot = (CKnot*) m_arrKnots.GetAt(iIndex);
		ASSERT(pKnot != NULL);
	}

	else pKnot = NULL;

	return pKnot;
}

CKnot* CCurveObject::GetKnot(UINT nIndex)
{
	ASSERT(IsValid());	//Verify Curve Object 

	CKnot* pKnot; //Knot Object

	int iIndex = m_arrKnots.GetUpperBound();

	if(iIndex != -1)
	{
		pKnot = (CKnot*) m_arrKnots.GetAt(nIndex);
		ASSERT(pKnot != NULL);
	}

	else pKnot = NULL;

	return pKnot;
}

CKnot* CCurveObject::GetNextKnot(UINT nIndex)
{
	ASSERT(IsValid());	//Verify Curve Object 

	CKnot* pKnot;	//Knot pointer
	
	int iNextIndex = nIndex + 1;

	if(iNextIndex > m_arrKnots.GetUpperBound())
		pKnot = NULL;
	else	pKnot = (CKnot*) m_arrKnots.GetAt(iNextIndex);
	
	return pKnot;
}

CKnot* CCurveObject::GetPrevKnot(UINT nIndex)
{
	ASSERT(IsValid());	//Verify Curve Object 

	CKnot* pKnot;	//Knot pointer

	int iPrevIndex = nIndex - 1;

	if(iPrevIndex < 0)
		pKnot = NULL;
	else pKnot = (CKnot*) m_arrKnots.GetAt(iPrevIndex);

	return pKnot;
}

///////////////////////////////////////////////////////////////////////
//Staus

UINT CCurveObject::GetKnotCount()
{	
	ASSERT(IsValid());	//Verify Curve Object 

	int iKnots = m_arrKnots.GetUpperBound();
	return iKnots;
}

///////////////////////////////////////////////////////////////////////
//CCurveObject Serialisation										 //
///////////////////////////////////////////////////////////////////////

void CCurveObject::Serialize(CArchive& ar) 
{
	CObject::Serialize(ar);

	if (ar.IsStoring())
	{	
		int iSize = m_arrKnots.GetSize();
		ar << iSize;

		for(int pos = 0; pos < iSize; pos++)
		{
			CKnot* pKnot = (CKnot* )m_arrKnots.GetAt(pos);
			pKnot->Serialize(ar);
		}
	
		CRect rc = m_rcClipRect;	
		ar << rc.left << rc.bottom << rc.right << rc.top;

		ar << m_strName;

		ar << m_bParabolic;			
		ar << m_bAveraging;
		ar << m_nSmoothing;
	}

	else
	{	
		int iSize;
		ar >> iSize;

		//RemoveAllKnots();
		for(int pos = 0; pos < iSize; pos++)
		{	
			CKnot* pKnot = new CKnot;
			pKnot->Serialize(ar);
			m_arrKnots.Add(pKnot);
		}
			
		
		CRect rc;				
		ar >> rc.left >> rc.bottom >> rc.right >> rc.top;
		m_rcClipRect = rc;	

		ar >> m_strName;

		ar >> m_bParabolic;			
		ar >> m_bAveraging;
		ar >> m_nSmoothing;

		m_bIsValid = true; //Set Valid Automaticaly
	}
}


void CCurveObject::Serialize( CXmlArchive &xmlAr )
{
	if (xmlAr.bLoading)
	{
		// Loading
		CLogFile::WriteLine("Loading Curve settings...");

		XmlNodeRef curve = xmlAr.root->findChild( "Curve" );
		if (!curve)
			return;

		curve->getAttr( "Name",m_strName );
		curve->getAttr( "Parabolic",m_bParabolic );
		curve->getAttr( "Averaging",m_bAveraging );
		curve->getAttr( "Smoothing",m_nSmoothing );

		curve->getAttr( "ClipLeft",m_rcClipRect.left );
		curve->getAttr( "ClipRight",m_rcClipRect.right );
		curve->getAttr( "ClipTop",m_rcClipRect.top );
		curve->getAttr( "ClipBottom",m_rcClipRect.bottom );

		for(int pos = 0; pos < curve->getChildCount(); pos++)
		{
			CKnot* pKnot = new CKnot;
			XmlNodeRef knot = curve->getChild(pos);
			knot->getAttr( "X",pKnot->x );
			knot->getAttr( "Y",pKnot->y );
			knot->getAttr( "Val",pKnot->dwData );
			m_arrKnots.Add(pKnot);
		}
	}
	else
	{
		// Storing
		CLogFile::WriteLine("Storing Curve settings...");

		XmlNodeRef curve = xmlAr.root->newChild( "Curve" );

		curve->setAttr( "Name",m_strName );
		curve->setAttr( "Parabolic",m_bParabolic );
		curve->setAttr( "Averaging",m_bAveraging );
		curve->setAttr( "Smoothing",m_nSmoothing );

		curve->setAttr( "ClipLeft",m_rcClipRect.left );
		curve->setAttr( "ClipRight",m_rcClipRect.right );
		curve->setAttr( "ClipTop",m_rcClipRect.top );
		curve->setAttr( "ClipBottom",m_rcClipRect.bottom );

		for(int pos = 0; pos < m_arrKnots.GetSize(); pos++)
		{
			CKnot* pKnot = (CKnot* )m_arrKnots.GetAt(pos);
			XmlNodeRef knot = curve->newChild( "Knot" );
			knot->setAttr( "X",pKnot->x );
			knot->setAttr( "Y",pKnot->y );
			knot->setAttr( "Val",pKnot->dwData );
		}
	}
}

