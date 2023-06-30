//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//	History:
//		03/15/2003 MartinM created
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "synchedrandomseed.h"
#include "XPlayer.h"								// CPlayer


float CSynchedRandomSeed::m_fRandomTable[]=
{
	0.809f,0.585f,0.480f,0.350f,0.896f,0.823f,0.747f,0.174f,0.859f,0.711f,0.514f,0.304f,0.015f,0.091f,0.364f,0.147f,
	0.166f,0.989f,0.446f,0.119f,0.005f,0.009f,0.378f,0.532f,0.571f,0.602f,0.607f,0.166f,0.663f,0.451f,0.352f,0.057f,
	0.608f,0.783f,0.803f,0.520f,0.302f,0.876f,0.727f,0.956f,0.926f,0.539f,0.142f,0.462f,0.235f,0.862f,0.210f,0.780f,
	0.844f,0.997f,1.000f,0.611f,0.392f,0.266f,0.297f,0.840f,0.024f,0.376f,0.093f,0.677f,0.056f,0.009f,0.919f,0.276f,
	0.273f,0.588f,0.691f,0.838f,0.726f,0.485f,0.205f,0.744f,0.468f,0.458f,0.949f,0.744f,0.108f,0.599f,0.385f,0.735f,
	0.609f,0.572f,0.361f,0.152f,0.225f,0.425f,0.803f,0.517f,0.990f,0.752f,0.346f,0.169f,0.657f,0.492f,0.064f,0.700f,
	0.505f,0.147f,0.950f,0.142f,0.905f,0.693f,0.303f,0.427f,0.070f,0.967f,0.683f,0.153f,0.877f,0.822f,0.582f,0.191f,
	0.178f,0.817f,0.475f,0.156f,0.504f,0.732f,0.406f,0.280f,0.569f,0.682f,0.756f,0.722f,0.475f,0.123f,0.368f,0.835f,
	0.035f,0.517f,0.663f,0.426f,0.105f,0.949f,0.921f,0.550f,0.346f,0.472f,0.375f,0.847f,0.317f,0.456f,0.272f,0.983f,
	0.298f,0.739f,0.567f,0.196f,0.761f,0.839f,0.398f,0.501f,0.890f,0.027f,0.995f,0.573f,0.051f,0.531f,0.194f,0.843f,
	0.627f,0.658f,0.198f,0.842f,0.123f,0.110f,0.743f,0.314f,0.941f,0.286f,0.336f,0.140f,0.733f,0.835f,0.708f,0.600f,
	0.747f,0.253f,0.144f,0.002f,0.061f,0.806f,0.853f,0.211f,0.116f,0.553f,0.014f,0.114f,0.455f,0.752f,0.686f,0.543f,
	0.074f,0.437f,0.202f,0.696f,0.290f,0.437f,0.232f,0.578f,0.533f,0.629f,0.160f,0.504f,0.963f,0.696f,0.925f,0.190f,
	0.336f,0.178f,0.995f,0.457f,0.998f,0.098f,0.625f,0.094f,0.438f,0.932f,0.048f,0.895f,0.290f,0.227f,0.769f,0.411f,
	0.202f,0.628f,0.604f,0.452f,0.466f,0.598f,0.635f,0.855f,0.829f,0.625f,0.721f,0.566f,0.375f,0.184f,0.738f,0.555f,
	0.905f,0.243f,0.189f,0.605f,0.699f,0.585f,0.351f,0.494f,0.080f,0.741f,0.612f,0.620f,0.691f,0.805f,0.149f,0.576f,
};

float CSynchedRandomSeed::GetRandTable(BYTE idx)
{ 
	return m_fRandomTable[idx];
}


CSynchedRandomSeed::CSynchedRandomSeed()
{
	m_ucRandomSeedCS=0;		// inital value doesn't matter
	m_ucStartRandomSeedCS=0;
	m_bSynchToAllClientsS=true;
	m_pParentPlayer=0;
}

CSynchedRandomSeed::~CSynchedRandomSeed()
{
}



void CSynchedRandomSeed::SetStartRandomSeedC( const uint8 Value )
{
	assert(m_pParentPlayer);

	m_ucStartRandomSeedCS=Value;
}

uint8 CSynchedRandomSeed::GetStartRandomSeedS()
{
	return m_ucStartRandomSeedCS;
}

void CSynchedRandomSeed::SetParent( CPlayer *pPlayer )
{
	assert(pPlayer);

	m_pParentPlayer=pPlayer;
}

void CSynchedRandomSeed::IncreaseRandomSeedC()
{
	assert(m_pParentPlayer);	// call SetParent()

	m_ucRandomSeedCS++;		// 0..255 -> 0..255 -> ..

	m_LastTimeRandomSeedChangedC = m_pParentPlayer->m_pTimer->GetCurrTimePrecise();
}


uint8 CSynchedRandomSeed::GetRandomSeedC()
{
	assert(m_pParentPlayer);	// call SetParent()

	if(IsTimeToSyncToServerC())
	{
		if(!m_pParentPlayer->IsMyPlayer())
			m_ucRandomSeedCS=m_ucStartRandomSeedCS;
	}

	return m_ucRandomSeedCS;
}

bool CSynchedRandomSeed::IsTimeToSyncToServerC()
{
	assert(m_pParentPlayer);	// call SetParent()

	CTimeValue yet=m_pParentPlayer->m_pTimer->GetCurrTimePrecise();

	CTimeValue PauseTime;			PauseTime.SetMilliSeconds(300);		// hard coded value, maybe dependent on the ping would be better

	return yet-m_LastTimeRandomSeedChangedC > PauseTime;
}


uint8 CSynchedRandomSeed::GetStartRandomSeedC()
{
	assert(m_pParentPlayer);	// call SetParent()

	if(IsTimeToSyncToServerC())
	{
		if(m_pParentPlayer->IsMyPlayer())
			m_ucStartRandomSeedCS=m_ucRandomSeedCS;
	}

	return m_ucStartRandomSeedCS;
}

bool CSynchedRandomSeed::GetSynchToClientsS() const
{
	return m_bSynchToAllClientsS;
}


void CSynchedRandomSeed::EnableSyncRandomSeedS( const uint8 Value )
{
	m_ucStartRandomSeedCS=Value;
	m_bSynchToAllClientsS=true;
}

void CSynchedRandomSeed::DisableSyncRandomSeedS()
{
	m_bSynchToAllClientsS=false;
}