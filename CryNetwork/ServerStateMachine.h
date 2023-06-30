//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: ServerStateMachine.h
//  Description: 
//
//	History:
//	-08/25/2001: Alberto Demichelis, created
//  -01/26/2003: Martin Mittring, commented the stated
//
//////////////////////////////////////////////////////////////////////


#ifndef _SERVER_STATE_MACHINE_H_
#define _SERVER_STATE_MACHINE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <platform.h>
#include "StateMachine.h"
#include "Interfaces.h"

#define STATUS_SRV_WAIT_FOR_CONNECT_RESP		STATUS_BASE+1
#define STATUS_SRV_CONNECTED								STATUS_BASE+2
#define STATUS_SRV_WAIT_FOR_CONTEXT_READY		STATUS_BASE+3
#define STATUS_SRV_READY										STATUS_BASE+4
#define STATUS_SRV_DISCONNECTED							STATUS_BASE+5

//in signals
#define SIG_SRV_START												SIGNAL_BASE+1
#define SIG_SRV_SETUP												SIGNAL_BASE+2
#define SIG_SRV_CONNECT_RESP								SIGNAL_BASE+3
#define SIG_SRV_CONTEXT_SETUP								SIGNAL_BASE+4
#define SIG_SRV_CONTEXT_READY								SIGNAL_BASE+5
#define SIG_SRV_DISCONNECT									SIGNAL_BASE+6
#define SIG_SRV_ACTIVE_DISCONNECT						SIGNAL_BASE+7
//out signals
#define SIG_SRV_SEND_CONNECT								SIGNAL_BASE+8
#define SIG_SRV_SEND_DISCONNECT							SIGNAL_BASE+9
#define SIG_SRV_SEND_CONTEXT_SETUP					SIGNAL_BASE+10
#define SIG_SRV_NOTIFY_CONTEXT_READY				SIGNAL_BASE+12
#define SIG_SRV_CONNECTED										SIGNAL_BASE+13
#define SIG_SRV_DISCONNECTED								SIGNAL_BASE+14


#define TM_CONNECT_RESP									TIMER_BASE+1

#ifdef _DEBUG
	#define TM_CONNECT_RESP_ET						200000
#else
	#define TM_CONNECT_RESP_ET						20000
#endif

#define TM_CONTEXT_READY								TIMER_BASE+2
#define TM_CONTEXT_READY_ET							(5*60*1000)				// 5 min to load the map



class CServerStateMachine :public CStateMachine
{
	BEGIN_STATUS_MAP()
		STATUS_ENTRY(STATUS_IDLE,HandleIDLE)
			// goes to the next state when getting the CCPSetup packet (PlayerPassword,ProtocolVersion) from the client
			//   or to STATUS_SRV_DISCONNECTED when the version doesn't match

		STATUS_ENTRY(STATUS_SRV_WAIT_FOR_CONNECT_RESP,HandleWAIT_FOR_CONNECT_RESP)
			// goes to the next state when getting the CCPConnectResp packet (AuthorizationID) from the client

		STATUS_ENTRY(STATUS_SRV_CONNECTED,HandleCONNECTED)
			// goes to the next state when the authorization is ok

		STATUS_ENTRY(STATUS_SRV_WAIT_FOR_CONTEXT_READY,HandleWAIT_FOR_CONTEXT_READY)
			// the client is currently loading the map and we wait 
			//   when getting the CCPContextReady packet (bHost,p_name,p_model) from the client
			//     we sends all entities to the client (CXServerSlot::OnContextReady) and go to the next state

		STATUS_ENTRY(STATUS_SRV_READY,HandleREADY)
			// goes back to STATUS_SRV_WAIT_FOR_CONTEXT_READY when CServerSlot::ContextSetup() was called
			//   to change the map/mod and sync the server variables once again 

		STATUS_ENTRY(STATUS_SRV_DISCONNECTED,HandleDISCONNECTED)
			// when going in this state send the SIG_DISCONNECTED signal and the system will remove the client soon

	END_STATUS_MAP()

public:  // ---------------------------------------------------------------------

	//! constructor
	CServerStateMachine();
	//! destructor
	virtual ~CServerStateMachine();
	//!
	void Init(_IServerSlotServices *pParent);

private: // ---------------------------------------------------------------------

	//status handlers
	void HandleIDLE(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandleWAIT_FOR_CONNECT_RESP(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandleCONNECTED(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandleWAIT_FOR_CONTEXT_READY(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandleREADY(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandleDISCONNECTED(unsigned int dwIncomingSignal,DWORD_PTR dwParam);

	void OnSignal(unsigned int dwOutgoingSignal,DWORD_PTR dwParam);

	//tracing
	void _Trace(char *s);
	void _TraceStatus(unsigned int dwStatus);

	_IServerSlotServices *						m_pParent;		//!< pointer to the parent object (is not released), is 0 is you forgot to call Init()
};

#endif //_SERVER_STATE_MACHINE_H_
