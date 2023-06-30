////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   entityprototypemanager.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __entityprototypemanager_h__
#define __entityprototypemanager_h__
#pragma once

#include "BaseLibraryManager.h"

class CEntityPrototype;
class CEntityPrototypeLibrary;

/** Manages all entity prototypes and prototype libraries.
*/
class CRYEDIT_API CEntityPrototypeManager : public CBaseLibraryManager
{
public:
	CEntityPrototypeManager();
	~CEntityPrototypeManager();

	void ClearAll();

	//! Find prototype by fully specified prototype name.
	//! Name is given in this form: Library.Group.ItemName (ex. AI.Cover.Merc)
	CEntityPrototype* FindPrototype( REFGUID guid ) const { return (CEntityPrototype*)FindItem(guid); }

	//! Loads a new entity archetype from a xml node.
	CEntityPrototype* LoadPrototype( CEntityPrototypeLibrary *pLibrary,XmlNodeRef &node );

private:
	virtual CBaseLibraryItem* MakeNewItem();
	virtual CBaseLibrary* MakeNewLibrary();
	//! Root node where this library will be saved.
	virtual CString GetRootNodeName();
	//! Path to libraries in this manager.
	virtual CString GetLibsPath();

	CString m_libsPath;
};

#endif // __entityprototypemanager_h__
