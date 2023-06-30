////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   GameMods.cpp
//  Version:     v1.00
//  Created:     13/1/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//   
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "GameMods.h"
#include <ICryPak.h>
#include "Game.h"

#if defined(LINUX)
	#include <sys/io.h>
#else
#	include <io.h>
#endif

//////////////////////////////////////////////////////////////////////////
CGameMods::CGameMods( CXGame *pGame )
{
	m_pGame = pGame;
	m_pSystem = pGame->GetSystem();
	m_pILog=m_pSystem->GetILog();
	m_pMod=NULL;
	m_sCurrentMod=string("FarCry");
	ScanMods();
}

//////////////////////////////////////////////////////////////////////////
CGameMods::~CGameMods()
{
	ClearMods();
}

//////////////////////////////////////////////////////////////////////////
void CGameMods::ClearMods()
{
	for (int i = 0; i < (int)(m_mods.size()); i++)
	{
		CloseMod(m_mods[i]);
		delete m_mods[i];
	}
	m_mods.clear();
}

//////////////////////////////////////////////////////////////////////////
SGameModDescription* CGameMods::Find( const char *sModName ) const
{
	// Find this mod.
	for (int i = 0; i < (int)(m_mods.size()); i++)
	{
		if (stricmp(sModName,m_mods[i]->sName.c_str()) == 0)
		{
			return m_mods[i];
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
const SGameModDescription* CGameMods::GetModDescription( const char *sModName ) const
{
	return Find( sModName );
}

//////////////////////////////////////////////////////////////////////////
const char* CGameMods::GetCurrentMod() const
{
	return m_sCurrentMod.c_str();
}

//////////////////////////////////////////////////////////////////////////
void CGameMods::CloseMod(SGameModDescription *pMod)
{
	if (pMod)
	{	
		// remove mod folder from crypak
		string sMOD=string("Mods/")+pMod->sName;
		m_pSystem->GetIPak()->RemoveMod(sMOD.c_str());
		
		// close language pack
		ICVar *pLanguage=m_pSystem->GetIConsole()->GetCVar("g_language");
		if (pLanguage && pLanguage->GetString())
		{	
			char szPakName[512];		
			sprintf(szPakName,"Mods/%s/%s/Localized/%s.pak",m_sCurrentMod.c_str(),DATA_FOLDER,pLanguage->GetString());
			m_pSystem->GetIPak()->ClosePack(szPakName);
		}	

		// close paks in the root
		string sModPacks=sMOD+"/"+string("*.pak");
		m_pSystem->GetIPak()->ClosePacks(sMOD.c_str());

		// close basic packs
		sModPacks=sMOD+"/"+string(DATA_FOLDER)+"/*.pak";
		m_pSystem->GetIPak()->ClosePacks(sMOD.c_str());
	} 
}

//////////////////////////////////////////////////////////////////////////
bool CGameMods::SetCurrentMod(const char *sModName,bool bNeedsRestart)
{
	ASSERT(sModName);
#if defined(LINUX)
	RemoveCRLF(sModName);
#endif
	if (stricmp(m_sCurrentMod.c_str(),sModName)==0)
	{
		m_pILog->Log("MOD %s already loaded",sModName);
		return (true); // already set
	}

	// remove the previous mod (if any)
	CloseMod(m_pMod);

	m_pMod=NULL; 
	m_sCurrentMod.clear();

	bool bNormalGame=false;
	if	((stricmp(sModName,"FarCry")==0))
		bNormalGame=true;

	m_pILog->Log("Loading MOD %s",sModName);

	// switch back to normal farcry 
	if (bNormalGame)		
	{
		if (!m_pGame->m_bEditor)
		{
			if (bNeedsRestart) 
			{
				// realunch if changing at runtime back to FC - since no new paks
				// should be loaded
				m_sCurrentMod=string(sModName);
				m_pGame->SendMessage("Relaunch"); 
				return (true);						
			}

			// it is already set - this can happen only when returning to normal
			// FC after another mod was loaded - so it is already set
			return (true);
		}
		else
		{
			// TODO: all previous packs are closed now, editor should reload 
			// the level and other stuff if necessary
			return (true);
		}
	}
		
	// We're trying to set a MOD which is not normal farcry.
	// Find this mod.
	m_pMod = Find( sModName );
	if (!m_pMod)	
	{		
		// mod not found - keep the current one
		//m_sCurrentMod.clear();
		return (false); 	 
	}
	
	m_sCurrentMod = m_pMod->sName;
 
	if (!m_pGame->m_bEditor && bNeedsRestart)
	{
		// in game mode, after changing mod runtime, always relaunch to assure
		// everything, including dlls, gets reloaded
		// if bNeedsRestart if false, it means the game was launched with
		// command line MOD (best/preferred way)
		m_pILog->Log("New MOD set - relaunching game");
		m_pGame->SendMessage("Relaunch");
		return (true);			 
	}
									
	// make the crypak system aware of the new MOD
	m_sCurrentMod = sModName;

	// Open the language pack directly from the MOD folder
	ICVar *pLanguage=m_pSystem->GetIConsole()->GetCVar("g_language");
	if (pLanguage && pLanguage->GetString())
	{	
		char szPakName[512];		
		sprintf(szPakName,"Mods/%s/%s/Localized/%s.pak",m_sCurrentMod.c_str(),DATA_FOLDER,pLanguage->GetString());
		m_pSystem->GetIPak()->OpenPack( "",szPakName );
	}
 
	string sMOD=string("Mods/")+sModName;
	m_pSystem->GetIPak()->AddMod(sMOD.c_str());		

	// Open all paks in the mod folder.
	char sPaksFilter[_MAX_PATH];
	_makepath( sPaksFilter,NULL,m_pMod->sFolder.c_str(),"*","pak" );
	m_pSystem->GetIPak()->OpenPacks( "",sPaksFilter );
	
	// Open all basic *.pak files in the MOD folder		
	string paksmodFolder = string("Mods/")+string(m_sCurrentMod)+"/"+string(DATA_FOLDER)+"/*.pak";	
	m_pSystem->GetIPak()->OpenPacks( "",paksmodFolder.c_str() );	
 	
	//////////////////////////////////////////////////////////////////////////
	// Reload materials.
	/*
	// this is unsupported for now
	if (m_pGame->m_bEditor)
	{
		// TODO: Editor should reload the level
		m_pGame->m_XSurfaceMgr.LoadMaterials( "Scripts/Materials",true,true );
	}
	*/
	//////////////////////////////////////////////////////////////////////////	
		 
	return true;
}

// extract value from a text, should be included between ""
//////////////////////////////////////////////////////////////////////////
bool CGameMods::GetValue(const char *szKey,const char *szText,char *szRes) 
{
	// find the key
	const char *szStart=strstr(szText,szKey);
	if (!szStart)
		return (false);

	// find the starting point
	const char *szValueStart=strstr(szStart,"\"");
  if (!szValueStart)
		return (false);
	 
	const char *szCurr=szValueStart+1; // skip "

	// get the string
	while (*szCurr && *szCurr!='"')
	{
		*szRes++=*szCurr++;		
	}

	if (*szCurr!='"')
		return (false);

	*szRes=0;
 
	return (true);
}

// gets all needed info from the mod desc text file
//////////////////////////////////////////////////////////////////////////
bool CGameMods::ParseModDescription(const char *szFolder,SGameModDescription*pMod)
{
	char szFilename[256];
	sprintf(szFilename,"%s/ModDesc.txt",szFolder);
	ICryPak *pIPak = m_pSystem->GetIPak();
	FILE *pFile=pIPak->FOpen(szFilename,"rb");
	if (!pFile)	
		return (false);			

	// read the mod desc in a buffer
	pIPak->FSeek(pFile, 0, SEEK_END); 
	int nSize = pIPak->FTell(pFile); 
	pIPak->FSeek(pFile, 0, SEEK_SET); 
	if (nSize==0)
	{
		pIPak->FClose(pFile); 
		return (false);
	}

	char *pBuffer;	
	pBuffer = new char[nSize+1];	
	if (pIPak->FRead(pBuffer, nSize, 1, pFile) == 0) 
	{		
		delete [] pBuffer;
		pIPak->FClose(pFile); 
		return (false);
	}
	pIPak->FClose(pFile); 
	pBuffer[nSize]=0; // null terminate the string

	// extract info
	char *pBufferTemp = new char[nSize+1];

	if (GetValue("_Title_",pBuffer,pBufferTemp))
		pMod->sTitle=string(pBufferTemp);

	if (GetValue("_Author_",pBuffer,pBufferTemp))
		pMod->sAuthor=string(pBufferTemp);

	if (GetValue("_Version_",pBuffer,pBufferTemp))
		pMod->version.Set(pBufferTemp);

	if (GetValue("_Website_",pBuffer,pBufferTemp))
		pMod->sWebsite=string(pBufferTemp);

	if (GetValue("_Description_",pBuffer,pBufferTemp))
		pMod->sDescription=string(pBufferTemp);

	delete [] pBuffer;
	delete [] pBufferTemp;
	
	return (true);
}

//////////////////////////////////////////////////////////////////////////
void CGameMods::ScanMods()
{
	// delete previous mods
	ClearMods();

	// search all files in the mods folder
	struct _finddata_t c_file;
  intptr_t hFile;
	string sSearchPattern = "Mods/*.*";  

	ICryPak *pIPak = m_pSystem->GetIPak();
	if ((hFile = pIPak->FindFirst(sSearchPattern.c_str(),&c_file)) == -1L )					
		return;
	do
	{
		// skip files and ., .., keep only real folders
		if ((c_file.attrib &_A_SUBDIR) && ((strncmp(c_file.name,".",1)!=0)))
		{			
			m_pILog->Log("Found MOD game: %s",c_file.name);
			if (stricmp(c_file.name,"FarCry")==0)
				continue;

			SGameModDescription *pGameMod=new SGameModDescription;

			pGameMod->sName=c_file.name;
			pGameMod->sFolder=string("Mods/")+c_file.name;			
			if (!ParseModDescription(pGameMod->sFolder.c_str(),pGameMod))
			{				
				pGameMod->sAuthor=string("Unknown");		
				pGameMod->sTitle=string("No Title");		
			}
			m_mods.push_back(pGameMod);
		}
  
	} while(pIPak->FindNext( hFile, &c_file ) == 0);

	pIPak->FindClose( hFile );
}

//////////////////////////////////////////////////////////////////////////
const char *CGameMods::GetModPath(const char *szSource)
{
	if (m_sCurrentMod.empty() || (stricmp(m_sCurrentMod.c_str(),"FarCry")==0))
		return (NULL); 

	m_sReturnPath=string("Mods/")+m_sCurrentMod+"/"+string(szSource);
	return (m_sReturnPath.c_str());
}
 
//////////////////////////////////////////////////////////////////////////
bool CXGame::OpenPacks(const char *szFolder)
{
	// open first packs in the farcry folder 
	bool bFound=m_pSystem->GetIPak()->OpenPacks(szFolder);
	
	if (m_pGameMods)
	{
		const char *szMOD=m_pGameMods->GetCurrentMod();
		// override paks		
		if (szMOD && (stricmp(szMOD,"FarCry")!=0))
		{		
			string sPaks=string("Mods/")+string(szMOD)+"/"+szFolder;
			if (m_pSystem->GetIPak()->OpenPacks(sPaks.c_str()))
				return (true);
		}
	} 
  
	return(bFound);
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::ClosePacks(const char *szFolder)
{	
	if (m_pGameMods)
	{
		const char *szMOD=m_pGameMods->GetCurrentMod();
		if (strlen(szMOD)>0)
		{		
			string sPaks=string("Mods/")+string(szMOD)+"/"+szFolder;
			m_pSystem->GetIPak()->ClosePacks(sPaks.c_str());			
		}
	} 

	return(m_pSystem->GetIPak()->ClosePacks(szFolder));	
}

//////////////////////////////////////////////////////////////////////////
string CXGame::GetLevelsFolder() const
{
	string sFolder = "Levels";	
	/*
	if (strlen(m_pGameMods->GetCurrentMod()) > 0)
	{
		// We are using a MOD or TC
		const SGameModDescription *pMod = m_pGameMods->GetModDescription( m_pGameMods->GetCurrentMod() );
		if (pMod)
		{
			sFolder = pMod->sFolder + "/Levels";
		}
	}
	*/
	return sFolder;
}

//////////////////////////////////////////////////////////////////////////
const char *CXGame::IsMODLoaded()
{
	if (m_pGameMods)
	{
		const char *szMod=m_pGameMods->GetCurrentMod();
		if ((szMod) && (stricmp(szMod,"FarCry")!=0))
			return (szMod);
	}

	return (NULL);
}