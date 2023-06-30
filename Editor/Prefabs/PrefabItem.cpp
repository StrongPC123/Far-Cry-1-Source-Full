////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PrefabItem.cpp
//  Version:     v1.00
//  Created:     10/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PrefabItem.h"

#include "PrefabLibrary.h"
#include "BaseLibraryManager.h"

#include "Settings.h"
#include "Grid.h"

#include "Objects\ObjectManager.h"
#include "Objects\PrefabObject.h"
#include "Objects\SelectionGroup.h"

//////////////////////////////////////////////////////////////////////////
CPrefabItem::CPrefabItem()
{
}

//////////////////////////////////////////////////////////////////////////
CPrefabItem::~CPrefabItem()
{

}

//////////////////////////////////////////////////////////////////////////
void CPrefabItem::Serialize( SerializeContext &ctx )
{
	CBaseLibraryItem::Serialize( ctx );
	XmlNodeRef node = ctx.node;
	if (ctx.bLoading)
	{
		XmlNodeRef objects = node->findChild( "Objects" );
		if (objects)
		{
			m_objectsNode = objects;
		}
	}
	else
	{
		if (m_objectsNode)
			node->addChild( m_objectsNode );
	}
}

//////////////////////////////////////////////////////////////////////////
void CPrefabItem::Update()
{
	// Mark library as modified.
	GetLibrary()->SetModified();
}

//////////////////////////////////////////////////////////////////////////
void CPrefabItem::MakeFromSelection( CSelectionGroup &fromSelection )
{
	IObjectManager *pObjMan = GetIEditor()->GetObjectManager();
	CSelectionGroup selection;
	
	//////////////////////////////////////////////////////////////////////////
	// Clone selected objects, without changes thier names.
	bool bPrevGenUniqNames = pObjMan->EnableUniqObjectNames( false );
	fromSelection.Clone( selection );
	pObjMan->EnableUniqObjectNames( bPrevGenUniqNames );

	// Snap center to grid.
	Vec3 vCenter = gSettings.pGrid->Snap( selection.GetBounds().min );

	//////////////////////////////////////////////////////////////////////////
	// Transform all objects in selection into local space of prefab.
	Matrix44 invParentTM;
	invParentTM.SetIdentity();
	invParentTM.SetTranslationOLD( vCenter );
	invParentTM.Invert44();

	int i;
	for (i = 0; i < selection.GetCount(); i++)
	{
		CBaseObject *pObj = selection.GetObject(i);
		Matrix44 localTM = pObj->GetWorldTM() * invParentTM;
		pObj->SetLocalTM( localTM );
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Save all objects in flat selection to XML.
	CSelectionGroup flatSelection;
	selection.FlattenHierarchy( flatSelection );

	m_objectsNode = new CXmlNode("Objects");
	CObjectArchive ar( pObjMan,m_objectsNode,false );
	for (i = 0; i < flatSelection.GetCount(); i++)
	{
		ar.SaveObject( flatSelection.GetObject(i) );
	}

	//////////////////////////////////////////////////////////////////////////
	// Delete all objects in cloned flat selection.
	for (i = 0; i < flatSelection.GetCount(); i++)
	{
		pObjMan->DeleteObject( flatSelection.GetObject(i) );
	}

	CUndo undo( "Make Prefab" );
	//////////////////////////////////////////////////////////////////////////
	// Create prefab object.
	CBaseObject *pObj = pObjMan->NewObject( PREFAB_OBJECT_CLASS_NAME );
	if (pObj && pObj->IsKindOf(RUNTIME_CLASS(CPrefabObject)))
	{
		CPrefabObject *pPrefabObj = (CPrefabObject*)pObj;

		pPrefabObj->SetUniqName( GetName() );
		pPrefabObj->SetPos( vCenter );
		pPrefabObj->SetPrefab( this,false );
	}
	else if (pObj)
		pObjMan->DeleteObject( pObj );

	GetLibrary()->SetModified();
}
