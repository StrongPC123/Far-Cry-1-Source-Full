////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PrefabItem.h
//  Version:     v1.00
//  Created:     10/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PrefabItem_h__
#define __PrefabItem_h__
#pragma once

#include "BaseLibraryItem.h"
#include <I3dEngine.h>

/*! CPrefabItem contain definition of particle system spawning parameters.
 *	
 */
class CRYEDIT_API CPrefabItem : public CBaseLibraryItem
{
public:
	CPrefabItem();
	~CPrefabItem();

	void Serialize( SerializeContext &ctx );

	//////////////////////////////////////////////////////////////////////////
	// Make prefab from selection of objects.
	void MakeFromSelection( CSelectionGroup &selection );

	//! Called after particle parameters where updated.
	void Update();
	//! Returns xml node containing prefab objects.
	XmlNodeRef GetObjectsNode() { return m_objectsNode; };

private:
	//! Parent of this material (if this is sub material).
	CPrefabItem *m_pParentParticles;
	//! Array of sub particle items.
	std::vector<TSmartPtr<CPrefabItem> > m_childs;
	XmlNodeRef m_objectsNode;
};

#endif // __PrefabItem_h__


