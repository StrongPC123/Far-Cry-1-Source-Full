////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   MusicThemeLibItem.h
//  Version:     v1.00
//  Created:     3/2/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __musicthemelibitem_h__
#define __musicthemelibitem_h__
#pragma once

#include "BaseLibraryItem.h"

struct SMusicTheme;
struct SMusicMood;
struct SPatternDef;
struct SMusicPatternSet;

/** CMusicThemeLibItem class holds collection of several dynamic music themes.
*/
class CRYEDIT_API CMusicThemeLibItem : public CBaseLibraryItem
{
public:
	CMusicThemeLibItem();
	~CMusicThemeLibItem();

	//! Serialize material settings to xml.
	virtual void Serialize( SerializeContext &ctx );
	static void SerializeMood( SerializeContext &ctx,SMusicMood *pMood );
	static void SerializePatternSet( SerializeContext &ctx,SMusicPatternSet *pPatternSet );
	static void SerializePattern( SerializeContext &ctx,SPatternDef *pPattern );

	void SetTheme( SMusicTheme *pTheme ) { m_pTheme = pTheme; };
	SMusicTheme* GetTheme() { return m_pTheme; }

	virtual void GatherUsedResources( CUsedResources &resources );
private:
	void AddMusicResourceFile( const char *szFilename,CUsedResources &resources );

	SMusicTheme *m_pTheme;
};

#endif __musicthemelibitem_h__
