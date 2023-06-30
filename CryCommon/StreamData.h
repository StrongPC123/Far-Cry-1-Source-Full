#pragma once

#include "ISystem.h"						// ISystem
#include "IGame.h"							// IGame
#include "IStreamEngine.h"			// IStreamEngine

#if defined(LINUX)
//	#include "stream.h"
#endif

// usable:
//
//		CStreamData_WorldPos			Vec3(x,y,z) world position with fallback if outside of typical area
//		CStreamData_Normal				normalized Vec3(x,y,z) in very well compressed in 16bit (cubemap side+2d pos on cubemap)

// next possible candidates:
//
//		Direction								Vec3, components in range -1..1
//		Angle										range: 0..2*PI
//		EulerAnglesXYZ					components in range: 0..2*PI
//		EulerAnglesXZ						components in range: 0..2*PI
//

// todo:
//
//    test the whole system in multiplayer
//		save stream compression mask in file or init 0 for load and save
//		check if fallback is working

// -------------------------------------------------------------------------------------------------------------

class CStreamDataBase :public IStreamData
{
public:
	//!
	static bool DoCompression( const DWORD indwMask )
	{
		ISystem *pSystem=GetISystem();														assert(pSystem);
		IGame *pGame=pSystem->GetIGame();													assert(pGame);
		IStreamEngine *pStreamE=pSystem->GetStreamEngine();				assert(pStreamE);

		if(!pGame->GetModuleState(EGameMultiplayer))
			return false;									// compression is only needed to save bandwidth

		return (pStreamE->GetStreamCompressionMask()&indwMask) !=0;
	}
};


// -------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------


// world position (always positive) 96 -> 59 bits for 2mm resolution
class CStreamData_WorldPos :public CStreamDataBase
{
	Vec3 &										m_vData;			//!< reference to the Vec3 data

public:	// --------------------------

	//! constructor
	CStreamData_WorldPos( Vec3 &inoutvValue ) :m_vData(inoutvValue)
	{
	}
	
	//! to reproduce the quality loss form compression and decompression
	Vec3 GetCompressed() const
	{
		CStream stm;

		Write(stm);Read(stm);

		return m_vData;
	}

	static bool IsActive()
	{
		ISystem *pSystem=GetISystem();														assert(pSystem);
		IGame *pGame=pSystem->GetIGame();													assert(pGame);

//		return DoCompression(0x1);
		return pGame->GetModuleState(EGameMultiplayer);			// compression is only needed to save bandwidth
	}

	// interface IStreamData -----------------------------

	virtual bool Read( CStream &inStream ) const
	{
		if(!IsActive())																// same mask for read and write
			return inStream.Read(m_vData);

		bool bCompressed;

		if(!inStream.Read(bCompressed)) return false;

		if(!bCompressed)
			return inStream.Read(m_vData);

		DWORD x=0,y=0,z=0;

		if(!inStream.ReadNumberInBits((unsigned int&)x, (size_t)20)) return false;
		if(!inStream.ReadNumberInBits((unsigned int&)y, (size_t)20)) return false;
		if(!inStream.ReadNumberInBits((unsigned int&)z, (size_t)18)) return false;

		m_vData=Vec3((float)x,(float)y,(float)z) * 0.002f;		// 2mm units

		return true;
	}

	virtual bool Write( CStream &inStream ) const
	{
		if(!IsActive())																// same mask for read and write
			return inStream.Write(m_vData);

		DWORD x,y,z;

		if(m_vData.x<0 || m_vData.x>2048.0f										// do a fallback outside of the typical world area
		|| m_vData.y<0 || m_vData.y>2048.0f
		|| m_vData.z<0 || m_vData.z>512.0f)
		{
			if(!inStream.Write(false)) return false;						// uncompressed
			return inStream.Write(m_vData);
		}

		x=(DWORD)(m_vData.x*500.0f+0.5f);											// 2mm units +0.5f for rounding
		y=(DWORD)(m_vData.y*500.0f+0.5f);
		z=(DWORD)(m_vData.z*500.0f+0.5f);

		if(!inStream.Write(true)) return false;								// compressed

		if(!inStream.WriteNumberInBits((unsigned int)x, 20)) return false;
		if(!inStream.WriteNumberInBits((unsigned int)y, 20)) return false;
		if(!inStream.WriteNumberInBits((unsigned int)z, 18)) return false;

		return true;
	}
};



// -------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------


// low quality (16bit) normalized vector 96 -> 16 bits
// projected on a cube: x=0..104 x y=0..140 x side=0..5 -> 0.. 64896
class CStreamData_Normal :public CStreamDataBase
{
	Vec3 &										m_vData;			//!< reference to the Vec3 data

public:	// --------------------------

	//! constructor
	CStreamData_Normal( Vec3 &inoutvValue ) :m_vData(inoutvValue)
	{
	}

	//! to reproduce the quality loss form compression and decompression
	Vec3 GetCompressed() const
	{
		CStream stm;

		Write(stm);Read(stm);

		return m_vData;
	}

	static bool IsActive()
	{
		ISystem *pSystem=GetISystem();														assert(pSystem);
		IGame *pGame = pSystem->GetIGame();													assert(pGame);

//		return DoCompression(0x4);
		return pGame->GetModuleState(EGameMultiplayer);			// compression is only needed to save bandwidth
	}

	// interface IStreamData -----------------------------

	virtual bool Read( CStream &inStream ) const
	{
		if(!IsActive())																// same mask for read and write
			return inStream.Read(m_vData);

		DWORD dwWord=0;

		if(!inStream.ReadNumberInBits((unsigned int&)dwWord, (size_t)16)) return false;

		DWORD dwDominantDirection=dwWord%6;
		DWORD dwIntermediate=dwWord / 6;	// 6 sides
		DWORD dwX=dwIntermediate % 104;		// 104 values on a cube size
		DWORD dwY=dwIntermediate / 104;		// 104 values on a cube size

		float fX=(((float)dwX+0.5f)/(float)(104))*2.0f-1.0f;
		float fY=(((float)dwY+0.5f)/(float)(104))*2.0f-1.0f;

		switch(dwDominantDirection)
		{
			case 0:	m_vData[0]=1.0f;		m_vData[1]=fX;			m_vData[2]=fY;		break;
			case 1: m_vData[0]=fY;			m_vData[1]=1.0f;		m_vData[2]=fX;		break;
			case 2: m_vData[0]=fX;			m_vData[1]=fY;			m_vData[2]=1.0f;	break;
			case 3: m_vData[0]=-1.0f;		m_vData[1]=fX;			m_vData[2]=fY;		break;
			case 4: m_vData[0]=fY;			m_vData[1]=-1.0f;		m_vData[2]=fX;		break;
			case 5: m_vData[0]=fX;			m_vData[1]=fY;			m_vData[2]=-1.0f;	break;
			default: assert(0);
		}

		m_vData.Normalize();

		return true;
	}

	virtual bool Write( CStream &inStream ) const
	{
		if(!IsActive())																// same mask for read and write
			return inStream.Write(m_vData);

		int iDominantDirection=-1;

		float vAbs[3]={ m_vData[0], m_vData[1], m_vData[2] };

		// calc absolute value
		if(vAbs[0]<0)vAbs[0]=-vAbs[0];
		if(vAbs[1]<0)vAbs[1]=-vAbs[1];
		if(vAbs[2]<0)vAbs[2]=-vAbs[2];

		if(vAbs[0]>=vAbs[1] && vAbs[0]>=vAbs[2])iDominantDirection=0;
		else
		if(vAbs[1]>=vAbs[0] && vAbs[1]>=vAbs[2])iDominantDirection=1;
		else
		if(vAbs[2]>=vAbs[1] && vAbs[2]>=vAbs[0])iDominantDirection=2;

		assert(iDominantDirection!=-1 && "this should never hapen or the vector has zero length");
		if(iDominantDirection==-1)iDominantDirection=0;

		float fX=m_vData[(iDominantDirection+1)%3];
		float fY=m_vData[(iDominantDirection+2)%3];
		float fZ=m_vData[iDominantDirection];

		if(vAbs[iDominantDirection]!=m_vData[iDominantDirection])iDominantDirection+=3;

		float fScale=1.0f/(float)fabs(fZ);

		fX*=fScale;fY*=fScale;

		DWORD dwX=(DWORD)((fX*0.5f+0.5f)*104);		// 104 values on a cube size
		DWORD dwY=(DWORD)((fY*0.5f+0.5f)*104);		// 104 values on a cube size

		if(dwX>=104)dwX=104-1;										// 104 values on a cube size
		if(dwY>=104)dwY=104-1;										// 104 values on a cube size

		DWORD dwWord=iDominantDirection+6*(dwX+104*dwY);	// 104 values on a cube size

		if(!inStream.WriteNumberInBits((unsigned int)dwWord, 16)) return false;

		return true;
	}
};
