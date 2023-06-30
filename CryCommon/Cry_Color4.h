//////////////////////////////////////////////////////////////////////
//
//  Crytek (C) 2001 
//
//  File: CryColor4.h
//  Description: 4D Color template.
//
//  History:
//  - August 12, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYTEK_CRYCOLOR4_H
#define CRYTEK_CRYCOLOR4_H

//////////////////////////////////////////////////////////////////////////////////////////////
#define RGBA8( r,g,b,a ) (  uint32( ##(r)|(g<<8)|(b<<16)|(a<<24)##) )

template <class T> struct color4;

typedef color4<uint8>	Color; // [0, 255]

typedef color4<f32>	color4f; // [0.0, 1.0]
typedef color4<f64>	color4d; // [0.0, 1.0]
typedef color4<uint8>	color4b; // [0, 255]
typedef color4<uint16>	color4w; // [0, 65535]		



//////////////////////////////////////////////////////////////////////////////////////////////
// RGBA Color structure.
//////////////////////////////////////////////////////////////////////////////////////////////
template <class T> struct color4
{

	union {
		struct { T	r,g,b,a;	};
		T v[4];
	};

	inline color4();
	inline color4(const T *p_elts);
	inline color4(const color4 & v);
	inline color4(T _x, T _y = 0, T _z = 0, T _w = 0);

	inline color4( uint32 c ) {	*(uint32*)(&v)=c; } //use this with RGBA8 macro!
	
	inline void set(T _x, T _y = 0, T _z = 0, T _w = 0);
	inline void set(T _x, T _y = 0, T _z = 0);

	inline color4 operator + () const;
	inline color4 operator - () const;
	
	inline color4 & operator += (const color4 & v);
	inline color4 & operator -= (const color4 & v);
	inline color4 & operator *= (const color4 & v);
	inline color4 & operator /= (const color4 & v);
	inline color4 & operator *= (T s);
	inline color4 & operator /= (T s);
	
	inline color4 operator + (const color4 & v) const;
	inline color4 operator - (const color4 & v) const;
	inline color4 operator * (const color4 & v) const;
	inline color4 operator / (const color4 & v) const;
	inline color4 operator * (T s) const;
	inline color4 operator / (T s) const;
	
	inline bool operator == (const color4 & v) const;
	inline bool operator != (const color4 & v) const;

	inline unsigned char pack_rgb332();
	inline unsigned short pack_argb4444();
	inline unsigned short pack_rgb555();
	inline unsigned short pack_rgb565();
	inline unsigned int pack_rgb888();
	inline unsigned int pack_argb8888();

	inline unsigned int pack8() { return pack_rgb332(); }
	inline unsigned int pack12() { return pack_argb4444(); }
	inline unsigned int pack15() { return pack_rgb555(); }
	inline unsigned int pack16() { return pack_rgb565(); }
	inline unsigned int pack24() { return pack_rgb888(); }
	inline unsigned int pack32() { return pack_argb8888(); }

	inline void clamp(T bottom = 0.0f, T top = 1.0f);

	inline void maximum(const color4<T> &ca, const color4<T> &cb);
	inline void minimum(const color4<T> &ca, const color4<T> &cb);
	inline void abs();
	
	inline void adjust_contrast(T c);
	inline void adjust_saturation(T s);

	inline void lerp(const color4<T> &ca, const color4<T> &cb, T s);
	inline void negative(const color4<T> &c);
	inline void grey(const color4<T> &c);
	inline void black_white(const color4<T> &c, T s);
};

//////////////////////////////////////////////////////////////////////////////////////////////
// functions implementation

///////////////////////////////////////////////
template <class T>
inline color4<T>::color4() { }

///////////////////////////////////////////////
template <class T>
inline color4<T>::color4(const T *p_elts)
{
	r = p_elts[0]; g = p_elts[1]; b = p_elts[2]; a = p_elts[3];
}

///////////////////////////////////////////////
template <class T>
inline color4<T>::color4(const color4<T> & v)
{
	r = v.r; g = v.g; b = v.b; a = v.a;
}

///////////////////////////////////////////////
template <class T>
inline color4<T>::color4(T _x, T _y, T _z, T _w)
{
	r = _x; g = _y; b = _z; a = _w;
}

///////////////////////////////////////////////
template <class T>
inline void color4<T>::set(T _x, T _y, T _z, T _w)
{
	r = _x; g = _y; b = _z; a = _w;
}

///////////////////////////////////////////////
template <class T>
inline void color4<T>::set(T _x, T _y, T _z)
{
	r = _x; g = _y; b = _z; a = 1;
}

///////////////////////////////////////////////
template <class T>
inline color4<T> color4<T>::operator + () const
{
	return *this;
}

///////////////////////////////////////////////
template <class T>
inline color4<T> color4<T>::operator - () const
{
	return color4<T>(-r, -g, -b, -a);
}

///////////////////////////////////////////////
template <class T>
inline color4<T> & color4<T>::operator += (const color4<T> & v)
{
	r += v.r; g += v.g; b += v.b; a += v.a;
	return *this;
}

///////////////////////////////////////////////
template <class T>
inline color4<T> & color4<T>::operator -= (const color4<T> & v)
{
	r -= v.r; g -= v.g; b -= v.b; a -= v.a;
	return *this;
}

///////////////////////////////////////////////
template <class T>
inline color4<T> & color4<T>::operator *= (const color4<T> & v)
{
	r *= v.r; g *= v.g; b *= v.b; a *= v.a;
	return *this;
}

///////////////////////////////////////////////
template <class T>
inline color4<T> & color4<T>::operator /= (const color4<T> & v)
{
	r /= v.r; g /= v.g; b /= v.b; a /= v.a;
	return *this;
}

///////////////////////////////////////////////
template <class T>
inline color4<T> & color4<T>::operator *= (T s)
{
	r *= s; g *= s; b *= s; a *= s;
	return *this;
}

///////////////////////////////////////////////
template <class T>
inline color4<T> & color4<T>::operator /= (T s)
{
	s = 1.0f / s;
	r *= s; g *= s; b *= s; a *= s;
	return *this;
}

///////////////////////////////////////////////
template <class T>
inline color4<T> color4<T>::operator + (const color4<T> & v) const
{
	return color4<T>(r + v.r, g + v.g, b + v.b, a + v.a);
}

///////////////////////////////////////////////
template <class T>
inline color4<T> color4<T>::operator - (const color4<T> & v) const
{
	return color4<T>(r - v.r, g - v.g, b - v.b, a - v.a);
}

///////////////////////////////////////////////
template <class T>
inline color4<T> color4<T>::operator * (T s) const
{
	return color4<T>(r * s, g * s, b * s, a * s);
}

///////////////////////////////////////////////
template <class T>
inline color4<T> color4<T>::operator / (T s) const
{
	s = 1.0f / s;
	return color4<T>(r * s, g * s, b * s, a * s);
}

///////////////////////////////////////////////
template <class T>
inline bool color4<T>::operator == (const color4<T> & v) const
{
	return (r == v.r) && (g == v.g) && (b == v.b) && (a == v.a);
}

///////////////////////////////////////////////
template <class T>
inline bool color4<T>::operator != (const color4<T> & v) const
{
	return (r != v.r) || (g != v.g) || (b != v.b) || (a != v.a);
}

///////////////////////////////////////////////
template <class T>
inline color4<T> operator * (T s, const color4<T> & v)
{
	return color4<T>(v.r * s, v.g * s, v.b * s, v.a * s);
}

///////////////////////////////////////////////
template <class T>
inline unsigned char color4<T>::pack_rgb332()
{
	unsigned char cr;
	unsigned char cg;
	unsigned char cb;
	if(sizeof(r) == 1) // char and unsigned char
	{
		cr = r;
		cg = g;
		cb = b;
	}
	else if(sizeof(r) == 2) // short and unsigned short
	{
		cr = (unsigned short)(r)>>8;
		cg = (unsigned short)(g)>>8;
		cb = (unsigned short)(b)>>8;
	}
	else // float or double
	{
		cr = (unsigned char)(r * 255.0f);
		cg = (unsigned char)(g * 255.0f);
		cb = (unsigned char)(b * 255.0f);
	}
	return ((cr >> 5) << 5) | ((cg >> 5) << 2) | (cb >> 5);
}

///////////////////////////////////////////////
template <class T>
inline unsigned short color4<T>::pack_argb4444()
{
	unsigned char cr;
	unsigned char cg;
	unsigned char cb;
	unsigned char ca;
	if(sizeof(r) == 1) // char and unsigned char
	{
		cr = r;
		cg = g;
		cb = b;
		ca = a;
	}
	else if(sizeof(r) == 2) // short and unsigned short
	{
		cr = (unsigned short)(r)>>8;
		cg = (unsigned short)(g)>>8;
		cb = (unsigned short)(b)>>8;
		ca = (unsigned short)(a)>>8;
	}
	else // float or double
	{
		cr = (unsigned char)(r * 255.0f);
		cg = (unsigned char)(g * 255.0f);
		cb = (unsigned char)(b * 255.0f);
		ca = (unsigned char)(a * 255.0f);
	}
	return ((ca >> 4) << 12) | ((cr >> 4) << 8) | ((cg >> 4) << 4) | (cb >> 4);
}

///////////////////////////////////////////////
template <class T>
inline unsigned short color4<T>::pack_rgb555()
{
	unsigned char cr;
	unsigned char cg;
	unsigned char cb;
	if(sizeof(r) == 1) // char and unsigned char
	{
		cr = r;
		cg = g;
		cb = b;
	}
	else if(sizeof(r) == 2) // short and unsigned short
	{
		cr = (unsigned short)(r)>>8;
		cg = (unsigned short)(g)>>8;
		cb = (unsigned short)(b)>>8;
	}
	else // float or double
	{
		cr = (unsigned char)(r * 255.0f);
		cg = (unsigned char)(g * 255.0f);
		cb = (unsigned char)(b * 255.0f);
	}
	return ((cr >> 3) << 10) | ((cg >> 3) << 5) | (cb >> 3);
}

///////////////////////////////////////////////
template <class T>
inline unsigned short color4<T>::pack_rgb565()
{
	unsigned char cr;
	unsigned char cg;
	unsigned char cb;
	if(sizeof(r) == 1) // char and unsigned char
	{
		cr = r;
		cg = g;
		cb = b;
	}
	else if(sizeof(r) == 2) // short and unsigned short
	{
		cr = (unsigned short)(r)>>8;
		cg = (unsigned short)(g)>>8;
		cb = (unsigned short)(b)>>8;
	}
	else // float or double
	{
		cr = (unsigned char)(r * 255.0f);
		cg = (unsigned char)(g * 255.0f);
		cb = (unsigned char)(b * 255.0f);
	}
	return ((cr >> 3) << 11) |	((cg >> 2) << 5) | (cb >> 3);
}

///////////////////////////////////////////////
template <class T>
inline unsigned int color4<T>::pack_rgb888()
{
	unsigned char cr;
	unsigned char cg;
	unsigned char cb;
	if(sizeof(r) == 1) // char and unsigned char
	{
		cr = r;
		cg = g;
		cb = b;
	}
	else if(sizeof(r) == 2) // short and unsigned short
	{
		cr = (unsigned short)(r)>>8;
		cg = (unsigned short)(g)>>8;
		cb = (unsigned short)(b)>>8;
	}
	else // float or double
	{
		cr = (unsigned char)(r * 255.0f);
		cg = (unsigned char)(g * 255.0f);
		cb = (unsigned char)(b * 255.0f);
	}
	return (cr << 16) | (cg << 8) | cb;
}

///////////////////////////////////////////////
template <class T>
inline unsigned int color4<T>::pack_argb8888()
{
	unsigned char cr;
	unsigned char cg;
	unsigned char cb;
	unsigned char ca;
	if(sizeof(r) == 1) // char and unsigned char
	{
		cr = r;
		cg = g;
		cb = b;
		ca = a;
	}
	else if(sizeof(r) == 2) // short and unsigned short
	{
		cr = (unsigned short)(r)>>8;
		cg = (unsigned short)(g)>>8;
		cb = (unsigned short)(b)>>8;
		ca = (unsigned short)(a)>>8;
	}
	else // float or double
	{
		cr = (unsigned char)(r * 255.0f);
		cg = (unsigned char)(g * 255.0f);
		cb = (unsigned char)(b * 255.0f);
		ca = (unsigned char)(a * 255.0f);
	}
	return (ca << 24) | (cr << 16) | (cg << 8) | cb;
}

///////////////////////////////////////////////
template <class T>
inline void color4<T>::clamp(T bottom, T top)
{
	     if(r < bottom)	r = bottom;
	else if(r > top)	r = top;
	     if(g < bottom)	g = bottom;
	else if(g > top)	g = top;
	     if(b < bottom)	b = bottom;
	else if(b > top)	b = top;
	     if(a < bottom)	a = bottom;
	else if(a > top)	a = top;
}

///////////////////////////////////////////////
template <class T>
void color4<T>::maximum(const color4<T> &ca, const color4<T> &cb)
{
	r = (ca.r > cb.r) ? ca.r : cb.r;
	g = (ca.g > cb.g) ? ca.g : cb.g;
	b = (ca.b > cb.b) ? ca.b : cb.b;
	a = (ca.a > cb.a) ? ca.a : cb.a;
}

///////////////////////////////////////////////
template <class T>
void color4<T>::minimum(const color4<T> &ca, const color4<T> &cb)
{
	r = (ca.r < cb.r) ? ca.r : cb.r;
	g = (ca.g < cb.g) ? ca.g : cb.g;
	b = (ca.b < cb.b) ? ca.b : cb.b;
	a = (ca.a < cb.a) ? ca.a : cb.a;
}

///////////////////////////////////////////////
template <class T>
void color4<T>::abs()
{
	r = (r < 0) ? -r : r;
	g = (g < 0) ? -g : g;
	b = (b < 0) ? -b : b;
	a = (a < 0) ? -a : a;
}

///////////////////////////////////////////////
template <class T>
void color4<T>::adjust_contrast(T c)
{
	r = 0.5f + c * (r - 0.5f);
	g = 0.5f + c * (g - 0.5f);
	b = 0.5f + c * (b - 0.5f);
	a = 0.5f + c * (a - 0.5f);
}

///////////////////////////////////////////////
template <class T>
void color4<T>::adjust_saturation(T s)
{
	// Approximate values for each component's contribution to luminance.
    // Based upon the NTSC standard described in ITU-R Recommendation BT.709.
    T grey = r * 0.2125f + g * 0.7154f + b * 0.0721f;    
    r = grey + s * (r - grey);
	g = grey + s * (g - grey);
	b = grey + s * (b - grey);
	a = grey + s * (a - grey);
}

///////////////////////////////////////////////
template <class T>
void color4<T>::lerp(const color4<T> &ca, const color4<T> &cb, T s)
{
	r = ca.r + s * (cb.r - ca.r);
	g = ca.g + s * (cb.g - ca.g);
	b = ca.b + s * (cb.b - ca.b);
	a = ca.a + s * (cb.a - ca.a);
}

///////////////////////////////////////////////
template <class T>
void color4<T>::negative(const color4<T> &c)
{
	r = T(1.0f) - r;
	g = T(1.0f) - g;
	b = T(1.0f) - b;
	a = T(1.0f) - a;
}

///////////////////////////////////////////////
template <class T>
void color4<T>::grey(const color4<T> &c)
{
	T m = (r + g + b) / T(3);

	r = m;
	g = m;
	b = m;
	a = a;
}
/*
///////////////////////////////////////////////
template <class T>
void color4<T>::black_white(const color4<T> &c, T s)
{
	T add = r + g + b;
	if(add <= s)
	{
		r = T(0.0f);
		g = T(0.0f);
		b = T(0.0f);
	}
	else
	{
		r = T(1.0f);
		g = T(1.0f);
		b = T(1.0f);
	}
	a = pC->a;
}
*/
#endif // CRYTEK_CRYCOLOR4_H
