////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   centityprototype.cpp
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EntityPrototype.h"
#include "Objects\EntityScript.h"

//////////////////////////////////////////////////////////////////////////
// CEntityPrototype implementation.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CEntityPrototype::CEntityPrototype()
{
}

//////////////////////////////////////////////////////////////////////////
CEntityPrototype::~CEntityPrototype()
{
}

//////////////////////////////////////////////////////////////////////////
void CEntityPrototype::SetEntityClassName( const CString &className )
{
	m_className = className;

	// clear old properties.
	m_properties = 0;

	// Find this script in registry.
	m_script = CEntityScriptRegistry::Instance()->Find( m_className );
	if (m_script)
	{
		// Assign properties.
		if (!m_script->IsValid())
		{
			if (!m_script->Load())
			{
				Error( "Failed to initialize Entity Script %s from %s",(const char*)m_className,(const char*)m_script->GetFile() );
				m_script = 0;
			}
		}
	}
	if (m_script)
	{
		CVarBlock *scriptProps = m_script->GetProperties();
		if (scriptProps)
			m_properties = scriptProps->Clone(true);
	}
};

//////////////////////////////////////////////////////////////////////////
void CEntityPrototype::Reload()
{
	if (m_script)
	{
		CVarBlockPtr oldProperties = m_properties;
		m_properties = 0;
		m_script->Reload();
		CVarBlock *scriptProps = m_script->GetProperties();
		if (scriptProps)
			m_properties = scriptProps->Clone(true);

		if (m_properties != NULL && oldProperties != NULL)
			m_properties->CopyValuesByName(oldProperties);
	}
	Update();
}

//////////////////////////////////////////////////////////////////////////
CVarBlock* CEntityPrototype::GetProperties()
{
	return m_properties;
}

//////////////////////////////////////////////////////////////////////////
CEntityScript* CEntityPrototype::GetScript()
{
	return m_script;
}

//////////////////////////////////////////////////////////////////////////
void CEntityPrototype::Serialize( SerializeContext &ctx )
{
	CBaseLibraryItem::Serialize( ctx );
	XmlNodeRef node = ctx.node;
	if (ctx.bLoading)
	{
		CString className;
		// Loading
		node->getAttr( "Class",className );
		node->getAttr( "Description",m_description );

		SetEntityClassName( className );

		if (m_properties)
		{
			// Serialize properties.
			XmlNodeRef props = node->findChild( "Properties" );
			if (props)
			{
				m_properties->Serialize( props,ctx.bLoading );
			}
		}
	}
	else
	{
		// Saving.
		node->setAttr( "Class",m_className );
		node->setAttr( "Description",m_description );
		if (m_properties)
		{
			// Serialize properties.
			XmlNodeRef props = node->newChild( "Properties" );
			m_properties->Serialize( props,ctx.bLoading );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityPrototype::AddUpdateListener( UpdateCallback cb )
{
	m_updateListeners.push_back(cb);
}

//////////////////////////////////////////////////////////////////////////
void CEntityPrototype::RemoveUpdateListener( UpdateCallback cb )
{
	std::list<UpdateCallback>::iterator it = std::find(m_updateListeners.begin(),m_updateListeners.end(),cb);
	if (it != m_updateListeners.end())
		m_updateListeners.erase(it);
}

//////////////////////////////////////////////////////////////////////////
void CEntityPrototype::Update()
{
	for (std::list<UpdateCallback>::iterator pCallback = m_updateListeners.begin(); pCallback != m_updateListeners.end(); ++pCallback)
	{
		// Call callback.
		(*pCallback)();
	}
}