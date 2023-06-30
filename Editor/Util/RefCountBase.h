////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   refcountbase.h
//  Version:     v1.00
//  Created:     21/2/2002 by Timur.
//  Compilers:   Visual C++.NET
//  Description: Reference counted base object.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __refcountbase_h__
#define __refcountbase_h__
#pragma once

//! Derive from this class to get reference counting for your class.
class CRYEDIT_API CRefCountBase : public CObject
{
public:
	CRefCountBase() { m_nRefCount = 0; };

	//! Add new refrence to this object.
	int AddRef()
	{
		m_nRefCount++;
		return m_nRefCount;
	};

	//! Release refrence to this object.
	//! when reference count reaches zero, object is deleted.
	int Release()
	{
		int refs = --m_nRefCount;
		if (m_nRefCount <= 0)
			delete this;
		return refs;
	}

protected:
	virtual ~CRefCountBase() {};

private:
	int m_nRefCount;
};

//////////////////////////////////////////////////////////////////////////
//! Derive from this class to get reference counting for your class.
//////////////////////////////////////////////////////////////////////////
template <class ParentClass>
class CRYEDIT_API TRefCountBase : public ParentClass
{
public:
	TRefCountBase() { m_nRefCount = 0; };

	//! Add new refrence to this object.
	int AddRef()
	{
		m_nRefCount++;
		return m_nRefCount;
	};

	//! Release refrence to this object.
	//! when reference count reaches zero, object is deleted.
	int Release()
	{
		int refs = --m_nRefCount;
		if (m_nRefCount <= 0)
			delete this;
		return refs;
	}

protected:
	virtual ~TRefCountBase() {};

private:
	int m_nRefCount;
};


#endif // __refcountbase_h__
