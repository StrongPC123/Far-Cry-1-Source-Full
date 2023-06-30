//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: ServerStateMachine.cpp
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "ServerStateMachine.h"

#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

#include "IConsole.h"					// IConsole

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerStateMachine::CServerStateMachine()
{
	m_pParent=0;
}

CServerStateMachine::~CServerStateMachine()
{
}

void CServerStateMachine::_Trace(char *s)
{
	NET_TRACE(s);
}

void CServerStateMachine::_TraceStatus(unsigned int dwStatus)
{
	switch (dwStatus)
	{
		case STATUS_IDLE:
			_Trace("STATUS_IDLE");
			break;
		case STATUS_SRV_WAIT_FOR_CONNECT_RESP:
			_Trace("STATUS_SRV_WAIT_FOR_CONNECT_RESP");
			break;
		case STATUS_SRV_CONNECTED:
			_Trace("STATUS_SRV_CONNECTED");
			break;
		case STATUS_SRV_WAIT_FOR_CONTEXT_READY:
			_Trace("STATUS_SRV_WAIT_FOR_CONTEXT_READY");
			break;
		case STATUS_SRV_READY:
			_Trace("STATUS_SRV_READY");
			break;
		case STATUS_SRV_DISCONNECTED:
			_Trace("STATUS_SRV_DISCONNECTED");
			break;
		default:
			_Trace("UNKNOWN");
			break;
	}	
}

void CServerStateMachine::HandleIDLE(unsigned int dwIncomingSignal, DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_SETUP)
			if(dwParam==0) //the protocol versions do not match
			{
				SetStatus(STATUS_SRV_DISCONNECTED);
				const char *serr="wrong protocol version";
				OnSignal(SIG_SRV_SEND_DISCONNECT, (ULONG_PTR)serr);
				OnSignal(SIG_SRV_DISCONNECTED, (ULONG_PTR)serr);
			}
			else
			{
				SetStatus(STATUS_SRV_WAIT_FOR_CONNECT_RESP);
				SetTimer(TM_CONNECT_RESP, TM_CONNECT_RESP_ET);
				OnSignal(SIG_SRV_SEND_CONNECT, dwParam);
			}
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_ACTIVE_DISCONNECT)
			SetStatus(STATUS_SRV_DISCONNECTED);
			OnSignal(SIG_SRV_SEND_DISCONNECT, dwParam);
			OnSignal(SIG_SRV_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()
}

void CServerStateMachine::HandleWAIT_FOR_CONNECT_RESP(unsigned int dwIncomingSignal, DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_CONNECT_RESP)
			ResetTimer();
			SetStatus(STATUS_SRV_CONNECTED);
			OnSignal(SIG_SRV_CONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(TM_CONNECT_RESP)
			SetStatus(STATUS_SRV_DISCONNECTED);
			OnSignal(SIG_SRV_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()
}

void CServerStateMachine::HandleCONNECTED(unsigned int dwIncomingSignal, DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_CONTEXT_SETUP)																		// we now have this info which map, initiate the loading on the client
			SetStatus(STATUS_SRV_WAIT_FOR_CONTEXT_READY);
			SetTimer(TM_CONTEXT_READY, TM_CONTEXT_READY_ET);
			OnSignal(SIG_SRV_SEND_CONTEXT_SETUP, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_ACTIVE_DISCONNECT)
			SetStatus(STATUS_SRV_DISCONNECTED);
			OnSignal(SIG_SRV_SEND_DISCONNECT, dwParam);
			OnSignal(SIG_SRV_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_DISCONNECT)
			ResetTimer();
			SetStatus(STATUS_SRV_DISCONNECTED);
			OnSignal(SIG_SRV_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()
}

void CServerStateMachine::HandleWAIT_FOR_CONTEXT_READY(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(TM_CONTEXT_READY)
			ResetTimer();
			SetStatus(STATUS_SRV_DISCONNECTED);
			OnSignal(SIG_SRV_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_CONTEXT_SETUP)																		// we changed the map, initiate the loading on the client
			SetTimer(TM_CONTEXT_READY, TM_CONTEXT_READY_ET);
			OnSignal(SIG_SRV_SEND_CONTEXT_SETUP, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_CONTEXT_READY)
			ResetTimer();
			SetStatus(STATUS_SRV_READY);
			OnSignal(SIG_SRV_NOTIFY_CONTEXT_READY, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_DISCONNECT)
			ResetTimer();
			SetStatus(STATUS_SRV_DISCONNECTED);
			OnSignal(SIG_SRV_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_ACTIVE_DISCONNECT)
			ResetTimer();
			SetStatus(STATUS_SRV_DISCONNECTED);
			OnSignal(SIG_SRV_SEND_DISCONNECT, dwParam);
			OnSignal(SIG_SRV_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()

}

void CServerStateMachine::HandleREADY(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_CONTEXT_SETUP)																		// we changed the map, initiate the loading on the client
			SetStatus(STATUS_SRV_WAIT_FOR_CONTEXT_READY);
			SetTimer(TM_CONTEXT_READY, TM_CONTEXT_READY_ET);
			OnSignal(SIG_SRV_SEND_CONTEXT_SETUP, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_DISCONNECT)
			SetStatus(STATUS_SRV_DISCONNECTED);
			OnSignal(SIG_SRV_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SRV_ACTIVE_DISCONNECT)
			SetStatus(STATUS_SRV_DISCONNECTED);
			OnSignal(SIG_SRV_SEND_DISCONNECT, dwParam);
			OnSignal(SIG_SRV_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()

}


void CServerStateMachine::Init(_IServerSlotServices *pParent)
{
	m_pParent=pParent;
}

void CServerStateMachine::HandleDISCONNECTED(unsigned int dwIncomingSignal, DWORD_PTR dwParam)
{
	// the machine can't leave this status
}

void CServerStateMachine::OnSignal(unsigned int dwOutgoingSignal, DWORD_PTR dwParam)
{
	switch (dwOutgoingSignal)
	{
		case SIG_SRV_SEND_CONNECT:
			m_pParent->SendConnect();
			break;
		case SIG_SRV_SEND_DISCONNECT:
			m_pParent->SendDisconnect((const char *)dwParam);
			break;
		case SIG_SRV_SEND_CONTEXT_SETUP:
			m_pParent->SendContextSetup();
			break;
		case SIG_SRV_NOTIFY_CONTEXT_READY:
			m_pParent->OnContextReady();
			break;
		case SIG_SRV_CONNECTED:
			m_pParent->OnConnect();
			break;
		case SIG_SRV_DISCONNECTED:
			m_pParent->OnDisconnect((const char *)dwParam);
			break;
		default:
			NET_ASSERT(0);
			break;
	}
}

