////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tcbpreviewctrl.h
//  Version:     v1.00
//  Created:     30/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __tcbpreviewctrl_h__
#define __tcbpreviewctrl_h__

#if _MSC_VER > 1000
#pragma once
#endif

struct IAnimTrack;

// CTcbPreviewCtrl

class CTcbPreviewCtrl : public CWnd
{
	DECLARE_DYNAMIC(CTcbPreviewCtrl)

public:
	CTcbPreviewCtrl();
	virtual ~CTcbPreviewCtrl();

	void SetTcb( float tens,float cont,float bias,float easeto,float easefrom );

protected:
	DECLARE_MESSAGE_MAP()


	afx_msg void OnPaint();
	
	void DrawSpline( CDC &dc );


	CRect m_rcClient;
	
	// Tcb params,
	float m_tens;
	float m_cont;
	float m_bias;
	float m_easeto;
	float m_easefrom;

	TSmartPtr<IAnimTrack> m_spline;
};


#endif // __tcbpreviewctrl_h__