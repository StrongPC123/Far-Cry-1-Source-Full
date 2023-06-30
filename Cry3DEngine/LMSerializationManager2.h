#ifndef __LM_SERIALIZATION_MANAGER_2_H__
#define __LM_SERIALIZATION_MANAGER_2_H__

#include <ILMSerializationManager.h>
#include "LMCompStructures.h"
#include "I3DEngine.h"

#include <map>

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

class CTempFile
{
public:
	CTempFile(unsigned nReserve = 0)
	{
		m_arrData.reserve (nReserve);
	}

	void WriteData (const void* pData, unsigned nSize)
	{
		if (nSize)
		{
			unsigned nOffset = m_arrData.size();
			m_arrData.resize(nOffset + nSize);
			memcpy (&m_arrData[nOffset], pData, nSize);
		}
	}

	template <typename T>
	void Write (const T& t)
	{
		WriteData (&t, sizeof(T));
	}

	template <typename T>
	void Write (const T* pT, unsigned numElements)
	{
		WriteData (pT, numElements*sizeof(T));
	}

	unsigned GetSize() const {return m_arrData.size();}
	void SetSize (unsigned n){m_arrData.resize (n);}
	char* GetData() {return &m_arrData[0];}

	void Clear() {m_arrData.clear();}
	void Reserve (unsigned n) {m_arrData.reserve (n);}
	void Init (unsigned nSize)
	{
		m_arrData.clear();
		m_arrData.resize (nSize);
	}
protected:
	std::vector<char> m_arrData;
};

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

class CLMSerializationManager2 : public Cry3DEngineBase, public ILMSerializationManager
{	
public:

	CLMSerializationManager2( );
	virtual ~CLMSerializationManager2();

	// interface ILMSerializationManager ------------------------------------

	virtual void Release() { delete this; };
	virtual bool ApplyLightmapfile( const char *pszFileName, std::vector<IEntityRender *>& vIGLMs );
	virtual bool Load(const char *pszFileName, const bool cbLoadTextures = true);
	virtual unsigned int Save(const char *pszFileName, LMGenParam rParam, const bool cbAppend = false);
	virtual void AddRawLMData( 
		const DWORD indwWidth, const DWORD indwHeight, const std::vector<int>& _cGLM_IDs_UsingPatch,
		BYTE *_pColorLerp4, BYTE *_pHDRColorLerp4, BYTE *_pDomDirection3, BYTE *_pOccl2 = 0);
	virtual void AddTexCoordData(const std::vector<TexCoord2Comp>& vTexCoords, int iGLM_ID_UsingTexCoord, const DWORD indwHashValue, const std::vector<std::pair<EntityId, EntityId> >& rOcclIDs);
	virtual DWORD GetHashValue(const int iniGLM_ID_UsingTexCoord) const;
	virtual bool ExportDLights(const char *pszFileName, const CDLight **ppLights, UINT iNumLights, bool bNewZip = true) const;
	virtual bool LoadDLights(const char *pszFileName, CDLight **&ppLightsOut, UINT &iNumLightsOut) const;
	RenderLMData * CreateLightmap(const string& strDirPath, int nItem,UINT iWidth, UINT iHeight, const bool cbLoadHDRMaps, const bool cbLoadOcclMaps = false);

	//! Create a dot3 lightmap ColorLerp / DomDirection tetxure pair
	virtual RenderLMData * CreateLightmap(const char *pszFileName, int nItem, UINT iWidth, UINT iHeight, BYTE *pColorLerp4, BYTE *pHDRColorLerp4, BYTE *pDomDirection3, BYTE *pOccl2 = 0);
// ----------------------------------------------

protected:
	struct FileHeader
	{
		enum {MAGIC_NUMBER  = 0x8F23123E};
		enum {VERSION = 3};
		FileHeader() { iMagicNumber = MAGIC_NUMBER;iVersion = VERSION; };

		UINT iMagicNumber;
		UINT iVersion;
		UINT iNumLM_Pairs;
		UINT iNumTexCoordSets;

		UINT reserved[4];
	};

	struct LMHeader
	{
		// the dimensions of LM
		UINT iWidth;
		UINT iHeight;
		// number of GLMs using this LM
		UINT numGLMs; 
	};

	struct UVSetHeader
	{
		UINT nIdGLM;
		UINT nHashGLM;
		UINT numUVs;
		UVSetHeader() : nIdGLM(0), nHashGLM(0), numUVs(0){}
	};
	
	//new version available from ver 3 on
	struct UVSetHeader3 : UVSetHeader
	{
		EntityId OcclIds[4*2];	//new: the occlusion map colour channel light id's, this corresponds to the std::pair<EntityId, EntityId>
		unsigned char ucOcclCount/*1..4*/;
		UVSetHeader3():ucOcclCount(0)
		{
			UVSetHeader::UVSetHeader();
			OcclIds[0] = OcclIds[1] = OcclIds[2] = OcclIds[3] = 0;
		}
	};

	struct RawLMData
	{
		//! /param _pColorLerp4 if !=0 this memory is copied
		//! /param _pDomDirection3 if !=0 this memory is copied
		RawLMData(const DWORD indwWidth, const DWORD indwHeight, const std::vector<int>& _vGLM_IDs_UsingPatch )
		{
			vGLM_IDs_UsingPatch = _vGLM_IDs_UsingPatch;
			m_dwWidth=indwWidth;
			m_dwHeight=indwHeight;
			m_bUseOcclMaps = false;
		};

		enum BitmapEnum
		{
			TEX_COLOR,
			TEX_DOMDIR,
			TEX_OCCL,
			TEX_HDR
		};

		// initializes from raw bitmaps
		void initFromBMP (BitmapEnum t, const void* pSource);

		// initializes from files
		bool initFromDDS (BitmapEnum t, ICryPak* pPak, const string& szFileName);

		std::vector<int>						vGLM_IDs_UsingPatch;				//!< vector of object ids that use this lightmap

		// the color DDS, as is in the file
		CTempFile m_ColorLerp4;
		// the color DDS, as is in the file
		CTempFile m_HDRColorLerp4;
		// the dominant direction DDS, as is in the file
		CTempFile m_DomDirection3;
		// the occlusion map DDS, as is in the file
		CTempFile m_Occl2;

		DWORD												m_dwWidth;									//!<
		DWORD												m_dwHeight;									//!<
		bool												m_bUseOcclMaps;
		bool												m_bUseHDRMaps;

	private:

		//! copy constructor (forbidden)
		RawLMData( const RawLMData &a )	{}

		//! assignment operator (forbidden)
		RawLMData &operator=( const RawLMData &a ) {	return(*this); }
	};

	struct RawTexCoordData
	{
		//! default constructor (needed for std::map)
		RawTexCoordData()	{}

		RawTexCoordData( const std::vector<TexCoord2Comp>& _vTexCoords, const DWORD indwHashValue, const std::vector<std::pair<EntityId, EntityId> >& rOcclIDs )
		{
			vTexCoords = _vTexCoords;
			m_dwHashValue=indwHashValue;
			vOcclIDs = rOcclIDs;
		};

		RawTexCoordData( const std::vector<TexCoord2Comp>& _vTexCoords, const DWORD indwHashValue )
		{
			vTexCoords = _vTexCoords;
			m_dwHashValue=indwHashValue;
			vOcclIDs.clear();
		};

		std::vector<TexCoord2Comp>		vTexCoords;									//!<
		DWORD							m_dwHashValue;								//!< to detect changes in the lighting (for incremental recompile)
		std::vector<std::pair<EntityId, EntityId> >	vOcclIDs;						//!< occlusion indices corresponding to the 0..4 colour channels
	};

	std::vector<RawLMData *>			m_vLightPatches;							//!< class is responsible for deleteing this 
	std::map<int,RawTexCoordData>		m_vTexCoords;								//!<

	// \param inpIGLMs pointer to the objects we want to assign the data(instance is not touched), 0 if we want to load it to the instance
	bool _Load( const char *pszFileName, std::vector<IEntityRender *> *inpIGLMs, const bool cbLoadTextures = true);

	void WriteString(const char *pszStr, CTempFile& f) const
	{
		UINT iStrLen = (UINT) strlen(pszStr);
		f.Write (iStrLen);
		f.WriteData(pszStr, iStrLen);
	};
};
#endif // __LM_SERIALIZATION_MANAGER_H__