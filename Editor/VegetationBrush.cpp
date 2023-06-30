////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   vegetationbrush.cpp
//  Version:     v1.00
//  Created:     20/2/2002 by Timur.
//  Compilers:   Visual C++.NET
//  Description: Vegetation Brush Implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Heightmap.h"
#include "VegetationBrush.h"

#include "VegetationMap.h"

//////////////////////////////////////////////////////////////////////////
CVegetationBrush::CVegetationBrush()
{
	m_elevationMin = 0;
	m_elevationMax = 0;
	m_slopeMin = 0;
	m_slopeMax = 100;
	m_density = 10;
	m_bSelected = false;
	m_bHidden = false;
}

CVegetationBrush::~CVegetationBrush()
{
}

void CVegetationBrush::Serialize( XmlNodeRef &node,bool bLoading )
{
	if (bLoading)
	{
		// Loading.
		node->getAttr( "Name",m_name );

		node->getAttr( "ElevationnMin",m_elevationMin );
		node->getAttr( "ElevationnMax",m_elevationMax );

		node->getAttr( "SlopeMin",m_slopeMin );
		node->getAttr( "SlopeMax",m_slopeMax );

		node->getAttr( "Density",m_density );
		
		CVegetationMap *vegetationMap = GetIEditor()->GetHeightmap()->GetVegetationMap();
		assert( vegetationMap != 0 );

		m_objects.clear();
		XmlNodeRef objectsNode = node->findChild("Objects");
		if (objectsNode)
		{
			for (int i = 0; i < objectsNode->getChildCount(); i++)
			{
				CVegetationObject *obj = vegetationMap->CreateObject();
				if (!obj)
					continue;
				obj->Serialize( objectsNode->getChild(i),bLoading );
				obj->SetElevation( m_elevationMin,m_elevationMax );
				obj->SetSlope( m_slopeMin,m_slopeMax );
				obj->SetCategory( m_name );
				//m_objects.push_back(obj);
			}
		}
	}
	else
	{
		/*
		// Saving.
		node->setAttr( "Name",m_name );

		node->setAttr( "ElevationnMin",m_elevationMin );
		node->setAttr( "ElevationnMax",m_elevationMax );

		node->setAttr( "SlopeMin",m_slopeMin );
		node->setAttr( "SlopeMax",m_slopeMax );

		node->setAttr( "Density",m_density );

		XmlNodeRef objectsNode = node->newChild("Objects");
		for (int i = 0; i < GetObjectCount(); i++)
		{
			XmlNodeRef objNode = objectsNode->newChild("Object");
			GetObject(i)->Serialize( objNode,bLoading );
		}
		*/
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationBrush::AddObject( CVegetationObject *obj )
{
	m_objects.push_back( obj );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationBrush::RemoveObject( CVegetationObject *obj )
{
	m_objects.erase( std::find(m_objects.begin(),m_objects.end(),obj) );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationBrush::CopyFrom( CVegetationBrush *brush )
{
	m_name = brush->m_name;

	m_elevationMin = brush->m_elevationMin;
	m_elevationMax = brush->m_elevationMax;

	m_slopeMin = brush->m_slopeMin;
	m_slopeMax = brush->m_slopeMax;

	m_density = brush->m_density;
}

/*
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Undo object for Vegetation brush.
class CUndoVegetationBrush : public IUndoObject
{
public:
	CUndoVegetationBrush( XmlNodeRef &node,const CString &undoValue )
	{
		assert( node != 0 );
		m_node = node;
		m_undoValue = undoValue;
	}
protected:
	virtual void Release() { delete this; };
	virtual int GetSize() { return sizeof(*this); };
	virtual const char* GetDescription() { return "Brush Modify"; };

	virtual void Undo( bool bUndo )
	{
		if (bUndo)
		{
			m_node->getAttr( VALUE_ATTR,m_redoValue );
		}
		//OverrideNode( m_node,m_undo );
		m_node->setAttr( VALUE_ATTR,m_undoValue );
		
		if (m_sCurrentPropertyWindow)
			m_sCurrentPropertyWindow->ReloadValues();
	}
	virtual void Redo()
	{
		m_node->setAttr( VALUE_ATTR,m_redoValue );
		if (m_sCurrentPropertyWindow)
			m_sCurrentPropertyWindow->ReloadValues();
	}
	
	//! Recursivelly override target Xml Node attributes with values from source Xml node.
	void OverrideNode( XmlNodeRef &trg,const XmlNodeRef &src )
	{
		trg->setAttributes( src->getAttributes() );
		for (int i = 0; i < trg->getChildCount(); i++)
		{
			if (i < src->getChildCount())
			{
				OverrideNode( trg->getChild(i),src->getChild(i) );
			}
		}
	}

private:
	TSmartPtr<CVegetationBrush> brush;
	XmlNodeRef m_undo;
	XmlNodeRef m_redo;
};
*/

//////////////////////////////////////////////////////////////////////////
void CVegetationBrush::SetHidden( bool bHidden )
{
	m_bHidden = bHidden;
	for (int i = 0; i < GetObjectCount(); i++)
	{
		CVegetationObject *obj = GetObject(i);
		obj->SetInUse( !bHidden );
	}
}