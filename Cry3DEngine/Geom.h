////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   geom.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GEOM_H__D90B8CB0_DAA4_49C7_8D72_4A73FA38E640__INCLUDED_)
#define AFX_GEOM_H__D90B8CB0_DAA4_49C7_8D72_4A73FA38E640__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "baseobj.h"

class CGeom  : public CBaseObj
{
public:
	CryVertex	  *m_pVertices;
	CryFace		  *m_pFaces;
	CryUV		    *m_pUVs;
	CryTexFace	*m_pTexFaces;
	CryIRGB		  *m_pVcols;
//	int			    *m_pNumLinks;
//	CryLink		** m_pLinks;

	char * m_sPropstr;
	MESH_CHUNK_DESC	m_Chunk;

	CGeom();
	virtual ~CGeom();
	
	//from BaseObj
	bool Load(CXFile *f, int pos);

  MESH_CHUNK_DESC	* GetChunk() { return &m_Chunk; }

	//CGeom methods
	void Deform();
};

#endif // !defined(AFX_GEOM_H__D90B8CB0_DAA4_49C7_8D72_4A73FA38E640__INCLUDED_)
