//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XClientSnapshot.cpp
//  Description: Snapshot manager class.
//
//  History:
//  - August 14, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XClientSnapshot.h"
#include "Stream.h"

///////////////////////////////////////////////
CXClientSnapshot::CXClientSnapshot()
{
	sv_maxcmdrate = GetISystem()->GetIConsole()->GetCVar("sv_maxcmdrate");

	Reset();
	SetSendPerSecond(0);


//	m_bFirstOne = true;
}

///////////////////////////////////////////////
CXClientSnapshot::~CXClientSnapshot()
{
}

///////////////////////////////////////////////
bool CXClientSnapshot::IsTimeToSend(float fFrameTimeInSec)
{
	int iServerMax = sv_maxcmdrate->GetIVal();
	int iSendPerSecond = min(iServerMax,(int)m_cSendPerSecond);

	unsigned int	nTimeToUpdate = 1000/iSendPerSecond;

	m_nTimer += (unsigned int)(fFrameTimeInSec*1000.0f);
	
	if(m_nTimer >= nTimeToUpdate)
		return true;

	return false;
}

///////////////////////////////////////////////
void CXClientSnapshot::Reset()
{
	m_ReliableStream.Reset();
	m_UnreliableStream.Reset();
	m_nTimer = 0;
}

///////////////////////////////////////////////
void CXClientSnapshot::SetSendPerSecond(BYTE cSendPerSecond)
{
	m_cSendPerSecond = cSendPerSecond?cSendPerSecond:25;
}

