// ToolBarContainer.cpp : implementation file
//

#include "stdafx.h"
#include "ToolBarContainer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolBarContainer

CToolBarContainer::CToolBarContainer()
{
}

CToolBarContainer::~CToolBarContainer()
{
}


BEGIN_MESSAGE_MAP(CToolBarContainer, CWnd)
	//{{AFX_MSG_MAP(CToolBarContainer)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CToolBarContainer message handlers

void CToolBarContainer::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect rect;
	CBrush cFillBrush;
	
	// Paint the window with a dark gray bursh

	// Get the rect of the client window
	GetClientRect(&rect);

	// Create the brush
	cFillBrush.CreateStockObject(BLACK_BRUSH);

	// Fill the entire client area
	dc.FillRect(&rect, &cFillBrush);
	
	// Do not call CView::OnPaint() for painting messages
}
