////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   animationserializer.h
//  Version:     v1.00
//  Created:     22/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __animationserializer_h__
#define __animationserializer_h__

#if _MSC_VER > 1000
#pragma once
#endif

// forward declarations.
struct IAnimSequence;
struct IAnimBlock;
class CPakFile;

/** Used by Editor to serialize animation data.
*/
class CAnimationSerializer
{
public:
	CAnimationSerializer();
	~CAnimationSerializer();

	/** Save all animation sequences to files in givven directory.
	*/
	void SerializeSequences( XmlNodeRef &xmlNode,bool bLoading );

	/** Saves single animation sequence to file in givven directory.
	*/
	void SaveSequence( IAnimSequence *seq,const char *szFilePath, bool bSaveEmpty=true );

	/** Load sequence from file.
	*/
	IAnimSequence* LoadSequence( const char *szFilePath );

	/** Save all animation sequences to files in givven directory.
	*/
	void SaveAllSequences( const char *szPath,CPakFile &pakFile );

	/** Load all animation sequences from givven directory.
	*/
	void LoadAllSequences( const char *szPath );
};

#endif // __animationserializer_h__