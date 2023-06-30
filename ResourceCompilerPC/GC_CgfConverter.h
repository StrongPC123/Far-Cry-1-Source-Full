////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cgfconvertor.h
//  Version:     v1.00
//  Created:     5/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __GC_cgfconvertor_h__
#define __GC_cgfconvertor_h__
#pragma once

#include "IRCLog.h"
#include "IConvertor.h"
#include "CryCompiledFile.h"
#include "CryChunkedFile.h"
#include "RenderMeshBuilder.h"

struct ConvertContext;










// This is utility that can be used to easily write CCF format files
// just open the file, create this object on stack and add the chunk with
// AddChunk(nType)
// Then write the chunk directly to the file, and then add new chunk,
// repeat the process..
// Destroy (or call CloseChunk()) this object before you close the file.
// To have nested chunks, create another instance of CCFWriter and use it for some time.
// When you destruct it, you can use the upper-level writer. And so on.
class GC_CCFFileWriter
{
public:
	GC_CCFFileWriter (FILE* f):
		m_pFile (f), m_nLastChunk (-1)
	{
	}

	GC_CCFFileWriter ():
		m_pFile(NULL), m_nLastChunk(-1)
	{
	}

	~GC_CCFFileWriter()
	{
		CloseChunk();
	}

	// changes the file handle
	void SetFile (FILE* f)
	{
		CloseChunk();
		m_pFile = f;
	}

	// adds a new chunk of the specified type
	// returns 1 if successfully wrote the chunk header or 0 if an error occurred
	int AddChunk(CCFChunkTypeEnum nType)
	{
		// first, end the previous chunk
		CloseChunk();
		// remember the current position
		m_nLastChunk = ftell (m_pFile);

		// write the chunk header
		CCFChunkHeader header;
		header.nSize = 0x58585858;
		header.nType = SWAP32(nType); //0x204f5649;
		return fwrite (&header, sizeof(header), 1, m_pFile);
	}

	// Signals the end of the chunk that was added with AddChunk
	// Automatically aligns the chunk data by 4-byte boundary, if needed
	void CloseChunk()
	{
		if (m_nLastChunk < 0)
			return; // no last chunk, or the last chunk was closed

		long nNewChunkPos = ftell (m_pFile);
		if (nNewChunkPos&3)
		{
			// align by 4-byte boundary
			int nPad = 0;
			fwrite (&nPad, 1, 4-(nNewChunkPos&3), m_pFile);
			nNewChunkPos = ftell (m_pFile);
		}

		// write the size of the chunk to the chunk header
		fseek (m_pFile, (int)(&((CCFChunkHeader*)m_nLastChunk)->nSize), SEEK_SET);
		unsigned nSize = SWAP32(nNewChunkPos - m_nLastChunk);
		fwrite (&nSize, sizeof(nSize), 1, m_pFile);
		// set the file pointer back where it was
		fseek (m_pFile, nNewChunkPos, SEEK_SET);

		// forget about the last chunk
		m_nLastChunk = -1;
	}

protected:
	// the file to which  the chunks are written
	FILE* m_pFile;
	// the last chunk position within the file (used to update the chunk)
	long m_nLastChunk;
};

















/** Convertor implementation for CGF files.
*/
class GC_CGFConvertor : public IConvertor
{
public:
	// this class will be thrown from the internal method during conversion
	// indicating some error that should abort conversion
	class Error
	{
	public:
		Error (int nCode);
		Error (const char* szFormat, ...);
		const char* c_str()const {return m_strReason.c_str();}
	protected:
		string m_strReason;
	};

	GC_CGFConvertor ():
	m_fTarget(NULL),
		m_pStatCGFCompiler(NULL)
	{
	}

	~GC_CGFConvertor()
	{
		clear();
		if (m_pStatCGFCompiler)
			m_pStatCGFCompiler->Release();
	}

	//! Release memory of interface.
	void Release() { delete this; };

	//! Process file.
	virtual bool Process( ConvertContext &cc );

	//! Return name of output file that will be produced from this file.
	// @param sourceFileName File name plus extension of source file, must not contain path.
	virtual bool GetOutputFile( ConvertContext &cc );

	//! Return platforms supported by this convertor.
	virtual int GetNumPlatforms() const;
	//! Get supported platform.
	//! @param index Index of platform must be in range 0 < index < GetNumPlatforms().
	virtual Platform GetPlatform( int index ) const;

	//! Get number of supported extensions.
	virtual int GetNumExt() const { return 1; };
	//! Get supported extension.
	//! @param index Index of extension must be in range 0 < index < GetNumExt().
	virtual const char* GetExt( int index ) const { return "cgf"; };

	// this should retrieve the timestamp of the convertor executable:
	// when it was created by the linker, normally. This date/time is used to
	// compare with the compiled file date/time and even if the compiled file
	// is not older than the source file, it will be recompiled if it's older than the
	// convertor
	virtual DWORD GetTimestamp() const ;
protected:
	// internally used stuff
	// writes the data directly into the file
	void writeRaw (const void* pData, unsigned nSize);

	template <class T>
		void write (const T& rData)
	{
		writeRaw (&rData, sizeof(T));
	}

	template <class T>
		void write (const T* pData, unsigned numElements)
	{
		writeRaw (pData, numElements * sizeof(T));
	}

	template <class T>
		void writeArray (const std::vector<T>& arrData)
	{
		if (!arrData.empty()){
			unsigned long a0=arrData.size();
			write (&arrData[0], arrData.size());
		
		}
	}

	//void LogV (const char* szFormat, va_list args);
	//void Log (const char* szFormat, ...);

	void WriteGeometryInfo (unsigned nLOD);
	void WriteHeader();
	void WriteBoneInfo();
	void WriteMaterials();
	void writeIntFaces(unsigned nLOD);
	void WriteVertices (CryChunkedFile* pSource);
	void WriteNormals (CryChunkedFile* pSource);

	void writeShadowConnectivity (unsigned nLOD);
	void writeVertexSkin (unsigned nLOD);
	void writeNormalSkin (unsigned nLOD);
	void writeTangSkin (unsigned nLOD);

	void WriteBoneGeometry (unsigned nLOD);

	void writeBGBone(unsigned nLOD, unsigned nBone, CryChunkedFile::MeshDesc* pMesh);
	void WriteMorphTargetSet ();
	void writeMorphTargets (unsigned nLOD);
	void WriteLights();
	bool isAnimationFastCheck();
protected:
	// loads the m_arrLODs
	bool LoadLODs();
	IConvertor* getStatCGFCompiler();

	// builds the m_arrRenderMeshes array with the CRenderMeshBuilder
	// structures containing all the additional info required for leaf buffer creation
	void BuildRenderMeshes();

	// Updates physics if needed (if there is LOD1, which contains physical data for LOD1 physics
	void UpdateDeadBodyPhysics();


	// remaps, if necessary, the bones from the LOD source to the Master source
	// and changes the links in the LOD source so that the boneid's there point to the indices (not IDs!)
	// of the bones in the Master Source, not LOD Source
	// This assumes that the bone information from the LOD source won't be used at all
	// throws an error if there's some unrecognized bone in the LOD source
	void RemapBones (unsigned nLOD);

	// remaps the bone indices, throws error if some indices cannot be remapped for some reason
	// (e.g. mapping contains -1, i.e. no mapping)
	void RemapBoneIndices (CryChunkedFile* pLOD, const std::vector<int>& arrMap);

	// releases all the resources
	void clear();

	// calculates the number of materials used by the previous LODs (< than given)
	// this is to offset the mtl numbers of the nLOD to use the range of materials
	// belonging to that LOD
	unsigned getLODMtlOffset (unsigned nLOD);

	void LogWarning (const char* szFormat, ...)
	{
		va_list args;
		va_start (args, szFormat);
		m_pContext->pLog->LogV (IMiniLog::eWarning, szFormat, args);
		va_end(args);
	}

	void LogError (const char* szFormat, ...)
	{
		va_list args;
		va_start (args, szFormat);
		m_pContext->pLog->LogV (IMiniLog::eError, szFormat, args);
		va_end(args);
	}

	void Log (const char* szFormat, ...)
	{
		va_list args;
		va_start (args, szFormat);
		m_pContext->pLog->LogV (IMiniLog::eMessage, szFormat, args);
		va_end(args);
	}
protected:
	ConvertContext* m_pContext;

	// the source files: one file for each LOD. At least LOD 0 Must be present for conversion.
	CryChunkedFile_AutoArray m_arrLODs;

	// the mapping from bone indices in Master (LOD0) to bone indices in LOD>0
	typedef std::vector<int> BoneMap;
	std::vector<BoneMap> m_arrLODBoneMaps;

	// the render meshes for each LOD
	std::vector<CRenderMeshBuilder> m_arrRenderMeshes;

	GC_CCFFileWriter m_Writer;

	// the target file
	FILE* m_fTarget;


	IConvertor* m_pStatCGFCompiler;
};


#endif // __cgfconvertor_h__
