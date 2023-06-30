////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   NamedData.h
//  Version:     v1.00
//  Created:     30/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Collection of Named data blocks.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __NamedData_h__
#define __NamedData_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "MemoryBlock.h"

class CPakFile;

class CNamedData : public CObject
{
DECLARE_SERIAL( CNamedData )
public:
	CNamedData();
	virtual ~CNamedData();
	void AddDataBlock( const CString &blockName,void*	pData,int nSize,bool bCompress=true );
	void AddDataBlock( const CString &blockName,CMemoryBlock &block );
	//! Returns uncompressed block data.
	bool GetDataBlock( const CString &blockName,void*	&pData, int &nSize );
	//! Returns raw data block in original form (Compressed or Uncompressed).
	CMemoryBlock* GetDataBlock( const CString &blockName,bool &bCompressed );

	void Clear();
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCurveObject)
	public:
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

	//! Save named data to pak file.
	void Save( CPakFile &pakFile );
	//! Load named data from pak file.
	void Load( const CString &levelPath,CPakFile &pakFile );

private:
	struct DataBlock
	{
		CString blockName;
		CMemoryBlock data;
		CMemoryBlock compressedData;
		//! This block is compressed.
		bool bCompressed;
	};
	typedef std::map<CString,DataBlock*,stl::less_stricmp<CString> > Blocks;
	Blocks m_blocks;
};

#endif // __NamedData_h__
