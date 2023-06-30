////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particleitem.h
//  Version:     v1.00
//  Created:     17/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __particleitem_h__
#define __particleitem_h__
#pragma once

#include "BaseLibraryItem.h"
#include <I3dEngine.h>

/*! CParticleItem contain definition of particle system spawning parameters.
 *	
 */
class CRYEDIT_API CParticleItem : public CBaseLibraryItem
{
public:
	CParticleItem();
	~CParticleItem();

	virtual void SetName( const CString &name );
	void Serialize( SerializeContext &ctx );

	//////////////////////////////////////////////////////////////////////////
	// Child particle systems.
	//////////////////////////////////////////////////////////////////////////
	//! Get number of sub Particles childs.
	int GetChildCount() const;
	//! Get sub Particles child by index.
	CParticleItem* GetChild( int index ) const;
	//! Adds a new sub Particles.
	void AddChild( CParticleItem *mtl );
	//! Remove specific sub Particles
	void RemoveChild( CParticleItem *mtl );
	//! Remove all sub Particles.
	void ClearChilds();
	//! Insert sub particles in between other particles.
	//void InsertChild( int slot,CParticleItem *mtl );
	//! Find slot where sub Particles stored.
	//! @retun slot index if Particles found, -1 if Particles not found.
	int  FindChild( CParticleItem *mtl );
	//! Returns parent Particles.
	CParticleItem* GetParent() const;

	void GenerateIdRecursively();

	//! Called when new patricle effect created.
	void SetDefaults();

	//! Called after particle parameters where updated.
	void Update();

	//! Get particle effect assigned to this particle item.
	IParticleEffect* GetEffect() const;

	virtual void GatherUsedResources( CUsedResources &resources );

	void GetFrom3DEngine();

private:
	IParticleEffect_AutoPtr m_pEffect;

	//! Parent of this material (if this is sub material).
	CParticleItem *m_pParentParticles;
	//! Array of sub particle items.
	std::vector<TSmartPtr<CParticleItem> > m_childs;
};

#endif // __particleitem_h__

