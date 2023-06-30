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
#include <io.h>
#include "Dbghelp.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "ResourceCompilerPC.h"
#include "IRCLog.h"

#include "GC_CgfConverter.h"
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



extern void ValidateHeap();
typedef std::map<unsigned, unsigned> UIntUIntMap;
extern void BuildBoneCtrlMap (const std::vector<CryBoneDesc>& arrBones, UIntUIntMap& mapCtrl);
extern unsigned NumParents(const CryBoneDesc* pBone);
extern Vec3d Quantize (const Vec3d& v);
extern float Quantize (float x);





IConvertor* GC_CGFConvertor::getStatCGFCompiler()
{
	extern IConvertor* NewStatCGFCompiler();
	if (!m_pStatCGFCompiler)
		m_pStatCGFCompiler = NewStatCGFCompiler();
	return m_pStatCGFCompiler;
}

//////////////////////////////////////////////////////////////////////////
bool GC_CGFConvertor::Process( ConvertContext &cc )
{
	bool bResult = true;
	m_pContext = &cc;
	// Here loading/conversion/bla bla

	g_pLog = cc.pLog;

	if (!isAnimationFastCheck())
		return getStatCGFCompiler()->Process(cc);

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
				
				WriteMaterials ();
		//		WriteVertices();
				
				for (unsigned nLOD = 0; nLOD < m_arrLODs.size(); ++nLOD) {
					WriteGeometryInfo (nLOD);
					if (nLOD < 2)	WriteBoneGeometry (nLOD);
				}
				WriteMorphTargetSet();
				WriteLights();


				// write the LOD 0 and possibly 1 physical geometry for bones
				for (unsigned nLOD = 0; nLOD < m_arrLODs.size(); ++nLOD)
					m_Writer.SetFile(NULL);


				fclose (m_fTarget);
			}
			else
			{
				Log ("Error: cannot open target file %s", (cc.getOutputPath().GetString()));
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
		Log ("Error converting \"%s\": \"%s\"", cc.sourceFile.GetString(), e.c_str());
		bResult = false;
	}
#ifndef _DEBUG
	catch (...)
	{
		Log ("Unspecified Error converting \"%s\"", cc.sourceFile.GetString());
		bResult = false;
	}
#endif
	clear();
	return bResult;
}


// loads the m_arrLODs
// throws error
// if the CGF doesn't have bones, returns false
bool GC_CGFConvertor::LoadLODs()
{
	CString sourceFile = m_pContext->getSourcePath();
	string strFileNoExt = sourceFile;
	CryStringUtils::StripFileExtension(strFileNoExt);
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
			LogWarning ("  Please re-export the CGF with the latest exporter.");
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






// remaps, if necessary, the bones from the LOD source to the Master source
// and changes the links in the LOD source so that the boneid's there point to the indices (not IDs!)
// of the bones in the Master Source, not LOD Source
// This assumes that the bone information from the LOD source won't be used at all
// throws an error if there's some unrecognized bone in the LOD source
void GC_CGFConvertor::RemapBones (unsigned nLOD)
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
			Log("Cannot map bone %s; the Physics will remain uninitialized", rBoneMaster.getNameCStr());
	}

	// now remap the bone ids
	RemapBoneIndices (pLOD, arrLODToMaster);
}


// remaps the bone indices, throws error if some indices cannot be remapped for some reason
// (e.g. mapping contains -1, i.e. no mapping)
void GC_CGFConvertor::RemapBoneIndices (CryChunkedFile* pLOD, const std::vector<int>& arrMap)
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
void GC_CGFConvertor::BuildRenderMeshes()
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
void GC_CGFConvertor::UpdateDeadBodyPhysics()
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


GC_CGFConvertor::Error::Error (const char* szFormat, ...)
{
	char szBuffer[0x800];
	va_list arg;
	va_start(arg,szFormat);
	_vsnprintf (szBuffer, sizeof(szBuffer), szFormat, arg);
	va_end(arg);
	this->m_strReason = szBuffer;
}

GC_CGFConvertor::Error::Error (int nCode)
{
	char szBuffer[36];
	sprintf (szBuffer, "Generic Error #%d", nCode);
	this->m_strReason = szBuffer;
}


// releases all the resources
void GC_CGFConvertor::clear()
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
bool GC_CGFConvertor::GetOutputFile( ConvertContext &cc )
{
	//specify output path
	cc.outputFile = Path::ReplaceExtension( cc.sourceFile,"gc_ccg" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
int GC_CGFConvertor::GetNumPlatforms() const
{
	return 4;
}

//////////////////////////////////////////////////////////////////////////
Platform GC_CGFConvertor::GetPlatform( int index ) const
{
	switch (index)
	{
		case 0:	return PLATFORM_PC;
		case 1:	return PLATFORM_XBOX;
		case 2:	return PLATFORM_PS2;
		case 3:	return PLATFORM_GAMECUBE;
	};
	//assert(0);
	return PLATFORM_UNKNOWN;
}


// writes the data directly into the file
void GC_CGFConvertor::writeRaw (const void* pData, unsigned nSize)
{
	fwrite ( 
		(pData), 
		(nSize), 
		(1), 
		(m_fTarget)
		);
}


void GC_CGFConvertor::WriteBoneGeometry (unsigned nLOD)
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
	CCFBoneGeometry bg1;
	bg1.numBGBones = SWAP32( mapBoneMesh.size() );
	write (bg1);

	GC_CCFFileWriter SubChunks (m_fTarget);

	for (BoneMeshDescMap::iterator it = mapBoneMesh.begin(); it != mapBoneMesh.end(); ++it)
	{
		SubChunks.AddChunk (CCF_BG_BONE);
		writeBGBone(nLOD, it->first, it->second);
	}

}


void GC_CGFConvertor::WriteMorphTargetSet ()
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

void GC_CGFConvertor::writeMorphTargets (unsigned nLOD)
{
	CryChunkedFile* pSource = m_arrLODs[nLOD];
	CryChunkedFile::MeshDesc* pMesh = &pSource->arrMeshes[0];
	unsigned numMorphTargets = pMesh->arrMorphTargets.size();
	CRenderMeshBuilder*pRMesh = &m_arrRenderMeshes[nLOD];

	CRCSkinVertexSource VertexSource (*pRMesh, *pMesh);

	// build the array of inverse-default-global matrices from the bone infos
	// These matrices are required by the morph skin builder to determine the offsets
	// of morphed vertices relatively to the skin
	unsigned numBones = pSource->Bones.numBones();
	std::vector<Matrix44> arrMatInvDef;
	arrMatInvDef.resize (numBones);
	for (unsigned nBone = 0; nBone < numBones; ++nBone)
		arrMatInvDef[nBone] = pSource->Bones.getBoneByIndex(nBone).getInvDefGlobal();

	CrySkinMorphBuilder builder (&VertexSource, &arrMatInvDef[0], numBones);

	GC_CCFFileWriter SubChunks (m_fTarget);
	for (unsigned nMorphTarget = 0; nMorphTarget < numMorphTargets; ++nMorphTarget)
	{
		CryChunkedFile::MorphTargetDesc* pMorphTarget = &pMesh->arrMorphTargets[nMorphTarget];
		CrySkinMorph Skin;

		/*std::vector <SMeshMorphTargetVertex> arrMorphVertices;
		arrMorphVertices.resize (pMorphTarget->numMorphVertices);
		Matrix tmNode;
		if (pMesh->pNode)
		tmNode = pMesh->pNode->getWorldTransform();
		for (unsigned i = 0; i < pMorphTarget->numMorphVertices; ++i)
		{
		arrMorphVertices[i] = pMorphTarget->pMorphVertices[i];
		arrMorphVertices[i].ptVertex = tmNode.TransformPoint(arrMorphVertices[i].ptVertex);
		}*/

		builder.initSkinMorph (pMorphTarget->pMorphVertices, pMorphTarget->numMorphVertices, &Skin);
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

void GC_CGFConvertor::writeBGBone(unsigned nLOD, unsigned nBone, CryChunkedFile::MeshDesc* pMesh)
{
	CCFBGBone bgb;
	bgb.nBone = nBone;
	assert (pMesh);
	bgb.numVertices = pMesh->numVertices();
	bgb.numFaces    = pMesh->numFaces();
	//write (bgb);

	CCFBGBone bgb0;
	bgb0.nBone			=	SWAP32( bgb.nBone );
	bgb0.numVertices =SWAP32( bgb.numVertices );
	bgb0.numFaces    =SWAP32( bgb.numFaces );
	write (bgb0);


	//write the vertices
	std::vector<Vec3d> arrVertices;
	arrVertices.resize (pMesh->numVertices());

	std::vector<Vec3d> arrVertices_GC;
	arrVertices_GC.resize (pMesh->numVertices());
	for (unsigned v = 0; v < pMesh->numVertices(); ++v){ 
		arrVertices[v]			= pMesh->pVertices[v].p;
		arrVertices_GC[v].x = FSWAP32(pMesh->pVertices[v].p.x);
		arrVertices_GC[v].y = FSWAP32(pMesh->pVertices[v].p.y);
		arrVertices_GC[v].z = FSWAP32(pMesh->pVertices[v].p.z);
	}
	writeArray (arrVertices_GC);



	// immediately, write the faces followed by the materials
	std::vector<unsigned char> arrMtls;
	arrMtls.resize (bgb.numFaces);
	std::vector<CCFIntFace> arrFaces;
	arrFaces.resize (bgb.numFaces);



	std::vector<unsigned char> arrMtls_GC;
	arrMtls_GC.resize (bgb.numFaces);
	std::vector<CCFIntFace> arrFaces_GC;
	arrFaces_GC.resize (bgb.numFaces);

	unsigned nMtlOffset = getLODMtlOffset(nLOD);
	for (unsigned f = 0; f < bgb.numFaces; ++f)
	{
		arrFaces[f]					= pMesh->pFaces[f];
		arrMtls[f]					= pMesh->pFaces[f].MatID+nMtlOffset;

		arrFaces_GC[f].v[0]	= SWAP16(pMesh->pFaces[f].v0);
		arrFaces_GC[f].v[1]	= SWAP16(pMesh->pFaces[f].v1);
		arrFaces_GC[f].v[2]	= SWAP16(pMesh->pFaces[f].v2);
		arrMtls_GC[f]				= pMesh->pFaces[f].MatID+nMtlOffset;
	}
	writeArray (arrFaces_GC);
	writeArray (arrMtls_GC);
}


void GC_CGFConvertor::WriteGeometryInfo (unsigned nLOD)
{
	m_Writer.AddChunk (CCF_GEOMETRY_INFO);
	CCFAnimGeomInfo gi;

	CryChunkedFile* pSource = m_arrLODs[nLOD];
	const CryChunkedFile::MeshDesc& rMeshDesc = pSource->arrMeshes[0];
	const CRenderMeshBuilder& rRendMesh = m_arrRenderMeshes[nLOD];

	gi.numVertices		= SWAP32(rMeshDesc.numVertices());
	gi.numExtTangents = SWAP32(rRendMesh.m_arrExtTangents.size());
	gi.numFaces				= SWAP32(rMeshDesc.numFaces());
	gi.numIndices			= SWAP32(rRendMesh.m_arrIndices.size());
	gi.numPrimGroups	= SWAP32(rRendMesh.m_arrPrimGroups.size());
	write (gi);

	GC_CCFFileWriter SubChunks (m_fTarget);
	SubChunks.AddChunk (CCF_GI_PRIMITIVE_GROUPS);

// array of these structures represents the groups of indices in the index buffer:
// each group has its own material id and number of elements (indices, i.e. number of faces * 3 in case of strip stripification)
/*struct CCFMaterialGroup
{
	// material index in the original indexation of CGF
	unsigned nMaterial;
	// the first index in the final index buffer
	unsigned short nIndexBase;
	// number of indices in the final index buffer
	unsigned short numIndices;
};*/


	int mc=rRendMesh.m_arrPrimGroups.size();
	for (int x=0; x<mc; x++){
		CCFMaterialGroup mg;
		mg.nMaterial		=	SWAP32(rRendMesh.m_arrPrimGroups[x].nMaterial);
		mg.nIndexBase		=	SWAP16(rRendMesh.m_arrPrimGroups[x].nIndexBase);
		mg.numIndices		=	SWAP16(rRendMesh.m_arrPrimGroups[x].numIndices);
		write (mg);
	}

	/*for (int x=0; x<6; x++){ 
		rRendMesh.m_arrPrimGroups[x].nMaterial	=	SWAP32(rRendMesh.m_arrPrimGroups[x].nMaterial);
		rRendMesh.m_arrPrimGroups[x].nIndexBase	=	0;//SWAP16(rRendMesh.m_arrPrimGroups[x].nIndexBase);
		rRendMesh.m_arrPrimGroups[x].numIndices	=	SWAP16(rRendMesh.m_arrPrimGroups[x].numIndices);
	}*/
	//writeArray (rRendMesh.m_arrPrimGroups);




	SubChunks.AddChunk(CCF_GI_INDEX_BUFFER);
	int ic=rRendMesh.m_arrIndices.size();
	for (int x=0; x<ic; x++){
		unsigned short index;
		index		=	SWAP16(rRendMesh.m_arrIndices[x]);
		write (index);
	}
  //writeArray (rRendMesh.m_arrIndices);


	SubChunks.AddChunk(CCF_GI_EXT_TO_INT_MAP);
	int xic=rRendMesh.m_arrExtTangMap.size();
	for (int x=0; x<xic; x++){
		unsigned short index;
		index		=	SWAP16(rRendMesh.m_arrExtTangMap[x]);
		write (index);
	}
  //writeArray (rRendMesh.m_arrExtTangMap);


	std::vector<CryUV> arrExtUVs;
	arrExtUVs.resize (rRendMesh.m_arrExtTangents.size());
	for (unsigned i = 0; i < rRendMesh.m_arrExtTangents.size(); ++i)
	{
		arrExtUVs[i] = rMeshDesc.pUVs[rRendMesh.m_arrExtUVMap[i]];
		// for some reason, the renderer flips v coordinate
		arrExtUVs[i].v = 1- arrExtUVs[i].v;

		arrExtUVs[i].u=FSWAP32(arrExtUVs[i].u);
		arrExtUVs[i].v=FSWAP32(arrExtUVs[i].v);
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


void GC_CGFConvertor::WriteHeader()
{
	m_Writer.AddChunk(CCF_HEADER_CCG);
	CCFCCGHeader Header;
	Header.numLODs = m_arrLODs.size();
	write (Header);
}


void GC_CGFConvertor::WriteBoneInfo()
{
	CryChunkedFile* pMaster = m_arrLODs[0];
	m_Writer.AddChunk(CCF_BONE_DESC_ARRAY);
	CryBoneHierarchyLoader::CryBoneDescArray
		// bones of LOD 0 (main body)
		&arrBones = pMaster->Bones.m_arrBones;

	CCFBoneDescArrayHeader Header;
	Header.numBones = arrBones.size();

	 CCFBoneDescArrayHeader Header_GC;
	 Header_GC.numBones = SWAP32(arrBones.size());
	 write (Header_GC);

	std::vector<byte> arrBuf;

	for (unsigned nBone = 0; nBone < arrBones.size(); ++nBone)
	{
		CryBoneDesc& rBone			=	arrBones[nBone];
		rBone.m_numChildren			=	SWAP32(rBone.m_numChildren);
		rBone.m_nOffsetChildren	=	SWAP32(rBone.m_nOffsetChildren);
		rBone.m_nOffsetParent		= SWAP32(rBone.m_nOffsetParent);
		rBone.m_nLimbId					=	SWAP32(rBone.m_nLimbId);
		rBone.m_nControllerID		=	SWAP32(rBone.m_nControllerID);
		
		rBone.m_matInvDefGlobal(0,0)	=	FSWAP32(rBone.m_matInvDefGlobal(0,0));
		rBone.m_matInvDefGlobal(0,1)	=	FSWAP32(rBone.m_matInvDefGlobal(0,1));
		rBone.m_matInvDefGlobal(0,2)	=	FSWAP32(rBone.m_matInvDefGlobal(0,2));
		rBone.m_matInvDefGlobal(0,3)	=	FSWAP32(rBone.m_matInvDefGlobal(0,3));

		rBone.m_matInvDefGlobal(1,0)	=	FSWAP32(rBone.m_matInvDefGlobal(1,0));
		rBone.m_matInvDefGlobal(1,1)	=	FSWAP32(rBone.m_matInvDefGlobal(1,1));
		rBone.m_matInvDefGlobal(1,2)	=	FSWAP32(rBone.m_matInvDefGlobal(1,2));
		rBone.m_matInvDefGlobal(1,3)	=	FSWAP32(rBone.m_matInvDefGlobal(1,3));

		rBone.m_matInvDefGlobal(2,0)	=	FSWAP32(rBone.m_matInvDefGlobal(2,0));
		rBone.m_matInvDefGlobal(2,1)	=	FSWAP32(rBone.m_matInvDefGlobal(2,1));
		rBone.m_matInvDefGlobal(2,2)	=	FSWAP32(rBone.m_matInvDefGlobal(2,2));
		rBone.m_matInvDefGlobal(2,3)	=	FSWAP32(rBone.m_matInvDefGlobal(2,3));

		rBone.m_matInvDefGlobal(3,0)	=	FSWAP32(rBone.m_matInvDefGlobal(3,0));
		rBone.m_matInvDefGlobal(3,1)	=	FSWAP32(rBone.m_matInvDefGlobal(3,1));
		rBone.m_matInvDefGlobal(3,2)	=	FSWAP32(rBone.m_matInvDefGlobal(3,2));
		rBone.m_matInvDefGlobal(3,3)	=	FSWAP32(rBone.m_matInvDefGlobal(3,3));
		

			//		rBone.m_PhysInfo				=	SWAP32(rBone.m_PhysInfo);

		unsigned nSizeBuf = rBone.Serialize(true, NULL, 0);
		arrBuf.resize(nSizeBuf);

		if (nSizeBuf != rBone.Serialize(true, &arrBuf[0], nSizeBuf))
			throw Error ("Unexpected error while serializing bone \"%s\"", rBone.getNameCStr());

		writeArray (arrBuf);
		arrBuf.clear();
	}

	int i=0;
}


void GC_CGFConvertor::WriteMaterials ()
{
	m_Writer.AddChunk(CCF_MATERIALS);
	for (unsigned nLOD = 0; nLOD < m_arrLODs.size(); ++nLOD)
	{
		writeArray(m_arrLODs[nLOD]->arrMtls);
	}
}

void GC_CGFConvertor::WriteVertices (CryChunkedFile* pSource)
{
	m_Writer.AddChunk(CCF_VERTICES);

	unsigned numVertices = (pSource->arrMeshes[0].numVertices());

	const CryVertex* pVertices = pSource->arrMeshes[0].pVertices;

	for (unsigned nVertex = 0; nVertex < numVertices; ++nVertex)
		write (pVertices[nVertex].p);

}


void GC_CGFConvertor::WriteNormals (CryChunkedFile* pSource)
{
	m_Writer.AddChunk(CCF_NORMALS);
	unsigned numVertices = pSource->arrMeshes[0].numVertices();
	std::vector<Vec3d>& arrNormals = pSource->arrMeshes[0].arrNormals;
	writeArray (arrNormals);
}


const float fQuantizeTolerance = 4;



// Creates and serialize the connectivity info for the character
void GC_CGFConvertor::writeShadowConnectivity (unsigned nLOD)
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
		Log ("Warning: Shadow Connectivity Builder: %u orphaned edges discovered in the model %s", Builder.numOrphanedEdges(), m_pContext->sourceFile.GetString());

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


void GC_CGFConvertor::writeVertexSkin (unsigned nLOD)
{
	CRCSkinVertexSource VSource (m_arrRenderMeshes[nLOD], m_arrLODs[nLOD]->arrMeshes[0]);
	CrySkinBuilder builder (&VSource);
	CrySkinFull VertexSkin;
	builder.initSkinFull (&VertexSkin);

	unsigned nSizeRequired = VertexSkin.Serialize_GC (true, NULL, 0);
	std::vector<byte> arrBuffer;
	arrBuffer.resize (nSizeRequired);
	unsigned nSizeWritten = VertexSkin.Serialize_GC (true, &arrBuffer[0], arrBuffer.size());

	if (nSizeRequired != nSizeWritten)
		throw Error ("Cannot serialize the vertex skin: %d of %d bytes serialized", nSizeWritten, nSizeRequired);
	writeArray (arrBuffer);
}


void GC_CGFConvertor::writeNormalSkin (unsigned nLOD)
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


void GC_CGFConvertor::writeTangSkin (unsigned nLOD)
{
	CryChunkedFile* pSource = m_arrLODs[nLOD];
	CRCSkinVertexSource VSource (m_arrRenderMeshes[nLOD], pSource->arrMeshes[0]);

	// form the array of inverse-default-global matrices for the bones
	std::vector<Matrix44> arrMatInvDef;
	unsigned numBones = m_arrLODs[0]->Bones.numBones();
	arrMatInvDef.resize (numBones);
	for (unsigned i = 0; i < numBones; ++i)
		arrMatInvDef[i] = m_arrLODs[0]->Bones.getBoneByIndex(i).getInvDefGlobal();

	CrySkinBasisBuilder builder (&VSource, &arrMatInvDef[0], numBones);
	builder.setDestinationInterval(0,0xFFFFFFFF);
	CrySkinRigidBasis TangSkin;
	builder.initRigidBasisSkin (&TangSkin);

	unsigned nSizeRequired = TangSkin.Serialize (true, NULL, 0);
	std::vector<byte> arrBuffer;
	arrBuffer.resize (nSizeRequired);
	unsigned nSizeWritten = TangSkin.Serialize (true, &arrBuffer[0], arrBuffer.size());
	if (nSizeRequired != nSizeWritten)
		throw Error ("Cannot serialize the tangent skin: %d of %d bytes serialized", nSizeWritten, nSizeRequired);
	writeArray (arrBuffer);
}

void GC_CGFConvertor::writeIntFaces (unsigned nLOD)
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

// calculates the number of materials used by the previous LODs (< than given)
// this is to offset the mtl numbers of the nLOD to use the range of materials
// belonging to that LOD
unsigned GC_CGFConvertor::getLODMtlOffset (unsigned nLOD)
{
	unsigned nOffset = 0, i;
	for (i = 0; i < nLOD && i < m_arrLODs.size(); ++i)
		nOffset += m_arrLODs[i]->arrMtls.size();
	return nOffset;
}


void GC_CGFConvertor::WriteLights()
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
DWORD GC_CGFConvertor::GetTimestamp() const
{
	return GetTimestampForLoadedLibrary(g_hInst);
}


bool GC_CGFConvertor::isAnimationFastCheck()
{
	string strFileNoExt = m_pContext->getSourcePath();
	CryStringUtils::StripFileExtension(strFileNoExt);
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