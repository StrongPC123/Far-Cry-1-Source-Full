////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   chunkfile.cpp
//  Version:     v1.00
//  Created:     15/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ChunkFile.h"

#include "CryHeaders.h"

//////////////////////////////////////////////////////////////////////////
// CChunkFile implementation.
//////////////////////////////////////////////////////////////////////////
CChunkFile::CChunkFile()
{
	m_fileHeader.FileType				= FileType_Geom;
	m_fileHeader.ChunkTableOffset		= -1;
	m_fileHeader.Version				= GeomFileVersion;
		
	strcpy(m_fileHeader.Signature,FILE_SIGNATURE);
}

//////////////////////////////////////////////////////////////////////////
CChunkFile::~CChunkFile()
{
}

//////////////////////////////////////////////////////////////////////////
bool CChunkFile::Write( const char *filename )
{
	FILE *file = fopen( filename,"wb" );
	if (!file)
		return false;

	unsigned i;

	fseek( file,sizeof(m_fileHeader),SEEK_SET );

	for(i=0;i<m_chunks.size();i++)
	{
		int ofs = ftell(file);
		// write header.
		ChunkDesc &ch = m_chunks[i];
		ch.hdr.FileOffset = ofs;
		memcpy( ch.data->GetBuffer(),&ch.hdr,sizeof(ch.hdr) );
		// write data.
		fwrite( ch.data->GetBuffer(),ch.data->GetSize(),1,file );
	}

	int chunkTableOffset = ftell(file);

	//=======================
	//Write # of Chunks
	//=======================
	unsigned nch = m_chunks.size();
	if (fwrite(&nch,sizeof(nch),1,file) !=1)
		return false;

	//=======================
	//Write Chunk List
	//=======================
	for(i=0;i<nch;i++)
	{
		CHUNK_HEADER &hdr = m_chunks[i].hdr;
		if (fwrite(&hdr,sizeof(hdr),1,file)!=1)
			return false;
	}

	//update Header for chunk list offset
	m_fileHeader.ChunkTableOffset	= chunkTableOffset;
	fseek(file,0,SEEK_SET);
	if (fwrite(&m_fileHeader,sizeof(m_fileHeader),1,file) != 1)
		return false;

	fclose(file);

	return true;
}

//////////////////////////////////////////////////////////////////////////
CChunkFile::ChunkDesc* CChunkFile::FindChunkByType( int type )
{
	for(unsigned i=0; i<m_chunks.size();i++)
	{
		if (m_chunks[i].hdr.ChunkType == type) 
		{
			return &m_chunks[i];
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CChunkFile::ChunkDesc* CChunkFile::FindChunkById( int id )
{
	for(unsigned i=0; i<m_chunks.size();i++)
	{
		if (m_chunks[i].hdr.ChunkID == id) 
		{
			return &m_chunks[i];
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
int CChunkFile::AddChunk( const CHUNK_HEADER &hdr,void *chunkData,int chunkSize )
{
	ChunkDesc chunk;
	chunk.hdr = hdr;
	chunk.data = new CMemoryBlock;
	chunk.data->Allocate( chunkSize );
	chunk.data->Copy( chunkData,chunkSize );

	int chunkID = m_chunks.size() + 1;
	chunk.hdr.ChunkID = chunkID;
	m_chunks.push_back( chunk );
	return chunkID;
}

