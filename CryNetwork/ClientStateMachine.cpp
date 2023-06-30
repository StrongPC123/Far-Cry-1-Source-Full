//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: ClientStateMachine.cpp
//  Description: 
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "ClientStateMachine.h"

#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

#include "IConsole.h"					// IConsole

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClientStateMachine::CClientStateMachine()
{
}

CClientStateMachine::~CClientStateMachine()
{
}

void CClientStateMachine::_Trace(char *s)
{
	//::OutputDebugString(s);
}

void CClientStateMachine::_TraceStatus(unsigned int dwStatus)
{
	switch(dwStatus){
	case STATUS_IDLE:
		_Trace("STATUS_IDLE");
		break;
	case STATUS_WAIT_FOR_CONNECT:
		_Trace("STATUS_WAIT_FOR_CONNECT");
		break;
	case STATUS_CONNECTED:
		_Trace("STATUS_CONNECTED");
		break;
	case STATUS_PROCESSING_CONTEXT:
		_Trace("STATUS_PROCESSING_CONTEXT");
		break;
	case STATUS_WAIT_FOR_SERVER_READY:
		_Trace("STATUS_WAIT_FOR_SERVER_READY");
		break;
	case STATUS_READY:
		_Trace("STATUS_READY");
		break;
	case STATUS_DISCONNECTED:
		_Trace("STATUS_DISCONNECTED");
		break;
	default:
		_Trace("UNKNOWN");
		break;
	}	
}

unsigned int CClientStateMachine::HandleANY(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
{
	ANY_SIGNAL_EXCEPT(STATUS_DISCONNECTED);
	BEGIN_ANY_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_DISCONNECT)
			SetStatus(STATUS_DISCONNECTED);
			OnSignal(SIG_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_ACTIVE_DISCONNECT)
			SetStatus(STATUS_DISCONNECTED);
			OnSignal(SIG_SEND_DISCONNECT,dwParam);
			OnSignal(SIG_DISCONNECTED, dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_ANY_SIGNAL_HANDLER()
}

void CClientStateMachine::HandleIDLE(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_START)
			SetStatus(STATUS_WAIT_FOR_CONNECT);
			SetTimer(TM_CONNECT,TM_CONNECT_ET);
			OnSignal(SIG_SEND_SETUP,0);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()
}

void CClientStateMachine::HandleWAIT_FOR_CONNECT(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_CONNECT)
			SetStatus(STATUS_CONNECTED);
			ResetTimer();
			OnSignal(SIG_SEND_CONNECT_RESP,0);
			OnSignal(SIG_CONNECTED,0);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(TM_CONNECT)
				SetStatus(STATUS_DISCONNECTED);
				OnSignal(SIG_DISCONNECTED, (DWORD_PTR)"@ConnectionTimeout");
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()
}

void CClientStateMachine::HandleCONNECTED(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_CONTEXT_SETUP)
			SetStatus(STATUS_PROCESSING_CONTEXT);
			OnSignal(SIG_INCOMING_CONTEXT_SETUP,dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()
}

void CClientStateMachine::HandlePROCESSING_CONTEXT(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_CONTEXT_READY)
			SetTimer(TM_SERVER_READY,TM_SERVER_READY_ET);
			SetStatus(STATUS_WAIT_FOR_SERVER_READY);
			OnSignal(SIG_SEND_CONTEXT_READY,dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_CONTEXT_SETUP)
			SetStatus(STATUS_PROCESSING_CONTEXT);
			OnSignal(SIG_INCOMING_CONTEXT_SETUP,dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()
}

void CClientStateMachine::HandleWAIT_FOR_SERVER_READY(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_SERVER_READY)
			SetStatus(STATUS_READY);
			ResetTimer();
			OnSignal(SIG_INCOMING_SERVER_READY,dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_CONTEXT_SETUP)
			SetStatus(STATUS_PROCESSING_CONTEXT);
			OnSignal(SIG_INCOMING_CONTEXT_SETUP,dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(TM_SERVER_READY)
//			SetStatus(STATUS_DISCONNECTED);
			ResetTimer();
//			OnSignal(SIG_DISCONNECTED, (ULONG_PTR)"timeout(TM_SERVER_READY)");
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()

}

void CClientStateMachine::HandleREADY(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
{
	BEGIN_SIGNAL_HANDLER(dwIncomingSignal)
/////////////////////////////////////////////////////////////////////
		IF_SIGNAL(SIG_CONTEXT_SETUP)
			SetStatus(STATUS_PROCESSING_CONTEXT);
			OnSignal(SIG_INCOMING_CONTEXT_SETUP,dwParam);
		ENDIF_SIGNAL()
/////////////////////////////////////////////////////////////////////
	END_SIGNAL_HANDLER()
}

void CClientStateMachine::HandleDISCONNECTED(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
{
		//the machine can't leave this status
}


void CClientStateMachine::Init(_IClientServices *pParent)
{
	m_pParent=pParent;
}

void CClientStateMachine::OnSignal(unsigned int dwOutgoingSignal,DWORD_PTR dwParam)
{
	switch(dwOutgoingSignal){
	case SIG_SEND_SETUP:
		m_pParent->SendSetup();
		break;
	case SIG_SEND_CONNECT_RESP:
		m_pParent->SendConnectResp();
		break;
	case SIG_SEND_CONTEXT_READY:			
		m_pParent->SendContextReady();
		break;
	case SIG_SEND_DISCONNECT:
		m_pParent->SendDisconnect((const char*)dwParam);
		break;
	case SIG_CONNECTED:
		m_pParent->OnConnect();
		break;
	case SIG_INCOMING_CONTEXT_SETUP:
		m_pParent->OnContextSetup();
		break;
	case SIG_INCOMING_SERVER_READY:
		m_pParent->OnServerReady();
		break;
	case SIG_DISCONNECTED:
		m_pParent->OnDisconnect((const char *)dwParam);	// this pointer is destroyed aftr this call
		break;
	default:
		NET_ASSERT(0);
		break;
	}
}
