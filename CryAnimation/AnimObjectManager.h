////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   animobjectmanager.h
//  Version:     v1.00
//  Created:     14/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Manages collection of AnimObjects.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __animobjectmanager_h__
#define __animobjectmanager_h__
#pragma once

#include "AnimObject.h"

//////////////////////////////////////////////////////////////////////////
// Manages collection of AnimObjects.
// Controls creation/destruction of AnimObjects.
//////////////////////////////////////////////////////////////////////////
class CAnimObjectManager
{
public:
	CAnimObjectManager();

	//! Creates anim object, or get already created one.
	ICryCharInstance* MakeAnimObject( const char *animFile );
	bool RemoveCharacter( ICryCharInstance* obj );

	// puts the size of the whole subsystem into this sizer object, classified,
	// according to the flags set in the sizer
	void GetMemoryUsage(class ICrySizer* pSizer)const;

	unsigned NumObjects () {return (unsigned)m_objects.size();}

	void LockResources();
	void UnlockResources();
private:
	typedef std::set<ICryCharInstance*> ObjectsSet;
	ObjectsSet m_objects;
	typedef std::set<_smart_ptr<CAnimObject> > AnimObjectsSet;
	AnimObjectsSet m_animObjects;

	std::vector<_smart_ptr<CAnimObject> > m_lockArray;
	bool m_bResourcesLocked;
};


#endif // __animobjectmanager_h__
