////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ClassDesc.h
//  Version:     v1.00
//  Created:     8/11/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Class description of CBaseObject
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ClassDesc_h__
#define __ClassDesc_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "plugin.h"
#include "ObjectEvent.h"

//! Standart categories.
#define CATEGORY_STATIC			"Static"
#define CATEGORY_TAGPOINTS	"TagPoint"
#define CATEGORY_BUILDING		"Building"
#define CATEGORY_ENTITY			"Entity"
#define CATEGORY_SHAP				"Shape"
#define CATEGORY_SOUND			"Sound"

#define OBJTYPE_ANY_DEFINED (OBJTYPE_GROUP|OBJTYPE_TAGPOINT|OBJTYPE_AIPOINT|OBJTYPE_ENTITY|OBJTYPE_SHAPE|OBJTYPE_VOLUME|OBJTYPE_BRUSH|OBJTYPE_PREFAB)

/*!
 *	Virtual base class description of CBaseObject.
 *  Ovveride this class to create specific Class descriptions for every base object class.
 *	Type name is specified like this:
 *     Category\Type ex: "TagPoint\Respawn"
 */
class CObjectClassDesc : public IClassDesc
{
public:
	//! Release class description.
	virtual ObjectType GetObjectType() = 0;

	//! Create instance of editor object.
	virtual CObject* Create()
	{
		return GetRuntimeClass()->CreateObject();
	}

	//! Get MFC runtime class for this object.
	virtual CRuntimeClass* GetRuntimeClass() = 0;

	//! If this function return not empty string,object of this class must be created with file.
	//! Return root path where to look for files this object supports.
	//! Also wild card for files can be specified, ex: Objects\*.cgf
	virtual const char* GetFileSpec()
	{
		return "";
	}
	virtual ESystemClassID SystemClassID() { return ESYSTEM_CLASS_OBJECT; };
	
	virtual void ShowAbout() {};
	virtual bool CanExitNow() { return true; }
	virtual void Serialize( CXmlArchive &ar ) {};
	virtual void Event( EClassEvent event ) {};
	//! Ex. Object with creation order 200 will be created after any object with order 100.
	virtual int GameCreationOrder() { return 100; };

	// IUnknown implementation.
	ULONG STDMETHODCALLTYPE AddRef() { return 0; } // Ignore.
	ULONG STDMETHODCALLTYPE Release() { delete this; return 0; }
};



#endif // __ClassDesc_h__
