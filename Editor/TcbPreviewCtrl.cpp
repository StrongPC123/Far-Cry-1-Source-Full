////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tcbpreviewctrl.cpp
//  Version:     v1.00
//  Created:     30/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TcbPreviewCtrl.h"

#include <IMovieSystem.h>


// CTcbPreviewCtrl

IMPLEMENT_DYNAMIC(CTcbPreviewCtrl, CWnd)
CTcbPreviewCtrl::CTcbPreviewCtrl()
{
	m_tens = 0;
	m_cont = 0;
	m_bias = 0;
	m_easefrom = 0;
	m_easeto = 0;

	m_spline = GetIEditor()->GetMovieSystem()->CreateTrack( ATRACK_TCB_VECTOR );
	m_spline->SetNumKeys(3);
}

CTcbPreviewCtrl::~CTcbPreviewCtrl()
{
}


BEGIN_MESSAGE_MAP(CTcbPreviewCtrl, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()


void CTcbPreviewCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages
	dc.FillSolidRect( dc.m_ps.rcPaint.left,dc.m_ps.rcPaint.top,
										dc.m_ps.rcPaint.right,dc.m_ps.rcPaint.bottom,RGB(255,255,255) );
	GetClientRect( m_rcClient );
	DrawSpline( dc );
}

void CTcbPreviewCtrl::DrawSpline( CDC &dc )
{
	Vec3 v[3];

	CRect rc = m_rcClient;
	float xoffset = 0;
	
	int xbase = m_rcClient.left+2;
	int ybase = m_rcClient.bottom+2;

	float w = m_rcClient.Width() - 4;
	float h = m_rcClient.Height() - 8;

	v[0]( xbase,ybase,0 );
	v[1]( xbase+w/2,ybase-h,0 );
	v[2]( xbase+w,ybase,0 );

	// Initialize 3 keys.
	ITcbKey key[3];
	key[0].time = 0;
	key[0].SetValue( v[0] );
	
	key[1].time = 0.5f;
	key[1].SetValue( v[1] );
	key[1].tens = m_tens;
	key[1].bias = m_bias;
	key[1].cont = m_cont;
	key[1].easefrom = m_easefrom;
	key[1].easeto = m_easeto;
	
	key[2].time = 1.0f;
	key[2].SetValue( v[2] );
	
	// Set keys to spline.
	m_spline->SetKey( 0,&key[0] );
	m_spline->SetKey( 1,&key[1] );
	m_spline->SetKey( 2,&key[2] );

	CPen grayPen( PS_SOLID,1,RGB(150,150,150) );
	CPen bluePen( PS_SOLID,1,RGB(0,0,255) );
	CPen redPen( PS_SOLID,1,RGB(255,0,0) );


	float dt = 1.0f / m_rcClient.Width();
	float time = 0;

	// Draw spline.
	CPen *prevPen = dc.SelectObject( &grayPen );
	dc.MoveTo( xbase,ybase );
	for (time = 0; time < 1.0f; time += dt)
	{
		Vec3 vec;
		m_spline->GetValue( time,vec );

		int px = vec.x;
		int py = vec.y;
		if (px > m_rcClient.left && px < m_rcClient.right &&
				py > m_rcClient.top && py < m_rcClient.bottom)
		{
			dc.LineTo( px,py );
		}
	}

	dt = 4.0f / m_rcClient.Width();
	// Draw spline ticks.
	dc.SelectObject( &bluePen );
	for (time = 0; time < 1.0f; time += dt)
	{
		Vec3 vec;
		m_spline->GetValue( time,vec );

		int px = vec.x;
		int py = vec.y;
		if (px > m_rcClient.left && px < m_rcClient.right &&
				py > m_rcClient.top && py < m_rcClient.bottom)
		{
			dc.MoveTo( px,py-1 );
			dc.LineTo( px,py+2 );
		}
	}

	// Draw red cross at middle key.
	{
		dc.SelectObject( &redPen );
		Vec3 vec;
		m_spline->GetValue( 0.5f,vec );
		int px = vec.x;
		int py = vec.y;
		dc.MoveTo( px,py-2 );
		dc.LineTo( px,py+3 );
		dc.MoveTo( px-2,py );
		dc.LineTo( px+3,py );
	}

	dc.SelectObject( prevPen );
}

void CTcbPreviewCtrl::SetTcb( float tens,float cont,float bias,float easeto,float easefrom )
{
	m_tens = tens;
	m_cont = cont;
	m_bias = bias;
	m_easefrom = easefrom;
	m_easeto = easeto;

	if (m_hWnd)
		Invalidate();
}