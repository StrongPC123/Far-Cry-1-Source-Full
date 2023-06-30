////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   material.h
//  Version:     v1.00
//  Created:     3/2/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __material_h__
#define __material_h__
#pragma once

#include "BaseLibraryItem.h"

typedef std::vector<SShaderParam> ShaderPublicParams;

/*
struct SMaterial
{
	bool m_bLighting;
	SLightMaterial m_lightMaterial;
	SShaderResources m_shaderResources;
	ShaderPublicParams m_shaderParams;
};
*/

/** CMaterial class
		Every Material is a member of material library.
		Materials can have child sub materials,
		Sub materials are applied to the same geometry of the parent material in the other material slots.
*/
class CRYEDIT_API CMaterial : public CBaseLibraryItem
{
public:
	enum EMtlFlags
	{
		MF_WIRE					= 0x0001,
		MF_2SIDED				= 0x0002,
		MF_ADDITIVE			= 0x0004,
		MF_ADDITIVE_DECAL	= 0x0008,
		MF_LIGHTING			= 0x0010,
		MF_NOSHADOW			= 0x0020,
		MF_ALWAYS_USED	= 0x0040, //! When set forces material to be export even if not explicitly used.
	};
	//////////////////////////////////////////////////////////////////////////
	CMaterial();
	~CMaterial();

	void SetName( const CString &name );

	//! Set name of shader used by this material.
	void SetShaderName( const CString &shaderName );
	//! Get name of shader used by this material.
	CString GetShaderName() const { return m_shaderName; };

	//! Sets one or more material flags from EMtlFlags enum.
	void SetMaterialFlags( int flags ) { m_mtlFlags = flags; };
	//! Query this material flags.
	int GetMaterialFlags() const { return m_mtlFlags; }

	SInputShaderResources& GetShaderResources() { return m_shaderResources; };

	//! Get public parameters of material in variable block.
	CVarBlock* GetPublicVars();

	//////////////////////////////////////////////////////////////////////////
	CVarBlock* GetShaderGenParamsVars();
	void SetShaderGenParamsVars( CVarBlock *pBlock );
	unsigned int GetShaderGenMask() { return m_nShaderGenMask; }

	//! Sets variable block of publich shader parameters.
	//! VarBlock must be in same format as returned by GetPublicVars().
	void SetPublicVars( CVarBlock *publicVars );

	//! Return variable block of shader params.
	SShaderItem& GetShaderItem() { return m_shaderItem; };

	//! Get texture map usage mask for shader in this material.
	unsigned int GetTexmapUsageMask() const;

	//! Load new shader.
	bool LoadShader( const CString &shaderName );

	//! Reload shader, update all shader parameters.
	void Update();

	//! Serialize material settings to xml.
	virtual void Serialize( SerializeContext &ctx );

	//! Assign this material to static geometry.
	void AssignToEntity( IEntityRender *pEntity );
	//! Assign material settings from givven geometry.
	void AssignFromGeometry( IStatObj *pGeometry );
	//! Assign material settings from character.
	void AssignFromGeometry( ICryCharInstance *pCharacter );

	//////////////////////////////////////////////////////////////////////////
	// Child Sub materials.
	//////////////////////////////////////////////////////////////////////////
	//! Get number of sub materials childs.
	int GetSubMaterialCount() const;
	//! Get sub material child by index.
	CMaterial* GetSubMaterial( int index ) const;
	//! Adds a new sub material.
	void AddSubMaterial( CMaterial *mtl );
	//! Remove specific sub material
	void RemoveSubMaterial( CMaterial *mtl );
	//! Remove all sub materials.
	void RemoveAllSubMaterials();
	//! Insert sub material in between other materials.
	void InsertSubMaterial( int slot,CMaterial *mtl );
	//! Find slot where sub material stored.
	//! @retun slot index if material found, -1 if material not found.
	int  FindSubMaterial( CMaterial *mtl );
	//! Returns parent material.
	CMaterial* GetParent() const;

	void GenerateIdRecursively();

	//! Return pointer to engine material.
	IMatInfo* GetMatInfo() const { return m_pMatInfo; }

	//! Swap contents of this material, with other material.
	void SwapContent( CMaterial *mtl );

	//! Validate materials for errors.
	void Validate();

	//! Set material to be in use.
	void SetUsed( bool bUsed=true ) { m_bInUse = bUsed; };
	bool IsUsed() const { return m_bInUse || (m_mtlFlags & MF_ALWAYS_USED); };

	virtual void GatherUsedResources( CUsedResources &resources );

private:
	void SetFromMatInfo( CMatInfo *pMatInfo );
	//////////////////////////////////////////////////////////////////////////
	// Variables.
	//////////////////////////////////////////////////////////////////////////
	CString m_shaderName;

	//! Material flags.
	int m_mtlFlags;

	//////////////////////////////////////////////////////////////////////////
	// Shader resources.
	//////////////////////////////////////////////////////////////////////////
	SShaderItem m_shaderItem;
	SLightMaterial m_lightMaterial;
	SInputShaderResources m_shaderResources;
	ShaderPublicParams m_shaderParams;
	//CVarBlockPtr m_shaderParamsVar;
	//! Common shader flags.
	unsigned int m_nShaderGenMask;

	IMatInfo *m_pMatInfo;

	bool m_bRegetPublicParams;

	//! Parent of this material (if this is sub material).
	CMaterial *m_pParentMtl;
	//! Array of sub materials.
	std::vector<TSmartPtr<CMaterial> > m_subMaterials;

	//! Material Used in level.
	bool m_bInUse;
};

#endif // __material_h__
