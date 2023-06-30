#define NO_GDI
	#include "stdafx.h"
#undef NO_GDI
#include "Cry3dEngineBase.h"
//#include <d3d8types.h>
#include "dds.h"
#include "LMSerializationManager2.h"
#include <StringUtils.h>
#include "IEntityRenderState.h"
#include <algorithm>									// STL find
#include <set>												// STL set<>

#include <memory>

#include "ObjMan.h"
#include "Brush.h"

#define LEVELLM_PAK_NAME "LevelLM.pak"

CLMSerializationManager2::CLMSerializationManager2(){}

CLMSerializationManager2::~CLMSerializationManager2()
{
	for (std::vector<RawLMData *>::iterator itLMData=m_vLightPatches.begin(); itLMData!=m_vLightPatches.end(); itLMData++)
		delete (*itLMData); 
}

void CLMSerializationManager2::AddRawLMData( 
	const DWORD indwWidth, const DWORD indwHeight, const std::vector<int>& _vGLM_IDs_UsingPatch,
	BYTE *_pColorLerp4, BYTE *_pHDRColorLerp4, BYTE *_pDomDirection3, BYTE *_pOccl2)
{
	std::vector<RawLMData *>::iterator itLMData;
	// for all stored lightmaps
	for(itLMData=m_vLightPatches.begin(); itLMData!=m_vLightPatches.end();)
	{
		std::vector<int>::const_iterator itID;

		RawLMData *pCurRawLMData = *itLMData;

		// for all the given new object ids
		for (itID=_vGLM_IDs_UsingPatch.begin(); itID!=_vGLM_IDs_UsingPatch.end(); itID++)
		{
			int id=*itID;
			std::vector<int>::iterator itRemove=std::find(pCurRawLMData->vGLM_IDs_UsingPatch.begin(),pCurRawLMData->vGLM_IDs_UsingPatch.end(),id);
			if(itRemove!=pCurRawLMData->vGLM_IDs_UsingPatch.end())
				pCurRawLMData->vGLM_IDs_UsingPatch.erase(itRemove);
		}
 
		// if the lightmap is no longer used by any object - remove it
		if(pCurRawLMData->vGLM_IDs_UsingPatch.empty())
		{
			delete pCurRawLMData;
			pCurRawLMData=0;
			itLMData=m_vLightPatches.erase(itLMData);
		}
		else
			++itLMData;
	}

	RawLMData* pRawLM = new RawLMData(indwWidth, indwHeight,_vGLM_IDs_UsingPatch);
	pRawLM->initFromBMP (RawLMData::TEX_COLOR, _pColorLerp4);
	pRawLM->initFromBMP (RawLMData::TEX_DOMDIR, _pDomDirection3);
	if(_pOccl2 != 0)
	{
		pRawLM->initFromBMP (RawLMData::TEX_OCCL, _pOccl2);
		pRawLM->m_bUseOcclMaps = true;
	}
	else
		pRawLM->m_bUseOcclMaps = false;
	if(_pHDRColorLerp4 != 0)
	{
		pRawLM->initFromBMP (RawLMData::TEX_HDR, _pHDRColorLerp4);
		pRawLM->m_bUseHDRMaps = true;
	}
	else
		pRawLM->m_bUseHDRMaps = false;
	m_vLightPatches.push_back(pRawLM);
}

void CLMSerializationManager2::AddTexCoordData(const std::vector<TexCoord2Comp>& vTexCoords, int iGLM_ID_UsingTexCoord, const DWORD indwHashValue, const std::vector<std::pair<EntityId, EntityId> >& rOcclIDs)
{ 
	m_vTexCoords[iGLM_ID_UsingTexCoord] = RawTexCoordData(vTexCoords,indwHashValue, rOcclIDs); 
} 

struct AutoFileCloser: Cry3DEngineBase
{
	AutoFileCloser(FILE*pFile):m_pFile(pFile){}
	~AutoFileCloser(){if (m_pFile)GetPak()->FClose(m_pFile);}
protected:
	FILE* m_pFile;
};

#ifdef WIN64
#pragma warning( push )									//AMD Port
#pragma warning( disable : 4267 )
#endif

bool CLMSerializationManager2::_Load( const char *pszFileName, std::vector<IEntityRender *> *inpIGLMs, const bool cbLoadTextures) 
{
	// ---------------------------------------------------------------------------------------------
	// Reconstruct lightmap data from pszFileName
	// ---------------------------------------------------------------------------------------------
	GetSystem()->GetILog()->UpdateLoadingScreen("\003Loading lightmaps ...");

	string strDirName = CryStringUtils::GetParentDirectory<string>(pszFileName);
	// make sure the pak file in which this LM data resides is opened
	GetPak()->OpenPack ((strDirName + "\\" LEVELLM_PAK_NAME).c_str());

	FileHeader sHeader;
	size_t iNumItemsRead;
	std::map<int, RenderLMData_AutoPtr> mapLMData;			// only used if inpIGLMs!=0

	if(inpIGLMs)
	{
		if(inpIGLMs->empty())
		{
			GetSystem()->GetILog()->Log("No GLMs to load lightmaps for");
			return false;
		}
	}
	else
	{
		assert(m_vLightPatches.empty());
		assert(m_vTexCoords.empty());
	}

	// Open file
	FILE* hFile = GetPak()->FOpen(pszFileName, "rb");
	if (hFile == NULL)
	{
		GetSystem()->GetILog()->Log("Could not load lightmap file");
		return false;
	}
	AutoFileCloser _AutoClose (hFile);

	// Read header
	iNumItemsRead = GetPak()->FRead(&sHeader, sizeof(FileHeader), 1, hFile);
	// Read LM texture data
	for (UINT iCurLMTexData = 0; iCurLMTexData < sHeader.iNumLM_Pairs; ++iCurLMTexData)
	{
		LMHeader LM;

		// width, height and the number of UVs
		if (1 != GetPak()->FRead(&LM, sizeof(LM), 1, hFile))
		{
			Warning (0, pszFileName, "Cannot read LM data header #%d", iCurLMTexData);
			GetSystem()->GetILog()->Log("Cannot read Lightmap data header ");
			return false;
		}

		std::vector<int> vGLM_IDs_UsingPatch;				// objects that use this lightmap texture
		vGLM_IDs_UsingPatch.resize (LM.numGLMs);

		// Read IDs of GLMs which use this texture pair
		if (1 != GetPak()->FRead(&vGLM_IDs_UsingPatch[0], sizeof(int)*LM.numGLMs, 1, hFile))
		{
			Warning (0, pszFileName, "Cannot read LM data #%d", iCurLMTexData);
			GetSystem()->GetILog()->Log("Cannot read Lightmap data");
			return false;
		}

		std::vector<BYTE>vDomLightDir, vColorMap;
		if(inpIGLMs)
		{
			// Create lightmap
			RenderLMData_AutoPtr pNewLM = 
				sHeader.iVersion == 2?
				CreateLightmap(strDirName, iCurLMTexData,LM.iWidth, LM.iHeight, false/*don't load occl maps*/) :
				CreateLightmap(strDirName, iCurLMTexData,LM.iWidth, LM.iHeight, true/*load occl maps*/);
		
			if (!pNewLM)
			{
				GetSystem()->GetILog()->Log("CreateLightmap has failed");
				return false;
			}

			// Instance is assigned later when we get the texture coordinates. Store a
			// reference so we can later safely release each element
			for (std::vector<int>::iterator itID=vGLM_IDs_UsingPatch.begin(); itID!=vGLM_IDs_UsingPatch.end(); itID++)
			{
				// Shouldn't happen, probably need to fix something if. Might be wrong LM data, just
				// delete the file or ignore the assert
				assert (mapLMData.find(*itID) == mapLMData.end());

				mapLMData[*itID] = pNewLM;
			}
		}
		else
		{
			RawLMData* pRawLM = new RawLMData(LM.iWidth,LM.iHeight,vGLM_IDs_UsingPatch);

			char szPostfix[8];
			if(cbLoadTextures)
			{
				sprintf(szPostfix, "%d.dds", iCurLMTexData);
				pRawLM->initFromDDS(RawLMData::TEX_COLOR, GetPak(), strDirName + "\\c" + szPostfix);
				pRawLM->initFromDDS(RawLMData::TEX_DOMDIR, GetPak(), strDirName + "\\d" + szPostfix);
				if(sHeader.iVersion > 2)//load occlusion maps too
        {
					pRawLM->initFromDDS(RawLMData::TEX_OCCL, GetPak(), strDirName + "\\o" + szPostfix);
					pRawLM->initFromDDS(RawLMData::TEX_HDR, GetPak(), strDirName + "\\r" + szPostfix);
        }
			}
			m_vLightPatches.push_back(pRawLM);
		}
	}
	// Read texture coordinate data
	for (UINT iNumTexCoordSets=0; iNumTexCoordSets<sHeader.iNumTexCoordSets; iNumTexCoordSets++)
	{
		UVSetHeader *pUVh(NULL);
		if(sHeader.iVersion >= 3)
			pUVh = new UVSetHeader3();
		else
			pUVh = new UVSetHeader();//old format
		assert(pUVh);
		std::auto_ptr<UVSetHeader> uvh(pUVh);
		// Read position of GLM which uses this texture coordinate set
		if (1 != GetPak()->FRead(pUVh, (sHeader.iVersion>=3)?sizeof(UVSetHeader3):sizeof(UVSetHeader), 1, hFile))
		{
			GetSystem()->GetILog()->Log("Could not read texture coordinates for lightmaps");
			return false;
		}
		std::vector<TexCoord2Comp> vTexCoords;
		vTexCoords.resize(pUVh->numUVs);
		if (1 != GetPak()->FRead(&vTexCoords[0], sizeof(TexCoord2Comp)*pUVh->numUVs, 1, hFile))
		{
			GetSystem()->GetILog()->Log("Could not read texture coordinates for lightmaps");
			return false;
		}
#ifdef _DEBUG
		for (UINT iCurTexCrd=0; iCurTexCrd<vTexCoords.size(); iCurTexCrd++)
		{
			if(	vTexCoords[iCurTexCrd].s >= 0.0f && vTexCoords[iCurTexCrd].s <= 1.0f && 
					vTexCoords[iCurTexCrd].t >= 0.0f && vTexCoords[iCurTexCrd].t <= 1.0f )
					continue;

			Warning(0, pszFileName, "CLMSerializationManager2::_Load: Lightmap texture coordinates out of range");
			break;
		}
#endif
		// Hand out data/references to the GLMs
		if(inpIGLMs)
		{
			std::vector<IEntityRender *>::iterator itGLM;

			for (itGLM=inpIGLMs->begin(); itGLM!=inpIGLMs->end(); ++itGLM)
			{
				IEntityRender *pICurGLM = (* itGLM);
				
				// Correct GLM for this texture coordinate set ?
				if ( pICurGLM->GetEditorObjectId() == pUVh->nIdGLM && pICurGLM->GetEntityRS() )
				{
					std::map<int, RenderLMData_AutoPtr>::iterator itRenderLMData = mapLMData.find(pUVh->nIdGLM);

					if (itRenderLMData == mapLMData.end())
					{
						// We shouldn't have a texture coordinate set for a GLM that has no matching LM object
						assert(itRenderLMData != mapLMData.end());
						break;
				 	}

					// Copy texture coordinates and hand out a reference to the lightmap object
					if(sHeader.iVersion >= 3 && (strcmp(pICurGLM->GetEntityClassName(), "Brush") == 0))
					{
						const UVSetHeader3 *puvh = static_cast<const UVSetHeader3*>(pUVh);
						//create vector with light ids
						std::vector<std::pair<EntityId,EntityId> > vIDs;	vIDs.resize(puvh->ucOcclCount);
						for(int i=0; i<puvh->ucOcclCount; ++i)
						{
							vIDs[i].first = puvh->OcclIds[2*i];
							vIDs[i].second = puvh->OcclIds[2*i+1];
						}
						CBrush *pBrush = reinterpret_cast<CBrush*>(pICurGLM);//safe due to GetEntityClassName
						pBrush->SetLightmap (itRenderLMData->second, (float *) &vTexCoords[0], vTexCoords.size(), puvh->ucOcclCount, vIDs);
					}
					else
						pICurGLM->SetLightmap (itRenderLMData->second, (float *) &vTexCoords[0], vTexCoords.size());

					// Texture coordinates are per-instance
					break;
				}
			}
		}
		else	// !inpIGLMs
		{
			m_vTexCoords[pUVh->nIdGLM] = RawTexCoordData(vTexCoords, pUVh->nHashGLM);
		}
	}

	GetSystem()->GetILog()->UpdateLoadingScreenPlus(" %d elements loaded", mapLMData.size());
	GetSystem()->GetILog()->Log("Finished loading lightmaps ...");
	return true;
}

bool CLMSerializationManager2::Load(const char *pszFileName, const bool cbLoadTextures) 
{
	return(_Load(pszFileName, 0, cbLoadTextures));
}

bool CLMSerializationManager2::ApplyLightmapfile(const char *pszFileName, std::vector<IEntityRender *>& vIGLMs ) 
{
	return(_Load(pszFileName,&vIGLMs));
}

//////////////////////////////////////////////////////////////////////////
uint8* MemBufferCounter=0;
int m_numMips=0;

HRESULT SaveCompessedMipmapLevel( void * data, int miplevel, DWORD size, int width, int height, void * user_data )
{
	assert( MemBufferCounter );
	uint8* src=(uint8*)data;
	for (uint32 i=0; i<size; i++) 
	{
		*MemBufferCounter=src[i];
		MemBufferCounter++;
	}
	m_numMips++;
	return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------
unsigned int CLMSerializationManager2::Save(const char *pszFilePath, LMGenParam rParam, const bool cbAppend )
{ 
	// ---------------------------------------------------------------------------------------------
	// Store the lightmap data added in pszFileName
	// ---------------------------------------------------------------------------------------------
	string strDirName = CryStringUtils::GetParentDirectory<string>(pszFilePath);
	const char* pFileName = pszFilePath + (strDirName.empty()?0:strDirName.length()+1);
	string strPakName = strDirName + "\\" LEVELLM_PAK_NAME;
	GetPak()->ClosePack(strPakName.c_str());
	// make sure the pak file in which this LM data resides is opened
	SetFileAttributes(strPakName.c_str(), FILE_ATTRIBUTE_NORMAL);
	ICryArchive_AutoPtr pPak = GetPak()->OpenArchive (strPakName.c_str(), ICryArchive::FLAGS_RELATIVE_PATHS_ONLY);
	if (!pPak)
		return NSAVE_RESULT::EPAK_FILE_OPEN_FAIL;
	unsigned int uiStartIndex = 0;
	if(cbAppend)
	{
		//seek to the right file
		for(;;)
		{
			char fileName[10];
			//create filename
			sprintf(fileName, "x%d.dds", uiStartIndex);
			if(pPak->FindFile(fileName) == NULL)
				break;
			uiStartIndex++;	
		}
	}

	FileHeader sHeader;
	std::map<int,RawTexCoordData>::iterator itTexCrdData;
	RawLMData *pCurRawLMData = NULL;
	RawTexCoordData *pCurRawTexCrdData = NULL;

	CTempFile fMem;
	// Write header
	sHeader.iNumLM_Pairs = m_vLightPatches.size();
	sHeader.iNumTexCoordSets = m_vTexCoords.size();
	fMem.Write(sHeader);

	// Write LM texture data
	for (UINT iCurLMTexData = 0; iCurLMTexData < m_vLightPatches.size(); ++iCurLMTexData)
	{
		pCurRawLMData = m_vLightPatches[iCurLMTexData];
		LMHeader LM;
		LM.iHeight = pCurRawLMData->m_dwHeight;
		LM.iWidth  = pCurRawLMData->m_dwWidth;
		LM.numGLMs = pCurRawLMData->vGLM_IDs_UsingPatch.size();
		fMem.Write(LM);

		fMem.Write(&pCurRawLMData->vGLM_IDs_UsingPatch[0], LM.numGLMs);
			
		if(iCurLMTexData >= uiStartIndex)
		{
			uint32 size	= pCurRawLMData->m_ColorLerp4.GetSize();
			uint8* pSr0	= (uint8*)pCurRawLMData->m_ColorLerp4.GetData();
			uint8* pSr1	= (uint8*)pCurRawLMData->m_DomDirection3.GetData();
			DDS_HEADER* pDDS=(DDS_HEADER*)(pSr0+4);
			uint32 width	=	pDDS->dwWidth;
			uint32 height	=	pDDS->dwHeight;

			char szName[8];
			sprintf(szName, "x%d.dds", iCurLMTexData/* + uiStartIndex*/);

			DDS_HEADER ddsh;
			if (0) 
			{//save uncompressed version
				szName[0] = 'c';
				pPak->UpdateFile (szName, pCurRawLMData->m_ColorLerp4.GetData(), pCurRawLMData->m_ColorLerp4.GetSize(), ICryArchive::METHOD_COMPRESS, ICryArchive::LEVEL_BEST);

			} else 
			{
			//-------------------------------------------------------------------------------
			//---- create DXT3-image for color-map ------------------------------------------
			//-------------------------------------------------------------------------------
				uint8* pColorMaps32	= (uint8*)pCurRawLMData->m_ColorLerp4.GetData();;
				uint8* pColorDXT3=(uint8*)malloc(size);
				MemBufferCounter = pColorDXT3;
				m_numMips=0;

				memset( (void*)(&ddsh), 0, sizeof(ddsh) );
				*MemBufferCounter='D'; MemBufferCounter++;
				*MemBufferCounter='D'; MemBufferCounter++;
				*MemBufferCounter='S'; MemBufferCounter++;
				*MemBufferCounter=' '; MemBufferCounter++;
				ddsh.dwSize = sizeof(DDS_HEADER);
				ddsh.dwWidth	= width;
				ddsh.dwHeight = height;
				ddsh.dwMipMapCount = 0;
				ddsh.ddspf = DDSPF_DXT3;
				ddsh.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP;
				ddsh.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE | DDS_SURFACE_FLAGS_MIPMAP;

				uint8* hsrcDXT3=(uint8*)(&ddsh);
				for (uint32 i=0; i<sizeof(ddsh); i++) {
					*MemBufferCounter=hsrcDXT3[i];	MemBufferCounter++;
				}

				if(!GetSystem()->GetIRenderer()->DXTCompress( 
					&pColorMaps32[0x80], 
					width, 
					height, 
					eTF_DXT3,										//DXT-format
					false,											//use hardware
					0,												//mipmaps on/off
					4,												//bytes per pixel 
					SaveCompessedMipmapLevel	//callback
				))	return NSAVE_RESULT::EDXT_COMPRESS_FAIL;

				DDS_HEADER* pDXT3=(DDS_HEADER*)(pColorDXT3+4);
				pDXT3->dwMipMapCount = m_numMips;
				if (m_numMips==1) pDXT3->dwMipMapCount = 0;

				szName[0] = 'c';
				uint32 dxt3size=MemBufferCounter-pColorDXT3;
				pPak->UpdateFile (szName, pColorDXT3, dxt3size, ICryArchive::METHOD_COMPRESS, ICryArchive::LEVEL_BEST);

				szName[0] = 'h';
				pPak->UpdateFile (szName, pCurRawLMData->m_ColorLerp4.GetData(), pCurRawLMData->m_ColorLerp4.GetSize(), ICryArchive::METHOD_COMPRESS, ICryArchive::LEVEL_BEST);

				free(pColorDXT3);
			}
			//-------------------------------------------------------------------------------------------
			szName[0] = 'd';
			pPak->UpdateFile (szName, pCurRawLMData->m_DomDirection3.GetData(), pCurRawLMData->m_DomDirection3.GetSize(), ICryArchive::METHOD_COMPRESS, ICryArchive::LEVEL_BEST);

			if(pCurRawLMData->m_bUseOcclMaps)
			{
				szName[0] = 'o';
				pPak->UpdateFile (szName, pCurRawLMData->m_Occl2.GetData(), pCurRawLMData->m_Occl2.GetSize(), ICryArchive::METHOD_COMPRESS, ICryArchive::LEVEL_BEST);
			}
			if(pCurRawLMData->m_bUseHDRMaps)
			{
				szName[0] = 'r';
				pPak->UpdateFile (szName, pCurRawLMData->m_HDRColorLerp4.GetData(), pCurRawLMData->m_HDRColorLerp4.GetSize(), ICryArchive::METHOD_COMPRESS, ICryArchive::LEVEL_BEST);
			}

			uint8* pSimpleLightmaps32	= (uint8*)malloc(size);
			//copy headder
			for (uint32 i=0; i<0x80; i++) {
				pSimpleLightmaps32[i] = pSr0[i];
			} 

			//create simple lightmap (RGBA)
			for (uint32 i=0x80; i<size; i=i+4) 
			{
				// compute lambert term with the surface normal and the averager light direction
				float NdotL = ((float)pSr1[i] - 127.5f) / 127.5f;
				// light behind the surface is 0
				if (NdotL < 0)	NdotL = 0;

				// convert RGBA 0..255 color to float 0..1
				float lmColor[4];
	#ifdef APPLY_COLOUR_FIX
	#ifdef USE_ALPHA_FOR_LOWSPEC
				lmColor[0] = (float)pSr0[i+0] / 255.0f;
				lmColor[1] = (float)pSr0[i+1] / 255.0f;
				lmColor[2] = (float)pSr0[i+2] / 255.0f;
	#else
				lmColor[0] = (float)pSr0[i+0] * (float)pSr1[i+3] / 255.0f / 255.0f;
				lmColor[1] = (float)pSr0[i+1] * (float)pSr1[i+3] / 255.0f / 255.0f;
				lmColor[2] = (float)pSr0[i+2] * (float)pSr1[i+3] / 255.0f / 255.0f;
	#endif
	#else
				lmColor[0] = (float)pSr0[i+0] / 255.0f;
				lmColor[1] = (float)pSr0[i+1] / 255.0f;
				lmColor[2] = (float)pSr0[i+2] / 255.0f;
	#endif
				lmColor[3] = (float)pSr0[i+3] / 255.0f;

				// calculate the sum of the directional term (NdotL * lmColor[3])
				// and the ambient term (1.0f-lmColor[3])
				float lmIntens = NdotL * lmColor[3] + (1.0f-lmColor[3]);

				// weight the colour with the calculated sum
	#ifdef USE_ALPHA_FOR_LOWSPEC
				pSimpleLightmaps32[i+0] = (byte)(lmColor[0] * 255.0f);
				pSimpleLightmaps32[i+1] = (byte)(lmColor[1] * 255.0f);
				pSimpleLightmaps32[i+2] = (byte)(lmColor[2] * 255.0f);
				// alpha is used
				pSimpleLightmaps32[i+3] = (byte)(lmIntens * (float)(pSr1[i+3]));
	#else
				pSimpleLightmaps32[i+0] = (byte)(lmColor[0] * lmIntens * 255.0f);
				pSimpleLightmaps32[i+1] = (byte)(lmColor[1] * lmIntens * 255.0f);
				pSimpleLightmaps32[i+2] = (byte)(lmColor[2] * lmIntens * 255.0f);
				// alpha is not used
				pSimpleLightmaps32[i+3] = 255;
	#endif
			}
		//-------------------------------------------------------------------------------
		//---- create DXT1-image --------------------------------------------------------
		//-------------------------------------------------------------------------------
			uint8* pSimpleLightmapsDXT1=(uint8*)malloc(size);
			MemBufferCounter = pSimpleLightmapsDXT1;
			m_numMips=0;
			memset( (void*)(&ddsh), 0, sizeof(ddsh) );
			*MemBufferCounter='D'; MemBufferCounter++;
			*MemBufferCounter='D'; MemBufferCounter++;
			*MemBufferCounter='S'; MemBufferCounter++;
			*MemBufferCounter=' '; MemBufferCounter++;
			ddsh.dwSize = sizeof(DDS_HEADER);
			ddsh.dwWidth	= width;
			ddsh.dwHeight	= height;
	#ifdef USE_ALPHA_FOR_LOWSPEC
			ddsh.ddspf		= DDSPF_DXT5;
	#else
			ddsh.ddspf		= DDSPF_DXT1;
	#endif
			ddsh.dwMipMapCount = 0;
			ddsh.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP;
			ddsh.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE | DDS_SURFACE_FLAGS_MIPMAP;

			uint8* hsrcDXT1=(uint8*)(&ddsh);
			for (uint32 i=0; i<sizeof(ddsh); i++) {
				*MemBufferCounter=hsrcDXT1[i];	MemBufferCounter++;
			}

	#ifdef USE_ALPHA_FOR_LOWSPEC
			if(!GetSystem()->GetIRenderer()->DXTCompress( 
				&pSimpleLightmaps32[0x80], 
				width, 
				height, 
				eTF_DXT5,										//DXT-format
				false,											//use hardware
				0,												//mipmaps on/off
				4,												//bytes per pixel 
				SaveCompessedMipmapLevel	//callback
			))
				return NSAVE_RESULT::EDXT_COMPRESS_FAIL;
	#else
			if(!GetSystem()->GetIRenderer()->DXTCompress( 
				&pSimpleLightmaps32[0x80], 
				width, 
				height, 
				eTF_DXT1,										//DXT-format
				false,											//use hardware
				0,							 					//mipmaps on/off
				4,												//bytes per pixel 
				SaveCompessedMipmapLevel	//callback
			))
				return NSAVE_RESULT::EDXT_COMPRESS_FAIL;
	#endif
			DDS_HEADER* pDXT1=(DDS_HEADER*)(pSimpleLightmapsDXT1+4);
			pDXT1->dwMipMapCount = m_numMips;
			if (m_numMips==1) pDXT1->dwMipMapCount = 0;
			uint32 dxtsize=MemBufferCounter-pSimpleLightmapsDXT1;
			szName[0] = 'x';
			pPak->UpdateFile (szName, pSimpleLightmapsDXT1, dxtsize, ICryArchive::METHOD_COMPRESS, ICryArchive::LEVEL_BEST);

			free(pSimpleLightmapsDXT1);
			free(pSimpleLightmaps32); 

		}
	}
	// Write texture coordinate data
	for (itTexCrdData=m_vTexCoords.begin(); itTexCrdData!=m_vTexCoords.end(); itTexCrdData++)
	{
		UVSetHeader3 uvh;
		pCurRawTexCrdData = &((*itTexCrdData).second);
		uvh.nIdGLM   = (*itTexCrdData).first;
		uvh.nHashGLM = pCurRawTexCrdData->m_dwHashValue;
		uvh.numUVs   = pCurRawTexCrdData->vTexCoords.size();
		uvh.ucOcclCount = pCurRawTexCrdData->vOcclIDs.size();
		for(int i=0; i<uvh.ucOcclCount; ++i)
		{
			uvh.OcclIds[2*i] = pCurRawTexCrdData->vOcclIDs[i].first;//real entity id
			uvh.OcclIds[2*i+1] = pCurRawTexCrdData->vOcclIDs[i].second;//light index in statlights
		}
		fMem.Write (uvh);
		fMem.Write(&pCurRawTexCrdData->vTexCoords[0], uvh.numUVs);
	}

	if(0 == pPak->UpdateFile(pFileName, fMem.GetData(), fMem.GetSize()))
		return NSAVE_RESULT::ESUCCESS;
	else
		return NSAVE_RESULT::EPAK_FILE_UPDATE_FAIL;
}

#ifdef WIN64
#pragma warning( pop )									//AMD Port
#endif

DWORD CLMSerializationManager2::GetHashValue( const int iniGLM_ID_UsingTexCoord ) const
{
	std::map<int,RawTexCoordData>::const_iterator itTexCrdData;

	for (itTexCrdData=m_vTexCoords.begin(); itTexCrdData!=m_vTexCoords.end(); itTexCrdData++)
	{
		int iGLM_ID_UsingTexCoord=(*itTexCrdData).first;

		if(iGLM_ID_UsingTexCoord==iniGLM_ID_UsingTexCoord)
		{
			const RawTexCoordData &rCurRawTexCrdData = (*itTexCrdData).second;

			return(rCurRawTexCrdData.m_dwHashValue);
		}
	}	
	return(0x12341234);		// object not found
}

RenderLMData * CLMSerializationManager2::CreateLightmap(const string& strDirPath, int nItem, UINT iWidth, UINT iHeight, const bool cbLoadHDRMaps, const bool cbLoadOcclMaps)
{
	// ---------------------------------------------------------------------------------------------
	// Create a DOT3 Lightmap object
	// ---------------------------------------------------------------------------------------------

	IRenderer *pIRenderer = GetSystem()->GetIRenderer();
	int iColorLerpTex = 0, iHDRColorLerpTex = 0, iDomDirectionTex = 0, iOcclTex = 0;

  int nGPU = pIRenderer->GetFeatures() & RFT_HW_MASK;
	char szPostfix[8];
	sprintf(szPostfix, "%d.dds", nItem);
	//---------------------------------------------------------------------------------------------------
	//----    load precalculated SimpleLightmaps    -----------------------------------------------------
	//---------------------------------------------------------------------------------------------------
	if (GetCVars()->e_light_maps_quality==0 || nGPU == RFT_HW_GF2)
	{
		ITexPic *tp = pIRenderer->EF_LoadTexture((strDirPath + "\\x" + szPostfix).c_str(), FT_LM, FT2_NOANISO, eTT_Base);
		iColorLerpTex = tp->GetTextureID();
		return new RenderLMData(pIRenderer, iColorLerpTex, iHDRColorLerpTex, 0);
	}
  else
	if (GetCVars()->e_light_maps_quality==1)
	{
		//load lightsmaps in DXT3-format
		iColorLerpTex = pIRenderer->EF_LoadTexture((strDirPath + "\\c" + szPostfix).c_str(), FT_LM | FT_CLAMP, FT2_NOANISO, eTT_Base)->GetTextureID();
		iDomDirectionTex = pIRenderer->EF_LoadTexture((strDirPath + "\\d" + szPostfix).c_str(), FT_LM | FT_CLAMP, FT2_NOANISO, eTT_Base)->GetTextureID();
	}
  else
	if (GetCVars()->e_light_maps_quality==2)
	{

		//load lightmaps with high quality
		ITexPic *tp = pIRenderer->EF_LoadTexture((strDirPath + "\\h" + szPostfix).c_str(), FT_LM | FT_CLAMP, FT2_NOANISO, eTT_Base);
		if (tp && tp->IsTextureLoaded())
			iColorLerpTex = tp->GetTextureID();
		else
		{
			SAFE_RELEASE(tp);
			iColorLerpTex = pIRenderer->EF_LoadTexture((strDirPath + "\\c" + szPostfix).c_str(), FT_LM | FT_CLAMP, FT2_NOANISO, eTT_Base)->GetTextureID();
		}
		iDomDirectionTex = pIRenderer->EF_LoadTexture((strDirPath + "\\d" + szPostfix).c_str(), FT_LM | FT_CLAMP, FT2_NOANISO, eTT_Base)->GetTextureID();
		if(cbLoadOcclMaps)
		{
			iOcclTex = pIRenderer->EF_LoadTexture((strDirPath + "\\o" + szPostfix).c_str(), FT_LM | FT_CLAMP, FT2_NOANISO, eTT_Base)->GetTextureID();
		}
    if ((pIRenderer->GetFeatures() & RFT_HW_HDR) && cbLoadHDRMaps)
    {
		  ITexPic *tp = pIRenderer->EF_LoadTexture((strDirPath + "\\r" + szPostfix).c_str(), FT_LM | FT_CLAMP, FT2_NOANISO, eTT_Base);
		  if (tp && tp->IsTextureLoaded())
			  iHDRColorLerpTex = tp->GetTextureID();
      else
  			SAFE_RELEASE(tp);
    }
	} 
	return new RenderLMData(pIRenderer, iColorLerpTex, iHDRColorLerpTex, iDomDirectionTex, iOcclTex);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
RenderLMData * CLMSerializationManager2::CreateLightmap(const char *pszFileName, int nItem, UINT iWidth, UINT iHeight, BYTE *pColorLerp4, BYTE *pHDRColorLerp, BYTE *pDomDirection3, BYTE *pOccl2)
{
	// Create a DOT3 Lightmap object
	IRenderer *pIRenderer = GetSystem()->GetIRenderer();
	int iColorLerpTex = 0, iHDRColorLerpTex = 0, iDomDirectionTex = 0, iOcclTex = 0;

	assert(!IsBadReadPtr(pColorLerp4, sizeof(BYTE) * 4 * iWidth * iHeight));
#ifdef USE_DOT3_ALPHA
	assert(!IsBadReadPtr(pDomDirection3, sizeof(BYTE) * 4 * iWidth * iHeight));
#else
	assert(!IsBadReadPtr(pDomDirection3, sizeof(BYTE) * 3 * iWidth * iHeight));
#endif
	if(pOccl2)
		assert(!IsBadReadPtr(pOccl2, sizeof(BYTE) * 2 * iWidth * iHeight));
	if(pHDRColorLerp)
		assert(!IsBadReadPtr(pHDRColorLerp, sizeof(BYTE) * 4 * iWidth * iHeight));

	char szName[128];
	if (pszFileName)
	{
		sprintf(szName, "$DOT3LM%d$%s", nItem, pszFileName);
		iColorLerpTex = pIRenderer->DownLoadToVideoMemory(pColorLerp4, iWidth, iHeight, eTF_RGBA, eTF_RGBA, 0, false, FILTER_BILINEAR, 0, szName);
	}
	else
		iColorLerpTex = pIRenderer->DownLoadToVideoMemory(pColorLerp4, iWidth, iHeight, eTF_RGBA, eTF_RGBA, 0, false, FILTER_BILINEAR, 0, NULL);
	assert(iColorLerpTex != 0);

#ifndef USE_DOT3_ALPHA
	std::vector<BYTE> vRGBAData;
	vRGBAData.resize(iWidth * iHeight * 4);
	for (UINT i=0; i<iWidth * iHeight; i++)
	{
		memcpy(&vRGBAData[i * 4], &pDomDirection3[i * 3], 3);
		vRGBAData[i * 4 + 3] = 0;
	}
	if (pszFileName)
	{
		sprintf(szName, "$DOT3LMDir%d$%s", nItem, pszFileName);
		iDomDirectionTex = pIRenderer->DownLoadToVideoMemory(&vRGBAData[0], iWidth, iHeight, eTF_RGBA, eTF_RGBA, 0, false, FILTER_BILINEAR, 0, szName);
	}
	else
		iDomDirectionTex = pIRenderer->DownLoadToVideoMemory(&vRGBAData[0], iWidth, iHeight, eTF_RGBA, eTF_RGBA, 0, false, FILTER_BILINEAR, 0, NULL);
#else
	if (pszFileName)
	{
		sprintf(szName, "$DOT3LMDir%d$%s", nItem, pszFileName);
		iDomDirectionTex = pIRenderer->DownLoadToVideoMemory(pDomDirection3, iWidth, iHeight, eTF_RGBA, eTF_RGBA, 0, false, FILTER_BILINEAR, 0, szName);
	}
	else
		iDomDirectionTex = pIRenderer->DownLoadToVideoMemory(pDomDirection3, iWidth, iHeight, eTF_RGBA, eTF_RGBA, 0, false, FILTER_BILINEAR, 0, NULL);
#endif
	assert(iDomDirectionTex != 0); 
  int nGPU = pIRenderer->GetFeatures() & RFT_HW_MASK;
	bool bLow = (nGPU == RFT_HW_GF2);
	if(!bLow && pOccl2 != 0 && (GetCVars()->e_light_maps_quality==2))
	{
		if (pszFileName)
		{
			sprintf(szName, "$DOT3LMOccl%d$%s", nItem, pszFileName);
			iOcclTex = pIRenderer->DownLoadToVideoMemory(pOccl2, iWidth, iHeight, eTF_4444, eTF_4444, 0, false, FILTER_BILINEAR, 0, szName);
		}
		else
			iOcclTex = pIRenderer->DownLoadToVideoMemory(pOccl2, iWidth, iHeight, eTF_4444, eTF_4444, 0, false, FILTER_BILINEAR, 0, NULL);
	}
	if(!bLow && pHDRColorLerp != 0 && (GetCVars()->e_light_maps_quality==2) && (pIRenderer->GetFeatures() & RFT_HW_HDR))
	{
		if (pszFileName)
		{
			sprintf(szName, "$DOT3LMHDR%d$%s", nItem, pszFileName);
			iHDRColorLerpTex = pIRenderer->DownLoadToVideoMemory(pHDRColorLerp, iWidth, iHeight, eTF_RGBA, eTF_RGBA, 0, false, FILTER_BILINEAR, 0, szName);
		}
		else
			iHDRColorLerpTex = pIRenderer->DownLoadToVideoMemory(pHDRColorLerp, iWidth, iHeight, eTF_RGBA, eTF_RGBA, 0, false, FILTER_BILINEAR, 0, NULL);
	}

  if (GetCVars()->e_light_maps_quality==0 || bLow)
  {
    ITexPic *pColorLerpTex = pIRenderer->EF_GetTextureByID(iColorLerpTex);
    byte *pDataLerpTex = pColorLerpTex->GetData32();
    ITexPic *pDomDirectionTex = pIRenderer->EF_GetTextureByID(iDomDirectionTex);
    byte *pDataDirectionTex = pDomDirectionTex->GetData32();
    int Width = pColorLerpTex->GetWidth();
    int Height = pColorLerpTex->GetHeight();
    assert(Width == pDomDirectionTex->GetWidth() && Height == pDomDirectionTex->GetHeight());
    byte *pDst = new byte[Width*Height*4];
    for (int i=0; i<Height; i++)
    {
      byte *pDs = &pDst[i*Width*4];  
      byte *pSr0 = &pDataLerpTex[i*Width*4]; 
      byte *pSr1 = &pDataDirectionTex[i*Width*4]; 
      for (int j=0; j<Width; j++)
      {
        Vec3d v;
		float NdotL = ((float)pSr1[0] - 127.5f) / 127.5f;
        if (NdotL < 0)
          NdotL = 0;
        float lmColor[4];

#ifdef APPLY_COLOUR_FIX
			lmColor[0] = (float)pSr0[0] * (float)pSr1[3] / 255.0f / 255.0f;
			lmColor[1] = (float)pSr0[1] * (float)pSr1[3] / 255.0f / 255.0f;
			lmColor[2] = (float)pSr0[2] * (float)pSr1[3] / 255.0f / 255.0f;
#else
			lmColor[0] = (float)pSr0[0] / 255.0f;
			lmColor[1] = (float)pSr0[1] / 255.0f;
			lmColor[2] = (float)pSr0[2] / 255.0f;
#endif
        lmColor[3] = pSr0[3] / 255.0f;
        float lmIntens = NdotL * lmColor[3] + (1.0f-lmColor[3]);
        pDs[0] = (byte)(lmColor[0] * lmIntens * 255.0f);
        pDs[1] = (byte)(lmColor[1] * lmIntens * 255.0f);
        pDs[2] = (byte)(lmColor[2] * lmIntens * 255.0f);
        pDs[3] = 255;

        pDs += 4;
        pSr0 += 4;
        pSr1 += 4;
      }
    }
    pColorLerpTex->Release();
    pDomDirectionTex->Release();
    if (pszFileName)
    {
      char szCacheName[512];
      sprintf(szCacheName, "$LM%d$%s", nItem, pszFileName);
      iColorLerpTex = pIRenderer->DownLoadToVideoMemory(pDst, Width, Height, eTF_8888, eTF_8888, 0, false, FILTER_BILINEAR, 0, szCacheName);
    }
    else
      iColorLerpTex = pIRenderer->DownLoadToVideoMemory(pDst, Width, Height, eTF_8888, eTF_8888, 0, false, FILTER_BILINEAR, 0, NULL);
    iDomDirectionTex = 0;
  }

	return new RenderLMData(pIRenderer, iColorLerpTex, iHDRColorLerpTex, iDomDirectionTex, iOcclTex);
}

typedef LMStatLightFileHeader LightFileHeader;

bool CLMSerializationManager2::ExportDLights(const char *pszFilePath, const CDLight **ppLights, UINT iNumLights, bool bNewZip) const
{
	// ---------------------------------------------------------------------------------------------
	// Serialize dynamic lights into a file
	// ---------------------------------------------------------------------------------------------
	string strDirName = CryStringUtils::GetParentDirectory<string>(pszFilePath);
	const char* pFileName = pszFilePath + (strDirName.empty()?0:strDirName.length()+1);
	string strPakName = strDirName + "\\" LEVELLM_PAK_NAME;
	GetPak()->ClosePack(strPakName.c_str());
	// make sure the pak file in which this LM data resides is opened
	SetFileAttributes(strPakName.c_str(), FILE_ATTRIBUTE_NORMAL);
	if (!bNewZip)
		GetPak()->ClosePack( strPakName.c_str() );
	ICryArchive_AutoPtr pPak = GetPak()->OpenArchive (strPakName.c_str(), ICryArchive::FLAGS_RELATIVE_PATHS_ONLY|(bNewZip?ICryArchive::FLAGS_CREATE_NEW:0));
	if (!pPak)
		return false;

	LightFileHeader sHeader;
	UINT iCurLight;
	CTempFile fMem;

	if (iNumLights == 0)
	{
		pPak->RemoveFile (pFileName);
		return true;
	}

	assert(!IsBadReadPtr(ppLights, sizeof(CDLight *) * iNumLights));

	sHeader.iNumDLights = iNumLights;
	fMem.Write (sHeader);

	for (iCurLight=0; iCurLight<iNumLights; iCurLight++)
	{
		fMem.Write(*(ppLights[iCurLight]));

		int nFlags2 = ppLights[iCurLight]->m_pLightImage ? ppLights[iCurLight]->m_pLightImage->GetFlags2() : 0;
		fMem.Write (nFlags2);

		ITexPic* pLightImg = ppLights[iCurLight]->m_pLightImage;
		WriteString(pLightImg? pLightImg->GetName() : "" , fMem);

		IShader* pShader = ppLights[iCurLight]->m_pShader;
		WriteString(pShader ? pShader->GetName() : "", fMem);
	}
	return 0 == pPak->UpdateFile (pFileName, fMem.GetData(), fMem.GetSize());
}

bool CLMSerializationManager2::LoadDLights(const char *pszFileName, CDLight **&ppLightsOut, UINT &iNumLightsOut) const
{
	return true;
}

// writes the mips to the given DDS
// gets the highest level MIP from the dds itself (to the moment there must be the 
// MIP written to the end of dds file). Returns the number of mips generated
// (including already given highest-level mip)
unsigned GenerateDDSMips( CTempFile& dds, int wdt, int hgt)
{
	int wd;
	int i, j;
	int num;

	// the size of all MIPs including the highest-level (given) one
	unsigned numBytes = 0;
	int w = wdt;
	int h = hgt;
	while (w || h)
	{
		if (w == 0) w = 1;
		if (h == 0) h = 1;
		numBytes += w * h * 4;
		w >>= 1;
		h >>= 1;
	}
	// the source mip offset: will be changed as the mips are generated sequentially
	int nOffsetSrcMip = dds.GetSize() - wdt*hgt*4;
	if (nOffsetSrcMip < 0)
		return 0;
	dds.SetSize(nOffsetSrcMip + numBytes);
	
	num = 1;                 // number of mips
	byte* src = (byte*)dds.GetData() + nOffsetSrcMip;
	byte* dst = src + wdt*hgt*4; // pointer to next mip
	wd = wdt<<2;			     // width of one row of the source mip
	wdt >>= 1;						 // next mip dimension
	hgt >>= 1;
	while (wdt || hgt)
	{
		if (wdt < 1)
			wdt = 1;
		if (hgt < 1)
			hgt = 1;
		byte* src1 = src;
		byte* dst1 = dst;
		for (i=0; i<hgt; i++)	 // for all rows in the NEW mip
		{
			byte *src2 = src1;	 // running pointer to the previous mip's 2x2 block
			for (j=0; j<wdt; j++)
			{
				assert (dst1 < (byte*)dds.GetData() + dds.GetSize());
				dst1[0] = (src2[0]+src2[4]+src2[wd]+src2[wd+4])>>2;
				dst1[1] = (src2[1]+src2[5]+src2[wd+1]+src2[wd+5])>>2;
				dst1[2] = (src2[2]+src2[6]+src2[wd+2]+src2[wd+6])>>2;
				dst1[3] = (src2[3]+src2[7]+src2[wd+3]+src2[wd+7])>>2;
				dst1 += 4;         // run the target pointer to the next pixel
				src2 += 8;        // run the source pointer to the next 2x2 block
			}
			src1 += wd<<1;
		}
		src = dst;
		dst = dst1;
		num++;
		wd = wdt<<2;			     // width of one row of the source mip
		wdt >>= 1;						 // next mip dimension
		hgt >>= 1;
	}
	assert (dst == (byte*)dds.GetData() + dds.GetSize());
	return num;
}


static CTempFile* g_pDDSTarget;
static unsigned int g_numMips;
static bool DDSCompressCallback(void * data, int miplevel, DWORD size)
{
	g_pDDSTarget->WriteData(data, size);
	return false;
}

// initializes from raw bitmaps
void CLMSerializationManager2::RawLMData::initFromBMP (BitmapEnum t, const void* pSource)
{
	CTempFile* pBMP;
	switch (t) 
	{
		case TEX_COLOR:		pBMP = &m_ColorLerp4; break;
		case TEX_DOMDIR:	pBMP = &m_DomDirection3; break;
		case TEX_OCCL:		pBMP = &m_Occl2; break;
		case TEX_HDR:		  pBMP = &m_HDRColorLerp4; break;
		default: return;
	}

	int nBytesPerPixel = 4;
	int nBytesReservedPerPixel = 4;
	switch(t)
	{
	case TEX_COLOR:		nBytesPerPixel = 4; nBytesReservedPerPixel = 4; break;
#ifdef USE_DOT3_ALPHA
	case TEX_DOMDIR:	nBytesPerPixel = 4; nBytesReservedPerPixel = 4; break;
#else
	case TEX_DOMDIR:	nBytesPerPixel = 3; nBytesReservedPerPixel = 4; break;
#endif
	case TEX_OCCL:		nBytesPerPixel = 2; nBytesReservedPerPixel = 2; break;
	case TEX_HDR: 		nBytesPerPixel = 4; nBytesReservedPerPixel = 4; break;
	}

	pBMP->Clear();
	pBMP->Reserve (sizeof(DWORD) + sizeof(DDS_HEADER) + m_dwWidth*m_dwHeight*nBytesReservedPerPixel);

	DWORD dwMagic = MAKEFOURCC('D','D','S',' ');
	pBMP->Write( dwMagic);

	DDS_HEADER ddsh;
	ZeroStruct(ddsh);
	ddsh.dwSize = sizeof(DDS_HEADER);
	ddsh.dwWidth = m_dwWidth;
	ddsh.dwHeight = m_dwHeight;
	if(t == TEX_DOMDIR || t == TEX_COLOR || t == TEX_HDR)
	{
		ddsh.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP;
		ddsh.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE | DDS_SURFACE_FLAGS_MIPMAP;
		ddsh.dwMipMapCount = 1;
	}
	else
	{
		ddsh.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE;
		ddsh.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE;
		ddsh.dwMipMapCount = 0;
	}
	unsigned nOffsetDDSH = pBMP->GetSize();

	if(t == TEX_DOMDIR || t == TEX_COLOR || t == TEX_HDR)
		ddsh.ddspf = DDSPF_A8R8G8B8;
	else
		ddsh.ddspf = DDSPF_A4R4G4B4;
	pBMP->Write (ddsh);
	if(t == TEX_DOMDIR || t == TEX_COLOR || t == TEX_HDR)
	{
		for (unsigned i = 0; i < m_dwWidth*m_dwHeight; ++i)
		{
			const char* pSrc = ((const char*)pSource)+ i * nBytesPerPixel;
			pBMP->Write<char>(pSrc[2]);
			pBMP->Write<char>(pSrc[1]);
			pBMP->Write<char>(pSrc[0]);
			pBMP->Write<char>((nBytesPerPixel == 4) ? pSrc[3] : (char)0xFF);
		}
		unsigned numMips = GenerateDDSMips(*pBMP, m_dwWidth, m_dwHeight);
		((DDS_HEADER*)((byte*)pBMP->GetData() + nOffsetDDSH))->dwMipMapCount = numMips;
	}
	else
	{
		for (unsigned i = 0; i < m_dwWidth*m_dwHeight; ++i)
		{
			const char* pSrc = ((const char*)pSource)+ i * nBytesPerPixel;//data already preswizzled into BGRA each 4 bit
			pBMP->Write<char>(pSrc[0]);
			pBMP->Write<char>(pSrc[1]);
		}
	}
}


// initializes from files
bool CLMSerializationManager2::RawLMData::initFromDDS (BitmapEnum t, ICryPak* pPak, const string& szFileName)
{
	FILE* f = pPak->FOpen (szFileName.c_str(), "rb");
	if (!f)
		return false;

	AutoFileCloser _AC(f);
	if (pPak->FSeek(f, 0, SEEK_END))
		return false;
	long nLength = pPak->FTell(f);
	if (nLength <= 0)
		return false;

	if (pPak->FSeek(f, 0, SEEK_SET))
		return false;

	CTempFile* pBMP;
	switch (t)
	{
		case TEX_COLOR:		pBMP = &m_ColorLerp4;		break;
		case TEX_DOMDIR:	pBMP = &m_DomDirection3;	break;
		case TEX_OCCL:		pBMP = &m_Occl2;			break;
		case TEX_HDR: 		pBMP = &m_HDRColorLerp4;			break;
		default: return false;
	}
	pBMP->Init(nLength);
	if (pPak->FRead(pBMP->GetData(), nLength, 1, f) != 1)
	{
		pBMP->Init(0);
		return false;
	}
	return true;
}
