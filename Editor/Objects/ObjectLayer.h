////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   objectlayer.h
//  Version:     v1.00
//  Created:     9/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __objectlayer_h__
#define __objectlayer_h__

#if _MSC_VER > 1000
#pragma once
#endif

// forward declarations.
class CObjectLayerManager;
class CObjectManager;

//////////////////////////////////////////////////////////////////////////
/*!
	 Object Layer.
	 All objects are orginized in hierarchical layers. 
	 Every layer can be made hidden/frozen or can be exported or imported back.
*/
//////////////////////////////////////////////////////////////////////////
class CObjectLayer : public CRefCountBase
{
public:
	CObjectLayer( CObjectLayerManager *pLayerManager );

	//! Set layer name.
	void SetName( const CString &name );
	//! Get layer name.
	const CString& GetName() const { return m_name; }

	//! Get GUID assigned to this layer.
	REFGUID GetGUID() const { return m_guid; }

	//////////////////////////////////////////////////////////////////////////
	// Query layer status.
	//////////////////////////////////////////////////////////////////////////
	bool IsVisible() const { return !m_hidden; }
	bool IsFrozen() const { return m_frozen; }
	bool IsRemovable() const { return m_removable; };
	bool IsExternal() const { return m_external; };
	bool IsExportable() const { return m_exportable; };

	//////////////////////////////////////////////////////////////////////////
	// Set layer status.
	//////////////////////////////////////////////////////////////////////////
	void SetVisible( bool b,bool bRecursive=false );
	void SetFrozen( bool b,bool bRecursive=false );
	void SetRemovable( bool b ) { m_removable = b; };
	void SetExternal( bool b ) { m_external = b; };
	void SetExportable( bool b ) { m_exportable = b; };

	//////////////////////////////////////////////////////////////////////////
	//! Save/Load layer to/from xml node.
	void Serialize( XmlNodeRef &node,bool bLoading );

	//////////////////////////////////////////////////////////////////////////
	// Child layers.
	//////////////////////////////////////////////////////////////////////////
	void AddChild( CObjectLayer *pLayer );
	void RemoveChild( CObjectLayer *pLayer );
	int	GetChildCount() const { return m_childLayers.size(); }
	CObjectLayer* GetChild( int index ) const;
	CObjectLayer* GetParent() const { return m_parent; }

	//! Check if specified layer is direct or indirect parent of this layer.
	bool IsChildOf( CObjectLayer *pParent );
	//! Find child layer with specified GUID.
	//! Search done recursively, so it find child layer in any depth.
	CObjectLayer* FindChildLayer( REFGUID guid );
	//////////////////////////////////////////////////////////////////////////

	bool IsParentExternal() const;

	//////////////////////////////////////////////////////////////////////////
	// User interface support.
	//////////////////////////////////////////////////////////////////////////
	void Expand( bool bExpand );
	bool IsExpanded() const { return m_expanded; };

private:
	friend CObjectLayerManager;
	//! GUID of this layer.
	GUID m_guid;
	//! Layer Name.
	CString m_name;

	// Layer state.
	//! If true Object of this layer is hidden.
	bool m_hidden;
	//! If true Object of this layer is frozen (not selectable).
	bool m_frozen;
	//! True if this layer can be deleted.
	bool m_removable;
	//! True if layer is stored externally to project file.
	bool m_external;
	//! True if objects on this layer must be exported to the game.
	bool m_exportable;

	//! True when layer is expanded in GUI. (Should not be serialized)
	bool m_expanded;

	// List of child layers.
	typedef std::vector<TSmartPtr<CObjectLayer> > ChildLayers;
	ChildLayers m_childLayers;

	//! Pointer to parent layer.
	CObjectLayer *m_parent;
	//! Parent layer GUID.
	GUID m_parentGUID;

	// Object manager owning this layer.
	CObjectManager *m_pObjectManager;

	//! Manager of object layers.
	CObjectLayerManager* m_pLayerManager;
};

#endif // __objectlayer_h__
