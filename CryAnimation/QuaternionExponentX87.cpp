#include "stdafx.h"
#include "QuaternionExponentX87.h"

#if !defined(LINUX)
#if defined(_CPU_X86) 
static const float fEpsilon = 1e-4f;
//////////////////////////////////////////////////////////////////////////
// x87 asm optimized quaternion exponent
// PARAMETERS:
//   pSrcVector[IN] - the vector to calculate the exponent for
//   pDstQuat  [OUT]- the quaternion (exponent of the input)
// NOTE:
//   The input vector mimics a quaternion with 0 real component (W)
//   This version uses FSINCOS, which takes ~70% of execution time
//////////////////////////////////////////////////////////////////////////
extern "C" void quaternionExponent_x87(const float* pSrc, float* pDst)
{
	_asm
	{
		mov ESI, pSrc
		mov EDI, pDst
		
		// double d = sqrt( double(pSrc[0])*pSrc[1] + double(pSrc[1])*pSrc[1] + double(pSrc[2])*pSrc[2]);

		fld [ESI+8]
		fld [ESI+4]
		fld [ESI  ]
		fld ST(0)
		fmul ST(0),ST(0)
		fld ST(2)
		fmul ST(0),ST(0)
		faddp ST(1),ST(0)
		fld ST(3)
		fmul ST(0),ST(0)
		faddp ST(1),ST(0)

		// ST(0): x^2+y^2+z^2 == d^2
		// ST(1): x
		// ST(2): y
		// ST(3): z

		fld fEpsilon
		fcomip ST, ST(1)
		jnc small_rotation // this path is almost never taken

		fsqrt
		fld ST(0)

		// now we need cos, sin to replace the current value
		fsincos 
		// STACK: cos, sin, sqrt, x, y, z

		fstp dword ptr [EDI]
		fdivrp ST(1),ST(0)   // STACK: sin(d)/d, x,y,z
		
		fmul ST(1),ST(0)
		fmul ST(2),ST(0)
		fmulp ST(3),ST(0)
		fstp dword ptr [EDI+ 4]
		fstp dword ptr [EDI+ 8]
		fstp dword ptr [EDI+12]
	}
	return;
	_asm
	{
small_rotation:
		fld1
		fsubrp ST(1),ST(0)
		fstp dword ptr [EDI  ]
		fstp dword ptr [EDI+4]
		fstp dword ptr [EDI+8]
		fstp dword ptr [EDI+12]
	}
}


static const float
	fDivBy2 = 1/2.0f,
	fDivBy3 = 1/3.0f,
	fDivBy4 = 1/4.0f,
	fDivBy5 = 1/5.0f,
	fDivBy6 = 1/6.0f,
	fDivBy7 = 1/7.0f,
	fDivBy8 = 1/8.0f,
	fDivBy9 = 1/9.0f;

// Takes pSrc: the x,y,z of the imaginary part of the quaternion 0+xi+yj+zk to calculate the exponent
// into pDst: the x,y,z,w of the resulting quaternion IN THAT ORDER
extern "C" void quaternionExponent_x87approx(const float* pSrc, float* pDst)
{
	_asm
	{
		mov ESI, pSrc
		mov EDI, pDst
		
		// double d = sqrt( double(pSrc[0])*pSrc[1] + double(pSrc[1])*pSrc[1] + double(pSrc[2])*pSrc[2]);

		fld [ESI+8]
		fld [ESI+4]
		fld [ESI  ]
		fld1
		fld1
		// STACK: 1 1 x y z
		fld ST(2)
		fmul ST(0),ST(0)
		fld ST(4)
		fmul ST(0),ST(0)
		faddp ST(1),ST(0)
		fld ST(5)
		fmul ST(0),ST(0)
		faddp ST(1),ST(0)

		// ST(0): x^2+y^2+z^2 == d^2
		// ST(1): 1
		// ST(2): 1
		// ST(3): x
		// ST(4): y
		// ST(5): z

		// now we need cos, sin to replace the current value
		fld ST(0)
		// STACK: D^2(temp), D^2(const), 1(will be cos), 1(will be sin/D), x, y, z
		fmul fDivBy2
		fsub ST(2),ST(0)
		fmul fDivBy3
		fsub ST(3),ST(0)

		// STACK: D^2/3!, D^2, 1-D^2/2!(will be cos), 1-D^2/3!(will be sin/D), x, y, z

		fmul ST(0),ST(1)
		fmul fDivBy4
		fadd ST(2),ST(0)
		fmul fDivBy5
		fadd ST(3),ST(0)

		// STACK: D^4/5!, D^2, 1-D^2/2!+D^4/4!(will be cos), 1-D^2/3!+D^4/5!(will be sin/D), x, y, z
		fmul ST(0),ST(1)
		fmul fDivBy6
		fsub ST(2),ST(0)
		fmul fDivBy7
		fsub ST(3),ST(0)

		// STACK: D^6/7!, D^2, 1-D^2/2!+D^4/4!-D^6/6!, 1-D^2/3!+D^4/5!-D^6/7!, x, y, z
		
		// the last step
		fmulp ST(1),ST(0)
		// STACK: D^8/7!, 1-D^2/2!+D^4/4!-D^6/6!, 1-D^2/3!+D^4/5!-D^6/7!, x, y, z
		fmul fDivBy8
		fadd ST(1),ST(0)
		fmul fDivBy9
		faddp ST(2),ST(0)
		// STACK: 1-D^2/2!+D^4/4!-D^6/6!+D^8/8!, 1-D^2/3!+D^4/5!-D^6/7!+D^8/9!, x, y, z
		// cos(D), sin(D)/D,x,y,z

		fstp dword ptr [EDI]
		
		fmul ST(1),ST(0)
		fmul ST(2),ST(0)
		fmulp ST(3),ST(0)
		fstp dword ptr [EDI+ 4]
		fstp dword ptr [EDI+ 8]
		fstp dword ptr [EDI+12]
	}
}
#endif
#endif
