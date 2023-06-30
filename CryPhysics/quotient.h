//////////////////////////////////////////////////////////////////////
//
//	Quotient header
//	
//	File: quotient.h
//	Description : quotient template class declaration and inlined implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef quotient_h
#define quotient_h
#pragma once

// warning: all comparisons assume quotent's y>=0, use fixsign() to ensure this is the case

template<class ftype> class quotient_tpl {
public:
	quotient_tpl() {}
	quotient_tpl(type_min) { x=-1;y=0; }
	quotient_tpl(type_max) { x=1;y=0; }
	explicit quotient_tpl(ftype _x,ftype _y=1) { x=_x;y=_y; }
	quotient_tpl(const quotient_tpl<ftype>& src) { x=src.x;y=src.y; }
	template<class ftype1> quotient_tpl(const quotient_tpl<ftype1>& src) { x=src.x;y=src.y; }
	template<class ftype1,class ftype2> quotient_tpl& set(ftype1 nx,ftype2 ny) { (*this=nx)/=ny; return *this; }

	quotient_tpl& operator=(const quotient_tpl<ftype>& src) { x=src.x;y=src.y; return *this; }
	template<class ftype1> quotient_tpl& operator=(const quotient_tpl<ftype1>& src) { x=src.x;y=src.y; return *this; }
	quotient_tpl& operator=(ftype src) { x=src;y=1; return *this; }

	quotient_tpl& fixsign() { int sgny=::sgnnz(y); x*=sgny; y*=sgny; return *this; }
	ftype val() { return y!=0 ? x/y : 0; }

	quotient_tpl operator-() const { return quotient_tpl(-x,y); }

	quotient_tpl operator*(ftype op) const { return quotient_tpl(x*op,y); }
	quotient_tpl operator/(ftype op) const { return quotient_tpl(x,y*op); }
	quotient_tpl operator+(ftype op) const { return quotient_tpl(x+y*op,y); }
	quotient_tpl operator-(ftype op) const { return quotient_tpl(x-y*op,y); }

	quotient_tpl& operator*=(ftype op) { x*=op; return *this; }
	quotient_tpl& operator/=(ftype op) { y*=op; return *this; }
	quotient_tpl& operator+=(ftype op) { x+=op*y; return *this; }
	quotient_tpl& operator-=(ftype op) { x-=op*y; return *this; }

	bool operator==(ftype op) const { return x==op*y; }
	bool operator!=(ftype op) const { return x!=op*y; }
	bool operator<(ftype op) const { return x-op*y<0; }
	bool operator>(ftype op) const { return x-op*y>0; }
	bool operator<=(ftype op) const { return x-op*y<=0; }
	bool operator>=(ftype op) const { return x-op*y>=0; }

	int sgn() { return ::sgn(x); }
	int sgnnz() { return ::sgnnz(x); }
	int isneg() { return ::isneg(x); }
	int isnonneg() { return ::isnonneg(x); }
	int isin01() { return ::isneg(fabs_tpl(x*2-y)-fabs_tpl(y)); }

	ftype x,y;
};

template<> inline quotient_tpl<float>::quotient_tpl(type_min) { x=-1;y=1E-15f; }
template<> inline quotient_tpl<float>::quotient_tpl(type_max) { x=1;y=1E-15f; }
template<> inline quotient_tpl<double>::quotient_tpl(type_min) { x=-1;y=1E-50f; }
template<> inline quotient_tpl<double>::quotient_tpl(type_max) { x=1;y=1E-50f; }

template<class ftype1,class ftype2>
quotient_tpl<ftype1> operator*(const quotient_tpl<ftype1> &op1,const quotient_tpl<ftype2> &op2) { return quotient_tpl<ftype1>(op1.x*op2.x,op1.y*op2.y); }
template<class ftype1,class ftype2>
quotient_tpl<ftype1> operator/(const quotient_tpl<ftype1> &op1,const quotient_tpl<ftype2> &op2) { return quotient_tpl<ftype1>(op1.x*op2.y,op1.y*op2.x); }
template<class ftype1,class ftype2>
quotient_tpl<ftype1> operator+(const quotient_tpl<ftype1> &op1,const quotient_tpl<ftype2> &op2) 
{ return /*op1.y==op2.y ? quotient_tpl<ftype1>(op1.x+op2.x,op1.y) :*/ quotient_tpl<ftype1>(op1.x*op2.y+op2.x*op1.y, op1.y*op2.y); }
template<class ftype1,class ftype2>
quotient_tpl<ftype1> operator-(const quotient_tpl<ftype1> &op1,const quotient_tpl<ftype2> &op2) 
{ return /*op1.y==op2.y ? quotient_tpl<ftype1>(op1.x-op2.x,op1.y) :*/ quotient_tpl<ftype1>(op1.x*op2.y-op2.x*op1.y, op1.y*op2.y); }

template<class ftype1,class ftype2>
quotient_tpl<ftype1>& operator*=(quotient_tpl<ftype1> &op1,const quotient_tpl<ftype2> &op2) { op1.x*=op2.x; op1.y*=op2.y; return op1; }
template<class ftype1,class ftype2>
quotient_tpl<ftype1>& operator/=(quotient_tpl<ftype1> &op1,const quotient_tpl<ftype2> &op2) { op1.x*=op2.y; op1.y*=op2.x; return op1; }
template<class ftype1,class ftype2>	quotient_tpl<ftype1>& operator+=(quotient_tpl<ftype1> &op1,const quotient_tpl<ftype2> &op2) 
{ /*if (op1.y==op2.y) op1.x+=op2.x; else*/ { op1.x=op1.x*op2.y+op2.x*op1.y; op1.y*=op2.y; } return op1;	}
template<class ftype1,class ftype2>	quotient_tpl<ftype1>& operator-=(quotient_tpl<ftype1> &op1,const quotient_tpl<ftype2> &op2) 
{ /*if (op1.y==op2.y) op1.x-=op2.x; else*/ { op1.x=op1.x*op2.y-op2.x*op1.y; op1.y*=op2.y; } return op1;	}

template<class ftype> quotient_tpl<ftype> operator*(ftype op, const quotient_tpl<ftype> &q) { return quotient_tpl<ftype>(q.x*op,q.y); }
template<class ftype> quotient_tpl<ftype> operator/(ftype op, const quotient_tpl<ftype> &q) { return quotient_tpl<ftype>(q.x,q.y*op); }
template<class ftype> quotient_tpl<ftype> operator+(ftype op, const quotient_tpl<ftype> &q) { return quotient_tpl<ftype>(op*q.y+q.x,q.y); }
template<class ftype> quotient_tpl<ftype> operator-(ftype op, const quotient_tpl<ftype> &q) { return quotient_tpl<ftype>(op*q.y-q.x,q.y); }

template<class ftype> bool operator==(ftype op1, const quotient_tpl<ftype> &op2) { return op1*op2.y==op2.x; }
template<class ftype> bool operator!=(ftype op1, const quotient_tpl<ftype> &op2) { return op1*op2.y!=op2.x; }
template<class ftype> bool operator<(ftype op1, const quotient_tpl<ftype> &op2) { return op1*op2.y-op2.x<0; }
template<class ftype> bool operator>(ftype op1, const quotient_tpl<ftype> &op2) { return op1*op2.y-op2.x>0; }
template<class ftype> bool operator<=(ftype op1, const quotient_tpl<ftype> &op2) { return op1*op2.y-op2.x<=0; }
template<class ftype> bool operator>=(ftype op1, const quotient_tpl<ftype> &op2) { return op1*op2.y-op2.x>=0; }

template<class ftype1,class ftype2> bool operator==(const quotient_tpl<ftype1> &op1, const quotient_tpl<ftype2> &op2) 
{ return op1.x*op2.y==op2.x*op1.y; }
template<class ftype1,class ftype2> bool operator!=(const quotient_tpl<ftype1> &op1, const quotient_tpl<ftype2> &op2) 
{ return op1.x*op2.y!=op2.x*op1.y; }
template<class ftype1,class ftype2> bool operator<(const quotient_tpl<ftype1> &op1, const quotient_tpl<ftype2> &op2) 
{ return op1.x*op2.y-op2.x*op1.y < 0; }
template<class ftype1,class ftype2> bool operator>(const quotient_tpl<ftype1> &op1, const quotient_tpl<ftype2> &op2) 
{ return op1.x*op2.y-op2.x*op1.y > 0; }
template<class ftype1,class ftype2> bool operator<=(const quotient_tpl<ftype1> &op1, const quotient_tpl<ftype2> &op2) 
{ return op1.x*op2.y-op2.x*op1.y <= 0; }
template<class ftype1,class ftype2> bool operator>=(const quotient_tpl<ftype1> &op1, const quotient_tpl<ftype2> &op2) 
{ return op1.x*op2.y-op2.x*op1.y >= 0; }

template<class ftype> int sgn(const quotient_tpl<ftype> &op) { return sgn(op.x); }
template<class ftype> int sgnnz(const quotient_tpl<ftype> &op) { return sgnnz(op.x); }
template<class ftype> int isneg(const quotient_tpl<ftype> &op) { return isneg(op.x); }
template<class ftype> int isnonneg(const quotient_tpl<ftype> &op) { return isnonneg(op.x); }
template<class ftype> int sgn_safe(const quotient_tpl<ftype> &op) { return sgn(op.x)*sgnnz(op.y); }
template<class ftype> int sgnnz_safe(const quotient_tpl<ftype> &op) { return sgnnz(op.x)*sgnnz(op.y); }
template<class ftype> int isneg_safe(const quotient_tpl<ftype> &op) { return isneg(op.x)^isneg(op.y); }
template<class ftype> int isnonneg_safe(const quotient_tpl<ftype> &op) { return isnonneg(op.x)*isnonneg(op.y); }
template<class ftype> quotient_tpl<ftype> fabs_tpl(const quotient_tpl<ftype> op) { return quotient_tpl<ftype>(fabs_tpl(op.x),fabs_tpl(op.y)); }
template<class ftype> quotient_tpl<ftype> max(const quotient_tpl<ftype> &op1,const quotient_tpl<ftype> &op2) { 
	int mask1=isneg(op2.x*op1.y-op1.x*op2.y), mask2=mask1^1;
	return quotient_tpl<ftype>(op1.x*mask1+op2.x*mask2, op1.y*mask1+op2.y*mask2); 
}
template<class ftype> quotient_tpl<ftype> min(const quotient_tpl<ftype> &op1,const quotient_tpl<ftype> &op2) { 
	int mask1=isneg(op1.x*op2.y-op2.x*op1.y), mask2=mask1^1;
	return quotient_tpl<ftype>(op1.x*mask1+op2.x*mask2, op1.y*mask1+op2.y*mask2); 
}

template<class ftype> quotient_tpl<ftype> fake_atan2(ftype y,ftype x) {
	quotient_tpl<ftype> res;
	ftype src[2] = { x,y };
	int ix=isneg(x),iy=isneg(y),iflip=isneg(fabs_tpl(x)-fabs_tpl(y));
	res.x = src[iflip^1]*(1-iflip*2)*sgnnz(src[iflip]); res.y = fabs_tpl(src[iflip]);
	res += (iy*2+(ix^iy)+(iflip^ix^iy))*2;
	return res;
}

typedef quotient_tpl<float> quotientf;
typedef quotient_tpl<real> quotient;
typedef quotient_tpl<int> quotienti;

#endif