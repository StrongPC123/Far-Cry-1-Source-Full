////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   range.h
//  Version:     v1.00
//  Created:     25/4/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Time TRange class.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __range_h__
#define __range_h__

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

/*!
		Class TRange, can represent anything that is range between two values, mostly used for time ranges.
 */
template <class T>
class	TRange
{
public:
	T start;
	T end;

	TRange()	{ start = 0; end = 0; };
	TRange( const TRange &r ) { start = r.start; end = r.end; };
	TRange( T s,T e ) { start = s; end = e; };

	void Set( T s,T e ) { start = s; end = e; };
	void Clear() { start = 0; end = 0; };

	//! Get length of range.
	T	Length() const { return end - start; };
	//! Check if range is empty.
	bool IsEmpty()	const { return (start == 0 && end == 0); }

	//! Check if value is inside range.
	bool IsInside( T val ) { return val >= start && val <= end; };

	void ClipValue( T &val )
	{
		if (val < start) val = start;
		if (val > end) val = end;
	}

	//! Compare two ranges.
	bool	operator == ( const TRange &r ) const {
		return start == r.start && end == r.end;
	}
	//! Assign operator.
	TRange&	operator =( const TRange &r ) {
		start = r.start;
		end = r.end;
		return *this;
	}
	//! Interect two ranges.
	TRange	operator & ( const TRange &r )	{
		return TRange( MAX(start,r.start),MIN(end,r.end) );
	}
	TRange&	operator &= ( const TRange &r )	{
		return (*this = (*this & r));
	}
	//! Concatent two ranges.
	TRange	operator | ( const TRange &r )	{
		return TRange( MIN(start,r.start),MAX(end,r.end) );
	}
	TRange&	operator |= ( const TRange &r )	{
		return (*this = (*this | r));
	}
	//! Add new value to range.
	TRange	operator + ( T v )	{
		T s = start, e = end;
		if (v < start) s = v;
		if (v > end) e = v;
		return TRange( s,e );
	}
	//! Add new value to range.
	TRange&	operator += ( T v )	{
		if (v < start) start = v;
		if (v > end) end = v;
		return *this;
	}
};

//! CRange if just TRange for floats..
typedef TRange<float> Range;

#endif // __range_h__