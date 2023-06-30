////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewcontrolbar.h
//  Version:     v1.00
//  Created:     17/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __trackviewcontrolbar_h__
#define __trackviewcontrolbar_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "Controls\sizecbar.h"
#include "Controls\scbarg.h"
#include "TrackViewDialog.h"

// CTrackViewControlBar

class CTrackViewControlBar : public CSizingControlBarG
{
	DECLARE_DYNAMIC(CTrackViewControlBar)

public:
	CTrackViewControlBar();
	virtual ~CTrackViewControlBar();

	void Update();

	CTrackViewDialog* GetDialog() { return &m_dlgTrackView; }

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);

	CTrackViewDialog m_dlgTrackView;
public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


#endif // __trackviewcontrolbar_h__