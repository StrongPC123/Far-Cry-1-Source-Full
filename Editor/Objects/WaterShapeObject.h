////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   watershapeobject.h
//  Version:     v1.00
//  Created:     10/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __watershapeobject_h__
#define __watershapeobject_h__
#pragma once

#include "ShapeObject.h"
#include "Material\Material.h"

class CWaterShapeObject : public CShapeObject
{
	DECLARE_DYNCREATE(CWaterShapeObject)
public:
	CWaterShapeObject();

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();

	virtual void SetName( const CString &name );

	//////////////////////////////////////////////////////////////////////////
	// Override materials.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetMaterial( CMaterial *mtl );
	virtual CMaterial* GetMaterial() const { return m_pMaterial; };

	//void Display( DisplayContext &dc );
	void Serialize( CObjectArchive &ar );
	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );
	//////////////////////////////////////////////////////////////////////////

protected:
	void OnWaterChange( IVariable *var );
	virtual void UpdateGameArea( bool bRemove=false );
	//////////////////////////////////////////////////////////////////////////
	// Water shape parameters.
	//////////////////////////////////////////////////////////////////////////
	CVariable<CString> mv_waterShader;
	CVariable<float> mv_waterStreamSpeed;
	CVariable<float> mv_waterTriMinSize;
	CVariable<float> mv_waterTriMaxSize;
	CVariable<bool>	 mv_bAffectToVolFog;

	//! Material of this object.
	TSmartPtr<CMaterial> m_pMaterial;
	GUID m_materialGUID;

	struct IWaterVolume* m_waterVolume;
};

/*!
 * Class Description of ShapeObject.	
 */
class CWaterShapeObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {3CC1CF42-917A-4c4d-80D2-2A81E6A32BDB}
		static const GUID guid = { 0x3cc1cf42, 0x917a, 0x4c4d, { 0x80, 0xd2, 0x2a, 0x81, 0xe6, 0xa3, 0x2b, 0xdb } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_VOLUME; };
	const char* ClassName() { return "WaterVolume"; };
	const char* Category() { return "Area"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CWaterShapeObject); };
	int GameCreationOrder() { return 16; };
};

#endif // __watershapeobject_h__
