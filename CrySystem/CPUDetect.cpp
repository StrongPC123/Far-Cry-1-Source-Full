/*=============================================================================
  CPUDetect.cpp : CPU detection.
  Copyright 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "stdafx.h"
#include "System.h"

/* features */
#define FPU_FLAG  0x0001
#define SERIAL_FLAG 0x40000
#define MMX_FLAG  0x800000
#define ISSE_FLAG 0x2000000

static int OSSupport;
static int OSExceptions;
static int sSignature;
/*
inline DWORD sCycles()
{
  uint L;
#if defined(WIN32) && !defined(WIN64)  
  __asm
  {
    xor   eax,eax	          // Required so that VC++ realizes EAX is modified.
    _emit 0x0F		          // RDTSC  -  Pentium+ time stamp register to EDX:EAX.
    _emit 0x31		          // Use only 32 bits in EAX - even a Ghz cpu would have a 4+ sec period.
    mov   [L],eax           // Save low value.
    xor   edx,edx	          // Required so that VC++ realizes EDX is modified.
  }
#else
  L = 0;
#endif
  return L;
}
*/
inline double sCycles2()
{
#if defined(WIN32) && !defined(WIN64)  
	uint L,H;
  __asm
  {
    xor   eax,eax	// Required so that VC++ realizes EAX is modified.
    xor   edx,edx	// Required so that VC++ realizes EDX is modified.
    _emit 0x0F		// RDTSC  -  Pentium+ time stamp register to EDX:EAX.
    _emit 0x31		// Use only 32 bits in EAX - even a Ghz cpu would have a 4+ sec period.
    mov   [L],eax   // Save low value.
    mov   [H],edx   // Save high value.
  }
  return ((double)L +  4294967296.0 * (double)H);
#else
  return __rdtsc();
#endif
  
}

#ifdef __GNUC__
# define cpuid(op, eax, ebx, ecx, edx) __asm__("cpuid" : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (op) : "cc");
#endif

static double measure_clock_speed(double& SecondsPerCycle )
{
  LARGE_INTEGER Freq;
  LARGE_INTEGER c0, c1;

#if !defined(_XBOX) && !defined(LINUX)
  
  uint  priority_class;
  int   thread_priority;

  /* get a copy of the current thread and process priorities */
  priority_class = GetPriorityClass( GetCurrentProcess() );
  thread_priority = GetThreadPriority( GetCurrentThread() );

  /* make this thread the highest possible priority so we get the best timing */
  SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS );
  SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
#endif
  
  QueryPerformanceFrequency(&Freq);

  double Frequency = double((INT64)(Freq.HighPart)*((INT64)65536*(INT64)65536)+(INT64)Freq.LowPart);
  assert (Frequency!=0);

  QueryPerformanceCounter(&c0);
  double ts0 = sCycles2();
  Sleep(100);
  double ts1 = sCycles2();
  QueryPerformanceCounter(&c1);

  double Count0 = double((INT64)(c0.HighPart)*((INT64)65536*(INT64)65536)+(INT64)c0.LowPart) / Frequency;
  double Count1 = double((INT64)(c1.HighPart)*((INT64)65536*(INT64)65536)+(INT64)c1.LowPart) / Frequency;

  SecondsPerCycle = (Count1-Count0) / (ts1-ts0);

#if !defined(_XBOX) && !defined(LINUX)

  /* restore the thread priority */
  SetPriorityClass( GetCurrentProcess(), priority_class );
  SetThreadPriority( GetCurrentThread(), thread_priority );

#endif

  return 1.0e-6/SecondsPerCycle;
}

/* TODO: Alpha AXP */
static unsigned long __stdcall DetectProcessor( void *arg )
{
  const unsigned char hex_chars[16] =
  {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };
  unsigned long signature = 0;
  unsigned long cache_temp;
  unsigned long cache_eax;
  unsigned long cache_ebx;
  unsigned long cache_ecx;
  unsigned long cache_edx;
  unsigned long features_ebx = 0;
  unsigned long features_ecx = 0;
  unsigned long features_edx = 0;
  unsigned long serial_number[3];
  unsigned short  fp_status;
  unsigned char cpu_type;
  unsigned char fpu_type;
  unsigned char CPUID_flag = 0;
  unsigned char celeron_flag = 0;
  unsigned char pentiumxeon_flag = 0;
  unsigned char amd3d_flag = 0;
  unsigned char name_flag = 0;
  unsigned char cyrix_flag = 0;
  unsigned char v86_flag;
  char  vendor[13];
  char  name[49];
  char  *serial;
  char  *cpu_string;
  char  *cpu_extra_string;
  char  *fpu_string;
  char  *vendor_string;
  SCpu  *p = (SCpu *) arg;
  float     test_val[4] = { 1.f, 1.f, 1.f, 1.f };

#if (!defined(WIN64) && !defined(LINUX))
  __asm
  {
    /*
     * 8086 processor check
     * Bits 12-15 of the FLAGS register are always set on the
     * 8086 processor.
     */
    pushf         /* push original FLAGS */
    pop ax          /* get original FLAGS */
    mov cx, ax        /* save original FLAGS */
    and ax, 0fffh     /* clear bits 12-15 in FLAGS */
    push ax         /* save new FLAGS value on stack */
    popf          /* replace current FLAGS value */
    pushf         /* get new FLAGS */
    pop ax          /* store new FLAGS in AX */
    and ax, 0f000h      /* if bits 12-15 are set, then */
    cmp ax, 0f000h      /* processor is an 8086/8088 */
    mov cpu_type, 0     /* turn on 8086/8088 flag */
    jne check_80286     /* go check for 80286 */
    push sp         /* double check with push sp */
    pop dx          /* if value pushed was different */
    cmp dx, sp        /* means it’s really an 8086 */
    jne end_cpu_type    /* jump if processor is 8086/8088 */
    mov cpu_type, 010h    /* indicate unknown processor */
    jmp end_cpu_type

    /*
     * 286 processor check
     * Bits 12-15 of the FLAGS register are always clear on the
     * Intel 286 processor in real-address mode.
     */
check_80286:
    smsw ax         /* save machine status word */
    and ax, 1       /* isolate PE bit of MSW */
    mov v86_flag, al    /* save PE bit to indicate V86 */
    or cx, 0f000h     /* try to set bits 12-15 */
    push cx         /* save new FLAGS value on stack */
    popf          /* replace current FLAGS value */
    pushf         /* get new FLAGS */
    pop ax          /* store new FLAGS in AX */
    and ax, 0f000h      /* if bits 12-15 are clear */
    mov cpu_type, 2     /* processor=80286, turn on 80286 flag */
    jz end_cpu_type     /* jump if processor is 80286 */

    /*
     * 386 processor check
     * The AC bit, bit #18, is a new bit introduced in the EFLAGS
     * register on the 486 processor to generate alignment
     * faults.
     * This bit cannot be set on the 386 processor.
    */
    mov cpu_type, 3     /* turn on 80386 processor flag */
    pushfd          /* push original EFLAGS */
    pop eax         /* get original EFLAGS */
    mov ebx, eax      /* save original EFLAGS */
    xor eax, 040000h    /* flip AC bit in EFLAGS */
    push eax        /* save new EFLAGS value on stack */
    popfd         /* replace current EFLAGS value */
    pushfd          /* get new EFLAGS */
    pop eax         /* store new EFLAGS in EAX */
    cmp eax, ebx      /* can’t toggle AC bit, processor=80386 */
    jz end_cpu_type     /* jump if 80386 processor */
    push ebx
    popfd         /* restore AC bit in EFLAGS */

    /*
     * Checking for ability to set/clear ID flag (Bit 21) in EFLAGS
     * which indicates the presence of a processor with the CPUID
     * instruction.
     */
    mov cpu_type, 4     /* turn on 80486 processor flag */
    pushfd            /* save EFLAGS to stack */
    pop eax           /* store EFLAGS in EAX */
    mov ebx, eax      /* save in EBX for testing later */
    xor eax, 0200000h   /* flip bit 21 in EFLAGS */
    push eax          /* save new EFLAGS value on stack */
    popfd             /* replace current EFLAGS value */
    pushfd            /* get new EFLAGS */
    pop eax           /* store new EFLAGS in EAX */
    cmp eax, ebx      /* see if bit 21 has changed */
    jne yes_CPUID     /* CPUID present */
    /* Cyrix processor check */
    xor ax, ax        /* clear ax */
    sahf            /* clear flags, bit 1 is always 1 in flags */
    mov ax, 5       /* move 5 into the dividend */
    mov bx, 2       /* move 2 into the divisor */
    div bl          /* do an operation that does not change flags */
    lahf            /* get flags */
    cmp ah, 2       /* check for change in flags */
    jne not_cyrix   /* flags changed, not a Cyrix CPU */
    mov cyrix_flag, 1
    /* TODO: identify cyrix processor */
not_cyrix:
    jmp end_cpu_type
yes_CPUID:
    push ebx
    popfd
    mov CPUID_flag, 1   /* flag indicating use of CPUID instruction */

    /*
     * Execute CPUID instruction to determine vendor, family,
     * model, stepping and features.
     */
    push ebx        /* save registers */
    push esi
    push edi
    xor eax, eax    /* set up for CPUID instruction */
    cpuid           /* get and save vendor ID */
    mov byte ptr[vendor], bl
    mov byte ptr[vendor + 1], bh
    ror ebx, 16
    mov byte ptr[vendor + 2], bl
    mov byte ptr[vendor + 3], bh
    mov byte ptr[vendor + 4], dl
    mov byte ptr[vendor + 5], dh
    ror edx, 16
    mov byte ptr[vendor + 6], dl
    mov byte ptr[vendor + 7], dh
    mov byte ptr[vendor + 8], cl
    mov byte ptr[vendor + 9], ch
    ror ecx, 16
    mov byte ptr[vendor + 10], cl
    mov byte ptr[vendor + 11], ch
    mov byte ptr[vendor + 12], 0
    cmp eax, 1        /* make sure 1 is valid input for CPUID */
    jl end_CPUID_type   /* if not, jump to end */
    mov eax, 1
    cpuid         /* get family/model/stepping/features */
    mov signature, eax
    mov features_ebx, ebx
    mov features_ecx, ecx
    mov features_edx, edx
    shr eax, 8        /* isolate family */
    and eax, 0fh
    mov cpu_type, al    /* set cpu_type with family */

    /*
     * Execute CPUID instruction to determine the cache descriptor
     * information.
     */
    xor eax, eax      /* set up to check the EAX value */
    cpuid
    cmp ax, 2       /* are cache descriptors supported? */
    jl end_CPUID_type
    mov eax, 2        /* set up to read cache descriptor */
    cpuid
    cmp al, 1       /* is one iteration enough to obtain */
    jne end_CPUID_type    /* cache information? */

    /* this code supports one iteration only. */
    mov cache_eax, eax    /* store cache information */
    mov cache_ebx, ebx    /* NOTE: for future processors, CPUID */
    mov cache_ecx, ecx    /* instruction may need to be run more */
    mov cache_edx, edx    /* than once to get complete cache information */
end_CPUID_type:
    pop edi         /* restore registers */
    pop esi
    pop ebx
    
    /* check for 3DNow! */
    mov eax, 080000000h   /* query for extended functions */
    cpuid         /* get extended function limit */
    cmp eax, 080000001h   /* functions up to 80000001h must be present */
    jb no_extended      /* 80000001h is not available */
    mov eax, 080000001h   /* setup extended function 1 */
    cpuid         /* call the function */
    test edx, 080000000h  /* test bit 31 */
    jz end_3dnow
    mov amd3d_flag, 1   /* 3DNow! supported */
end_3dnow:

    /* get CPU name */
    mov eax, 080000000h
    cpuid
    cmp eax, 080000004h
    jb end_name       /* functions up to 80000004h must be present */
    mov name_flag, 1
    mov eax, 080000002h
    cpuid
    mov dword ptr[name], eax
    mov dword ptr[name + 4], ebx
    mov dword ptr[name + 8], ecx
    mov dword ptr[name + 12], edx
    mov eax, 080000003h
    cpuid
    mov dword ptr[name + 16], eax
    mov dword ptr[name + 20], ebx
    mov dword ptr[name + 24], ecx
    mov dword ptr[name + 28], edx
    mov eax, 080000004h
    cpuid
    mov dword ptr[name + 32], eax
    mov dword ptr[name + 36], ebx
    mov dword ptr[name + 40], ecx
    mov dword ptr[name + 44], edx
end_name:

no_extended:

end_cpu_type:

    /* detect FPU */
    fninit          /* reset FP status word */
    mov fp_status, 05a5ah /* initialize temp word to non-zero */
    fnstsw fp_status    /* save FP status word */
    mov ax, fp_status   /* check FP status word */
    cmp al, 0       /* was correct status written */
    mov fpu_type, 0     /* no FPU present */
    jne end_fpu_type
    /* check control word */
    fnstcw fp_status    /* save FP control word */
    mov ax, fp_status   /* check FP control word */
    and ax, 103fh     /* selected parts to examine */
    cmp ax, 3fh       /* was control word correct */
    mov fpu_type, 0
    jne end_fpu_type    /* incorrect control word, no FPU */
    mov fpu_type, 1
    /* 80287/80387 check for the Intel386 processor */
    cmp cpu_type, 3
    jne end_fpu_type
    fld1          /* must use default control from FNINIT */
    fldz          /* form infinity */
    fdiv          /* 8087/Intel287 NDP say +inf = -inf */
    fld st          /* form negative infinity */
    fchs          /* Intel387 NDP says +inf <> -inf */
    fcompp          /* see if they are the same */
    fstsw fp_status     /* look at status from FCOMPP */
    mov ax, fp_status
    mov fpu_type, 2     /* store Intel287 NDP for FPU type */
    sahf          /* see if infinities matched */
    jz end_fpu_type     /* jump if 8087 or Intel287 is present */
    mov fpu_type, 3     /* store Intel387 NDP for FPU type */
end_fpu_type:
  }
#else
	cpu_type = 0xF;
  fpu_type = 3;
	signature = 0;
#endif
  p->mFamily = cpu_type;
  p->mModel = (signature >> 4) & 0xf;
  p->mStepping = signature & 0xf;

  p->mSpeed = (int)measure_clock_speed(p->m_SecondsPerCycle);
  p->mFeatures = 0;

  p->mFeatures |= amd3d_flag ? CFI_3DNOW : 0;
  p->mFeatures |= (features_edx & MMX_FLAG)  ? CFI_MMX : 0;
  p->mFeatures |= (features_edx & ISSE_FLAG) ? CFI_SSE : 0;
  p->mbSerialPresent = ((features_edx & SERIAL_FLAG) != 0);

  if( features_edx & SERIAL_FLAG )
  {
#if (defined(WIN64) || defined (LINUX))
		serial_number[0] = serial_number[1] = serial_number[2] = 0;
#else
    /* read serial number */
    __asm
    {
      mov eax, 1
      cpuid
      mov serial_number[2], eax /* top 32 bits are the processor signature bits */
      mov eax, 3
      cpuid
      mov serial_number[1], edx
      mov serial_number[0], ecx
    }
#endif

    /* format number */
    serial = p->mSerialNumber;

    serial[0] = hex_chars[(serial_number[2] >> 28) & 0x0f];
    serial[1] = hex_chars[(serial_number[2] >> 24) & 0x0f];
    serial[2] = hex_chars[(serial_number[2] >> 20) & 0x0f];
    serial[3] = hex_chars[(serial_number[2] >> 16) & 0x0f];

    serial[4] = '-';

    serial[5] = hex_chars[(serial_number[2] >> 12) & 0x0f];
    serial[6] = hex_chars[(serial_number[2] >>  8) & 0x0f];
    serial[7] = hex_chars[(serial_number[2] >>  4) & 0x0f];
    serial[8] = hex_chars[(serial_number[2] >>  0) & 0x0f];

    serial[9] = '-';

    serial[10] = hex_chars[(serial_number[1] >> 28) & 0x0f];
    serial[11] = hex_chars[(serial_number[1] >> 24) & 0x0f];
    serial[12] = hex_chars[(serial_number[1] >> 20) & 0x0f];
    serial[13] = hex_chars[(serial_number[1] >> 16) & 0x0f];

    serial[14] = '-';

    serial[15] = hex_chars[(serial_number[1] >> 12) & 0x0f];
    serial[16] = hex_chars[(serial_number[1] >>  8) & 0x0f];
    serial[17] = hex_chars[(serial_number[1] >>  4) & 0x0f];
    serial[18] = hex_chars[(serial_number[1] >>  0) & 0x0f];

    serial[19] = '-';

    serial[20] = hex_chars[(serial_number[0] >> 28) & 0x0f];
    serial[21] = hex_chars[(serial_number[0] >> 24) & 0x0f];
    serial[22] = hex_chars[(serial_number[0] >> 20) & 0x0f];
    serial[23] = hex_chars[(serial_number[0] >> 16) & 0x0f];

    serial[24] = '-';

    serial[25] = hex_chars[(serial_number[0] >> 12) & 0x0f];
    serial[26] = hex_chars[(serial_number[0] >>  8) & 0x0f];
    serial[27] = hex_chars[(serial_number[0] >>  4) & 0x0f];
    serial[28] = hex_chars[(serial_number[0] >>  0) & 0x0f];

    serial[29] = 0;
  }

  vendor_string = "Unknown";
  cpu_string = "Unknown";
  cpu_extra_string = "";
  fpu_string = "Unknown";

  if( !CPUID_flag )
  {
    switch( cpu_type )
    {
      case 0:
        cpu_string = "8086";
        break;

      case 2:
        cpu_string = "80286";
        break;

      case 3:
        cpu_string = "80386";
        switch( fpu_type )
        {
          case 2:
            fpu_string = "80287";
            break;

          case 1:
            fpu_string = "80387";
            break;

          default:
            fpu_string = "None";
          break;
        }
        break;

      case 4:
        if( fpu_type )
        {
          cpu_string = "80486DX, 80486DX2 or 80487SX";
          fpu_string = "on-chip";
        }
        else
          cpu_string = "80486SX";
        break;
    }
  }
  else
  {    /* using CPUID instruction */
    if( !name_flag )
    {
      if( !strcmp(vendor, "GenuineIntel") )
      {
        vendor_string = "Intel";
        switch( cpu_type )
        {
          case 4:
            switch( p->mModel )
            {
              case 0:
              case 1:
                cpu_string = "80486DX";
                break;

              case 2:
                cpu_string = "80486SX";
                break;

              case 3:
                cpu_string = "80486DX2";
                break;

              case 4:
                cpu_string = "80486SL";
                break;

              case 5:
                cpu_string = "80486SX2";
                break;

              case 7:
                cpu_string = "Write-Back Enhanced 80486DX2";
                break;

              case 8:
                cpu_string = "80486DX4";
                break;

              default:
                cpu_string = "80486";
            }
            break;

          case 5:
            switch( p->mModel )
            {
              default:
              case 1:
              case 2:
              case 3:
                cpu_string = "Pentium";
                break;

              case 4:
                cpu_string = "Pentium MMX";
                break;
            }
            break;

          case 6:
            switch( p->mModel )
            {
              case 1:
                cpu_string = "Pentium Pro";
                break;

              case 3:
                cpu_string = "Pentium II";
                break;

              case 5:
              case 7:
                {
                  cache_temp = cache_eax & 0xFF000000;
                  if( cache_temp == 0x40000000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x44000000) && (cache_temp <= 0x45000000) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_eax & 0xFF0000;
                  if( cache_temp == 0x400000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x440000) && (cache_temp <= 0x450000) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_eax & 0xFF00;
                  if( cache_temp == 0x4000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x4400) && (cache_temp <= 0x4500) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_ebx & 0xFF000000;
                  if( cache_temp == 0x40000000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x44000000) && (cache_temp <= 0x45000000) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_ebx & 0xFF0000;
                  if( cache_temp == 0x400000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x440000) && (cache_temp <= 0x450000) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_ebx & 0xFF00;
                  if( cache_temp == 0x4000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x4400) && (cache_temp <= 0x4500) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_ebx & 0xFF;
                  if( cache_temp == 0x40 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x44) && (cache_temp <= 0x45) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_ecx & 0xFF000000;
                  if( cache_temp == 0x40000000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x44000000) && (cache_temp <= 0x45000000) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_ecx & 0xFF0000;
                  if( cache_temp == 0x400000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x440000) && (cache_temp <= 0x450000) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_ecx & 0xFF00;
                  if( cache_temp == 0x4000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x4400) && (cache_temp <= 0x4500) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_ecx & 0xFF;
                  if( cache_temp == 0x40 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x44) && (cache_temp <= 0x45) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_edx & 0xFF000000;
                  if( cache_temp == 0x40000000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x44000000) && (cache_temp <= 0x45000000) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_edx & 0xFF0000;
                  if( cache_temp == 0x400000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x440000) && (cache_temp <= 0x450000) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_edx & 0xFF00;
                  if( cache_temp == 0x4000 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x4400) && (cache_temp <= 0x4500) )
                    pentiumxeon_flag = 1;
                  cache_temp = cache_edx & 0xFF;
                  if( cache_temp == 0x40 )
                    celeron_flag = 1;
                  if( (cache_temp >= 0x44) && (cache_temp <= 0x45) )
                    pentiumxeon_flag = 1;

                  if( celeron_flag )
                  {
                    cpu_string = "Celeron";
                  }
                  else
                  {
                    if(pentiumxeon_flag)
                    {
                      if( p->mModel == 5 )
                      {
                        cpu_string = "Pentium II Xeon";
                      }
                      else
                      {
                        cpu_string = "Pentium III Xeon";
                      }
                    }
                    else
                    {
                      if( p->mModel == 5 )
                      {
                        cpu_string = "Pentium II";
                      }
                      else
                      {
                        cpu_string = "Pentium III";
                      }
                    }
                  }
                }
                break;

              case 6:
                cpu_string = "Celeron";
                break;

              case 8:
                cpu_string = "Pentium III";
                break;
            }
            break;

        }

        if( signature & 0x1000 )
        {
          cpu_extra_string = " OverDrive";
        }
        else
        if( signature & 0x2000 )
        {
          cpu_extra_string = " dual upgrade";
        }
      }
      else
      if( !strcmp( vendor, "CyrixInstead" ) )
      {
        vendor_string = "Cyrix";
        switch( p->mFamily )
        {
          case 4:
            switch( p->mModel )
            {
              case 4:
                cpu_string = "MediaGX";
                break;
            }
            break;

          case 5:
            switch( p->mModel )
            {
              case 2:
                cpu_string = "6x86";
                break;

              case 4:
                cpu_string = "GXm";
                break;
            }
            break;

          case 6:
            switch( p->mModel )
            {
              case 0:
                cpu_string = "6x86MX";
                break;
            }
            break;
        }
      }
      else
      if( !strcmp(vendor, "AuthenticAMD") )
      {
        strncpy( p->mVendor, "AMD", sizeof(p->mVendor) );
        switch( p->mFamily )
        {
          case 4:
            cpu_string = "Am486 or Am5x86";
            break;

          case 5:
            switch( p->mModel )
            {
              case 0:
              case 1:
              case 2:
              case 3:
                cpu_string = "K5";
                break;

              case 4:
              case 5:
              case 6:
              case 7:
                cpu_string = "K6";
                break;

              case 8:
                cpu_string = "K6-2";
                break;

              case 9:
                cpu_string = "K6-III";
                break;
            }
            break;

          case 6:
            cpu_string = "Athlon";
            break;
        }
      }
      else
      if( !strcmp(vendor, "CentaurHauls") )
      {
        vendor_string = "Centaur";
        switch( cpu_type )
        {
          case 5:
            switch( p->mModel )
            {
              case 4:
                cpu_string = "WinChip";
                break;

              case 8:
                cpu_string = "WinChip2";
                break;
            }
            break;
        }
      }
      else
      if( !strcmp(vendor, "UMC UMC UMC ") )
      {
        vendor_string = "UMC";
      }
      else
      if( !strcmp(vendor, "NexGenDriven") )
      {
        vendor_string = "NexGen";
      }
    }
    else
    {
      vendor_string = vendor;
      cpu_string = name;
    }

    if( features_edx & FPU_FLAG )
    {
      fpu_string = "on-chip";
    }
    else
    {
      fpu_string = "Unknown";
    }
  }

  strncpy( p->mCpuType, cpu_string, sizeof(p->mCpuType) );
  strncat( p->mCpuType, cpu_extra_string, sizeof(p->mCpuType) );
  strncpy( p->mFpuType, fpu_string, sizeof(p->mFpuType) );
  strncpy( p->mVendor, vendor_string, sizeof(p->mVendor) );
  sSignature = signature;
  
  if (!stricmp(vendor_string, "Intel"))
    p->meVendor = eCVendor_Intel;
  else
  if (!stricmp(vendor_string, "Cyrix"))
    p->meVendor = eCVendor_Cyrix;
  else
  if (!stricmp(vendor_string, "AMD"))
    p->meVendor = eCVendor_AMD;
  else
  if (!stricmp(vendor_string, "Centaur"))
    p->meVendor = eCVendor_Centaur;
  else
  if (!stricmp(vendor_string, "NexGen"))
    p->meVendor = eCVendor_NexGen;
  else
  if (!stricmp(vendor_string, "UMC"))
    p->meVendor = eCVendor_UMC;
  else
    p->meVendor = eCVendor_Unknown;

  if (strstr(cpu_string, "8086"))
    p->meModel = eCpu_8086;
  else
  if (strstr(cpu_string, "80286"))
    p->meModel = eCpu_80286;
  else
  if (strstr(cpu_string, "80386"))
    p->meModel = eCpu_80386;
  else
  if (strstr(cpu_string, "80486"))
    p->meModel = eCpu_80486;
  else
  if (!stricmp(cpu_string, "Pentium MMX") || !stricmp(cpu_string, "Pentium"))
    p->meModel = eCpu_Pentium;
  else
  if (!stricmp(cpu_string, "Pentium Pro"))
    p->meModel = eCpu_PentiumPro;
  else
  if (!stricmp(cpu_string, "Pentium II"))
    p->meModel = eCpu_Pentium2;
  else
  if (!stricmp(cpu_string, "Pentium III"))
    p->meModel = eCpu_Pentium3;
  else
  if (!stricmp(cpu_string, "Pentium 4"))
    p->meModel = eCpu_Pentium4;
  else
  if (!stricmp(cpu_string, "Celeron"))
    p->meModel = eCpu_Celeron;
  else
  if (!stricmp(cpu_string, "Pentium II Xeon"))
    p->meModel = eCpu_Pentium2Xeon;
  else
  if (!stricmp(cpu_string, "Pentium III Xeon"))
    p->meModel = eCpu_Pentium3Xeon;
  else
  if (!stricmp(cpu_string, "MediaGX"))
    p->meModel = eCpu_CyrixMediaGX;
  else
  if (!stricmp(cpu_string, "6x86"))
    p->meModel = eCpu_Cyrix6x86;
  else
  if (!stricmp(cpu_string, "GXm"))
    p->meModel = eCpu_CyrixGXm;
  else
  if (!stricmp(cpu_string, "6x86MX"))
    p->meModel = eCpu_Cyrix6x86MX;
  else
  if (!stricmp(cpu_string, "Am486 or Am5x86"))
    p->meModel = eCpu_Am5x86;
  else
  if (!stricmp(cpu_string, "K5"))
    p->meModel = eCpu_AmK5;
  else
  if (!stricmp(cpu_string, "K6"))
    p->meModel = eCpu_AmK6;
  else
  if (!stricmp(cpu_string, "K6-2"))
    p->meModel = eCpu_AmK6_2;
  else
  if (!stricmp(cpu_string, "K6-III"))
    p->meModel = eCpu_AmK6_3;
  else
  if (!stricmp(cpu_string, "Athlon"))
    p->meModel = eCpu_AmAthlon;
  else
  if (!stricmp(cpu_string, "Duron"))
    p->meModel = eCpu_AmDuron;
  else
  if (!stricmp(cpu_string, "WinChip"))
    p->meModel = eCpu_CenWinChip;
  else
  if (!stricmp(cpu_string, "WinChip2"))
    p->meModel = eCpu_CenWinChip2;
  else
    p->meModel = eCpu_Unknown;

  return 1;
}

/* ------------------------------------------------------------------------------ */
void CCpuFeatures::Detect(void)
{
#if !defined(PS2) && !defined (GC) && !defined (LINUX)

#if !defined(_XBOX) && !defined(LINUX)
  SYSTEM_INFO   sys_info;
  DWORD_PTR system_affinity_mask;
#endif
  HANDLE      thread;
  unsigned long thread_id;
  int current_processor;
  uint  current_processor_element;
  DWORD_PTR process_affinity_mask;
  uint  thread_processor_mask;
  unsigned char c;

  /* get the system info to derive the number of processors within the system. */
#if !defined(_XBOX) && !defined(LINUX)
  GetSystemInfo( &sys_info );
  m_NumSystemProcessors = sys_info.dwNumberOfProcessors;
  m_NumAvailProcessors = 0;
  GetProcessAffinityMask( GetCurrentProcess(), &process_affinity_mask, &system_affinity_mask );
#else
  m_NumSystemProcessors = 1;
  m_NumAvailProcessors = 0;
  process_affinity_mask = 1;
#endif


  for( c = 0; c < m_NumSystemProcessors; c++ )
  {
    if( process_affinity_mask & (1 << c) )
    {
      m_NumAvailProcessors++;
    }
  }

  OSSupport = OSExceptions = 0;

  current_processor_element = 0;

  for( current_processor = 0; current_processor < m_NumSystemProcessors; current_processor++ )
  {
    thread_processor_mask = 1 << current_processor;

    /* Is the processor we are about to set for the detect thread part of the
     * process affinity mask. If it is not then it cannot be set.
     */
    if( (process_affinity_mask & thread_processor_mask) )
    {
      /* create a thread that is suspended */
      thread = CreateThread( NULL, 0, DetectProcessor, &m_Cpu[current_processor_element], CREATE_SUSPENDED, &thread_id );

      if( thread )
      {
        /*
         * set the affinity of the thread so to force it to run
         * on the required processor
         */
#if !defined(_XBOX) && !defined(LINUX)
        if( SetThreadAffinityMask( thread, thread_processor_mask ) )
#endif
        {
          /*
           * Now we have set the processor resume the thread and it
           * will run only on the specified processor.
           */
          ResumeThread( thread );

          /* wait for the current detection thread to finish */
          WaitForSingleObject( thread, INFINITE );

          /* close the handle to the now finished thread */
          CloseHandle( thread );

          /* use the next element of the array */
          current_processor_element++;

          if( current_processor_element == MAX_CPU )
            break;
          if( current_processor_element == m_NumAvailProcessors )
            break;
        }
      }
    }
  }

  m_bOS_ISSE = OSSupport != 0;
  m_bOS_ISSE_EXCEPTIONS = OSExceptions != 0;

  CryLogAlways("\n--- CPU detection ---\n" );
  CryLogAlways("Number of system processors: %d\n", m_NumSystemProcessors );
  CryLogAlways("Number of available processors: %d\n", m_NumAvailProcessors );
  int num = m_NumAvailProcessors;
  if( num > MAX_CPU )
    num = MAX_CPU;
  for(int i=0; i<num; i++)
  {
    SCpu *p = &m_Cpu[i];

    CryLogAlways("Processor %d:\n", i );
    CryLogAlways("CPU: %s %s\n", p->mVendor, p->mCpuType );
    CryLogAlways("Family: %d, Model: %d, Stepping: %d\n", p->mFamily, p->mModel, p->mStepping );
    CryLogAlways("FPU: %s\n", p->mFpuType );
    CryLogAlways("CPU Speed (estimated): %f MHz\n", 1.0e-6/p->m_SecondsPerCycle );
    if (p->mFeatures & CFI_MMX)
      CryLogAlways("MMX: present\n");
    else
      CryLogAlways("MMX: not present\n");
    if (p->mFeatures & CFI_SSE)
      CryLogAlways("SSE: present\n");
    else
      CryLogAlways("SSE: not present\n");
    if (p->mFeatures & CFI_3DNOW)
      CryLogAlways("3DNow!: present\n");
    else
      CryLogAlways("3DNow!: not present\n");
    if( p->mbSerialPresent )
      CryLogAlways("Serial number: %s\n\n", p->mSerialNumber );
    else
      CryLogAlways("Serial number not present or disabled\n\n" );
  }
  CryLogAlways("---------------------" );

#endif
}

