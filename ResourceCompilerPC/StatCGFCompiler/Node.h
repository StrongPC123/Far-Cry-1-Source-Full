////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   node.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_NODE_H__8DFD8741_DBA1_4357_9F50_8E37EA039BCB__INCLUDED_)
#define AFX_NODE_H__8DFD8741_DBA1_4357_9F50_8E37EA039BCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseObj.h"

class Controller;

class CNodeCGF : public CBaseObj
{
public:
	NODE_CHUNK_DESC m_Chunk;
//	int				  *m_pnChildrenIds;

	CBaseObj		*m_pObj;
/*	char		    *m_sPropStr;
	CNodeCGF		** m_ppChildren;*/
	CNodeCGF		*m_pParent;
	/*CBaseObj		*m_pMtl;
	Controller	*m_pConPos,*m_pConRot,*m_pConScl;*/
//	CryMatrix	   m_NodeMatrix;

	CNodeCGF();
	virtual ~CNodeCGF();

	virtual bool Load(CXFile *f, int pos);			  
	virtual void Bind(CBaseObj **all_objects, int n_obj);

  char * GetName() { return m_Chunk.name; }
  float * GetMatrixData() { return m_Chunk.tm.GetData(); }
};

#endif // !defined(AFX_NODE_H__8DFD8741_DBA1_4357_9F50_8E37EA039BCB__INCLUDED_)
