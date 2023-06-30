//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XEntityProcessingCmd.h
//  Description: Command processing helper class.
//
//  History:
//  - August 23, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_XENTITYPROCESSINGCMD_H
#define GAME_XENTITYPROCESSINGCMD_H

//////////////////////////////////////////////////////////////////////////////////////////////
//! Structure used to hold the commands
class CXEntityProcessingCmd
{
public:
	//! constructor
	CXEntityProcessingCmd();
	//! destructor
	virtual ~CXEntityProcessingCmd();

	void Release() { delete this; }
	
	// Flag management... mouarf
	void AddAction(unsigned int nFlags);
	void RemoveAction(unsigned int nFlags);
	void Reset();
	bool CheckAction(unsigned int nFlags);

	// Angles...
	Vec3& GetDeltaAngles();
	void SetDeltaAngles( const Vec3 &ang );
	void SetPhysicalTime(int iPTime)//,float fTimeSlice)
	{
		m_iPhysicalTime=iPTime;
		//m_fTimeSlice=fTimeSlice;
	}
	int GetPhysicalTime()
	{
		return m_iPhysicalTime;
	}
	void SetLeaning(float fLeaning){m_fLeaning=fLeaning;}
	float GetLeaning(){return m_fLeaning;}

	int AddTimeSlice(float *pfSlice,int nSlices=1)
	{
		for(int i=0; i<nSlices && m_nTimeSlices<sizeof(m_fTimeSlices)/sizeof(m_fTimeSlices[0]); i++,m_nTimeSlices++)
			m_fTimeSlices[m_nTimeSlices] = pfSlice[i];
		return m_nTimeSlices;
	}
	int InsertTimeSlice(float fSlice,int iBefore=0)
	{
		m_nTimeSlices = min(m_nTimeSlices+1,sizeof(m_fTimeSlices)/sizeof(m_fTimeSlices[0]));
		for(int i=m_nTimeSlices; i>iBefore; i--)
			m_fTimeSlices[i] = m_fTimeSlices[i-1];
		m_fTimeSlices[iBefore] = fSlice;
		return m_nTimeSlices;
	}
	int GetTimeSlices(float *&pfSlices)
	{
		pfSlices = m_fTimeSlices;
		return m_nTimeSlices;
	}
	void ResetTimeSlices() { m_nTimeSlices=0; }

	void SetPhysDelta(float fServerDelta,float fClientDelta=0)
	{
		m_fServerDelta=fServerDelta;
		m_fClientDelta=fClientDelta;
	}
	float GetServerPhysDelta(){return m_fServerDelta;}
	float GetClientPhysDelta(){return m_fClientDelta;}

	//! \param pBitStream must not be 0 (compressed or uncompressed)
	bool Write( CStream &stm, IBitStream *pBitStream, bool bWriteAngles );

	//! \param pBitStream must not be 0 (compressed or uncompressed)
	bool Read( CStream &stm, IBitStream *pBitStream );

	
public: // ------------------------------------------------------------------

	unsigned int			m_nActionFlags[2];	//!< Timedemo recorded needs access to these.

private: 	// ------------------------------------------------------------------

	int								m_iPhysicalTime;		//!<
	Ang3							m_vDeltaAngles;			//!<
	float							m_fTimeSlices[32];	//!<
	unsigned char			m_nTimeSlices;			//!<

	// non serialized variables -----------------------------------------

	float							m_fServerDelta;			//!<
	float							m_fClientDelta;			//!<
	float							m_fLeaning;					//!<
};

#endif // GAME_XENTITYPROCESSINGCMD_H
