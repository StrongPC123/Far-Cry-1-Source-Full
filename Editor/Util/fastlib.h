#ifndef __FAST_CODE_H_INCLUDED__
#define __FAST_CODE_H_INCLUDED__

// Needed to control floating-point pecision
#include <float.h>

////////////////////////////////////////////////////////////////////////
// Casting - Taken from the www.gamedev.net forums and www.nvidia.com
////////////////////////////////////////////////////////////////////////

#define __clamp(v, _min, _max) ( (v>_max) ? (_max) : ( (v<_min) ? (_min) : (v) ) )

#ifdef WIN64

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Round a floating point number to an integer. Note that (int + .5)
// is rounded to (int + 1).
__forceinline int RoundFloatToInt (float f)
{
	return (int)(f+0.5f);
}

// Doesn't take the pointer, is a bit faster
__forceinline int __stdcall FloatToIntRet(float  x)
{
	return (int)(x+0.5f);
}

// Casting floats to unsigned chars is also very expensive, just
// NEVER cast with (unsigned char)
__forceinline BYTE __stdcall FloatToByte(float  x) 
{ 
	float  t = x + (float) 0xC00000;
	return * (BYTE *) &t; 
}

// Fast floor() for (x >= 0) && (x < 2^31). MUCH faster than the normal
// floor()
__forceinline unsigned int __stdcall ifloor(float  x)
{
	DWORD e = (0x7F + 31) - ((* (DWORD *) &x & 0x7F800000) >> 23);
	DWORD m = 0x80000000 | (* (DWORD *) &x << 8);
	return (m >> e) & -(e < 32); 
}

// Converts to integer equal to or less than, asm version
__forceinline int ftoi(float f)
{
	return (int)(f);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#else //WIN64

// Round a floating point number to an integer. Note that (int + .5)
// is rounded to (int + 1).
__forceinline int RoundFloatToInt (float f)
{
	int i;
	__asm fld [f]
	__asm fistp [i]
	return i;
}

// Doesn't take the pointer, is a bit faster
__forceinline int __stdcall FloatToIntRet(float  x)
{
	int	   t;
	__asm  fld   x  
	__asm  fistp t
	return t;
}
 
// Casting floats to unsigned chars is also very expensive, just
// NEVER cast with (unsigned char)
__forceinline BYTE __stdcall FloatToByte(float  x) 
{ 
	float  t = x + (float) 0xC00000;
	return * (BYTE *) &t; 
}

// Fast floor() for (x >= 0) && (x < 2^31). MUCH faster than the normal
// floor()
__forceinline unsigned int __stdcall ifloor(float  x)
{
	DWORD e = (0x7F + 31) - ((* (DWORD *) &x & 0x7F800000) >> 23);
	DWORD m = 0x80000000 | (* (DWORD *) &x << 8);
	return (m >> e) & -(e < 32); 
}

// Converts to integer equal to or less than, asm version
__forceinline int ftoi(float f)
{
	static float Half = 0.5;
	int i; 
	__asm fld [f]
	__asm fsub [Half]
	__asm fistp [i]
	return i;
}

#endif //WIN64

//! Return random value in [-1,1] diaposone.
inline float frand()
{
	return ((float)rand()*2.0f / RAND_MAX) - 1.0f;
}

#endif // __FAST_CODE_H_INCLUDED__