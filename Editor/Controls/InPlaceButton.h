////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   inplacebutton.h
//  Version:     v1.00
//  Created:     6/6/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __inplacebutton_h__
#define __inplacebutton_h__

#if _MSC_VER > 1000
#pragma once
#endif

// CInPlaceButton
#include <XTToolkit.h>

class CInPlaceButton : public CXTButton
{
	DECLARE_DYNAMIC(CInPlaceButton)

public:
	typedef Functor0 OnClick;

	CInPlaceButton( OnClick onClickFunctor );
	virtual ~CInPlaceButton();

	// Simuale on click.
	void Click();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClicked();

	OnClick m_onClick;
};

#endif // __inplacebutton_h__