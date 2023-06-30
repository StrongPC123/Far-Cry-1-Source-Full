// NetEntityInfo.h: interface for the CNetEntityInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETENTITYINFO_H__6641B99B_3343_4117_BA60_92A4BFCC2056__INCLUDED_)
#define AFX_NETENTITYINFO_H__6641B99B_3343_4117_BA60_92A4BFCC2056__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IEntitySystem.h>

struct ITimer;
class CPlayer;


class CNetEntityInfo  
{
public:
	//! constructor
	explicit CNetEntityInfo();
	//! constructor
	CNetEntityInfo(const CNetEntityInfo& nei);
	//! constructor
	CNetEntityInfo(CXServerSlot *pServerSlot,ITimer *pTimer,IEntity* pEntity);
	//! destructor
	virtual ~CNetEntityInfo();

	// -------------------------------------------------------------------------------

	//! \param pServer must not be 0
	bool Write( CXServer *pServer, CStream &stm );
	//!
	bool NeedUpdate(){ return m_nPriority!=0; }
	//!
	void Update(Vec3 v3d);
	//!
	void Reset();
	//!
	void Invalidate(){m_pEntity=NULL;};
	//!
	bool operator ==(EntityId id) const;
	//!
	inline IEntity *GetEntity(){return m_pEntity;}
	//!
	inline unsigned int GetPriority(){return m_nPriority;}
	//!
	float GetTimeAffectedPriority();
	//!
	float GetDistanceTo( const Vec3d &vPos );

	//! \return in bits
	uint32 CalcEstimatedSize();

private: // --------------------------------------------------------------------------

	IEntity *						m_pEntity;							//!<
	float								m_fLastUpdate;					//!< absolute time
	short								m_nScore;								//!< only used for bLocalPlayer==true, from struct PlayerStats.score
	ITimer *						m_pTimer;								//!<
	char								m_cState;								//!< entity state
	uint32							m_dwBitSizeEstimate;		//!< based on last packet size (maybe this needs to be improved), in bits per packet

	//temp vars for determinate the priority
	uint32							m_nPriority;						//!< 0=no update at all, 1=lowest update priority ...
	uint32							m_nUpdateNumber;				//!<
	EntityCloneState		m_ecsClone;							//!<

	//! \return pointer is always valid
	CXServerSlot *GetXServerSlot();
};

#endif // !defined(AFX_NETENTITYINFO_H__6641B99B_3343_4117_BA60_92A4BFCC2056__INCLUDED_)
