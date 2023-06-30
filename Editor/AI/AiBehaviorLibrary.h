////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   aiBehaviorLibrary.h
//  Version:     v1.00
//  Created:     21/3/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __aibehaviorlibrary_h__
#define __aibehaviorlibrary_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "AiBehavior.h"

/*!
 * CAIBehaviorLibrary is collection of global AI behaviors.
 */
class CAIBehaviorLibrary
{
public:
	CAIBehaviorLibrary();
	~CAIBehaviorLibrary() {};

	//! Add new behavior to the library.
	void AddBehavior( CAIBehavior* behavior );
	//! Remove behavior from the library.
	void RemoveBehavior( CAIBehavior* behavior );

	CAIBehavior* FindBehavior( const CString &name ) const;

	//! Clear all behaviors from library.
	void ClearBehaviors();

	//! Get all stored behaviors as a vector.
	void GetBehaviors( std::vector<CAIBehaviorPtr> &behaviors );
	
	//! Load all behaviors from givven path and add them to library.
	void LoadBehaviors( const CString &path );

	//! Reload behavior scripts.
	void ReloadScripts();

	//! Get all available characters in system.
	void GetCharacters( std::vector<CAICharacterPtr> &characters );

	//! Add new behavior to the library.
	void AddCharacter( CAICharacter* chr );
	//! Remove behavior from the library.
	void RemoveCharacter( CAICharacter* chr );

	// Finds specified character.
	CAICharacter* FindCharacter( const CString &name ) const;

private:
	void LoadCharacters();

	StdMap<CString,TSmartPtr<CAIBehavior> > m_behaviors;
	StdMap<CString,TSmartPtr<CAICharacter> > m_characters;
	CString m_scriptsPath;
};

#endif // __aibehaviorlibrary_h__