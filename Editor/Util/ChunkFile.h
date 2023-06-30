////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   chunkfile.h
//  Version:     v1.00
//  Created:     15/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __chunkfile_h__
#define __chunkfile_h__
#pragma once

#include "CryHeaders.h"
#include "MemoryBlock.h"

/** Chunk files used to save .cgf models.
*/
class CChunkFile
{
public:
	struct ChunkDesc
	{
		CHUNK_HEADER hdr;
		TSmartPtr<CMemoryBlock> data;

		ChunkDesc() {}
		ChunkDesc( const ChunkDesc& d ) { *this = d; }
		ChunkDesc& operator=( const ChunkDesc &d )
		{
			hdr = d.hdr;
			data = d.data;
			return *this;
		}
	};

	CChunkFile();
	~CChunkFile();

	bool Write( const char *filename );

	//! Add chunk to file.
	//! @retun ChunkID of added chunk.
	int AddChunk( const CHUNK_HEADER &hdr,void *chunkData,int chunkSize );

private:
	ChunkDesc* FindChunkByType( int type );
	ChunkDesc* FindChunkById( int id );


	//////////////////////////////////////////////////////////////////////////
	// variables.
	//////////////////////////////////////////////////////////////////////////
	FILE_HEADER m_fileHeader;
	std::vector<ChunkDesc> m_chunks;
};

#endif // __chunkfile_h__
