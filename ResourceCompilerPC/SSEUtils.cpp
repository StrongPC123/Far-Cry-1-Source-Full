#include "stdafx.h"
#include "SSEUtils.h"

namespace cpu
{
DWORD g_dwFeatures = 0;
DWORD g_dwFeaturesEx = 0;

const char* g_arrCPUCaps[] = {
	"FPU", "VME", "DE", "PSE", "TSC", "MSR", "PAE", "MCE", "CX8", "APIC",
	"Unknown(10)", "SEP", "MTRR",	"PGE", "MCA", "CMOV", "PAT", "PSE-36", "PSN", "CLFSH",
	"Unknown(20)", "DS", "ACPI", "MMX", "FXSR", "SSE", "SSE2", "SS", "HTT", "TM",
	"Unknown(30)", "PBE"
};

const char* g_arrCPUCapsLong[] = {
	"Floating Point Unit On-Chip", "Virtual 8086 Mode Enhancements", "Debugging Extensions", "Page Size Extension", "Time Stamp Counter", "Model Specific Registers RDMSR and WRMSR Instructions", "Physical Address Extension", "Machine Check Exception", "CMPXCHG8B Instruction", "Advanced Programmable Interrupt Controller On-Chip",
	"Unknown(10)", "SYSENTER and SYSEXIT Instructions", "Memory Type Range Registers",	"Page Directory Entries Global Bit", "Machine Check Architecture", "Conditional Move Instructions", "Page Attribute Table", "32-Bit Page Size Extension", "Processor Serial Number", "CLFLUSH Instruction",
	"Unknown(20)", "Debug Store", "Thermal Monitor and Software Controlled Clock Facilities", "MMX Technology", "FXSAVE and FXRSTOR Instructions", "SSE", "SSE2", "Self Snoop", "Hyper-Threading Technology", "Thermal Monitor",
	"Unknown(30)", "Pending Break Enable"
};

void logCaps()
{
#ifdef _CRY_ANIMATION_BASE_HEADER_
#ifdef _CPU_X86
	g_GetLog()->LogToFile ("CPU capabilities: ");
	for (unsigned nCap = 0; nCap < 32; ++nCap)
		if (g_dwFeatures&(1<<nCap))
		{
#ifdef _DEBUG
			g_GetLog()->LogToFilePlus(" %s.", g_arrCPUCapsLong[nCap]);
#else
			g_GetLog()->LogToFilePlus(" %s", g_arrCPUCaps[nCap]);
#endif
		}
	if (has3DNow())
		g_GetLog()->LogToFilePlus(" 3DNow!");
#endif
#endif
}

// detects CPU features (SSE) and perhaps sets up some
// pointers to functions
void detect ()
{
#if !defined(LINUX) && defined(_CPU_X86)
	__try
	{
		_asm
		{
			// 386 processor check
			// The AC bit, bit #18, is a new bit introduced in the EFLAGS
			// register on the 486 processor to generate alignment
			// faults.
			// This bit cannot be set on the 386 processor.

			pushfd          // push original EFLAGS
			pop eax         // get original EFLAGS
			mov ebx, eax      // save original EFLAGS
			xor eax, 040000h    // flip AC bit in EFLAGS 
			push eax        // save new EFLAGS value on stack 
			popfd         // replace current EFLAGS value 
			pushfd          // get new EFLAGS 
			pop eax         // store new EFLAGS in EAX 
			cmp eax, ebx      // can’t toggle AC bit, processor=80386 
			jz label386     // jump if 80386 processor 
			push ebx
			popfd         // restore AC bit in EFLAGS 

			// Checking for ability to set/clear ID flag (Bit 21) in EFLAGS
			// which indicates the presence of a processor with the CPUID
			// instruction.
			pushfd            // save EFLAGS to stack 
			pop eax           // store EFLAGS in EAX 
			mov ebx, eax      // save in EBX for testing later 
			xor eax, 0200000h // flip bit 21 in EFLAGS 
			push eax          // save new EFLAGS value on stack 
			popfd             // replace current EFLAGS value 
			pushfd            // get new EFLAGS 
			pop eax           // store new EFLAGS in EAX 
			cmp eax, ebx      // see if bit 21 has changed 
			jz labelNoCPUID   // CPUID is not present 

			mov EAX, 1
			cpuid
			mov g_dwFeatures, EDX
			
			// check for 3DNow!
			mov eax, 080000000h   // query for extended functions
			cpuid         // get extended function limit
			cmp eax, 080000001h   /* functions up to 80000001h must be present */
			jb labelNoExtended      /* 80000001h is not available */
			mov eax, 080000001h   /* setup extended function 1 */
			cpuid         /* call the function */
			mov g_dwFeaturesEx, edx /* bit 31 will be set for 3D Now! support*/

labelNoExtended:
			test g_dwFeatures, g_featureSSE
			jz labelNoSSE
			// SSE is present. to check for OS support...
			xorps xmm0, xmm0
			// if we got here safely after xorps, it only can mean we have SSE
			//or g_dwFeatures, g_featureSSE
			jmp labelEndDetect
label386:
labelNoCPUID:
labelNoSSE:
labelEndDetect:
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
    // OS doesn't support some of the instructions we executed..

		if(_exception_code () == STATUS_ILLEGAL_INSTRUCTION)
			g_dwFeatures &= ~g_featureSSE;
	}

	if ((g_dwFeatures & (g_featureFXSR|g_featureSSE)) == (g_featureFXSR|g_featureSSE))
	{
		unsigned nMXCSR;
		__try
		{
			_asm
			{
				stmxcsr nMXCSR
				or nMXCSR, 0x8000
				ldmxcsr nMXCSR
				or nMXCSR, 0x40
				ldmxcsr nMXCSR
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
#endif
}



}

#if !defined(LINUX) && defined(_CPU_X86) 
// given the array of matrices, calculates the min/max
// of their positions, and puts them into the min and max Vec3d
// NOTE: the matrix array must be aligned on 16-byte boundary
void getBBoxSSE (const Matrix44* pBones, unsigned numBones, CryAABB* pBBox)
{
	assert (numBones > 0);
	_asm
	{
		mov EBX, pBones
		movaps xmm0, [EBX+0x30]
		movaps xmm1, xmm0
		mov EDX, pBBox
		add EBX, 0x70 // now EBX points to the next bone matrix
		mov ECX, numBones
		dec ECX
		jz label_End

		label_Start:
		movaps xmm2, [EBX]
		minps xmm0, xmm2
		maxps xmm1, xmm2
		add EBX, 0x40
		loop label_Start

		label_End:
		//SSE_MOVSS(EDX,xmm0)
		movss [EDX], xmm0       
		shufps xmm0,xmm0, 0xE5	 
		movss [EDX+4], xmm0		 
		shufps xmm0,xmm0, 0xE6   
		movss [EDX+8], xmm0		 
		//SSE_MOVSS(EDX+0x0C,xmm1)
		movss [EDX+0x0C], xmm1
		shufps xmm1,xmm1, 0xE5	 
		movss [EDX+0x0C+4], xmm1		 
		shufps xmm1,xmm1, 0xE6   
		movss [EDX+0x0C+8], xmm1
	}
}

// given the array of matrices, calculates the min/max
// of their positions, and puts them into the min and max Vec3d
// NOTE: the matrix array must be aligned on 16-byte boundary
void getBBoxSSE (const Matrix44* pBones, const CryBBoxA16* pBoneBBox, unsigned numBones, CryAABB* pBBox)
{
	assert (numBones > 0);
	_asm
	{
		mov EBX, pBones
			movaps xmm0, [EBX+0x30]
			movaps xmm1, xmm0
				mov EDX, pBBox
				add EBX, 0x70 // now EBX points to the next bone matrix
				mov ECX, numBones
				dec ECX
				jz label_End

label_Start:
			movaps xmm2, [EBX]
			minps xmm0, xmm2
				maxps xmm1, xmm2
				add EBX, 0x40
				loop label_Start

label_End:
			//SSE_MOVSS(EDX,xmm0)
			movss [EDX], xmm0       
				shufps xmm0,xmm0, 0xE5	 
				movss [EDX+4], xmm0		 
				shufps xmm0,xmm0, 0xE6   
				movss [EDX+8], xmm0		 
				//SSE_MOVSS(EDX+0x0C,xmm1)
				movss [EDX+0x0C], xmm1
				shufps xmm1,xmm1, 0xE5	 
				movss [EDX+0x0C+4], xmm1		 
				shufps xmm1,xmm1, 0xE6   
				movss [EDX+0x0C+8], xmm1
	}
}
#endif

// packs the array of Vec3dA16 into Vec3d's
// nCount - number of vertices
// pData  - [IN] Vec3dA16, [OUT] Vec3d
void packVec3d16 (void* pData, unsigned nCount)
{
#if !defined(LINUX) && defined(_CPU_X86)
	_asm
	{
		mov ESI, pData
		mov EDI, ESI

		add ESI, 0x10
		add EDI, 0xC
		
		mov ECX, nCount
		dec ECX
		jz endLoop

startLoop:
		mov EAX, [ESI]
		mov [EDI], EAX
		mov EBX, [ESI+4]
		mov [EDI+4], EBX
		mov EDX, [ESI+8]
		mov [EDI+8], EDX

		add ESI, 0x10
		add EDI, 0xC
		loop startLoop
endLoop:
	}
#else
	float* pTo = (float*)pData + 3; 
	float* pFrom = (float*)pData + 4;
	for (unsigned i = 1; i < nCount; ++i)
	{
		pTo[0] = pFrom[0];
		pTo[1] = pFrom[1];
		pTo[2] = pFrom[2];
		pTo += 3;
		pFrom += 4;
	}
#endif
}


#if !defined(LINUX) && defined(_CPU_X86)
__declspec(naked) void PIII_Mult00_4x4_4x4( float *src1, float *src2, float *dst)
{
	__asm
	{
	mov edx, dword ptr [esp+4] ; src1
	mov eax, dword ptr [esp+0Ch] ; dst
	mov ecx, dword ptr [esp+8] ; src2
	movss xmm0, dword ptr [edx]
	movaps xmm1, xmmword ptr [ecx]
	shufps xmm0, xmm0, 0
	movss xmm2, dword ptr [edx+4]
	mulps xmm0, xmm1
	shufps xmm2, xmm2, 0
	movaps xmm3, xmmword ptr [ecx+10h]
	movss xmm7, dword ptr [edx+8]
	mulps xmm2, xmm3
	shufps xmm7, xmm7, 0
	addps xmm0, xmm2
	movaps xmm4, xmmword ptr [ecx+20h]
	movss xmm2, dword ptr [edx+0Ch]
	mulps xmm7, xmm4
	shufps xmm2, xmm2, 0
	addps xmm0, xmm7
	movaps xmm5, xmmword ptr [ecx+30h]
	movss xmm6, dword ptr [edx+10h]
	mulps xmm2, xmm5
	movss xmm7, dword ptr [edx+14h]
	shufps xmm6, xmm6, 0
	addps xmm0, xmm2
	shufps xmm7, xmm7, 0
	movlps qword ptr [eax], xmm0
	movhps qword ptr [eax+8], xmm0
	mulps xmm7, xmm3
	movss xmm0, dword ptr [edx+18h]
	mulps xmm6, xmm1
	shufps xmm0, xmm0, 0
	addps xmm6, xmm7
	mulps xmm0, xmm4
	movss xmm2, dword ptr [edx+24h]
	addps xmm6, xmm0
	movss xmm0, dword ptr [edx+1Ch]
	movss xmm7, dword ptr [edx+20h]
	shufps xmm0, xmm0, 0
	shufps xmm7, xmm7, 0
	mulps xmm0, xmm5
	mulps xmm7, xmm1
	addps xmm6, xmm0
	shufps xmm2, xmm2, 0
	movlps qword ptr [eax+10h], xmm6
	movhps qword ptr [eax+18h], xmm6
	mulps xmm2, xmm3
	movss xmm6, dword ptr [edx+28h]
	addps xmm7, xmm2
	shufps xmm6, xmm6, 0
	movss xmm2, dword ptr [edx+2Ch]
	mulps xmm6, xmm4
	shufps xmm2, xmm2, 0
	addps xmm7, xmm6
	mulps xmm2, xmm5
	movss xmm0, dword ptr [edx+34h]
	addps xmm7, xmm2
	shufps xmm0, xmm0, 0
	movlps qword ptr [eax+20h], xmm7
	movss xmm2, dword ptr [edx+30h]
	movhps qword ptr [eax+28h], xmm7
	mulps xmm0, xmm3
	shufps xmm2, xmm2, 0
	movss xmm6, dword ptr [edx+38h]
	mulps xmm2, xmm1
	shufps xmm6, xmm6, 0
	addps xmm2, xmm0
	mulps xmm6, xmm4
	movss xmm7, dword ptr [edx+3Ch]
	shufps xmm7, xmm7, 0
	addps xmm2, xmm6
	mulps xmm7, xmm5
	addps xmm2, xmm7
	movaps xmmword ptr [eax+30h], xmm2
	ret
	}
}
#endif

void multMatrix(float *product, const float *m1, const float *m2)
{
#if defined(LINUX) || !defined(_CPU_X86)
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

#else
  __asm
  {
    mov eax, m2;
    mov ecx, m1;
    mov edx, product;
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
  }
#endif
}

