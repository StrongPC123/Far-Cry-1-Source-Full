
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Timer.h
//
//	History:
//	01/31/2001 Created by Marco Corbetta
//  08/03/2003 MartinM improved precision (intern from float to int64)
//
//////////////////////////////////////////////////////////////////////

#ifndef TIMER_H
#define TIMER_H

#if _MSC_VER > 1000
# pragma once
#endif




//////////////////////////////////////////////////////////////////////
#include <ITimer.h>						// ITimer

#define FPS_FRAMES 16

/*
===========================================
The Timer interface Class
===========================================
*/

#define TIME_PROFILE_PARAMS 256

//! Implements all common timing routines
class CTimer : public ITimer
{
public:
	//! constructor
	CTimer();
	//! destructor
	~CTimer() {};

	//! @name ITimer implementation
	//@{
	bool Init( ISystem *pSystem );
	virtual void Reset();		
	virtual void Update();
	virtual void Enable(bool bEnable);

	virtual const CTimeValue GetCurrTimePrecise() const
	{
		int64 llNow=(*m_pfnUpdate)()-m_lBaseTime;

		// can be done much better
		double fac=(double)TIMEVALUE_PRECISION/(double)m_lTicksPerSec;

		return CTimeValue(int64(llNow*fac));
	}

	//! \return in seconds
	virtual inline const float GetCurrTime() const 
	{
		return m_fCurrTime;
	}
	
//	virtual const CTimeValue GetFrameStart() const;

	virtual inline const float GetAsyncCurTime()  
	{ 
		int64 llNow=(*m_pfnUpdate)()-m_lBaseTime;
		double dVal=(double)llNow;
		float fRet=(float)(dVal/(double)(m_lTicksPerSec));

		return fRet; 
	}
	
	virtual inline const float GetFrameTime() const 
	{ 
		return m_fFrameTime;   
	} 
	
	virtual float	GetFrameRate();
	virtual float gfGetTime() { return GetAsyncCurTime(); }				// to be removed
  virtual float MeasureTime(const char* comment);
	bool GetBasicStats( float & fStatsLogic, float & fStatsRender, float & fStatsSumm );
	//@}
private:
	
	typedef int64 (*TimeUpdateFunc) ();	//  absolute, in microseconds,

	//! get the timer in microseconds. using QueryPerformanceCounter
	static int64	GetPerformanceCounterTime();
	//! get the timer in microseconds. using winmm
	static int64	GetMMTime();
	//! updates m_fCurrTime
	void RefreshCurrTime();
		
	TimeUpdateFunc	m_pfnUpdate;								//!< pointer to the timer function (performance counter or timegettime)
	int64						m_lBaseTime;								//!< absolute in ticks, 1 sec = m_lTicksPerSec units
	int64						m_lLastTime;								//!< absolute in ticks, 1 sec = m_lTicksPerSec units

	int64						m_lTicksPerSec;							//!< units per sec

	int64						m_lCurrentTime;							//!< absolute, in microseconds, at the CTimer:Update() call
	float						m_fFrameTime;								//!< in seconds since the last update
	float						m_fCurrTime;								//!< absolute time in seconds
	bool						m_bEnabled;									//!<

	// smoothing
	float						m_previousTimes[FPS_FRAMES];//!<
	int							m_timecount;								//!<

	//////////////////////////////////////////////////////////////////////////
	// Console vars.
	//////////////////////////////////////////////////////////////////////////
  int							m_e_time_smoothing;					//!< [0..FPS_FRAMES-2] optional smoothing, 0=no, x=x additional steps,  
  int							m_e_time_profiling;					//!< 
	float						m_fixed_time_step;					//!< 
	//////////////////////////////////////////////////////////////////////////

  struct time_info_struct
  {
    time_info_struct() { param_name[0]=0; value=0; }
    char param_name[32];
    float value;
  } m_TimeInfoTable[TIME_PROFILE_PARAMS];			//!< this table contain times and code point names

  int							m_nCurProfLine;  						//!< 
	// todo: change from float to int64
  float						m_fProfStartTime;						//!< in milliseconds, -1 menst value is not set

	ISystem *				m_pSystem;									//!< 
	
	// game/render ratio profiling
	float						m_fStatsLogic;							//!< 
	float						m_fStatsRender;							//!< 
	float						m_fStatsSumm;								//!< 
  int							m_nCurProfLinesNum;  				//!< 
};





/*

int _tmain(int argc, _TCHAR* argv[])
{
	int64 step=1000000;						// 1 sec
	int64 start=1*1000;						// 1 ms

	step*=3600;											// 1 hour

	for(int64 i=start;;i+=step)		// 1 day step
	{
		float f1=(float)(i/1000000.0f);		// convert from microsec to sec
		float f2=(i/1000)/1000.0f;	// convert from microsec to sec

		printf("f1 sec:%f, hours:%f, days:%f\n",f1,f1/3600.0f,f1/3600.0f/24.0f);
		printf("f2 sec:%f, hours:%f, days:%f\n",f2,f2/3600.0f,f2/3600.0f/24.0f);
	}
	return 0;
}

Output:
f1 sec:0.001000, hours:0.000000, days:0.000000
f2 sec:0.001000, hours:0.000000, days:0.000000
f1 sec:3600.000977, hours:1.000000, days:0.041667
f2 sec:3600.000977, hours:1.000000, days:0.041667
f1 sec:7200.000977, hours:2.000000, days:0.083333
f2 sec:7200.000977, hours:2.000000, days:0.083333
f1 sec:10800.000977, hours:3.000000, days:0.125000
f2 sec:10800.000977, hours:3.000000, days:0.125000
f1 sec:14400.000977, hours:4.000000, days:0.166667
f2 sec:14400.000977, hours:4.000000, days:0.166667
f1 sec:18000.001953, hours:5.000001, days:0.208333
f2 sec:18000.001953, hours:5.000001, days:0.208333
f1 sec:21600.001953, hours:6.000001, days:0.250000
f2 sec:21600.001953, hours:6.000001, days:0.250000
f1 sec:25200.001953, hours:7.000001, days:0.291667
f2 sec:25200.001953, hours:7.000001, days:0.291667
f1 sec:28800.001953, hours:8.000001, days:0.333333
f2 sec:28800.001953, hours:8.000001, days:0.333333
f1 sec:32400.001953, hours:9.000001, days:0.375000
f2 sec:32400.001953, hours:9.000001, days:0.375000
f1 sec:36000.000000, hours:10.000000, days:0.416667			* at that point we lost the 1ms precision completely
f2 sec:36000.000000, hours:10.000000, days:0.416667     * f2 is showing the same problem, that means it's not the calculation
f1 sec:39600.000000, hours:11.000000, days:0.458333       it's the float precision that causes that
f2 sec:39600.000000, hours:11.000000, days:0.458333
f1 sec:43200.000000, hours:12.000000, days:0.500000
f2 sec:43200.000000, hours:12.000000, days:0.500000
f1 sec:46800.000000, hours:13.000000, days:0.541667
f2 sec:46800.000000, hours:13.000000, days:0.541667
f1 sec:50400.000000, hours:14.000000, days:0.583333
f2 sec:50400.000000, hours:14.000000, days:0.583333
f1 sec:54000.000000, hours:15.000000, days:0.625000
f2 sec:54000.000000, hours:15.000000, days:0.625000
f1 sec:57600.000000, hours:16.000000, days:0.666667
f2 sec:57600.000000, hours:16.000000, days:0.666667
f1 sec:61200.000000, hours:17.000000, days:0.708333
f2 sec:61200.000000, hours:17.000000, days:0.708333
f1 sec:64800.000000, hours:18.000000, days:0.750000
f2 sec:64800.000000, hours:18.000000, days:0.750000
*/


#endif //timer