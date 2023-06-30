/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//
//  Contains the load code for CryModel class, loads geometry and animation
//
/////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef _CRY_MODEL_LOADER_HDR_
#define _CRY_MODEL_LOADER_HDR_

class CryCharBody;
class CControllerManager;

#include "CryAnimationBase.h"
#include "ChunkFileReader.h"
#include <StlUtils.h>

class CryModelLoader
{
public:
	CryModelLoader (CControllerManager* pControllerManager);
	~CryModelLoader();

	CryModel* loadNew (CryCharBody* pBody, const string& strGeomFileName, float fScale);

	// cleans up the resources allocated during load
	void clear();
protected:
	// tries to find out if there are any animations for this file; if there are some, 
	// prepares to load them and returns true; otherwise returns false
	bool searchAnimations ();

	// searches for lod models for the given model; returns false in case of some error
	bool preloadCGFs();

	// loads the CCG in memory
	bool preloadCCG();

	// loads animations for already loaded model
	bool loadAnimations();

	// loads animations for the given file from the given directory, loading all cal files
	// which begin with the cgf file name and underscope (this is the convention for the cgf's that don't have cal file associated)
	unsigned loadAnimationsNoCAL ();

	// loads animations for this cgf from the given cal file
	// does NOT close the file (the file belongs to the calling party)
	unsigned loadAnimationsWithCAL ();


	// loads the geometry files (LOD files, starting with the main one)
	bool loadCGFs();

	// loads the CCG (including all the LODs in it)
	bool loadCCG ();

	// creates the skin objects for the geometry and morph targets
	bool buildSkins();

	bool loadTextures();

	// information about an animation to load
	struct SAnimFile
	{
		string strFileName;
		string strAnimName;
		unsigned nAnimFlags; // combination of GlobalAnimation internal flags
		SAnimFile (const string& fileName, const char* szAnimName, unsigned animflags):
		strFileName(fileName), strAnimName(szAnimName), nAnimFlags(animflags) {}
		SAnimFile():nAnimFlags(0) {}
	};

	// the animation file array
	typedef std::vector<SAnimFile> AnimFileArray;
	// loads the animations fromthe array
	unsigned loadAnimationArray (const AnimFileArray& arrAnimFiles);

	typedef CAutoClear<CryModelLoader> CAutoClearLoader;

	// forgets about the m_pModel (so that it doesn't get deleted upon done()),
	// and returns it
	CryModel* detachModel();

	void indicateProgress(const char*szMsg=NULL);

	string getCCGFilePath()
	{
		return 
			m_bExtCCG ? m_strGeomFileNameNoExt + ".ccg"
			:
			"CCGF_CACHE\\" + m_strGeomFileNameNoExt+".ccg";
	}
protected:

	// the controller manager for the new model; this remains the same during the whole lifecycle
	CControllerManager* m_pControllerManager;
	// the model being loaded
	CryModel* m_pModel;

	// the file without extension
	string m_strGeomFileNameNoExt;

	// the name of the cal file
	string m_strCalFileName;
	// the CAL file handle, or NULL if none
	FILE * m_fCalFile;
	// the handle with which the animations are to be found, -1 by default
	intptr_t m_nCafFindFileHandle;
	struct _finddata_t m_fileinfo;

	// different LODs are kept in different files; each file will have its own reader here
	TFixedArray<CChunkFileReader_AutoPtr> m_arrLodFiles;

	// if the model is loaded from a CCG, then this gets loaded:
	// the data buffer for the CCG
	std::vector<char> m_arrBufferCCG;

	// model scale
	float m_fScale;

	bool m_bExtCCG; // we're loading the CCG by the command-line (we don't transform CGF name)

	// gets set to true if and only if the initial position of bone infos have been set
	// from the initial pos chunk (i.e. no need to set it from the default animation)
	// gets initialized in loadLODs()
	// gets used in loadAnimations()
	bool m_bBoneInitPosInitialized;

	// set to true when we load from a compiled file (CCG) rather than CGF
	bool m_bLoadFromCCG;
};


#endif