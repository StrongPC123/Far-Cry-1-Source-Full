////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cgfconvertor.cpp
//  Version:     v1.00
//  Created:     5/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:		 1/14/2003 :- Taken over by Sergiy Migdalskiy
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AutoFile.h"
#include <io.h>
#include "Dbghelp.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "ResourceCompilerPC.h"
#include "IRCLog.h"
#include "CgfUtils.h"								// CMatEntityNameTokenizer
#include <ICryAnimation.h>	        // AnimSinkEventData
#include "CgfConvertor.h"
#include "CryChunkedFile.h"
#include "ConvertContext.h"
#include "CryCompiledFile.h"
#include "CryBoneHierarchyLoader.h"
#include "CryBoneDesc.h"
#include "RenderMeshBuilder.h"
#include "StencilShadowConnectivity.h"
#include "StencilShadowConnectivityBuilder.h"
#include "SkinDataSources.h"
#include "CrySkinBuilder.h"
#include "CrySkinFull.h"
#include "CrySkinMorph.h"
#include "CrySkinMorphBuilder.h"
#include "CrySkinRigidBasis.h"
#include "CrySkinBasisBuilder.h"
#include "BoneLightBindInfo.h"
#include "StringUtils.h"
#include "StlUtils.h"
#include "iconfig.h"
#include "Controller.h"
extern unsigned g_nFrameID;
#include "CryAnimationInfo.h"

void ValidateHeap()
{
#ifdef WIN64
	//assert (_CrtCheckMemory());
#else
	int nHeapCheck = _heapchk();
	assert (nHeapCheck == _HEAPEMPTY || nHeapCheck == _HEAPOK);
#endif
}

CGFConvertor::CGFConvertor ():
m_fTarget(NULL),
m_pStatCGFCompiler(NULL)
{
	extern IConvertor* NewStatCGFCompiler();
	m_pStatCGFCompiler = NewStatCGFCompiler();
}

CGFConvertor::~CGFConvertor()
{
	clear();
	if (m_pStatCGFCompiler)
		m_pStatCGFCompiler->Release();
}

IConvertor* CGFConvertor::getStatCGFCompiler()
{
	return m_pStatCGFCompiler;
}

bool CGFConvertor::ProcessStatic( ConvertContext &cc )
{
	IConvertor* pConv = getStatCGFCompiler();
	ConvertContext cc2 = cc;

	return pConv->Process(cc2);
}

//////////////////////////////////////////////////////////////////////////
bool CGFConvertor::Process( ConvertContext &cc )
{
	bool bResult = true;
	m_pContext = &cc;
	// Here loading/conversion/bla bla

	g_pLog = cc.pLog;

	CString strSourceFilePath = cc.getSourcePath();

	
	if (CryStringUtils::MatchWildcard(CryStringUtils::toLower(string(cc.sourceFile.GetString())).c_str(), "*_lod?.cgf")
		&&isAnimatedFastCheck(strSourceFilePath.GetString()))
	{
		return true; // we ignore lods of existing animated objects
	}

	if (!isAnimatedFastCheck( strSourceFilePath.GetString() ))
	{
		if (!cc.config->GetAs<bool>("StaticCGF", true))
			return true;
		return ProcessStatic(cc);
	}

	if (!cc.config->GetAs<bool>("AnimatedCGF", true))
		return true;

	try
	{
		if (!LoadLODs())
			return getStatCGFCompiler()->Process(cc);
//		cc.pLog->Log("Converting as an Animated Character: %s", (const char*)cc.sourceFile);
		cc.pLog->Log(" Converting as an Animated Character ...");

		cc.pLog->Log(" BuildRenderMeshes() ...");
		BuildRenderMeshes();
		cc.pLog->Log(" UpdateDeadBodyPhysics() ...");
		UpdateDeadBodyPhysics();

		if (!m_arrLODs.empty())
		{
			m_fTarget = fopen (cc.getOutputPath().GetString(), "wb");
			if (m_fTarget)
			{

				m_Writer.SetFile(m_fTarget);
				WriteHeader();
				WriteBoneInfo();
				WriteMaterials( cc.getSourcePath().GetString() );
				for (unsigned nLOD = 0; nLOD < m_arrLODs.size(); ++nLOD)
				{
					WriteGeometryInfo (nLOD);
					if (nLOD < 2)
					{ // write the LOD 0 and possibly 1 physical geometry for bones
						WriteBoneGeometry (nLOD);
					}
				}
				WriteMorphTargetSet();
				WriteLights();

				{
					CAutoFile fCal (GetCalFilePath().c_str(), "rt");
					if (fCal)
						WriteAnimListWithCAL(fCal);
					else
						WriteAnimListNoCAL();
				}
				WriteSceneProps();

				m_Writer.SetFile(NULL);
				fclose (m_fTarget);
			}
			else
			{
				Log ("Error: cannot open target file %s", (const char*)cc.outputFile.GetString());
				bResult = false;
			}
		}
		else
		{
			Log ("Error: Cannot load cry chunked file");
			bResult = false;
		}
	}
	catch (Error&e)
	{
//		Log ("Error converting \"%s\": \"%s\"", (const char*)cc.sourceFile, e.c_str());
		LogError("%s",e.c_str());
		bResult = false;
	}
#ifndef _DEBUG
	catch (...)
	{
		Log ("Unspecified Error converting \"%s\"", Path::Make(cc.sourceFile,cc.sourceFile).GetString() );
		bResult = false;
	}
#endif
	clear();
	return bResult;
}


// loads the m_arrLODs
// throws error
// if the CGF doesn't have bones, returns false
bool CGFConvertor::LoadLODs()
{
	CString sourceFile = m_pContext->getSourcePath();

	string strFileNoExt = sourceFile;
	CryStringUtils::StripFileExtension(strFileNoExt);
	m_arrLODs.clear();
	m_arrLODBoneMaps.clear();
	for (;;)
	{
		unsigned nLOD = m_arrLODs.size(); // the LOD currently being loaded
		string strFile = sourceFile;
		if (m_arrLODs.empty())
		{
			//LOD 0 - no modifications
			strFile = sourceFile;
		}
		else
		{
			// LOD > 0
			char szLodSuffix[24];
			sprintf (szLodSuffix, "_lod%d.cgf", nLOD);
			strFile = strFileNoExt + szLodSuffix;
		}

		CryChunkedFile_AutoPtr pSource = m_pContext->pRC->LoadCryChunkedFile(strFile.c_str());

		if (!pSource)
		{
			if (GetFileAttributes (strFile.c_str()) != 0xFFFFFFFF)
				throw Error ("Malformed CGF: %s", strFile.c_str());
			else
				break;
		}
		if (!pSource->Bones.numBones())
		{
			if (nLOD)
			{
				LogWarning ("LOD %d doesn't have bones. Converting as static.", nLOD);
				return false;
			}
		}

		if (!pSource->IsBoneInitialPosPresent())
		{
			LogWarning ("The CGF doesn't contain initial pose data and can be converted incorrectly.");
			LogWarning ("Please re-export the CGF with the latest exporter.");
		}

		if (pSource->arrMeshes.empty())
			throw Error("No mesh found");

		if (pSource->arrNames.size() != pSource->Bones.numBones())
			throw Error("There are %d bone names, but %d bones", pSource->arrNames.size(), pSource->Bones.numBones());

		if (pSource->arrMeshes.size() != 1)
			throw Error ("In LOD %d, there are %d meshes. One and Only One mesh per LOD is supported", m_arrLODs.size(), pSource->arrMeshes.size());

		if (!m_arrLODs.empty() && pSource->Bones.numBones() != m_arrLODs[0]->Bones.numBones())
		{
			LogWarning ("In LOD%d, %d bones found. In Master LOD, there are %d bones.", nLOD, pSource->Bones.numBones(), m_arrLODs[0]->Bones.numBones());
			LogWarning ("   Please re-export LOD%d",nLOD);
		}

		m_arrLODs.push_back(pSource);

		RemapBones(nLOD);
	}
	return true;
}


typedef std::map<unsigned, unsigned> UIntUIntMap;
void BuildBoneCtrlMap (const std::vector<CryBoneDesc>& arrBones, UIntUIntMap& mapCtrl)
{
	for (unsigned i = 0; i < arrBones.size(); ++i)
		mapCtrl.insert (UIntUIntMap::value_type(arrBones[i].getControllerId(), i));
}


// returns the number of parents for this bone: 0 for root, 1 for its children, 2 for grandchildren and so on
unsigned NumParents(const CryBoneDesc* pBone)
{
	unsigned nResult = 0;
	for (nResult = 0;pBone->getParentIndexOffset();++nResult)
		pBone = pBone + pBone->getParentIndexOffset();
	return nResult;
}


// remaps, if necessary, the bones from the LOD source to the Master source
// and changes the links in the LOD source so that the boneid's there point to the indices (not IDs!)
// of the bones in the Master Source, not LOD Source
// This assumes that the bone information from the LOD source won't be used at all
// throws an error if there's some unrecognized bone in the LOD source
void CGFConvertor::RemapBones (unsigned nLOD)
{
	CryChunkedFile* pMaster = m_arrLODs[0];
	CryChunkedFile* pLOD = m_arrLODs[nLOD];

	if (m_arrLODBoneMaps.size() < nLOD+1)
		m_arrLODBoneMaps.resize (nLOD+1);
	std::vector<int>& arrBoneMap = m_arrLODBoneMaps[nLOD];

	// this will be the map from the LOD Source to Master Source
	// for bones that have no match, there will be -1's
	unsigned numMasterBones = pMaster->Bones.numBones();
	unsigned numLODBones = pLOD->Bones.numBones();
	arrBoneMap.resize (numMasterBones, -1);

	if (nLOD == 0)
	{
		// the special case - LOD 0 IS a Master
		for (unsigned nBone = 0; nBone < numLODBones; ++nBone)
			arrBoneMap[nBone] = nBone;
		return;
	}

	std::vector<int> arrLODToMaster;
	arrLODToMaster.resize (numLODBones, -1);

	// build the map for CtrlID->bone index
	UIntUIntMap mapCtrlLOD;
	BuildBoneCtrlMap (pLOD->Bones.m_arrBones, mapCtrlLOD);


	for (unsigned nBone = 0; nBone < numMasterBones; ++nBone)
	{
		// try to find the matching bone in LOD
		const CryBoneDesc& rBoneMaster = pMaster->Bones.m_arrBones[nBone];
		UIntUIntMap::iterator it = mapCtrlLOD.find (rBoneMaster.getControllerId());

		if (it != mapCtrlLOD.end()
			/*&& NumParents (&rBoneMaster) == NumParents(&pLOD->Bones.m_arrBones[it->second])*/)
		{
			// the bones match
			arrBoneMap[nBone] = it->second;
			arrLODToMaster[it->second] = nBone;
		}
		else
			LogError ("Cannot map bone %s; the Physics will remain uninitialized", rBoneMaster.getNameCStr());
	}

	// now remap the bone ids
	RemapBoneIndices (pLOD, arrLODToMaster);
}


// remaps the bone indices, throws error if some indices cannot be remapped for some reason
// (e.g. mapping contains -1, i.e. no mapping)
void CGFConvertor::RemapBoneIndices (CryChunkedFile* pLOD, const std::vector<int>& arrMap)
{
	for (unsigned i = 0; i  < pLOD->arrMeshes.size(); ++i)
	{
		CryChunkedFile::MeshDesc& rMeshDesc = pLOD->arrMeshes[i];
		for (unsigned nVertex = 0; nVertex < rMeshDesc.arrVertBinds.size(); ++nVertex)
		{
			CryVertexBinding& rLinks = rMeshDesc.arrVertBinds[nVertex];
			for (unsigned nLink = 0; nLink < rLinks.size(); ++nLink)
			{
				CryLink& rLink = rLinks[nLink];
				if ((unsigned)rLink.BoneID > arrMap.size())
					throw Error ("Cannot convert vertex %u link %u because bone link (%d) is out of range", nVertex, nLink, rLink.BoneID);
				if (arrMap[rLink.BoneID] < 0)
					throw Error ("Cannot convert vertex %u link %u because the bone (%d) wasn't mapped", nVertex, nLink, rLink.BoneID);
				rLink.BoneID = arrMap[rLink.BoneID];
			}
		}
	}
}


// builds the m_arrRenderMeshes array with the CRenderMeshBuilder
// structures containing all the additional info required for leaf buffer creation
void CGFConvertor::BuildRenderMeshes()
{
	try
	{
		m_arrRenderMeshes.resize (m_arrLODs.size());
		// this is the base to add to the material id in each subsequent LOD
		// it reflects the fact that a common array of materials is used for all LODs
		for (unsigned nLOD = 0; nLOD < m_arrLODs.size(); ++nLOD)
		{
			CryChunkedFile* pSource = m_arrLODs[nLOD];
			if (pSource->arrMeshes.size() != 1)
				throw Error ("Unexpected number of meshes (%d) in LOD %d", pSource->arrMeshes.size(), nLOD);

			m_arrRenderMeshes[nLOD].build(&pSource->arrMeshes[0]);
			if (m_arrRenderMeshes[nLOD].m_arrExtFaces.size() != pSource->arrMeshes[0].numFaces())
				LogWarning ("The prepared mesh number of faces doesn't match that of the original CGF mesh. THere are perhaps %d degenerate faces", 
					pSource->arrMeshes[0].numFaces() - m_arrRenderMeshes[nLOD].m_arrExtFaces.size());
			m_arrRenderMeshes[nLOD].addMaterialOffset(getLODMtlOffset(nLOD));
		}
	}
	catch (CRenderMeshBuilder::Error& e)
	{
		throw Error ("Cannot build render meshes: \"%s\"", e.c_str());
	}
}


//////////////////////////////////////////////////////////////////////////
// Updates physics if needed (if there is LOD1, which contains physical data for LOD1 physics
// This is actually hardcoded hack: the dead body physics is kept in LOD 1
// and copied into LOD 0 1st physics slot from 0th physics slot of LOD 1 here
void CGFConvertor::UpdateDeadBodyPhysics()
{
	if (m_arrLODs.size() <= 1)
		return;
	
	CryBoneHierarchyLoader::CryBoneDescArray
		// bones of LOD 0 (main body)
		&arrMainBones = m_arrLODs[0]->Bones.m_arrBones,
		// bones of LOD 1
		&arrSecBones = m_arrLODs[1]->Bones.m_arrBones;

  if (arrMainBones.size() != arrSecBones.size())
		LogWarning ("LOD 1 has %d bones, LOD 0 has %d bones", arrSecBones.size(), arrMainBones.size());

	unsigned numBones = arrMainBones.size();
	for (unsigned nBone = 0; nBone < numBones; ++nBone)
	{
		CryBoneDesc& rBoneMain = arrMainBones[nBone];
		int nLODBone = m_arrLODBoneMaps[1][nBone];
		if (nLODBone < 0)
		{
			LogWarning("bone \"%s\" won't have dead body physics because there's no such bone in LOD1", rBoneMain.getNameCStr());
			rBoneMain.resetPhysics (1);
			continue;
		}
		assert ((unsigned)nLODBone < arrSecBones.size());
		CryBoneDesc& rBoneSec  = arrSecBones[nLODBone];
		//if (!rBoneMain.isEqual(rBoneSec))
		//	throw Error ("LOD 0 and 1 bone #%d are different", nBone);
		rBoneMain.setPhysics (1, rBoneSec.getPhysics(0));
	}
}


CGFConvertor::Error::Error (const char* szFormat, ...)
{
	char szBuffer[0x800];
	va_list arg;
	va_start(arg,szFormat);
	_vsnprintf (szBuffer, sizeof(szBuffer), szFormat, arg);
	va_end(arg);
	this->m_strReason = szBuffer;
}

CGFConvertor::Error::Error (int nCode)
{
	char szBuffer[36];
	sprintf (szBuffer, "Generic Error #%d", nCode);
	this->m_strReason = szBuffer;
}


// releases all the resources
void CGFConvertor::clear()
{
	m_arrLODs.clear();

	m_Writer.SetFile(NULL);
	if (m_fTarget)
	{
		fclose (m_fTarget);
		m_fTarget = NULL;
	}
	
	m_pContext = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool CGFConvertor::GetOutputFile( ConvertContext &cc )
{
	m_pContext = &cc;
	if (isAnimatedFastCheck(cc.getSourcePath().GetString() ))
	{
		//specify output path
		cc.outputFile = Path::ReplaceExtension( cc.sourceFile,"ccg" );
		cc.outputFolder = cc.masterFolder +CString("CCGF_CACHE") + "\\" + cc.outputFolder;
	}
	else
		m_pStatCGFCompiler->GetOutputFile(cc);
	return true;
}

//////////////////////////////////////////////////////////////////////////
int CGFConvertor::GetNumPlatforms() const
{
	return 4;
}

//////////////////////////////////////////////////////////////////////////
Platform CGFConvertor::GetPlatform( int index ) const
{
	switch (index)
	{
	case 0:	return PLATFORM_PC;
	case 1:	return PLATFORM_XBOX;
	//case 2:	return PLATFORM_PS2;
	//case 3:	return PLATFORM_GAMECUBE;
	};
	//assert(0);
	return PLATFORM_UNKNOWN;
}


// writes the data directly into the file
void CGFConvertor::writeRaw (const void* pData, unsigned nSize)
{
	fwrite (pData, nSize, 1, m_fTarget);
}


void CGFConvertor::WriteBoneGeometry (unsigned nLOD)
{
	// construct the corresponding map Bone Index -> Bone Mesh
	// for each bone in the Master, get the corresponding bone in LOD
	// and get its corresponding geometry chunk id. Find the MeshDesc and map it
	typedef std::map<unsigned, CryChunkedFile::MeshDesc*> BoneMeshDescMap;
	BoneMeshDescMap mapBoneMesh;

	CryChunkedFile *pMaster = m_arrLODs[0], *pLOD = m_arrLODs[nLOD];

	for (unsigned nBone = 0; nBone < pMaster->Bones.numBones(); ++nBone)
	{
		int nLODBone = m_arrLODBoneMaps[nLOD][nBone];
		if (nLODBone < 0)
			continue; // we don't export it.

		// the chunk id for the bone mesh
		int nChunkIdMesh = pLOD->Bones.m_arrBones[nLODBone].getPhysGeomId (0);
		CryChunkedFile::MeshDesc* pMesh = pLOD->GetBoneMeshDesc(nChunkIdMesh);
		if (!pMesh)
		{
			if (nChunkIdMesh != -1)
				LogError ("Cannot find bone mesh for bone %d (lod bone %d) LOD %d (mesh chunk id expected: 0x%08X). No physics will be initialized for this bone.", nBone, nLODBone, nLOD, nChunkIdMesh);
			continue;
		}
		mapBoneMesh.insert (BoneMeshDescMap::value_type(nBone, pMesh));
	}

	if (mapBoneMesh.empty())
		return;

	// we export at least one bone in this LOD. Write the header
	m_Writer.AddChunk(CCF_BONE_GEOMETRY);
	CCFBoneGeometry bg;
	bg.numBGBones = mapBoneMesh.size();
	write (bg);

	CCFFileWriter SubChunks (m_fTarget);

	for (BoneMeshDescMap::iterator it = mapBoneMesh.begin(); it != mapBoneMesh.end(); ++it)
	{
		SubChunks.AddChunk (CCF_BG_BONE);
		writeBGBone(nLOD, it->first, it->second);
	}
}


void CGFConvertor::WriteMorphTargetSet ()
{
	CCFMorphTargetSet mts;
	CryChunkedFile::MeshDesc* pMesh = &(m_arrLODs[0]->arrMeshes[0]);
	CRenderMeshBuilder* pRMesh = &m_arrRenderMeshes[0];
	mts.numMorphTargets = pMesh->arrMorphTargets.size();

	if (!mts.numMorphTargets)
		return;
	m_Writer.AddChunk (CCF_MORPH_TARGET_SET);
	write (mts);
	writeMorphTargets(0);
}

void CGFConvertor::writeMorphTargets (unsigned nLOD)
{
	CryChunkedFile* pSource = m_arrLODs[nLOD];
	CryChunkedFile::MeshDesc* pMesh = &pSource->arrMeshes[0];
	unsigned numMorphTargets = pMesh->arrMorphTargets.size();
	CRenderMeshBuilder*pRMesh = &m_arrRenderMeshes[nLOD];

	// build the array of inverse-default-global matrices from the bone infos
	// These matrices are required by the morph skin builder to determine the offsets
	// of morphed vertices relatively to the skin
	// the default global matrices are used to recreate the skin
	unsigned numBones = pSource->Bones.numBones();
	std::vector<Matrix44> arrMatInvDef, arrMatDef;
	arrMatInvDef.resize (numBones);
	arrMatDef.resize (numBones);
	for (unsigned nBone = 0; nBone < numBones; ++nBone)
	{
		arrMatInvDef[nBone] = pSource->Bones.getBoneByIndex(nBone).getInvDefGlobal();
		if (pSource->Bones.hasInitPos ())
			arrMatDef[nBone] = pSource->Bones.getInitPosMatrixByIndex(nBone);
		else
		{
			arrMatDef[nBone] = arrMatInvDef[nBone];
			arrMatDef[nBone].Invert44(); // since the matrix is orthogonal, this can be made simpler..
		}
	}

	CRCSkinVertexSource VertexSource (*pRMesh, *pMesh, &arrMatDef[0]);


	CrySkinMorphBuilder builder (&VertexSource, &arrMatInvDef[0], numBones);

	CCFFileWriter SubChunks (m_fTarget);
	for (unsigned nMorphTarget = 0; nMorphTarget < numMorphTargets; ++nMorphTarget)
	{
		CryChunkedFile::MorphTargetDesc* pMorphTarget = &pMesh->arrMorphTargets[nMorphTarget];
		CrySkinMorph Skin;

		std::vector <SMeshMorphTargetVertex> arrMorphVertices;
		arrMorphVertices.resize (pMorphTarget->numMorphVertices);
		Matrix44 tmNode;
		if (pMesh->pNode)
			tmNode = pMesh->pNode->getWorldTransform();
		for (unsigned i = 0; i < pMorphTarget->numMorphVertices; ++i)
		{
			arrMorphVertices[i] = pMorphTarget->pMorphVertices[i];
			arrMorphVertices[i].ptVertex = tmNode.TransformPointOLD(arrMorphVertices[i].ptVertex);
		}

		builder.initSkinMorph (&arrMorphVertices[0]/*pMorphTarget->pMorphVertices*/, pMorphTarget->numMorphVertices, &Skin);
		SubChunks.AddChunk(CCF_MORPH_TARGET);
		CCFMorphTarget mt;
		mt.numLODs = 1;
		write (mt);
		
		// write the skin - make the buffer(4-byte-aligned)
		std::vector<char>arrBuffer;
		arrBuffer.resize ((Skin.Serialize_PC (true, NULL, 0)+3)&~3);
		if (arrBuffer.empty())
			throw Error ("Can't (pre-)serialize morph target %s", pMorphTarget->strName.c_str());
		unsigned nBytesWritten = Skin.Serialize_PC(true, &arrBuffer[0], arrBuffer.size());
		if (!nBytesWritten)
			throw Error ("Can't serialize morph target %s", pMorphTarget->strName.c_str());
		writeArray (arrBuffer);

		// write the name
		unsigned nLen = pMorphTarget->strName.length() + 1; // write the ternimating null also
		write (pMorphTarget->strName.c_str(), nLen); 
		// pad the name with 0s if necessary
		if (nLen & 3)
		{
			char nPad[4] = {0,0,0,0};
			write(nPad, 4-(nLen&3));
		}
	}
}

void CGFConvertor::writeBGBone(unsigned nLOD, unsigned nBone, CryChunkedFile::MeshDesc* pMesh)
{
	CCFBGBone bgb;
	bgb.nBone = nBone;
	assert (pMesh);
	bgb.numVertices = pMesh->numVertices();
	bgb.numFaces    = pMesh->numFaces();
	write (bgb);

	//write the vertices
	std::vector<Vec3d> arrVertices;
	arrVertices.resize (pMesh->numVertices());
	for (unsigned v = 0; v < pMesh->numVertices(); ++v)
		arrVertices[v] = pMesh->pVertices[v].p;
	writeArray (arrVertices);

	// immediately, write the faces followed by the materials
	std::vector<unsigned char> arrMtls;
	std::vector<CCFIntFace> arrFaces;
	arrFaces.resize (bgb.numFaces);
	arrMtls.resize (bgb.numFaces);
	unsigned nMtlOffset = getLODMtlOffset(nLOD);

	CMatEntityNameTokenizer tok;
	if (nLOD > 0)
	{
		// Remap face material ID by physics name.
		std::vector<MAT_ENTITY>& arrLodMtls = m_arrLODs[nLOD]->arrMtls;
		for (unsigned f = 0; f < bgb.numFaces; ++f)
		{
			arrFaces[f] = pMesh->pFaces[f];
			arrMtls[f] = pMesh->pFaces[f].MatID+nMtlOffset;

			// This can use some optimization.
			MAT_ENTITY& me = arrLodMtls[pMesh->pFaces[f].MatID];
			tok.tokenize(me.name);
			PhysMatIDMap::iterator it = m_physMtlToFaceIdMap.find(tok.szPhysMtl);
			if (it != m_physMtlToFaceIdMap.end())
			{
				int remappedMatId = it->second;
				arrMtls[f] = remappedMatId;
			}
			else
			{
				Log ("Warning: Lod%d Reference physics material %s which is not in the main LOD",(int)nLOD,tok.szPhysMtl );
			}

		}
	}
	else
	{
		for (unsigned f = 0; f < bgb.numFaces; ++f)
		{
			arrFaces[f] = pMesh->pFaces[f];
			arrMtls[f] = pMesh->pFaces[f].MatID+nMtlOffset;
		}
	}
	writeArray (arrFaces);
	writeArray (arrMtls);
}


void CGFConvertor::WriteGeometryInfo (unsigned nLOD)
{
	m_Writer.AddChunk (CCF_GEOMETRY_INFO);
	CCFAnimGeomInfo gi;

	CryChunkedFile* pSource = m_arrLODs[nLOD];
	const CryChunkedFile::MeshDesc& rMeshDesc = pSource->arrMeshes[0];
	const CRenderMeshBuilder& rRendMesh = m_arrRenderMeshes[nLOD];

	gi.numVertices = rMeshDesc.numVertices();
	gi.numExtTangents = rRendMesh.m_arrExtTangents.size();
	gi.numFaces = rMeshDesc.numFaces();
	gi.numIndices = rRendMesh.m_arrIndices.size();
	gi.numPrimGroups = rRendMesh.m_arrPrimGroups.size();
	write (gi);

	CCFFileWriter SubChunks (m_fTarget);
	SubChunks.AddChunk (CCF_GI_PRIMITIVE_GROUPS);
	writeArray (rRendMesh.m_arrPrimGroups);

	SubChunks.AddChunk(CCF_GI_INDEX_BUFFER);
	writeArray (rRendMesh.m_arrIndices);

	SubChunks.AddChunk(CCF_GI_EXT_TO_INT_MAP);
	writeArray (rRendMesh.m_arrExtTangMap);

	std::vector<CryUV> arrExtUVs;
	arrExtUVs.resize (rRendMesh.m_arrExtTangents.size());
	for (unsigned i = 0; i < rRendMesh.m_arrExtTangents.size(); ++i)
	{
		arrExtUVs[i] = rMeshDesc.pUVs[rRendMesh.m_arrExtUVMap[i]];
		// for some reason, the renderer flips v coordinate
		arrExtUVs[i].v = 1- arrExtUVs[i].v;
	}
	SubChunks.AddChunk(CCF_GI_EXT_UVS);
	writeArray (arrExtUVs);

	/*
	SubChunks.AddChunk(CCF_GI_EXT_TANGENTS);
	writeArray (rRendMesh.m_arrExtTangents);
	*/

	SubChunks.AddChunk(CCF_STENCIL_SHADOW_CONNECTIVITY);
	writeShadowConnectivity (nLOD);
	
	SubChunks.AddChunk (CCF_SKIN_VERTICES);
	writeVertexSkin (nLOD);

	SubChunks.AddChunk (CCF_SKIN_NORMALS);
	writeNormalSkin (nLOD);

	SubChunks.AddChunk(CCF_SKIN_TANGENTS);
	writeTangSkin(nLOD);

	SubChunks.AddChunk(CCF_GI_INT_FACES);
	writeIntFaces (nLOD);
}


void CGFConvertor::WriteHeader()
{
	m_Writer.AddChunk(CCF_HEADER_CCG);
	CCFCCGHeader Header;
	Header.numLODs = m_arrLODs.size();
	write (Header);
}


void CGFConvertor::WriteBoneInfo()
{
	CryChunkedFile* pMaster = m_arrLODs[0];
	m_Writer.AddChunk(CCF_BONE_DESC_ARRAY);
	CryBoneHierarchyLoader::CryBoneDescArray
		// bones of LOD 0 (main body)
		&arrBones = pMaster->Bones.m_arrBones;


	CCFBoneDescArrayHeader Header;
	Header.numBones = arrBones.size();
	write (Header);
	std::vector<byte> arrBuf;
	for (unsigned nBone = 0; nBone < arrBones.size(); ++nBone)
	{
		CryBoneDesc& rBone = arrBones[nBone];
		unsigned nSizeBuf = rBone.Serialize(true, NULL, 0);
		arrBuf.resize(nSizeBuf);
	
		if (nSizeBuf != rBone.Serialize(true, &arrBuf[0], nSizeBuf))
			throw Error ("Unexpected error while serializing bone \"%s\"", rBone.getNameCStr());

		writeArray (arrBuf);
		arrBuf.clear();
	}
}






//////////////////////////////////////////////////////////////////////////
// Writes out the material chunk into the compiled cgf
void CGFConvertor::WriteMaterials( const char *inszSrcFileName )
{
	m_Writer.AddChunk(CCF_MATERIALS);
	
	// the set of textures used in all the LODs
//	std::set<string> setTextures;

	// keep the track of how many materials are already written. If some lod uses
	// more materials, the last materials are taken from that lod
	unsigned numMtlsWritten = 0; 
	for (unsigned nLOD = 0; nLOD < m_arrLODs.size(); ++nLOD)
	{
		std::vector<MAT_ENTITY>& arrMtls = m_arrLODs[nLOD]->arrMtls;
		for (; numMtlsWritten < arrMtls.size(); ++numMtlsWritten)
		{
			MAT_ENTITY& me = arrMtls[numMtlsWritten];
			write(me);
			// dump the dependencies

			CMatEntityNameTokenizer tok;
			tok.tokenize(me.name);

			m_physMtlToFaceIdMap[tok.szPhysMtl] = numMtlsWritten;

			m_pContext->pRC->AddDependencyMaterial(inszSrcFileName,tok.szName,tok.szPhysMtl);

//Log ("  %s", me.name);
#define ADD_MAP(MAP) if (me.map_##MAP.name[0]) m_pContext->pRC->AddDependencyFile(inszSrcFileName,me.map_##MAP.name);
			ADD_MAP(a);
			ADD_MAP(d);
			ADD_MAP(o);
			ADD_MAP(b);
			ADD_MAP(s);
			ADD_MAP(g);
			ADD_MAP(detail);
			ADD_MAP(e);
			ADD_MAP(subsurf);
			ADD_MAP(displ);
#undef ADD_MAP
		}
	}

	/*
	Log ("Maps used:");
	for (std::set<string>::iterator it = setTextures.begin(); it != setTextures.end(); ++it)
		Log ("  %s", it->c_str());
	*/
}

void CGFConvertor::WriteVertices (CryChunkedFile* pSource)
{
	m_Writer.AddChunk(CCF_VERTICES);
	unsigned numVertices = pSource->arrMeshes[0].numVertices();
	const CryVertex* pVertices = pSource->arrMeshes[0].pVertices;
	for (unsigned nVertex = 0; nVertex < numVertices; ++nVertex)
		write (pVertices[nVertex].p);

}


void CGFConvertor::WriteNormals (CryChunkedFile* pSource)
{
	m_Writer.AddChunk(CCF_NORMALS);
	unsigned numVertices = pSource->arrMeshes[0].numVertices();
	std::vector<Vec3d>& arrNormals = pSource->arrMeshes[0].arrNormals;
	writeArray (arrNormals);
}


const float fQuantizeTolerance = 4;

float Quantize (float x)
{
	return float(floor (x/fQuantizeTolerance+0.5)*fQuantizeTolerance);
}

// rounds up the x,y,z values of the vector to allow more coarse connectivity
Vec3d Quantize (const Vec3d& v)
{
	return v;
	//return Vec3d(Quantize(v.x), Quantize(v.y),Quantize(v.z));
}

// Creates and serialize the connectivity info for the character
void CGFConvertor::writeShadowConnectivity (unsigned nLOD)
{
	CryChunkedFile* pSource = m_arrLODs[nLOD];

	CStencilShadowConnectivityBuilder Builder;
	if (pSource->arrMeshes.size() != 1)
		throw Error ("Unexpected number of meshes in CGF (%d)", pSource->arrMeshes.size());

	CryChunkedFile::MeshDesc& rMesh = pSource->arrMeshes[0];
	unsigned numFaces = rMesh.numFaces(), numVertices = rMesh.numVertices();
	Builder.ReserveForTriangles(numFaces,numVertices);
	//Builder.SetWeldTolerance(0.1f);
	for (unsigned i = 0; i < numFaces; ++i)
	{
		const CryFace& rFace = rMesh.pFaces[i];
		if (!pSource->DoesMtlCastShadow(rFace.MatID))
			continue;

		// with welding
		unsigned a = rFace.v0, b = rFace.v1, c = rFace.v2;
		Builder.AddTriangleWelded (a, b, c, Quantize(rMesh.pVertices[a].p), Quantize(rMesh.pVertices[b].p), Quantize(rMesh.pVertices[c].p));
	}

	if (Builder.numOrphanedEdges() > 0)
		Log ("Warning: Shadow Connectivity Builder: %u orphaned edges discovered in the model %s", Builder.numOrphanedEdges(), (const char*)m_pContext->sourceFile.GetString());

	IStencilShadowConnectivity* pConnectivity = Builder.ConstructConnectivity();

	if (!pConnectivity)
		throw Error ("Could not construct connectivity");

	unsigned nRequiredSize = pConnectivity->Serialize(true, NULL, 0);

	DWORD dwVertCount,dwTriCount;
	pConnectivity->GetStats(dwVertCount,dwTriCount);
	//Log("StencilEdgeConnectivity Stats: %d/%d Vertices %d/%d Faces. %u bytes.",dwVertCount,numVertices,dwTriCount,numFaces, nRequiredSize);

	std::vector<byte> arrConnectivityPacked;
	arrConnectivityPacked.resize (nRequiredSize);
	unsigned nWrittenBytes = pConnectivity->Serialize(true, &arrConnectivityPacked[0], arrConnectivityPacked.size());
	pConnectivity->Release();
	if (nRequiredSize != nWrittenBytes)
		throw Error ("Could not serialize connectivity: %d of %d bytes were written", nWrittenBytes, nRequiredSize);

	writeArray (arrConnectivityPacked);
}


void CGFConvertor::writeVertexSkin (unsigned nLOD)
{
	CRCSkinVertexSource VSource (m_arrRenderMeshes[nLOD], m_arrLODs[nLOD]->arrMeshes[0]);
	CrySkinBuilder builder (&VSource);
	CrySkinFull VertexSkin;
	builder.initSkinFull (&VertexSkin);

	unsigned nSizeRequired = VertexSkin.Serialize_PC (true, NULL, 0);
	std::vector<byte> arrBuffer;
	arrBuffer.resize (nSizeRequired);
	unsigned nSizeWritten = VertexSkin.Serialize_PC (true, &arrBuffer[0], arrBuffer.size());
	if (nSizeRequired != nSizeWritten)
		throw Error ("Cannot serialize the vertex skin: %d of %d bytes serialized", nSizeWritten, nSizeRequired);
	writeArray (arrBuffer);
}


void CGFConvertor::writeNormalSkin (unsigned nLOD)
{
	CRCSkinNormalSource VSource (m_arrRenderMeshes[nLOD], m_arrLODs[nLOD]->arrMeshes[0], m_arrLODs[0]->Bones.m_arrBones);
	CrySkinBuilder builder (&VSource);
	CrySkinFull NormalSkin;
	builder.initSkinFull (&NormalSkin);

	unsigned nSizeRequired = NormalSkin.Serialize_PC (true, NULL, 0);
	std::vector<byte> arrBuffer;
	arrBuffer.resize (nSizeRequired);
	unsigned nSizeWritten = NormalSkin.Serialize_PC (true, &arrBuffer[0], arrBuffer.size());
	if (nSizeRequired != nSizeWritten)
		throw Error ("Cannot serialize the vertex skin: %d of %d bytes serialized", nSizeWritten, nSizeRequired);
	writeArray (arrBuffer);
}


void CGFConvertor::writeTangSkin (unsigned nLOD)
{
	CryChunkedFile* pSource = m_arrLODs[nLOD];
	CRCSkinVertexSource VSource (m_arrRenderMeshes[nLOD], pSource->arrMeshes[0]);

	// form the array of inverse-default-global matrices for the bones
	std::vector<Matrix44> arrMatInvDef;
	unsigned numBones = m_arrLODs[0]->Bones.numBones();
	arrMatInvDef.resize (numBones);
	unsigned i;
	for (i = 0; i < numBones; ++i)
		arrMatInvDef[i] = m_arrLODs[0]->Bones.getBoneByIndex(i).getInvDefGlobal();

	validateTangents(&VSource.getExtTangent(0), VSource.numExtTangents());

	CrySkinBasisBuilder builder (&VSource, &arrMatInvDef[0], numBones);
	builder.setDestinationInterval(0,0xFFFFFFFF);
	CrySkinRigidBasis TangSkin;
	builder.initRigidBasisSkin (&TangSkin);

	//////////////////////////////////////////////////////////////////////////
	// Validation code - calculates the tangents in the default pose and compares them to the 
	// source tangents supplied by the render mesh
	std::vector<SPipTangentsA> arrBases;
	arrBases.resize (VSource.numExtTangents());
	std::vector<Matrix44> arrMatDef;
	arrMatDef.resize(numBones);
	for (i = 0; i < numBones; ++i)
	{
		arrMatDef[i] = Matrix44::GetInverted44 (arrMatInvDef[i]);
	}
	TangSkin.skin (&arrMatDef[0], &arrBases[0]);

	// the minimal cosine between the calculated and the original tangents
	float fMinCos = 1;
	// compare the calculated tangents with those supplied by the RenderMesh
	for (i = 0; i < VSource.numExtTangents(); ++i)
	{
		float fCos;
		fCos = VSource.getExtTangent(i).tangent * arrBases[i].m_Tangent;
		if (fCos < fMinCos && fabs(fCos)>1e-2 )
			fMinCos = fCos;
		fCos = VSource.getExtTangent(i).tnormal * arrBases[i].m_TNormal;
		if (fCos < fMinCos && fabs(fCos)>1e-2)
			fMinCos = fCos;
		fCos = VSource.getExtTangent(i).binormal * arrBases[i].m_Binormal;
		if (fCos < fMinCos && fabs(fCos)>1e-2)
			fMinCos = fCos;
	}
	Log ("Validated tangents difference: the error is no more than %g degrees", cry_acosf (fMinCos)*180/M_PI);

	//
	//////////////////////////////////////////////////////////////////////////

	unsigned nSizeRequired = TangSkin.Serialize (true, NULL, 0);
	std::vector<byte> arrBuffer;
	arrBuffer.resize (nSizeRequired);
	unsigned nSizeWritten = TangSkin.Serialize (true, &arrBuffer[0], arrBuffer.size());
	if (nSizeRequired != nSizeWritten)
		throw Error ("Cannot serialize the tangent skin: %d of %d bytes serialized", nSizeWritten, nSizeRequired);
	writeArray (arrBuffer);
}

// validate the calculated tangents from which the tangent skin will be formed 
void CGFConvertor::validateTangents (const TangData* pTangents, unsigned numTangents)
{
	unsigned numDegraded = 0, numLeftHanded = 0, numNonOrthogonal = 0, numNonUnit = 0;
	float fMaxEDTB = 0, fMaxEDTN = 0, fMaxEDBN = 0;
	for (unsigned i = 0; i < numTangents; ++i)
	{
		const TangData& td = pTangents[i];
		SBasisProperties Prop (td);
		if (Prop.bMatrixDegraded)
			++numDegraded;
		else
		{
			if (Prop.bLeftHanded)
				++numLeftHanded;
			if (!Prop.isOrthogonal())
				++numNonOrthogonal;
			if (!isUnit(td,1e-3f))
				++numNonUnit;
		}

		fMaxEDTB = max(fMaxEDTB, Prop.fErrorDegTB);
		fMaxEDTN = max(fMaxEDTN, Prop.fErrorDegTN);
		fMaxEDBN = max(fMaxEDBN, Prop.fErrorDegBN);
	}
	if (numNonOrthogonal|numNonUnit)
	{
		LogWarning ("Substandard tangents found:");
		LogWarning ("  %d non-orthogonal, %d non-unit", numNonOrthogonal, numNonUnit);
		LogWarning ("  %d lefthanded, %d degraded", numLeftHanded, numDegraded);
	}
	else
	if (numDegraded)
		Log ("%d degraded tangents. %d of %d are left-handed", numDegraded, numLeftHanded, numTangents);
	Log ("Max deviation from ortho angles: max[Tangent^Binormal]=%.2f, max[Tangent^Normal]=%.2f, max[Binormal^Normal]=%.2f", fMaxEDTB, fMaxEDTN, fMaxEDBN);
}

void CGFConvertor::writeIntFaces (unsigned nLOD)
{
	CryChunkedFile* pSource = m_arrLODs[nLOD];
	std::vector<CCFIntFace> arrFaces;
	std::vector<CCFIntFaceMtlID> arrFaceMtls;
	if (pSource->arrMeshes.size()!=1)
		throw ("Multiple mesh not supported");
	const CryChunkedFile::MeshDesc& rMesh = pSource->arrMeshes[0];
	arrFaces.resize (rMesh.numFaces());
	arrFaceMtls.resize (rMesh.numFaces());
	
	unsigned nOffset = getLODMtlOffset (nLOD);

	for (unsigned nFace = 0; nFace < rMesh.numFaces(); ++nFace)
	{
		const CryFace& rFace = rMesh.pFaces[nFace];
		arrFaces[nFace] = rFace;
		arrFaceMtls[nFace] = rFace.MatID + nOffset;
	}

	writeArray (arrFaces);
	writeArray (arrFaceMtls);
}

// !!!OBSOLETE!!!
// calculates the number of materials used by the previous LODs (< than given)
// this is to offset the mtl numbers of the nLOD to use the range of materials
// belonging to that LOD
// !!!OBSOLETE!!!
unsigned CGFConvertor::getLODMtlOffset (unsigned nLOD)
{
	// WARNING
	// According to the new paradigm, each LOD uses the same materials as LOD0
	return 0;
	/*
	unsigned nOffset = 0, i;
	for (i = 0; i < nLOD && i < m_arrLODs.size(); ++i)
		nOffset += m_arrLODs[i]->arrMtls.size();
	return nOffset;
	*/
}


void CGFConvertor::WriteLights()
{
	std::vector<CBoneLightBindInfo>arrLights;
	CryChunkedFile* pSource = m_arrLODs[0];
	arrLights.resize (pSource->m_numBoneLightBinds);
	unsigned i;
	for (i = 0; i < pSource->m_numBoneLightBinds; ++i)
	{
		SBoneLightBind Bind = pSource->m_pBoneLightBind[i];

		if (Bind.nBoneId >= pSource->Bones.numBones())
			throw Error ("Bone %d out of range in bone-light binding %d", Bind.nBoneId, i);

		Bind.nBoneId = pSource->Bones.mapIdToIndex(Bind.nBoneId);
		// find the light chunk and the chunk of the node of that light
		const LIGHT_CHUNK_DESC* pLightChunk = pSource->GetLightDesc(Bind.nLightChunkId);
		// the node chunk is required to determine the light's transformation
		// (it's stored in the light's node)
		const NODE_CHUNK_DESC* pNodeChunk = pSource->GetObjectNodeDesc(Bind.nLightChunkId)->pDesc;

		if (!pLightChunk)
			throw Error ("Invalid light chunk id %d in light-bone binding %d", Bind.nLightChunkId, i);

		if (!pNodeChunk)
			LogWarning ("No node chunk for light %d", Bind.nLightChunkId);
		
		arrLights[i].load (Bind, *pLightChunk, pNodeChunk?pNodeChunk->name:"_hs_Unknown", 1);
	}

	if (arrLights.empty())
		return;

	std::sort (arrLights.begin(), arrLights.end());

	m_Writer.AddChunk(CCF_CHAR_LIGHT_DESC);
	CCFCharLightDesc Header;
	Header.numLights = arrLights.size();
	Header.numLocalLights = 0;
	for (i = 0; i < arrLights.size(); ++i)
		if (!arrLights[i].isLocal())
			break;
	write (Header);
	std::vector<char> arrData;
	for (i = 0; i < arrLights.size(); ++i)
	{
		CBoneLightBindInfo& rLight = arrLights[i];
		unsigned nRequiredBytes = (rLight.Serialize(true, NULL, 0) + 3)&~3;
		if (arrData.size() < nRequiredBytes)
		{
			arrData.clear();
			arrData.resize (nRequiredBytes);
		}
		unsigned numBytesWritten = rLight.Serialize(true, &arrData[0], nRequiredBytes);
		if (!numBytesWritten || ((numBytesWritten+3)&~3) != nRequiredBytes)
			throw Error ("Cannot write bone light", rLight.getBone());
		write (&arrData[0], nRequiredBytes);
	}
}


// this should retrieve the timestamp of the convertor executable:
// when it was created by the linker, normally. This date/time is used to
// compare with the compiled file date/time and even if the compiled file
// is not older than the source file, it will be recompiled if it's older than the
// convertor
DWORD CGFConvertor::GetTimestamp() const
{
	return GetTimestampForLoadedLibrary(g_hInst);
}

string CGFConvertor::GetCalFilePath()
{
	string strFileNoExt = m_pContext->getSourcePath();
	CryStringUtils::StripFileExtension(strFileNoExt);
	return strFileNoExt+".cal";
}

string CGFConvertor::GetSourceDir()
{
	string strFileNoExt = m_pContext->getSourcePath();
	return CryStringUtils::GetParentDirectory(strFileNoExt);
}

bool CGFConvertor::isAnimatedFastCheck( const char *filename )
{
	bool bStaticCGF = false;
	m_pContext->config->Get("StaticCGF", bStaticCGF);
	if(bStaticCGF)
		return false;

	if (stricmp(CryStringUtils::FindExtension(filename), "cgf"))
		return false; // only cgf files can be animated cgfs

	string strFileNoExt = filename;
	CryStringUtils::StripFileExtension(strFileNoExt);

	/*
	CString tempfile = Path::GetFileName(filename);
	tempfile = tempfile.MakeLower();
	if (tempfile.GetLength() > 5) {}
	*/

	if (GetFileAttributes((strFileNoExt+".cal").c_str())!= 0xFFFFFFFF)
		return true;

	__finddata64_t fi;
	int nFind = _findfirst64 ((strFileNoExt+"_*.caf").c_str(), &fi);
	if (nFind != -1)
	{
		_findclose (nFind);
		return true;
	}
	return false;
}

// Reads the CAL file or the list of caf files in the same directory with the corresponding name,
// and writes the animation list chunk with the animation names, file paths and compiled directives

void CGFConvertor::WriteAnimListWithCAL(FILE* fCal)
{
	assert (fCal);
	// Load cal file and load animations from animations folder
	// make anim folder name
	// make search path
	string strFileNameNoExt = m_pContext->getSourcePath();
	CryStringUtils::StripFileExtension(strFileNameNoExt);
	string strDirName = CryStringUtils::GetParentDirectory(strFileNameNoExt);
	string strAnimDirName = CryStringUtils::GetParentDirectory(strDirName, 2) + "\\animations";
	Vec3d vOffset(0,0,0); // model offset

	// the flags applicable to the currently being loaded animation
	unsigned nAnimFlags = 0;

	m_Writer.AddChunk(CCF_ANIM_SCRIPT);

	CCFFileWriter SubChunks (m_fTarget);

	for (int i = 0; fCal && !feof(fCal); ++i)
	{
		char sBuffer[0x200]="";
		fgets(sBuffer,sizeof(sBuffer),(FILE*)fCal);
		char*szAnimName;
		char*szFileName;

		if(sBuffer[0] == '/' || sBuffer[0]=='\r' || sBuffer[0]=='\n' || sBuffer[0]==0)
			continue;

		szAnimName = strtok (sBuffer, " \t\n\r=");
		if (!szAnimName)
			continue;
		szFileName = strtok(NULL, " \t\n\r=");
		if (!szFileName || szFileName[0] == '?')
		{
			//RegisterDummyAnimation(szAnimName);
			SubChunks.AddChunk(CCF_ANIM_SCRIPT_DUMMYANIM);
			writeString (szAnimName);
			continue;
		}

		if (szAnimName[0] == '/' && szAnimName[1] == '/')
			continue; // comment

		{ // remove firsrt '\' and replace '/' with '\'
			while(szFileName[0]=='/' || szFileName[0]=='\\')
				memmove(szFileName,szFileName+1,sizeof(szFileName)-1);

			for(char * p = szFileName+strlen(szFileName); p>=szFileName; p--)
				if(*p == '/')
					*p = '\\';
		}

		// process the possible directives
		if (szAnimName[0] == '$')
		{
			const char* szDirective = szAnimName + 1;
			if (!stricmp(szDirective, "AnimationDir")
				||!stricmp(szDirective, "AnimDir")
				||!stricmp(szDirective, "AnimationDirectory")
				||!stricmp(szDirective, "AnimDirectory"))
			{
				// delete the trailing slashes
				for (char* p = szFileName+strlen(szFileName)-1; p >= szFileName && *p == '\\' || *p == '/'; --p)
					*p = '\0';
				if (*szFileName)
				{
					SubChunks.AddChunk(CCF_ANIM_SCRIPT_ANIMDIR);
					writeString(szFileName);

					strAnimDirName = strDirName + "\\" + szFileName;
				}
			}
			else
			if (!stricmp (szDirective, "ModelOffsetX"))
			{
				float fValue;
				if (sscanf (szFileName, "%f", &fValue) != 1)
					LogWarning("CAL Directive ModelOffsetX %s couldn't be interpreted", szFileName);
				else
					vOffset.x = fValue;
			}
			else
			if (!stricmp (szDirective, "ModelOffsetY"))
			{
				float fValue;
				if (sscanf (szFileName, "%f", &fValue) != 1)
					LogWarning("CAL Directive ModelOffsetY %s couldn't be interpreted", szFileName);
				else
					vOffset.y = fValue;
			}
			else
			if (!stricmp (szDirective, "ModelOffsetZ"))
			{
				float fValue;
				if (sscanf (szFileName, "%f", &fValue) != 1)
					LogWarning("CAL Directive ModelOffsetZ %s couldn't be interpreted", szFileName);
				else
					vOffset.z = fValue;
			}
			else
			if (!stricmp(szDirective, "AutoUnload"))
			{
				switch (CryStringUtils::toYesNoType(szFileName))
				{
				case CryStringUtils::nYNT_Yes:
					nAnimFlags &= ~GlobalAnimation::FLAGS_DISABLE_AUTO_UNLOAD;
					break;
				case CryStringUtils::nYNT_No:
					nAnimFlags |= GlobalAnimation::FLAGS_DISABLE_AUTO_UNLOAD;
					break;
				default:
					LogWarning("Invalid option for AutoUnload directive (must be yes or no) in file %s.cal", strFileNameNoExt.c_str());
					break;
				}
			}
			else
			if (!stricmp(szDirective, "DelayLoad"))
			{
				switch (CryStringUtils::toYesNoType(szFileName))
				{
				case CryStringUtils::nYNT_Yes:
					nAnimFlags &= ~GlobalAnimation::FLAGS_DISABLE_DELAY_LOAD;
					break;
				case CryStringUtils::nYNT_No:
					nAnimFlags |= GlobalAnimation::FLAGS_DISABLE_DELAY_LOAD;
					break;
				default:
					LogWarning("Invalid option for DelayLoad directive (must be yes or no) in file %s.cal", strFileNameNoExt.c_str());
					break;
				}
			}
			else
				LogWarning("Unknown directive %s", szDirective);
			continue;
		}

		WriteAnimInfo (SubChunks, szAnimName, strAnimDirName + "\\" + szFileName, nAnimFlags);
	}

	if (vOffset != Vec3d(0,0,0))
	{
		SubChunks.AddChunk(CCF_ANIM_SCRIPT_MODELOFFSET);
		write (vOffset);
	}
}

void CGFConvertor::WriteAnimInfo (CCFFileWriter &rSubChunks, const char* szAnimName, const string& strFilePath, unsigned nFlags)
{
	CCFAnimInfo animInfo;
	LoadAnimInfo (strFilePath, animInfo);
	animInfo.nAnimFlags = nFlags;

	rSubChunks.AddChunk(CCF_ANIM_SCRIPT_ANIMINFO);
	write (animInfo);
	writeString (szAnimName);
	writeString (strFilePath.c_str());
}

void CGFConvertor::WriteAnimListNoCAL()
{
	m_Writer.AddChunk(CCF_ANIM_SCRIPT);

	CCFFileWriter SubChunks (m_fTarget);

	// load the default pose first
	string strFileNoExt = m_pContext->getSourcePath();
	CryStringUtils::StripFileExtension(strFileNoExt);
	string strDirName = CryStringUtils::GetParentDirectory (strFileNoExt);
	string strDefaultPose = (strFileNoExt + "_default.caf").c_str();

	if (GetFileAttributes(strDefaultPose.c_str()) != -1)
		// we need default animation immediately, but unlikely we'll need it in the future (so we can unload it)
		WriteAnimInfo(SubChunks, "default", strDefaultPose, GlobalAnimation::FLAGS_DEFAULT_ANIMATION);

	// the name of the base cgf (before the underscore) + 1
	unsigned nBaseNameLength = strFileNoExt.length() - strDirName.length();
	__finddata64_t fd;
	long hFF = _findfirst64 ((strFileNoExt + "_*.caf").c_str(), &fd);
	if (hFF != -1)
	{
		do
		{
			string strFileName = strDirName + "\\" + fd.name;

			if(!stricmp(strDefaultPose.c_str(), strFileName.c_str()))
				// skip the default pose as it has already been loaded
				continue;

			//if (!stricmp(FindExtension(fileinfo.name), "caf")) // actually ,according to the search mask, this should be met automatically
			char* szExtension = CryStringUtils::StripFileExtension(fd.name);
			assert (!stricmp(szExtension, "caf"));
			assert (strlen(fd.name) > nBaseNameLength);

			const char* szAnimName = fd.name + nBaseNameLength;
			WriteAnimInfo (SubChunks, szAnimName, strFileName, 0);
		}
		while (_findnext64( hFF, &fd) != -1);
		_findclose (hFF);
	}
}

inline string MakeString (const char* szBuf, size_t nSizeBuf)
{
	return string (szBuf, CryStringUtils::strnlen(szBuf, szBuf+nSizeBuf));
}


void CGFConvertor::WriteSceneProps()
{
	m_Writer.AddChunk(CCF_USER_PROPERTIES);
	typedef std::map<string,string>PropMap;
	PropMap mapProps;
	for (CryChunkedFile_AutoArray::reverse_iterator itLod = m_arrLODs.rbegin(); itLod != m_arrLODs.rend(); ++itLod)
	{
		for (unsigned n = 0; n < (*itLod)->numSceneProps; ++n)
		{
			const SCENEPROP_ENTITY& Prop = (*itLod)->pSceneProps[n];
			mapProps.insert (PropMap::value_type(MakeString(Prop.name,sizeof(Prop.name)), MakeString(Prop.value, sizeof(Prop.value))));
		}
	}

	unsigned nWrittenBytes = 0;
	for (PropMap::iterator it = mapProps.begin(); it != mapProps.end(); ++it)
	{
		nWrittenBytes += writeString (it->first);
		nWrittenBytes += writeString (it->second);
	}

	// pad with up to 3 bytes to align on 4-byte boundary
	if (nWrittenBytes & 3)
	{
		char szPad[4] = {'\0','\0','\0','\0'};
		write (szPad, 4-(nWrittenBytes & 3));
	}
}

// loads the given animation info into the given structure
// returns false upon failure
bool CGFConvertor::LoadAnimInfo (const string& strFilePath, CCFAnimInfo &Anim)
{
	CChunkFileReader Reader;
	Anim.init();
	if (!Reader.open (strFilePath))
	{
		if (!(Anim.nAnimFlags & GlobalAnimation::FLAGS_DISABLE_LOAD_ERROR_LOG))
			LogError ("LoadAnimInfo: File loading error: %s, last error is: %s", strFilePath.c_str(), Reader.getLastError());
		return false;
	}

	// check the file header for validity
	const FILE_HEADER& fh = Reader.getFileHeader();

	if(fh.Version != AnimFileVersion || fh.FileType != FileType_Anim) 
	{
		LogError("LoadAnimInfo: file version error or not an animation file: %s", strFilePath.c_str());
		return false;
	}

	bool bTimingChunkFound = false;
	for (int nChunk = 0; nChunk < Reader.numChunks (); ++nChunk)
	{
		// this is the chunk header in the chunk table at the end of the file
		const CHUNK_HEADER& chunkHeader = Reader.getChunkHeader(nChunk);
		// this is the chunk raw data, starts with the chunk header/descriptor structure
		const void* pChunk = Reader.getChunkData (nChunk);
		unsigned nChunkSize = Reader.getChunkSize (nChunk);

		switch (chunkHeader.ChunkType)
		{
		case ChunkType_Controller:
			++Anim.numControllers;
			break;

		case ChunkType_Timing:
			{
				// memorize the timing info
				const TIMING_CHUNK_DESC* pTimingChunk = static_cast<const TIMING_CHUNK_DESC*> (pChunk);
				Anim.nTicksPerFrame  = pTimingChunk->TicksPerFrame;
				Anim.fSecsPerTick    = pTimingChunk->SecsPerTick;
				Anim.nRangeStart     = pTimingChunk->global_range.start;
				Anim.nRangeEnd       = pTimingChunk->global_range.end;
				bTimingChunkFound = true;
			}
			break;
		}
	}

	if (!bTimingChunkFound)
	{
		LogError("LoadAnimInfo: file doesn't contain Timing chunk: %s", strFilePath.c_str());
		return false;
	}
	return true;
}