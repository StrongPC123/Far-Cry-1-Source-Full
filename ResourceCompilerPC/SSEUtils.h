#ifndef _CRY_ANIMATION_SSE_UTILS_HDR_
#define _CRY_ANIMATION_SSE_UTILS_HDR_

#include "MathUtils.h"

#ifdef _CPU_X86
// given the array of matrices, calculates the min/max
// of their positions, and puts them into the min and max Vec3d
// NOTE: the matrix array must be aligned on 16-byte boundary
extern void getBBoxSSE (const Matrix44* pBones, unsigned numBones, CryAABB* pBBox);
extern void getBBoxSSE (const Matrix44* pBones, const CryBBoxA16* pBoneBBox, unsigned numBones, CryAABB* pBBox);
#endif

// packs the array of Vec3dA16 into Vec3d's
extern void packVec3d16 (void* pData, unsigned nCount);

namespace cpu
{
// detects CPU features (SSE) and perhaps sets up some
// pointers to functions
void detect ();

// the CPU feature set, as returned by CPUID(1):EDX
extern DWORD g_dwFeatures;

// the extended feature set, as returned by CPUID(0x80000001):EDX
extern DWORD g_dwFeaturesEx;

enum FeaturesEnum
{
	// Floating Point Unit On-Chip. The processor contains an x87 FPU.
	g_featureFPU  = 1,

	// Virtual 8086 Mode Enhancements. Virtual 8086 mode enhancements, including
	// CR4.VME for controlling the feature, CR4.PVI for protected mode virtual
	// interrupts, software interrupt indirection, expansion of the TSS with the software
	// indirection bitmap, and EFLAGS.VIF and EFLAGS.VIP flags.
	g_featureVME  = 1 << 1,


	// Debugging Extensions. Support for I/O breakpoints, including CR4.DE for
	// controlling the feature, and optional trapping of accesses to DR4 and DR5.
	g_featureDE   = 1 << 2,
	g_featureDebuggingExtensions = g_featureDE,

	// Page Size Extension. Large pages of size 4Mbyte are supported, including
	// CR4.PSE for controlling the feature, the defined dirty bit in PDE (Page Directory
	// Entries), optional reserved bit trapping in CR3, PDEs, and PTEs.
	g_featurePSE  = 1 << 3,	

	// Time Stamp Counter. The RDTSC instruction is supported, including CR4.TSD
	// for controlling privilege.
	g_featureTSC  = 1 << 4,

	// Model Specific Registers RDMSR and WRMSR Instructions. The RDMSR and
	// WRMSR instructions are supported. Some of the MSRs are implementation
	// dependent.
	g_featureMSR  = 1 << 5,

	// PAE Physical Address Extension. Physical addresses greater than 32 bits are
	// supported: extended page table entry formats, an extra level in the page
	// translation tables is defined, 2 Mbyte pages are supported instead of 4 Mbyte
	// pages if PAE bit is 1. The actual number of address bits beyond 32 is not defined,
	// and is implementation specific.
	g_featurePAE  = 1 << 6,

	// MCE Machine Check Exception. Exception 18 is defined for Machine Checks,
	// including CR4.MCE for controlling the feature. This feature does not define the
	// model-specific implementations of machine-check error logging, reporting, and
	// processor shutdowns. Machine Check exception handlers may have to depend on
	// processor version to do model specific processing of the exception, or test for the
	// presence of the Machine Check feature.
	g_featureMCE  = 1 << 7,

	// CMPXCHG8B Instruction. The compare-and-exchange 8 bytes (64 bits)
	// instruction is supported (implicitly locked and atomic).
	g_featureCX8  = 1 << 8,

	// APIC On-Chip. The processor contains an Advanced Programmable Interrupt
	// Controller (APIC), responding to memory mapped commands in the physical
	// address range FFFE0000H to FFFE0FFFH (by default - some processors permit
	// the APIC to be relocated).
	g_featureAPIC = 1 << 9,

	// SYSENTER and SYSEXIT Instructions. The SYSENTER and SYSEXIT and
	// associated MSRs are supported.
	g_featureSEP  = 1 << 11,

	// Memory Type Range Registers. MTRRs are supported. The MTRRcap MSR
	// contains feature bits that describe what memory types are supported, how many
	// variable MTRRs are supported, and whether fixed MTRRs are supported.
	g_featureMTRR = 1 << 12,

	// PTE Global Bit. The global bit in page directory entries (PDEs) and page table
	// entries (PTEs) is supported, indicating TLB entries that are common to different
	// processes and need not be flushed. The CR4.PGE bit controls this feature.
	g_featurePGE  = 1 << 13,

	// Machine Check Architecture. The Machine Check Architecture, which provides
	// a compatible mechanism for error reporting in P6 family, Pentium 4, and Intel
	// Xeon processors, and future processors, is supported. The MCG_CAP MSR
	// contains feature bits describing how many banks of error reporting MSRs are
	// supported.
	g_featureMCA  = 1 << 14,

	// Conditional Move Instructions. The conditional move instruction CMOV is
	// supported. In addition, if x87 FPU is present as indicated by the CPUID.FPU
	// feature bit, then the FCOMI and FCMOV instructions are supported
	g_featureCMOV = 1 << 15,

	//Page Attribute Table. Page Attribute Table is supported. This feature augments
	//the Memory Type Range Registers (MTRRs), allowing an operating system to
	//specify attributes of memory on a 4K granularity through a linear address.
	g_featurePAT  = 1 << 16,

	//32-Bit Page Size Extension. Extended 4-MByte pages that are capable of
	//addressing physical memory beyond 4 GBytes are supported. This feature
	//indicates that the upper four bits of the physical address of the 4-MByte page is
	//encoded by bits 13-16 of the page directory entry.
	g_featurePSE36 = 1 << 17,

	// PSN Processor Serial Number. The processor supports the 96-bit processor
	// identification number feature and the feature is enabled.
	g_featurePSN   = 1 << 18,

	// CLFLUSH Instruction. CLFLUSH Instruction is supported.
	g_featureCLFSH   = 1 << 19,

	// Reserved
	g_featureReserved   = 1 << 20,

	// DS Debug Store. The processor supports the ability to write debug information into a
	// memory resident buffer. This feature is used by the branch trace store (BTS) and
	// precise event-based sampling (PEBS) facilities (see Chapter 15, Debugging and
	// Performance Monitoring, in the IA-32 Intel Architecture Software Developer’s
	// Manual, Volume 3).
	g_featureDS   = 1 << 21,

	// Thermal Monitor and Software Controlled Clock Facilities. The processor
	// implements internal MSRs that allow processor temperature to be monitored and
	// processor performance to be modulated in predefined duty cycles under software
	// control.
	g_featureACPI = 1 << 22,

	// Intel MMX Technology. The processor supports the Intel MMX technology.
	g_featureMMX  = 1 << 23,

	// FXSAVE and FXRSTOR Instructions. The FXSAVE and FXRSTOR instructions
	// are supported for fast save and restore of the floating point context. Presence of
	// this bit also indicates that CR4.OSFXSR is available for an operating system to
	// indicate that it supports the FXSAVE and FXRSTOR instructions
	g_featureFXSR = 1 << 24, 

	// SSE. The processor supports the SSE extensions.
	g_featureSSE   = 1 << 25,

	// SSE2. The processor supports the SSE2 extensions.
	g_featureSSE2  = 1 << 26,

	// Self Snoop. The processor supports the management of conflicting memory
	// types by performing a snoop of its own cache structure for transactions issued to
	// the bus
	g_featureSS    = 1 << 27,

	// Hyper-Threading Technology. The processor implements Hyper-Threading
	// Technology.
	g_featureHTT   = 1 << 28,

	// TM Thermal Monitor. The processor implements the thermal monitor automatic
	// thermal control circuitry (TCC).
	g_featureTM    = 1 << 29,

	// Reserved
	g_featureReserved30   = 1 << 30,

	// PBE Pending Break Enable. The processor supports the use of the FERR#/PBE# pin
	// when the processor is in the stop-clock state (STPCLK# is asserted) to signal the
	// processor that an interrupt is pending and that the processor should return to
	// normal operation to handle the interrupt. Bit 10 (PBE enable) in the
	// IA32_MISC_ENABLE MSR enables this capability.
	g_featurePBE   = 1 << 31,

	// this is 3D Now! instruction set bit
	g_featureEx3DNow = 0x80000000
};

inline int hasSSE()
{
#ifdef _AMD64_
	return g_featureSSE;
#else
	return (g_dwFeatures & g_featureSSE);
#endif
}

// does the machine have the RDTSC instruction?
inline int hasRDTSC()
{
	return (g_dwFeatures & g_featureTSC);
}

// does the machine have 3D Now instruction set support?
inline int has3DNow()
{
	return (g_dwFeaturesEx & g_featureEx3DNow);
}

extern void logCaps();
}


#endif