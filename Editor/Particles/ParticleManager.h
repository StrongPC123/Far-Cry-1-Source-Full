////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particlemanager.h
//  Version:     v1.00
//  Created:     17/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particlemanager_h__
#define __particlemanager_h__
#pragma once

#include "BaseLibraryManager.h"

class CParticleItem;
class CParticleLibrary;
struct SExportParticleEffect;

/** Manages Particle libraries and systems.
*/
class CRYEDIT_API CParticleManager : public CBaseLibraryManager
{
public:
	CParticleManager();
	~CParticleManager();

	// Clear all prototypes and material libraries.
	void ClearAll();

	virtual void DeleteItem( CBaseLibraryItem* pItem );

	//////////////////////////////////////////////////////////////////////////
	// Materials.
	//////////////////////////////////////////////////////////////////////////
	//! Loads a new material from a xml node.
	void PasteToParticleItem( CParticleItem* pItem,XmlNodeRef &node,bool bWithChilds );

	//! Serialize manager.
	virtual void Serialize( XmlNodeRef &node,bool bLoading );

	//! Export particle systems to game.
	void Export( XmlNodeRef &node );

protected:
	void AddExportItem( std::vector<SExportParticleEffect> &effects,CParticleItem *pItem,int parent );
	virtual CBaseLibraryItem* MakeNewItem();
	virtual CBaseLibrary* MakeNewLibrary();
	//! Root node where this library will be saved.
	virtual CString GetRootNodeName();
	//! Path to libraries in this manager.
	virtual CString GetLibsPath();

	CString m_libsPath;
};

#endif // __particlemanager_h__
