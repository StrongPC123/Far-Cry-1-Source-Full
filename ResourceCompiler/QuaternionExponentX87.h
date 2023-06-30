#ifndef _CRY_ANIMATION_QUATERNION_EXPONENT_HDR_
#define _CRY_ANIMATION_QUATERNION_EXPONENT_HDR_

#include "platform.h"

#if defined(_CPU_X86) && !defined(LINUX)
#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////
// x87 asm optimized quaternion exponent
// Estimated runtime in good conditions: 315 cycles on P4
// PARAMETERS:
//   pSrcVector[IN] - the vector to calculate the exponent for
//   pDstQuat  [OUT]- the quaternion (exponent of the input)
// NOTE:
//   The input vector mimics a quaternion with 0 real component (W)
//   This version uses FSINCOS, which takes ~70% of execution time
//////////////////////////////////////////////////////////////////////////
void quaternionExponent_x87(const float* pSrc, float* pDst);

//////////////////////////////////////////////////////////////////////////
// x87 asm optimized quaternion exponent
// Estimated runtime: 110 cycles on P4
// PARAMETERS:
//   pSrcVector[IN] - the vector to calculate the exponent for
//   pDstQuat  [OUT]- the quaternion (exponent of the input)
// WARNING:
//   the source vector length should be no more than 3-4, otherwise the sin/cos
//   approximations won't work
// NOTE:
//   The input vector mimics a quaternion with 0 real component (W)
//   This version uses approximation to FSINCOS (tailor series up to 9th magnitude)
//////////////////////////////////////////////////////////////////////////
void quaternionExponent_x87approx(const float* pSrc, float* pDst);

#ifdef __cplusplus
}
#endif

#endif

#endif
