//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	Description:  
//		random seed helper class (randomize the weapon bullet shooting)
//		each player has a random seed the increases by one when used
//		after a pause of not using it (e.g. 300ms) the value is constantly sent to the server.
//		this stops again when using it.
//		the server is distributing this info to the other players
//		the result is a synchronized random seed value for every player
//
//	History:
//		03/15/2003 MartinM created
//
//////////////////////////////////////////////////////////////////////

#ifndef SYNCHEDRANDOMSEED_H
#define SYNCHEDRANDOMSEED_H

#include "TimeValue.h"				// CTimeValue


class CPlayer;

class CSynchedRandomSeed
{
public:
	//! constructor
	CSynchedRandomSeed();
	//! destructor
	virtual ~CSynchedRandomSeed();

	//! \param pPlayer must not be 0
	void SetParent( CPlayer *pPlayer );
	
	//! detect pauses of usage (to sync the value)
	bool IsTimeToSyncToServerC();
	//!  used to randomize the weapon bullet shooting
	void IncreaseRandomSeedC();
	//!
	void SetStartRandomSeedC( const uint8 Value );
	//!  used to randomize the weapon bullet shooting
	void EnableSyncRandomSeedS( const uint8 Value );
	//!
	void DisableSyncRandomSeedS();
	//!  used to randomize the weapon bullet shooting
	uint8 GetRandomSeedC();
	//!  value we send to the server
	uint8 GetStartRandomSeedC();
	//! value we send to the clients
	uint8 GetStartRandomSeedS();
	//! \return true=we should send the seed value the clients, false otherwise
	bool GetSynchToClientsS() const;
	//!
	static float GetRandTable( BYTE idx );

private: // --------------------------------------------------------------------------------

	uint8					m_ucRandomSeedCS;								//!< current value (used to randomize the weapon bullet shooting)
	uint8					m_ucStartRandomSeedCS;					//!< used to randomize the weapon bullet shooting, value we send to the server
	CTimeValue		m_LastTimeRandomSeedChangedC;		//!< absolute time to detect pauses of usage
	bool					m_bSynchToAllClientsS;					//!<

	CPlayer	*				m_pParentPlayer;							//!<

	static float 	m_fRandomTable[256];						//!< 
};

#endif // SYNCHEDRANDOMSEED_H