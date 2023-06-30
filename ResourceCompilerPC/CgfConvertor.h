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

#ifndef __cgfconvertor_h__
#define __cgfconvertor_h__
#pragma once

#include "IRCLog.h"
#include "IConvertor.h"
#include "CryCompiledFile.h"
#include "CryChunkedFile.h"
#include "RenderMeshBuilder.h"

struct ConvertContext;


/** Convertor implementation for CGF files.
*/
class CGFConvertor : public IConvertor
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

	CGFConvertor ();
	~CGFConvertor();

	//! Release memory of interface.
	void Release() { delete this; };

	//! Process file.
	virtual bool Process( ConvertContext &cc );

	bool ProcessStatic( ConvertContext &cc );

	//! Return name of output file that will be produced from this file.
	// @param sourceFileName File name plus extension of source file, must not contain path.
	virtual bool GetOutputFile( ConvertContext &cc );

	//! Return platforms supported by this convertor.
	virtual int GetNumPlatforms() const;
	//! Get supported platform.
	//! @param index Index of platform must be in range 0 < index < GetNumPlatforms().
	virtual Platform GetPlatform( int index ) const;

	//! Get number of supported extensions.
	virtual int GetNumExt() const { return 1+m_pStatCGFCompiler->GetNumExt(); };
	//! Get supported extension.
	//! @param index Index of extension must be in range 0 < index < GetNumExt().
	virtual const char* GetExt( int index ) const { return index?"cgf":m_pStatCGFCompiler->GetExt(index-1); };

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
	void write (const T* pData, size_t numElements)
	{
		writeRaw (pData, numElements * sizeof(T));
	}

	template <class T>
	void writeArray (const std::vector<T>& arrData)
	{
		if (!arrData.empty())
			write (&arrData[0], arrData.size());
	}

	// writes the 0-terminated string, returns the number of bytes (including the terminating 0) written
	size_t writeString (const char* szString)
	{
		size_t n = strlen(szString)+1;
		writeRaw (szString, n);
		return n;
	}

	// writes the 0-terminated string, returns the number of bytes (including the terminating 0) written
	size_t writeString (const string& strString)
	{
		size_t n = strString.length() + 1;
		writeRaw (strString.c_str(), n);
		return n;
	}

	//void LogV (const char* szFormat, va_list args);
	//void Log (const char* szFormat, ...);

	void WriteGeometryInfo (unsigned nLOD);
	void WriteHeader();
	void WriteBoneInfo();
	void WriteMaterials( const char *inszSrcFileName );
	void writeIntFaces(unsigned nLOD);
	void WriteVertices (CryChunkedFile* pSource);
	void WriteNormals (CryChunkedFile* pSource);

	void writeShadowConnectivity (unsigned nLOD);
	void writeVertexSkin (unsigned nLOD);
	void writeNormalSkin (unsigned nLOD);
	void writeTangSkin (unsigned nLOD);
	void validateTangents (const TangData* pTangents, unsigned numTangents);

	void WriteBoneGeometry (unsigned nLOD);

	void writeBGBone(unsigned nLOD, unsigned nBone, CryChunkedFile::MeshDesc* pMesh);
	void WriteMorphTargetSet ();
	void writeMorphTargets (unsigned nLOD);
	void WriteLights();
	bool isAnimatedFastCheck( const char *filename );

	string GetCalFilePath ();
	string GetSourceDir();

	// loads the given animation info into the given structure
	// returns false upon failure
	bool LoadAnimInfo (const string& strFilePath, CCFAnimInfo &animInfo);
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

	// !!!OBSOLETE!!!
	// calculates the number of materials used by the previous LODs (< than given)
	// this is to offset the mtl numbers of the nLOD to use the range of materials
	// belonging to that LOD
	// !!!OBSOLETE!!!
	unsigned getLODMtlOffset (unsigned nLOD);

	// Reads the CAL file or the list of caf files in the same directory with the corresponding name,
	// and writes the animation list chunk with the animation names, file paths and compiled directives
	void WriteAnimListWithCAL (FILE* fCal);
	void WriteAnimListNoCAL ();
	void WriteAnimInfo (CCFFileWriter &rSubChunks, const char* szAnimName, const string& strFilePath, unsigned nFlag);

	void WriteSceneProps();

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

	typedef std::map<CString,int> PhysMatIDMap;
	PhysMatIDMap m_physMtlToFaceIdMap; // Map for every LOD (except first).

	// the render meshes for each LOD
	std::vector<CRenderMeshBuilder> m_arrRenderMeshes;

	CCFFileWriter m_Writer;

	// the target file
	FILE* m_fTarget;


	IConvertor* m_pStatCGFCompiler;
};

#endif // __cgfconvertor_h__
