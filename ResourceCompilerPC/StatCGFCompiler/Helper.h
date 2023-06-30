////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   helper.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPER_H__3A967DC3_988F_4A67_BC33_AC26573B86FA__INCLUDED_)
#define AFX_HELPER_H__3A967DC3_988F_4A67_BC33_AC26573B86FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseObj.h"

class CHelper : public CBaseObj  
{
public:
	HELPER_CHUNK_DESC m_Chunk;

	CHelper();
	virtual ~CHelper();

	virtual bool Load(CXFile *f, int pos);	
};

#endif // !defined(AFX_HELPER_H__3A967DC3_988F_4A67_BC33_AC26573B86FA__INCLUDED_)
