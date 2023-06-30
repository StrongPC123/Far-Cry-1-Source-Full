////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   IDataBaseManager.h
//  Version:     v1.00
//  Created:     3/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IDataBaseManager_h__
#define __IDataBaseManager_h__
#pragma once

struct IDataBaseItem;
struct IDataBaseLibrary;

//////////////////////////////////////////////////////////////////////////
//
// Interface to the collection of all items or specific type 
// in data base libraries.
//
//////////////////////////////////////////////////////////////////////////
struct IDataBaseManager
{
	//! Clear all libraries.
	virtual void ClearAll() = 0;

	//////////////////////////////////////////////////////////////////////////
	// Library items.
	//////////////////////////////////////////////////////////////////////////
	//! Make a new item in specified library.
	virtual IDataBaseItem* CreateItem( IDataBaseLibrary *pLibrary ) = 0;
	//! Delete item from library and manager.
	virtual void DeleteItem( IDataBaseItem* pItem ) = 0;

	//! Find Item by its GUID.
	virtual IDataBaseItem* FindItem( REFGUID guid ) const = 0;
	virtual IDataBaseItem* FindItemByName( const CString &fullItemName ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Libraries.
	//////////////////////////////////////////////////////////////////////////
	//! Add Item library.
	virtual IDataBaseLibrary* AddLibrary( const CString &library ) = 0;
	virtual void DeleteLibrary( const CString &library ) = 0;
	//! Get number of libraries.
	virtual int GetLibraryCount() const = 0;
	//! Get Item library by index.
	virtual IDataBaseLibrary* GetLibrary( int index ) const = 0;

	//! Find Items Library by name.
	virtual IDataBaseLibrary* FindLibrary( const CString &library ) = 0;

	//! Load Items library.
	virtual IDataBaseLibrary* LoadLibrary( const CString &filename ) = 0;

	//! Save all modified libraries.
	virtual void SaveAllLibs() = 0;

	//! Serialize property manager.
	virtual void Serialize( XmlNodeRef &node,bool bLoading ) = 0;

	//! Export items to game.
	virtual void Export( XmlNodeRef &node ) {};

	//! Returns unique name base on input name.
	virtual CString MakeUniqItemName( const CString &name ) = 0;
	virtual CString MakeFullItemName( IDataBaseLibrary *pLibrary,const CString &group,const CString &itemName ) = 0;

	//! Root node where this library will be saved.
	virtual CString GetRootNodeName() = 0;
	//! Path to libraries in this manager.
	virtual CString GetLibsPath() = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Validate library items for errors.
	virtual void Validate() = 0;

	// Description:
	//		Collects names of all resource files used by managed items.
	// Arguments:
	//		resources - Structure where all filenames are collected.
	virtual void GatherUsedResources( CUsedResources &resources ) = 0;
};

#endif // __IDataBaseManager_h__
