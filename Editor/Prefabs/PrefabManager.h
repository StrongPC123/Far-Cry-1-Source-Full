////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PrefabManager.h
//  Version:     v1.00
//  Created:     10/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PrefabManager_h__
#define __PrefabManager_h__
#pragma once

#include "BaseLibraryManager.h"

class CPrefabItem;
class CPrefabLibrary;

/** Manages Particle libraries and systems.
*/
class CRYEDIT_API CPrefabManager : public CBaseLibraryManager
{
public:
	CPrefabManager();
	~CPrefabManager();

	// Clear all prototypes and material libraries.
	void ClearAll();

	//! Serialize manager.
	virtual void Serialize( XmlNodeRef &node,bool bLoading );

	//! Export particle systems to game.
	void Export( XmlNodeRef &node );

	//! Make new prefab item from selection.
	CPrefabItem* MakeFromSelection();

protected:
	virtual CBaseLibraryItem* MakeNewItem();
	virtual CBaseLibrary* MakeNewLibrary();
	//! Root node where this library will be saved.
	virtual CString GetRootNodeName();
	//! Path to libraries in this manager.
	virtual CString GetLibsPath();

	CString m_libsPath;
};

#endif // __PrefabManager_h__
