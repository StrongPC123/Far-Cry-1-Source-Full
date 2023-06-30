#include "RenderPCH.h"

// Not for AMD64
#if !defined(WIN64) && !defined(LINUX)

#pragma warning(push)
#pragma warning(disable:4731) // frame pointer register 'ebp' modified by inline assembly code

struct SConstSSE
{
  float m_fVal0;
  float m_fVal1;
  float m_fVal2;
  float m_fVal3;
};

_declspec(align(16)) SConstSSE _tabCoef64_1[4][6] =
{
  {
    {1.000000f, 0.995185f, 0.980785f, 0.956940f},
    {0.000000f, -0.098017f, -0.195090f, -0.290285f},
    {1.000000f, 0.980785f, 0.923880f, 0.831470f},
    {0.000000f, -0.195090f, -0.382683f, -0.555570f},
    {1.000000f, 0.956940f, 0.831470f, 0.634393f},
    {0.000000f, -0.290285f, -0.555570f, -0.773010f},
  },
  {
    {0.923880f, 0.881921f, 0.831470f, 0.773010f},
    {-0.382683f, -0.471397f, -0.555570f, -0.634393f},
    {0.707107f, 0.555570f, 0.382683f, 0.195090f},
    {-0.707107f, -0.831470f, -0.923880f, -0.980785f},
    {0.382683f, 0.098017f, -0.195090f, -0.471397f},
    {-0.923880f, -0.995185f, -0.980785f, -0.881921f},
  },
  {
    {0.707107f, 0.634393f, 0.555570f, 0.471397f},
    {-0.707107f, -0.773010f, -0.831470f, -0.881921f},
    {0.000000f, -0.195090f, -0.382683f, -0.555570f},
    {-1.000000f, -0.980785f, -0.923880f, -0.831470f},
    {-0.707107f, -0.881921f, -0.980785f, -0.995185f},
    {-0.707107f, -0.471397f, -0.195090f, 0.098017f},
  },
  {
    {0.382683f, 0.290285f, 0.195090f, 0.098017f},
    {-0.923880f, -0.956940f, -0.980785f, -0.995185f},
    {-0.707107f, -0.831470f, -0.923880f, -0.980785f},
    {-0.707107f, -0.555570f, -0.382683f, -0.195090f},
    {-0.923880f, -0.773010f, -0.555570f, -0.290285f},
    {0.382683f, 0.634393f, 0.831470f, 0.956940f},
  }
};

_declspec(align(16)) SConstSSE _tabCoef64_2[4][6] =
{
  {
    {1.000000f, 0.923880f, 0.707107f, 0.382683f},
    {0.000000f, -0.382683f, -0.707107f, -0.923880f},
    {1.000000f, 0.707107f, 0.000000f, -0.707107f},
    {0.000000f, -0.707107f, -1.000000f, -0.707107f},
    {1.000000f, 0.382683f, -0.707107f, -0.923880f},
    {0.000000f, -0.923880f, -0.707107f, 0.382683f},
  }
};

void cradix4c_64(float* ar, float* ai, int nm)
{
  int wdt = OCEANGRID;
  float *arw = &ar[wdt];
  float *aiw = &ai[wdt];
  _asm
  {
    mov         eax, arw
    mov         ebx, aiw
    mov         edx, ai
    mov         ecx, ar
    sub         esp, 20h
    mov         [esp+0ch], ebp
    mov         [esp], ecx
    mov         [esp+4], edx
    mov         ebp, ecx
    mov         [esp+10h], eax
    mov         [esp+14h], ebx
    mov         [esp+18h], ebp
    mov         [esp+1ch], edx
    lea         ebx, [ebp+40h]
    lea         edi, _tabCoef64_1
    mov         [esp+8], ebx
_lAlign:
    movaps      xmm0,xmmword ptr [ebp]
    movaps      xmm4,xmm0
    movaps      xmm2,xmmword ptr [ebp+80h]
    subps       xmm0,xmm2
    movaps      xmm1,xmmword ptr [ebp+40h]
    addps       xmm4,xmm2
    movaps      xmm3,xmmword ptr [ebp+0C0h]
    movaps      xmm5,xmm1
    movaps      xmm2,xmmword ptr [edx]
    addps       xmm5,xmm3
    jmp         _lCicleAlign
align 4
_lStartAlign:
    movaps      xmmword ptr [edx+0B0h],xmm0
    movaps      xmm0,xmmword ptr [ebp]
    subps       xmm7,xmm4
    movaps      xmm4,xmm0
    movaps      xmmword ptr [ebp+0B0h],xmm5
    addps       xmm3,xmm1
    movaps      xmm2,xmmword ptr [ebp+80h]
    movaps      xmmword ptr [ebp+30h],xmm7
    subps       xmm0,xmm2
    movaps      xmm1,xmmword ptr [ebp+40h]
    addps       xmm4,xmm2
    movaps      xmm5,xmm1
    movaps      xmmword ptr [edx+30h],xmm3
    movaps      xmm2,xmmword ptr [edx]
    movaps      xmm3,xmmword ptr [ebp+0C0h]
    addps       xmm5,xmm3
_lCicleAlign:
align 4
    movaps      xmm7,xmm4
    subps       xmm1,xmm3
    prefetcht0  [edi]
    addps       xmm4,xmm5
    movaps      xmm3,xmm2
    movaps      xmm6,xmmword ptr [edx+80h]
    subps       xmm7,xmm5
    prefetcht0  [edi+10h]
    movaps      xmmword ptr [ebp],xmm4
    addps       xmm3,xmm6
    movaps      xmm5,xmmword ptr [edx+40h]
    subps       xmm2,xmm6
    prefetcht0  [edi+20h]
    movaps      xmm4,xmmword ptr [edx+0C0h]
    movaps      xmm6,xmm5
    addps       xmm5,xmm4
    prefetcht0  [edi+30h]
    subps       xmm6,xmm4
    movaps      xmm4,xmm5
    addps       xmm5,xmm3
    movaps      xmmword ptr [edx],xmm5
    movaps      xmm5,xmm0
    addps       xmm0,xmm6
    prefetcht0  [edi+40h]
    subps       xmm3,xmm4
    movaps      xmm4,xmm2
    subps       xmm2,xmm1
    prefetcht0  [edi+50h]
    addps       xmm1,xmm4
    subps       xmm5,xmm6
    movaps      xmm4,xmm0
    movaps      xmm6,xmm2
    mulps       xmm0,xmmword ptr [edi]
    mulps       xmm2,xmmword ptr [edi+10h]
    mulps       xmm4,xmmword ptr [edi+10h]
    add         ebp,10h
    add         edx,10h
    subps       xmm0,xmm2
    mulps       xmm6,xmmword ptr [edi]
    movaps      xmm2,xmm1
    movaps      xmmword ptr [ebp+70h],xmm0
    mulps       xmm1,xmmword ptr [edi+50h]
    movaps      xmm0,xmm5
    addps       xmm4,xmm6
    mulps       xmm5,xmmword ptr [edi+40h]
    mulps       xmm2,xmmword ptr [edi+40h]
    mulps       xmm0,xmmword ptr [edi+50h]
    subps       xmm5,xmm1
    movaps      xmmword ptr [edx+70h],xmm4
    movaps      xmm1,xmm7
    mulps       xmm7,xmmword ptr [edi+20h]
    movaps      xmm4,xmm3
    addps       xmm0,xmm2
    mulps       xmm3,xmmword ptr [edi+20h]
    mulps       xmm4,xmmword ptr [edi+30h]
    cmp         ebp,ebx
    mulps       xmm1,xmmword ptr [edi+30h]
    lea         edi,[edi+60h]
    jl          _lStartAlign
    movaps      xmmword ptr [ebp+0B0h],xmm5
    addps       xmm3,xmm1
    subps       xmm7,xmm4
    movaps      xmmword ptr [edx+0B0h],xmm0
    movaps      xmmword ptr [edx+30h],xmm3
    movaps      xmmword ptr [ebp+30h],xmm7
    mov         ebp,[esp]
    mov         edx,[esp+4]
    add         ebp,100h
    add         edx,100h
    mov         [esp],ebp
    mov         [esp+4],edx
    cmp         ebp,[esp+10h]
    lea         ebx,[ebp+40h]
    mov         [esp+8],ebx
    lea         edi,_tabCoef64_1
    jl          _lAlign

    mov         ebp, [esp+18h];
    mov         edx, [esp+1ch];
    mov         [esp], ebp
    mov         [esp+4], edx
    lea         ebx, [ebp+10h]
    lea         edi, _tabCoef64_2
    mov         [esp+8], ebx
_lAlign2:
    movaps      xmm0,xmmword ptr [ebp]
    movaps      xmm4,xmm0
    movaps      xmm2,xmmword ptr [ebp+20h]
    subps       xmm0,xmm2
    movaps      xmm1,xmmword ptr [ebp+10h]
    addps       xmm4,xmm2
    movaps      xmm3,xmmword ptr [ebp+30h]
    movaps      xmm5,xmm1
    movaps      xmm2,xmmword ptr [edx]
    addps       xmm5,xmm3
    jmp         _lCicleAlign2
align 4
_lStartAlign2:
    movaps      xmmword ptr [edx+20h],xmm0
    movaps      xmm0,xmmword ptr [ebp]
    subps       xmm7,xmm4
    movaps      xmm4,xmm0
    movaps      xmmword ptr [ebp+20h],xmm5
    addps       xmm3,xmm1
    movaps      xmm2,xmmword ptr [ebp+20h]
    movaps      xmmword ptr [ebp],xmm7
    subps       xmm0,xmm2
    movaps      xmm1,xmmword ptr [ebp+10h]
    addps       xmm4,xmm2
    movaps      xmm5,xmm1
    movaps      xmmword ptr [edx],xmm3
    movaps      xmm2,xmmword ptr [edx]
    movaps      xmm3,xmmword ptr [ebp+30h]
    addps       xmm5,xmm3
_lCicleAlign2:
    movaps      xmm7,xmm4
    subps       xmm1,xmm3
    prefetcht0  [edi]
    addps       xmm4,xmm5
    movaps      xmm3,xmm2
    movaps      xmm6,xmmword ptr [edx+20h]
    subps       xmm7,xmm5
    prefetcht0  [edi+10h]
    movaps      xmmword ptr [ebp],xmm4
    addps       xmm3,xmm6
    movaps      xmm5,xmmword ptr [edx+10h]
    subps       xmm2,xmm6
    prefetcht0  [edi+20h]
    movaps      xmm4,xmmword ptr [edx+30h]
    movaps      xmm6,xmm5
    addps       xmm5,xmm4
    prefetcht0  [edi+30h]
    subps       xmm6,xmm4
    movaps      xmm4,xmm5
    addps       xmm5,xmm3
    movaps      xmmword ptr [edx],xmm5
    movaps      xmm5,xmm0
    addps       xmm0,xmm6
    prefetcht0  [edi+40h]
    subps       xmm3,xmm4
    movaps      xmm4,xmm2
    subps       xmm2,xmm1
    prefetcht0  [edi+50h]
    addps       xmm1,xmm4
    subps       xmm5,xmm6
    movaps      xmm4,xmm0
    movaps      xmm6,xmm2
    mulps       xmm0,xmmword ptr [edi]
    mulps       xmm2,xmmword ptr [edi+10h]
    mulps       xmm4,xmmword ptr [edi+10h]
    add         ebp,10h
    add         edx,10h
    subps       xmm0,xmm2
    mulps       xmm6,xmmword ptr [edi]
    movaps      xmm2,xmm1
    movaps      xmmword ptr [ebp+10h],xmm0
    mulps       xmm1,xmmword ptr [edi+50h]
    movaps      xmm0,xmm5
    addps       xmm4,xmm6
    mulps       xmm5,xmmword ptr [edi+40h]
    mulps       xmm2,xmmword ptr [edi+40h]
    mulps       xmm0,xmmword ptr [edi+50h]
    subps       xmm5,xmm1
    movaps      xmmword ptr [edx+10h],xmm4
    movaps      xmm1,xmm7
    mulps       xmm7,xmmword ptr [edi+20h]
    movaps      xmm4,xmm3
    addps       xmm0,xmm2
    mulps       xmm3,xmmword ptr [edi+20h]
    mulps       xmm4,xmmword ptr [edi+30h]
    cmp         ebp,ebx
    mulps       xmm1,xmmword ptr [edi+30h]
    lea         edi,[edi+60h]
    jl          _lStartAlign2
    movaps      xmmword ptr [ebp+20h],xmm5
    addps       xmm3,xmm1
    subps       xmm7,xmm4
    movaps      xmmword ptr [edx+20h],xmm0
    movaps      xmmword ptr [edx],xmm3
    movaps      xmmword ptr [ebp],xmm7
    mov         ebp,[esp]
    mov         edx,[esp+4]
    add         ebp,40h
    add         edx,40h
    mov         [esp],ebp
    mov         [esp+4],edx
    cmp         ebp,[esp+10h]
    lea         ebx,[ebp+10h]
    mov         [esp+8],ebx
    lea         edi,_tabCoef64_2
    jl          _lAlign2

    mov         ebp, [esp+18h]
    mov         edx, [esp+1ch]
    mov         [esp], ebp
    mov         [esp+4], edx
    mov         ebx, [esp+10h]
    fld         dword ptr [ebp]
    fld         st(0)
    fld         dword ptr [ebp+8]
    fsub        st(1),st
    faddp       st(2),st
    fld         dword ptr [ebp+4]
    fxch        st(2)
    fld         st(2)
    fld         dword ptr [ebp+0Ch]
    fadd        st(1),st
    fsubp       st(4),st
    jmp         _lEnd3
align 4
_lCicle:
    fstp        dword ptr [ebp-4]
    fld         dword ptr [ebp]
    fxch        st(1)
    fstp        dword ptr [ebp-8]
    fld         st(0)
    fld         dword ptr [ebp+8]
    fadd        st(2),st
    fxch        st(3)
    fstp        dword ptr [edx-10h]
    fsubrp      st(2),st
    fld         dword ptr [ebp+4]
    fld         st(0)
    fld         dword ptr [ebp+0Ch]
    fadd        st(2),st
    fxch        st(5)
    fstp        dword ptr [edx-4]
    fsubrp      st(4),st
_lEnd3:
    fld         dword ptr [edx]
    fld         dword ptr [edx]
    fld         st(3)
    fadd        st,st(3)
    fld         dword ptr [edx+8]
    fadd        st(3),st
    fxch        st(5)
    fsubrp      st(4),st
    fld         dword ptr [edx+4]
    fxch        st(2)
    fsubrp      st(5),st
    fstp        dword ptr [ebp]
    fld         st(0)
    fld         dword ptr [edx+0Ch]
    fadd        st(2),st
    fxch        st(4)
    fstp        dword ptr [ebp+4]
    fsubrp      st(3),st
    fst         [esp+10h]
    fadd        st,st(1)
    fxch        st(4)
    fst         [esp+14h]
    fadd        st,st(2)
    fxch        st(3)
    fst         [esp+18h]
    fsub        st,st(5)
    fld         [esp+10h]
    fsubp       st(2),st
    fld         [esp+14h]
    fsubrp      st(3),st
    fld         [esp+18h]
    faddp       st(6),st
    add         ebp,10h
    add         edx,10h
    fstp        dword ptr [edx-8]
    cmp         ebp,ebx
    fstp        dword ptr [edx-0Ch]
    jl          _lCicle
    fstp        dword ptr [ebp-4]
    fstp        dword ptr [ebp-8]
    fstp        dword ptr [edx-10h]
    fstp        dword ptr [edx-4]
    mov         ebp, [esp+0ch]
    add         esp, 20h
  }
}

void bittabc(int *p, int sn)
{
  int i2 = sn;
  int j = 1;
  int i, k;
  int ind = 0;
  i2 >>= 1;
  for (i=1; i<=sn-1; i++)
  {
    if (i < j)
    {
      ind += 2;
      p[ind] = i-1;
      p[ind+1] = j-1;
    }
    k = i2;
    while (j > k)
    {
      j -= k;
      k >>= 1;
    }
    j += k;
  }
  p[0] = 0;
  p[1] = ind >> 1;
}

void coef4r22c(int *ptr, int nm)
{
  int sn = 1<<nm;
  bittabc(ptr, sn);
}


void cbitrevc(float* ar, float* ai, int *p)
{
  int nCount = p[1];
  int *pInd = &p[2];
  int nC = nCount >> 2;
  while (nC)
  {
    Exchange(ar[pInd[0]], ar[pInd[1]]);
    Exchange(ar[pInd[2]], ar[pInd[3]]);
    Exchange(ar[pInd[4]], ar[pInd[5]]);
    Exchange(ar[pInd[6]], ar[pInd[7]]);
    pInd += 8;
    nC--;
  }
  pInd = &p[2];
  nC = nCount >> 2;
  while (nC)
  {
    Exchange(ai[pInd[0]], ai[pInd[1]]);
    Exchange(ai[pInd[2]], ai[pInd[3]]);
    Exchange(ai[pInd[4]], ai[pInd[5]]);
    Exchange(ai[pInd[6]], ai[pInd[7]]);
    pInd += 8;
    nC--;
  }
}

void xcfft1dc(float* ar, float* ai, int *p, int nm)
{
  cradix4c_64(ar, ai, nm);
  cbitrevc(ar, ai, p);
}

void FFTSSE_64(float* ar, float* ai)
{
  int i, j;
  const int nm = 6;

  _declspec(align(16)) int p0[OCEANGRID*8];
  coef4r22c(p0, nm);
  for (i=0; i<OCEANGRID; i++)
  {
    xcfft1dc(&ar[OCEANGRID*i], &ai[OCEANGRID*i], p0, nm);
  }

  coef4r22c(p0, nm);

  _declspec(align(16)) float p1[4][OCEANGRID];
  _declspec(align(16)) float p2[4][OCEANGRID];

  float *src, *dst;
  for (j=0; j<OCEANGRID; j+=4)
  {
    src = &ar[j];
    for (i=0; i<OCEANGRID; i++, src+=OCEANGRID)
    {
      p1[0][i] = src[0];
      p1[1][i] = src[1];
      p1[2][i] = src[2];
      p1[3][i] = src[3];
    }
    src = &ai[j];
    for (i=0; i<OCEANGRID; i++, src+=OCEANGRID)
    {
      p2[0][i] = src[0];
      p2[1][i] = src[1];
      p2[2][i] = src[2];
      p2[3][i] = src[3];
    }

    xcfft1dc(&p1[0][0], &p2[0][0], p0, nm);
    xcfft1dc(&p1[1][0], &p2[1][0], p0, nm);
    xcfft1dc(&p1[2][0], &p2[2][0], p0, nm);
    xcfft1dc(&p1[3][0], &p2[3][0], p0, nm);

    dst = &ar[j];
    for (i=0; i<OCEANGRID; i++, dst+=OCEANGRID)
    {
      dst[0] = p1[0][i];
      dst[1] = p1[1][i];
      dst[2] = p1[2][i];
      dst[3] = p1[3][i];
    }
    dst = &ai[j];
    for (i=0; i<OCEANGRID; i++, dst+=OCEANGRID)
    {
      dst[0] = p2[0][i];
      dst[1] = p2[1][i];
      dst[2] = p2[2][i];
      dst[3] = p2[3][i];
    }
  }
}

#pragma warning(pop)

#endif // !defined(WIN64)