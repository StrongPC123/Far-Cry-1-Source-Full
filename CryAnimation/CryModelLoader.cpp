#include "stdafx.h"
#include <StlUtils.h>
#include <CryCompiledFile.h>
#include "ChunkFileReader.h"
#include "CryModel.h"
#include "CryModelState.h"
#include "CryModelLoader.h"
#include "CryModelGeometryLoader.h"
#include "StringUtils.h"
#include "CrySkinMorph.h"
#include "CrySkinMorphBuilder.h"
#include "ControllerManager.h"
#include "CgfUtils.h"
#include "CVars.h"
#include "CryModelSubmesh.h"
using namespace CryStringUtils;

class CAutoFile
{
	FILE* m_f;
public:
	CAutoFile (const char* szName, const char* szMode)
	{
		m_f = g_GetPak()->FOpen (szName, szMode);
	}
	~CAutoFile ()
	{
		if (m_f)
			g_GetPak()->FClose (m_f);
	}

	long GetSize()
	{
		if (g_GetPak()->FSeek (m_f, 0, SEEK_END))
			return -1;

		long nSize = g_GetPak()->FTell (m_f);

		if (g_GetPak()->FSeek(m_f, 0, SEEK_SET))
			return -1;

		return nSize;
	}

	bool Read (void* pData, unsigned nSize)
	{
		return (1 == g_GetPak()->FRead (pData, nSize, 1, m_f));
	}

	operator FILE*() {return m_f;}
};


CryModelLoader::CryModelLoader (CControllerManager* pControllerManager):
	m_pControllerManager (pControllerManager),
	m_fCalFile (NULL),
	m_nCafFindFileHandle (-1),
	m_pModel (NULL),
	m_bLoadFromCCG (false),
	m_bExtCCG(false)
{
	
}

CryModelLoader::~CryModelLoader()
{
	// clean up the resources
	clear();
}

// cleans up the resources allocated during load
void CryModelLoader::clear()
{
	m_strGeomFileNameNoExt = "";
	m_strCalFileName = "";
	
	if (m_fCalFile)
	{
		g_GetPak()->FClose (m_fCalFile);
		m_fCalFile = NULL;
	}

	if (m_nCafFindFileHandle != -1)
	{
		g_GetPak()->FindClose(m_nCafFindFileHandle);
		m_nCafFindFileHandle = -1;
	}

	if (m_pModel)
	{
		delete m_pModel;
		m_pModel = NULL;
	}

	m_arrLodFiles.clear();
	m_arrBufferCCG.clear();
}

bool IsValidFile (const char* szFilePath)
{
	FILE* f = g_GetPak()->FOpen(szFilePath, "rb");
	if (f)
	{
		g_GetPak()->FClose (f);
		return true;
	}
	return false;
}

CryModel* CryModelLoader::loadNew (CryCharBody* pBody, const string& strGeomFileName, float fScale)
{
	AUTO_PROFILE_SECTION(g_dTimeGeomLoad);
	// make sure we call done() whenever we return from this function
	CAutoClearLoader _autoClear (this);

	const char* szExt = FindExtension(strGeomFileName.c_str());
	m_strGeomFileNameNoExt.assign (strGeomFileName.c_str(), *szExt?szExt-1:szExt);
	m_bExtCCG = !stricmp(szExt, "ccg");
	m_fScale = fScale;

	m_bLoadFromCCG = g_GetCVars()->ca_EnableCCG() && IsValidFile (getCCGFilePath().c_str());

	// first, initialize the search of animations. If the file has no animations at all (no cal file, no _..caf files)
	// then this is not an animated file and must be loaded somewhere else (namely in the 3D Engine)
	if (!m_bLoadFromCCG)
		searchAnimations (); // no need to exit if no animations: maybe it's a body part

	g_GetLog()->UpdateLoadingScreen ("\003Loading %s", cutString(m_bLoadFromCCG?getCCGFilePath():strGeomFileName, 40).c_str());
	// find how many LODs we have
	if (!(m_bLoadFromCCG?preloadCCG():preloadCGFs()))
		return NULL;

	m_pModel = new CryModel (pBody, m_pControllerManager);

	if (m_bLoadFromCCG)
	{
		if (!loadCCG())
			return NULL;
	}
	else
	{
		if (!loadCGFs())
			return NULL;

		//m_pModel->buildMorphSkins();

		if (!loadTextures())
			return NULL;

		m_pModel->m_pDefaultModelState->GetCryModelSubmesh(0)->GenerateRenderArrays (strGeomFileName.c_str());
	
		if (!loadAnimations())
			return NULL;
	}


#if 0
	// needed for XBox development
	extern void exportTestModel(CryGeometryInfo* pGeometry, CLeafBuffer* pLeafBuffer);
	// on this stage, if m_pCryModel == NULL, it means the loader couldn't load the model
	if (m_pModel) 
		exportTestModel( m_pModel->getGeometryInfo(), m_pModel->m_pDefaultModelState->m_pLeafBuffers[0]);
#endif


	if (g_GetCVars()->ca_ZDeleteConstructionData())
	{
		m_pModel->clearConstructionData();
	}
	else
		g_GetLog()->LogWarning ("\005The construction data wasn't deleted");

	// return the initialized(loaded) model, forgetting about it so that it doesn't get destructed by the following call to done()
	return detachModel();
}


// tries to find out if there are any animations for this file; if there are some, 
// prepares to load them and returns true; otherwise returns false
bool CryModelLoader::searchAnimations ()
{
	AUTO_PROFILE_SECTION(g_dTimeGeomChunkLoad);
	// make up the cal file name
	m_strCalFileName = m_strGeomFileNameNoExt + ".cal";

	// try to find out - if there are any animations for this file. if there are none, then return an error
	m_fCalFile = g_GetPak()->FOpen(m_strCalFileName.c_str(), "r");
	m_nCafFindFileHandle = -1;
	
	if (!m_fCalFile)
	{
		// finish making search path
		string strSeachFilter = m_strGeomFileNameNoExt + "_*.caf";

		// search															
		m_nCafFindFileHandle = g_GetPak()->FindFirst (strSeachFilter.c_str(), &m_fileinfo);
	}

	return m_fCalFile || m_nCafFindFileHandle != -1;
}


// searches for lod models for the given model; returns false in case of some error
bool CryModelLoader::preloadCGFs()
{
	AUTO_PROFILE_SECTION (g_dTimeGeomChunkLoadFileIO);
	CChunkFileReader_AutoPtr pReader = new CChunkFileReader ();
	// first try to open LOD 0
	if (!pReader->open (m_strGeomFileNameNoExt + ".cgf"))
	{
		g_GetLog()->LogError ("\003CryModelLoader::preloadCGFs(%s): main file not found", m_strGeomFileNameNoExt.c_str());
		return false;
	}

	if (0 == pReader->numChunksOfType(ChunkType_BoneAnim))
		return false; // the cgf doesn't contain bone info

	m_arrLodFiles.reinit (1);
	m_arrLodFiles[0] = pReader;

	for (unsigned nLOD = 1; nLOD < g_nMaxGeomLodLevels; ++nLOD)
	{
		pReader = new CChunkFileReader();
		if (pReader->open(m_strGeomFileNameNoExt + "_lod" + toString(nLOD) + ".cgf"))
			m_arrLodFiles.push_back(pReader);
		else
			break; // we have opened all the LOD files so far

		indicateProgress();
	}

	// we have LOD 0, so it's optional to have any other and we return true anyway
	return true;
}



// loads the CCG in memory buffer m_arrBufferCCG
bool CryModelLoader::preloadCCG()
{
	AUTO_PROFILE_SECTION(g_dTimeGeomChunkLoadFileIO);
	CAutoFile fIn (getCCGFilePath().c_str(), "rb");
	if (!fIn)
		return false;

	long nSize = fIn.GetSize();
	if (nSize <= 0)
		return false;

	m_arrBufferCCG.resize(nSize);

	if (!fIn.Read (&m_arrBufferCCG[0], nSize))
		return false;

	// data has been read - automatically close the file and return success
	return true;
}


// loads animations for already loaded model
bool CryModelLoader::loadAnimations()
{
	AUTO_PROFILE_SECTION(g_dTimeAnimLoadBind);

	// the number of animations loaded
	unsigned numAnimations = 0;

	{
		AUTO_PROFILE_SECTION (g_dTimeTest1);
		if(m_fCalFile)
			numAnimations = loadAnimationsWithCAL ();
		else
		if (m_nCafFindFileHandle != -1)
			numAnimations = loadAnimationsNoCAL ();
	}

	{
		AUTO_PROFILE_SECTION(g_dTimeTest2);
		if (!numAnimations && !m_pModel->numMorphTargets())
		{
			g_GetLog()->LogWarning ("\004CryModelLoader::loadAnimations(%s): couldn't find any animations for the model. Standalone character will be useless.", m_strGeomFileNameNoExt.c_str());
			//return false;
		}

		if (!m_bLoadFromCCG)
		{
			g_GetLog()->UpdateLoadingScreenPlus ("\003 precomputing");
			m_pModel->LoadPostInitialize (!m_bBoneInitPosInitialized);
			g_GetLog()->UpdateLoadingScreenPlus ("\003done.");
		}
	}

	if (numAnimations)
		g_GetLog()->UpdateLoadingScreen("  %d animations loaded (total animations: %d)", numAnimations, m_pControllerManager->NumAnimations());
	
	// m_pControllerManager->LogUsageStats();
	//m_pModel->shrinkControllerArrays();

	return m_pModel->m_pDefaultModelState
		&& ((m_pModel->numBoneInfos()
				&& m_pModel->m_pDefaultModelState->getRootBone())
		|| m_pModel->numMorphTargets());
}

// loads the animations from the array: pre-allocates the necessary controller arrays
// the 0th animation is the default animation
unsigned CryModelLoader::loadAnimationArray (const AnimFileArray& arrAnimFiles)
{
	unsigned nAnimID = 0;
	if (arrAnimFiles.empty())
		return 0;

	indicateProgress("\003 Anims");
	{
		AUTO_PROFILE_SECTION(g_dTimeAnimLoadBindPreallocate);
		m_pModel->prepareLoadCAFs ((unsigned)arrAnimFiles.size());
	}

	indicateProgress("\003:");

	// load the default animation - it must be always loaded synchronously

	unsigned nDefAnimFlags = arrAnimFiles[0].nAnimFlags;
	if (!stricmp(arrAnimFiles[0].strAnimName.c_str(), "default"))
		nDefAnimFlags |= GlobalAnimation::FLAGS_DEFAULT_ANIMATION;

	if(m_pModel->LoadCAF(arrAnimFiles[0].strFileName.c_str(), m_fScale, nAnimID, arrAnimFiles[0].strAnimName.c_str(), nDefAnimFlags) >= 0)
		nAnimID++;
	else
		if (g_GetCVars()->ca_Debug())
			g_GetLog()->LogWarning ("\005Default pose for %s was not found, object may not skin as expected", m_strGeomFileNameNoExt.c_str());

	for (unsigned i = 1; i < arrAnimFiles.size(); ++i)
	{
		if (m_pModel->LoadCAF(arrAnimFiles[i].strFileName.c_str(), m_fScale, nAnimID, arrAnimFiles[i].strAnimName.c_str(),arrAnimFiles[i].nAnimFlags) >= 0)
		{
			nAnimID++;
			//if((i%10)==0)
			//	indicateProgress();
		}
		else
			g_GetLog()->LogWarning ("\002Animation (Caf) file \"%s\" could not be read (it's an animation of \"%s.cgf\")", arrAnimFiles[i].strFileName.c_str(), m_strGeomFileNameNoExt.c_str());
	}
	indicateProgress("\003done;");
	return nAnimID;
}


//////////////////////////////////////////////////////////////////////////
// loads animations for the given file from the given directory, loading
// all cal files which begin with the cgf file name and underscope (this
// is the convention for the cgf's that don't have cal file associated)
// PARAMETERS:
//	 m_strGeomFileNameNoExt - the name of the cgf/cid file without extension
//   m_nCafFindFileHandle   - the search handle opened by _findfirst() for all cal files belonging to this cgf
//   m_fScale               - the scale factor to be applied to all controllers
unsigned CryModelLoader::loadAnimationsNoCAL ()
{
	// animation files to load, excluding the default animation
	AnimFileArray arrAnimFiles;
	{
	AUTO_PROFILE_SECTION(g_dTimeAnimLoadBindNoCal);
	// make search path
	string strDirName = GetParentDirectory (m_strGeomFileNameNoExt).c_str();

	// load the default pose first
	string strDefaultPose = (m_strGeomFileNameNoExt + "_default.caf").c_str();

	arrAnimFiles.reserve (64);
	// we need default animation immediately, but unlikely we'll need it in the future (so we can unload it)
	arrAnimFiles.push_back (SAnimFile(strDefaultPose, "default", GlobalAnimation::FLAGS_DEFAULT_ANIMATION));

	// the name of the base cgf (before the understrike) + 1
	unsigned nBaseNameLength = unsigned(m_strGeomFileNameNoExt.length() - strDirName.length());
	indicateProgress();
	do
	{
		SAnimFile AnimFile;
		AnimFile.strFileName = strDirName + "\\" + m_fileinfo.name;

		if(!stricmp(AnimFile.strFileName.c_str(), strDefaultPose.c_str()))
			// skip the default pose as it has already been loaded
			continue;

		//if (!stricmp(FindExtension(fileinfo.name), "caf")) // actually ,according to the search mask, this should be met automatically
		char* szExtension = StripFileExtension(m_fileinfo.name);
		assert (!stricmp(szExtension, "caf"));
		assert (strlen(m_fileinfo.name) > nBaseNameLength);

		AnimFile.strAnimName = m_fileinfo.name + nBaseNameLength;
		arrAnimFiles.push_back (AnimFile);
	}
	while (g_GetPak()->FindNext( m_nCafFindFileHandle, &m_fileinfo ) != -1);
	}
	return loadAnimationArray(arrAnimFiles);
}


//////////////////////////////////////////////////////////////////////////
// loads animations for this cgf from the given cal file
// does NOT close the file (the file belongs to the calling party)
// PARAMETERS
//	 m_strGeomFileNameNoExt - the name of the cgf/cid file without extension
//   m_fCalFile             - the file opened by fopen() for the cal associated with this cgf
//   m_fScale               - the scale factor to be applied to all controllers
unsigned CryModelLoader::loadAnimationsWithCAL ()
{
	AnimFileArray arrAnimFiles;
	{
	AUTO_PROFILE_SECTION(g_dTimeAnimLoadBindWithCal);
	// Load cal file and load animations from animations folder
	// make anim folder name
	// make search path
	string strDirName = GetParentDirectory(m_strGeomFileNameNoExt).c_str();
	string strAnimDirName = GetParentDirectory(strDirName, 2) + "\\animations";
	// the flags applicable to the currently being loaded animation
	unsigned nAnimFlags = 0;
	
	arrAnimFiles.reserve (256);

	indicateProgress();
	for (int i = 0; m_fCalFile && !g_GetPak()->FEof(m_fCalFile); ++i)
	{
		char sBuffer[512]="";
		g_GetPak()->FGets(sBuffer,512,m_fCalFile);
		char*szAnimName;
		char*szFileName;

		if(sBuffer[0] == '/' || sBuffer[0]=='\r' || sBuffer[0]=='\n' || sBuffer[0]==0)
			continue;

		//if(sscanf(sBuffer, "%s=%s", szAnimName, szFileName) != 2)
		//	continue;
		szAnimName = strtok (sBuffer, " \t\n\r=");
		if (!szAnimName)
			continue;
		szFileName = strtok(NULL, " \t\n\r=");
		if (!szFileName || szFileName[0] == '?')
		{
			m_pModel->RegisterDummyAnimation(szAnimName);
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
				strAnimDirName = strDirName + "\\" + szFileName;
				// delete the trailing slashes
				while (
					!strAnimDirName.empty()
					&& strAnimDirName [strAnimDirName.length()-1] == '\\'
					)
					strAnimDirName[strAnimDirName.length()-1] = '\0';
			}
			else
			if (!stricmp (szDirective, "ModelOffsetX"))
			{
				float fValue;
				if (sscanf (szFileName, "%f", &fValue) != 1)
					g_GetLog ()->LogToFile("\003Warning:directive ModelOffsetX %s couldn't be read in file %s.cal", szFileName, m_strGeomFileNameNoExt.c_str());
				else
					m_pModel->m_vModelOffset.x = fValue;
			}
			else
			if (!stricmp (szDirective, "ModelOffsetY"))
			{
				float fValue;
				if (sscanf (szFileName, "%f", &fValue) != 1)
					g_GetLog ()->LogToFile("\003Warning:directive ModelOffsetY %s couldn't be read in file %s.cal", szFileName, m_strGeomFileNameNoExt.c_str());
				else
					m_pModel->m_vModelOffset.y = fValue;
			}
			else
			if (!stricmp (szDirective, "ModelOffsetZ"))
			{
				float fValue;
				if (sscanf (szFileName, "%f", &fValue) != 1)
					g_GetLog ()->LogToFile("\003Warning:directive ModelOffsetZ %s couldn't be read in file %s.cal", szFileName, m_strGeomFileNameNoExt.c_str());
				else
					m_pModel->m_vModelOffset.z = fValue;
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
						g_GetLog()->LogWarning ("\003invalid option for AutoUnload directive (must be yes or no) in file %s.cal", m_strGeomFileNameNoExt.c_str());
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
					g_GetLog()->LogWarning ("\003invalid option for DelayLoad directive (must be yes or no) in file %s.cal", m_strGeomFileNameNoExt.c_str());
					break;
				}
			}
			else
				g_GetLog()->LogWarning ("\003Unknown directive %s", szDirective);
			continue;
		}

		arrAnimFiles.push_back(SAnimFile (strAnimDirName + "\\" + szFileName, szAnimName,nAnimFlags));
	}
  if (arrAnimFiles.empty())
    return false;
	}
	return loadAnimationArray (arrAnimFiles);
}


// loads the CCG (including all the LODs in it)
bool CryModelLoader::loadCCG()
{
	CCFMemReader Reader (&m_arrBufferCCG[0], (unsigned)m_arrBufferCCG.size());
	if (Reader.IsEnd())
		return false;

	string strDirName = GetParentDirectory(m_strGeomFileNameNoExt).c_str();
	string strAnimDirName = GetParentDirectory(strDirName, 2) + "\\animations";
	if (!m_pModel->initFromCCG(strDirName, strAnimDirName, Reader, m_fScale))
		return false;

	// when we load CCG, we always have the init pose initialized,
	// otherwise we fail to load
	m_bBoneInitPosInitialized = true; 

	return true;
}

// loads the geometry files (LOD files, starting with the main one)
bool CryModelLoader::loadCGFs()
{
	unsigned numLODs = (unsigned)m_arrLodFiles.size();
	m_pModel->m_arrGeomInfo.reinit (numLODs);

	CryModelGeometryLoader GeomLoader;
	unsigned i;

	for (i = 0; i < numLODs; ++i)
	{
		if (!GeomLoader.load (m_pModel, m_arrLodFiles[i], i, m_fScale))
		{
			if (i)
				g_GetLog()->LogWarning ("\003Modes LOD %d can't be loaded. Please reexport the LOD file. Animated object will not be loaded.", i);
			return false;
		}

		if (i == 0)
			m_bBoneInitPosInitialized = GeomLoader.hasBoneInfoInitPos();
	}

	if (g_GetCVars()->ca_NoMtlSorting())
		return true;

	m_pModel->deleteUnusedMaterials();

	unsigned numMtls = (unsigned)m_pModel->m_arrMaterials.size();

	// this will map from the new material id to the old material id after sorting
	// the materials
	std::vector<unsigned> arrMapMtlsOld, arrMapMtlsNew;
	arrMapMtlsOld.resize (numMtls);
	for (i = 0; i < numMtls; ++i)
		arrMapMtlsOld[i] = i;
	
	// initial (identity) mapping created, now tokenize the material names
	std::vector<CMatEntityNameTokenizer> arrTokenizers;
	arrTokenizers.resize (numMtls);
	for (i = 0; i < numMtls; ++i)
		arrTokenizers[i].tokenize (m_pModel->m_arrMaterials[i].name);
	
	
	// now sort the indices to receive the correct distribution of materials
	// thus we'll effectively receive perm[new]=old
	std::sort (arrMapMtlsOld.begin(), arrMapMtlsOld.end(), CMatEntityIndexSort(&arrTokenizers[0], numMtls));
	
	//std::swap (arrMapMtlsOld.front(), arrMapMtlsOld.back());

	// form the permutation perm[old]=new
	arrMapMtlsNew.resize (numMtls);
	ConstructReversePermutation(&arrMapMtlsOld[0], &arrMapMtlsNew[0], numMtls);

	// resort the material entities in the model
	RemapMatEntities (&m_pModel->m_arrMaterials[0], numMtls, &arrMapMtlsOld[0]);

	// now remap the material ids in the faces
	for (i = 0; i < m_pModel->numLODs(); ++i)
		m_pModel->getGeometryInfo(i)->remapMtlIds(&arrMapMtlsNew[0], numMtls);

	return true;
}


bool CryModelLoader::loadTextures()
{
	AUTO_PROFILE_SECTION(g_dTimeShaderLoad);
	if (g_GetCVars()->ca_Debug())
		g_GetLog()->UpdateLoadingScreen("\005  Loading shaders from %s", m_strGeomFileNameNoExt.c_str());
	return true;
}

// forgets about the m_pModel (so that it doesn't get deleted upon done()),
// and returns it
CryModel* CryModelLoader::detachModel()
{
	CryModel* pModel = m_pModel; // this model will be returned
	m_pModel = NULL; // forget about the model
	return pModel;
}

void CryModelLoader::indicateProgress(const char*szMsg)
{
	if (szMsg)
		g_GetLog()->UpdateLoadingScreenPlus(szMsg?szMsg:"\003.");
}
