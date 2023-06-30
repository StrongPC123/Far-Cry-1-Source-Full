////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   baselibraryitem.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __baselibraryitem_h__
#define __baselibraryitem_h__
#pragma once

#include "IDataBaseItem.h"

class CBaseLibrary;

//////////////////////////////////////////////////////////////////////////
/** Base class for all items contained in BaseLibraray.
*/
class CBaseLibraryItem : public TRefCountBase<IDataBaseItem>
{
public:
	CBaseLibraryItem();
	~CBaseLibraryItem();

	//! Set item name.
	//! Its virtual, in case you want to override it in derrived item.
	virtual void SetName( const CString &name );
	//! Get item name.
	const CString& GetName() const;

	//! Get full item name, including name of library.
	//! Name formed by adding dot after name of library
	//! eg. library Pickup and item PickupRL form full item name: "Pickups.PickupRL".
	CString GetFullName() const;

	//! Get only nameof group from prototype.
	CString GetGroupName();
	//! Get short name of prototype without group.
	CString GetShortName();

	//! Return Library this item are contained in.
	//! Item can only be at one library.
	IDataBaseLibrary* GetLibrary() const;

	//////////////////////////////////////////////////////////////////////////
	//! Serialize library item to archive.
	virtual void Serialize( SerializeContext &ctx );

	//////////////////////////////////////////////////////////////////////////
	//! Generate new unique id for this item.
	void GenerateId();
	//! Returns GUID of this material.
	REFGUID GetGUID() const { return m_guid; }

	//! Validate item for errors.
	virtual void Validate() {};

	//////////////////////////////////////////////////////////////////////////
	//! Gathers resources by this item.
	virtual void GatherUsedResources( CUsedResources &resources ) {};

protected:
	void SetGUID( REFGUID guid );
	friend class CBaseLibrary;
	friend class CBaseLibraryManager;
	// Name of this prototype.
	CString m_name;
	//! Reference to prototype library who contains this prototype.
	TSmartPtr<CBaseLibrary> m_library;

	//! Every base library item have unique id.
	GUID m_guid;
};

TYPEDEF_AUTOPTR(CBaseLibraryItem);


#endif // __baselibraryitem_h__
