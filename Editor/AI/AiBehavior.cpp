////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   aibehavior.cpp
//  Version:     v1.00
//  Created:     9/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "aibehavior.h"
#include "..\Util\FileUtil.h"
#include "IScriptSystem.h"

#include "..\Objects\ObjectManager.h"

//////////////////////////////////////////////////////////////////////////
void CAIBehavior::ReloadScript()
{
	// Execute script file in script system.
	if (m_script.IsEmpty())
		return;
	
	if (CFileUtil::CompileLuaFile( GetScript() ))
	{
		IScriptSystem *scriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();
		// Script compiled succesfully.
		scriptSystem->ReloadScript( m_script );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAIBehavior::Edit()
{
	CFileUtil::EditTextFile( GetScript() );
}


//////////////////////////////////////////////////////////////////////////
void CAICharacter::ReloadScript()
{
	// Execute script file in script system.
	if (m_script.IsEmpty())
		return;
	
	if (CFileUtil::CompileLuaFile( GetScript() ))
	{
		IScriptSystem *scriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();
		// Script compiled succesfully.
		scriptSystem->ReloadScript( m_script );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAICharacter::Edit()
{
	CFileUtil::EditTextFile( GetScript() );
}
