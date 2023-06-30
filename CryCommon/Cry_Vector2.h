//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//	
//	File:Cry_Vector2.h
//	Description: Common matrix class
//
//	History:
//	-Feb 27,2003: Created by Ivo Herzeg
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYTEK_CRYVECTOR2_H
#define CRYTEK_CRYVECTOR2_H

#include "platform.h"
#include "Cry_Vector3.h"

template <class frype> struct Vec2_tpl;

typedef Vec2_tpl<float> Vec2;
typedef Vec2_tpl<real> Vec2_f64;


typedef Vec2_tpl<float> vector2f;
#if defined(LINUX64)
typedef Vec2_tpl<int>	 vector2l;
#else
typedef Vec2_tpl<long>	 vector2l;
#endif
typedef Vec2_tpl<float> vector2df;
typedef Vec2_tpl<real> vector2d;
typedef Vec2_tpl<int> vector2di;
typedef Vec2_tpl<unsigned int> vector2dui;





template<class F> struct Vec2_tpl {

F x,y;
	
Vec2_tpl() {}
Vec2_tpl(F _x,F _y) { x=_x;y=_y; }
Vec2_tpl(const Vec2_tpl<F> &src) { x=src.x;y=src.y; }
Vec2_tpl& set(F nx,F ny) { x=nx;y=ny; return *this; }


template<class F1> Vec2_tpl(const Vec2_tpl<F1> &src) { x=src.x;y=src.y; }
template<class F1> explicit Vec2_tpl(const Vec3_tpl<F1> &src) { x=src.x;y=src.y; }
template<class F1> explicit Vec2_tpl(const F1 *psrc) { x=psrc[0]; y=psrc[1]; }
Vec2_tpl& operator=(const Vec2_tpl<F>& src) { x=src.x;y=src.y; return *this; }
template<class F1> Vec2_tpl& operator=(const Vec2_tpl<F1>& src) { x=src.x;y=src.y; return *this; }
template<class F1> Vec2_tpl& operator=(const Vec3_tpl<F1>& src) { x=src.x;y=src.y; return *this; }

int operator!() const { return x==0 && y==0; }
Vec2_tpl& normalize() { F rlen=sqrt_tpl(x*x+y*y); if (rlen>0) { rlen=(F)1.0/rlen; x*=rlen;y*=rlen; } return *this; }
Vec2_tpl normalized() const { 
F rlen=sqrt_tpl(x*x+y*y); if (rlen>0) { rlen=(F)1.0/rlen; return Vec2_tpl<F>(x*rlen,y*rlen); } 
return Vec2_tpl<F>(1,0); 
}
F len() const { return sqrt_tpl(x*x+y*y); }
F len2() const { return x*x+y*y; }
F area() const { return x*y; }

F& operator[](int idx) { return *((F*)&x+idx); }
F operator[](int idx) const { return *((F*)&x+idx); }
operator F*() { return &x; }
Vec2_tpl& flip() { x=-x;y=-y; return *this; }
Vec2_tpl& zero() { x=y=0; return *this; }
Vec2_tpl rot90ccw() { return Vec2_tpl(-y,x); }
Vec2_tpl rot90cw() { return Vec2_tpl(y,-x); }

#ifdef quotient_h
quotient_tpl<F> fake_atan2() const {
quotient_tpl<F> res;
int quad = -(signnz(x*x-y*y)-1>>1); // hope the optimizer will remove complement shifts and adds
if (quad) { res.x=-y; res.y=x; } 
else { res.x=x; res.y=y; } 
int sgny = signnz(res.y);	quad |= 1-sgny; //(res.y<0)<<1;
res.x *= sgny; res.y *= sgny;
res += 1+(quad<<1);
return res;
}
#endif
F atan2() const { return atan2_tpl(y,x); }

Vec2_tpl operator-() const { return Vec2_tpl(-x,-y); }

Vec2_tpl operator*(F k) const { return Vec2_tpl(x*k,y*k); }
Vec2_tpl& operator*=(F k) { x*=k;y*=k; return *this; }
Vec2_tpl operator/(F k) const { return *this*((F)1.0/k); }
Vec2_tpl& operator/=(F k) { return *this*=((F)1.0/k); }

};

template<class F1,class F2>
F1 operator*(const Vec2_tpl<F1> &op1, const Vec2_tpl<F2>& op2) { return op1.x*op2.x+op1.y*op2.y; } // dot product

template<class F1,class F2>
F1 operator^(const Vec2_tpl<F1> &op1, const Vec2_tpl<F2>& op2) { return op1.x*op2.y-op1.y*op2.x; } // cross product

template<class F1,class F2>
Vec2_tpl<F1> operator+(const Vec2_tpl<F1> &op1, const Vec2_tpl<F2> &op2) { 
return Vec2_tpl<F1>(op1.x+op2.x,op1.y+op2.y); 
}
template<class F1,class F2>
Vec2_tpl<F1> operator-(const Vec2_tpl<F1> &op1, const Vec2_tpl<F2> &op2) { 
return Vec2_tpl<F1>(op1.x-op2.x,op1.y-op2.y); 
}

template<class F1,class F2>
Vec2_tpl<F1>& operator+=(Vec2_tpl<F1> &op1, const Vec2_tpl<F2> &op2) { 
op1.x+=op2.x;op1.y+=op2.y; return op1; 
}
template<class F1,class F2>
Vec2_tpl<F1>& operator-=(Vec2_tpl<F1> &op1, const Vec2_tpl<F2> &op2) { 
op1.x-=op2.x;op1.y-=op2.y; return op1; 
}





#endif // CRYTEK_CRYVECTOR2_H
