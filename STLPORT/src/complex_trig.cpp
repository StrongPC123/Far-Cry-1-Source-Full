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


// Trigonometric and hyperbolic functions for complex<float>, 
// complex<double>, and complex<long double>


#include "complex_impl.h"

#include <cfloat>
#include <cmath>

_STLP_BEGIN_NAMESPACE


//----------------------------------------------------------------------
// helpers

#ifdef __sgi
  static const union { unsigned int i; float f; } float_ulimit = { 0x42b2d4fc };
  static const float float_limit = float_ulimit.f;
  static union {
    struct { unsigned int h; unsigned int l; } w;
    double d;
  } double_ulimit = { 0x408633ce, 0x8fb9f87d };
  static const double double_limit = double_ulimit.d;
  static union {
    struct { unsigned int h[2]; unsigned int l[2]; } w;
    long double ld;
  } ldouble_ulimit = {0x408633ce, 0x8fb9f87e, 0xbd23b659, 0x4e9bd8b1};
# ifndef _STLP_NO_LONG_DOUBLE
  static const long double ldouble_limit = ldouble_ulimit.ld;
# endif
#else
  static const float float_limit = _STLP_LOGF(FLT_MAX);
  static const double double_limit = _STLP_DO_LOG(double)(DBL_MAX);
# ifndef _STLP_NO_LONG_DOUBLE
  static const long double ldouble_limit = _STLP_LOGL(LDBL_MAX);
# endif
#endif


//----------------------------------------------------------------------
// sin

_STLP_DECLSPEC complex<float>  _STLP_CALL sin(const complex<float>& z) {
  return complex<float>(_STLP_SINF(z._M_re) * _STLP_COSHF(z._M_im),
                        _STLP_COSF(z._M_re) * _STLP_SINHF(z._M_im));
}

_STLP_DECLSPEC complex<double> _STLP_CALL sin(const complex<double>& z) {
  return complex<double>(_STLP_SIN(z._M_re) * _STLP_COSH(z._M_im),
                         _STLP_COS(z._M_re) * _STLP_SINH(z._M_im));
}

#ifndef _STLP_NO_LONG_DOUBLE
_STLP_DECLSPEC complex<long double> _STLP_CALL sin(const complex<long double>& z) {
  return complex<long double>(_STLP_SINL(z._M_re) * _STLP_COSHL(z._M_im),
                              _STLP_COSL(z._M_re) * _STLP_SINHL(z._M_im));
}
#endif

//----------------------------------------------------------------------
// cos

_STLP_DECLSPEC complex<float> _STLP_CALL cos(const complex<float>& z) {
  return complex<float>(_STLP_COSF(z._M_re) * _STLP_COSHF(z._M_im),
                        -_STLP_SINF(z._M_re) * _STLP_SINHF(z._M_im));
}

_STLP_DECLSPEC complex<double> _STLP_CALL cos(const complex<double>& z) {
  return complex<double>(_STLP_COS(z._M_re) * _STLP_COSH(z._M_im),
                        -_STLP_SIN(z._M_re) * _STLP_SINH(z._M_im));
}

#ifndef _STLP_NO_LONG_DOUBLE
_STLP_DECLSPEC complex<long double> _STLP_CALL cos(const complex<long double>& z) {
  return complex<long double>(_STLP_COSL(z._M_re) * _STLP_COSHL(z._M_im),
                              -_STLP_SINL(z._M_re) * _STLP_SINHL(z._M_im));
}
# endif

//----------------------------------------------------------------------
// tan

_STLP_DECLSPEC complex<float> _STLP_CALL tan(const complex<float>& z) {
  float re2 = 2.f * z._M_re;
  float im2 = 2.f * z._M_im;

  if (_STLP_ABSF(im2) > float_limit)
    return complex<float>(0.f, (im2 > 0 ? 1.f : -1.f));
  else {
    float den = _STLP_COSF(re2) + _STLP_COSHF(im2);
    return complex<float>(_STLP_SINF(re2) / den, _STLP_SINHF(im2) / den);
  }
}

_STLP_DECLSPEC complex<double> _STLP_CALL tan(const complex<double>& z) {
  double re2 = 2. * z._M_re;
  double im2 = 2. * z._M_im;

  if (fabs(float(im2)) > double_limit)
    return complex<double>(0., (im2 > 0 ? 1. : -1.));
  else {
    double den = _STLP_COS(re2) + _STLP_COSH(im2);
    return complex<double>(_STLP_SIN(re2) / den, _STLP_SINH(im2) / den);
  }
}

#ifndef _STLP_NO_LONG_DOUBLE
_STLP_DECLSPEC complex<long double> _STLP_CALL tan(const complex<long double>& z) {
  long double re2 = 2.l * z._M_re;
  long double im2 = 2.l * z._M_im;
  if (_STLP_ABSL(im2) > ldouble_limit)
    return complex<long double>(0.l, (im2 > 0 ? 1.l : -1.l));
  else {
    long double den = _STLP_COSL(re2) + _STLP_COSHL(im2);
    return complex<long double>(_STLP_SINL(re2) / den, _STLP_SINHL(im2) / den);
  }
}

# endif

//----------------------------------------------------------------------
// sinh

_STLP_DECLSPEC complex<float> _STLP_CALL sinh(const complex<float>& z) {
  return complex<float>(_STLP_SINHF(z._M_re) * _STLP_COSF(z._M_im),
                        _STLP_COSHF(z._M_re) * _STLP_SINF(z._M_im));
}

_STLP_DECLSPEC complex<double> _STLP_CALL sinh(const complex<double>& z) {
  return complex<double>(_STLP_SINH(z._M_re) * _STLP_COS(z._M_im),
                         _STLP_COSH(z._M_re) * _STLP_SIN(z._M_im));
}

#ifndef _STLP_NO_LONG_DOUBLE
_STLP_DECLSPEC complex<long double> _STLP_CALL sinh(const complex<long double>& z) {
  return complex<long double>(_STLP_SINHL(z._M_re) * _STLP_COSL(z._M_im),
                              _STLP_COSHL(z._M_re) * _STLP_SINL(z._M_im));
}
#endif

//----------------------------------------------------------------------
// cosh

_STLP_DECLSPEC complex<float> _STLP_CALL cosh(const complex<float>& z) {
  return complex<float>(_STLP_COSHF(z._M_re) * _STLP_COSF(z._M_im),
                        _STLP_SINHF(z._M_re) * _STLP_SINF(z._M_im));
}

_STLP_DECLSPEC complex<double> _STLP_CALL cosh(const complex<double>& z) {
  return complex<double>(_STLP_COSH(z._M_re) * _STLP_COS(z._M_im),
                         _STLP_SINH(z._M_re) * _STLP_SIN(z._M_im));
}

#ifndef _STLP_NO_LONG_DOUBLE
_STLP_DECLSPEC complex<long double> _STLP_CALL cosh(const complex<long double>& z) {
  return complex<long double>(_STLP_COSHL(z._M_re) * _STLP_COSL(z._M_im),
                              _STLP_SINHL(z._M_re) * _STLP_SINL(z._M_im));
}
#endif

//----------------------------------------------------------------------
// tanh

_STLP_DECLSPEC complex<float> _STLP_CALL tanh(const complex<float>& z) {
  float re2 = 2.f * z._M_re;
  float im2 = 2.f * z._M_im;
  if (_STLP_ABSF(re2) > float_limit)
    return complex<float>((re2 > 0 ? 1.f : -1.f), 0.f);
  else {
    float den = _STLP_COSHF(re2) + _STLP_COSF(im2);
    return complex<float>(_STLP_SINHF(re2) / den, _STLP_SINF(im2) / den);
  }
}

_STLP_DECLSPEC complex<double> _STLP_CALL tanh(const complex<double>& z) {
  double re2 = 2. * z._M_re;
  double im2 = 2. * z._M_im;  
  if (fabs(float(re2)) > double_limit)
    return complex<double>((re2 > 0 ? 1. : -1.), 0.);
  else {
    double den = _STLP_COSH(re2) + _STLP_COS(im2);
    return complex<double>(_STLP_SINH(re2) / den, _STLP_SIN(im2) / den);
  }
}

#ifndef _STLP_NO_LONG_DOUBLE
_STLP_DECLSPEC complex<long double> _STLP_CALL tanh(const complex<long double>& z) {
  long double re2 = 2.l * z._M_re;
  long double im2 = 2.l * z._M_im;
  if (_STLP_ABSL(re2) > ldouble_limit)
    return complex<long double>((re2 > 0 ? 1.l : -1.l), 0.l);
  else {
    long double den = _STLP_COSHL(re2) + _STLP_COSL(im2);
    return complex<long double>(_STLP_SINHL(re2) / den, _STLP_SINL(im2) / den);
  }
}
#endif
_STLP_END_NAMESPACE
