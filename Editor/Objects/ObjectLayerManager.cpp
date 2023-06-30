////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   objectlayermanager.cpp
//  Version:     v1.00
//  Created:     9/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ObjectLayerManager.h"
#include "ObjectLoader.h"
#include "ObjectManager.h"
#include "GameEngine.h"
#include "ErrorReport.h"
#include "Settings.h"

#define LAYER_FILE_EXTENSION ".lyr"

//////////////////////////////////////////////////////////////////////////
CObjectLayerManager::CObjectLayerManager( CObjectManager *pObjectManager )
{
	m_pObjectManager = pObjectManager;
	m_layersPath = "Layers\\";
	
	// Create main root level.
	m_pMainLayer = new CObjectLayer( this );
	m_layersMap[m_pMainLayer->GetGUID()] = m_pMainLayer;
	m_pMainLayer->SetRemovable(false);
	m_pMainLayer->SetExternal(false);
	m_pMainLayer->SetName( "Main" );
	m_pMainLayer->Expand(true);
	m_pCurrentLayer = m_pMainLayer;
	m_bOverwriteDuplicates = false;
}

void CObjectLayerManager::GetLayers( std::vector<CObjectLayer*> &layers ) const
{
	layers.clear();
	layers.reserve( m_layersMap.size() );
	for (LayersMap::const_iterator it = m_layersMap.begin(); it != m_layersMap.end(); ++it)
	{
		layers.push_back( it->second );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::ClearLayers()
{
	// Erase all removable layers.
	LayersMap::iterator it,itnext;
	for (LayersMap::iterator it = m_layersMap.begin(); it != m_layersMap.end(); it = itnext)
	{
		CObjectLayer *pLayer = it->second;
		itnext = it;
		itnext++;
		
		if (pLayer->IsRemovable())
		{
			NotifyListeners( ON_LAYER_REMOVE,pLayer );
			m_layersMap.erase( it );
		}
	}
	m_pCurrentLayer = 0;
	if (!m_layersMap.empty())
	{
		SetCurrentLayer( m_layersMap.begin()->second );
	}
	NotifyListeners( ON_LAYER_UPDATEALL,NULL );
}

//////////////////////////////////////////////////////////////////////////
CObjectLayer* CObjectLayerManager::CreateLayer()
{
	CObjectLayer *pLayer = new CObjectLayer( this );
	m_layersMap[pLayer->GetGUID()] = pLayer;
	//m_pMainLayer->AddChild( pLayer );
	NotifyListeners( ON_LAYER_ADD,pLayer );
//	pLayer->SetExternal(true);
	return pLayer;
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::AddLayer( CObjectLayer *pLayer )
{
	assert(pLayer);

	if (!m_bOverwriteDuplicates)
	{
		CObjectLayer *pPrevLayer = FindLayerByName( pLayer->GetName() );
		if (pPrevLayer)
		{
			CErrorRecord err;
			err.error.Format( _T("Duplicate Layer Name <%s>"),(const char*)pLayer->GetName() );
			err.severity = CErrorRecord::ESEVERITY_ERROR;
			GetIEditor()->GetErrorReport()->ReportError(err);
			return;
		}

		if (m_layersMap.find(pLayer->GetGUID()) != m_layersMap.end())
		{
			pPrevLayer = FindLayer(pLayer->GetGUID());

			CErrorRecord err;
			err.error.Format( _T("Duplicate Layer GUID,Layer <%s> collides with layer <%s>"),
				(const char*)pPrevLayer->GetName(),(const char*)pLayer->GetName() );
			err.severity = CErrorRecord::ESEVERITY_ERROR;
			GetIEditor()->GetErrorReport()->ReportError(err);
			return;
		}
	}

	m_layersMap[pLayer->GetGUID()] = pLayer;
	NotifyListeners( ON_LAYER_ADD,pLayer );
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::DeleteLayer( CObjectLayer *pLayer )
{
	assert( pLayer );
	// cannot remove non removable layer.
	if (!pLayer->IsRemovable())
		return;

	// First delete all child layers.
	for (int i = 0; i < pLayer->GetChildCount(); i++)
	{
		DeleteLayer( pLayer->GetChild(i) );
	}

	// prevent reference counted layer to be released before this function ends.
	TSmartPtr<CObjectLayer> pLayerHolder = pLayer;

	if (pLayer->GetParent())
		pLayer->GetParent()->RemoveChild( pLayer );

	// Delete all objects for this layer.
	std::vector<CBaseObjectPtr> objects;
	m_pObjectManager->GetAllObjects( objects );
	for (int k = 0; k < objects.size(); k++)
	{
		if (objects[k]->GetLayer() == pLayer)
			m_pObjectManager->DeleteObject(objects[k]);
	}

	if (m_pCurrentLayer == pLayer)
		m_pCurrentLayer = pLayer->GetParent();
	m_layersMap.erase( pLayer->GetGUID() );

	if (!m_pCurrentLayer)
		m_pCurrentLayer = m_pMainLayer;

	NotifyListeners( ON_LAYER_REMOVE,pLayer );
	NotifyListeners( ON_LAYER_SELECT,m_pCurrentLayer );
}

//////////////////////////////////////////////////////////////////////////
CObjectLayer* CObjectLayerManager::FindLayer( REFGUID guid ) const
{
	CObjectLayer *pLayer = stl::find_in_map( m_layersMap,guid,(CObjectLayer*)0 );
	return pLayer;
}

//////////////////////////////////////////////////////////////////////////
CObjectLayer* CObjectLayerManager::FindLayerByName( const CString &layerName ) const
{
	for (LayersMap::const_iterator it = m_layersMap.begin(); it != m_layersMap.end(); ++it)
	{
		if (stricmp(it->second->GetName(),layerName) == 0)
			return it->second;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::NotifyLayerChange( CObjectLayer *pLayer )
{
	assert(pLayer);
	// check if this layers is already registered in manager.
	if (m_layersMap.find(pLayer->GetGUID()) != m_layersMap.end())
	{
		// Notify listeners of this change.
		NotifyListeners( ON_LAYER_MODIFY,pLayer );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::AddUpdateListener( const LayersUpdateCallback &cb )
{
	m_listeners.Add( cb );
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::RemoveUpdateListener( const LayersUpdateCallback &cb )
{
	m_listeners.Remove(cb);
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::NotifyListeners( EUpdateType type,CObjectLayer *pLayer )
{
	m_listeners.Call( type,pLayer );
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::Serialize( CObjectArchive &ar,bool bIgnoreExternalLayers )
{
	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		ClearLayers();
		// Loading from archive.
		XmlNodeRef layers = xmlNode->findChild( "ObjectLayers" );
		if (layers)
		{
			for (int i = 0; i < layers->getChildCount(); i++)
			{
				XmlNodeRef layerNode = layers->getChild(i);
				TSmartPtr<CObjectLayer> pLayer = new CObjectLayer( this );
				pLayer->Serialize( layerNode,true );
				if (pLayer->IsExternal())
				{
					if (!bIgnoreExternalLayers)
						LoadExternalLayer( pLayer->GetName(),ar );
				}
				else
				{
					// Check if this layer is alyeady loaded.
					if (!FindLayer( pLayer->GetGUID() ))
						AddLayer( pLayer );
				}
			}
			
			GUID currentLayerGUID;
			if (layers->getAttr( "CurrentGUID",currentLayerGUID ))
			{
				CObjectLayer *pLayer = FindLayer( currentLayerGUID );
				if (pLayer)
					SetCurrentLayer( pLayer );
			}
		}
		ResolveLayerLinks();
	}
	else
	{
		// Saving to archive.
		XmlNodeRef layersNode = xmlNode->newChild( "ObjectLayers" );

		//! Store currently selected layer.
		if (m_pCurrentLayer)
		{
			layersNode->setAttr( "Current",m_pCurrentLayer->GetName() );
			layersNode->setAttr( "CurrentGUID",m_pCurrentLayer->GetGUID() );
		}

		for (LayersMap::const_iterator it = m_layersMap.begin(); it != m_layersMap.end(); ++it)
		{
			CObjectLayer *pLayer = it->second;
			if (!pLayer->IsRemovable())
				continue;

			XmlNodeRef layerNode = layersNode->newChild("Layer");
			pLayer->Serialize( layerNode,false );
			
			if (pLayer->IsExternal() && !bIgnoreExternalLayers)
			{
				// Save external level additionally to file.
				SaveExternalLayer( pLayer,ar );
			}
		}

	}
	ar.node = xmlNode;
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::SaveExternalLayer( CObjectLayer *pLayer,CObjectArchive &ar )
{
	// Form file name from layer name.
	CString path = Path::AddBackslash( GetIEditor()->GetGameEngine()->GetLevelPath() ) + m_layersPath;
	CString file = path + pLayer->GetName() + LAYER_FILE_EXTENSION;

	CFileUtil::CreateDirectory( path );

	// Make a backup of file.
	if (gSettings.bBackupOnSave)
		CFileUtil::BackupFile( file );

	// Serialize this layer.
	XmlNodeRef rootFileNode = new CXmlNode( "ObjectLayer" );
	XmlNodeRef layerNode = rootFileNode->newChild( "Layer" );
	ar.node = layerNode;
	ExportLayer( pLayer,ar,false ); // Also save all childs but not external layers.
	// Save xml file to disk.
	rootFileNode->saveToFile( file );
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::LoadExternalLayer( const CString &layerName,CObjectArchive &archive )
{
	// Form file name from layer name.
	CString file = Path::AddBackslash( GetIEditor()->GetGameEngine()->GetLevelPath() );
	file += m_layersPath + layerName + LAYER_FILE_EXTENSION;

	XmlParser parser;
	XmlNodeRef root = parser.parse( file );
	if (!root)
	{
		CErrorRecord err;
		err.error.Format( _T("Failed to import external object layer <%s> from File %s"),(const char*)layerName,(const char*)file );
		err.file = file;
		err.severity = CErrorRecord::ESEVERITY_ERROR;
		archive.ReportError(err);

		return;
	}

	XmlNodeRef layerDesc = root->findChild("Layer");
	if (layerDesc)
	{
		XmlNodeRef prevRoot = archive.node;
		archive.node = layerDesc;
		m_bOverwriteDuplicates = true;
		ImportLayer( archive,false );
		m_bOverwriteDuplicates = false;
		archive.node = prevRoot;
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::ExportLayer( CObjectLayer *pLayer,CObjectArchive &ar,bool bExportExternalChilds )
{
	pLayer->Serialize( ar.node,false );

	XmlNodeRef orgNode = ar.node;
	XmlNodeRef layerObjects = ar.node->newChild("LayerObjects");
	ar.node = layerObjects;

	std::vector<CBaseObject*> objects;
	GetIEditor()->GetObjectManager()->GetObjects( objects,pLayer );
	// Save all objects to XML.
	for (int i = 0; i < objects.size(); i++)
	{
		ar.SaveObject( objects[i] );
	}

	if (pLayer->GetChildCount() > 0)
	{
		XmlNodeRef childLayers = orgNode->newChild("ChildLayers");
		// Export layer childs.
		for (int i = 0; i < pLayer->GetChildCount(); i++)
		{
			CObjectLayer *pChildLayer = pLayer->GetChild(i);
			if (pChildLayer->IsExternal() && !bExportExternalChilds)
				continue;

			XmlNodeRef childLayer = childLayers->newChild("Layer");
			ar.node = childLayer;
			ExportLayer( pChildLayer,ar,bExportExternalChilds );
		}
	}

	ar.node = orgNode;
}

//////////////////////////////////////////////////////////////////////////
CObjectLayer* CObjectLayerManager::ImportLayer( CObjectArchive &ar,bool bResolveObjects )
{
	XmlNodeRef layerNode = ar.node;
	if (stricmp(layerNode->getTag(),"Layer") != 0)
	{
		CErrorRecord err;
		err.error.Format( _T("Not a valid Layer XML Node %s, Must be \"Layer\""),(const char*)layerNode->getTag() );
		err.severity = CErrorRecord::ESEVERITY_WARNING;
		ar.ReportError(err);

		//Warning( "Not a valid Layer XML Node" );
		// Bad layer,Import fails.
		return 0;
	}
	TSmartPtr<CObjectLayer> pLayer = new CObjectLayer( this );
	pLayer->Serialize( layerNode,true );

	CString layerName = pLayer->GetName();

	bool bRemoveLayer = false;
	CObjectLayer *pPrevLayer = FindLayer( pLayer->GetGUID() );
	if (pPrevLayer)
	{
		if (m_bOverwriteDuplicates)
		{
			pLayer = pPrevLayer;
			pLayer->Serialize( layerNode,true ); // Serialize it again.
		}
		else
		{
			// Duplicate layer imported.
			CString layerName = pPrevLayer->GetName();
			bRemoveLayer = true;
			CString str;
			str.Format( _T("Replace Layer %s?\r\nAll object of replaced layer will be deleted."),(const char*)layerName );
			if (AfxGetMainWnd()->MessageBox( str,_T("Confirm Replace Layer"),MB_YESNO|MB_ICONQUESTION) == IDNO)
			{
				return 0;
			}
			DeleteLayer(pPrevLayer);
		}
	}

	if (pLayer)
	{
		AddLayer( pLayer );
	}

	XmlNodeRef layerObjects = layerNode->findChild("LayerObjects");
	if (layerObjects)
	{
		int numObjects = layerObjects->getChildCount();
		if (bResolveObjects)
			m_pObjectManager->StartObjectsLoading( numObjects );
		//ar.node = layerObjects;
		//m_pObjectManager->LoadObjects( ar,false );
		ar.LoadObjects( layerObjects );

		if (bResolveObjects)
			m_pObjectManager->EndObjectsLoading();
	}

	XmlNodeRef childLayers = layerNode->findChild("ChildLayers");
	if (childLayers)
	{
		// Import child layers.
		for (int i = 0; i < childLayers->getChildCount(); i++)
		{
			XmlNodeRef childLayer = childLayers->getChild(i);
			ar.node = childLayer;
			CObjectLayer *pChildLayer = ImportLayer( ar,false );
			if (pChildLayer)
			{
				// Import child layers.
				pLayer->AddChild( pChildLayer );
			}
		}
		ar.node = layerNode;
	}

	if (bResolveObjects)
	{
		ResolveLayerLinks();
		ar.ResolveObjects();
	}

	return pLayer;
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::SetCurrentLayer( CObjectLayer* pCurrLayer )
{
	assert( pCurrLayer );
	if (pCurrLayer == m_pCurrentLayer)
		return;
	m_pCurrentLayer = pCurrLayer;
	NotifyListeners( ON_LAYER_SELECT,m_pCurrentLayer );
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayerManager::ResolveLayerLinks()
{
	for (LayersMap::const_iterator it = m_layersMap.begin(); it != m_layersMap.end(); ++it)
	{
		CObjectLayer *pLayer = it->second;
		if (!pLayer->IsRemovable())
			continue;

		// Try to connect to parent layer.
		CObjectLayer *pNewParent = FindLayer( pLayer->m_parentGUID );

		/*
		if (pNewParent == NULL && pLayer->GetParent() == m_pMainLayer)
		{
			// If parent is already main layer, skip.
			continue;
		}
		*/
		
		if (pLayer->GetParent() != NULL && pLayer->GetParent() != pNewParent)
		{
			// Deatch from old parent layer.
			pLayer->GetParent()->RemoveChild( pLayer );
		}
		if (pNewParent)
		{
			// Attach to new parent layer.
			pNewParent->AddChild( pLayer );
		}

		/*
		if (pLayer->GetParent() == NULL)
		{
			// all removable layers without parent must be attached to main layer.
			m_pMainLayer->AddChild( pLayer );
		}
		*/
	}
	NotifyListeners( ON_LAYER_UPDATEALL,0 );
}