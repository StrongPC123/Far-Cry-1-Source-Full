////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   baselibrary.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __baselibrary_h__
#define __baselibrary_h__
#pragma once

#include "IDataBaseLibrary.h"

class CBaseLibraryManager;

/** This a base class for all Libraries used by Editor.
*/
class CBaseLibrary : public TRefCountBase<IDataBaseLibrary>
{
public:
	CBaseLibrary( CBaseLibraryManager *pManager );
	~CBaseLibrary();

	//! Set library name.
	virtual void SetName( const CString &name );
	//! Get library name.
	const CString& GetName() const;

	//! Set new filename for this library.
	void SetFilename( const CString &filename ) { m_filename = filename; };
	const CString& GetFilename() const { return m_filename; };

	virtual bool Save() = 0;
	virtual bool Load( const CString &filename ) = 0;
	virtual void Serialize( XmlNodeRef &node,bool bLoading ) = 0;

	//! Mark library as modified.
	void SetModified( bool bModified=true );
	//! Check if library was modified.
	bool IsModified() const { return m_bModified; };

	//////////////////////////////////////////////////////////////////////////
	// Working with items.
	//////////////////////////////////////////////////////////////////////////
	//! Add a new prototype to library.
	void AddItem( IDataBaseItem* item );
	//! Get number of known prototypes.
	int GetItemCount() const { return m_items.size(); }
	//! Get prototype by index.
	IDataBaseItem* GetItem( int index );

	//! Delete item by pointer of item.
	void RemoveItem( IDataBaseItem* item );

	//! Delete all items from library.
	void RemoveAllItems();

	//! Find library item by name.
	//! Using linear search.
	IDataBaseItem* FindItem( const CString &name );

	//! Check if this library is local level library.
	bool IsLevelLibrary() const { return m_bLevelLib; };

	//! Set library to be level library.
	void SetLevelLibrary( bool bEnable ) { m_bLevelLib = bEnable; };

	//////////////////////////////////////////////////////////////////////////
	//! Return manager for this library.
	IDataBaseManager* GetManager();

private:
	//! Name of the library.
	CString m_name;
	//! Filename of the library.
	CString m_filename;

	//! Flag set when library was modified.
	bool m_bModified;

	//! Level library is saved within the level .cry file and is local for this level.
	bool m_bLevelLib;

	//////////////////////////////////////////////////////////////////////////
	// Manager.
	CBaseLibraryManager *m_pManager;

	// Array of all our library items.
	std::vector<TSmartPtr<CBaseLibraryItem> > m_items;
};

#endif // __baselibrary_h__
