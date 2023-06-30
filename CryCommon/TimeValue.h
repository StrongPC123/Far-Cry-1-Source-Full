#ifndef _TIMEVALUE_H_
#define _TIMEVALUE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define TIMEVALUE_PRECISION		1000

class CTimeValue
{
public:
	//! default constructor
	CTimeValue()
	{
		m_lValue=0;
	}

	//! constructor
	// /param inllValue positive negative, absolute or relative in 1 second= TIMEVALUE_PRECISION units
	CTimeValue( const int64 &inllValue )
	{
		m_lValue=inllValue;
	}

	//! copy constructor
	CTimeValue( const CTimeValue &inValue )
	{
		m_lValue=inValue.m_lValue;
	}

	//! destructor, virtual to ensure correct memory deallocation
	virtual ~CTimeValue(){}
	
	//! assignment operator
	//! /param inRhs right side
	CTimeValue operator=( const CTimeValue &inRhs )
	{
		m_lValue = inRhs.m_lValue;
		return *this;
	};

	//! use only for relative value, absolute values suffer a lot from precision loss
	float GetSeconds() const
	{
		return m_lValue/(float)TIMEVALUE_PRECISION;
	}

	//!
	void SetSeconds( const DWORD indwSec )
	{
		m_lValue=indwSec*TIMEVALUE_PRECISION;
	}

	//!
	void SetMilliSeconds( const DWORD indwMilliSec )
	{
		m_lValue=indwMilliSec*(TIMEVALUE_PRECISION/1000);
	}

	//! use only for relative value, absolute values suffer a lot from precision loss
	float GetMilliSeconds() const
	{
		return m_lValue/(float)(TIMEVALUE_PRECISION/1000);
	}

	// math operations -----------------------

	//! minus
	CTimeValue operator-( const CTimeValue &inRhs ) const {	CTimeValue ret;	ret.m_lValue = m_lValue - inRhs.m_lValue;return ret; };
	//! plus
	CTimeValue operator+( const CTimeValue &inRhs ) const {	CTimeValue ret;	ret.m_lValue = m_lValue + inRhs.m_lValue;return ret;	};
	//! unary minus
	CTimeValue operator-() const { CTimeValue ret; ret.m_lValue = -m_lValue;return ret; };

	// comparison -----------------------

	bool operator<( const CTimeValue &inRhs ) const {	return m_lValue < inRhs.m_lValue; };
	bool operator>( const CTimeValue &inRhs ) const {	return m_lValue > inRhs.m_lValue;	};
	bool operator>=( const CTimeValue &inRhs ) const { return m_lValue >= inRhs.m_lValue;	};
	bool operator<=( const CTimeValue &inRhs ) const { return m_lValue <= inRhs.m_lValue;	};
	bool operator==( const CTimeValue &inRhs ) const { return m_lValue == inRhs.m_lValue;	};
	bool operator!=( const CTimeValue &inRhs ) const { return m_lValue != inRhs.m_lValue;	};

private:
	int64 m_lValue;				//!< absolute or relative value in 1/TIMEVALUE_PRECISION, might be negative
};

#endif // _TIMEVALUE_H_
