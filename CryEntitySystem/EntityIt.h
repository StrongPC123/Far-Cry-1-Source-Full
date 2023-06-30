// EntityIt.h: interface for the CEntityIt class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ENTITYIT_H__95AE38A4_7F15_4069_A97A_2F3A1F06F670__INCLUDED_)
#define AFX_ENTITYIT_H__95AE38A4_7F15_4069_A97A_2F3A1F06F670__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEntity;
struct IEntity;

#include "EntitySystem.h"

class CEntityItMap : public IEntityIt
{
public:
	CEntityItMap(EntityMap *pMap)
	{
		m_nRefCount=0;
		m_pEntityMap = pMap;
		MoveFirst();
	};

	bool IsEnd() { return (m_itEntityMap == m_pEntityMap->end()); };
	IEntity * Next() { return IsEnd() ? NULL : (IEntity *) (* m_itEntityMap++).second; };
	void MoveFirst() { m_itEntityMap = m_pEntityMap->begin(); };
	void AddRef(){m_nRefCount++;}
	void Release() { --m_nRefCount; if(m_nRefCount<=0){delete this;} };
	
protected:
	int m_nRefCount;
	EntityMap *m_pEntityMap;
	EntityMapItor m_itEntityMap;

};

class CEntityItVec : public IEntityIt
{
public:
	CEntityItVec(EntityVector *pVec)
	{
		m_nRefCount=0;
		m_pEntityVec = pVec;
		MoveFirst();
	};

	bool IsEnd() { return (m_itEntityVec == m_pEntityVec->end()); };
	IEntity * Next() { return IsEnd() ? NULL : (*m_itEntityVec++); };
	void MoveFirst() { m_itEntityVec = m_pEntityVec->begin(); };
	void AddRef(){m_nRefCount++;}
	void Release() { --m_nRefCount; if(m_nRefCount<=0){delete this;} };

protected:
	int m_nRefCount;
	EntityVector *m_pEntityVec;
	EntityVectorItor m_itEntityVec;

};

#endif // !defined(AFX_ENTITYIT_H__95AE38A4_7F15_4069_A97A_2F3A1F06F670__INCLUDED_)
