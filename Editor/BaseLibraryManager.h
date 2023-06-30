////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   baselibrarymanager.h
//  Version:     v1.00
//  Created:     10/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __baselibrarymanager_h__
#define __baselibrarymanager_h__
#pragma once

#include "IDataBaseItem.h"
#include "IDataBaseLibrary.h"
#include "IDataBaseManager.h"

class CBaseLibraryItem;
class CBaseLibrary;

/** Manages all Libraries and Items.
*/
class CBaseLibraryManager : public TRefCountBase<IDataBaseManager>, public IDocListener
{
public:
	CBaseLibraryManager();
	~CBaseLibraryManager();

	//! Clear all libraries.
	virtual void ClearAll();

	//////////////////////////////////////////////////////////////////////////
	// IDocListener implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual	void OnNewDocument();
	virtual	void OnLoadDocument();
	virtual void OnCloseDocument();
	virtual void OnMissionChange();

	//////////////////////////////////////////////////////////////////////////
	// Library items.
	//////////////////////////////////////////////////////////////////////////
	//! Make a new item in specified library.
	virtual IDataBaseItem* CreateItem( IDataBaseLibrary *pLibrary );
	//! Delete item from library and manager.
	virtual void DeleteItem( IDataBaseItem* pItem );

	//! Find Item by its GUID.
	virtual IDataBaseItem* FindItem( REFGUID guid ) const;
	virtual IDataBaseItem* FindItemByName( const CString &fullItemName );

	//////////////////////////////////////////////////////////////////////////
	// Libraries.
	//////////////////////////////////////////////////////////////////////////
	//! Add Item library.
	virtual IDataBaseLibrary* AddLibrary( const CString &library );
	virtual void DeleteLibrary( const CString &library );
	//! Get number of libraries.
	virtual int GetLibraryCount() const { return m_libs.size(); };
	//! Get Item library by index.
	virtual IDataBaseLibrary* GetLibrary( int index ) const;

	//! Find Items Library by name.
	virtual IDataBaseLibrary* FindLibrary( const CString &library );

	//! Load Items library.
	virtual IDataBaseLibrary* LoadLibrary( const CString &filename );

	//! Save all modified libraries.
	virtual void SaveAllLibs();

	//! Serialize property manager.
	virtual void Serialize( XmlNodeRef &node,bool bLoading );

	//! Export items to game.
	virtual void Export( XmlNodeRef &node ) {};

	//void AddNotifyListener( 

	//! Returns unique name base on input name.
	virtual CString MakeUniqItemName( const CString &name );
	virtual CString MakeFullItemName( IDataBaseLibrary *pLibrary,const CString &group,const CString &itemName );

	//! Root node where this library will be saved.
	virtual CString GetRootNodeName() = 0;
	//! Path to libraries in this manager.
	virtual CString GetLibsPath() = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Validate library items for errors.
	void Validate();

	//////////////////////////////////////////////////////////////////////////
	void GatherUsedResources( CUsedResources &resources );

	//////////////////////////////////////////////////////////////////////////
	void RegisterItem( CBaseLibraryItem *pItem,REFGUID newGuid );
	void RegisterItem( CBaseLibraryItem *pItem );
	void UnregisterItem( CBaseLibraryItem *pItem );

protected:
	void SplitFullItemName( const CString &fullItemName,CString &libraryName,CString &itemName );

	//////////////////////////////////////////////////////////////////////////
	// Must be overriden.
	//! Makes a new Item.
	virtual CBaseLibraryItem* MakeNewItem() = 0;
	virtual CBaseLibrary* MakeNewLibrary() = 0;
	//////////////////////////////////////////////////////////////////////////

	virtual void ReportDuplicateItem( CBaseLibraryItem *pItem,CBaseLibraryItem *pOldItem );

	//! Array of all loaded entity items libraries.
	std::vector<TSmartPtr<CBaseLibrary> > m_libs;

	// There is always one current level library.
	TSmartPtr<CBaseLibrary> m_pLevelLibrary;

	// GUID to item map.
	typedef std::map<GUID,TSmartPtr<CBaseLibraryItem>,guid_less_predicate> ItemsMap;
	ItemsMap m_itemsMap;
};

#endif // __baselibrarymanager_h__
