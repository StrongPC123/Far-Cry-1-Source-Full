////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewsplitter.cpp
//  Version:     v1.00
//  Created:     24/4/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: CTrackViewSplitter implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "TrackViewSplitter.h"


// CTrackViewSplitter

IMPLEMENT_DYNAMIC(CTrackViewSplitter, CSplitterWnd)
CTrackViewSplitter::CTrackViewSplitter()
{
	m_cxSplitter = m_cySplitter = 3 + 1 + 1;
	m_cxBorderShare = m_cyBorderShare = 0;
	m_cxSplitterGap = m_cySplitterGap = 3 + 1 + 1;
	m_cxBorder = m_cyBorder = 1;
}

CTrackViewSplitter::~CTrackViewSplitter()
{
}


BEGIN_MESSAGE_MAP(CTrackViewSplitter, CSplitterWnd)
END_MESSAGE_MAP()



// CTrackViewSplitter message handlers

void CTrackViewSplitter::SetPane( int row,int col,CWnd *pWnd,SIZE sizeInit )
{
	assert( pWnd != NULL );
	
	// set the initial size for that pane
	m_pColInfo[col].nIdealSize = sizeInit.cx;
	m_pRowInfo[row].nIdealSize = sizeInit.cy;

	pWnd->ModifyStyle( 0,WS_BORDER,WS_CHILD|WS_VISIBLE );
	pWnd->SetParent(this);
	
	CRect rect(CPoint(0,0), sizeInit);
	pWnd->MoveWindow( 0,0,sizeInit.cx,sizeInit.cy,FALSE );
	pWnd->SetDlgCtrlID( IdFromRowCol(row,col) );

	ASSERT((int)::GetDlgCtrlID(pWnd->m_hWnd) == IdFromRowCol(row, col));
}

void CTrackViewSplitter::OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rectArg)
{
	// Let CSplitterWnd handle everything but the border-drawing
	if((nType != splitBorder) || (pDC == NULL))
	{
		CSplitterWnd::OnDrawSplitter(pDC, nType, rectArg);
		return;
	}

	ASSERT_VALID(pDC);

	// Draw border
	pDC->Draw3dRect(rectArg, GetSysColor(COLOR_BTNSHADOW), GetSysColor(COLOR_BTNHIGHLIGHT));
}