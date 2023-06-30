//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:CryCharBody.h
//  Declaration of CryCharBody class
//
//	History:
//	August 15, 2002: Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////

#ifndef _CRY_CHAR_BODY_HEADER_
#define _CRY_CHAR_BODY_HEADER_

#include "CryModel.h"

class CryCharManager;
class CryCharInstance;

class CryCharBody:
	public ICryCharModel
{
public:
	CryCharBody (CryCharManager* pManager, const string& strFileName);
	~CryCharBody();


	// Returns the pointer to the loaded model for this body.
	// also works as an indicator of load operation success: if the file was not successfully loaded, then returns NULL
	CryModel *GetModel();
	CVertexBuffer* GetVertexBuffer ();

	const string& GetFilePath()const;
	const char* GetFilePathCStr()const;
	const char* GetNameCStr()const;

	float GetFrameRate ()const;

	void RegisterInstance (CryCharInstance*);
	void UnregisterInstance (CryCharInstance*);

	// destroys all characters
	// THis may (and should) lead to destruction and self-deregistration of this body
	void CleanupInstances();

	// Returns the scale of the model - not used now
	virtual float GetScale() const;

	// Returns the interface for animations applicable to this model
	virtual ICryAnimationSet* GetAnimationSet ();

	// Return name of bone from bone table, return zero id nId is out of range (the game gets this id from physics)
	virtual const char * GetBoneName(int nId) const;

	// Returns the number of bones; all bone ids are in the range from 0 to this number exclusive; 0th bone is the root
	virtual int NumBones() const;

	// Returns the index of the bone by its name or -1 if no such bone exists; this is Case-Sensitive
	virtual int GetBoneByName (const char* szName);

	void GetSize(ICrySizer* pSizer);

	// makes all character instances spawn some particles (for test purposes)
	void SpawnTestParticles(bool bStart);

	// returns the file name of the character model
	const char* GetFileName();

	// dumps the model info into the log, one line
	void DumpModel();

	//Executes a per-body script command
	bool ExecScriptCommand (int nCommand, void* pParams, void* pResult);

	// returns true if the instance is registered in this body
	bool DoesInstanceExist (CryCharInstance* pInstance)
	{
		return m_setInstances.find (pInstance) != m_setInstances.end();
	}

	unsigned NumInstances()
	{
		return (unsigned)m_setInstances.size();
	}

	// returns the extra data attached to the character by the artist during exporting
	// as the scene user properties. if there's no such data, returns NULL
	const char* GetProperty(const char* szName) {return m_pCryModel->GetProperty(szName);}

	virtual ClassEnum GetClass() {return CLASS_CRYCHARBODY;}
protected:
	// the character file name, empty string means no geometry was loaded (e.g. because of an error)
	const string m_strFilePath;

	CryModel * m_pCryModel;
	float m_fAnimationFrameRate;

	CryCharManager* m_pManager;

	// the set of all child objects created; used for the final clean up
	typedef std::set<CryCharInstance*> CryCharInstanceRegistry;
	CryCharInstanceRegistry m_setInstances;
};

TYPEDEF_AUTOPTR(CryCharBody);

typedef std::set<CryCharBody_AutoPtr> CryCharBody_AutoSet;

#endif