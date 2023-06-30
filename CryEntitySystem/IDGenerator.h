//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: IDGenerator.h
//  Description: ID Generator class.
//
//  History:
//  - August 6, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_IDGENERATOR_H
#define GAME_IDGENERATOR_H
#if _MSC_VER > 1000
# pragma once
#endif

#include <vector>

typedef std::vector<bool> BitVector;							// depending on STL implementation this might be a bit vector or a byte vector
typedef BitVector::iterator BitVectorItor;

class CIDGenerator  
{
public:
	//! constructor
	CIDGenerator();
	//! destructor
	virtual ~CIDGenerator();
	//!
	void Reset();
	//! just a different range, might return a static id if there is no dynamic left (very unlikely)
	WORD GetNewDynamic();
	//! just a different range, might return a static id if there is no dynamic left (very unlikely)
	WORD GetNewStatic();
	//!
	void Remove(WORD nID);
	//!
	void Mark(WORD nID);
	//!
	unsigned sizeofThis()const
	{
		return sizeof(*this) + sizeof(m_vUsedIDs[0]) * m_vUsedIDs.size();
	}
	//!
	bool IsUsed(WORD nID);

	//!
	static bool IsDynamicEntityId( WORD nID );

private: // ------------------------------------------------------------

	BitVector				m_vUsedIDs;			//!<
};

#endif // GAME_IDGENERATOR_H