////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   animsplinetrack.h
//  Version:     v1.00
//  Created:     22/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __animsplinetrack_h__
#define __animsplinetrack_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IMovieSystem.h"
#include "Spline.h"

#define MIN_TIME_PRECISION 0.01f

/*!
		Templated class that used as a base for all Tcb spline tracks.
 */
template <class ValueType>
class TAnimTcbTrack : public IAnimTrack
{
public:
	TAnimTcbTrack()
	{
		AllocSpline();
		m_flags = 0;
	}
	~TAnimTcbTrack()
	{
		delete m_spline;
	}

	int GetNumKeys()
	{
		return m_spline->num_keys();
	}

	void SetNumKeys( int numKeys )
	{
		m_spline->resize(numKeys);
	}

	void RemoveKey( int num )
	{
		m_spline->erase(num);
	}

	void GetKey( int index,IKey *key )
	{
		assert( index >= 0 && index < GetNumKeys() );
		assert( key != 0 );
		Spline::key_type &k = m_spline->key(index);
		ITcbKey *tcbkey = (ITcbKey*)key;
		tcbkey->time = k.time;
		tcbkey->flags = k.flags;
		
		tcbkey->tens = k.tens;
		tcbkey->cont = k.cont;
		tcbkey->bias = k.bias;
		tcbkey->easeto = k.easeto;
		tcbkey->easefrom = k.easefrom;

		tcbkey->SetValue( k.value );
	}

	void SetKey( int index,IKey *key )
	{
		assert( index >= 0 && index < GetNumKeys() );
		assert( key != 0 );
		Spline::key_type &k = m_spline->key(index);
		ITcbKey *tcbkey = (ITcbKey*)key;
		k.time = tcbkey->time;
		k.flags = tcbkey->flags;
		k.tens = tcbkey->tens;
		k.cont = tcbkey->cont;
		k.bias = tcbkey->bias;
		k.easeto = tcbkey->easeto;
		k.easefrom = tcbkey->easefrom;
		tcbkey->GetValue( k.value );
		Invalidate();
	}

	float GetKeyTime( int index )
	{
		assert( index >= 0 && index < GetNumKeys() );
		return m_spline->time(index);
	}
	void SetKeyTime( int index,float time )
	{
		assert( index >= 0 && index < GetNumKeys() );
		m_spline->key(index).time = time;
		Invalidate();
	}
	int GetKeyFlags( int index )
	{
		assert( index >= 0 && index < GetNumKeys() );
		return m_spline->key(index).flags;
	}
	void SetKeyFlags( int index,int flags )
	{
		assert( index >= 0 && index < GetNumKeys() );
		m_spline->key(index).flags = flags;
	}

	virtual EAnimTrackType GetType() { assert(0); return 0; }
	virtual EAnimValue GetValueType() { assert(0); return 0; }

	virtual void GetValue( float time,float &value ) { assert(0); }
	virtual void GetValue( float time,Vec3 &value ) { assert(0); }
	virtual void GetValue( float time,Quat &value ) { assert(0); }
	virtual void GetValue( float time,bool &value ) { assert(0); }

	virtual void SetValue( float time,const float &value,bool bDefault=false ) { assert(0); }
	virtual void SetValue( float time,const Vec3 &value,bool bDefault=false ) { assert(0); }
	virtual void SetValue( float time,const Quat &value,bool bDefault=false ) { assert(0); }
	virtual void SetValue( float time,const bool &value,bool bDefault=false ) { assert(0); }

	bool Serialize( XmlNodeRef &xmlNode,bool bLoading, bool bLoadEmptyTracks );

	void GetKeyInfo( int key,const char* &description,float &duration )
	{
		description = 0;
		duration = 0;
	}

	//! Sort keys in track (after time of keys was modified).
	void SortKeys() {
		m_spline->sort_keys();
	};

	//! Get track flags.
	int GetFlags() { return m_flags; };
	
	//! Set track flags.
	void SetFlags( int flags )
	{
		m_flags = flags;
		if (m_flags & ATRACK_LOOP)
		{
			m_spline->ORT( Spline::ORT_LOOP );
		}
		else if (m_flags & ATRACK_CYCLE)
		{
			m_spline->ORT( Spline::ORT_CYCLE );
		}
		else
		{
			m_spline->ORT( Spline::ORT_CONSTANT );
		}
		if (m_flags & ATRACK_LINEAR)
		{
			//m_spline->flag_set( ORT_LINEAR );
		}
	}

	void Invalidate() {
		m_spline->flag_set( Spline::MODIFIED );
	};

	void SetTimeRange( const Range &timeRange )
	{
		m_spline->SetRange( timeRange.start,timeRange.end );
	}

	int FindKey( float time )
	{
		// Find key with givven time.
		int num = m_spline->num_keys();
		for (int i = 0; i < num; i++)
		{
			float keyt = m_spline->key(i).time;
			if (fabs(keyt-time) < MIN_TIME_PRECISION)
			{
				return i;
			}
		}
		return -1;
	}

	//! Create key at givven time, and return its index.
	int CreateKey( float time )
	{
		ValueType value;
		
		int nkey = GetNumKeys();

		if (nkey > 0)
			GetValue( time,value );
		else
			value = m_defaultValue;

		SetNumKeys( nkey+1 );
		
		ITcbKey key;
		key.time = time;
		key.SetValue(value);
		SetKey( nkey,&key );

		return nkey;
	}

	int CloneKey( int srcKey )
	{
		ITcbKey key;
		GetKey( srcKey,&key );
		int nkey = GetNumKeys();
		SetNumKeys( nkey+1 );
		SetKey( nkey,&key );

		return nkey;
	}

	int CopyKey( IAnimTrack *pFromTrack, int nFromKey )
	{
		ITcbKey key;
		pFromTrack->GetKey( nFromKey,&key );
		int nkey = GetNumKeys();
		SetNumKeys( nkey+1 );
		SetKey( nkey,&key );
		return nkey;
	}

	//! Get key at givven time,
	//! If key not exist adds key at this time.
	void SetKeyAtTime( float time,IKey *key )
	{
		assert( key != 0 );

		key->time = time;
		
		bool found = false;
		// Find key with givven time.
		for (int i = 0; i < m_spline->num_keys(); i++)
		{
			float keyt = m_spline->key(i).time;
			if (fabs(keyt-time) < MIN_TIME_PRECISION)
			{
				SetKey( i,key );
				found = true;
				break;
			}
			//if (keyt > time)
				//break;
		}
		if (!found)
		{
			// Key with this time not found.
			// Create a new one.
			m_spline->resize( m_spline->num_keys()+1 );
			SetKey( m_spline->num_keys()-1,key );
			Invalidate();
		}
	}

private:
	//! Spawns new instance of Tcb spline.
	void AllocSpline()
	{
		m_spline = new Spline;
	}

	typedef TCBSpline<ValueType> Spline;
	Spline* m_spline;
	ValueType m_defaultValue;

	//! Keys of float track.
	int m_flags;
};

template <class T>
bool TAnimTcbTrack<T>::Serialize( XmlNodeRef &xmlNode,bool bLoading, bool bLoadEmptyTracks )
{
	if (bLoading)
	{
		int num = xmlNode->getChildCount();

		int flags = m_flags;
		xmlNode->getAttr( "Flags",flags );
		SetFlags( flags );

		T value;
		SetNumKeys( num );
		for (int i = 0; i < num; i++)
		{
			ITcbKey key; // Must be inside loop.

			XmlNodeRef keyNode = xmlNode->getChild(i);
			keyNode->getAttr( "time",key.time );
			
			if (keyNode->getAttr( "value",value ))
				key.SetValue( value );

			keyNode->getAttr( "tens",key.tens );
			keyNode->getAttr( "cont",key.cont );
			keyNode->getAttr( "bias",key.bias );
			keyNode->getAttr( "easeto",key.easeto );
			keyNode->getAttr( "easefrom",key.easefrom );

			SetKey( i,&key );
		}

		if ((!num) && (!bLoadEmptyTracks))
			return false;
	}
	else
	{
		int num = GetNumKeys();
		xmlNode->setAttr( "Flags",GetFlags() );
		ITcbKey key;
		T value;
		for (int i = 0; i < num; i++)
		{
			GetKey( i,&key );
			XmlNodeRef keyNode = xmlNode->newChild( "Key" );
			keyNode->setAttr( "time",key.time );
			
			key.GetValue( value );
			keyNode->setAttr( "value",value );

			if (key.tens != 0)
				keyNode->setAttr( "tens",key.tens );
			if (key.cont != 0)
				keyNode->setAttr( "cont",key.cont );
			if (key.bias != 0)
				keyNode->setAttr( "bias",key.bias );
			if (key.easeto != 0)
				keyNode->setAttr( "easeto",key.easeto );
			if (key.easefrom != 0)
				keyNode->setAttr( "easefrom",key.easefrom );
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//! Specialize for single float track.
template <> TAnimTcbTrack<float>::TAnimTcbTrack<float>()
{
	AllocSpline();
	m_flags = 0;
	m_defaultValue = 0;
}
template <> void TAnimTcbTrack<float>::GetValue( float time,float &value ) { m_spline->interpolate(time,value); }
template <> EAnimTrackType TAnimTcbTrack<float>::GetType() { return ATRACK_TCB_FLOAT; }
template <> EAnimValue TAnimTcbTrack<float>::GetValueType() { return AVALUE_FLOAT; }
template <> void TAnimTcbTrack<float>::SetValue( float time,const float &value,bool bDefault )
{
	if (!bDefault)
	{
		ITcbKey key;
		key.SetValue( value );
		SetKeyAtTime( time,&key );
	}
	else
		m_defaultValue = value;
}

//////////////////////////////////////////////////////////////////////////
template<> void TAnimTcbTrack<float>::GetKeyInfo( int index,const char* &description,float &duration )
{
	duration = 0;

	static char str[64];
	description = str;
	assert( index >= 0 && index < GetNumKeys() );
	Spline::key_type &k = m_spline->key(index);
	sprintf( str,"%g",k.value );
}

//////////////////////////////////////////////////////////////////////////
//! Specialize for Vector track.
template <> TAnimTcbTrack<Vec3>::TAnimTcbTrack<Vec3>()
{
	AllocSpline();
	m_flags = 0;
	m_defaultValue = Vec3(0,0,0);
}
template <> void TAnimTcbTrack<Vec3>::GetValue( float time,Vec3 &value ) { m_spline->interpolate(time,value); }
template <> EAnimTrackType TAnimTcbTrack<Vec3>::GetType() { return ATRACK_TCB_VECTOR; }
template <> EAnimValue TAnimTcbTrack<Vec3>::GetValueType() { return AVALUE_VECTOR; }
template <> void TAnimTcbTrack<Vec3>::SetValue( float time,const Vec3 &value,bool bDefault )
{
	if (!bDefault)
	{
		ITcbKey key;
		key.SetValue( value );
		SetKeyAtTime( time,&key );
	}
	else
		m_defaultValue = value;
}

//////////////////////////////////////////////////////////////////////////
template <> void TAnimTcbTrack<Vec3>::GetKeyInfo( int index,const char* &description,float &duration )
{
	duration = 0;

	static char str[64];
	description = str;

	assert( index >= 0 && index < GetNumKeys() );
	Spline::key_type &k = m_spline->key(index);
	sprintf( str,"%g,%g,%g",k.value[0],k.value[1],k.value[2] );
}

//////////////////////////////////////////////////////////////////////////
//! Specialize for Quaternion track.
//! Spezialize spline creation for quaternion.
template <> TAnimTcbTrack<Quat>::TAnimTcbTrack<Quat>()
{
	m_spline = new TCBQuatSpline;
	m_flags = 0;
	m_defaultValue.SetIdentity();
}

template <> void TAnimTcbTrack<Quat>::GetValue( float time,Quat &value ) { m_spline->interpolate(time,value); }
template <> EAnimTrackType TAnimTcbTrack<Quat>::GetType() { return ATRACK_TCB_QUAT; }
template <> EAnimValue TAnimTcbTrack<Quat>::GetValueType() { return AVALUE_QUAT; }
template <> void TAnimTcbTrack<Quat>::SetValue( float time,const Quat &value,bool bDefault )
{
	if (!bDefault)
	{
		ITcbKey key;
		key.SetValue( value );
		SetKeyAtTime( time,&key );
	}
	else
		m_defaultValue = value;
}

//////////////////////////////////////////////////////////////////////////
template <> void TAnimTcbTrack<Quat>::GetKeyInfo( int index,const char* &description,float &duration )
{
	duration = 0;

	static char str[64];
	description = str;

	assert( index >= 0 && index < GetNumKeys() );
	Spline::key_type &k = m_spline->key(index);
	Vec3 Angles=RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(k.value)));
	sprintf( str,"%g,%g,%g",Angles.x, Angles.y, Angles.z );
}

typedef TAnimTcbTrack<float>	CTcbFloatTrack;
typedef TAnimTcbTrack<Vec3>		CTcbVectorTrack;
typedef TAnimTcbTrack<Quat>		CTcbQuatTrack;

#endif // __animsplinetrack_h__
