//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: ClientStateMachine.h
//  Description: 
//
//	History:
//	-08/25/2001: Alberto Demichelis, created
//  -01/26/2003: Martin Mittring, commented the stated
//
//////////////////////////////////////////////////////////////////////


#ifndef _CLIENT_STATE_MACHINE_H_
#define _CLIENT_STATE_MACHINE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StateMachine.h"
#include "Interfaces.h"

#define STATUS_WAIT_FOR_CONNECT				STATUS_BASE+1
#define STATUS_CONNECTED							STATUS_BASE+2
#define STATUS_PROCESSING_CONTEXT			STATUS_BASE+3
#define STATUS_WAIT_FOR_SERVER_READY	STATUS_BASE+4
#define STATUS_READY									STATUS_BASE+5
#define STATUS_DISCONNECTED						STATUS_BASE+6

//in signals
#define SIG_START									SIGNAL_BASE+1
#define SIG_CONNECT								SIGNAL_BASE+2
#define SIG_CONTEXT_SETUP					SIGNAL_BASE+3
#define SIG_CONTEXT_READY					SIGNAL_BASE+4
#define SIG_SERVER_READY					SIGNAL_BASE+5
#define SIG_DISCONNECT						SIGNAL_BASE+6
#define SIG_ACTIVE_DISCONNECT			SIGNAL_BASE+7

//out signals
#define SIG_SEND_SETUP							SIGNAL_BASE+8
#define SIG_SEND_CONNECT_RESP				SIGNAL_BASE+9
#define SIG_SEND_CONTEXT_READY			SIGNAL_BASE+10
#define SIG_SEND_DISCONNECT					SIGNAL_BASE+11
#define SIG_CONNECTED								SIGNAL_BASE+12
#define SIG_INCOMING_CONTEXT_SETUP	SIGNAL_BASE+13
#define SIG_INCOMING_SERVER_READY		SIGNAL_BASE+14
#define SIG_DISCONNECTED						SIGNAL_BASE+15

#define TM_CONNECT								TIMER_BASE+1

#ifdef _DEBUG
	#define TM_CONNECT_ET						200000
#else
	#define TM_CONNECT_ET						10000
#endif

#define TM_SERVER_READY						TIMER_BASE+2
#define TM_SERVER_READY_ET				120000




class CClientStateMachine :public CStateMachine
{
	BEGIN_STATUS_MAP()
		STATUS_ENTRY(STATUS_IDLE,HandleIDLE)
			// goes to the next state right after creation of the client

		STATUS_ENTRY(STATUS_WAIT_FOR_CONNECT,HandleWAIT_FOR_CONNECT)
			// goes to the next state when the client gets the CCPConnect packet (time value from the server) from the server

		STATUS_ENTRY(STATUS_CONNECTED,HandleCONNECTED)
			// goes to the next state when the client gets the CCPContextSetup packet (map name, mod, player model, playerclass ..) from the server

		STATUS_ENTRY(STATUS_PROCESSING_CONTEXT,HandlePROCESSING_CONTEXT)
			// goes to the next state when finished loading the map and
			//   sends the CCPContextReady packet (bHost,p_name,p_model) to the server

		STATUS_ENTRY(STATUS_WAIT_FOR_SERVER_READY,HandleWAIT_FOR_SERVER_READY)
			// is getting all entities from the server (XSERVERMSG_ADDENTITY)
			// goes to the next state when the client gets the CCPServerReady packet (empty) from the server

		STATUS_ENTRY(STATUS_READY,HandleREADY)
			// goes back to STATUS_PROCESSING_CONTEXT when the client gets the CCPContextSetup packet

		STATUS_ENTRY(STATUS_DISCONNECTED,HandleDISCONNECTED)
			// when going in this state send the SIG_DISCONNECTED signal and the system will remove the client soon

	END_STATUS_MAP()

public: // ---------------------------------------------------------------------------

	//! constructor
	CClientStateMachine();
	//! destructor
	virtual ~CClientStateMachine();
	//!
	void Init(_IClientServices *pParent);

private: // --------------------------------------------------------------------------

	//status handlers
	void HandleIDLE(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandleWAIT_FOR_CONNECT(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandleCONNECTED(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandlePROCESSING_CONTEXT(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandleWAIT_FOR_SERVER_READY(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandleREADY(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	void HandleDISCONNECTED(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	virtual unsigned int HandleANY(unsigned int dwIncomingSignal,DWORD_PTR dwParam);
	
	void OnSignal(unsigned int dwOutgoingSignal,DWORD_PTR dwParam);

	// tracing
	void _Trace(char *s);
	void _TraceStatus(unsigned int dwStatus);

	_IClientServices *						m_pParent;		//!<
};

#endif // _CLIENT_STATE_MACHINE_H_

