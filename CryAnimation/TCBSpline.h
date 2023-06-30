////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tcbspline.h
//  Version:     v1.00
//  Created:     12/04/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __tcbspline_h__
#define __tcbspline_h__
#pragma once

#if _MSC_VER > 1000
#pragma once
#endif

template	<int N>
class	BasisFunction	{
public:
	const float& operator[]( int i ) const { return m_f[i]; };
//	const float& operator[]( int i ) const { return m_f[i]; };
protected:
	float	m_f[N];
};

// Special functions that makes parameter zero.
template <class T>
inline void Zero( T &val )
{
	memset( &val,0,sizeof(val) );
} 


///////////////////////////////////////////////////////////////////////////////
// HermitBasis.
class	HermitBasis : public BasisFunction<4>
{
public:
	HermitBasis( float t ) {
		float t2,t3,t2_3,t3_2,t3_t2;
		
		t2 = t*t;												// t2 = t^2;
		t3 = t2*t;											// t3 = t^3;
		
		t3_2 = t3 + t3;
		t2_3 = 3*t2;
		t3_t2 = t3 - t2;
		m_f[0] = t3_2 - t2_3 + 1;
		m_f[1] = -t3_2 + t2_3;
		m_f[2] = t3_t2 - t2 + t;
		m_f[3] = t3_t2;
	}
};

inline float fast_fmod( float x,float y )
{
	return cry_fmod( x,y );
	//int ival = ftoi(x/y);
	//return x - ival*y;
}

/****************************************************************************
**                            Key classes																	 **
****************************************************************************/
template	<class T>
struct	SplineKey
{
	typedef	T	value_type;

	float				time;		//!< Key time.
	int					flags;	//!< Key flags.
	value_type	value;	//!< Key value.
	value_type	ds;			//!< Incoming tangent.
	value_type	dd;			//!< Outgoing tangent.
};

template	<class T>
bool	operator ==( const SplineKey<T> &k1,const SplineKey<T> &k2 ) { return k1.time == k2.time; };
template	<class T>
bool	operator !=( const SplineKey<T> &k1,const SplineKey<T> &k2 ) { return k1.time != k2.time; };
template	<class T>
bool	operator < ( const SplineKey<T> &k1,const SplineKey<T> &k2 ) { return k1.time < k2.time; };
template	<class T>
bool	operator > ( const SplineKey<T> &k1,const SplineKey<T> &k2 ) { return k1.time > k2.time; };

/****************************************************************************
**                           TCBSplineKey classes													 **
****************************************************************************/
/*!
	TCB spline key.
*/
template	<class T>
struct TCBSplineKey :  public SplineKey<T>
{
	// Key controls.
	float tens;         //!< Key tension value.
  float cont;         //!< Key continuity value.
  float bias;         //!< Key bias value.
  float easeto;       //!< Key ease to value.
  float easefrom;     //!< Key ease from value.

	TCBSplineKey() { tens = 0, cont = 0, bias = 0, easeto = 0, easefrom = 0; };
};

//! TCB spline key used in quaternion spline with angle axis as input.
struct TCBAngAxisKey :  public TCBSplineKey<CryQuat>
{
	float angle;
	Vec3 axis;
	
	TCBAngAxisKey() : axis(0,0,0),angle(0) {};
};

/****************************************************************************
**                           Spline class  																 **
****************************************************************************/
template <class KeyType,class BasisType>
class	TSpline
{
public:
	typedef	KeyType		key_type;
	typedef typename KeyType::value_type	value_type;
	typedef	BasisType	basis_type;

	// Out of range types.
	enum	{
		ORT_CONSTANT				=	0x0001,	// Constant track.
		ORT_CYCLE						=	0x0002,	// Cycle track
		ORT_LOOP						=	0x0003,	// Loop track.
		ORT_OSCILLATE				=	0x0004,	// Oscillate track.
		ORT_LINEAR					=	0x0005,	// Linear track.
		ORT_RELATIVE_REPEAT	=	0x0007	// Realtive repeat track.
	};
	// Spline flags.
	enum	{
		MODIFIED						=	0x0001,	// Track modified.
	};

	/////////////////////////////////////////////////////////////////////////////
	// Methods.
	TSpline();
	virtual	~TSpline() {};

	void	flag_set( int flag ) { m_flags |= flag; };
	void	flag_clr( int flag ) { m_flags &= ~flag; };
	int		flag( int flag )  { return m_flags&flag; };

	void	ORT( int ort ) { m_ORT = ort; };
	int		ORT() const { return m_ORT; };
	int		isORT( int o ) const { return (m_ORT == o); };

	void	SetRange( float start,float end ) { m_rangeStart = start; m_rangeEnd = end; };
	float GetRangeStart() const { return m_rangeStart; };
	float GetRangeEnd() const { return m_rangeEnd; };

	// Keys access methods.
	void	reserve_keys( int num ) { m_keys.reserve(num); };			// Reserve memory for more keys.
	void	resize( int num )		{ m_keys.resize(num); m_flags |= MODIFIED; };			// Set new key count.
	bool	empty() const								{ return m_keys.empty(); };		// Check if curve empty (no keys).
	int		num_keys() const						{ return (int)m_keys.size(); };		// Return number of keys in curve.
	key_type&	key( int n )						{ return m_keys[n]; };				// Return n key.
	float	time( int n ) const					{ return m_keys[n].time; };		// Shortcut to key n time.
	value_type&	value( int n )				{ return m_keys[n].value; };	// Shortcut to key n value.
	value_type&	ds( int n )						{ return m_keys[n].ds; };			// Shortcut to key n incoming tangent.
	value_type&	dd( int n )						{ return m_keys[n].dd; };			// Shortcut to key n outgoing tangent.
	
	void	erase( int key )						{ m_keys.erase( m_keys.begin() + key ); m_flags |= MODIFIED; };
	bool	closed()										{ return (ORT() == ORT_LOOP); } // return True if curve closed.

	void	sort_keys()
	{
		std::sort( m_keys.begin(),m_keys.end() );
		if (!m_keys.empty())
		{
			//@FIXME: check if this is correct?
			//m_rangeStart = time(0);
			//m_rangeEnd = time(num_keys()-1);
		}
	}

	void	push_back( const key_type &k )
	{ 
		m_keys.push_back( k ); m_flags |= MODIFIED;
		//if (k.time < m_rangeStart) m_rangeStart = k.time;
		//if (k.time > m_rangeEnd) m_rangeEnd = k.time;
	};

	// Precompute spline tangents.
	// Must be ovveriden by curved classes.
	virtual void comp_deriv() = 0;

	void	interpolate( float time,value_type& val );

	size_t sizeofThis()const
	{
		return sizeof(*this) + sizeof(key_type)*m_keys.size();
	}
protected:
	// Must be ovveriden by curve classes.
	virtual	void interp_keys( int key1,int key2,float u,value_type& val ) = 0;
	int			seek_key( float time );	// Return key before or equal to this time.

	int				m_flags;
	int				m_ORT;				// Out-Of-Range type.
	std::vector<key_type>	m_keys;	// List of keys.
	int				m_curr;				// Current key in track.
	
	float m_rangeStart;
	float m_rangeEnd;
};

template <class T,class Basis>
inline	TSpline<T,Basis>::TSpline()	{
	m_flags = MODIFIED;
	m_ORT = 0;
	m_curr = 0;
	m_rangeStart = 0;
	m_rangeEnd = 0;
}

template <class T,class Basis>
inline	int	TSpline<T,Basis>::seek_key( float t )	{
	if ((m_curr == num_keys()) || (time(m_curr) > t))	{
		// Search from begining.
		m_curr = 0;
	}
	if (m_curr < num_keys())	{
		int last = num_keys() - 1;
		while ((m_curr != last)&&(time(m_curr+1) <= t)) ++m_curr;
	}
	return m_curr;
}

template <class T,class Basis>
inline	void	TSpline<T,Basis>::interpolate( float tm,value_type& val )	{	
	if (empty()) return;

	float t = tm;
	int last = num_keys() - 1;

	if (m_flags&MODIFIED)
		sort_keys();

	if (m_flags&MODIFIED)	comp_deriv();

	if (t < time(0))	{	// Before first key.
		val = value(0);
		return;
	}

	if (isORT(ORT_CYCLE) || isORT(ORT_LOOP))
	{
		// Warp time.
		float endtime = time(last);
		//t = t - floor(t/endtime)*endtime;
		t = fast_fmod( t,endtime );
	}
	int curr = seek_key( t );
	if (curr < last)
	{
		t = (t - time(curr))/(time(curr+1) - time(curr));
		if (t >= 0)	{
			// Call actual interpolation function.
			interp_keys( curr,curr+1,t,val );
		}
		else
			val = value(curr);
	}	else	{
		val = value(last);
	}
}

/*
// Save curve to archive.
template <class T,class Basis>
Archive&	operator << ( Archive& ar,TSpline<T,Basis> &curve )	{
	TSpline<T,Basis>::value_type val;
	curve.interpolate( 0,val );	// Calc derivs if not calced yet. (for quat)
	ar << curve.flag( 0xFFFFFFFF );
	ar << curve.ORT();	// Save Out-Of-Range type.
	ar << curve.num_keys();	// Save num keys.
	for (int i = 0; i < curve.num_keys(); ++i)	{
		ar << curve.key(i);	// Save keys.
	}
	return ar;
}

// Load curve from archive.
template <class T,class Basis>
Archive&	operator >> ( Archive& ar,TSpline<T,Basis> &curve )	{
	int n = 0;
	ar >> n;	// Load num keys.
	curve.flag_set( n );
	ar >> n;	// Load Out-Of-Range type.
	curve.ORT( n );
	ar >> n;	// Load num keys.
	while (n-- > 0)	{ 
		TSpline<T,Basis>::key_type k;
		ar >> k;	// Load key.
		curve.push_back( k );
	}
	return ar;
}
*/

/****************************************************************************
**                        TCBSpline class implementation									   **
****************************************************************************/
template <class T,class Key = TCBSplineKey<T> >
class	TCBSpline : public TSpline< Key,HermitBasis >
{
public:
	virtual void comp_deriv();

protected:
	virtual	void interp_keys( int key1,int key2,float u,T& val );
	float calc_ease( float t,float a,float b );

	static Vec3 Concatenate(const Vec3& left, const Vec3& right)
	{
		return left + right;
	}
	static CryQuat Concatenate(const CryQuat& left, const CryQuat& right)
	{
		return left * right;
	}

	static Vec3 Subtract (const Vec3& left, const Vec3& right)
	{
		return left - right;
	}

	static CryQuat Subtract(const CryQuat& left, const CryQuat& right)
	{
		return left / right;
	}

private:
	void compMiddleDeriv( int curr );
	void compFirstDeriv();
	void compLastDeriv();
	void comp2KeyDeriv();
};

template	<class T,class Key>
inline	void TCBSpline<T,Key>::compMiddleDeriv( int curr )
{
	float	dsA,dsB,ddA,ddB;
	float	A,B,cont1,cont2;
	int last = this->num_keys() - 1; 

	// dsAdjust,ddAdjust apply speed correction when continuity is 0.
	// Middle key.
	if (curr == 0)	{
		// First key.
		float dts = (this->GetRangeEnd() - this->time(last)) + (this->time(0) - this->GetRangeStart());
		float dt = 2.0f / (dts + this->time(1) - this->time(0));
		dsA = dt * dts;
		ddA = dt * (this->time(1) - this->time(0));
	}	else	{
		if (curr == last)
		{
			// Last key.
			float dts = (this->GetRangeEnd() - this->time(last)) + (this->time(0) - this->GetRangeStart());
			float dt = 2.0f / (dts + this->time(last) - this->time(last-1));
			dsA = dt * dts;
			ddA = dt * (this->time(last) - this->time(last-1));
		}	else	{
			// Middle key.
			float dt = 2.0f/(this->time(curr+1) - this->time(curr-1));
			dsA = dt * (this->time(curr) - this->time(curr-1));
			ddA = dt * (this->time(curr+1) - this->time(curr));
		}
	}
	typename TSpline<Key,HermitBasis >::key_type &k = this->key(curr);

	float c = (float)fabs(k.cont);
	float sa = dsA + c*(1.0f - dsA);
	float da = ddA + c*(1.0f - ddA);

	A = 0.5f * (1.0f - k.tens) * (1.0f + k.bias);
	B = 0.5f * (1.0f - k.tens) * (1.0f - k.bias);
	cont1 = (1.0f - k.cont);
	cont2 = (1.0f + k.cont);
	//dsA = dsA * A * cont1;
	//dsB = dsA * B * cont2;
	//ddA = ddA * A * cont2;
	//ddB = ddA * B * cont1;
	dsA = sa * A * cont1;
	dsB = sa * B * cont2;
	ddA = da * A * cont2;
	ddB = da * B * cont1;

	T qp,qn;
	if (curr > 0) qp = this->value(curr-1);	else qp = this->value(last);
	if (curr < last) qn = this->value(curr+1); else qn = this->value(0);
	k.ds = Concatenate(dsA*Subtract(k.value, qp) , dsB*Subtract(qn,k.value));
	k.dd = Concatenate(ddA*Subtract(k.value, qp) , ddB*Subtract(qn,k.value));
}

template	<class T,class Key>
inline	void TCBSpline<T,Key>::compFirstDeriv()	
{
	typename TSpline<Key,HermitBasis >::key_type &k = this->key(0);
	Zero(k.ds);
	k.dd = 0.5f*
		(1.0f - k.tens)*( 3.0f*
		Subtract(Subtract(this->value(1),k.value),this->ds(1)));
}

template	<class T,class Key>
inline	void TCBSpline<T,Key>::compLastDeriv()	{
	int last = this->num_keys() - 1;
	typename TSpline<Key,HermitBasis >::key_type &k = this->key(last);
	k.ds = -0.5f*(1.0f - 
		k.tens)*( 3.0f*
		Concatenate(Subtract(this->value(last-1),k.value) ,	this->dd(last-1) ) );
	Zero(k.dd);
}

template	<class T,class Key>
inline	void TCBSpline<T,Key>::comp2KeyDeriv()	{
	typename TSpline<Key,HermitBasis >::key_type &k1 = this->key(0);
	typename TSpline<Key,HermitBasis >::key_type &k2 = this->key(1);

	typename TSpline<Key,HermitBasis >::value_type val = Subtract(this->value(1),this->value(0));
	
	Zero(k1.ds);
	k1.dd = (1.0f - k1.tens)*val;
	k2.ds = (1.0f - k2.tens)*val;
	Zero(k2.dd);
}

template	<class T,class Key>
inline	void	TCBSpline<T,Key>::comp_deriv()
{
	if (this->num_keys() > 1)	{
		if ((this->num_keys() == 2) && !this->closed())	{
			comp2KeyDeriv();
			return;
		}
		if (this->closed()) {
			for (int i = 0; i < this->num_keys(); ++i)	{
				compMiddleDeriv( i );
			}
		}	else	{
			for (int i = 1; i < (this->num_keys()-1); ++i)
			{
				compMiddleDeriv( i );
			}
			compFirstDeriv();
			compLastDeriv();
		}
	}
	this->m_curr = 0;
	this->m_flags &= ~TSpline<Key,HermitBasis >::MODIFIED;	// clear MODIFIED flag.
}

template	<class T,class Key>
inline	float	TCBSpline<T,Key>::calc_ease( float t,float a,float b )	{
	float k;
	float s = a+b;

	if (t == 0.0f || t == 1.0f) return t;
	if (s == 0.0f) return t;
	if (s > 1.0f) {
		k = 1.0f/s;
		a *= k;
		b *= k;
	}
	k = 1.0f/(2.0f-a-b);
	if (t < a)	{
		return ((k/a)*t*t);
	}	else	{
		if (t < 1.0f-b)	{
			return (k*(2.0f*t - a));
		}	else {
			t = 1.0f - t;
			return (1.0f - (k/b)*t*t);
		}
	}
}

template	<class T,class Key>
inline	void	TCBSpline<T,Key>::interp_keys( int from,int to,float u,T& val )
{
	u = calc_ease( u,this->key(from).easefrom,this->key(to).easeto );
	typename TSpline<Key,HermitBasis >::basis_type basis( u );
	val = Concatenate(
						Concatenate(
												Concatenate(
																		(basis[0]*this->value(from)),(basis[1]*this->value(to))
																		), 
												(basis[2]*this->dd(from))
									), 
										(basis[3]*this->ds(to))
									);
}

/****************************************************************************
**                        TCBQuatSpline class implementation							 **
****************************************************************************/
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class	TCBQuatSpline : public TCBSpline<CryQuat>
{
public:
	//void interpolate( float time,value_type& val );
	void comp_deriv();

protected:
	void interp_keys( int key1,int key2,float u,value_type& val );

private:
	void compKeyDeriv( int curr );

	// Loacal function to add quaternions.
	CryQuat AddQuat( const CryQuat &q1,const CryQuat &q2 )
	{
		return CryQuat( q1.w+q2.w, q1.v.x+q2.v.x, q1.v.y+q2.v.y, q1.v.z+q2.v.z );
	}
};

inline	void	TCBQuatSpline::interp_keys( int from,int to,float u,value_type& val )
{
	u = calc_ease( u,key(from).easefrom,key(to).easeto );
	basis_type basis( u );
	//val = SquadRev( angle(to),axis(to), value(from), dd(from), ds(to), value(to), u );
	val = Squad( value(from), dd(from), ds(to), value(to), u );
	val = GetNormalized(val);	// Normalize quaternion.
}


inline	void TCBQuatSpline::comp_deriv()
{
	if (num_keys() > 1)
	{
		for (int i = 0; i < num_keys(); ++i)	{
			compKeyDeriv( i );
		}
	}
	m_curr = 0;
	m_flags &= ~MODIFIED;	// clear MODIFIED flag.
}

#define	M_2PI		(2.0f*3.14159f - 0.00001f)
inline	void TCBQuatSpline::compKeyDeriv( int curr )
{
	CryQuat  qp,qm;	
	float fp,fn;
	int last = num_keys() - 1;
	
	if (curr > 0 || closed())
	{
		int prev = (curr != 0) ? curr - 1 : last;
		qm = value(prev);
		if ( (qm|(value(curr))) < 0.0f) qm = -qm;
		qm = LnDif( qm,value(curr) );
	}

	if (curr < last || closed())
	{
		int next = (curr != last) ? curr + 1 : 0;
		CryQuat qnext = value(next);
		if ( (qnext|(value(curr))) < 0.0f) qnext = -qnext;
		qp = value(curr);
		qp = LnDif( qp,qnext );
	}

	if (curr == 0 && !closed())   qm = qp;
	if (curr == last && !closed()) qp = qm;

	key_type &k = key(curr);
	float c = (float)fabs(k.cont);

	fp = fn = 1.0f;
	if ((curr > 0 && curr < last) || closed())
	{
		if (curr == 0)	{
			// First key.
			float dts = (this->GetRangeEnd() - this->time(last)) + (this->time(0) - this->GetRangeStart());
			float dt = 2.0f / (dts + this->time(1) - this->time(0));
			fp = dt * dts;
			fn = dt * (this->time(1) - this->time(0));
		}	else	{
			if (curr == last)	{
				// Last key.
				float dts = (this->GetRangeEnd() - this->time(last)) + (this->time(0) - this->GetRangeStart());
				float dt = 2.0f / (dts + this->time(last) - this->time(last-1));
				fp = dt * dts;
				fn = dt * (this->time(last) - this->time(last-1));
			}	else	{
				// Middle key.
				float dt = 2.0f/(this->time(curr+1) - this->time(curr-1));
				fp = dt * (this->time(curr) - this->time(curr-1));
				fn = dt * (this->time(curr+1) - this->time(curr));
			}
		}
		fp += c - c*fp;
		fn += c - c*fn;
	}

	float tm,cm,cp,bm,bp,tmcm,tmcp,ksm,ksp,kdm,kdp;

	cm = 1.0f - k.cont;
	tm = 0.5f*(1.0f - k.tens);
	cp = 2.0f - cm;
	bm = 1.0f - k.bias;
	bp = 2.0f - bm;
	tmcm = tm * cm;
	tmcp = tm * cp;
	ksm  = 1.0f - tmcm * bp * fp;
	ksp  = -tmcp * bm * fp;
	kdm  = tmcp * bp * fn;
	kdp  = tmcm * bm * fn - 1.0f;

	CryQuat qa = 0.5f * AddQuat(kdm*qm,kdp*qp);
	CryQuat qb = 0.5f * AddQuat(ksm*qm,ksp*qp);
	qa = exp(qa);
	qb = exp(qb);
	
	// ds = qb, dd = qa.
	k.ds = value(curr) * qb;
	k.dd = value(curr) * qa;
}

/****************************************************************************
**                    TCBAngleAxisSpline class implementation							 **
****************************************************************************/
///////////////////////////////////////////////////////////////////////////////
//
// TCBAngleAxisSpline takes as input relative Angle-Axis values.
// Interpolated result is returned as Normalized quaternion.
//
//////////////////////////////////////////////////////////////////////////
struct SAngleAxis
{
	float angle;
	Vec3 axis;
};


class	TCBAngleAxisSpline : public TCBSpline<CryQuat,TCBAngAxisKey>
{
public:
	//void interpolate( float time,value_type& val );
	void comp_deriv();

	// Angle axis used for quaternion.
	float&	angle( int i )	{ return key(i).angle; };
	Vec3& axis( int i )	{ return key(i).axis; };

protected:
	void interp_keys( int key1,int key2,float u,value_type& val );

private:
	virtual	void compKeyDeriv( int curr );

	// Loacal function to add quaternions.
	CryQuat AddQuat( const CryQuat &q1,const CryQuat &q2 )
	{
		return CryQuat( q1.w+q2.w,q1.v.x+q2.v.x,q1.v.y+q2.v.y,q1.v.z+q2.v.z );
	}
};

//////////////////////////////////////////////////////////////////////////
inline	void	TCBAngleAxisSpline::interp_keys( int from,int to,float u,value_type& val )
{
	u = calc_ease( u,key(from).easefrom,key(to).easeto );
	basis_type basis( u );
	val = SquadRev( angle(to),axis(to), value(from), dd(from), ds(to), value(to), u );
	val = GetNormalized(val);	// Normalize quaternion.
}

//////////////////////////////////////////////////////////////////////////
inline	void TCBAngleAxisSpline::comp_deriv()
{
	// Convert from relative angle-axis to absolute quaternion.
	CryQuat q,lastq; 
	lastq.SetIdentity();
	for (int i = 0; i < num_keys(); ++i)
	{
		key_type &k = key(i);
		q.SetRotationAA( angle(i),axis(i) );
		q.Normalize(); // Normalize quaternion
		q = lastq * q;
		lastq = q;
		value(i) = q;
	}

	if (num_keys() > 1)
	{
		for (int i = 0; i < num_keys(); ++i)
		{
			compKeyDeriv( i );
		}
	}
	m_curr = 0;
	m_flags &= ~MODIFIED;	// clear MODIFIED flag.
}

//////////////////////////////////////////////////////////////////////////
inline	void TCBAngleAxisSpline::compKeyDeriv( int curr )
{
	CryQuat  qp,qm;	
	float fp,fn;
	int last = num_keys() - 1;
	
	if (curr > 0 || closed())
	{
		int prev = (curr != 0) ? curr - 1 : last;
		if (angle(curr) > M_2PI) {
			Vec3 a = axis(curr);
			qm = log( CryQuat(0,a.x,a.y,a.z) );
		} else {
			qm = value(prev);
			if ( (qm|(value(curr))) < 0.0f)
				qm = -qm;
			qm = LnDif( qm,value(curr) );
		}
	}

	if (curr < last || closed())
	{
		int next = (curr != last) ? curr + 1 : 0;
		if (angle(next) > M_2PI) {
			Vec3 a = axis(next);
			qp = log(CryQuat(0,a.x,a.y,a.z));
		} else {
			CryQuat qnext = value(next);
			if ((qnext|(value(curr))) < 0.0f)
				qnext = -qnext;
			qp = value(curr);
			qp = LnDif( qp,qnext );
		}
	}

	if (curr == 0 && !closed())
		qm = qp;
	if (curr == last && !closed())
		qp = qm;

	key_type &k = key(curr);
	float c = (float)fabs(k.cont);

	fp = fn = 1.0f;
	if ((curr > 0 && curr < last) || closed())
	{
		if (curr == 0)	{
			// First key.
			float dts = (this->GetRangeEnd() - this->time(last)) + (this->time(0) - this->GetRangeStart());
			float dt = 2.0f / (dts + this->time(1) - this->time(0));
			fp = dt * dts;
			fn = dt * (this->time(1) - this->time(0));
		}	else	{
			if (curr == last)	{
				// Last key.
				float dts = (this->GetRangeEnd() - this->time(last)) + (this->time(0) - this->GetRangeStart());
				float dt = 2.0f / (dts + this->time(last) - this->time(last-1));
				fp = dt * dts;
				fn = dt * (this->time(last) - this->time(last-1));
			}	else	{
				// Middle key.
				float dt = 2.0f/(this->time(curr+1) - this->time(curr-1));
				fp = dt * (this->time(curr) - this->time(curr-1));
				fn = dt * (this->time(curr+1) - this->time(curr));
			}
		}
		fp += c - c*fp;
		fn += c - c*fn;
	}

	float tm,cm,cp,bm,bp,tmcm,tmcp,ksm,ksp,kdm,kdp;

	cm = 1.0f - k.cont;
	tm = 0.5f*(1.0f - k.tens);
	cp = 2.0f - cm;
	bm = 1.0f - k.bias;
	bp = 2.0f - bm;
	tmcm = tm * cm;
	tmcp = tm * cp;
	ksm  = 1.0f - tmcm * bp * fp;
	ksp  = -tmcp * bm * fp;
	kdm  = tmcp * bp * fn;
	kdp  = tmcm * bm * fn - 1.0f;

	CryQuat qa = 0.5f * AddQuat(kdm*qm,kdp*qp);
	CryQuat qb = 0.5f * AddQuat(ksm*qm,ksp*qp);
	qa = exp(qa);
	qb = exp(qb);
	
	// ds = qb, dd = qa.
	k.ds = value(curr) * qb;
	k.dd = value(curr) * qa;
}

#endif // __tcbspline_h__