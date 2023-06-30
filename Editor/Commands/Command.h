////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   command.h
//  Version:     v1.00
//  Created:     4/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __command_h__
#define __command_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** CCommand is a base class for all commands exectuted in editor.
		All new commands needs to be registered to CommandManager, each command
		must have uniq name for Ex: "Edit.Clone"
		Uses Command Pattern.
*/
class	CCommand : public CRefCountBase
{
public:
	//! Flags which can be specified for the name of the command.
	enum Flags {
	};

	//! Command must have a name.
	explicit CCommand( const CString &name,int Id=0,int flags=0 );
	
	//! Retrieve command name.
	const CString& GetName() const { return m_name; };
	
	//! Retrieve command flags.
	int GetFlags() const { return m_flags; };
	
	//! Retrieve command Id.
	int GetId() const { return m_id; }

	/** Execute Command.
			This method must be overriden in concrete command class, to perform specific action.
	*/
	virtual void Execute() = 0;

private:
	CString m_name;
	int m_flags;
	//! Command id.
	int m_id;
};

SMARTPTR_TYPEDEF(CCommand);

#endif // __command_h__
