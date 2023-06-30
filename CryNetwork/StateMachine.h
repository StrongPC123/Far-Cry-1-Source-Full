//////////////////////////////////////////////////////////////////////
//
//	Crytek Network source code
//	
//	File: statemachine.h
//  Description: state machine toolkit
//
//	History:
//	-July 25,2001:Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////


#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_
#include <stdio.h>
#define ANY_SIGNAL_NOT_HANDLED	0xFFFFFFFF
#define ANY_SIGNAL_HANDLED		0x00000000

#define STATUS_BASE	0xFFFF0000
#define STATUS_IDLE STATUS_BASE

#define SIGNAL_BASE	0x00000000
#define TIMER_BASE	0x0000FFFF

#ifndef PS2
#define DEBUG_STRING(sss) ::OutputDebugString(sss);
#else
#define DEBUG_STRING(sss) cout << sss;
#endif

#define BEGIN_STATUS_MAP()	void _ProcessSignal(unsigned int dwIncomingSignal,ULONG_PTR dwParam)	\
	{		\
	if(HandleANY(dwIncomingSignal,dwParam)==ANY_SIGNAL_NOT_HANDLED) \
	switch(m_dwStatus)	\
	{	

#define STATUS_ENTRY( _signal , _handler )	case _signal :	\
		_handler(dwIncomingSignal,dwParam);	\
		break;

#define END_STATUS_MAP()	default : \
		::OutputDebugString("singal not handled\n"); \
	break; \
	};	\
	} \
		
#define BEGIN_SIGNAL_HANDLER(signal)	switch(signal){	\

#define IF_SIGNAL(signal)	case signal:{

#define ENDIF_SIGNAL() 	}	\
		break;	\

#define END_SIGNAL_HANDLER()	default:	\
		_Trace("Signal not handled\n");	\
		break; \
		};



#define ANY_SIGNAL_EXCEPT(status){if(m_dwStatus==status){return ANY_SIGNAL_NOT_HANDLED;}}

#define BEGIN_ANY_SIGNAL_HANDLER(signal)	switch(signal){	

#define END_ANY_SIGNAL_HANDLER()	default:	\
		DEBUG_STRING("ANY Signal not handled\n");	\
		return ANY_SIGNAL_NOT_HANDLED; \
		break; \
		};	\
	return ANY_SIGNAL_HANDLED; 
		
class CStateMachine
{
protected:
	virtual void _ProcessSignal(unsigned int dwIncomingSignal,ULONG_PTR dwParam)=0;
	void SetTimer(unsigned int dwName,unsigned int dwElapsed)
	{
		m_dwTimerName=dwName;
		m_dwTimerElapsed=dwElapsed;
		m_dwTimerStart=::GetTickCount();
	}
	void ResetTimer()
	{
		m_dwTimerStart=0;
		m_dwTimerElapsed=0;
		m_dwTimerName=0;
		
	}
	void SetStatus(unsigned int dwStatus)
	{
		_TraceStatus(m_dwStatus);
		_Trace(">>");
		_TraceStatus(dwStatus);
		_Trace("\n");
		m_dwStatus=dwStatus;
	}
	virtual unsigned int HandleANY(unsigned int dwIncomingSignal,DWORD_PTR dwParam)
	{
		return ANY_SIGNAL_NOT_HANDLED;
	}
	virtual void OnSignal(unsigned int dwOutgoingSignal,DWORD_PTR  dwParam)
	{
		NET_TRACE("WARNING default OnSignal called\n");
	}
	virtual void _Trace(char *s){};
	virtual void _TraceStatus(unsigned int dwStatus){};
	
public:
	CStateMachine()
	{
		m_dwStatus=0;
		Reset();
	}
	virtual void Reset()
	{
		SetStatus(STATUS_IDLE);
		ResetTimer();
	}
	BOOL Update(unsigned int dwIncomingSignal=0,DWORD_PTR dwParam=0)
	{
		if(m_dwTimerName)
			if((::GetTickCount()-m_dwTimerStart)>=m_dwTimerElapsed)
			{
				unsigned int dwTimerName=m_dwTimerName;
				ResetTimer();
				_ProcessSignal(dwTimerName,0);
			}
		
		if(dwIncomingSignal)
		{
			_ProcessSignal(dwIncomingSignal,dwParam);	// this pointer might be destroyed after this call
		}
		
		return TRUE;
	}
	
	unsigned int GetCurrentStatus(){return m_dwStatus;}
	unsigned int m_dwStatus;
//
	unsigned int m_dwTimerStart;
	unsigned int m_dwTimerElapsed;
	unsigned int m_dwTimerName;
};
#endif //_STATE_MACHINE_H_
