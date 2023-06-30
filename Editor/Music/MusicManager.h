////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   MusicManager.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __musicmanager_h__
#define __musicmanager_h__
#pragma once

#include "BaseLibraryManager.h"

struct SMusicData;

class CMusicThemeLibrary;
class CMusicThemeLibItem;

/** Manages all entity prototypes and material libraries.
*/
class CRYEDIT_API CMusicManager : public CBaseLibraryManager
{
public:
	//! Notification callback.
	CMusicManager();
	~CMusicManager();

	// Clear all prototypes and material libraries.
	void ClearAll();

	//! Serialize property manager.
	virtual void Serialize( XmlNodeRef &node,bool bLoading );

	//! Export property manager to game.
	void Export( XmlNodeRef &node );

	void LoadFromLua( CBaseLibrary *pLibrary );
	//! Return Dynamic Music data definition.
	SMusicData* GetMusicData() { return m_pMusicData; }

protected:
	void ReleaseMusicData();
	virtual CBaseLibraryItem* MakeNewItem();
	virtual CBaseLibrary* MakeNewLibrary();
	//! Root node where this library will be saved.
	virtual CString GetRootNodeName();
	//! Path to libraries in this manager.
	virtual CString GetLibsPath();

	CString m_libsPath;

	// Stores all music data.
	SMusicData *m_pMusicData;
};

#endif // __musicmanager_h__
