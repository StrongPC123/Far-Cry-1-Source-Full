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

# ifndef COMPLEX_IMPL_H
#  define  COMPLEX_IMPL_H

#include <complex>
#include <cmath>
#include <math.h>
#include <stl/_cmath.h>

# if (defined (__sgi) && !defined(__GNUC__)) /* || defined (__DECCXX) */

# define _STLP_ABSF   _STLP_VENDOR_CSTD::fabsf
# define _STLP_ATAN2F _STLP_VENDOR_CSTD::atan2f
# define _STLP_SINF   _STLP_VENDOR_CSTD::sinf
# define _STLP_COSF   _STLP_VENDOR_CSTD::cosf
# define _STLP_SQRTF  _STLP_VENDOR_CSTD::sqrtf
# define _STLP_EXPF   _STLP_VENDOR_CSTD::expf
# define _STLP_LOG10F _STLP_VENDOR_CSTD::log10f
# define _STLP_LOGF   _STLP_VENDOR_CSTD::logf
# define _STLP_SINHF  _STLP_VENDOR_CSTD::sinhf
# define _STLP_COSHF  _STLP_VENDOR_CSTD::coshf
# define _STLP_HYPOTF _STLP_VENDOR_CSTD::hypotf


# define _STLP_ABSL   _STLP_VENDOR_CSTD::fabsl
# define _STLP_ATAN2L _STLP_VENDOR_CSTD::atan2l
# define _STLP_SINL   _STLP_VENDOR_CSTD::sinl
# define _STLP_COSL   _STLP_VENDOR_CSTD::cosl
# define _STLP_SQRTL  _STLP_VENDOR_CSTD::sqrtl
# define _STLP_EXPL   _STLP_VENDOR_CSTD::expl
# define _STLP_LOG10L _STLP_VENDOR_CSTD::log10l
# define _STLP_LOGL   _STLP_VENDOR_CSTD::logl
# define _STLP_SINHL  _STLP_VENDOR_CSTD::sinhl
# define _STLP_COSHL  _STLP_VENDOR_CSTD::coshl
// # define _STLP_HYPOT  ::hypot
# define _STLP_HYPOTL _STLP_VENDOR_CSTD::hypotl

#else

# define _STLP_ABSF (float)_STLP_DO_ABS(double)
# define _STLP_ABSL (long double)_STLP_DO_ABS(double)
# define _STLP_ATAN2F (float)_STLP_DO_ATAN2(double)
# define _STLP_ATAN2L (long double)_STLP_DO_ATAN2(double)
# define _STLP_SINF   (float)_STLP_DO_SIN(double)
# define _STLP_SINL   (long double)_STLP_DO_SIN(double)
# define _STLP_COSF   (float)_STLP_DO_COS(double)
# define _STLP_COSL   (long double)_STLP_DO_COS(double)
# define _STLP_SQRTF  (float)_STLP_DO_SQRT(double)
# define _STLP_SQRTL  (long double)_STLP_DO_SQRT(double)
# define _STLP_EXPF   (float)_STLP_DO_EXP(double)
# define _STLP_EXPL   (long double)_STLP_DO_EXP(double)
# define _STLP_LOG10F   (float)_STLP_DO_LOG10(double)
# define _STLP_LOG10L   (long double)_STLP_DO_LOG10(double)
# define _STLP_LOGF   (float)_STLP_DO_LOG(double)
# define _STLP_LOGL   (long double)_STLP_DO_LOG(double)
# define _STLP_SINHF   (float)_STLP_DO_SINH(double)
# define _STLP_SINHL   (long double)_STLP_DO_SINH(double)
# define _STLP_COSHF   (float)_STLP_DO_COSH(double)
# define _STLP_COSHL   (long double)_STLP_DO_COSH(double)
# define _STLP_HYPOTF   (float)_STLP_DO_HYPOT(double)
# define _STLP_HYPOTL   (long double)_STLP_DO_HYPOT(double)

#endif

# define _STLP_ABS      (double)_STLP_DO_ABS(double)
# define _STLP_ATAN2    (double)_STLP_DO_ATAN2(double)
# define _STLP_SIN      (double)_STLP_DO_SIN(double)
# define _STLP_COS      (double)_STLP_DO_COS(double)
# define _STLP_SQRT     (double)_STLP_DO_SQRT(double)
# define _STLP_EXP      (double)_STLP_DO_EXP(double)
# define _STLP_LOG10    (double)_STLP_DO_LOG10(double)
# define _STLP_LOG      (double)_STLP_DO_LOG(double)
# define _STLP_SINH     (double)_STLP_DO_SINH(double)
# define _STLP_COSH     (double)_STLP_DO_COSH(double)
# define _STLP_HYPOT    _STLP_DO_HYPOT(double)

#endif
