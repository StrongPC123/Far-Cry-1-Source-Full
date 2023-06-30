//////////////////////////////////////////////////////////////////////
//
//  Crytek (C) 2002
//
//  CryCommon Source Code
//
//  File:CryCompiledFile.h
//  Description: Cry Compiled File Format declarations: necessary structures,
//               chunk descriptions.
//
//	Notes on Cry Compiled File Format.
//  Generally, compiled format is used for faster load times in the game.
//  The source files exported from Max and otherwise created are compiled
//  into compiled files that are fast to read, but whose format can be changed
//  frequently and be inflexible.
//
//  The compiled format is essentially a bitstream structured with light-weight chunks.
//  THe compiled format file consists of several pieces (chunks) that can either contain some
//  binary data, or other chunks. Each chunk begins with CCFHeader structure.
//
//  If you need chunk identification (like ChunkId in the CGF/CAF), write another DWORD after the CCFChunkHeader
//
//  If you need chunk type version, write the version number as another DWORD after the CCFChunkHeader
//
//  If you need ANYTHING else, just write it after the CCFChunkHeader. DO NOT expand CCFChunkHeader.
//  If you need, use your own convention to write some other structure right after the CCFChunkHeader,
//  but everybody must be able to skip chunks using the nSize field in the CCFChunkHeader
//
//  The CCF format is for one-time stream-like reading, not for random access.
//  There's no point in having any chunk table or other dictionary structures.
//  
//  History:
//  - Jan 16, 2003 - Created by Sergiy Migdalskiy
//
//  Prerequisities:
//   #include <stdio.h>
//
//////////////////////////////////////////////////////////////////////
#ifndef __CRY_COMMON_CRY_COMPILED_FILE_FORMAT_HDR__
#define __CRY_COMMON_CRY_COMPILED_FILE_FORMAT_HDR__

// the required header for file i/o operations
#include <stdio.h>

// Cry Compiled File format chunk header
// Each chunk in the file begins with this structure
struct CCFChunkHeader
{
	// the chunk size, in bytes, including this header.
	// this must always be 4-byte aligned
	unsigned nSize;
	// the chunk type, must be one of the CCFChunkTypeEnum constants
	unsigned nType;
};

enum CCFChunkTypeEnum
{
	CCF_NOT_CHUNK = -1,  // this is used to signal absence of chunk

	CCF_EMPTY_CHUNK = 0,

	// the Compiled Cry Geometry header, contains one structure CCFCCGHeader
	CCF_HEADER_CCG,

	// information about geometry included into a compiled CGF file
	// this chunk is followed by the structure CCFAnimGeomInfo
	// and contains also subchunks:
	//  CCF_GI_PRIMITIVE_GROUPS
	//  CCF_GI_INDEX_BUFFER
	//  CCF_GI_EXT_TO_INT_MAP
	//  CCF_GI_EXT_UVS
	//  CCF_GI_EXT_TANGENTS
	//  CCF_GI_EXT_COLORS  {optionally}
	CCF_GEOMETRY_INFO,

	// This is a pure array of material groups in CCFMaterialGroup structures
	CCF_GI_PRIMITIVE_GROUPS,

	// this is purely an array of unsigned shorts representing the index buffer
	CCF_GI_INDEX_BUFFER,

	// external to internal indexation map, again, an array of unsigned shorts.
	CCF_GI_EXT_TO_INT_MAP,

	// an array of CryUV's - in external indexation, for immediate copying into the vertex buffer
	CCF_GI_EXT_UVS,

	// an array of tangent bases in TangData structures (external indexation)
	CCF_GI_EXT_TANGENTS,

	// an array of DWORDs representing colors - in internal indexation
	CCF_GI_INT_COLORS,

	// First, struct CCFBoneDescArray followed by
	// the linearized hierarchy of bones: each bone represented by serialized by CryBoneDesc data
	CCF_BONE_DESC_ARRAY,

	// this is an array of vertices, internal indexation; must be the exact size to contain 
	// number of vertices specified in CCF_GEOMETRY_INFO chunk
	CCF_VERTICES,

	// this is an array of normals, internal indexation; must be the exact size to contain
	// all the normals (specified in CCF_GEOMETRY_INFO chunk)
	CCF_NORMALS,

	// this is the serialized connectivity info, without any header
	CCF_STENCIL_SHADOW_CONNECTIVITY,

	// this is the vertex skin, serialized CrySkinFull
	CCF_SKIN_VERTICES,

	// the normal skin, serialized CrySkinFull
	CCF_SKIN_NORMALS,

	// the skin representing tangents,
	CCF_SKIN_TANGENTS,
	
	// the array of materials: it's just followed by an integral number of structures MAT_ENTITY
	CCF_MATERIALS,

	// these are the faces, in internal indexation. The chunk data is a set of triples of 
	// ushorts (CCFIntFace) representing each face's vertices (internal indexation) and
	// a set of unsigned chars (CCFIntFaceMtlID) representing the face materials
	CCF_GI_INT_FACES,

	// This is compound chunk containing physical info for bones of the corresponding LOD
	// (The LOD is the # of occurence of this chunk in the file; there must be at most 2 such chunks)
	// It's followed by a CCFBoneGeometry with the number of subsequent CCF_BG_BONE chunks
	CCF_BONE_GEOMETRY,

	// This is one bone geometry info for a bone. Followed by struct CCFBGBone
	// and the necessary info arrays (see CCFBGBone comments)
	CCF_BG_BONE,

	// This is the morph target set for the geometry in this file. It contains
	// ALL the morph targets for ALL LODs
	// The header is CCFMorphTargetSet, followed by the specified number of
	// CCF_MORPH_TARGET chunks
	CCF_MORPH_TARGET_SET,

	// This is the morph target. The header is CCFMorphTarget, followed by 
	// the raw morph target skin (serialized from the CrySkinMorph) and then the name
	// All the serialized skins and the name are padded for 4 byte alignment
	CCF_MORPH_TARGET,

	// This is the array of light description for a character
	// this chunk has the header CCFCharLightDesc and then
	// the corresponding number of serialized CBoneLightBindInfo's
	// all of them must be 4-byte-aligned.
	CCF_CHAR_LIGHT_DESC,

	// this is the compiled CAL file, that contains information about the 
	// animation names, file names and directives
	// in case there's no CAL, the Resource Compiler finds all the proper files
	// in the current directory and forms a simple CAL script from them.
	// this chunk consists of subchunks that define animations or commands (CCF_ANIM_SCRIPT_*)
	CCF_ANIM_SCRIPT,

	// this subchunk registers a dummy animation (animation that's ignored if it's started)
	// It's followed by the name of the animation
	CCF_ANIM_SCRIPT_DUMMYANIM,

	// the subchunk of CCF_ANIM_SCRIPT, defines the animation name and path, delimited by \0
	// it contains CCFAnimInfo structure with the base info about the animation (needed to avoid loading animation
	// at the beginning of the frame), which is followed by the animation name, then \0, then the path
	CCF_ANIM_SCRIPT_ANIMINFO,

	// the subchunk of CCF_ANIM_SCRIPT, defines a directive AnimationDir, contains the directory itself
	// this should be concatenated with the last directory (new current dir relative to the current directory)
	CCF_ANIM_SCRIPT_ANIMDIR,

	// the subchunks of CCF_ANIM_SCRIPT, define directives ModelOffsetX, ModelOffsetY, ModelOffsetZ
	// contains 3 floats with the values of offsets
	CCF_ANIM_SCRIPT_MODELOFFSET,

	// chunk with a set of string pairs representing the name and value of
	// user-defined scene properties. It can be inside a LOD (geometry) chunk
	// or as a top-level ccg chunk
	CCF_USER_PROPERTIES
};

// THis is the header of CCF_ANIM_SCRIPT_ANIMINFO
struct CCFAnimInfo
{
	// combination of GlobalAnimation internal flags
	unsigned nAnimFlags;
	
	// timing data, retrieved from the timing_chunk_desc
	int nTicksPerFrame;
	float fSecsPerTick;

	// the start/end of the animation, in ticks
	int nRangeStart;
	int nRangeEnd;

	// number of controllers in the file
	unsigned numControllers;

	// initializes the values in the structure to some sensible defaults
	void init()
	{
		nAnimFlags = 0;
		nTicksPerFrame = 160;
		fSecsPerTick = 0.000208f;
		nRangeStart = 0;
		nRangeEnd = 900; // this is in ticks, means 30 frames
		numControllers = 0;
	}
};


// this is the header of CCF_CHAR_LIGHT_DESC chunk and is followed by
// the corresponding number of serialized CBoneLightBindInfo's
// all of them must be 4-byte-aligned.
struct CCFCharLightDesc
{
	// total number of lights
	unsigned numLights;
	// the first numLocalLights lights are local
	unsigned numLocalLights;
};

// This is the header of CCF_MORPH_TARGET_SET chunk, followed by the specified number of
// CCF_MORPH_TARGET chunks
struct CCFMorphTargetSet
{
	// the number of morph targets in this set.
	unsigned numMorphTargets;
};

// Represents one morph target; header of CCF_MORPH_TARGET chunk
// followed by the raw morph target skin (serialized from the CrySkinMorph) and then the name
// All the serialized skins and the name are padded for 4 byte alignment
struct CCFMorphTarget
{
	// number of LODs (skins) in this morph target. Must be 1 by now
	unsigned numLODs;
};


// this is the bone geometry for physics chunk header
struct CCFBoneGeometry
{
	// the number of CCF_BG_BONE chunks inside this chunk
	unsigned numBGBones;
};

//////////////////////////////////////////////////////////////////////////
// The header of the physical bone chunk, containing the whole info to initialize
// physical geometry of the bone
struct CCFBGBone
{
	// the index of the bone, in LOD0 indexation (to be directly plugged into the bone array)
	unsigned nBone;
	
	// the geometry data
	// the number of vertices in the mesh. This structure is immediately followed by
	// a Vec3d array of this size
	unsigned numVertices;
	// the number of faces. The number of CCFIntFace structures immediately follow the
	// vertex array. After them, the same number of unsigned chars follow, that describe each face material
	unsigned numFaces;
};

#pragma pack(push,2)
struct CCFIntFace
{
	unsigned short v[3];
	CCFIntFace (){}
	
	CCFIntFace (const unsigned short* p)
	{
		v[0] = p[0]; v[1] = p[1]; v[2] = p[2];
	}
	void operator = (const CryFace& face)
	{
		v[0] = face.v0;
		v[1] = face.v1;
		v[2] = face.v2;
	}
};
#pragma pack(pop)
typedef unsigned char CCFIntFaceMtlID;

// this structure appears in the CCF_HEADER_CCG chunk at the top of the CCG file
// and contains general information about the model being loaded
struct CCFCCGHeader
{
	// number of lods in this file
	unsigned numLODs;
};

struct CCFAnimGeomInfo
{
	// number of vertices and normals in internal indexation
	unsigned numVertices;
	// number of vertices, normals, UVs and Tangent Bases (external indexation)
	unsigned numExtTangents;
	// number of faces
	unsigned numFaces;
	// number of indices in the index buffer (currently always 3*numFaces)
	unsigned numIndices;
	// number of material/primitive groups
	unsigned numPrimGroups;
};

struct CCFBoneDescArrayHeader
{
	// number of bones in the hierarchy; each bone should be read with Serialize function of CryBoneDesc
	unsigned numBones;
};

// array of these structures represents the groups of indices in the index buffer:
// each group has its own material id and number of elements (indices, i.e. number of faces * 3 in case of strip stripification)
struct CCFMaterialGroup
{
	// material index in the original indexation of CGF
	unsigned nMaterial;
	// the first index in the final index buffer
	unsigned short nIndexBase;
	// number of indices in the final index buffer
	unsigned short numIndices;
};


// This is utility that can be used to easily write CCF format files
// just open the file, create this object on stack and add the chunk with
// AddChunk(nType)
// Then write the chunk directly to the file, and then add new chunk,
// repeat the process..
// Destroy (or call CloseChunk()) this object before you close the file.
// To have nested chunks, create another instance of CCFWriter and use it for some time.
// When you destruct it, you can use the upper-level writer. And so on.
class CCFFileWriter
{
public:
	CCFFileWriter (FILE* f):
		m_pFile (f), m_nLastChunk (-1)
	{
	}

	CCFFileWriter ():
		m_pFile(NULL), m_nLastChunk(-1)
	{
	}

	~CCFFileWriter()
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
	size_t AddChunk(CCFChunkTypeEnum nType)
	{
		// first, end the previous chunk
		CloseChunk();
		// remember the current position
		m_nLastChunk = ftell (m_pFile);

		// write the chunk header
		CCFChunkHeader header;
		header.nType = nType;
		header.nSize = 0;
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
		fseek (m_pFile, (INT_PTR)(&((CCFChunkHeader*)0)->nSize)+m_nLastChunk, SEEK_SET);
		unsigned nSize = nNewChunkPos - m_nLastChunk;
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

//////////////////////////////////////////////////////////////////////////
// This class is used to read the CCF files.
// Just open the file, and pass it to an instance of this class CCFReader.
// Immediately after construction, check IsEnd(). If it's false, the chunk header will be available via 
// getType(), getChunkSize(), getDataSize() and readData()
// Call Skip() until there are no more available data (in which case Skip() will return false)
class CCFFileReader
{
public:
	// ASSUMES: the file pointer is exactly at some chunk header
	CCFFileReader (FILE* f):
		m_pFile (f)
	{
		Reset();
	}

	// starts all over again
	void Reset()
	{
		m_nNextChunk = 0;
		Skip();
	}

	// indicates the end of data stream, or error
	bool IsEnd()
	{
		return m_Header.nType == CCF_NOT_CHUNK;
	}

	CCFChunkTypeEnum GetChunkType()
	{
		return (CCFChunkTypeEnum)m_Header.nType;
	}

	// returns the whole chunk size, including the chunk header, in bytes
	unsigned GetChunkSize()const
	{
		return m_Header.nSize;
	}

	// returns the chunk data size, in bytes
	unsigned GetDataSize ()const
	{
		return m_Header.nSize - sizeof(m_Header);
	}

	// reads the data into the supplied buffer (must be at least GetDataSize() bytes)
	// returns true when successful
	bool ReadData (void* pBuffer)
	{
		return 1 == fread (pBuffer, GetDataSize(), 1, m_pFile);
	}

	// skips the current chunk and goes no to the next one.
	// returns true if successful
	bool Skip ()
	{
		// be pessimistic - we won't read it
		m_Header.nType = CCF_NOT_CHUNK;
		m_Header.nSize = -1;

		if (!m_pFile)
			return false;

		if (fseek (m_pFile, m_nNextChunk, SEEK_SET))
			return false; // couldn't seek

		if (1 != fread (&m_Header, sizeof(m_Header),1, m_pFile))
			return false; // couldn't read

		m_nNextChunk += m_Header.nSize;

		return true;// read it
	}
protected:
	// current file
	FILE* m_pFile;
	// current chunk header (the file pointer is located after this chunk)
	CCFChunkHeader m_Header;
	// the next chunk absolute position
	int m_nNextChunk;
};


//////////////////////////////////////////////////////////////////////////
// This class is used to read the CCF-formatted memory mapped files.
// Just pass the block of memory representing contents of CCF file to an instance of this class.
// Immediately after construction, check IsEnd(). If it's false, the chunk header will be available via 
// getChunkType(), getChunkSize(), getDataSize() and getData()
// Call Skip() until there are no more available data (in which case Skip() will return false)
class CCFMemReader
{
public:

	// we don't support chunks of > this size
	enum {g_nMaxChunkSize = 0x40000000};

	// ASSUMES: the file pointer is exactly at some chunk header
	CCFMemReader (const void* pData, unsigned nSize):
		m_pData ((const char*)pData), m_pEnd ((const char*)pData + nSize)
	{
		Reset();
	}


	// starts all over again
	void Reset ()
	{
		if (m_pEnd - m_pData > sizeof(CCFChunkHeader) && ((unsigned)(m_pEnd - m_pData)) <= g_nMaxChunkSize)
		{
			m_pHeader = (CCFChunkHeader*)m_pData;
			if (((const char*)m_pHeader)+m_pHeader->nSize > m_pEnd || m_pHeader->nSize > g_nMaxChunkSize)
				m_pHeader = NULL; // Error: data truncated
		}
		else
			m_pHeader = NULL; // Error: header truncated
	}

	// indicates the end of data stream, or error
	bool IsEnd()
	{
		return !m_pHeader;
	}


	CCFChunkTypeEnum GetChunkType()
	{
		return (CCFChunkTypeEnum)m_pHeader->nType;
	}

	// returns the whole chunk size, including the chunk header, in bytes
	unsigned GetChunkSize()const
	{
		return m_pHeader->nSize;
	}

	// returns the chunk data size, in bytes
	unsigned GetDataSize ()const
	{
		return m_pHeader->nSize - sizeof(*m_pHeader);
	}

	// reads the data into the supplied buffer (must be at least GetDataSize() bytes)
	// returns true when successful
	const void* GetData ()
	{
		return m_pHeader+1;
	}

	// skips the current chunk and goes no to the next one.
	// returns true if successful
	bool Skip ()
	{
		// be optimistic - assume we will end up with successful step to the next chunk
		m_pHeader = (const CCFChunkHeader*)(((const char*)m_pHeader) + m_pHeader->nSize);
		if ((const char*)(m_pHeader+1) > m_pEnd)
		{
			// there's no next chunk, or its header is truncated
			m_pHeader = NULL;
			return false;
		}

		if (m_pHeader->nSize < sizeof(CCFChunkHeader)
			|| (unsigned)m_pHeader->nSize > g_nMaxChunkSize)
		{
			// the header size is unsupported (most probably a noisy header)
			m_pHeader = NULL;
			return false;
		}

		if (((const char*)m_pHeader)+m_pHeader->nSize > m_pEnd)
		{
			// the chunk header is maybe ok, but the chunk data is truncated; or maybe
			// the chunk header contains noise
			m_pHeader = NULL;
			return false;
		}

		return true;
	}

protected:
	// the whole data 
	const char* m_pData;
	// the end of the whole data
	const char* m_pEnd;
	// current chunk header (the file pointer is located after this chunk)
	const CCFChunkHeader* m_pHeader;
};

#endif
