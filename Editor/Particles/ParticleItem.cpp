////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particleitem.cpp
//  Version:     v1.00
//  Created:     17/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ParticleItem.h"

#include "ParticleLibrary.h"
#include "BaseLibraryManager.h"

//////////////////////////////////////////////////////////////////////////
CParticleItem::CParticleItem()
{
	m_pParentParticles = 0;
	m_pEffect = GetIEditor()->Get3DEngine()->CreateParticleEffect();
}

//////////////////////////////////////////////////////////////////////////
CParticleItem::~CParticleItem()
{
	GetIEditor()->Get3DEngine()->DeleteParticleEffect( m_pEffect );
}

//////////////////////////////////////////////////////////////////////////
void CParticleItem::SetName( const CString &name )
{
	CBaseLibraryItem::SetName( name );
	if (m_pEffect)
		m_pEffect->SetName( GetFullName() );
}

//////////////////////////////////////////////////////////////////////////
void CParticleItem::Serialize( SerializeContext &ctx )
{
	CBaseLibraryItem::Serialize( ctx );
	XmlNodeRef node = ctx.node;
	if (ctx.bLoading)
	{
		// Loading.
		CString texture,geometry,material;
		int process = 0;
		for (int i = 0; i < node->getChildCount(); i++)
		{
			XmlNodeRef paramsNode = node->getChild(i);
			if (!paramsNode->isTag("Params"))
				continue;

			if (!m_pEffect)
				continue;

			ParticleParams &params = m_pEffect->GetParticleParams(process);

			paramsNode->getAttr( "Texture",texture );
			paramsNode->getAttr( "Geometry",geometry );
			paramsNode->getAttr( "Material",material );

			bool bEnabled = true;
			paramsNode->getAttr( "Active",bEnabled );
			m_pEffect->SetEnabled( bEnabled );

			int blendType;
			paramsNode->getAttr( "Flags",params.nParticleFlags );
			paramsNode->getAttr( "BlendType",blendType );
			paramsNode->getAttr( "Focus",params.fFocus );
			paramsNode->getAttr( "Speed",params.fSpeed.value );
			paramsNode->getAttr( "SpeedFadeOut",params.fSpeedFadeOut );
			paramsNode->getAttr( "SpeedAccel",params.fSpeedAccel );
			paramsNode->getAttr( "AirResistance",params.fAirResistance );
			paramsNode->getAttr( "SpeedVar",params.fSpeed.variation );
			paramsNode->getAttr( "Size",params.fSize.value );
			paramsNode->getAttr( "SizeVar",params.fSize.variation );
			paramsNode->getAttr( "SizeSpeed",params.fSizeSpeed );
			paramsNode->getAttr( "SizeFadeIn",params.fSizeFadeIn );
			paramsNode->getAttr( "SizeFadeOut",params.fSizeFadeOut );
			paramsNode->getAttr( "ObjectScale",params.fObjectScale );
			paramsNode->getAttr( "Count",params.nCount );
			paramsNode->getAttr( "LifeTime",params.fLifeTime.value );
			paramsNode->getAttr( "LifeTimeVar",params.fLifeTime.variation );
			paramsNode->getAttr( "FadeInTime",params.fFadeInTime );
			paramsNode->getAttr( "FramesCount",params.nTexAnimFramesCount );
			paramsNode->getAttr( "Tail",params.fTailLenght );
			paramsNode->getAttr( "TailSteps",params.nTailSteps );
			paramsNode->getAttr( "Stretch",params.fStretch );
			paramsNode->getAttr( "RealPhysics",params.bRealPhysics );
			paramsNode->getAttr( "DrawLast",params.nDrawLast );
			paramsNode->getAttr( "Bounciess",params.fBouncenes );
			paramsNode->getAttr( "TurbulenceSize",params.fTurbulenceSize );
			paramsNode->getAttr( "TurbulenceSpeed",params.fTurbulenceSpeed );
			//paramsNode->getAttr( "RandomPosOffset",params.fPosRandomOffset );

			paramsNode->getAttr( "StartColor",params.vColorStart );
			paramsNode->getAttr( "EndColor",params.vColorEnd );
			paramsNode->getAttr( "Rotation",params.vRotation.value );
			paramsNode->getAttr( "RotationVar",params.vRotation.variation );
			paramsNode->getAttr( "InitAngles",params.vInitAngles.value );
			paramsNode->getAttr( "InitAnglesVar",params.vInitAngles.variation );
			paramsNode->getAttr( "Gravity",params.vGravity );
			paramsNode->getAttr( "ChildSpawnPeriod",params.fChildSpawnPeriod );
			paramsNode->getAttr( "ChildSpawnTime",params.fChildSpawnTime );

			paramsNode->getAttr( "PositionOffset",params.vPositionOffset );
			paramsNode->getAttr( "RandomOffset",params.vRandomPositionOffset );
			paramsNode->getAttr( "SpawnDelay",params.fSpawnDelay.value );
			paramsNode->getAttr( "SpawnDelayVar",params.fSpawnDelay.variation );
			paramsNode->getAttr( "EmitterLifeTime",params.fEmitterLifeTime.value );
			paramsNode->getAttr( "EmitterLifeTimeVar",params.fEmitterLifeTime.variation );
			paramsNode->getAttr( "SpawnPeriod",params.fSpawnPeriod );

			IParticleEffect::SoundParams soundParams;
			CString sound;
			paramsNode->getAttr( "Sound",sound );
			paramsNode->getAttr( "SoundVolume",soundParams.volume );
			paramsNode->getAttr( "SoundMinRange",soundParams.minRadius );
			paramsNode->getAttr( "SoundMaxRange",soundParams.maxRadius );
			paramsNode->getAttr( "SoundLoop",soundParams.bLoop );
			paramsNode->getAttr( "SoundOnEverySpawn",soundParams.bOnEverySpawn );
			soundParams.szSound = sound;
			m_pEffect->SetSoundParams( soundParams );

			params.eBlendType = (ParticleBlendType)blendType;

			m_pEffect->SetGeometry( process,geometry );
			m_pEffect->SetTexture( process,texture );
			m_pEffect->SetMaterialName( process,material );

			process++;
		}

		// Serialize childs.

		CParticleManager *pManager = GetIEditor()->GetParticleManager();

		// Serialize childs.
		XmlNodeRef childsNode = node->findChild( "Childs" );
		if (childsNode)
		{
			for (i = 0; i < childsNode->getChildCount(); i++)
			{
				XmlNodeRef xchild = childsNode->getChild(i);
				CParticleItem *pItem = new CParticleItem;
				GetLibrary()->AddItem( pItem );
				SerializeContext childCtx(ctx);
				childCtx.node = xchild;
				pItem->Serialize( childCtx );

				AddChild( pItem );
			}
		}
	}
	else
	{
		int i;
		// Saving.
		for (i = 0; i < IParticleEffect::NUM_PARTICLE_PROCESSES; i++)
		{
			if (!m_pEffect)
				continue;

			ParticleParams &params = m_pEffect->GetParticleParams(i);
			// Save particle params.
			XmlNodeRef paramsNode = node->newChild( "Params" );

			paramsNode->setAttr( "Active",m_pEffect->IsEnabled() );
			paramsNode->setAttr( "Flags",params.nParticleFlags );
			paramsNode->setAttr( "BlendType",(int)params.eBlendType );
			paramsNode->setAttr( "Focus",params.fFocus );
			paramsNode->setAttr( "Speed",params.fSpeed.value );
			paramsNode->setAttr( "SpeedFadeOut",params.fSpeedFadeOut );
			paramsNode->setAttr( "SpeedAccel",params.fSpeedAccel );
			paramsNode->setAttr( "AirResistance",params.fAirResistance );
			paramsNode->setAttr( "SpeedVar",params.fSpeed.variation );
			paramsNode->setAttr( "Size",params.fSize.value );
			paramsNode->setAttr( "SizeVar",params.fSize.variation );
			paramsNode->setAttr( "SizeFadeIn",params.fSizeFadeIn );
			paramsNode->setAttr( "SizeFadeOut",params.fSizeFadeOut );
			paramsNode->setAttr( "SizeSpeed",params.fSizeSpeed );
			paramsNode->setAttr( "ObjectScale",params.fObjectScale );
			paramsNode->setAttr( "Count",params.nCount );
			paramsNode->setAttr( "LifeTime",params.fLifeTime.value );
			paramsNode->setAttr( "LifeTimeVar",params.fLifeTime.variation );
			paramsNode->setAttr( "FadeInTime",params.fFadeInTime );
			paramsNode->setAttr( "FramesCount",params.nTexAnimFramesCount );
			paramsNode->setAttr( "Tail",params.fTailLenght );
			paramsNode->setAttr( "TailSteps",params.nTailSteps );
			paramsNode->setAttr( "Stretch",params.fStretch );
			paramsNode->setAttr( "RealPhysics",params.bRealPhysics );
			paramsNode->setAttr( "DrawLast",params.nDrawLast );
			paramsNode->setAttr( "Bounciess",params.fBouncenes );
			paramsNode->setAttr( "TurbulenceSize",params.fTurbulenceSize );
			paramsNode->setAttr( "TurbulenceSpeed",params.fTurbulenceSpeed );
			//paramsNode->setAttr( "RandomPosOffset",params.fPosRandomOffset );

			paramsNode->setAttr( "StartColor",params.vColorStart );
			paramsNode->setAttr( "EndColor",params.vColorEnd );
			paramsNode->setAttr( "Rotation",params.vRotation );
			paramsNode->setAttr( "RotationVar",params.vRotation.variation );
			paramsNode->setAttr( "InitAngles",params.vInitAngles );
			paramsNode->setAttr( "InitAnglesVar",params.vInitAngles.variation );
			paramsNode->setAttr( "Gravity",params.vGravity );
			paramsNode->setAttr( "ChildSpawnPeriod",params.fChildSpawnPeriod );
			paramsNode->setAttr( "ChildSpawnTime",params.fChildSpawnTime );

			paramsNode->setAttr( "PositionOffset",params.vPositionOffset );
			paramsNode->setAttr( "RandomOffset",params.vRandomPositionOffset );
			paramsNode->setAttr( "SpawnDelay",params.fSpawnDelay.value );
			paramsNode->setAttr( "SpawnDelayVar",params.fSpawnDelay.variation );
			paramsNode->setAttr( "EmitterLifeTime",params.fEmitterLifeTime.value );
			paramsNode->setAttr( "EmitterLifeTimeVar",params.fEmitterLifeTime.variation );
			paramsNode->setAttr( "SpawnPeriod",params.fSpawnPeriod );

			paramsNode->setAttr( "Texture",m_pEffect->GetTexture(i) );
			paramsNode->setAttr( "Geometry",m_pEffect->GetGeometry(i) );
			paramsNode->setAttr( "Material",m_pEffect->GetMaterialName(i) );

			IParticleEffect::SoundParams soundParams;
			m_pEffect->GetSoundParams( soundParams );
			paramsNode->setAttr( "Sound",soundParams.szSound );
			paramsNode->setAttr( "SoundVolume",soundParams.volume );
			paramsNode->setAttr( "SoundMinRange",soundParams.minRadius );
			paramsNode->setAttr( "SoundMaxRange",soundParams.maxRadius );
			paramsNode->setAttr( "SoundLoop",soundParams.bLoop );
			paramsNode->setAttr( "SoundOnEverySpawn",soundParams.bOnEverySpawn );
		}
		// Serialize childs.
		XmlNodeRef childsNode = node->newChild( "Childs" );
		for (i = 0; i < m_childs.size(); i++)
		{
			XmlNodeRef xchild = childsNode->newChild( "Particles" );
			SerializeContext childCtx(ctx);
			childCtx.node = xchild;
			m_childs[i]->Serialize( childCtx );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int CParticleItem::GetChildCount() const
{
	return m_childs.size();
}

//////////////////////////////////////////////////////////////////////////
CParticleItem* CParticleItem::GetChild( int index ) const
{
	assert( index >= 0 && index < m_childs.size() );
	return m_childs[index];
}

//////////////////////////////////////////////////////////////////////////
void CParticleItem::AddChild( CParticleItem *pItem )
{
	assert( pItem );
	pItem->m_pParentParticles = this;
	m_childs.push_back(pItem);
	pItem->m_library = m_library;

	// Change name to be without group.
	if (pItem->GetName() != pItem->GetShortName())
	{
		pItem->SetName( m_library->GetManager()->MakeUniqItemName(pItem->GetShortName()) );
	}

	if (m_pEffect)
		m_pEffect->AddChild( pItem->GetEffect() );
}

//////////////////////////////////////////////////////////////////////////
void CParticleItem::RemoveChild( CParticleItem *pItem )
{
	assert( pItem );
	TSmartPtr<CParticleItem> refholder = pItem;
	if (stl::find_and_erase( m_childs,pItem ))
	{
		pItem->m_pParentParticles = NULL;
	}
	if (m_pEffect)
		m_pEffect->RemoveChild( pItem->GetEffect() );
}

//////////////////////////////////////////////////////////////////////////
void CParticleItem::ClearChilds()
{
	// Also delete them from the library.
	for (int i = 0; i < m_childs.size(); i++)
	{
		m_childs[i]->m_pParentParticles = NULL;
	}
	m_childs.clear();

	if (m_pEffect)
		m_pEffect->ClearChilds();
}

/*
//////////////////////////////////////////////////////////////////////////
void CParticleItem::InsertChild( int slot,CParticleItem *pItem )
{
	if (slot < 0)
		slot = 0;
	if (slot > m_childs.size())
		slot = m_childs.size();

	assert( pItem );
	pItem->m_pParentParticles = this;
	pItem->m_library = m_library;

	m_childs.insert( m_childs.begin() + slot,pItem );
	m_pMatInfo->RemoveAllSubMtls();
	for (int i = 0; i < m_childs.size(); i++)
	{
		m_pMatInfo->AddSubMtl( m_childs[i]->m_pMatInfo );
	}
}
*/

//////////////////////////////////////////////////////////////////////////
int CParticleItem::FindChild( CParticleItem *pItem )
{
	for (int i = 0; i < m_childs.size(); i++)
	{
		if (m_childs[i] == pItem)
		{
			return i;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
CParticleItem* CParticleItem::GetParent() const
{
	return m_pParentParticles;
}

//////////////////////////////////////////////////////////////////////////
IParticleEffect* CParticleItem::GetEffect() const
{
	return m_pEffect;
}

//////////////////////////////////////////////////////////////////////////
void CParticleItem::GenerateIdRecursively()
{
	GenerateId();
	for (int i = 0; i < m_childs.size(); i++)
	{
		m_childs[i]->GenerateIdRecursively();
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleItem::Update()
{
	// Mark library as modified.
	GetLibrary()->SetModified();
}

//////////////////////////////////////////////////////////////////////////
void CParticleItem::SetDefaults()
{
	if (m_pEffect)
	{
		{
			ParticleParams &params = m_pEffect->GetParticleParams(0);
			params.nCount = 1;
			params.fSize = 1;
			params.fLifeTime = 1;
			params.fSpeed = 1;
			params.vColorStart.Set(1,1,1);
			params.vColorEnd.Set(1,1,1);
		}
		{
			ParticleParams &params = m_pEffect->GetParticleParams(1);
			params.fSize = 1;
			params.fLifeTime = 1;
			params.fSpeed = 1;
			params.vColorStart.Set(1,1,1);
			params.vColorEnd.Set(1,1,1);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleItem::GatherUsedResources( CUsedResources &resources )
{
	for (int k = 0; k < IParticleEffect::NUM_PARTICLE_PROCESSES; k++)
	{
		if (strlen(m_pEffect->GetTexture(k)) > 0)
			resources.Add( m_pEffect->GetTexture(k) );
		if (strlen(m_pEffect->GetGeometry(k)) > 0)
			resources.Add( m_pEffect->GetGeometry(k) );

		IParticleEffect::SoundParams snd;
		m_pEffect->GetSoundParams( snd );
		if (strlen(snd.szSound) > 0)
			resources.Add( snd.szSound );
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleItem::GetFrom3DEngine()
{
	IParticleEffect_AutoPtr pEffect = GetIEditor()->Get3DEngine()->FindParticleEffect( GetFullName() );
	if (!pEffect)
		return;

	CString str = GetLibrary()->GetName() + ".";
	
	// Copy 3d engine particle effect to this one..
	ClearChilds();
	for (int i = 0; i < pEffect->GetChildCount(); i++)
	{
		CParticleItem *pItem = new CParticleItem;
		GetLibrary()->AddItem( pItem );
		pItem->m_pEffect = pEffect->GetChild(i);
		CString name = pEffect->GetName();
		name.Replace( str,"" );
		pItem->SetName( name );
		AddChild( pItem );
	}

	m_pEffect = pEffect;
}
