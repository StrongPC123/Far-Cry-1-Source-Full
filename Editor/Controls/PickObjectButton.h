////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   pickobjectbutton.h
//  Version:     v1.00
//  Created:     28/2/2002 by Timur.
//  Compilers:   Visual C++.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __pickobjectbutton_h__
#define __pickobjectbutton_h__
#pragma once

#include "ColorCheckBox.h"

/////////////////////////////////////////////////////////////////////////////
// CPickObjectButton window

class CPickObjectButton : public CColorCheckBox, public IPickObjectCallback
{
DECLARE_DYNAMIC(CPickObjectButton)
// Construction
public:
	CPickObjectButton();

	void SetPickCallback( IPickObjectCallback *callback,const CString &statusText,CRuntimeClass *targetClass=0,bool bMultiPick=false );
	afx_msg void OnClicked();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPickObjectButton)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPickObjectButton();

	//! Called when object picked.
	virtual void OnPick( CBaseObject *picked );
	//! Called when pick mode cancelled.
	virtual void OnCancelPick();
	virtual bool OnPickFilter( CBaseObject *filterObject );

	// Generated message map functions
protected:
	//{{AFX_MSG(CPickObjectButton)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	IPickObjectCallback *m_pickCallback;
	CString m_statusText;
	CRuntimeClass *m_targetClass;
	bool m_bMultipick;
};

#endif // __pickobjectbutton_h__