////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   NamedData.cpp
//  Version:     v1.00
//  Created:     30/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Collection of Named data blocks.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "NamedData.h"
#include "..\zlib\zlib.h"
#include "PakFile.h"

IMPLEMENT_SERIAL(CNamedData, CObject, 1)

//////////////////////////////////////////////////////////////////////////
CNamedData::CNamedData()
{
}

//////////////////////////////////////////////////////////////////////////
CNamedData::~CNamedData()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////////
void CNamedData::AddDataBlock( const CString &blockName,void*	pData,int nSize,bool bCompress )
{
	assert( pData );
	assert( nSize > 0 );

	bCompress = false;

	DataBlock *pBlock = stl::find_in_map( m_blocks,blockName,(DataBlock*)0 );
	if (pBlock)
	{
		delete pBlock;
	}

	pBlock = new DataBlock;

	if (bCompress)
	{
		pBlock->bCompressed = true;
		CMemoryBlock temp;
		temp.Attach( pData,nSize );
		temp.Compress( pBlock->compressedData );
	}
	else
	{
		pBlock->bCompressed = false;
		pBlock->data.Allocate( nSize );
		pBlock->data.Copy( pData,nSize );
	}
	m_blocks[blockName] = pBlock;
}

void CNamedData::AddDataBlock( const CString &blockName,CMemoryBlock &mem )
{
	DataBlock *pBlock = stl::find_in_map( m_blocks,blockName,(DataBlock*)0 );
	if (pBlock)
	{
		delete pBlock;
	}
	pBlock = new DataBlock;
	if (mem.GetUncompressedSize() != 0)
	{
		// This is compressed block.
		pBlock->bCompressed = true;
		pBlock->compressedData = mem;
	}
	else
	{
		pBlock->bCompressed = false;
		pBlock->data = mem;
	}
	m_blocks[blockName] = pBlock;
}

//////////////////////////////////////////////////////////////////////////
void CNamedData::Clear()
{
	for (Blocks::iterator it = m_blocks.begin(); it != m_blocks.end(); ++it)
	{
		delete it->second;
	}
	m_blocks.clear();
}

//////////////////////////////////////////////////////////////////////////
bool CNamedData::GetDataBlock( const CString &blockName,void*	&pData, int &nSize )
{
	pData = 0;
	nSize = 0;

	bool bUncompressed = false;
	CMemoryBlock *mem = GetDataBlock( blockName,bUncompressed );
	if (mem)
	{
		pData = mem->GetBuffer();
		nSize = mem->GetSize();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
CMemoryBlock* CNamedData::GetDataBlock( const CString &blockName,bool &bCompressed )
{
	DataBlock *pBlock = stl::find_in_map( m_blocks,blockName,(DataBlock*)0 );
	if (!pBlock)
		return 0;

	if (bCompressed)
	{
		// Return compressed data.
		if (!pBlock->compressedData.IsEmpty())
			return &pBlock->compressedData;
	}
	else
	{
		// Return normal data.
		if (!pBlock->data.IsEmpty())
		{
			return &pBlock->data;
		}
		else
		{
			// Uncompress compressed block.
			if (!pBlock->compressedData.IsEmpty())
			{
				pBlock->compressedData.Uncompress( pBlock->data );
				return &pBlock->data;
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CNamedData::Serialize(CArchive& ar) 
{
	CObject::Serialize(ar);

	if (ar.IsStoring())
	{	
		int iSize = m_blocks.size();
		ar << iSize;

		for (Blocks::iterator it = m_blocks.begin(); it != m_blocks.end(); it++)
		{
			CString key = it->first;
			DataBlock* pBlock = it->second;

			unsigned int nOriginalSize;
			unsigned int nSizeFlags;
			unsigned int flags = 0;

			if (pBlock->bCompressed)
			{
				nOriginalSize = pBlock->compressedData.GetUncompressedSize();
				// Compressed data.
				unsigned long destSize = pBlock->compressedData.GetSize();
				void *dest = pBlock->compressedData.GetBuffer();
				nSizeFlags = destSize | (1<<31);

				ar << key;
				ar << nSizeFlags; // Current size of data + 1 bit for compressed flag.
				ar << nOriginalSize;	// Size of uncompressed data.
				ar << flags;	// Some additional flags.
				ar.Write( dest,destSize );
			}
			else
			{
				nOriginalSize = pBlock->data.GetSize();
				void *dest = pBlock->data.GetBuffer();

        nSizeFlags = nOriginalSize;
				ar << key;
				ar << nSizeFlags;
				ar << nOriginalSize;	// Size of uncompressed data.
				ar << flags;	// Some additional flags.
				ar.Write( dest,nOriginalSize );
			}
		}
	}
	else
	{	
		Clear();

		int iSize;
		ar >> iSize;

		for(int i = 0; i < iSize; i++)
		{	
			CString key;
			unsigned int nSizeFlags = 0;
			unsigned int nSize = 0;
			unsigned int nOriginalSize = 0;
			unsigned int flags = 0;
			bool bCompressed = false;

			DataBlock *pBlock = new DataBlock;
			
			ar >> key;
			ar >> nSizeFlags;
			ar >> nOriginalSize;
			ar >> flags;

			nSize = nSizeFlags & (~(1<<31));
			bCompressed = (nSizeFlags & (1<<31)) != 0;

			if (bCompressed)
			{
				pBlock->compressedData.Allocate( nSize,nOriginalSize );
				void *pSrcData = pBlock->compressedData.GetBuffer();
				// Read compressed data.
				ar.Read( pSrcData,nSize );
			}
			else
			{
				pBlock->data.Allocate( nSize  );
				void *pSrcData = pBlock->data.GetBuffer();

				// Read uncompressed data.
				ar.Read( pSrcData,nSize );
			}
			m_blocks[key] = pBlock;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CNamedData::Save( CPakFile &pakFile )
{
	for (Blocks::iterator it = m_blocks.begin(); it != m_blocks.end(); it++)
	{
		CString key = it->first;
		DataBlock* pBlock = it->second;
		if (!pBlock->bCompressed)
		{
			CString filename = key + ".editor_data";
			pakFile.UpdateFile( filename,pBlock->data );
		}
		else
		{
			int nOriginalSize = pBlock->compressedData.GetUncompressedSize();
			CMemFile memFile;
			// Write uncompressed data size.
			memFile.Write( &nOriginalSize,sizeof(nOriginalSize) );
			// Write compressed data.
			memFile.Write( pBlock->compressedData.GetBuffer(),pBlock->compressedData.GetSize() );
			pakFile.UpdateFile( key+".editor_datac",memFile,false );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CNamedData::Load( const CString &levelPath,CPakFile &pakFile )
{
	int i;
	CFileUtil::FileArray files;
	CFileUtil::ScanDirectory( levelPath,"*.editor_data",files,false );
	for (i = 0; i < files.size(); i++)
	{
		CString filename = files[i].filename;
		CCryFile cfile;
		if (cfile.Open( Path::Make(levelPath,filename),"rb"))
		{
			int fileSize = cfile.GetLength();
			if (fileSize > 0)
			{
				CString key = Path::GetFileName(filename);
				// Read data block.
				DataBlock *pBlock = new DataBlock;
				pBlock->data.Allocate( fileSize );
				cfile.Read( pBlock->data.GetBuffer(),fileSize );
				m_blocks[key] = pBlock;
			}
		}
	}
	files.clear();
	// Scan compressed data.
	CFileUtil::ScanDirectory( levelPath,"*.editor_datac",files,false );
	for (i = 0; i < files.size(); i++)
	{
		CString filename = files[i].filename;
		CCryFile cfile;
		if (cfile.Open( Path::Make(levelPath,filename),"rb"))
		{
			int fileSize = cfile.GetLength();
			if (fileSize > 0)
			{
				// Read uncompressed data size.
				int nOriginalSize = 0;
				cfile.Read( &nOriginalSize,sizeof(nOriginalSize) );
				// Read uncompressed data.
				int nDataSize = fileSize - sizeof(nOriginalSize);

				CString key = Path::GetFileName(filename);
				// Read data block.
				DataBlock *pBlock = new DataBlock;
				pBlock->compressedData.Allocate( nDataSize,nOriginalSize );
				cfile.Read( pBlock->compressedData.GetBuffer(),nDataSize );
				m_blocks[key] = pBlock;
			}
		}
	}
}
