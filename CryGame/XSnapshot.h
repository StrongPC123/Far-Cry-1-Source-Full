//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XSnapshot.h
//  Description: Snapshot manager class.
//
//  History:
//  - August 14, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_XSNAPSHOT_H
#define GAME_XSNAPSHOT_H

class CXServer;
class CXServerSlot;
struct IXSystem;
struct ITimer;
class CStream;
#include "NetEntityInfo.h"
#include <EntityDesc.h>

typedef std::list<CNetEntityInfo> NetEntityList;
typedef NetEntityList::iterator NetEntityListItor;

//////////////////////////////////////////////////////////////////////////////////////////////
/*!manage the network snapshot of a single serverslot.
For every entity with "netpresence" stores a CNetEntityInfo instance.
This represent the status of a certain entity on the client
associated to the serverslot.
Every time a snapshot has to be sent this class sort all NetEntityInfo
by priority and sends all those that fit into a snapshot to the remote client.
*/
class CXSnapshot
{
public:
	//! constructor
	CXSnapshot();
	//! destructor
	~CXSnapshot();

	void Init(CXServer *pServer,CXServerSlot *pServerSlot);
	
	void SetSendPerSecond(int nSendPerSecond); // 0 mean 25 per second
	int GetSendPerSecond();

	inline CStream& GetReliableStream() { return m_stmReliable; }
	inline CStream& GetUnreliableStream() { return m_stmUnreliable; }

	void AddEntity(IEntity *pEntity);
	bool RemoveEntity(IEntity *pEntity);

	void SetClientBitsPerSecond(unsigned int rate);

	void BuildAndSendSnapshot();
	void Cleanup()
	{
		m_lstNetEntities.clear();
	}

	//! \return absolute time when BuildAndSendSnapshot was called
	float GetLastUpdate() const;

	unsigned int			m_nMaxSnapshotBitSize;		//!< in bits, used for statistics only
	unsigned int			m_nLastSnapshotBitSize;		//!< in bits, used for statistics only
	NetEntityList			m_lstNetEntities;					//!<

private: // -------------------------------------------------------------------

	void Reset();

	CXServer *				m_pServer;								//!<
	CXServerSlot *		m_pServerSlot;						//!<
	IXSystem *				m_pISystem;								//!<
	ITimer *					m_pTimer;									//!<
	IPhysicalWorld *	m_pPhysicalWorld;					//!<
	int								m_nSendPerSecond;					//!<
	float							m_fLastUpdate;						//!< absolute timer
	CStream						m_stmReliable;						//!<
	CStream						m_stmUnreliable;					//!<
	unsigned int			m_clientMaxBitsPerSecond;	//!<
	int								m_iCarryOverBps;					//!< in bits per second (negative or prositive)
};

#endif // GAME_XSNAPSHOT_H
