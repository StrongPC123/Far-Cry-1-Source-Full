////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   GameMods.h
//  Version:     v1.00
//  Created:     13/1/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __GameMods_h__
#define __GameMods_h__

#include "IGame.h"

class CXGame;

//////////////////////////////////////////////////////////////////////////
typedef std::vector<SGameModDescription*> lstMods;
typedef lstMods::iterator lstModsIt;

/*!	Implement IGameMods interface to access and manipulate Game Mods.
*/
//////////////////////////////////////////////////////////////////////////
class CGameMods : public IGameMods
{
public:
	CGameMods( CXGame *pGame );
	~CGameMods();
	
	// IGameMods implementation.
	virtual const SGameModDescription* GetModDescription( const char *sModName ) const;
	virtual const char* GetCurrentMod() const;
	virtual bool SetCurrentMod( const char *sModName,bool bNeedsRestart=false );
	virtual const char* GetModPath(const char *szSource);	

	//! Array of mod descriptions.
	lstMods m_mods;

private:

	//! Go thru Mods directory and find out what mods are installed.
	SGameModDescription* Find( const char *sModName ) const;
	void	ScanMods();	
	void	ClearMods();
	void	CloseMod(SGameModDescription *pMod);

	bool	ParseModDescription(const char *szFolder,SGameModDescription*pMod);
	bool	GetValue(const char *szKey,const char *szText,char *szRes);
	
	CXGame* m_pGame;
	ISystem *m_pSystem;
	ILog		*m_pILog;

	// Name of the mod currently active.
	string m_sCurrentMod;
	// Current mod
	SGameModDescription *m_pMod;
	// Used to return the path to other modules
	string m_sReturnPath;
};

#endif // __GameMods_h__

