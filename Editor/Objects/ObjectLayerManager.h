////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   objectlayermanager.h
//  Version:     v1.00
//  Created:     9/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Manages objects layers.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __objectlayermanager_h__
#define __objectlayermanager_h__
#pragma once

#include "ObjectLayer.h"

class CObjectManager;
class CObjectArchive;

/** Manager of objects layers.
		Instance of these hosted by CObjectManager class.
*/
class CObjectLayerManager
{
public:
	enum EUpdateType {
		ON_LAYER_ADD,
		ON_LAYER_REMOVE,
		ON_LAYER_MODIFY,
		ON_LAYER_SELECT,
		ON_LAYER_UPDATEALL
	};
	//! Update callback, first integer is one of EUpdateType enums.
	typedef Functor2<int,CObjectLayer*> LayersUpdateCallback;

	CObjectLayerManager( CObjectManager *pObjectManager );

	//! Creates a new layer, and associate it with layer manager.
	CObjectLayer* CreateLayer();
	//! Add already created layer to manager.
	void AddLayer( CObjectLayer *pLayer );
	//! Delete layer from layer manager.
	void DeleteLayer( CObjectLayer *pLayer );
	//! Delete all layers from layer manager.
	void ClearLayers();

	//! Find layer by layer GUID.
	CObjectLayer* FindLayer( REFGUID guid ) const;

	//! Search for layer by name.
	CObjectLayer* FindLayerByName( const CString &layerName ) const;

	//! Get a list of all managed layers.
	void GetLayers( std::vector<CObjectLayer*> &layers ) const;

	//! Set this layer is current.
	void SetCurrentLayer( CObjectLayer* pCurrLayer );
	CObjectLayer* GetCurrentLayer() const { return m_pCurrentLayer; };

	//! Return root main layer.
	CObjectLayer* GetMainLayer() const { return m_pMainLayer; }

	//! Associate On Layers update listener.
	//! This callback will be called everytime layers are added/removed or modified.
	void AddUpdateListener( const LayersUpdateCallback &cb );
	//! Remove update listeners.
	void RemoveUpdateListener( const LayersUpdateCallback &cb );

	//! Called when some layers gets modified.
	void NotifyLayerChange( CObjectLayer *pLayer );

	//! Get pointer to object manager who owns this layer manager.
	CObjectManager* GetObjectManager() const { return m_pObjectManager; }

	//! Export layer to objects archive.
	void ExportLayer( CObjectLayer *pLayer,CObjectArchive &ar,bool bExportExternalChilds );
	//! Import layer from objects archive.
	CObjectLayer* ImportLayer( CObjectArchive &ar,bool bResolveObject );

	// Serialize layer manager (called from object manager).
	void	Serialize( CObjectArchive &ar,bool bIgnoreExternalLayers );

	//! Resolve links between layers.
	void ResolveLayerLinks();

private:
	void SaveExternalLayer( CObjectLayer *pLayer,CObjectArchive &archive );
	void LoadExternalLayer( const CString &layerName,CObjectArchive &archive );
	void NotifyListeners( EUpdateType type,CObjectLayer *pLayer );

	//////////////////////////////////////////////////////////////////////////
	//! Map of layer GUID to layer pointer.
	typedef std::map<GUID,TSmartPtr<CObjectLayer>,guid_less_predicate> LayersMap;		
	LayersMap m_layersMap;

	//! Pointer to currently active layer.
	TSmartPtr<CObjectLayer>	m_pCurrentLayer;
	//! Main layer, root for all other layers.
	TSmartPtr<CObjectLayer> m_pMainLayer;
	//////////////////////////////////////////////////////////////////////////

	CFunctorsList<LayersUpdateCallback> m_listeners;
	CObjectManager *m_pObjectManager;
	//! Layers path relative to level folder.
	CString m_layersPath;

	bool m_bOverwriteDuplicates;
};

#endif // __objectlayermanager_h__
