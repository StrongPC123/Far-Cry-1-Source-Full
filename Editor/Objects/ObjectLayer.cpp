////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   objectlayer.cpp
//  Version:     v1.00
//  Created:     9/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ObjectLayer.h"

#include "ObjectManager.h"
#include "ObjectLayerManager.h"

#define LAYER_ID(x) ((x)>>16)
#define OBJECT_ID(x) ((x)&0xFFFF)
#define MAKE_ID(layerId,objectId) (((layerId)<<16)|(objectId))

//////////////////////////////////////////////////////////////////////////
CObjectLayer::CObjectLayer( CObjectLayerManager *pLayerManager )
{
	assert( pLayerManager != 0 );
	m_pLayerManager = pLayerManager;
	m_pObjectManager = pLayerManager->GetObjectManager();
	m_hidden = false;
	m_frozen = false;
	m_removable = true;
	m_parent = NULL;
	m_external = false;
	m_exportable = true;
	m_expanded = false;
	ZeroStruct(m_parentGUID);

	CoCreateGuid( &m_guid );
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayer::Serialize( XmlNodeRef &node,bool bLoading )
{
	if (bLoading)
	{
		bool bHidden = m_hidden;
		bool bFrozen = m_frozen;
		bool bExternal = false;
		// Loading.
		node->getAttr( "Name",m_name );
		node->getAttr( "GUID",m_guid );
		node->getAttr( "Hidden",bHidden );
		node->getAttr( "Frozen",bFrozen );
		node->getAttr( "External",bExternal );
		node->getAttr( "Exportable",m_exportable );

		ZeroStruct(m_parentGUID);
		node->getAttr( "ParentGUID",m_parentGUID );

		SetExternal( bExternal );
		SetVisible( !bHidden );
		SetFrozen( bFrozen );
		m_pObjectManager->InvalidateVisibleList();
	}
	else
	{
		// Saving.
		node->setAttr( "Name",m_name );
		node->setAttr( "GUID",m_guid );
		node->setAttr( "Hidden",m_hidden );
		node->setAttr( "Frozen",m_frozen );
		node->setAttr( "External",m_external );
		node->setAttr( "Exportable",m_exportable );
		
		GUID parentGUID = m_parentGUID;
		if (m_parent == m_pLayerManager->GetMainLayer())
			ZeroStruct(parentGUID);
		if (!GuidUtil::IsEmpty(parentGUID))
			node->setAttr( "ParentGUID",parentGUID );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayer::SetName( const CString &name )
{
	m_name = name;
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayer::AddChild( CObjectLayer *pLayer )
{
	assert( pLayer );
	if (IsChildOf(pLayer))
		return;
	if (pLayer->GetParent())
		pLayer->GetParent()->RemoveChild(pLayer);
	stl::push_back_unique( m_childLayers,pLayer );
	pLayer->m_parent = this;
	pLayer->m_parentGUID = GetGUID();
	m_pObjectManager->InvalidateVisibleList();
	
	// Notify layer manager on layer modification.
	m_pLayerManager->NotifyLayerChange(this);
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayer::RemoveChild( CObjectLayer *pLayer )
{
	assert( pLayer );
	pLayer->m_parent = 0;
	ZeroStruct(pLayer->m_parentGUID);
	stl::find_and_erase( m_childLayers,pLayer );
	m_pObjectManager->InvalidateVisibleList();

	// Notify layer manager on layer modification.
	m_pLayerManager->NotifyLayerChange(this);
}

//////////////////////////////////////////////////////////////////////////
CObjectLayer* CObjectLayer::GetChild( int index ) const
{
	assert( index >= 0 && index < m_childLayers.size() );
	return m_childLayers[index];
}

//////////////////////////////////////////////////////////////////////////
CObjectLayer* CObjectLayer::FindChildLayer( REFGUID guid )
{
	if (m_guid == guid)
		return this;

	for (int i = 0; i < GetChildCount(); i++)
	{
		CObjectLayer *pLayer = GetChild(i)->FindChildLayer(guid);
		if (pLayer)
			return pLayer;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CObjectLayer::IsChildOf( CObjectLayer *pParent )
{
	if (m_parent == pParent)
		return true;
	if (m_parent)
	{
		return m_parent->IsChildOf(pParent);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayer::SetVisible( bool b,bool bRecursive )
{
	bool bChange = m_hidden != !b;
	m_hidden = !b;

	if (bChange)
	{
		// Notify layer manager on layer modification.
		m_pLayerManager->NotifyLayerChange(this);
		m_pObjectManager->InvalidateVisibleList();
	}
	if (bRecursive)
	{
		for (int i = 0; i < GetChildCount(); i++)
		{
			GetChild(i)->SetVisible( b,bRecursive );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayer::SetFrozen( bool b,bool bRecursive )
{
	bool bChange = m_frozen != b;
	m_frozen = b;

	if (bChange)
	{
		// Notify layer manager on layer modification.
		m_pLayerManager->NotifyLayerChange(this);
		m_pObjectManager->InvalidateVisibleList();
	}
	if (bRecursive)
	{
		for (int i = 0; i < GetChildCount(); i++)
		{
			GetChild(i)->SetFrozen( b,bRecursive );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectLayer::Expand( bool bExpand )
{
	m_expanded = bExpand;
}

//////////////////////////////////////////////////////////////////////////
bool CObjectLayer::IsParentExternal() const
{
	if (m_parent)
	{
		if (m_parent->IsExternal())
			return true;
		return m_parent->IsParentExternal();
	}
	return false;
}