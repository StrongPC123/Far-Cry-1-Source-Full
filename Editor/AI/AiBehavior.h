////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   aibehavior.h
//  Version:     v1.00
//  Created:     21/3/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __aibehavior_h__
#define __aibehavior_h__

#if _MSC_VER > 1000
#pragma once
#endif

/** AI Behavior definition.
*/
class CAIBehavior : public CRefCountBase
{
public:
	CAIBehavior() {};
	virtual ~CAIBehavior() {};

	void SetName( const CString& name ) { m_name = name; }
	const CString& GetName() { return m_name; }

	//! Set name of script that implements this behavior.
	void SetScript( const CString &script ) { m_script = script; };
	const CString& GetScript() const { return m_script; };

	//! Get human readable description of this goal.
	const CString& GetDescription() { return m_description; }
	//! Set human readable description of this goal.
	void SetDescription( const CString& desc ) { m_description = desc; }

	//! Force reload of script file.
	void ReloadScript();

	//! Start editing script file in Text editor.
	void Edit();

private:
	CString m_name;
	CString m_description;
	CString m_script;
};

/** AICharacter behaviour definition.
*/
class CAICharacter : public CRefCountBase
{
public:
	CAICharacter() {};
	virtual ~CAICharacter() {};

	void SetName( const CString& name ) { m_name = name; }
	const CString& GetName() { return m_name; }

	//! Set name of script that implements this behavior.
	void SetScript( const CString &script ) { m_script = script; };
	const CString& GetScript() const { return m_script; };

	//! Get human readable description of this goal.
	const CString& GetDescription() { return m_description; }
	//! Set human readable description of this goal.
	void SetDescription( const CString& desc ) { m_description = desc; }

	//! Force reload of script file.
	void ReloadScript();

	//! Start editing script file in Text editor.
	void Edit();

private:
	CString m_name;
	CString m_description;
	CString m_script;
};

typedef TSmartPtr<CAIBehavior> CAIBehaviorPtr;
typedef TSmartPtr<CAICharacter> CAICharacterPtr;

#endif // __aibehavior_h__
