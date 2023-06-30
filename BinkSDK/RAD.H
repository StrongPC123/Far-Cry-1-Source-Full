#ifndef __RADH__
#define __RADH__

#define RADCOPYRIGHT "Copyright (C) 1994-2002, RAD Game Tools, Inc."

#ifndef __RADRES__

//  __RAD16__ means 16 bit code (Win16)
//  __RAD32__ means 32 bit code (DOS, Win386, Win32s, Mac)

//  __RADDOS__ means DOS code (16 or 32 bit)
//  __RADWIN__ means Windows code (Win16, Win386, Win32s)
//  __RADWINEXT__ means Windows 386 extender (Win386)
//  __RADNT__ means Win32s code
//  __RADMAC__ means Macintosh
//  __RADXBOX__ means the XBox console
//  __RADNGC__ means the Nintendo GameCube

//  __RADX86__ means Intel x86
//  __RADMMX__ means Intel x86 MMX instructions are allowed
//  __RAD68K__ means 68K
//  __RADPPC__ means PowerPC

// __RADLITTLEENDIAN__ means processor is little-endian (x86)
// __RADBIGENDIAN__ means processor is big-endian (680x0, PPC)

// @cdep pre $set(INCs,$INCs -I$clipfilename($file))

#ifndef __RADBASEH__
#include "radbase.h"
#endif

RADDEFSTART

/* 32 bit implementations */

#ifdef __RAD32__

  #ifdef __RADNGC__

    #define radmemcpy   memcpy

    #define radstrcat strcat

    #define radstrcpy   strcpy

    #define radstrlen   strlen

    #define radmemset   memset

    #define radmemcpydb memmove

    #define radstrcmp strcmp

    #define BreakPoint() asm(" .long 0x00000001")

    void radmemset16(void* dest,U16 value,U32 size);

  #elif defined(__RADMAC__)

    #define radstrcpy strcpy

    #define radstrcat strcat

    #define radmemcpy(dest,source,size) BlockMoveData((Ptr)(source),(Ptr)(dest),size)

    #define radmemcpydb(dest,source,size) BlockMoveData((Ptr)(source),(Ptr)(dest),size)

    #define radmemcmp memcmp

    #define radmemset memset

    #define radstrlen strlen

    #define radstrcmp strcmp

    #define BreakPoint() DebugStr("\pBreakPoint() was called")

    U32 RADCycleTimerAvail(void);

    void RADCycleTimerStartAddr(U32* addr);

    U32 RADCycleTimerDeltaAddr(U32* addr);

    void RADCycleTimerStartAddr64(U64* addr);

    void RADCycleTimerDeltaAddr64(U64* addr);

    #define RADCycleTimerStart(var) RADCycleTimerStartAddr(&var)

    #define RADCycleTimerDelta(var) RADCycleTimerDeltaAddr(&var)

    #define RADCycleTimerStart64(var) RADCycleTimerStartAddr64(&var)

    #define RADCycleTimerDelta64(var) RADCycleTimerDeltaAddr64(&var)

    void radmemset16(void* dest,U16 value,U32 size);

  #else

    #ifdef __WATCOMC__

      U32 DOSOut(const char* str);
      #pragma aux DOSOut = "cld" "mov ecx,0xffffffff" "xor eax,eax" "mov edx,edi" "repne scasb" "not ecx" "dec ecx" "mov ebx,1" "mov ah,0x40" "int 0x21" parm [EDI] modify [EAX EBX ECX EDX EDI] value [ecx];

      void DOSOutNum(const char* str,U32 len);
      #pragma aux DOSOutNum = "mov ah,0x40" "mov ebx,1" "int 0x21" parm [edx] [ecx] modify [eax ebx];

      U32 ErrOut(const char* str);
      #pragma aux ErrOut = "cld" "mov ecx,0xffffffff" "xor eax,eax" "mov edx,edi" "repne scasb" "not ecx" "dec ecx" "xor ebx,ebx" "mov ah,0x40" "int 0x21" parm [EDI] modify [EAX EBX ECX EDX EDI] value [ecx];

      void ErrOutNum(const char* str,U32 len);
      #pragma aux ErrOutNum = "mov ah,0x40" "xor ebx,ebx" "int 0x21" parm [edx] [ecx] modify [eax ebx];

      void radmemset16(void* dest,U16 value,U32 size);
      #pragma aux radmemset16 = "cld" "mov bx,ax" "shl eax,16" "mov ax,bx" "mov bl,cl" "shr ecx,1" "rep stosd" "mov cl,bl" "and cl,1" "rep stosw" parm [EDI] [EAX] [ECX] modify [EAX EDX EBX ECX EDI];

      void radmemset(void* dest,U8 value,U32 size);
      #pragma aux radmemset = "cld" "mov ah,al" "mov bx,ax" "shl eax,16" "mov ax,bx" "mov bl,cl" "shr ecx,2" "and bl,3" "rep stosd" "mov cl,bl" "rep stosb" parm [EDI] [AL] [ECX] modify [EAX EDX EBX ECX EDI];

      void radmemset32(void* dest,U32 value,U32 size);
      #pragma aux radmemset32 = "cld" "rep stosd" parm [EDI] [EAX] [ECX] modify [EAX EDX EBX ECX EDI];

      void radmemcpy(void* dest,const void* source,U32 size);
      #pragma aux radmemcpy = "cld" "mov bl,cl" "shr ecx,2" "rep movsd" "mov cl,bl" "and cl,3" "rep movsb" parm [EDI] [ESI] [ECX] modify [EBX ECX EDI ESI];

      void __far *radfmemcpy(void __far* dest,const void __far* source,U32 size);
      #pragma aux radfmemcpy = "cld" "push es" "push ds" "mov es,cx" "mov ds,dx" "mov ecx,eax" "shr ecx,2" "rep movsd" "mov cl,al" "and cl,3" "rep movsb" "pop ds" "pop es" parm [CX EDI] [DX ESI] [EAX] modify [ECX EDI ESI] value [CX EDI];

      void radmemcpydb(void* dest,const void* source,U32 size);  //Destination bigger
      #pragma aux radmemcpydb = "std" "mov bl,cl" "lea esi,[esi+ecx-4]" "lea edi,[edi+ecx-4]" "shr ecx,2" "rep movsd" "and bl,3" "jz dne" "add esi,3" "add edi,3" "mov cl,bl" "rep movsb" "dne:" "cld" parm [EDI] [ESI] [ECX] modify [EBX ECX EDI ESI];

      char* radstrcpy(void* dest,const void* source);
      #pragma aux radstrcpy = "cld" "mov edx,edi" "lp:" "mov al,[esi]" "inc esi" "mov [edi],al" "inc edi" "cmp al,0" "jne lp" parm [EDI] [ESI] modify [EAX EDX EDI ESI] value [EDX];

      char __far* radfstrcpy(void __far* dest,const void __far* source);
      #pragma aux radfstrcpy = "cld" "push es" "push ds" "mov es,cx" "mov ds,dx" "mov edx,edi" "lp:" "lodsb" "stosb" "test al,0xff" "jnz lp" "pop ds" "pop es" parm [CX EDI] [DX ESI] modify [EAX EDX EDI ESI] value [CX EDX];

      char* radstpcpy(void* dest,const void* source);
      #pragma aux radstpcpy = "cld" "lp:" "mov al,[esi]" "inc esi" "mov [edi],al" "inc edi" "cmp al,0" "jne lp" "dec edi" parm [EDI] [ESI] modify [EAX EDI ESI] value [EDI];

      char* radstpcpyrs(void* dest,const void* source);
      #pragma aux radstpcpyrs = "cld" "lp:" "mov al,[esi]" "inc esi" "mov [edi],al" "inc edi" "cmp al,0" "jne lp" "dec esi" parm [EDI] [ESI] modify [EAX EDI ESI] value [ESI];

      U32 radstrlen(const void* dest);
      #pragma aux radstrlen = "cld" "mov ecx,0xffffffff" "xor eax,eax" "repne scasb" "not ecx" "dec ecx" parm [EDI] modify [EAX ECX EDI] value [ECX];

      char* radstrcat(void* dest,const void* source);
      #pragma aux radstrcat = "cld" "mov ecx,0xffffffff" "mov edx,edi" "xor eax,eax" "repne scasb" "dec edi" "lp:" "lodsb" "stosb" "test al,0xff" "jnz lp" \
      parm [EDI] [ESI] modify [EAX ECX EDI ESI] value [EDX];

      char* radstrchr(const void* dest,char chr);
      #pragma aux radstrchr = "cld" "lp:" "lodsb" "cmp al,dl" "je fnd" "cmp al,0" "jnz lp" "mov esi,1" "fnd:" "dec esi" parm [ESI] [DL] modify [EAX ESI] value [esi];

      S8 radmemcmp(const void* s1,const void* s2,U32 len);
      #pragma aux radmemcmp = "cld" "rep cmpsb" "setne al" "jbe end" "neg al" "end:"  parm [EDI] [ESI] [ECX] modify [ECX EDI ESI];

      S8 radstrcmp(const void* s1,const void* s2);
      #pragma aux radstrcmp = "lp:" "mov al,[esi]" "mov ah,[edi]" "cmp al,ah" "jne set" "cmp al,0" "je set" "inc esi" "inc edi" "jmp lp" "set:" "setne al" "jbe end" "neg al" "end:" \
      parm [EDI] [ESI] modify [EAX EDI ESI];

      S8 radstricmp(const void* s1,const void* s2);
      #pragma aux radstricmp = "lp:" "mov al,[esi]" "mov ah,[edi]" "cmp al,'a'" "jb c1" "cmp al,'z'" "ja c1" "sub al,32" "c1:" "cmp ah,'a'" "jb c2" "cmp ah,'z'" "ja c2" "sub ah,32" "c2:" "cmp al,ah" "jne set" "cmp al,0" "je set" \
     "inc esi" "inc edi" "jmp lp" "set:" "setne al" "jbe end" "neg al" "end:" \
      parm [EDI] [ESI] modify [EAX EDI ESI];

      S8 radstrnicmp(const void* s1,const void* s2,U32 len);
      #pragma aux radstrnicmp = "lp:" "mov al,[esi]" "mov ah,[edi]" "cmp al,'a'" "jb c1" "cmp al,'z'" "ja c1" "sub al,32" "c1:" "cmp ah,'a'" "jb c2" "cmp ah,'z'" "ja c2" "sub ah,32" "c2:" "cmp al,ah" "jne set" "cmp al,0" "je set" \
      "dec ecx" "jz set" "inc esi" "inc edi" "jmp lp" "set:" "setne al" "jbe end" "neg al" "end:" \
      parm [EDI] [ESI] [ECX] modify [EAX ECX EDI ESI];

      char* radstrupr(void* s1);
      #pragma aux radstrupr = "mov ecx,edi" "lp:" "mov al,[edi]" "cmp al,'a'" "jb c1" "cmp al,'z'" "ja c1" "sub [edi],32" "c1:" "inc edi" "cmp al,0" "jne lp" parm [EDI] modify [EAX EDI] value [ecx];

      char* radstrlwr(void* s1);
      #pragma aux radstrlwr = "mov ecx,edi" "lp:" "mov al,[edi]" "cmp al,'A'" "jb c1" "cmp al,'Z'" "ja c1" "add [edi],32" "c1:" "inc edi" "cmp al,0" "jne lp" parm [EDI] modify [EAX EDI] value [ecx];

      U32 radstru32(const void* dest);
        #pragma aux radstru32 = "cld" "xor ecx,ecx" "xor ebx,ebx" "xor edi,edi" "lodsb" "cmp al,45" "jne skip2" "mov edi,1" "jmp skip" "lp:" "mov eax,10" "mul ecx" "lea ecx,[eax+ebx]" \
        "skip:" "lodsb" "skip2:" "cmp al,0x39" "ja dne" "cmp al,0x30" "jb dne" "mov bl,al" "sub bl,0x30" "jmp lp" "dne:" "test edi,1" "jz pos" "neg ecx" "pos:" \
        parm [ESI] modify [EAX EBX EDX EDI ESI] value [ecx];

      U16 GetDS();
      #pragma aux GetDS = "mov ax,ds" value [ax];

      #ifdef __RADWINEXT__

        #define _16To32(ptr16) ((void*)(((GetSelectorBase((U16)(((U32)(ptr16))>>16))+((U16)(U32)(ptr16)))-GetSelectorBase(GetDS()))))

      #endif

      #ifndef __RADWIN__
        #define int86 int386
        #define int86x int386x
      #endif

      #define u32regs x
      #define u16regs w

    #else

      #define radstrcpy strcpy
      #define radstrcat strcat
      #define radmemcpy memcpy
      #define radmemcpydb memmove
      #define radmemcmp memcmp
      #define radmemset memset
      #define radstrlen strlen
      #define radstrchr strchr
      #define radtoupper toupper
      #define radstru32(s) ((U32)atol(s))
      #define radstricmp _stricmp
      #define radstrcmp strcmp
      #define radstrupr _strupr
      #define radstrlwr _strlwr
      #define BreakPoint() __asm {int 3}
      #define DOSOut(str)

      #ifdef _MSC_VER

        #pragma warning( disable : 4035)

        typedef char* RADPCHAR;

        U8 __inline radinp(U16 p) {
          __asm {
            mov dx,[p]
            in al,dx
          }
        }

        void __inline radoutp(U16 p,U8 v) {
          __asm {
            mov dx,[p]
            mov al,[v]
            out dx,al
          }
        }

        RADPCHAR __inline radstpcpy(char* p1, char* p2) {
          __asm {
             mov edx,[p1] 
             mov ecx,[p2]
             cld
            lp:
             mov al,[ecx] 
             inc ecx 
             mov [edx],al
             inc edx
             cmp al,0 
             jne lp 
             dec edx
             mov eax,edx
          } 
        }

        RADPCHAR __inline radstpcpyrs(char* p1, char* p2) {
          __asm {
            mov edx,[p1]
            mov ecx,[p2]
            cld
           lp:
            mov al,[ecx]
            inc ecx
            mov [edx],al
            inc edx
            cmp al,0 
            jne lp
            dec ecx
            mov eax,ecx 
          }
        }

        void __inline radmemset16(void* dest,U16 value,U32 sizeb) {
          __asm {
            mov edi,[dest]
            mov ax,[value] 
            mov ecx,[sizeb]
            shl eax,16 
            cld 
            mov ax,[value] 
            mov bl,cl 
            shr ecx,1 
            rep stosd
            mov cl,bl 
            and cl,1 
            rep stosw
          }
        }

        void __inline radmemset32(void* dest,U32 value,U32 sizeb) {
          __asm {
            mov edi,[dest]
            mov eax,[value]
            mov ecx,[sizeb]
            cld
            rep stosd
          }
        }

        U32 __inline RADCycleTimerAvail(void)
        {
          U32 rdtscavail=(U32)-1;
          __try
          {
            __asm
            {
#ifdef __MWERKS__
              rdtsc
#else
#if _MSC_VER<=1100
              __emit 0xf
              __emit 0x31
#else
              rdtsc
#endif
#endif
            }
            rdtscavail=1;
          }
          __except (1)
          {
            rdtscavail=(U32)-1;
          }
          return rdtscavail;
        }

        void __inline RADCycleTimerStartAddr(U32* addr)
        {
          __asm {
            mov ecx,[addr]
#ifdef __MWERKS__
            rdtsc
#else
#if _MSC_VER<=1100
            __emit 0xf
            __emit 0x31
#else
            rdtsc
#endif
#endif
            mov [ecx],eax
          }
        }

        U32 __inline RADCycleTimerDeltaAddr(U32* addr)
        {
          __asm {
#ifdef __MWERKS__
            rdtsc
#else
#if _MSC_VER<=1100
            __emit 0xf
            __emit 0x31
#else
            rdtsc
#endif
#endif
            mov ecx,[addr]
            mov edx,eax
            sub eax,[ecx]
            mov [ecx],eax
          }
        }

        void __inline RADCycleTimerStartAddr64(U64* addr)
        {
          __asm {
            mov ecx,[addr]
#ifdef __MWERKS__
            rdtsc
#else
#if _MSC_VER<=1100
            __emit 0xf
            __emit 0x31
#else
            rdtsc
#endif
#endif
            mov [ecx],eax
            mov [ecx+4],edx
          }
        }

        void __inline RADCycleTimerDeltaAddr64(U64* addr)
        {
          __asm {
#ifdef __MWERKS__
            rdtsc
#else
#if _MSC_VER<=1100
            __emit 0xf
            __emit 0x31
#else
            rdtsc
#endif
#endif
            mov ecx,[addr]
            sub eax,[ecx]
            sbb edx,[ecx+4]
            mov [ecx],eax
            mov [ecx+4],edx
          }
        }

        #define RADCycleTimerStart(var) RADCycleTimerStartAddr(&var)
        #define RADCycleTimerDelta(var) RADCycleTimerDeltaAddr(&var)

        #define RADCycleTimerStart64(var) RADCycleTimerStartAddr64(&var)
        #define RADCycleTimerDelta64(var) RADCycleTimerDeltaAddr64(&var)

        #pragma warning( default : 4035)

      #endif

    #endif

  #endif

#else

  #ifdef __WATCOMC__

    U32 DOSOut(const char far* dest);
    #pragma aux DOSOut = "cld" "and edi,0xffff" "mov dx,di" "mov ecx,0xffffffff" "xor eax,eax" 0x67 "repne scasb" "not ecx" "dec ecx" "mov bx,1" "push ds" "push es" "pop ds" "mov ah,0x40" "int 0x21" "pop ds" "movzx eax,cx" "shr ecx,16" \
       parm [ES DI] modify [AX BX CX DX DI ES] value [CX AX];

    void DOSOutNum(const char far* str,U16 len);
    #pragma aux DOSOutNum = "push ds" "mov ds,cx" "mov cx,bx" "mov ah,0x40" "mov bx,1" "int 0x21" "pop ds" parm [cx dx] [bx] modify [ax bx cx];

    U32 ErrOut(const char far* dest);
    #pragma aux ErrOut = "cld" "and edi,0xffff" "mov dx,di" "mov ecx,0xffffffff" "xor eax,eax" 0x67 "repne scasb" "not ecx" "dec ecx" "xor bx,bx" "push ds" "push es" "pop ds" "mov ah,0x40" "int 0x21" "pop ds" "movzx eax,cx" "shr ecx,16" \
       parm [ES DI] modify [AX BX CX DX DI ES] value [CX AX];

    void ErrOutNum(const char far* str,U16 len);
    #pragma aux ErrOutNum = "push ds" "mov ds,cx" "mov cx,bx" "mov ah,0x40" "xor bx,bx" "int 0x21" "pop ds" parm [cx dx] [bx] modify [ax bx cx];

    void radmemset(void far *dest,U8 value,U32 size);
    #pragma aux radmemset = "cld" "and edi,0ffffh" "shl ecx,16" "mov cx,bx" "mov ah,al" "mov bx,ax" "shl eax,16" "mov ax,bx" "mov bl,cl" "shr ecx,2" 0x67 "rep stosd" "mov cl,bl" "and cl,3" "rep stosb" parm [ES DI] [AL] [CX BX];

    void radmemset16(void far* dest,U16 value,U32 size);
    #pragma aux radmemset16 = "cld" "and edi,0ffffh" "shl ecx,16" "mov cx,bx" "mov bx,ax" "shl eax,16" "mov ax,bx" "mov bl,cl" "shr ecx,1" "rep stosd" "mov cl,bl" "and cl,1" "rep stosw" parm [ES DI] [AX] [CX BX];

    void radmemcpy(void far* dest,const void far* source,U32 size);
    #pragma aux radmemcpy = "cld" "push ds" "mov ds,dx" "and esi,0ffffh" "and edi,0ffffh" "shl ecx,16" "mov cx,bx" "shr ecx,2" 0x67 "rep movsd" "mov cl,bl" "and cl,3" "rep movsb" "pop ds" parm [ES DI] [DX SI] [CX BX] modify [CX SI DI ES];

    S8 radmemcmp(const void far* s1,const void far* s2,U32 len);
    #pragma aux radmemcmp = "cld" "push ds" "mov ds,dx" "shl ecx,16" "mov cx,bx" "rep cmpsb" "setne al" "jbe end" "neg al" "end:" "pop ds"  parm [ES DI] [DX SI] [CX BX] modify [CX SI DI ES];

    char far* radstrcpy(void far* dest,const void far* source);
    #pragma aux radstrcpy = "cld" "push ds" "mov ds,dx" "and esi,0xffff" "and edi,0xffff" "mov dx,di" "lp:" "lodsb" "stosb" "test al,0xff" "jnz lp" "pop ds" parm [ES DI] [DX SI] modify [AX DX DI SI ES] value [es dx];

    char far* radstpcpy(void far* dest,const void far* source);
    #pragma aux radstpcpy = "cld" "push ds" "mov ds,dx" "and esi,0xffff" "and edi,0xffff" "lp:" "lodsb" "stosb" "test al,0xff" "jnz lp" "dec di" "pop ds" parm [ES DI] [DX SI] modify [DI SI ES] value [es di];

    U32 radstrlen(const void far* dest);
    #pragma aux radstrlen = "cld" "and edi,0xffff" "mov ecx,0xffffffff" "xor eax,eax" 0x67 "repne scasb" "not ecx" "dec ecx" "movzx eax,cx" "shr ecx,16" parm [ES DI] modify [AX CX DI ES] value [CX AX];

    char far* radstrcat(void far* dest,const void far* source);
    #pragma aux radstrcat = "cld" "and edi,0xffff" "mov ecx,0xffffffff" "and esi,0xffff" "push ds" "mov ds,dx" "mov dx,di" "xor eax,eax" 0x67 "repne scasb" "dec edi" "lp:" "lodsb" "stosb" "test al,0xff" "jnz lp" "pop ds" \
      parm [ES DI] [DX SI] modify [AX CX DI SI ES] value [es dx];

    char far* radstrchr(const void far* dest,char chr);
    #pragma aux radstrchr = "cld" "lp:" 0x26 "lodsb" "cmp al,dl" "je fnd" "cmp al,0" "jnz lp" "xor ax,ax" "mov es,ax" "mov si,1" "fnd:" "dec si" parm [ES SI] [DL] modify [AX SI ES] value [es si];

    S8 radstricmp(const void far* s1,const void far* s2);
    #pragma aux radstricmp = "and edi,0xffff" "push ds" "mov ds,dx" "and esi,0xffff" "lp:" "mov al,[esi]" "mov ah,[edi]" "cmp al,'a'" "jb c1" "cmp al,'z'" "ja c1" "sub al,32" "c1:" \
      "cmp ah,'a'" "jb c2" "cmp ah,'z'" "ja c2" "sub ah,32" "c2:" "cmp al,ah" "jne set" "cmp al,0" "je set" \
      "inc esi" "inc edi" "jmp lp" "set:" "setne al" "jbe end" "neg al" "end:" "pop ds" \
      parm [ES DI] [DX SI] modify [AX DI SI];

    U32 radstru32(const void far* dest);
    #pragma aux radstru32 = "cld" "xor ecx,ecx" "xor ebx,ebx" "xor edi,edi" 0x26 "lodsb" "cmp al,45" "jne skip2" "mov edi,1" "jmp skip" "lp:" "mov eax,10" "mul ecx" "lea ecx,[eax+ebx]" \
      "skip:" 0x26 "lodsb" "skip2:" "cmp al,0x39" "ja dne" "cmp al,0x30" "jb dne" "mov bl,al" "sub bl,0x30" "jmp lp" "dne:" "test edi,1" "jz pos" "neg ecx" "pos:" \
      "movzx eax,cx" "shr ecx,16" parm [ES SI] modify [AX BX DX DI SI] value [cx ax];

  #endif

#endif

RADDEFEND

#define u32neg1 ((U32)(S32)-1)
#define RAD_align(var) var; U8 junk##var[4-(sizeof(var)&3)];
#define RAD_align_after(var) U8 junk##var[4-(sizeof(var)&3)]={0};
#define RAD_align_init(var,val) var=val; U8 junk##var[4-(sizeof(var)&3)]={0};
#define RAD_align_array(var,num) var[num]; U8 junk##var[4-(sizeof(var)&3)];
#define RAD_align_string(var,str) char var[]=str; U8 junk##var[4-(sizeof(var)&3)]={0};


#ifdef __RADMAC__
  #pragma export on
#endif
RADEXPFUNC void RADEXPLINK RADSetMemory(RADMEMALLOC a,RADMEMFREE f);
#ifdef __RADMAC__
  #pragma export off
#endif

RADEXPFUNC void PTR4* RADEXPLINK radmalloc(U32 numbytes);
RADEXPFUNC void RADEXPLINK radfree(void PTR4* ptr);

#ifdef __RADDOS__

  RADDEFSTART
  extern void* RADTimerSetupAddr;
  extern void* RADTimerReadAddr;
  extern void* RADTimerDoneAddr;
  RADDEFEND

  typedef void RADEXPLINK (*RADTimerSetupType)(void);
  typedef U32 RADEXPLINK (*RADTimerReadType)(void);
  typedef void RADEXPLINK (*RADTimerDoneType)(void);

  #define RADTimerSetup() ((RADTimerSetupType)(RADTimerSetupAddr))()
  #define RADTimerRead() ((RADTimerReadType)(RADTimerReadAddr))()
  #define RADTimerDone() ((RADTimerDoneType)(RADTimerDoneAddr))()

#else

  #define RADTimerSetup()
  #define RADTimerDone()

  #if (defined(__RAD16__) || defined(__RADWINEXT__))

    #define RADTimerRead timeGetTime

  #else

    RADEXPFUNC U32 RADEXPLINK RADTimerRead(void);

  #endif

#endif


#ifdef __WATCOMC__

  char bkbhit();
  #pragma aux bkbhit = "mov ah,1" "int 0x16" "lahf" "shr eax,14" "and eax,1" "xor al,1" ;

  char bgetch();
  #pragma aux bgetch = "xor ah,ah" "int 0x16" "test al,0xff" "jnz done" "mov al,ah" "or al,0x80" "done:" modify [AX];

  void BreakPoint();
  #pragma aux BreakPoint = "int 3";

  U8 radinp(U16 p);
  #pragma aux radinp = "in al,dx" parm [DX];

  U8 radtoupper(U8 p);
  #pragma aux radtoupper = "cmp al,'a'" "jb c1" "cmp al,'z'" "ja c1" "sub al,32" "c1:" parm [al] value [al];

  void radoutp(U16 p,U8 v);
  #pragma aux radoutp = "out dx,al" parm [DX] [AL];

#else

// for multi-processor machines

#ifdef __RADNT__
  #define LockedIncrement(var) __asm { lock inc [var] }
  #define LockedDecrement(var) __asm { lock dec [var] }
  void __inline LockedIncrementFunc(void PTR4* var) {
    __asm {
      mov eax,[var]
      lock inc [eax]
    }
  }

  void __inline LockedDecrementFunc(void PTR4* var) {
    __asm {
       mov eax,[var]
       lock dec [eax]
    }
  }

  void __inline LockedAddFunc(void PTR4* var,U32 val) {
    __asm {
      mov eax,[var]
      mov edx,[val]
      lock add [eax],edx
    }
  }

#else

  #if defined(__RADCARBON__)

    #define LockedIncrement(var) OTAtomicAdd32(  1, (SInt32*) &(var) )
    #define LockedDecrement(var) OTAtomicAdd32( -1, (SInt32*) &(var) )

    #define LockedIncrementFunc(ptr) OTAtomicAdd32( 1, (SInt32*) (ptr) )
    #define LockedDecrementFunc(ptr) OTAtomicAdd32( -1, (SInt32*) (ptr) )
    
    #define LockedAddFunc(ptr,val) OTAtomicAdd32( val, (SInt32*) (ptr) )

  #elif defined(__RADMAC__) || defined(__RADNGC__)

    #define LockedIncrement(var) {++(var);}
    #define LockedDecrement(var) {--(var);}

    #define LockedIncrementFunc(ptr) {++(*((U32*)(ptr)));}
    #define LockedDecrementFunc(ptr) {--(*((U32*)(ptr)));}
    
    #define LockedAddFunc(ptr,val) {(*((U32*)(ptr)))+=(val);}

  #else

    #define LockedIncrement(var) __asm { inc [var] }
    #define LockedDecrement(var) __asm { dec [var] }
    void __inline LockedIncrementFunc(void PTR4* var) { __asm { mov eax,[var]
                                                                inc [eax] } }
    void __inline LockedDecrementFunc(void PTR4* var) { __asm { mov eax,[var]
                                                                dec [eax] } }
    void __inline LockedAddFunc(void PTR4* var,U32 val) {
      __asm {
        mov eax,[var]
        mov edx,[val]
        add [eax],edx
      }
    }

  #endif

#endif

#endif

#endif

#endif

