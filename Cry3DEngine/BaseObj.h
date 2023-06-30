////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   baseobj.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEOBJ_H__2D817DB5_34FA_4C73_8588_867E88C9CFB3__INCLUDED_)
#define AFX_BASEOBJ_H__2D817DB5_34FA_4C73_8588_867E88C9CFB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBaseObj  
{
public:
	CHUNK_HEADER	m_ChunkHeader;
	bool m_bBinded;
  int m_nUsers;
  
  CBaseObj()
  {
	  memset(&m_ChunkHeader,0,sizeof(m_ChunkHeader));
	  m_bBinded = false;
    m_nUsers = 0;
  }

  virtual ~CBaseObj(){};
  virtual bool Load(CXFile *f, int pos) { return false; }
  virtual void Bind(CBaseObj **all_objects, int n_obj){}
};

#endif // !defined(AFX_BASEOBJ_H__2D817DB5_34FA_4C73_8588_867E88C9CFB3__INCLUDED_)
