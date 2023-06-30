////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   MaterialManager.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __materialmanager_h__
#define __materialmanager_h__
#pragma once

#include "BaseLibraryManager.h"

class CMaterial;
class CMaterialLibrary;

/** Manages all entity prototypes and material libraries.
*/
class CRYEDIT_API CMaterialManager : public CBaseLibraryManager
{
public:
	//! Notification callback.
	typedef Functor0 NotifyCallback;

	CMaterialManager();
	~CMaterialManager();

	// Clear all prototypes and material libraries.
	void ClearAll();

	//////////////////////////////////////////////////////////////////////////
	// Materials.
	//////////////////////////////////////////////////////////////////////////
	//! Loads a new material from a xml node.
	CMaterial* LoadMaterial( CMaterialLibrary *pLibrary,XmlNodeRef &node,bool bNewGuid=false );

	//! Export property manager to game.
	void Export( XmlNodeRef &node );
	int ExportLib( CMaterialLibrary *pLib,XmlNodeRef &libNode );

	void SetCurrentMaterial( CMaterial *pMtl );
	//! Cet currently active material.
	CMaterial* GetCurrentMaterial() const;
	//! Serialize property manager.
	virtual void Serialize( XmlNodeRef &node,bool bLoading );

	//////////////////////////////////////////////////////////////////////////
	// IDocListener
	//////////////////////////////////////////////////////////////////////////
	virtual	void OnNewDocument();
	virtual	void OnLoadDocument();
	virtual void OnCloseDocument();
	virtual void OnMissionChange();
	//////////////////////////////////////////////////////////////////////////

protected:
	virtual CBaseLibraryItem* MakeNewItem();
	virtual CBaseLibrary* MakeNewLibrary();
	//! Root node where this library will be saved.
	virtual CString GetRootNodeName();
	//! Path to libraries in this manager.
	virtual CString GetLibsPath();
	virtual void ReportDuplicateItem( CBaseLibraryItem *pItem,CBaseLibraryItem *pOldItem );

	CString m_libsPath;
	
	//! List of notification callbacks.
	//std::list<NotifyCallback> m_notifyCallbacks;
	GUID m_currentMaterialGUID;
};

#endif // __materialmanager_h__
