////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   light.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_LIGHT_H__ACF97045_6471_4C13_BC9F_F26C16A0590E__INCLUDED_)
#define AFX_LIGHT_H__ACF97045_6471_4C13_BC9F_F26C16A0590E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseObj.h"

class CLight : public CBaseObj  
{
public:
	LIGHT_CHUNK_DESC m_Chunk;

	CLight();
	virtual ~CLight();

	virtual bool Load(CXFile *f, int pos);	
};

#endif // !defined(AFX_LIGHT_H__ACF97045_6471_4C13_BC9F_F26C16A0590E__INCLUDED_)
