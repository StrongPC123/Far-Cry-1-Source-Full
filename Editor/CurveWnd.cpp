// CCurveWnd : implementation file
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

#include "stdafx.h"
#include "CurveWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCurveWnd

CCurveWnd::CCurveWnd()
{
	//Initialise members
	m_bTracking = false;
	m_nActiveKnot = - 1;

	m_iKnotRadius = 3;

}

CCurveWnd::~CCurveWnd()
{
}


BEGIN_MESSAGE_MAP(CCurveWnd, CWnd)
	//{{AFX_MSG_MAP(CCurveWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCurveWnd message handlers

int CCurveWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//Create HitInfo Structure
	m_pHitInfo = new HITINFO;

	return 0;
}

void CCurveWnd::OnDestroy() 
{
	CWnd::OnDestroy();
	
	//clean up
	delete m_pHitInfo;
	delete m_pCurve;
}

/////////////////////////////////////////////////////////////////////////////
// CCurveWnd initialisation										       //
/////////////////////////////////////////////////////////////////////////////

BOOL CCurveWnd::Create(LPCTSTR lpszCurveName, const RECT &rect, CWnd* pWndParent, UINT nID, BOOL CreateCurveObj)
{
	ASSERT(nID != NULL);
	ASSERT(pWndParent != NULL);

	//Create CurveWnd
	DWORD dwExStyle			= WS_EX_CLIENTEDGE  ;
	LPCTSTR lpszClassName	= NULL;
	LPCTSTR lpszWindowName	= lpszCurveName;
	DWORD dwStyle			= WS_CHILD | WS_VISIBLE | WS_BORDER;
	const RECT& rc			= rect;
	CWnd* pParentWnd		= pWndParent;
	UINT nClientWndID		= nID;
	LPVOID lpParam			= NULL;
	
	BOOL b = CreateEx(dwExStyle, lpszClassName, lpszWindowName, 
		dwStyle, rc, pParentWnd, nClientWndID, lpParam);

	if(CreateCurveObj)
		CreateCurveObject(lpszCurveName);

	return b;
}

void CCurveWnd::CreateCurveObject(CString strCurve)
{
	//Create CurveObject Pointer
	CRect rcCurveWnd;
	GetClientRect(&rcCurveWnd);
	rcCurveWnd.DeflateRect(1,1);

	m_pCurve = new CCurveObject();	
	m_pCurve->CreateCurve(strCurve, rcCurveWnd);
}

/////////////////////////////////////////////////////////////////////////////
// CCurveWnd drawing										               //
/////////////////////////////////////////////////////////////////////////////

BOOL CCurveWnd::OnEraseBkgnd(CDC* pDC) { return TRUE;}

void CCurveWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect rcClient;
	GetClientRect(&rcClient);
	
	//Draw Window
	DrawWindow(&dc);
}

void CCurveWnd::DrawWindow(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	int cx = rcClient.Width();
	int cy = rcClient.Height();
	
	//Create Memory Device Context
	CDC MemDC;
	MemDC.CreateCompatibleDC(pDC);

	//Draw BackGround
	CBitmap bitmapBkGnd;;
	bitmapBkGnd.CreateCompatibleBitmap(pDC, cx, cy);
	CBitmap*  pOldbitmapBkGnd = MemDC.SelectObject(&bitmapBkGnd);
	
	//Draw Grid
	DrawGrid(&MemDC);
	
	//Draw Knots and Curve
	if(AfxIsValidAddress(m_pCurve, sizeof(CObject))) {
		
		DrawCurve(&MemDC);
		DrawKnots(&MemDC);
	}

	pDC->BitBlt (0, 0, cx, cy, &MemDC, 0, 0, SRCCOPY);
	MemDC.SelectObject(pOldbitmapBkGnd);
	MemDC.DeleteDC();

}

void CCurveWnd::DrawGrid(CDC* pDC)
{
	CRect rcClipBox;
	pDC->GetClipBox(&rcClipBox);

	int cx = rcClipBox.Width();
	int cy = rcClipBox.Height();
	
	LOGBRUSH logBrush;
	logBrush.lbStyle = BS_SOLID;
	logBrush.lbColor = RGB(75, 75, 75);

	CPen pen;
	pen.CreatePen(PS_COSMETIC | PS_ALTERNATE, 1, &logBrush);
	CPen* pOldPen = pDC->SelectObject(&pen);
	
	//Draw Vertical Grid Lines
	for(int y = 1; y < 10; y++) {
		pDC->MoveTo(y*cx/10, cy);
		pDC->LineTo(y*cx/10, 0);
	}

	//Draw Horizontal Grid Lines
	for(int x = 1; x < 10; x++) {
		pDC->MoveTo(0, x*cy/10);
		pDC->LineTo(cx, x*cy/10);
	}

	pDC->SelectObject(pOldPen);
}

void CCurveWnd::DrawCurve(CDC* pDC)
{
	CRect rcClipBox;
	pDC->GetClipBox(&rcClipBox);

	int cx = rcClipBox.Width();
	int cy = rcClipBox.Height();
	int iDrawX;

	//Draw Curve
	// create and select a thick, white pen
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	CPen* pOldPen = pDC->SelectObject(&pen);
	
	int iY;
	iY = m_pCurve->GetCurveY(1);	//Get Starting point
	pDC->MoveTo(1, iY);

	

	for(int iX = 1; iX < cx; iX++)
	{
		int iYnext = m_pCurve->GetCurveY((float) iX / cx * m_pCurve->m_rcClipRect.right);

		iDrawX = (float) iYnext / m_pCurve->m_rcClipRect.bottom * cy;
 		
		pDC->LineTo(CPoint(iX - 1, iDrawX));

		iY = iDrawX; //Set next starting point
	}

	// Put back the old objects
    pDC->SelectObject(pOldPen);
}

void CCurveWnd::DrawKnots(CDC* pDC)
{
	// create and select a thin, white pen
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	CPen* pOldPen = pDC->SelectObject(&pen);

	CRect rcClipBox;
	pDC->GetClipBox(&rcClipBox);
	int cx = rcClipBox.Width();
	int cy = rcClipBox.Height();

	//Draw Knots
	for(int pos = 1; pos < m_pCurve->GetKnotCount(); pos++)
    {
		// Create and select a solid white brush
		CKnot* pKnot =	m_pCurve->GetKnot(pos);
		CKnot cTmpKnow;
		cTmpKnow.x = pKnot->x;
		cTmpKnow.y = pKnot->y;
		cTmpKnow.dwData = pKnot->dwData;

				

		//Set knot brush color
		int cyColor;
		pKnot->dwData ? cyColor = 255 : cyColor = 0;
		CBrush brush(RGB(cyColor,cyColor,cyColor));
		CBrush* pOldBrush = pDC->SelectObject(&brush);

		cTmpKnow.x = (float) cTmpKnow.x / m_pCurve->m_rcClipRect.right * cx;
		cTmpKnow.y = (float) cTmpKnow.y / m_pCurve->m_rcClipRect.bottom * cy;
		
		//Draw knot
		CRect rc;
		rc.left   = cTmpKnow.x - m_iKnotRadius;
		rc.right  = cTmpKnow.x + m_iKnotRadius;
		rc.top    = cTmpKnow.y - m_iKnotRadius;
		rc.bottom = cTmpKnow.y + m_iKnotRadius;

		pDC->Ellipse(&rc); 

		pDC->SelectObject(pOldBrush);
	}

	// put back the old objects
	pDC->SelectObject(pOldPen);
}



/////////////////////////////////////////////////////////////////////////////
// CCurveWnd Message Handlers										       //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//Mouse Message Handlers

void CCurveWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if(m_bTracking)
		return;

	SetFocus();

	switch(m_pHitInfo->wHitCode)
	{
	case HTKNOT :
		{
			StartTracking();
			SetActiveKnot(m_pHitInfo->nKnotIndex);

		} break;
	case HTCANVAS :
		{
			SetActiveKnot(-1);
		}
	default :		break; //do nothing
	}
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CCurveWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CWnd::OnRButtonDown(nFlags, point);
}

void CCurveWnd::OnLButtonDblClk(UINT nFlags, CPoint point) 
{

	RECT rc;
	GetClientRect(&rc);

	point.x = (float) point.x / rc.right * m_pCurve->m_rcClipRect.right;
	point.y = (float) point.y / rc.bottom * m_pCurve->m_rcClipRect.bottom;

	switch(m_pHitInfo->wHitCode)
	{
	case HTCURVE : 
		{	
			int iIndex = m_pCurve->InsertKnot(point);
			SetActiveKnot(iIndex);

			RedrawWindow();
		} break;
	default : 
		break;//do nothing
	}

	CWnd::OnLButtonDblClk(nFlags, point);
}

void CCurveWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(GetCapture() != this)
		StopTracking();
	
	if(m_bTracking)
		TrackKnot(point);
	
	CWnd::OnMouseMove(nFlags, point);
}

void CCurveWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_bTracking)
		StopTracking();
			
	CWnd::OnLButtonUp(nFlags, point);
}

void CCurveWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{
	
	CWnd::OnRButtonUp(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////
// CCurveWnd Functions												       //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//Active Knot Acces Functions

void CCurveWnd::SetActiveKnot(UINT nIndex)
{	
	//Deactivate Old Knot
	if(m_nActiveKnot != - 1) {

		nIndex <= m_nActiveKnot ? 
			m_nActiveKnot += 1 : m_nActiveKnot = m_nActiveKnot;

		CKnot* pKnotOld =
			m_pCurve->GetKnot(m_nActiveKnot);
		pKnotOld->SetData(TRUE);
	}

	//Activate New Knot
	if(nIndex != -1) {
		
		CKnot* pKnotNew = 
			m_pCurve->GetKnot(nIndex);
		pKnotNew->SetData(FALSE);
	}
	
	m_nActiveKnot = nIndex;
	RedrawWindow();
}

UINT CCurveWnd::GetActiveKnot() { return m_nActiveKnot;}

/////////////////////////////////////////////////////////////////////////////
//Curve Object Functions

void CCurveWnd::SetCurveObject(CCurveObject* pObject, BOOL bRedraw)
{
	if(pObject != m_pCurve) 
	{
		m_nActiveKnot = -1; //Reset active knot
		m_pCurve = pObject; //Set Curve Object pointer
	}

	if(bRedraw)
		RedrawWindow();
}

CCurveObject* CCurveWnd::GetCurveObject(){ return m_pCurve;}

/////////////////////////////////////////////////////////////////////////////
//Cursor Message Handlers

BOOL CCurveWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	BOOL b = FALSE;

	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	switch(HitTest(point)) 
	{
	case HTCURVE :
		{
			HCURSOR hCursor;
			hCursor = AfxGetApp()->LoadCursor(IDC_ARRWHITE);
			SetCursor(hCursor);
			b = TRUE;;
		} break;
	case HTKNOT :
		{
			HCURSOR hCursor; 
			hCursor = AfxGetApp()->LoadCursor(IDC_ARRBLCK);
			SetCursor(hCursor);
			b = TRUE;
		} break;
	default : //do nothing
		break;
	}
	
	if(!b)
		return CWnd::OnSetCursor(pWnd, nHitTest, message);
	else return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//Keyboard Message Handlers

void CCurveWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	BOOL bHandeldMsg = false;
	
	if(m_nActiveKnot != -1)
	{		
		switch(nChar)
		{
			case VK_DELETE :
			{
				m_pCurve->RemoveKnot(m_nActiveKnot);
				m_nActiveKnot = -1;	
				bHandeldMsg = true;
			} break;
			case VK_UP	:
			{
				CKnot* pKnot = m_pCurve->GetKnot(m_nActiveKnot);
				
				CPoint pt;
				pKnot->GetPoint(&pt);
				pt.y -= 1;

				m_pCurve->MoveKnot(pt, m_nActiveKnot);
				bHandeldMsg = true;
			} break;
			case VK_DOWN :
			{		
				CKnot* pKnot = m_pCurve->GetKnot(m_nActiveKnot);
				
				CPoint pt;
				pKnot->GetPoint(&pt);
				pt.y += 1;

				m_pCurve->MoveKnot(pt, m_nActiveKnot);
				bHandeldMsg = true;
			} break;
			case VK_LEFT :
			{
				CKnot* pKnot = m_pCurve->GetKnot(m_nActiveKnot);
				
				CPoint pt;
				pKnot->GetPoint(&pt);
				pt.x -= 1;

				m_pCurve->MoveKnot(pt, m_nActiveKnot);
				bHandeldMsg = true;
			} break;
			case VK_RIGHT :
			{
				CKnot* pKnot = m_pCurve->GetKnot(m_nActiveKnot);
				
				CPoint pt;
				pKnot->GetPoint(&pt);
				pt.x += 1;

				m_pCurve->MoveKnot(pt, m_nActiveKnot);
				bHandeldMsg = true;
			} break;

		default :
			break; //do nothing
		}

		RedrawWindow();
	}
	
	if(!bHandeldMsg)
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

/////////////////////////////////////////////////////////////////////////////
// CCurveWnd functions										               //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CCurveWnd implementation										       //
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//Hit Testing

WORD CCurveWnd::HitTest(CPoint point)
{		

	RECT rc;
	GetClientRect(&rc);

	point.x = (float) point.x / rc.right * m_pCurve->m_rcClipRect.right;
	point.y = (float) point.y / rc.bottom * m_pCurve->m_rcClipRect.bottom;

	if(AfxIsValidAddress(m_pCurve, sizeof(CObject))) 
	{
		m_pCurve->HitTest(point, m_pHitInfo);
	}
	else 
		m_pHitInfo->wHitCode = HTCANVAS;

	return m_pHitInfo->wHitCode;
}

///////////////////////////////////////////////////////////////////////////////
//Tracking support

void CCurveWnd::StartTracking()
{
	m_bTracking = TRUE;
	SetCapture();

	HCURSOR hCursor;
	hCursor = AfxGetApp()->LoadCursor(IDC_ARRBLCKCROSS);
	SetCursor(hCursor);

}

void CCurveWnd::TrackKnot(CPoint point)
{
	RECT rc;
	GetClientRect(&rc);

	point.x = (float) point.x / rc.right * m_pCurve->m_rcClipRect.right;
	point.y = (float) point.y / rc.bottom * m_pCurve->m_rcClipRect.bottom;
	
	int iKnot = m_pHitInfo->nKnotIndex;	//Index knot	
	m_pCurve->MoveKnot(point, iKnot);
		
	RedrawWindow();
}

void CCurveWnd::StopTracking()
{
	if(!m_bTracking)
		return;

	m_bTracking = FALSE;
	ReleaseCapture();

}

///////////////////////////////////////////////////////////////////////////////////
//Implementation