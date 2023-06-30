////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   IconManager.h
//  Version:     v1.00
//  Created:     24/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Manages Texures used by Icon.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IconManager_h__
#define __IconManager_h__

#if _MSC_VER > 1000
#pragma once
#endif

enum EObject
{
	STATOBJECT_ARROW,
	STATOBJECT_AXIS,
	STATOBJECT_SPHERE,
	STATOBJECT_ANCHOR,
	STATOBJECT_ENTRANCE,
	STATOBJECT_HIDEPOINT,

	STATOBJECT_LAST,
};

enum EIcon
{
	ICON_QUAD,

	ICON_LAST,
};

/*!
 *	CIconManager contains map of icon names to icon textures,
 *	Ensuring that only one instance of texture for specified icon will be allocated.
 *	Also release textures when editor exit.
 *
 */
class CIconManager : public IDocListener
{
public:
	// Constraction
	CIconManager();
	~CIconManager();

	void Init();
	void Done();

	// Unload all loaded resources.
	void Reset();

	// Operations
	int	GetIconTexture( const CString &iconName );

	IStatObj*	GetObject( EObject object );

	//! Get icon texture.
	//! @return textureId of Icon.
	int GetIcon( EIcon icon );

	//////////////////////////////////////////////////////////////////////////
	// Implementation of IDocListener.
	//////////////////////////////////////////////////////////////////////////
	virtual	void OnNewDocument() { Reset(); };
	virtual	void OnLoadDocument() { Reset(); };
	virtual void OnCloseDocument() { Reset(); };
	virtual void OnMissionChange() { Reset(); };
	//////////////////////////////////////////////////////////////////////////

private:
	StdMap<CString,int> m_textures;
	
	IStatObj*	m_objects[STATOBJECT_LAST];
	int m_icons[ICON_LAST];
};


#endif // __IconManager_h__
