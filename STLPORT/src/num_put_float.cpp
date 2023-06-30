/*
 * Copyright (c) 1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */

# include "stlport_prefix.h"

# ifdef __DECCXX
#define NDIG 400
# else
#define NDIG 82
# endif

# ifdef _STLP_NO_LONG_DOUBLE
#  define MAXECVT 17
#  define MAXFCVT 18
typedef double max_double_type;
# else
#  define MAXECVT 35
#  define MAXFCVT 36
typedef long double max_double_type;
# endif

#define MAXFSIG MAXECVT
#define MAXESIZ 5

#define todigit(x) ((x)+'0')

# include <stl/_config.h>

# ifdef _STLP_UNIX

# if defined (__sun) 
#  include <floatingpoint.h>
# endif

//# if !(defined(_STLP_USE_GLIBC) || defined(__FreeBSD__) || defined(__NetBSD__) || defined (_AIX) || defined(__MVS__) || defined (__OS400__) || defined (__QNXNTO__) || defined (__APPLE__) || defined (__DJGPP))
# if defined (__sun) || defined (__digital__) || defined (__sgi) || defined (_STLP_SCO_OPENSERVER) || defined (__NCR_SVR)
// DEC, SGI & Solaris need this
#  include <values.h>
#  include <nan.h>
# endif

# if defined (__QNXNTO__) || ( defined(__GNUC__) && defined(__APPLE__) )
#  define USE_SPRINTF_INSTEAD
# endif

#  if defined( _AIX ) // JFA 3-Aug-2000
#    include <math.h>
#    include <float.h>
#  endif

# endif

# include <cstdlib>

//#if defined(_CRAY)
//# include <stdlib.h>
//#endif

#if defined (_MSC_VER) || defined (__MINGW32__) || defined (__BORLANDC__) || defined (__DJGPP)  || defined (_STLP_SCO_OPENSERVER) || defined (__NCR_SVR)
# include <float.h>
#endif

#if defined(__MRC__) || defined(__SC__)	|| defined(_CRAY)	//*TY 02/24/2000 - added support for MPW
# include <fp.h>
#endif

#include <cmath>

#if defined( __MSL__ )
# include <cstdlib>	// for atoi
# include <cstdio>	// for snprintf
# include <algorithm>
# include <cassert>
#endif

#if defined (__ISCPP__)
# include <cfloat>
#endif


# include "num_put.h"
# include <algorithm>


#if defined(__hpux) && !defined(_INCLUDE_HPUX_SOURCE)
     extern "C" double erf(double);
     extern "C" double erfc(double);
     extern "C" double gamma(double);                             /* obsolescent */
     extern "C" double hypot(double, double);
     extern "C" int    isnan(double);
     extern "C" double j0(double);
     extern "C" double j1(double);
     extern "C" double jn(int, double);
     extern "C" double lgamma(double);
     extern "C" double y0(double);
     extern "C" double y1(double);
     extern "C" double yn(int, double);

#  define HUGE_VALF     _SINFINITY
#  define INFINITY      _SINFINITY
#  define NAN           _SQNAN

#  define isnan(x)              _ISNAN(x)
#  define isinf(x)              _ISINF(x)
#  define signbit(x)            _SIGNBIT(x)
#  define isfinite(x)           _ISFINITE(x)
#  define isnormal(x)           _ISNORMAL(x)
#  define fpclassify(x)         _FPCLASSIFY(x)
#  define isunordered(x,y)      _ISUNORDERED(x,y)
#  define isgreater(x,y)        _ISGREATER(x,y)
#  define isgreaterequal(x,y)   _ISGREATEREQUAL(x,y)
#  define isless(x,y)           _ISLESS(x,y)
#  define islessequal(x,y)      _ISLESSEQUAL(x,y)
#  define islessgreater(x,y)    _ISLESSGREATER(x,y)

#  define FP_NORMAL     0
#  define FP_ZERO       1
#  define FP_INFINITE   2
#  define FP_SUBNORMAL  3
#  define FP_NAN        4

#  define DECIMAL_DIG   17

#  define _IS64(x) (sizeof(x)==sizeof(double))
#  define _IS32(x) (sizeof(x)==sizeof(float))

extern "C" {
     extern double copysign(double, double);
     extern const float _SINFINITY;
     extern const float _SQNAN;
#    ifdef _PA_RISC
#      define _ISNAN(x)          (_IS32(x)?_Isnanf(x):(isnan)(x))
#      define _ISINF(x)          (_IS32(x)?_Isinff(x):_Isinf(x))
#      define _SIGNBIT(x)        (_IS32(x)?_Signbitf(x):_Signbit(x))
#      define _ISFINITE(x)       (_IS32(x)?_Isfinitef(x):_Isfinite(x))
#      define _ISNORMAL(x)       (_IS32(x)?_Isnormalf(x):_Isnormal(x))
#      define _FPCLASSIFY(x)     (_IS32(x)?_Fpclassifyf(x)>>1:_Fpclassify(x)>>1)
#      define _ISUNORDERED(x,y)  (_IS32(x)&&_IS32(y)?_Isunorderedf(x,y):_Isunordered(x,y))
       extern int _Signbit(double);
       extern int _Signbitf(float);
       extern int _Isnanf(float);
       extern int _Isfinite(double);
       extern int _Isfinitef(float);
       extern int _Isinf(double);
       extern int _Isinff(float);
       extern int _Isnormal(double);
       extern int _Isnormalf(float);
       extern int _Isunordered(double, double);
       extern int _Isunorderedf(float, float);
       extern int _Fpclassify(double);
       extern int _Fpclassifyf(float);
#    else
#      include "math_ia64_internal.h"
#      define _FPCLASSIFY(x)     (_IS32(x)?_Fpclassf(x):_Fpclass(x))
       extern int _Fpclass(double);
       extern int _Fpclassf(float);
#    endif
}

#ifndef _INCLUDE_XOPEN_SOURCE_EXTENDED
extern "C" char *fcvt(double, int, int *, int *);
extern "C" char *ecvt(double, int, int *, int *);
#endif
#ifndef _INCLUDE_HPUX_SOURCE
#  ifndef _LONG_DOUBLE
#    define _LONG_DOUBLE
     typedef struct {
       uint32_t word1, word2, word3, word4;
     } long_double;
#  endif /* _LONG_DOUBLE */
extern "C" char *_ldecvt(long_double, int, int *, int *);
extern "C" char *_ldfcvt(long_double, int, int *, int *);

#endif

#endif /* __hpux */

_STLP_BEGIN_NAMESPACE

#if defined (__MWERKS__) || defined(__BEOS__)
#  define USE_SPRINTF_INSTEAD
#endif

# if defined (_AIX)
// Some OS'es only provide non-reentrant primitives, so we have to use additional synchronization here
# ifdef _REENTRANT
static _STLP_STATIC_MUTEX __put_float_mutex _STLP_MUTEX_INITIALIZER;
# define LOCK_CVT _STLP_auto_lock lock(__put_float_mutex);
# define RETURN_CVT(ecvt, x, n, pt, sign, buf) strcpy(buf, ecvt(x, n, pt, sign)); return buf;
# else
# define LOCK_CVT
# define RETURN_CVT(ecvt, x, n, pt, sign, buf) return ecvt(x, n, pt, sign);
# endif
# endif

// Tests for infinity and NaN differ on different OSs.  We encapsulate
// these differences here.

#ifdef USE_SPRINTF_INSTEAD

#elif defined (__hpux) || defined (__DJGPP) || ( defined(_STLP_USE_GLIBC) && ! defined (__MSL__) )
#  if defined (isfinite) 
inline bool _Stl_is_nan_or_inf(double x) { return !isfinite(x); }
#  else
inline bool _Stl_is_nan_or_inf(double x) { return !finite(x); }
#  endif
inline bool _Stl_is_neg_nan(double x)    { return isnan(x) && ( copysign(1., x) < 0 ); }
inline bool _Stl_is_inf(double x)        { return isinf(x); }
// inline bool _Stl_is_neg_inf(double x)    { return isinf(x) < 0; }  
inline bool _Stl_is_neg_inf(double x)    { return isinf(x) && x < 0; }
#elif defined(__unix) && !defined(__FreeBSD__)  && !defined(__NetBSD__) \
      && !defined(__APPLE__)  && !defined(__DJGPP) && !defined(__osf__) \
      && !defined(_CRAY)
inline bool _Stl_is_nan_or_inf(double x) { return IsNANorINF(x); }
inline bool _Stl_is_inf(double x)        { return IsNANorINF(x) && IsINF(x); }
inline bool _Stl_is_neg_inf(double x)    { return (IsINF(x)) && (x < 0.0); }
inline bool _Stl_is_neg_nan(double x)    { return IsNegNAN(x); }
# elif defined (__BORLANDC__) && ( __BORLANDC__ < 0x540 )
inline bool _Stl_is_nan_or_inf(double x) {  return !_finite(x); }
inline bool _Stl_is_inf(double x)        {  return _Stl_is_nan_or_inf(x) && ! _isnan(x);}
inline bool _Stl_is_neg_inf(double x)    {  return _Stl_is_inf(x) && x < 0 ; }
inline bool _Stl_is_neg_nan(double x)    { return _isnan(x) && x < 0 ; } 
#elif defined (_MSC_VER) || defined (__MINGW32__) || defined (__BORLANDC__)
inline bool _Stl_is_nan_or_inf(double x) { return !_finite(x); }
inline bool _Stl_is_inf(double x)        { 
  int fclass = _fpclass(x); 
  return fclass == _FPCLASS_NINF || fclass == _FPCLASS_PINF; 
}
inline bool _Stl_is_neg_inf(double x)    { return _fpclass(x) == _FPCLASS_NINF; }
inline bool _Stl_is_neg_nan(double x)    { return _isnan(x) && _copysign(1., x) < 0 ; } 
#elif defined(__MRC__) || defined(__SC__)		//*TY 02/24/2000 - added support for MPW
bool _Stl_is_nan_or_inf(double x) { return isnan(x) || !isfinite(x); }
bool _Stl_is_inf(double x)        { return !isfinite(x); }
bool _Stl_is_neg_inf(double x)    { return !isfinite(x) && signbit(x); }
bool _Stl_is_neg_nan(double x)    { return isnan(x) && signbit(x); }
#elif /* defined (__FreeBSD__) */ ( defined (__GNUC__) && defined (__APPLE__) )
inline bool _Stl_is_nan_or_inf(double x) { return !finite(x); }
inline bool _Stl_is_inf(double x)        {   return _Stl_is_nan_or_inf(x) && ! isnan(x); }
inline bool _Stl_is_neg_inf(double x)    {   return _Stl_is_inf(x) && x < 0 ; }
inline bool _Stl_is_neg_nan(double x)    { return isnan(x) && copysign(1., x) < 0 ; } 
#elif defined( _AIX ) // JFA 11-Aug-2000
bool _Stl_is_nan_or_inf(double x) { return isnan(x) || !finite(x); }
bool _Stl_is_inf(double x)        { return !finite(x); }
// bool _Stl_is_neg_inf(double x)    { return _class(x) == FP_MINUS_INF; }
bool _Stl_is_neg_inf(double x)    { return _Stl_is_inf(x) && ( copysign(1., x) < 0 );  }
bool _Stl_is_neg_nan(double x)    { return isnan(x) && ( copysign(1., x) < 0 );  }
#elif defined (__ISCPP__)
inline bool _Stl_is_nan_or_inf  (double x) { return _fp_isINF(x) || _fp_isNAN(x); }
inline bool _Stl_is_inf         (double x) { return _fp_isINF(x); }
inline bool _Stl_is_neg_inf     (double x) { return _fp_isINF(x) && x < 0; }
inline bool _Stl_is_neg_nan     (double x) { return _fp_isNAN(x) && x < 0; }
#elif defined(_CRAY)
 #if defined(_CRAYIEEE)
inline bool _Stl_is_nan_or_inf(double x) { return isnan(x) || isinf(x); }
inline bool _Stl_is_inf(double x)        { return isinf(x); }
inline bool _Stl_is_neg_inf(double x)    { return isinf(x) && signbit(x); }
inline bool _Stl_is_neg_nan(double x)    { return isnan(x) && signbit(x); }
 #else
inline bool _Stl_is_nan_or_inf(double x) { return false; }
inline bool _Stl_is_inf(double x)        { return false; }
inline bool _Stl_is_neg_inf(double x)    { return false; }
inline bool _Stl_is_neg_nan(double x)    { return false; }
 #endif
#elif ! defined (USE_SPRINTF_INSTEAD)
# define USE_SPRINTF_INSTEAD
#endif


# ifndef  USE_SPRINTF_INSTEAD
// Reentrant versions of floating-point conversion functions.  The argument
// lists look slightly different on different operating systems, so we're
// encapsulating the differences here.

#if defined (__CYGWIN__) || defined(__DJGPP)
  inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* buf)
    { return ecvtbuf(x, n, pt, sign, buf); }
  inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* buf)
    { return fcvtbuf(x, n, pt, sign, buf); }
# ifndef _STLP_NO_LONG_DOUBLE
  inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return ecvtbuf(x, n, pt, sign, buf); }
  inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return fcvtbuf(x, n, pt, sign, buf); }
# endif
#elif defined (_STLP_USE_GLIBC)
  inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* buf)
    { return buf + ecvt_r(x, n, pt, sign, buf, NDIG+2); }
  inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* buf)
    { return buf + fcvt_r(x, n, pt, sign, buf, NDIG+2); }
#  ifndef _STLP_NO_LONG_DOUBLE
  inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return buf + qecvt_r(x, n, pt, sign, buf, NDIG+2); }
  inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return buf + qfcvt_r(x, n, pt, sign, buf, NDIG+2); }
#  endif
#elif defined (_STLP_SCO_OPENSERVER) || defined (__NCR_SVR)
  inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* buf)
    { return ecvt(x, n, pt, sign); }
  inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* buf)
    { return fcvt(x, n, pt, sign); }
#  ifndef _STLP_NO_LONG_DOUBLE
  inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return ecvtl(x, n, pt, sign); }
  inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return fcvtl(x, n, pt, sign); }
#  endif
#elif defined (__sun)
  inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* buf)
    { return econvert(x, n, pt, sign, buf); }
  inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* buf)
    { return fconvert(x, n, pt, sign, buf); }
#  ifndef _STLP_NO_LONG_DOUBLE
  inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return qeconvert(&x, n, pt, sign, buf); }
  inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return qfconvert(&x, n, pt, sign, buf); }
#  endif
#elif defined (__DECCXX) 
  inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* buf)
    { return (ecvt_r(x, n, pt, sign, buf, NDIG)==0 ? buf : 0); }
  inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* buf)
    { return (fcvt_r(x, n, pt, sign, buf, NDIG)==0 ? buf : 0); }
#  ifndef _STLP_NO_LONG_DOUBLE
// fbp : no "long double" conversions !
  inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return (ecvt_r((double)x, n, pt, sign, buf, NDIG)==0 ? buf : 0) ; }
  inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return (fcvt_r((double)x, n, pt, sign, buf, NDIG)==0 ? buf : 0); }
#  endif
#elif defined (__hpux)
   inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* buf)
     { return ecvt(x, n, pt, sign); }
   inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* buf)
     { return fcvt(x, n, pt, sign); }
# ifndef _STLP_NO_LONG_DOUBLE

#  if defined( _REENTRANT ) && (defined(_PTHREADS_DRAFT4) || defined(PTHREAD_THREADS_MAX))
   inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf)
     { return (_ldecvt_r(*(long_double*)&x, n, pt, sign, buf, NDIG+2)==0 ? buf : 0); }
   inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf)
     { return (_ldfcvt_r(*(long_double*)&x, n, pt, sign, buf, NDIG+2)==0 ? buf : 0); }
#  else
   inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf)
     { return _ldecvt(*(long_double*)&x, n, pt, sign); }
   inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf)
     { return _ldfcvt(*(long_double*)&x, n, pt, sign); }
#  endif
# endif
#elif defined (_AIX)
  inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* buf)
    { LOCK_CVT RETURN_CVT(ecvt, x, n, pt, sign, buf) }
  inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* buf)
    { LOCK_CVT RETURN_CVT(fcvt, x, n, pt, sign, buf) }
# ifndef _STLP_NO_LONG_DOUBLE
  inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf)
    { LOCK_CVT RETURN_CVT(ecvt, x, n, pt, sign, buf) }
  inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf)
    { LOCK_CVT RETURN_CVT(fcvt, x, n, pt, sign, buf) }
# endif
#elif defined (__unix)  && !defined(__FreeBSD__)  && !defined(__NetBSD__) && !defined(__APPLE__) && !defined(_CRAY)
  inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* buf)
    { return ecvt_r(x, n, pt, sign, buf); }
  inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* buf)
    { return fcvt_r(x, n, pt, sign, buf); }
# ifndef _STLP_NO_LONG_DOUBLE
  inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return qecvt_r(x, n, pt, sign, buf); }
  inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf)
    { return qfcvt_r(x, n, pt, sign, buf); }
#  endif
#elif defined (_MSC_VER) || defined (__MINGW32__) || defined (__BORLANDC__)
// those guys claim _cvt functions being reentrant.
inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* buf) {  return _ecvt(x, n, pt, sign); }
inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* buf) { return _fcvt(x, n, pt, sign); }
# ifndef _STLP_NO_LONG_DOUBLE
inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf) { return _ecvt((double)x, n, pt, sign); }
inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf) { return _fcvt((double)x, n, pt, sign); }
# endif
#elif defined (__ISCPP__)
inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* buf)
{ return _fp_ecvt( x, n, pt, sign, buf); }

inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* buf)
{ return _fp_fcvt(x, n, pt, sign, buf); }

# ifndef _STLP_NO_LONG_DOUBLE
inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* buf)
{ return _fp_ecvt( x, n, pt, sign, buf); }

inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* buf)
{ return _fp_fcvt(x, n, pt, sign, buf); }
# endif
#elif defined (__MRC__) || defined(__SC__) || defined(_CRAY)
inline char* _Stl_ecvtR(double x, int n, int* pt, int* sign, char* )
{ return ecvt( x, n, pt, sign ); }
inline char* _Stl_fcvtR(double x, int n, int* pt, int* sign, char* )
{ return fcvt(x, n, pt, sign); }
#ifndef _STLP_NO_LONG_DOUBLE
inline char* _Stl_qecvtR(long double x, int n, int* pt, int* sign, char* )
{ return ecvt( x, n, pt, sign ); }
inline char* _Stl_qfcvtR(long double x, int n, int* pt, int* sign, char* )
{ return fcvt(x, n, pt, sign); }
#endif
#endif


//----------------------------------------------------------------------
// num_put

// Helper functions for _M_do_put_float

// __format_float formats a mantissa and exponent as returned by
// one of the conversion functions (ecvt_r, fcvt_r, qecvt_r, qfcvt_r)
// according to the specified precision and format flags.  This is
// based on doprnt but is much simpler since it is concerned only
// with floating point input and does not consider all formats.  It
// also does not deal with blank padding, which is handled by
// __copy_float_and_fill. 

void __format_float_scientific(char * buf, const char * bp, 
			       int decpt, int sign, bool is_zero,
			       ios_base::fmtflags flags,
			       int precision, bool /* islong */)
{
  char * suffix;
  char expbuf[MAXESIZ + 2];
  // sign if required
  if (sign)
    *buf++ = '-';
  else if (flags & ios_base::showpos)
    *buf++ = '+';
  
  // first digit of mantissa
  *buf++ = *bp++;

  // decimal point if required
  if (precision != 0 || flags & ios_base::showpoint)
    *buf++ = '.';
  // rest of mantissa
  int rz = precision;
  while (rz-- > 0 && *bp != 0)
    *buf++ = *bp++;

  // exponent
  *(suffix = &expbuf[MAXESIZ]) = 0;
  if (!is_zero) {
    int nn = decpt - 1;
    if (nn < 0)
      nn = -nn;
    for (; nn > 9; nn /= 10)
      *--suffix = (char) todigit(nn % 10);
    *--suffix = (char) todigit(nn);
  }
  	
  // prepend leading zeros to exponent
  while (suffix > &expbuf[MAXESIZ - 2])
    *--suffix = '0';
  
  // put in the exponent sign
  *--suffix = (char) ((decpt > 0 || is_zero ) ? '+' : '-');
  
  // put in the e
  *--suffix = flags & ios_base::uppercase ? 'E' : 'e';

  // copy the suffix
  strcpy(buf, suffix);
}

static inline 
void __flush_static_buf(string &buf, 
                        char *sbeginbuf, char *sendbuf, char *&scurpos) {
  if (scurpos == sendbuf) {
    buf.append(sbeginbuf, sendbuf);
    scurpos = sbeginbuf;
  }
}
  
void __format_float_fixed(string &buf, const char * bp,
			  int decpt, int sign, bool /* x */,
			  ios_base::fmtflags flags,
			  int precision, bool islong )
{
  char static_buf[128];
  char *sbuf = static_buf;
  char *sendbuf = static_buf + sizeof(static_buf);

  if (sign && decpt > -precision && *bp != 0)
    *sbuf++ = '-';
  else if (flags & ios_base::showpos)
    *sbuf++ = '+';
  
  int rzero   = 0;
  int nn      = decpt;
  int k       = 0;
  int maxfsig = islong ? 2*MAXFSIG : MAXFSIG;

  do {
    *sbuf++ = (char) ((nn <= 0 || *bp == 0 || k >= maxfsig) ?
      '0' : (k++, *bp++));
    __flush_static_buf(buf, static_buf, sendbuf, sbuf);
  } while (--nn > 0);

  // decimal point if needed
  if (flags & ios_base::showpoint || precision > 0) {
    *sbuf++ = '.';
    __flush_static_buf(buf, static_buf, sendbuf, sbuf);
  }

  // digits after decimal point if any
  nn = (min) (precision, MAXFCVT);
  if (precision > nn)
    rzero = precision - nn;
  while (--nn >= 0) {
    *sbuf++ = (++decpt <= 0 || *bp == '\0' || k >= maxfsig)
      		? '0' : (k++, *bp++);
    __flush_static_buf(buf, static_buf, sendbuf, sbuf);
  }

  // trailing zeros if needed
  while (rzero-- > 0) {
    *sbuf++ = '0';
    __flush_static_buf(buf, static_buf, sendbuf, sbuf);
  }
  buf.append(static_buf, sbuf);
}


void __format_nan_or_inf(char * buf, double x,
			 ios_base::fmtflags flags)
{
  static const char* inf[2] = { "inf", "Inf" };
  static const char* nan[2] = { "nan", "NaN" };
  const char** inf_or_nan = 0;
  if (_Stl_is_inf((double)x)) {            // Infinity
    inf_or_nan = inf;
    if (_Stl_is_neg_inf((double)x))
      *buf++ = '-';
    else if (flags & ios_base::showpos)
      *buf++ = '+';
  }
  else {                      // NaN
    inf_or_nan = nan;
    if (_Stl_is_neg_nan((double)x))
      *buf++ = '-';
    else if (flags & ios_base::showpos)
      *buf++ = '+';
  }
  strcpy(buf, flags & ios_base::uppercase ? inf_or_nan[1] : inf_or_nan[0]);  
}

template <class max_double_type>
static inline 
void __format_float(string &buf, const char * bp,
                    int decpt, int sign, max_double_type x,
                    ios_base::fmtflags flags,
                    int precision, bool islong)
{
  char static_buf[128];
  // Output of infinities and NANs does not depend on the format flags
  if (_Stl_is_nan_or_inf((double)x)) {       // Infinity or NaN
    __format_nan_or_inf(static_buf, x, flags);
    buf = static_buf;
  } 
  else {                        // representable number
    switch (flags & ios_base::floatfield) {
    case ios_base::scientific:
      __format_float_scientific(static_buf, bp, decpt, sign, x == 0.0, flags,
                                precision, islong);
      buf = static_buf;
      break;
      
    case ios_base::fixed:
      __format_float_fixed(buf, bp, decpt, sign, true, flags,
                           precision, islong);
      break;
      
    default: // g format
      // establish default precision
      if (flags & ios_base::showpoint || precision > 0) {
        if (precision == 0) precision = 1;
      }
      else
        precision = 6;
      
      // reset exponent if value is zero
      if (x == 0)
        decpt = 1;
      
      int kk = precision;
      if (!(flags & ios_base::showpoint)) {
        size_t n = strlen(bp);
        if (n < kk)
          kk = (int)n;
        while (kk >= 1 && bp[kk-1] == '0')
          --kk;
      }
      
      if (decpt < -3 || decpt > precision) {
        precision = kk - 1;
        __format_float_scientific(static_buf, bp, decpt, sign, x == 0,
                                  flags, precision, islong);
        buf = static_buf;
      }
      else {
        precision = kk - decpt;
        __format_float_fixed(buf, bp, decpt, sign, true,
                             flags, precision, islong);
      }
      break;
    } /* switch */
  } /* else */
}

# else
// Creates a format string for sprintf()
static int fill_fmtbuf(char* fmtbuf, ios_base::fmtflags flags, char long_modifier)
{
  fmtbuf[0] = '%';
  int i = 1;

  if (flags & ios_base::showpos)
    fmtbuf[i++] = '+';

  if (flags & ios_base::showpoint)
    fmtbuf[i++] = '#';

  fmtbuf[i++] = '.';
  fmtbuf[i++] = '*';
  
  if (long_modifier)
    fmtbuf[i++] = long_modifier;

  switch (flags & ios_base::floatfield)
    {
    case ios_base::scientific:
      fmtbuf[i++] = (flags & ios_base::uppercase) ?  'E' : 'e';      
      break;
    case ios_base::fixed:
#if defined (__FreeBSD__)
      fmtbuf[i++] = 'f';
#else
      fmtbuf[i++] = (flags & ios_base::uppercase) ? 'F' : 'f'; 
#endif
      break;
    default:
      fmtbuf[i++] = (flags & ios_base::uppercase) ?  'G' : 'g';      
      break;
    }

  fmtbuf[i] = 0;
  return i;
}
# endif  /* USE_SPRINTF_INSTEAD */


void  _STLP_CALL
__write_float(string &buf, ios_base::fmtflags flags, int precision,
              double x)
{
# ifdef USE_SPRINTF_INSTEAD
  char static_buf[128];
  char fmtbuf[32];
  fill_fmtbuf(fmtbuf, flags, 0);
  sprintf(static_buf, fmtbuf, precision, x);    
  // we should be able to return static_buf + sprintf(), but we do not trust'em...
  buf = static_buf;
# else
  char cvtbuf[NDIG+2];
  char * bp;
  int decpt, sign;

  switch (flags & ios_base::floatfield) {
  case ios_base::fixed:
    bp = _Stl_fcvtR(x, (min) (precision, MAXFCVT), &decpt, &sign, cvtbuf);
    break;
  case ios_base::scientific :
    bp = _Stl_ecvtR(x, (min) (precision + 1, MAXECVT), &decpt, &sign, cvtbuf);
    break;
  default :
    bp = _Stl_ecvtR(x, (min) (precision, MAXECVT), &decpt, &sign, cvtbuf);
    break;
  }
  __format_float(buf, bp, decpt, sign, x, flags, precision, false);
# endif

}

# ifndef _STLP_NO_LONG_DOUBLE
void _STLP_CALL
__write_float(string &buf, ios_base::fmtflags flags, int precision,
              long double x)
{
# ifdef USE_SPRINTF_INSTEAD
  char static_buf[128];
  char fmtbuf[64];
  int i = fill_fmtbuf(fmtbuf, flags, 'L');
  sprintf(static_buf, fmtbuf, precision, x);    
  // we should be able to return buf + sprintf(), but we do not trust'em...
  buf = static_buf;
# else
  char cvtbuf[NDIG+2];
  char * bp;
  int decpt, sign;

  switch (flags & ios_base::floatfield) {
  case ios_base::fixed:
    bp = _Stl_qfcvtR(x, (min) (precision, MAXFCVT), &decpt, &sign, cvtbuf);
    break;
  case ios_base::scientific :
    bp = _Stl_qecvtR(x, (min) (precision + 1, MAXECVT),     &decpt, &sign, cvtbuf);
    break;
  default :
    bp = _Stl_qecvtR(x, (min) (precision, MAXECVT), &decpt, &sign, cvtbuf);
    break;
  }
  __format_float(buf, bp, decpt, sign, x, flags, precision, true);
# endif
}
# endif


# ifndef _STLP_NO_WCHAR_T

wchar_t* _STLP_CALL
__convert_float_buffer(const char* first, const char* last, wchar_t* out,
                       const ctype<wchar_t>& ct, wchar_t dot)
{
  ct.widen(first, last, out);
  if (ct.widen('.') != dot)
    replace(out, out + (last - first), ct.widen('.'), dot);
  return out + (last - first);
}

# endif

void _STLP_CALL
__adjust_float_buffer(char* first, char* last, char dot)
{
  if ('.' != dot)
    replace(first, last, '.', dot);
}

_STLP_END_NAMESPACE

// Local Variables:
// mode:C++
// End:
