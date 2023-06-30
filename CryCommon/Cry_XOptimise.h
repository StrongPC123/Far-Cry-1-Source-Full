
//////////////////////////////////////////////////////////////////////
//
//	Crytek Common Source code
//
//	File:Cry_Math.h
//	Description: Misc mathematical functions
//
//	History:
//	-Jan 31,2001: Created by Marco Corbetta
//	-Jan 04,2003: SSE and 3DNow optimizations by Andrey Honich
//
//////////////////////////////////////////////////////////////////////

#ifndef CRY_SIMD_H
#define CRY_SIMD_H

#if _MSC_VER > 1000
# pragma once
#endif

#include "TArray.h"
#include "platform.h"
//========================================================================================

extern int g_CpuFlags;


inline float AngleMod(float a)
{
  a = (float)((360.0/65536) * ((int)(a*(65536/360.0)) & 65535));
  return a;
}

inline unsigned short Degr2Word(float f)
{
  return (unsigned short)(AngleMod(f)/360.0f*65536.0f);
}
inline float Word2Degr(unsigned short s)
{
  return (float)s / 65536.0f * 360.0f;
}

/*****************************************************
MISC FUNCTIONS
*****************************************************/


//////////////////////////////////////
#if defined(_CPU_X86)
ILINE float __fastcall Ffabs(float f) {
	*((unsigned *) & f) &= ~0x80000000;
	return (f);
}
#else
inline float Ffabs(float x) { return fabsf(x); }
#endif

//////////////////////////////////////////////////////////////////////////
#if defined(_CPU_X86) && !defined(LINUX)
inline int fastftol_positive(float f)
{
  int i;
  f -= 0.5f;
  __asm fld [f]
  __asm fistp [i]
  return i;
}
#else
inline int fastftol_positive (float x) { return (int)x; }
#endif


#if defined(_CPU_X86) && !defined(LINUX)
ILINE int __fastcall FtoI(float  x)
{
  int	   t;
  __asm
  {
    fld   x
    fistp t
  }
  return t;
}
#else
inline int FtoI(float x) { return (int)x; }
#endif



//////////////////////////////////////
#define rnd()	(((float)rand())/RAND_MAX)  // Floating point random number generator ( 0 -> 1)

//////////////////////////////////////
inline void multMatrices(double dst[16], const double a[16], const double b[16])
{
  int i, j;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      dst[i * 4 + j] =
        b[i * 4 + 0] * a[0 * 4 + j] +
        b[i * 4 + 1] * a[1 * 4 + j] +
        b[i * 4 + 2] * a[2 * 4 + j] +
        b[i * 4 + 3] * a[3 * 4 + j];
    }
  }
}

//////////////////////////////////////
inline void multMatrices(float dst[16], const float a[16], const float b[16])
{
  int i, j;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      dst[i * 4 + j] =
        b[i * 4 + 0] * a[0 * 4 + j] +
        b[i * 4 + 1] * a[1 * 4 + j] +
        b[i * 4 + 2] * a[2 * 4 + j] +
        b[i * 4 + 3] * a[3 * 4 + j];
    }
  }
}

//////////////////////////////////////
// transform vector
inline void matmult_trans_only(float a[3], float b[4][4], float result[3])
{
  result[0] = a[0] * b[0][0] + a[1] * b[1][0] + a[2] * b[2][0] + b[3][0];
  result[1] = a[0] * b[0][1] + a[1] * b[1][1] + a[2] * b[2][1] + b[3][1];
  result[2] = a[0] * b[0][2] + a[1] * b[1][2] + a[2] * b[2][2] + b[3][2];
}


/*
======================================
Matrix 4x4
======================================
*/


inline void multMatrixf(float *product, const float *m1, const float *m2)
{
#define A(row,col)  m1[(col<<2)+row]
#define B(row,col)  m2[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

  int i;
  for (i=0; i<4; i++)
  {
    float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
    P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
    P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
    P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
    P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
  }

#undef A
#undef B
#undef P
}

#if !defined (GAMECUBE) && !defined PS2

//==========================================================================================
// 3DNow! optimizations

#pragma warning(push)
#pragma warning(disable:4731) // frame pointer register 'ebp' modified by inline assembly code

struct SConst3DN
{
  float m_fVal0;
  float m_fVal1;
};
const SConst3DN const3DN_1_N1 = {1.0f, -1.0f};
const SConst3DN const3DN_0_1 = {0.0f, 1.0f};




/*!
Compute inverse of 4x4 transformation SINGLE-PRECISION matrix.
Code lifted from Brian Paul's Mesa freeware OpenGL implementation.
Return true for success, false for failure (singular matrix)
*/
inline bool QQinvertMatrixf(float *out, const float *m)
{
#define SWAP_ROWS(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

	float wtmp[4][8];
	float m0, m1, m2, m3, s;
	float *r0, *r1, *r2, *r3;

	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

	r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
		r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
		r0[4] = 1.0f, r0[5] = r0[6] = r0[7] = 0.0f,

		r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
		r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
		r1[5] = 1.0f, r1[4] = r1[6] = r1[7] = 0.0f,

		r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
		r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
		r2[6] = 1.0f, r2[4] = r2[5] = r2[7] = 0.0f,

		r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
		r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
		r3[7] = 1.0f, r3[4] = r3[5] = r3[6] = 0.0f;

	/* choose pivot - or die */
	if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
	if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
	if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
	if (0.0 == r0[0])
	{
		return 0;
	}

	/* eliminate first variable     */
	m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
	s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
	s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
	s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
	s = r0[4];
	if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
	s = r0[5];
	if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
	s = r0[6];
	if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
	s = r0[7];
	if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

	/* choose pivot - or die */
	if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
	if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
	if (0.0 == r1[1])
	{
		return 0;
	}

	/* eliminate second variable */
	m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
	r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
	r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
	s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
	s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
	s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
	s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

	/* choose pivot - or die */
	if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
	if (0.0 == r2[2])
	{
		return 0;
	}

	/* eliminate third variable */
	m3 = r3[2]/r2[2];
	r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
		r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
		r3[7] -= m3 * r2[7];

	/* last check */
	if (0.0 == r3[3])
	{
		return 0;
	}

	s = 1.0f/r3[3];              /* now back substitute row 3 */
	r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

	m2 = r2[3];                 /* now back substitute row 2 */
	s  = 1.0f/r2[2];
	r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
		r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
	m1 = r1[3];
	r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
		r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
		r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

	m1 = r1[2];                 /* now back substitute row 1 */
	s  = 1.0f/r1[1];
	r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
		r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
	m0 = r0[2];
	r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
		r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

	m0 = r0[1];                 /* now back substitute row 0 */
	s  = 1.0f/r0[0];
	r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
		r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

	MAT(out,0,0) = r0[4]; MAT(out,0,1) = r0[5],
		MAT(out,0,2) = r0[6]; MAT(out,0,3) = r0[7],
		MAT(out,1,0) = r1[4]; MAT(out,1,1) = r1[5],
		MAT(out,1,2) = r1[6]; MAT(out,1,3) = r1[7],
		MAT(out,2,0) = r2[4]; MAT(out,2,1) = r2[5],
		MAT(out,2,2) = r2[6]; MAT(out,2,3) = r2[7],
		MAT(out,3,0) = r3[4]; MAT(out,3,1) = r3[5],
		MAT(out,3,2) = r3[6]; MAT(out,3,3) = r3[7];

	return 1;

#undef MAT
#undef SWAP_ROWS
}

inline float cryISqrtf(float fVal)
{
  unsigned int *n1 = (unsigned int *)&fVal;
  unsigned int n = 0x5f3759df - (*n1 >> 1);
  float *n2 = (float *)&n;
  fVal = (1.5f - (fVal * 0.5f) * *n2 * *n2) * *n2;
  return fVal;
}
#if defined _CPU_X86 && !defined(LINUX)
// ***************************************************************************
inline void cryPrecacheSSE(const void *src, int nbytes)
{
  _asm
  {
    mov esi, src
    mov ecx, nbytes
    // 64 bytes per pass
    shr ecx, 6
    jz endLabel

loopMemToL1:
    prefetchnta 64[ESI] // Prefetch next loop, non-temporal
    prefetchnta 96[ESI]

    movq mm1,  0[ESI] // Read in source data
    movq mm2,  8[ESI]
    movq mm3, 16[ESI]
    movq mm4, 24[ESI]
    movq mm5, 32[ESI]
    movq mm6, 40[ESI]
    movq mm7, 48[ESI]
    movq mm0, 56[ESI]

    add esi, 64
    dec ecx
    jnz loopMemToL1

    emms

endLabel:
  }
}
// ***************************************************************************
inline void cryPrecacheMMX(const void *src, int nbytes)
{
  _asm
  {
    mov esi, src
    mov ecx, nbytes
    // 64 bytes per pass
    shr ecx, 6
    jz endLabel

loopMemToL1:
    movq mm1,  0[ESI] // Read in source data
    movq mm2,  8[ESI]
    movq mm3, 16[ESI]
    movq mm4, 24[ESI]
    movq mm5, 32[ESI]
    movq mm6, 40[ESI]
    movq mm7, 48[ESI]
    movq mm0, 56[ESI]

    add esi, 64
    dec ecx
    jnz loopMemToL1

    emms

endLabel:
  }
}


#endif

inline void cryPrefetchNTSSE(const void *src)
{
#if defined(WIN32) && !defined(WIN64) && !defined(LINUX)
	_asm
  {
    mov esi, src
    prefetchnta [ESI] // Prefetch non-temporal
  }
#endif
}



ILINE void cryPrefetchT0SSE(const void *src)
{
#if defined(WIN32) && !defined(WIN64)
  _asm
  {
    mov esi, src
    prefetchT0 [ESI] // Prefetch
  }
#endif

#if defined(WIN64) &&  defined(_CPU_AMD64) && !defined(LINUX)
		_mm_prefetch( (char*)src, _MM_HINT_T0 );
#endif

}

//=================================================================================

// Very optimized memcpy() routine for AMD Athlon and Duron family.
// This code uses any of FOUR different basic copy methods, depending
// on the transfer size.
// NOTE:  Since this code uses MOVNTQ (also known as "Non-Temporal MOV" or
// "Streaming Store"), and also uses the software prefetch instructions,
// be sure you're running on Athlon/Duron or other recent CPU before calling!

#define TINY_BLOCK_COPY 64       // upper limit for movsd type copy
// The smallest copy uses the X86 "movsd" instruction, in an optimized
// form which is an "unrolled loop".

#define IN_CACHE_COPY 64 * 1024  // upper limit for movq/movq copy w/SW prefetch
// Next is a copy that uses the MMX registers to copy 8 bytes at a time,
// also using the "unrolled loop" optimization.   This code uses
// the software prefetch instruction to get the data into the cache.

#define UNCACHED_COPY 197 * 1024 // upper limit for movq/movntq w/SW prefetch
// For larger blocks, which will spill beyond the cache, it's faster to
// use the Streaming Store instruction MOVNTQ.   This write instruction
// bypasses the cache and writes straight to main memory.  This code also
// uses the software prefetch instruction to pre-read the data.
// USE 64 * 1024 FOR THIS VALUE IF YOU'RE ALWAYS FILLING A "CLEAN CACHE"

#define BLOCK_PREFETCH_COPY  infinity // no limit for movq/movntq w/block prefetch
#define CACHEBLOCK 80h // number of 64-byte blocks (cache lines) for block prefetch
// For the largest size blocks, a special technique called Block Prefetch
// can be used to accelerate the read operations.   Block Prefetch reads
// one address per cache line, for a series of cache lines, in a short loop.
// This is faster than using software prefetch.  The technique is great for
// getting maximum read bandwidth, especially in DDR memory systems.


#if defined _CPU_X86 && !defined(LINUX)
// Inline assembly syntax for use with Visual C++
inline void cryMemcpy( void* Dst, const void* Src, int Count )
{
  if (g_CpuFlags & CPUF_SSE)
  {
	  __asm
	  {
		  mov		ecx, [Count]	; number of bytes to copy
		  mov		edi, [Dst]		; destination
		  mov		esi, [Src]		; source
		  mov		ebx, ecx		; keep a copy of count

		  cld
		  cmp		ecx, TINY_BLOCK_COPY
		  jb		$memcpy_ic_3	; tiny? skip mmx copy

		  cmp		ecx, 32*1024		; dont align between 32k-64k because
		  jbe		$memcpy_do_align	;  it appears to be slower
		  cmp		ecx, 64*1024
		  jbe		$memcpy_align_done
	  $memcpy_do_align:
		  mov		ecx, 8			; a trick thats faster than rep movsb...
		  sub		ecx, edi		; align destination to qword
		  and		ecx, 111b		; get the low bits
		  sub		ebx, ecx		; update copy count
		  neg		ecx				; set up to jump into the array
		  add		ecx, offset $memcpy_align_done
		  jmp		ecx				; jump to array of movsbs

	  align 4
		  movsb
		  movsb
		  movsb
		  movsb
		  movsb
		  movsb
		  movsb
		  movsb

	  $memcpy_align_done:			; destination is dword aligned
		  mov		ecx, ebx		; number of bytes left to copy
		  shr		ecx, 6			; get 64-byte block count
		  jz		$memcpy_ic_2	; finish the last few bytes

		  cmp		ecx, IN_CACHE_COPY/64	; too big 4 cache? use uncached copy
		  jae		$memcpy_uc_test

	  // This is small block copy that uses the MMX registers to copy 8 bytes
	  // at a time.  It uses the "unrolled loop" optimization, and also uses
	  // the software prefetch instruction to get the data into the cache.
	  align 16
	  $memcpy_ic_1:			; 64-byte block copies, in-cache copy

		  prefetchnta [esi + (200*64/34+192)]		; start reading ahead

		  movq	mm0, [esi+0]	; read 64 bits
		  movq	mm1, [esi+8]
		  movq	[edi+0], mm0	; write 64 bits
		  movq	[edi+8], mm1	;    note:  the normal movq writes the
		  movq	mm2, [esi+16]	;    data to cache; a cache line will be
		  movq	mm3, [esi+24]	;    allocated as needed, to store the data
		  movq	[edi+16], mm2
		  movq	[edi+24], mm3
		  movq	mm0, [esi+32]
		  movq	mm1, [esi+40]
		  movq	[edi+32], mm0
		  movq	[edi+40], mm1
		  movq	mm2, [esi+48]
		  movq	mm3, [esi+56]
		  movq	[edi+48], mm2
		  movq	[edi+56], mm3

		  add		esi, 64			; update source pointer
		  add		edi, 64			; update destination pointer
		  dec		ecx				; count down
		  jnz		$memcpy_ic_1	; last 64-byte block?

	  $memcpy_ic_2:
		  mov		ecx, ebx		; has valid low 6 bits of the byte count
	  $memcpy_ic_3:
		  shr		ecx, 2			; dword count
		  and		ecx, 1111b		; only look at the "remainder" bits
		  neg		ecx				; set up to jump into the array
		  add		ecx, offset $memcpy_last_few
		  jmp		ecx				; jump to array of movsds

	  $memcpy_uc_test:
		  cmp		ecx, UNCACHED_COPY/64	; big enough? use block prefetch copy
		  jae		$memcpy_bp_1

	  $memcpy_64_test:
		  or		ecx, ecx		; tail end of block prefetch will jump here
		  jz		$memcpy_ic_2	; no more 64-byte blocks left

	  // For larger blocks, which will spill beyond the cache, it's faster to
	  // use the Streaming Store instruction MOVNTQ.   This write instruction
	  // bypasses the cache and writes straight to main memory.  This code also
	  // uses the software prefetch instruction to pre-read the data.
	  align 16
	  $memcpy_uc_1:				; 64-byte blocks, uncached copy

		  prefetchnta [esi + (200*64/34+192)]		; start reading ahead

		  movq	mm0,[esi+0]		; read 64 bits
		  add		edi,64			; update destination pointer
		  movq	mm1,[esi+8]
		  add		esi,64			; update source pointer
		  movq	mm2,[esi-48]
		  movntq	[edi-64], mm0	; write 64 bits, bypassing the cache
		  movq	mm0,[esi-40]	;    note: movntq also prevents the CPU
		  movntq	[edi-56], mm1	;    from READING the destination address
		  movq	mm1,[esi-32]	;    into the cache, only to be over-written
		  movntq	[edi-48], mm2	;    so that also helps performance
		  movq	mm2,[esi-24]
		  movntq	[edi-40], mm0
		  movq	mm0,[esi-16]
		  movntq	[edi-32], mm1
		  movq	mm1,[esi-8]
		  movntq	[edi-24], mm2
		  movntq	[edi-16], mm0
		  dec		ecx
		  movntq	[edi-8], mm1
		  jnz		$memcpy_uc_1	; last 64-byte block?

		  jmp		$memcpy_ic_2		; almost done

	  // For the largest size blocks, a special technique called Block Prefetch
	  // can be used to accelerate the read operations.   Block Prefetch reads
	  // one address per cache line, for a series of cache lines, in a short loop.
	  // This is faster than using software prefetch.  The technique is great for
	  // getting maximum read bandwidth, especially in DDR memory systems.
	  $memcpy_bp_1:			; large blocks, block prefetch copy

		  cmp		ecx, CACHEBLOCK			; big enough to run another prefetch loop?
		  jl		$memcpy_64_test			; no, back to regular uncached copy

		  mov		eax, CACHEBLOCK / 2		; block prefetch loop, unrolled 2X
		  add		esi, CACHEBLOCK * 64	; move to the top of the block
	  align 16
	  $memcpy_bp_2:
		  mov		edx, [esi-64]		; grab one address per cache line
		  mov		edx, [esi-128]		; grab one address per cache line
		  sub		esi, 128			; go reverse order to suppress HW prefetcher
		  dec		eax					; count down the cache lines
		  jnz		$memcpy_bp_2		; keep grabbing more lines into cache

		  mov		eax, CACHEBLOCK		; now that its in cache, do the copy
	  align 16
	  $memcpy_bp_3:
		  movq	mm0, [esi   ]		; read 64 bits
		  movq	mm1, [esi+ 8]
		  movq	mm2, [esi+16]
		  movq	mm3, [esi+24]
		  movq	mm4, [esi+32]
		  movq	mm5, [esi+40]
		  movq	mm6, [esi+48]
		  movq	mm7, [esi+56]
		  add		esi, 64				; update source pointer
		  movntq	[edi   ], mm0		; write 64 bits, bypassing cache
		  movntq	[edi+ 8], mm1		;    note: movntq also prevents the CPU
		  movntq	[edi+16], mm2		;    from READING the destination address
		  movntq	[edi+24], mm3		;    into the cache, only to be over-written,
		  movntq	[edi+32], mm4		;    so that also helps performance
		  movntq	[edi+40], mm5
		  movntq	[edi+48], mm6
		  movntq	[edi+56], mm7
		  add		edi, 64				; update dest pointer

		  dec		eax					; count down

		  jnz		$memcpy_bp_3		; keep copying
		  sub		ecx, CACHEBLOCK		; update the 64-byte block count
		  jmp		$memcpy_bp_1		; keep processing chunks

	  // The smallest copy uses the X86 "movsd" instruction, in an optimized
	  // form which is an "unrolled loop".   Then it handles the last few bytes.
	  align 4
		  movsd
		  movsd			; perform last 1-15 dword copies
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd			; perform last 1-7 dword copies
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd
		  movsd

	  $memcpy_last_few:		; dword aligned from before movsds
		  mov		ecx, ebx	; has valid low 2 bits of the byte count
		  and		ecx, 11b	; the last few cows must come home
		  jz		$memcpy_final	; no more, lets leave
		  rep		movsb		; the last 1, 2, or 3 bytes

	  $memcpy_final:
		  emms				; clean up the MMX state
		  sfence				; flush the write buffer
	  //	mov		eax, [dest]	; ret value = destination pointer
	  }
  }
  else
  {
    memcpy(Dst, Src, Count);
  }
}

inline void cryPrefetch(const void* Src, int nCount)
{
  nCount >>= 6;
  if (nCount > 0)
  {
    _asm
    {
      mov esi, Src;
      mov ecx, nCount;
mPr0:
      align 16
      dec ecx;
      mov eax, [esi];
      mov eax,0;
      lea esi, [esi+40h];
      jne mPr0;
    }
  }
  else
  {
    _asm
    {
      mov esi, Src;
      mov ecx, nCount;
mPr1:
      align 16
      inc ecx;
      mov eax, [esi];
      mov eax,0;
      lea esi, [esi-40h];
      jne mPr1;
    }
  }
}

inline void cryMemcpy (void* inDst, const void* inSrc, int nCount, int nFlags)
{
  int i;
  int cnt;
  INT_PTR nAlign16;
  unsigned char *Src = (unsigned char *)inSrc;
  unsigned char *Dst = (unsigned char *)inDst;
  if (nCount < 32 || (nCount < 128 && !(nFlags & 4)))
  {
    memcpy(Dst, Src, nCount);
    return;
  }
  if (nFlags & 1)
    nAlign16 = (INT_PTR)Src & 0xf;
  else
    nAlign16 = (INT_PTR)Dst & 0xf;
  if (nCount >= 4)
  {
    if (nAlign16 & 3)
    {
      cnt = (int)(4 - nAlign16);
      for (i=0; i<cnt; i++)
      {
        *Dst++ = *Src++;
      }
      nCount -= cnt;
    }
    if (nCount >= 16 && (nAlign16 & 0xc))
    {
      cnt = (int)(16 - nAlign16);
      for (i=0; i<(cnt>>2); i++)
      {
        *(UINT_PTR *)Dst = *(UINT_PTR *)Src;
        Dst += 4;
        Src += 4;
      }
      nCount -= cnt;
    }
  }
  int Count_64 = 0xffffffc0 & nCount;
  int Count_16 = 0x30 & nCount;
  int Count_12 = 0xc & nCount;
  int Count_4 = 0x3 & nCount;
  if (Count_64)
  {
    if (nFlags & 4)
    {
      while (Count_64 > 2048)
      {
        cryPrefetch(Src, 2048);
        if (g_CpuFlags & CPUF_SSE)
        {
          if ((INT_PTR)Src & 0xf)
          {
            if ((INT_PTR)Dst & 0xf)
            {
              _asm
              {
                mov ebx, Src;
                mov edx, Dst;
                mov ecx, 32;
  St0:
                movups      xmm0,xmmword ptr [ebx]
                movups      xmm1,xmmword ptr [ebx+10h]
                movups      xmm2,xmmword ptr [ebx+20h]
                movups      xmm3,xmmword ptr [ebx+30h]
                add         ebx,40h
                movups      xmmword ptr [edx],xmm0
                movups      xmmword ptr [edx+10h],xmm1
                movups      xmmword ptr [edx+20h],xmm2
                movups      xmmword ptr [edx+30h],xmm3
                add         edx,40h
                dec         ecx
                jne         St0
              }
            }
            else
            {
              _asm
              {
                mov ebx, Src;
                mov edx, Dst;
                mov ecx, 32;
  St1:
                align 16
                movups      xmm0,xmmword ptr [ebx]
                movups      xmm1,xmmword ptr [ebx+10h]
                movups      xmm2,xmmword ptr [ebx+20h]
                movups      xmm3,xmmword ptr [ebx+30h]
                add         ebx,40h
                movaps      xmmword ptr [edx],xmm0
                movaps      xmmword ptr [edx+10h],xmm1
                movaps      xmmword ptr [edx+20h],xmm2
                movaps      xmmword ptr [edx+30h],xmm3
                add         edx,40h
                dec         ecx
                jne         St1
              }
            }
          }
          else
          {
            if ((INT_PTR)Dst & 0xf)
            {
              _asm
              {
                mov ebx, Src;
                mov edx, Dst;
                mov ecx, 32;
  St2:
                movaps      xmm0,xmmword ptr [ebx]
                movaps      xmm1,xmmword ptr [ebx+10h]
                movaps      xmm2,xmmword ptr [ebx+20h]
                movaps      xmm3,xmmword ptr [ebx+30h]
                add         ebx,40h
                movups      xmmword ptr [edx],xmm0
                movups      xmmword ptr [edx+10h],xmm1
                movups      xmmword ptr [edx+20h],xmm2
                movups      xmmword ptr [edx+30h],xmm3
                add         edx,40h
                dec         ecx
                jne         St2
              }
            }
            else
            {
              _asm
              {
                mov ebx, Src;
                mov edx, Dst;
                mov ecx, 32;
  St3:
                align 16
                movaps      xmm0,xmmword ptr [ebx]
                movaps      xmm1,xmmword ptr [ebx+10h]
                movaps      xmm2,xmmword ptr [ebx+20h]
                movaps      xmm3,xmmword ptr [ebx+30h]
                add         ebx,40h
                movaps      xmmword ptr [edx],xmm0
                movaps      xmmword ptr [edx+10h],xmm1
                movaps      xmmword ptr [edx+20h],xmm2
                movaps      xmmword ptr [edx+30h],xmm3
                add         edx,40h
                dec         ecx
                jne         St3
              }
            }
          }
        }
        else
        if (g_CpuFlags & CPUF_MMX)
        {
          _asm
          {
            mov ebx, Src;
            mov edx, Dst;
            mov ecx, 32;
St4:
            align 16
            movq      mm0,xmmword ptr [ebx]
            movq      mm1,xmmword ptr [ebx+08h]
            movq      mm2,xmmword ptr [ebx+10h]
            movq      mm3,xmmword ptr [ebx+18h]
            movq      mm4,xmmword ptr [ebx+20h]
            movq      mm5,xmmword ptr [ebx+28h]
            movq      mm6,xmmword ptr [ebx+30h]
            movq      mm7,xmmword ptr [ebx+38h]
            add       ebx,40h
            movq      mmword ptr [edx],mm0
            movq      mmword ptr [edx+08h],mm1
            movq      mmword ptr [edx+10h],mm2
            movq      mmword ptr [edx+18h],mm3
            movq      mmword ptr [edx+20h],mm0
            movq      mmword ptr [edx+28h],mm1
            movq      mmword ptr [edx+30h],mm2
            movq      mmword ptr [edx+38h],mm3
            add       edx,40h
            dec       ecx
            jne       St4
            emms
          }
        }
        else
        {
          _asm
          {
            mov ebx, Src;
            mov edx, Dst;
            mov ecx, 32;
            shl ecx, 4
St5:
            align 16
            mov      eax,dword ptr [ebx]
            add      ebx,4h
            mov      dword ptr [edx],eax
            add      edx,4h
            dec      ecx
            jne      St5
          }
        }
        Src += 2048;
        Dst += 2048;
        Count_64 -= 2048;
      }
      if (Count_64 > 128)
      {
        cryPrefetch(Src, Count_64);
        cnt = Count_64>>6;

        if (g_CpuFlags & CPUF_SSE)
        {
          if ((INT_PTR)Src & 0xf)
          {
            if ((INT_PTR)Dst & 0xf)
            {
              _asm
              {
                mov ebx, Src;
                mov edx, Dst;
                mov ecx, cnt;
  St6:
                movups      xmm0,xmmword ptr [ebx]
                movups      xmm1,xmmword ptr [ebx+10h]
                movups      xmm2,xmmword ptr [ebx+20h]
                movups      xmm3,xmmword ptr [ebx+30h]
                add         ebx,40h
                movups      xmmword ptr [edx],xmm0
                movups      xmmword ptr [edx+10h],xmm1
                movups      xmmword ptr [edx+20h],xmm2
                movups      xmmword ptr [edx+30h],xmm3
                add         edx,40h
                dec         ecx
                jne         St6
              }
            }
            else
            {
              _asm
              {
                mov ebx, Src;
                mov edx, Dst;
                mov ecx, cnt;
  St7:
                align 16
                movups      xmm0,xmmword ptr [ebx]
                movups      xmm1,xmmword ptr [ebx+10h]
                movups      xmm2,xmmword ptr [ebx+20h]
                movups      xmm3,xmmword ptr [ebx+30h]
                add         ebx,40h
                movaps      xmmword ptr [edx],xmm0
                movaps      xmmword ptr [edx+10h],xmm1
                movaps      xmmword ptr [edx+20h],xmm2
                movaps      xmmword ptr [edx+30h],xmm3
                add         edx,40h
                dec         ecx
                jne         St7
              }
            }
          }
          else
          {
            if ((INT_PTR)Dst & 0xf)
            {
              _asm
              {
                mov ebx, Src;
                mov edx, Dst;
                mov ecx, cnt;
  St8:
                movaps      xmm0,xmmword ptr [ebx]
                movaps      xmm1,xmmword ptr [ebx+10h]
                movaps      xmm2,xmmword ptr [ebx+20h]
                movaps      xmm3,xmmword ptr [ebx+30h]
                add         ebx,40h
                movups      xmmword ptr [edx],xmm0
                movups      xmmword ptr [edx+10h],xmm1
                movups      xmmword ptr [edx+20h],xmm2
                movups      xmmword ptr [edx+30h],xmm3
                add         edx,40h
                dec         ecx
                jne         St8
              }
            }
            else
            {
              _asm
              {
                mov ebx, Src;
                mov edx, Dst;
                mov ecx, cnt;
  St9:
                align 16
                movaps      xmm0,xmmword ptr [ebx]
                movaps      xmm1,xmmword ptr [ebx+10h]
                movaps      xmm2,xmmword ptr [ebx+20h]
                movaps      xmm3,xmmword ptr [ebx+30h]
                add         ebx,40h
                movaps      xmmword ptr [edx],xmm0
                movaps      xmmword ptr [edx+10h],xmm1
                movaps      xmmword ptr [edx+20h],xmm2
                movaps      xmmword ptr [edx+30h],xmm3
                add         edx,40h
                dec         ecx
                jne         St9
              }
            }
          }
        }
        else
        if (g_CpuFlags & CPUF_MMX)
        {
          _asm
          {
            mov ebx, Src;
            mov edx, Dst;
            mov ecx, cnt;
St10:
            align 16
            movq      mm0,xmmword ptr [ebx]
            movq      mm1,xmmword ptr [ebx+08h]
            movq      mm2,xmmword ptr [ebx+10h]
            movq      mm3,xmmword ptr [ebx+18h]
            movq      mm4,xmmword ptr [ebx+20h]
            movq      mm5,xmmword ptr [ebx+28h]
            movq      mm6,xmmword ptr [ebx+30h]
            movq      mm7,xmmword ptr [ebx+38h]
            add       ebx,40h
            movq      mmword ptr [edx],mm0
            movq      mmword ptr [edx+08h],mm1
            movq      mmword ptr [edx+10h],mm2
            movq      mmword ptr [edx+18h],mm3
            movq      mmword ptr [edx+20h],mm0
            movq      mmword ptr [edx+28h],mm1
            movq      mmword ptr [edx+30h],mm2
            movq      mmword ptr [edx+38h],mm3
            add       edx,40h
            dec       ecx
            jne       St10
            emms
          }
        }
        else
        {
          _asm
          {
            mov ebx, Src;
            mov edx, Dst;
            mov ecx, cnt;
            shl ecx, 4
St11:
            align 16
            mov      eax,dword ptr [ebx]
            add      ebx,4h
            mov      dword ptr [edx],eax
            add      edx,4h
            dec      ecx
            jne      St11
          }
        }
        Src += Count_64;
        Dst += Count_64;
        Count_64 = 0;
      }
    }
    if (Count_64)
    {
      cnt = Count_64>>6;

      if (g_CpuFlags & CPUF_SSE)
      {
        if ((INT_PTR)Src & 0xf)
        {
          if ((INT_PTR)Dst & 0xf)
          {
            _asm
            {
              mov ebx, Src;
              mov edx, Dst;
              mov ecx, cnt;
St12:
              movups      xmm0,xmmword ptr [ebx]
              movups      xmm1,xmmword ptr [ebx+10h]
              movups      xmm2,xmmword ptr [ebx+20h]
              movups      xmm3,xmmword ptr [ebx+30h]
              add         ebx,40h
              movups      xmmword ptr [edx],xmm0
              movups      xmmword ptr [edx+10h],xmm1
              movups      xmmword ptr [edx+20h],xmm2
              movups      xmmword ptr [edx+30h],xmm3
              add         edx,40h
              dec         ecx
              jne         St12
            }
          }
          else
          {
            _asm
            {
              mov ebx, Src;
              mov edx, Dst;
              mov ecx, cnt;
St13:
              align 16
              movups      xmm0,xmmword ptr [ebx]
              movups      xmm1,xmmword ptr [ebx+10h]
              movups      xmm2,xmmword ptr [ebx+20h]
              movups      xmm3,xmmword ptr [ebx+30h]
              add         ebx,40h
              movaps      xmmword ptr [edx],xmm0
              movaps      xmmword ptr [edx+10h],xmm1
              movaps      xmmword ptr [edx+20h],xmm2
              movaps      xmmword ptr [edx+30h],xmm3
              add         edx,40h
              dec         ecx
              jne         St13
            }
          }
        }
        else
        {
          if ((INT_PTR)Dst & 0xf)
          {
            _asm
            {
              mov ebx, Src;
              mov edx, Dst;
              mov ecx, cnt;
St14:
              movaps      xmm0,xmmword ptr [ebx]
              movaps      xmm1,xmmword ptr [ebx+10h]
              movaps      xmm2,xmmword ptr [ebx+20h]
              movaps      xmm3,xmmword ptr [ebx+30h]
              add         ebx,40h
              movups      xmmword ptr [edx],xmm0
              movups      xmmword ptr [edx+10h],xmm1
              movups      xmmword ptr [edx+20h],xmm2
              movups      xmmword ptr [edx+30h],xmm3
              add         edx,40h
              dec         ecx
              jne         St14
            }
          }
          else
          {
            _asm
            {
              mov ebx, Src;
              mov edx, Dst;
              mov ecx, cnt;
St15:
              align 16
              movaps      xmm0,xmmword ptr [ebx]
              movaps      xmm1,xmmword ptr [ebx+10h]
              movaps      xmm2,xmmword ptr [ebx+20h]
              movaps      xmm3,xmmword ptr [ebx+30h]
              add         ebx,40h
              movaps      xmmword ptr [edx],xmm0
              movaps      xmmword ptr [edx+10h],xmm1
              movaps      xmmword ptr [edx+20h],xmm2
              movaps      xmmword ptr [edx+30h],xmm3
              add         edx,40h
              dec         ecx
              jne         St15
            }
          }
        }
      }
      else
      if (g_CpuFlags & CPUF_MMX)
      {
        _asm
        {
          mov ebx, Src;
          mov edx, Dst;
          mov ecx, cnt;
St16:
          align 16
          movq      mm0,xmmword ptr [ebx]
          movq      mm1,xmmword ptr [ebx+08h]
          movq      mm2,xmmword ptr [ebx+10h]
          movq      mm3,xmmword ptr [ebx+18h]
          movq      mm4,xmmword ptr [ebx+20h]
          movq      mm5,xmmword ptr [ebx+28h]
          movq      mm6,xmmword ptr [ebx+30h]
          movq      mm7,xmmword ptr [ebx+38h]
          add       ebx,40h
          movq      mmword ptr [edx],mm0
          movq      mmword ptr [edx+08h],mm1
          movq      mmword ptr [edx+10h],mm2
          movq      mmword ptr [edx+18h],mm3
          movq      mmword ptr [edx+20h],mm0
          movq      mmword ptr [edx+28h],mm1
          movq      mmword ptr [edx+30h],mm2
          movq      mmword ptr [edx+38h],mm3
          add       edx,40h
          dec       ecx
          jne       St16
          emms
        }
      }
      else
      {
        _asm
        {
          mov ebx, Src;
          mov edx, Dst;
          mov ecx, cnt;
          shl ecx, 4
St17:
          align 16
          mov      eax,dword ptr [ebx]
          add      ebx,4h
          mov      dword ptr [edx],eax
          add      edx,4h
          dec      ecx
          jne      St17
        }
      }
      Src += Count_64;
      Dst += Count_64;
    }
  }
  if (Count_16)
  {
    cnt = Count_16>>4;
    if (g_CpuFlags & CPUF_SSE)
    {
      if ((INT_PTR)Src & 0xf)
      {
        if ((INT_PTR)Dst & 0xf)
        {
          _asm
          {
            mov ebx, Src;
            mov edx, Dst;
            mov ecx, cnt;
St18:
            movups      xmm0,xmmword ptr [ebx]
            add         ebx,10h
            movups      xmmword ptr [edx],xmm0
            add         edx,10h
            dec         ecx
            jne         St18
          }
        }
        else
        {
          _asm
          {
            mov ebx, Src;
            mov edx, Dst;
            mov ecx, cnt;
St19:
            align 16
            movups      xmm0,xmmword ptr [ebx]
            add         ebx,10h
            movaps      xmmword ptr [edx],xmm0
            add         edx,10h
            dec         ecx
            jne         St19
          }
        }
      }
      else
      {
        if ((INT_PTR)Dst & 0xf)
        {
          _asm
          {
            mov ebx, Src;
            mov edx, Dst;
            mov ecx, cnt;
St20:
            movaps      xmm0,xmmword ptr [ebx]
            add         ebx,10h
            movups      xmmword ptr [edx],xmm0
            add         edx,10h
            dec         ecx
            jne         St20
          }
        }
        else
        {
          _asm
          {
            mov ebx, Src;
            mov edx, Dst;
            mov ecx, cnt;
St21:
            align 16
            movaps      xmm0,xmmword ptr [ebx]
            add         ebx,10h
            movaps      xmmword ptr [edx],xmm0
            add         edx,10h
            dec         ecx
            jne         St21
          }
        }
      }
    }
    else
    if (g_CpuFlags & CPUF_MMX)
    {
      _asm
      {
        mov ebx, Src;
        mov edx, Dst;
        mov ecx, cnt;
St22:
        align 16
        movq      mm0,xmmword ptr [ebx]
        movq      mm1,xmmword ptr [ebx+08h]
        add       ebx,10h
        movq      mmword ptr [edx],mm0
        movq      mmword ptr [edx+08h],mm1
        add       edx,10h
        dec       ecx
        jne       St22
        emms
      }
    }
    else
    {
      _asm
      {
        mov ebx, Src;
        mov edx, Dst;
        mov ecx, cnt;
        shl ecx, 2
St23:
        align 16
        mov      eax,dword ptr [ebx]
        add      ebx,4h
        mov      dword ptr [edx],eax
        add      edx,4h
        dec      ecx
        jne      St23
      }
    }
    Src += Count_16;
    Dst += Count_16;
  }
  if (Count_12)
  {
    _asm
    {
      mov ebx, Src;
      mov edx, Dst;
      mov ecx, Count_12;
      shr ecx, 2
St24:
      align 16
      mov      eax,dword ptr [ebx]
      add      ebx,4h
      mov      dword ptr [edx],eax
      add      edx,4h
      dec      ecx
      jne      St24
    }
    Src += Count_12;
    Dst += Count_12;
  }
  if (Count_4)
  {
    _asm
    {
      mov ebx, Src;
      mov edx, Dst;
      mov ecx, Count_4;
St25:
      align 16
      mov      al,byte ptr [ebx]
      add      ebx,4h
      mov      byte ptr [edx],al
      add      edx,4h
      dec      ecx
      jne      St25
    }
  }
}

//==============================================================================


// Matrix inversion using 3DNow! instructions set
// NOTE: On AMD Athlon - much faster then SSE version
inline bool invertMatrixf_3DNow(float *pOut, const float *pIn)
{
  _asm
  {
    mov         edx, pIn
    sub         esp,40h
    movq        mm1,mmword ptr [edx+10h]
    movq        mm2,mmword ptr [edx+20h]
    pswapd      mm5,mm1
    movq        mm3,mmword ptr [edx+30h]
    pswapd      mm4,mm2
    movq        mm0,mmword ptr [edx]
    pswapd      mm6,mm3
    pswapd      mm7,mm2
    pfmul       mm5,mm0
    pfmul       mm4,mm0
    pfmul       mm6,mm0
    pswapd      mm0,mm3
    pfmul       mm7,mm1
    pfnacc      mm5,mm4
    pfmul       mm1,mm0
    pfmul       mm2,mm0
    pfnacc      mm6,mm7
    pfnacc      mm1,mm2
    movq        mm0,mmword ptr [edx+38h]
    movq        mm2,mm6
    movq        mm4,mm1
    movq        mm7,mmword ptr [edx+28h]
    punpckhdq   mm2,mm2
    movq        mm3,mm1
    punpckhdq   mm4,mm4
    pfmul       mm2,mm0
    punpckldq   mm3,mm3
    pfmul       mm4,mmword ptr [edx+18h]
    pfmul       mm3,mm7
    pfadd       mm2,mm4
    movq        mm4,mm6
    pfsub       mm2,mm3
    movq        mm3,mm1
    punpckldq   mm4,mm4
    movq        mmword ptr [esp+8],mm2
    movq        mm2,mm5
    punpckhdq   mm3,mm3
    punpckhdq   mm2,mm2
    pfmul       mm3,mmword ptr [edx+8]
    pfmul       mm2,mm0
    pfmul       mm4,mm7
    pfsub       mm2,mm4
    pfadd       mm2,mm3
    movq        mmword ptr [esp+18h],mm2
    movq        mm2,mm5
    movq        mm3,mm6
    punpckldq   mm1,mm1
    punpckldq   mm2,mm2
    punpckldq   mm3,mm3
    pfmul       mm1,mmword ptr [edx+8]
    pfmul       mm0,mm2
    pfmul       mm3,mmword ptr [edx+18h]
    pfadd       mm0,mm1
    pfsub       mm0,mm3
    movq        mmword ptr [esp+28h],mm0
    punpckhdq   mm5,mm5
    punpckhdq   mm6,mm6
    pfmul       mm2,mm7
    pfmul       mm5,mmword ptr [edx+18h]
    movq        mm0,mmword ptr [edx+8]
    pfmul       mm6,mmword ptr [edx+8]
    pfsub       mm2,mm5
    pfadd       mm2,mm6
    movq        mmword ptr [esp+38h],mm2
    movq        mm1,mmword ptr [edx+18h]
    movq        mm2,mmword ptr [edx+28h]
    movq        mm3,mmword ptr [edx+38h]
    pswapd      mm5,mm1
    pswapd      mm4,mm2
    pfmul       mm5,mm0
    pswapd      mm6,mm3
    pfmul       mm4,mm0
    pswapd      mm7,mm2
    pfmul       mm6,mm0
    pswapd      mm0,mm3
    pfnacc      mm5,mm4
    pfmul       mm7,mm1
    pfmul       mm1,mm0
    pfmul       mm2,mm0
    pfnacc      mm6,mm7
    pfnacc      mm1,mm2
    movq        mm0,mmword ptr [edx+30h]
    movq        mm4,mm1
    movq        mm3,mm1
    movq        mm7,mmword ptr [edx+20h]
    movq        mm2,mm6
    punpckhdq   mm4,mm4
    punpckldq   mm3,mm3
    punpckhdq   mm2,mm2
    pfmul       mm4,mmword ptr [edx+10h]
    pfmul       mm2,mm0
    pfmul       mm3,mm7
    pfadd       mm2,mm4
    pfsub       mm2,mm3
    movq        mmword ptr [esp],mm2
    movq        mm3,mm1
    movq        mm4,mm6
    movq        mm2,mm5
    punpckhdq   mm3,mm3
    punpckldq   mm4,mm4
    punpckhdq   mm2,mm2
    pfmul       mm3,mmword ptr [edx]
    pfmul       mm2,mm0
    pfmul       mm4,mm7
    pfsub       mm2,mm4
    pfadd       mm2,mm3
    movq        mmword ptr [esp+10h],mm2
    movq        mm4,mm0
    movq        mm2,mm5
    movq        mm3,mm6
    punpckldq   mm1,mm1
    punpckldq   mm2,mm2
    punpckldq   mm3,mm3
    pfmul       mm1,mmword ptr [edx]
    pfmul       mm0,mm2
    pfmul       mm3,mmword ptr [edx+10h]
    pfadd       mm0,mm1
    pfsub       mm0,mm3
    movq        mmword ptr [esp+20h],mm0
    punpckhdq   mm5,mm5
    punpckhdq   mm6,mm6
    pfmul       mm2,mm7
    pfmul       mm5,mmword ptr [edx+10h]
    pfmul       mm6,mmword ptr [edx]
    pfsub       mm2,mm5
    pfadd       mm2,mm6
    movq        mmword ptr [esp+30h],mm2
    punpckhdq   mm0,mm2
    punpckldq   mm7,mm4
    movq        mm3,mmword ptr [edx]
    movq        mm1,mmword ptr [esp]
    punpckldq   mm3,dword ptr [edx+10h]
    punpckhdq   mm1,mmword ptr [esp+10h]
    pfmul       mm0,mm7
    pfmul       mm3,mm1
    pfnacc      mm0,mm3
    pfacc       mm0,mm0
    movq        mm4,mm0
    pfrcp       mm1,mm0
    pxor        mm3,mm3
    punpckldq   mm0,mm0
    pfcmpeq     mm3,mm4
    pfrcpit1    mm0,mm1
    pfrcpit2    mm0,mm1
    movd        eax,mm3
    pfmul       mm0,mmword ptr const3DN_1_N1
    test        eax,eax
    pswapd      mm1,mm0
    jne         m1
    mov         eax, pOut
    movq        mm2,mmword ptr [esp]
    movq        mm5,mmword ptr [esp+8]
    movq        mm3,mmword ptr [esp+10h]
    movq        mm6,mmword ptr [esp+18h]
    pfmul       mm2,mm1
    pfmul       mm3,mm0
    pfmul       mm5,mm1
    pfmul       mm6,mm0
    movq        mm4,mm2
    movq        mm7,mm5
    punpckhdq   mm2,mm3
    punpckldq   mm4,mm3
    punpckhdq   mm5,mm6
    punpckldq   mm7,mm6
    movq        mmword ptr [eax],mm2
    movq        mmword ptr [eax+10h],mm4
    movq        mmword ptr [eax+20h],mm5
    movq        mmword ptr [eax+30h],mm7
    movq        mm2,mmword ptr [esp+20h]
    movq        mm5,mmword ptr [esp+28h]
    movq        mm3,mmword ptr [esp+30h]
    movq        mm6,mmword ptr [esp+38h]
    pfmul       mm2,mm1
    pfmul       mm3,mm0
    pfmul       mm5,mm1
    pfmul       mm6,mm0
    movq        mm4,mm2
    movq        mm7,mm5
    punpckhdq   mm2,mm3
    punpckldq   mm4,mm3
    punpckhdq   mm5,mm6
    punpckldq   mm7,mm6
    movq        mmword ptr [eax+8],mm2
    movq        mmword ptr [eax+18h],mm4
    movq        mmword ptr [eax+28h],mm5
    movq        mmword ptr [eax+38h],mm7
m1:
    femms
    add         esp,40h
  }
  return true;
}

// Matrix multiplication using 3DNow! instructions set
// NOTE: On AMD Athlon - much faster then SSE version
inline void multMatrixf_3DNow(float *product, const float *m1, const float *m2)
{
  __asm
  {
    femms
    sub         esp,44h
    mov         edx, m1
    mov         ecx, m2
    mov         eax, product
    mov         dword ptr [esp+40h],ebp
    mov         ebp,esp
    and         esp,0FFFFFFF8h
    movq        mm0,mmword ptr [edx]
    movq        mm1,mmword ptr [edx+10h]
    movq        mm3,mmword ptr [edx+28h]
    movq        mm4,mmword ptr [edx+38h]
    movq        mm2,mm0
    punpckldq   mm0,mm1
    punpckhdq   mm2,mm1
    movq        mm5,mm3
    movq        mmword ptr [esp],mm0
    movq        mmword ptr [esp+10h],mm2
    movq        mm6,mmword ptr [edx+8]
    punpckldq   mm3,mm4
    movq        mm7,mmword ptr [edx+18h]
    movq        mm0,mmword ptr [edx+20h]
    punpckhdq   mm5,mm4
    movq        mm1,mm6
    movq        mmword ptr [esp+28h],mm3
    movq        mmword ptr [esp+38h],mm5
    punpckldq   mm6,mm7
    movq        mm4,mmword ptr [edx+30h]
    movq        mm2,mm0
    punpckhdq   mm1,mm7
    movq        mmword ptr [esp+20h],mm6
    punpckhdq   mm0,mm4
    punpckldq   mm2,mm4
    movq        mmword ptr [esp+30h],mm1
    movq        mmword ptr [esp+18h],mm0
    movq        mmword ptr [esp+8],mm2
    movq        mm0,mmword ptr [ecx]
    movq        mm1,mmword ptr [ecx+8]
    movq        mm2,mm0
    movq        mm3,mm1
    movq        mm4,mm0
    movq        mm5,mm1
    pfmul       mm0,mmword ptr [esp]
    pfmul       mm1,mmword ptr [esp+8]
    movq        mm6,mm2
    movq        mm7,mm3
    pfmul       mm2,mmword ptr [esp+10h]
    pfmul       mm3,mmword ptr [esp+18h]
    pfmul       mm4,mmword ptr [esp+20h]
    pfmul       mm5,mmword ptr [esp+28h]
    pfmul       mm6,mmword ptr [esp+30h]
    pfmul       mm7,mmword ptr [esp+38h]
    pfacc       mm0,mm2
    pfacc       mm1,mm3
    pfacc       mm4,mm6
    pfacc       mm5,mm7
    pfadd       mm0,mm1
    pfadd       mm4,mm5
    movq        mmword ptr [eax],mm0
    movq        mmword ptr [eax+8],mm4
    movq        mm0,mmword ptr [ecx+10h]
    movq        mm1,mmword ptr [ecx+18h]
    movq        mm2,mm0
    movq        mm3,mm1
    movq        mm4,mm0
    movq        mm5,mm1
    pfmul       mm0,mmword ptr [esp]
    pfmul       mm1,mmword ptr [esp+8]
    movq        mm6,mm2
    movq        mm7,mm3
    pfmul       mm2,mmword ptr [esp+10h]
    pfmul       mm3,mmword ptr [esp+18h]
    pfmul       mm4,mmword ptr [esp+20h]
    pfmul       mm5,mmword ptr [esp+28h]
    pfmul       mm6,mmword ptr [esp+30h]
    pfmul       mm7,mmword ptr [esp+38h]
    pfacc       mm0,mm2
    pfacc       mm1,mm3
    pfacc       mm4,mm6
    pfacc       mm5,mm7
    pfadd       mm0,mm1
    pfadd       mm4,mm5
    movq        mmword ptr [eax+10h],mm0
    movq        mmword ptr [eax+18h],mm4
    movq        mm0,mmword ptr [ecx+20h]
    movq        mm1,mmword ptr [ecx+28h]
    movq        mm2,mm0
    movq        mm3,mm1
    movq        mm4,mm0
    movq        mm5,mm1
    pfmul       mm0,mmword ptr [esp]
    pfmul       mm1,mmword ptr [esp+8]
    movq        mm6,mm2
    movq        mm7,mm3
    pfmul       mm2,mmword ptr [esp+10h]
    pfmul       mm3,mmword ptr [esp+18h]
    pfmul       mm4,mmword ptr [esp+20h]
    pfmul       mm5,mmword ptr [esp+28h]
    pfmul       mm6,mmword ptr [esp+30h]
    pfmul       mm7,mmword ptr [esp+38h]
    pfacc       mm0,mm2
    pfacc       mm1,mm3
    pfacc       mm4,mm6
    pfacc       mm5,mm7
    pfadd       mm0,mm1
    pfadd       mm4,mm5
    movq        mmword ptr [eax+20h],mm0
    movq        mmword ptr [eax+28h],mm4
    movq        mm0,mmword ptr [ecx+30h]
    movq        mm1,mmword ptr [ecx+38h]
    movq        mm2,mm0
    movq        mm3,mm1
    movq        mm4,mm0
    movq        mm5,mm1
    pfmul       mm0,mmword ptr [esp]
    pfmul       mm1,mmword ptr [esp+8]
    movq        mm6,mm2
    movq        mm7,mm3
    pfmul       mm2,mmword ptr [esp+10h]
    pfmul       mm3,mmword ptr [esp+18h]
    pfmul       mm4,mmword ptr [esp+20h]
    pfmul       mm5,mmword ptr [esp+28h]
    pfmul       mm6,mmword ptr [esp+30h]
    pfmul       mm7,mmword ptr [esp+38h]
    pfacc       mm0,mm2
    pfacc       mm1,mm3
    pfacc       mm4,mm6
    pfacc       mm5,mm7
    pfadd       mm0,mm1
    pfadd       mm4,mm5
    movq        mmword ptr [eax+30h],mm0
    movq        mmword ptr [eax+38h],mm4
    mov         esp,ebp
    mov         ebp,dword ptr [esp+40h]
    add         esp,44h
    femms
  }
}

// Matrix transposing using 3DNow! instructions set
// NOTE: On AMD Athlon - much faster then SSE version
inline void transposeMatrixf_3DNow(float *product, const float *m)
{
  __asm
  {
    mov         edx,m
    movq        mm0,mmword ptr [edx]
    movq        mm1,mmword ptr [edx+10h]
    movq        mm3,mmword ptr [edx+28h]
    movq        mm4,mmword ptr [edx+38h]
    movq        mm2,mm0
    mov         eax,product
    punpckldq   mm0,mm1
    punpckhdq   mm2,mm1
    movq        mm5,mm3
    movq        mmword ptr [eax],mm0
    movq        mmword ptr [eax+10h],mm2
    movq        mm6,mmword ptr [edx+8]
    punpckldq   mm3,mm4
    movq        mm7,mmword ptr [edx+18h]
    movq        mm0,mmword ptr [edx+20h]
    punpckhdq   mm5,mm4
    movq        mm1,mm6
    movq        mmword ptr [eax+28h],mm3
    movq        mmword ptr [eax+38h],mm5
    punpckldq   mm6,mm7
    movq        mm4,mmword ptr [edx+30h]
    movq        mm2,mm0
    punpckhdq   mm1,mm7
    movq        mmword ptr [eax+20h],mm6
    punpckhdq   mm0,mm4
    punpckldq   mm2,mm4
    movq        mmword ptr [eax+30h],mm1
    movq        mmword ptr [eax+18h],mm0
    movq        mmword ptr [eax+8],mm2
    femms
  }
}

// Matrix identity using 3DNow! instructions set
// NOTE: On AMD Athlon - much faster then SSE version
inline void indentityMatrixf_3DNow(float *product)
{
  __asm
  {
    movd        mm1,dword ptr const3DN_1_N1
    mov         eax,dword ptr product
    pxor        mm0,mm0
    movq        mm2,mm1
    movq        mmword ptr [eax],mm1
    movq        mmword ptr [eax+8],mm0
    movq        mmword ptr [eax+18h],mm0
    punpckhdq   mm2,mm2
    movq        mmword ptr [eax+20h],mm0
    movq        mmword ptr [eax+28h],mm1
    punpckldq   mm2,mm1
    movq        mmword ptr [eax+30h],mm0
    movq        mmword ptr [eax+10h],mm2
    movq        mmword ptr [eax+38h],mm2
    femms
  }
}

// Scale Matrix using 3DNow! instructions set
// NOTE: On AMD Athlon - much faster then SSE version
inline void scaleMatrixf_3DNow(float *product, float x, float y, float z)
{
  __asm
  {
    femms
    mov         eax, product
    movd        mm3, y
    movq        mm1,mmword ptr const3DN_0_1
    movd        mm2, x
    pxor        mm0,mm0
    psllq       mm3,20h
    movd        mm4, z
    movq        mmword ptr [eax],mm2
    movq        mmword ptr [eax+8],mm0
    movq        mmword ptr [eax+18h],mm0
    movq        mmword ptr [eax+20h],mm0
    movq        mmword ptr [eax+30h],mm0
    movq        mmword ptr [eax+10h],mm3
    movq        mmword ptr [eax+28h],mm4
    movq        mmword ptr [eax+38h],mm1
    femms
  }
}

// Transform position using 3DNow! instructions set
// NOTE: On AMD Athlon - much faster then SSE version
inline void transformVec3f_3DNow(float *result, float *inV, float *matrix)
{
  _asm
  {
    femms
    mov         eax,inV
    mov         edx,matrix
    mov         ecx,result
    movq        mm0,mmword ptr [eax]
    movq        mm1,mm0
    punpckldq   mm0,mm0
    punpckhdq   mm1,mm1
    movd        mm2,dword ptr [eax+8]
    punpckldq   mm2,mm2
    movq        mm3,mm0
    pfmul       mm0,mmword ptr [edx]
    movq        mm4,mm1
    pfmul       mm1,mmword ptr [edx+10h]
    movq        mm5,mm2
    pfmul       mm2,mmword ptr [edx+20h]
    pfadd       mm0,mmword ptr [edx+30h]
    pfmul       mm3,mmword ptr [edx+8]
    pfadd       mm1,mm2
    pfmul       mm4,mmword ptr [edx+18h]
    pfmul       mm5,mmword ptr [edx+28h]
    pfadd       mm3,mmword ptr [edx+38h]
    pfadd       mm0,mm1
    pfadd       mm4,mm5
    pfadd       mm3,mm4
    movq        mmword ptr [ecx],mm0
    movq        mmword ptr [ecx+8],mm3
    femms
  }
}

// Transform normal using 3DNow! instructions set
// NOTE: On AMD Athlon - much faster then SSE version
inline void transformVec3Nf_3DNow(float *result, float *inV, float *matrix)
{
  _asm
  {
    femms
    mov         eax,inV
    mov         edx,matrix
    mov         ecx,result
    movq        mm0,mmword ptr [eax]
    movq        mm1,mm0
    punpckldq   mm0,mm0
    punpckhdq   mm1,mm1
    movd        mm2,dword ptr [eax+8]
    punpckldq   mm2,mm2
    movq        mm3,mm0
    pfmul       mm0,mmword ptr [edx]
    movq        mm4,mm1
    pfmul       mm1,mmword ptr [edx+10h]
    movq        mm5,mm2
    pfmul       mm2,mmword ptr [edx+20h]
    pfmul       mm3,mmword ptr [edx+8]
    pfadd       mm1,mm2
    pfmul       mm4,mmword ptr [edx+18h]
    pfmul       mm5,mmword ptr [edx+28h]
    pfadd       mm0,mm1
    pfadd       mm4,mm5
    pfadd       mm3,mm4
    movq        mmword ptr [ecx],mm0
    movd        dword ptr [ecx+8],mm3
    femms
  }
}

//==========================================================================================
// SSE optimizations

// Matrix inversion using SSE instructions set
inline bool invertMatrixf_SSE(float *pOut, const float *pIn)
{
  _asm
  {
    mov         ebx, esp
    and         esp,0FFFFFFF0h
    sub         esp,90h
    mov         eax, pIn
    movaps      xmm2,xmmword ptr [esp+30h]
    movlps      xmm2,qword ptr [eax]
    movaps      xmm1,xmmword ptr [esp+70h]
    lea         ecx,[eax+10h]
    movhps      xmm2,qword ptr [ecx]
    lea         edx,[eax+20h]
    movlps      xmm1,qword ptr [edx]
    movaps      xmm0,xmm2
    lea         ecx,[eax+30h]
    movhps      xmm1,qword ptr [ecx]
    shufps      xmm0,xmm1,88h
    shufps      xmm1,xmm2,0DDh
    lea         edx,[eax+8]
    movlps      xmm2,qword ptr [edx]
    movaps      xmm3,xmm2
    movaps      xmm2,xmmword ptr [esp+70h]
    lea         ecx,[eax+18h]
    movhps      xmm3,qword ptr [ecx]
    movaps      xmm5,xmm3
    lea         ecx,[eax+38h]
    add         eax,28h
    movlps      xmm2,qword ptr [eax]
    movhps      xmm2,qword ptr [ecx]
    shufps      xmm5,xmm2,88h
    shufps      xmm2,xmm3,0DDh
    movaps      xmm3,xmm5
    mulps       xmm3,xmm2
    shufps      xmm3,xmm3,0B1h
    movaps      xmm4,xmm1
    mulps       xmm4,xmm3
    movaps      xmmword ptr [esp+50h],xmm4
    movaps      xmm4,xmm3
    shufps      xmm4,xmm3,4Eh
    movaps      xmm6,xmm0
    mulps       xmm6,xmm3
    movaps      xmm3,xmm1
    mulps       xmm3,xmm4
    subps       xmm3,xmmword ptr [esp+50h]
    movaps      xmm7,xmm0
    mulps       xmm7,xmm4
    movaps      xmm4,xmm7
    subps       xmm4,xmm6
    movaps      xmmword ptr [esp+10h],xmm4
    movaps      xmm6,xmm1
    mulps       xmm6,xmm5
    shufps      xmm6,xmm6,0B1h
    movaps      xmm7,xmm0
    mulps       xmm7,xmm6
    movaps      xmmword ptr [esp+40h],xmm7
    movaps      xmm4,xmm2
    mulps       xmm4,xmm6
    shufps      xmm6,xmm6,4Eh
    movaps      xmm7,xmm2
    mulps       xmm7,xmm6
    movaps      xmmword ptr [esp+60h],xmm7
    movaps      xmm7,xmm0
    mulps       xmm7,xmm6
    movaps      xmm6,xmm7
    subps       xmm6,xmmword ptr [esp+40h]
    movaps      xmmword ptr [esp+40h],xmm6
    movaps      xmm6,xmm1
    shufps      xmm6,xmm1,4Eh
    mulps       xmm6,xmm2
    shufps      xmm6,xmm6,0B1h
    shufps      xmm5,xmm5,4Eh
    movaps      xmm7,xmm5
    mulps       xmm7,xmm6
    movaps      xmmword ptr [esp+20h],xmm7
    addps       xmm4,xmm3
    subps       xmm4,xmmword ptr [esp+60h]
    movaps      xmm3,xmmword ptr [esp+20h]
    movaps      xmm7,xmm0
    mulps       xmm7,xmm6
    movaps      xmmword ptr [esp],xmm7
    shufps      xmm6,xmm6,4Eh
    movaps      xmm7,xmm5
    mulps       xmm7,xmm6
    addps       xmm3,xmm4
    movaps      xmmword ptr [esp+30h],xmm6
    subps       xmm3,xmm7
    movaps      xmmword ptr [esp+50h],xmm3
    movaps      xmm7,xmm2
    movaps      xmm3,xmm0
    mulps       xmm3,xmmword ptr [esp+30h]
    subps       xmm3,xmmword ptr [esp]
    movaps      xmmword ptr [esp],xmm3
    movaps      xmm3,xmm2
    movaps      xmm6,xmm0
    mulps       xmm6,xmm1
    shufps      xmm6,xmm6,0B1h
    mulps       xmm3,xmm6
    movaps      xmm4,xmm5
    mulps       xmm4,xmm6
    shufps      xmm6,xmm6,4Eh
    mulps       xmm7,xmm6
    movaps      xmmword ptr [esp+20h],xmm7
    movaps      xmm7,xmm5
    mulps       xmm7,xmm6
    movaps      xmmword ptr [esp+70h],xmm7
    movaps      xmm6,xmm0
    mulps       xmm6,xmm2
    shufps      xmm6,xmm6,0B1h
    movaps      xmm7,xmm5
    mulps       xmm7,xmm6
    movaps      xmmword ptr [esp+30h],xmm7
    movaps      xmm7,xmm1
    mulps       xmm7,xmm6
    shufps      xmm6,xmm6,4Eh
    movaps      xmmword ptr [esp+60h],xmm7
    movaps      xmm7,xmm5
    mulps       xmm7,xmm6
    movaps      xmmword ptr [esp+80h],xmm7
    movaps      xmm7,xmm1
    mulps       xmm7,xmm6
    movaps      xmm6,xmmword ptr [esp]
    shufps      xmm6,xmmword ptr [esp],4Eh
    addps       xmm3,xmm6
    movaps      xmm6,xmm3
    movaps      xmm3,xmmword ptr [esp+20h]
    subps       xmm3,xmm6
    movaps      xmm6,xmm3
    movaps      xmm3,xmmword ptr [esp+60h]
    addps       xmm3,xmm6
    subps       xmm3,xmm7
    movaps      xmmword ptr [esp],xmm3
    movaps      xmm3,xmm0
    mulps       xmm0,xmmword ptr [esp+50h]
    mulps       xmm3,xmm5
    movaps      xmm5,xmm3
    shufps      xmm5,xmm3,0B1h
    movaps      xmm3,xmm2
    movaps      xmm6,xmm1
    mulps       xmm3,xmm5
    mulps       xmm6,xmm5
    shufps      xmm5,xmm5,4Eh
    mulps       xmm2,xmm5
    movaps      xmmword ptr [esp+20h],xmm6
    movaps      xmm6,xmm2
    movaps      xmm2,xmmword ptr [esp+10h]
    shufps      xmm2,xmmword ptr [esp+10h],4Eh
    subps       xmm2,xmmword ptr [esp+30h]
    movaps      xmm7,xmm2
    movaps      xmm2,xmmword ptr [esp+80h]
    addps       xmm2,xmm7
    addps       xmm3,xmm2
    movaps      xmm2,xmmword ptr [esp+40h]
    shufps      xmm2,xmmword ptr [esp+40h],4Eh
    subps       xmm4,xmm2
    subps       xmm4,xmmword ptr [esp+70h]
    subps       xmm4,xmmword ptr [esp+20h]
    movaps      xmm2,xmm0
    shufps      xmm2,xmm0,4Eh
    addps       xmm2,xmm0
    movaps      xmm0,xmm2
    shufps      xmm0,xmm2,0B1h
    mulps       xmm1,xmm5
    addss       xmm0,xmm2
    subps       xmm3,xmm6
    addps       xmm1,xmm4
    xorps       xmm2,xmm2
    xor         eax,eax
    inc         eax
    xor         ecx,ecx
    comiss      xmm0,xmm2
    cmove       ecx,eax
    test        ecx,ecx
    je          m1
    xor         eax,eax
    jmp         mEnd
m1:
    mov         eax, pOut
    movaps      xmmword ptr [esp+10h],xmm0
    rcpss       xmm2,xmm0
    movss       dword ptr [esp+10h],xmm2
    movaps      xmm2,xmmword ptr [esp+10h]
    movaps      xmm4,xmm2
    mulss       xmm4,xmm2
    mulss       xmm0,xmm4
    movaps      xmm4,xmm0
    movaps      xmm0,xmm2
    addss       xmm0,xmm2
    subss       xmm0,xmm4
    shufps      xmm0,xmm0,0
    movaps      xmm2,xmm0
    mulps       xmm2,xmmword ptr [esp+50h]
    movlps      qword ptr [eax],xmm2
    lea         ecx,[eax+8]
    movhps      qword ptr [ecx],xmm2
    movaps      xmm2,xmm0
    mulps       xmm2,xmm3
    lea         ecx,[eax+10h]
    movlps      qword ptr [ecx],xmm2
    lea         ecx,[eax+18h]
    movhps      qword ptr [ecx],xmm2
    movaps      xmm2,xmm0
    mulps       xmm2,xmmword ptr [esp]
    lea         ecx,[eax+20h]
    movlps      qword ptr [ecx],xmm2
    lea         ecx,[eax+28h]
    movhps      qword ptr [ecx],xmm2
    lea         ecx,[eax+30h]
    mulps       xmm0,xmm1
    movlps      qword ptr [ecx],xmm0
    lea         ecx,[eax+38h]
    movhps      qword ptr [ecx],xmm0
mEnd:
    mov         esp, ebx
  }
  return true;
}

// Matrix multiplication using SSE instructions set
// IMPORTANT NOTE: much faster if matrices m1 and product is 16 bytes aligned
inline void multMatrixf_SSE(float *product, const float *m1, const float *m2)
{
  __asm
  {
    mov         eax, m2;
    mov         ecx, m1;
    mov         edx, product;
    test        dl,0Fh
    jne         lNonAligned
    test        cl,0Fh
    jne         lNonAligned
    movss       xmm0,dword ptr [eax]
    movaps      xmm1,xmmword ptr [ecx]
    shufps      xmm0,xmm0,0
    movss       xmm2,dword ptr [eax+4]
    mulps       xmm0,xmm1
    shufps      xmm2,xmm2,0
    movaps      xmm3,xmmword ptr [ecx+10h]
    movss       xmm4,dword ptr [eax+8]
    mulps       xmm2,xmm3
    shufps      xmm4,xmm4,0
    addps       xmm0,xmm2
    movaps      xmm2,xmmword ptr [ecx+20h]
    movss       xmm5,dword ptr [eax+0Ch]
    mulps       xmm4,xmm2
    shufps      xmm5,xmm5,0
    movaps      xmm6,xmmword ptr [ecx+30h]
    mulps       xmm5,xmm6
    addps       xmm4,xmm5
    addps       xmm0,xmm4
    movaps      xmmword ptr [edx],xmm0
    movss       xmm0,dword ptr [eax+10h]
    movss       xmm4,dword ptr [eax+14h]
    shufps      xmm0,xmm0,0
    shufps      xmm4,xmm4,0
    mulps       xmm0,xmm1
    mulps       xmm4,xmm3
    movss       xmm5,dword ptr [eax+18h]
    addps       xmm0,xmm4
    shufps      xmm5,xmm5,0
    movss       xmm4,dword ptr [eax+1Ch]
    mulps       xmm5,xmm2
    shufps      xmm4,xmm4,0
    mulps       xmm4,xmm6
    addps       xmm5,xmm4
    addps       xmm0,xmm5
    movaps      xmmword ptr [edx+10h],xmm0
    movss       xmm0,dword ptr [eax+20h]
    movss       xmm4,dword ptr [eax+24h]
    shufps      xmm0,xmm0,0
    shufps      xmm4,xmm4,0
    mulps       xmm0,xmm1
    mulps       xmm4,xmm3
    movss       xmm5,dword ptr [eax+28h]
    addps       xmm0,xmm4
    shufps      xmm5,xmm5,0
    movss       xmm4,dword ptr [eax+2Ch]
    mulps       xmm5,xmm2
    shufps      xmm4,xmm4,0
    mulps       xmm4,xmm6
    addps       xmm5,xmm4
    addps       xmm0,xmm5
    movaps      xmmword ptr [edx+20h],xmm0
    movss       xmm0,dword ptr [eax+30h]
    movss       xmm4,dword ptr [eax+34h]
    shufps      xmm0,xmm0,0
    shufps      xmm4,xmm4,0
    mulps       xmm0,xmm1
    mulps       xmm4,xmm3
    movss       xmm1,dword ptr [eax+38h]
    addps       xmm0,xmm4
    shufps      xmm1,xmm1,0
    movss       xmm3,dword ptr [eax+3Ch]
    mulps       xmm1,xmm2
    shufps      xmm3,xmm3,0
    mulps       xmm3,xmm6
    addps       xmm1,xmm3
    addps       xmm0,xmm1
    movaps      xmmword ptr [edx+30h],xmm0
    jmp         lEnd
lNonAligned:
    movlps      xmm0,qword ptr [ecx]
    movhps      xmm0,qword ptr [ecx+8]
    movlps      xmm1,qword ptr [ecx+10h]
    movhps      xmm1,qword ptr [ecx+18h]
    movlps      xmm2,qword ptr [ecx+20h]
    movhps      xmm2,qword ptr [ecx+28h]
    movlps      xmm3,qword ptr [ecx+30h]
    movhps      xmm3,qword ptr [ecx+38h]
    movss       xmm4,dword ptr [eax]
    movss       xmm5,dword ptr [eax+4]
    movss       xmm6,dword ptr [eax+8]
    movss       xmm7,dword ptr [eax+0Ch]
    shufps      xmm4,xmm4,0
    shufps      xmm5,xmm5,0
    shufps      xmm6,xmm6,0
    shufps      xmm7,xmm7,0
    mulps       xmm4,xmm0
    mulps       xmm5,xmm1
    mulps       xmm6,xmm2
    mulps       xmm7,xmm3
    addps       xmm4,xmm5
    addps       xmm6,xmm7
    addps       xmm4,xmm6
    movss       xmm5,dword ptr [eax+10h]
    movss       xmm6,dword ptr [eax+14h]
    movss       xmm7,dword ptr [eax+18h]
    shufps      xmm5,xmm5,0
    shufps      xmm6,xmm6,0
    shufps      xmm7,xmm7,0
    mulps       xmm5,xmm0
    mulps       xmm6,xmm1
    mulps       xmm7,xmm2
    addps       xmm5,xmm6
    addps       xmm5,xmm7
    movss       xmm6,dword ptr [eax+1Ch]
    shufps      xmm6,xmm6,0
    mulps       xmm6,xmm3
    addps       xmm5,xmm6
    movss       xmm6,dword ptr [eax+20h]
    movss       xmm7,dword ptr [eax+24h]
    shufps      xmm6,xmm6,0
    shufps      xmm7,xmm7,0
    mulps       xmm6,xmm0
    mulps       xmm7,xmm1
    addps       xmm6,xmm7
    movss       xmm7,dword ptr [eax+28h]
    shufps      xmm7,xmm7,0
    mulps       xmm7,xmm2
    addps       xmm6,xmm7
    movss       xmm7,dword ptr [eax+2Ch]
    shufps      xmm7,xmm7,0
    mulps       xmm7,xmm3
    addps       xmm6,xmm7
    movss       xmm7,dword ptr [eax+30h]
    shufps      xmm7,xmm7,0
    mulps       xmm0,xmm7
    movss       xmm7,dword ptr [eax+34h]
    shufps      xmm7,xmm7,0
    mulps       xmm1,xmm7
    movss       xmm7,dword ptr [eax+38h]
    shufps      xmm7,xmm7,0
    mulps       xmm2,xmm7
    movss       xmm7,dword ptr [eax+3Ch]
    shufps      xmm7,xmm7,0
    mulps       xmm3,xmm7
    movlps      qword ptr [edx],xmm4
    movhps      qword ptr [edx+8],xmm4
    addps       xmm0,xmm1
    movlps      qword ptr [edx+10h],xmm5
    movhps      qword ptr [edx+18h],xmm5
    addps       xmm2,xmm3
    movlps      qword ptr [edx+20h],xmm6
    movhps      qword ptr [edx+28h],xmm6
    addps       xmm0,xmm2
    movlps      qword ptr [edx+30h],xmm0
    movhps      qword ptr [edx+38h],xmm0
lEnd:
  }
}

// Transform position using SSE instructions set
// IMPORTANT NOTE: much faster if matrix is 16 bytes aligned
inline void transformVec3f_SSE(float *result, float *inV, float *matrix)
{
  _asm
  {
    and         esp,0FFFFFFF0h
    sub         esp,1Ch
    mov         ecx,inV
    mov         eax,matrix
    test        al,0Fh
    movaps      xmm0,xmmword ptr [esp+0Ch]
    movlps      xmm0,qword ptr [ecx]
    mov         ecx,dword ptr [ecx+8]
    push        esi
    jne         lNonAligned
    movaps      xmm2,xmmword ptr [eax+30h]
    movaps      xmm3,xmmword ptr [eax+20h]
    mov         dword ptr [esp+0Ch],ecx
    movss       xmm1,dword ptr [esp+0Ch]
    shufps      xmm1,xmm1,0
    mulps       xmm3,xmm1
    addps       xmm3,xmm2
    movaps      xmm2,xmmword ptr [eax+10h]
    movaps      xmm1,xmm0
    shufps      xmm1,xmm0,55h
    mulps       xmm2,xmm1
    movaps      xmm1,xmm0
    shufps      xmm1,xmm0,0
    movaps      xmm0,xmmword ptr [eax]
    jmp         lEnd
lNonAligned:
    movaps      xmm1,xmmword ptr [esp+10h]
    movaps      xmm3,xmmword ptr [esp+10h]
    lea         edx,[eax+38h]
    lea         esi,[eax+30h]
    movlps      xmm1,qword ptr [esi]
    movhps      xmm1,qword ptr [edx]
    movaps      xmm2,xmm1
    mov         dword ptr [esp+0Ch],ecx
    movss       xmm1,dword ptr [esp+0Ch]
    shufps      xmm1,xmm1,0
    lea         edx,[eax+20h]
    movlps      xmm3,qword ptr [edx]
    lea         ecx,[eax+28h]
    movhps      xmm3,qword ptr [ecx]
    mulps       xmm3,xmm1
    addps       xmm3,xmm2
    movaps      xmm2,xmmword ptr [esp+10h]
    movaps      xmm1,xmm0
    shufps      xmm1,xmm0,55h
    lea         ecx,[eax+18h]
    lea         edx,[eax+10h]
    movlps      xmm2,qword ptr [edx]
    movhps      xmm2,qword ptr [ecx]
    mulps       xmm2,xmm1
    movaps      xmm1,xmm0
    shufps      xmm1,xmm0,0
    movaps      xmm0,xmmword ptr [esp+10h]
    movlps      xmm0,qword ptr [eax]
    lea         ecx,[eax+8]
    movhps      xmm0,qword ptr [ecx]
lEnd:
    mov         eax, result
    mulps       xmm0,xmm1
    addps       xmm0,xmm2
    addps       xmm0,xmm3
    movups      xmmword ptr [eax],xmm0
  }
}

// Transform normal using SSE instructions set
// IMPORTANT NOTE: much faster if matrix is 16 bytes aligned
inline void transformVec3Nf_SSE(float *result, float *inV, float *matrix)
{
  _asm
  {
    and         esp,0FFFFFFF0h
    sub         esp,20h
    mov         eax, matrix
    test        al,0Fh
    mov         ecx, inV
    jne         lNonAligned
    movaps      xmm1,xmmword ptr [esp+10h]
    movlps      xmm1,qword ptr [ecx]
    mov         ecx,dword ptr [ecx+8]
    movaps      xmm2,xmmword ptr [eax+20h]
    mov         dword ptr [esp+0Ch],ecx
    movss       xmm0,dword ptr [esp+0Ch]
    shufps      xmm0,xmm0,0
    mulps       xmm0,xmm2
    movaps      xmm2,xmm0
    movaps      xmm0,xmmword ptr [eax+10h]
    movaps      xmm3,xmm1
    shufps      xmm3,xmm1,55h
    mulps       xmm3,xmm0
    movaps      xmm0,xmmword ptr [eax]
    movaps      xmm4,xmm1
    shufps      xmm4,xmm1,0
    mulps       xmm4,xmm0
    movaps      xmm0,xmm4
    jmp         lEnd
lNonAligned:
    movaps      xmm0,xmmword ptr [esp+10h]
    movlps      xmm0,qword ptr [ecx]
    mov         ecx,dword ptr [ecx+8]
    movaps      xmm2,xmmword ptr [esp+10h]
    movaps      xmm3,xmmword ptr [esp+10h]
    mov         dword ptr [esp+0Ch],ecx
    movss       xmm1,dword ptr [esp+0Ch]
    shufps      xmm1,xmm1,0
    lea         edx,[eax+20h]
    movlps      xmm2,qword ptr [edx]
    lea         ecx,[eax+28h]
    movhps      xmm2,qword ptr [ecx]
    mulps       xmm2,xmm1
    movaps      xmm1,xmm0
    shufps      xmm1,xmm0,55h
    lea         ecx,[eax+18h]
    lea         edx,[eax+10h]
    movlps      xmm3,qword ptr [edx]
    movhps      xmm3,qword ptr [ecx]
    mulps       xmm3,xmm1
    movaps      xmm1,xmm0
    shufps      xmm1,xmm0,0
    movaps      xmm0,xmmword ptr [esp+10h]
    movlps      xmm0,qword ptr [eax]
    lea         ecx,[eax+8]
    movhps      xmm0,qword ptr [ecx]
    mulps       xmm0,xmm1
lEnd:
    mov         eax, result
    addps       xmm0,xmm3
    addps       xmm0,xmm2
    movaps      xmm1,xmm0
    lea         ecx,[eax+8]
    movhlps     xmm1,xmm0
    movlps      qword ptr [eax],xmm0
    movss       dword ptr [ecx],xmm1
    mov         esp,ebp
    pop         ebp
    ret         0Ch
  }
}
#else

const int PREFNTA_BLOCK = 0x4000;

ILINE void cryMemcpy(void* Dst, const void* Src, int n) {
	char* dst=(char*)Dst;
	char* src=(char*)Src;
	while (n > PREFNTA_BLOCK) 
	{
#if !defined(LINUX)
		for (int p = 0; p < PREFNTA_BLOCK; p+=64) { _mm_prefetch((char *) src + p, _MM_HINT_NTA); }
#endif
		memcpy(dst, src, PREFNTA_BLOCK);
		src += PREFNTA_BLOCK;
		dst += PREFNTA_BLOCK;
		n -= PREFNTA_BLOCK;
	}
#if !defined(LINUX)
	for (int p = 0; p < n; p+=64) { _mm_prefetch((char *) src + p, _MM_HINT_NTA); }
#endif
	memcpy(dst, src, n);
}

ILINE void cryMemcpy( void* Dst, const void* Src, INT n, int nFlags )
{
	char* dst=(char*)Dst;
	char* src=(char*)Src;
	while (n > PREFNTA_BLOCK) 
	{
#if !defined(LINUX)
		for (int p = 0; p < PREFNTA_BLOCK; p+=64) { _mm_prefetch((char *) src + p, _MM_HINT_NTA); }
#endif
		memcpy(dst, src, PREFNTA_BLOCK);
		src += PREFNTA_BLOCK;
		dst += PREFNTA_BLOCK;
		n -= PREFNTA_BLOCK;
	}
#if !defined(LINUX)
	for (int p = 0; p < n; p+=64) { _mm_prefetch((char *) src + p, _MM_HINT_NTA); }
#endif
	memcpy(dst, src, n);
}
#endif

inline void mathTransformVec3fN(float *pOut, float *pIn, float *matrix, int nV, int OptFlags)
{
#if defined _CPU_X86 && !defined(LINUX)
	// TODO: AMD64 port: NEED TO IMPLEMENT!!!
	int i;
  if (OptFlags & CPUF_3DNOW)
  {
    for (i=0; i<nV; i++)
    {
      transformVec3Nf_3DNow(pOut, pIn, matrix);
      pIn += 3;
      pOut += 3;
    }
  }
  else
  if (OptFlags & CPUF_SSE)
  {
    for (i=0; i<nV; i++)
    {
      transformVec3Nf_SSE(pOut, pIn, matrix);
      pIn += 3;
      pOut += 3;
    }
  }
  else
  {
    for (i=0; i<nV; i++)
    {
      transformVec3Nf_SSE(pOut, pIn, matrix);
      pIn += 3;
      pOut += 3;
    }
  }
#endif
}

inline void mathTransformVec3f(float *pOut, float *pIn, float *matrix, int nV, int OptFlags)
{
#if defined(_CPU_X86) && !defined(LINUX)
	int i;
	// TODO: AMD64 port: NEED TO IMPLEMENT!!!
  if (OptFlags & CPUF_3DNOW)
  {
    for (i=0; i<nV; i++)
    {
      transformVec3f_3DNow(pOut, pIn, matrix);
      pIn += 3;
      pOut += 3;
    }
  }
  else
  if (OptFlags & CPUF_SSE)
  {
    for (i=0; i<nV; i++)
    {
      transformVec3f_SSE(pOut, pIn, matrix);
      pIn += 3;
      pOut += 3;
    }
  }
  else
  {
    for (i=0; i<nV; i++)
    {
      transformVec3f_SSE(pOut, pIn, matrix);
      pIn += 3;
      pOut += 3;
    }
  }
#endif
}

inline void mathMatrixInverse(float *pOut, float *pIn, int OptFlags)
{
#if defined(_CPU_X86) && !defined(LINUX)
	// TODO: AMD64 port: NEED TO IMPLEMENT!!!
  if (OptFlags & CPUF_3DNOW)
    invertMatrixf_3DNow(pOut, pIn);
  else
  if (OptFlags & CPUF_SSE)
    invertMatrixf_SSE(pOut, pIn);
  else
#endif
    QQinvertMatrixf(pOut, pIn);
}

inline void mathMatrixMultiply(float *pOut, float *pM1, float *pM2, int OptFlags)
{
#if defined _CPU_X86 && !defined(LINUX)
  if (OptFlags & CPUF_3DNOW)
    multMatrixf_3DNow(pOut, pM1, pM2);
  else
  if (OptFlags & CPUF_SSE)
    multMatrixf_SSE(pOut, pM1, pM2);
  else
#endif
    multMatrixf(pOut, pM1, pM2);
}

inline void mathMatrixTranspose(float *pOut, float *pIn, int OptFlags)
{
#if defined _CPU_X86 && !defined(LINUX)
	// TODO: AMD64 port: NEED TO IMPLEMENT!!!
  if (OptFlags & CPUF_3DNOW)
    transposeMatrixf_3DNow(pOut, pIn);
  else
#endif
	{
    if (pOut == pIn)
    {
      Exchange(pOut[1], pOut[4]);
      Exchange(pOut[2], pOut[8]);
      Exchange(pOut[3], pOut[12]);

      Exchange(pOut[6], pOut[9]);
      Exchange(pOut[7], pOut[13]);

      Exchange(pOut[11], pOut[14]);
    }
    else
    {
      pOut[0] = pIn[0];
      pOut[4] = pIn[1];
      pOut[8] = pIn[2];
      pOut[12] = pIn[3];

      pOut[1] = pIn[4];
      pOut[5] = pIn[5];
      pOut[9] = pIn[6];
      pOut[13] = pIn[7];

      pOut[2] = pIn[8];
      pOut[6] = pIn[9];
      pOut[10] = pIn[10];
      pOut[14] = pIn[11];

      pOut[3] = pIn[12];
      pOut[7] = pIn[13];
      pOut[11] = pIn[14];
      pOut[15] = pIn[15];
    }
  }
}

inline void mathRotateX(float *pMatr, float fDegr, int OptFlags)
{
  Matrix44 rm;
  float cossin[2];
  sincos_tpl(fDegr*gf_DEGTORAD, cossin);
  rm(0,0) = 1; rm(0,1) = 0; rm(0,2) = 0; rm(0,3) = 0;
  rm(1,0) = 0; rm(1,1) = cossin[0]; rm(1,2) = cossin[1]; rm(1,3) = 0;
  rm(2,0) = 0; rm(2,1) = -cossin[1]; rm(2,2) = cossin[0]; rm(2,3) = 0;
  rm(3,0) = 0; rm(3,1) = 0; rm(3,2) = 0; rm(3,3) = 1;
  mathMatrixMultiply(pMatr, pMatr, rm.GetData(), OptFlags);
}
inline void mathRotateY(float *pMatr, float fDegr, int OptFlags)
{
  Matrix44 rm;
  float cossin[2];
  sincos_tpl(fDegr*gf_DEGTORAD, cossin);
  rm(0,0) = cossin[0]; rm(0,1) = 0; rm(0,2) = -cossin[1]; rm(0,3) = 0;
  rm(1,0) = 0; rm(1,1) = 1; rm(1,2) = 0; rm(1,3) = 0;
  rm(2,0) = cossin[1]; rm(2,1) = 0; rm(2,2) = cossin[0]; rm(2,3) = 0;
  rm(3,0) = 0; rm(3,1) = 0; rm(3,2) = 0; rm(3,3) = 1;
  mathMatrixMultiply(pMatr, pMatr, rm.GetData(), OptFlags);
}
inline void mathRotateZ(float *pMatr, float fDegr, int OptFlags)
{
  Matrix44 rm;
  float cossin[2];
  sincos_tpl(fDegr*gf_DEGTORAD, cossin);
  rm(0,0) = cossin[0]; rm(0,1) = cossin[1]; rm(0,2) = 0; rm(0,3) = 0;
  rm(1,0) = -cossin[1]; rm(1,1) = cossin[0]; rm(1,2) = 0; rm(1,3) = 0;
  rm(2,0) = 0; rm(2,1) = 0; rm(2,2) = 1; rm(2,3) = 0;
  rm(3,0) = 0; rm(3,1) = 0; rm(3,2) = 0; rm(3,3) = 1;
  mathMatrixMultiply(pMatr, pMatr, rm.GetData(), OptFlags);
}

inline void mathScale(float *pMatr, Vec3d vScale, int)
{
  pMatr[0] *= vScale.x;   pMatr[4] *= vScale.y;   pMatr[8]  *= vScale.z;
  pMatr[1] *= vScale.x;   pMatr[5] *= vScale.y;   pMatr[9]  *= vScale.z;
  pMatr[2] *= vScale.x;   pMatr[6] *= vScale.y;   pMatr[10] *= vScale.z;
  pMatr[3] *= vScale.x;   pMatr[7] *= vScale.y;   pMatr[11] *= vScale.z;
}

inline void mathCalcMatrix(Matrix44& Matrix, Vec3d vPos, Vec3d vAngs, Vec3d vScale, int OptFlags)
{
  Matrix.SetTranslationMat(vPos);

  if (vAngs.z)
    mathRotateZ(Matrix.GetData(), vAngs.z, OptFlags);
  if (vAngs.y)
    mathRotateY(Matrix.GetData(), vAngs.y, OptFlags);
  if (vAngs.x)
    mathRotateX(Matrix.GetData(), vAngs.x, OptFlags);
  if (!IsEquivalent(vScale, Vec3d(1.0f, 1.0f, 1.0f)))
    mathScale(Matrix.GetData(), vScale, OptFlags);
}

#pragma warning(pop)

#endif






#endif //math