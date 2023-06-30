//  __RAD16__ means 16 bit code (Win16)
//  __RAD32__ means 32 bit code (DOS, Win386, Win32s, Mac)

//  __RADDOS__ means DOS code (16 or 32 bit)
//  __RADWIN__ means Windows code (Win16, Win386, Win32s)
//  __RADWINEXT__ means Windows 386 extender (Win386)
//  __RADNT__ means Win32s code
//  __RADMAC__ means Macintosh
//  __RADCARBON__ means Carbon
//  __RADMACH__ means MachO
//  __RADXBOX__ means the XBox console
//  __RADNGC__ means the Nintendo GameCube

//  __RADX86__ means Intel x86
//  __RADMMX__ means Intel x86 MMX instructions are allowed
//  __RAD68K__ means 68K
//  __RADPPC__ means PowerPC

// __RADLITTLEENDIAN__ means processor is little-endian (x86)
// __RADBIGENDIAN__ means processor is big-endian (680x0, PPC)

#ifndef __RADBASEH__
  #define __RADBASEH__

  #define RADCOPYRIGHT "Copyright (C) 1994-2003, RAD Game Tools, Inc."

  #ifndef __RADRES__

    #if defined(GEKKO)

      #define __RADNGC__
      #define __RAD32__
      #define __RADPPC__
      #define __RADBIGENDIAN__
      #define RADINLINE inline

    #elif (defined(__MWERKS__) && !defined(__INTEL__)) || defined(__MRC__) || defined(THINK_C) || defined(powerc) || defined(macintosh) || defined(__powerc) || defined(__APPLE__) || defined(__MACH__)
      #define __RADMAC__
      #if defined(powerc) || defined(__powerc) || defined(__ppc__)
        #define __RADPPC__
      #else
        #define __RAD68K__
      #endif

      #define __RAD32__
      #define __RADBIGENDIAN__

      #if defined(__MWERKS__)
        #if (defined(__cplusplus) || ! __option(only_std_keywords))
          #define RADINLINE inline
        #endif
      #elif defined(__MRC__)
        #if defined(__cplusplus)
          #define RADINLINE inline
        #endif
      #elif defined(__GNUC__) || defined(__GNUG__)
        #define RADINLINE inline
        #define __RADMACH__
      #endif

      #ifdef __MACH__
        #define __RADMACH__
      #endif
      
      #ifdef TARGET_API_MAC_CARBON
        #if TARGET_API_MAC_CARBON
          #ifndef __RADCARBON__
            #define __RADCARBON__
          #endif
        #endif
      #endif

    #else

      #define __RADX86__
      #define __RADMMX__

      #ifdef __MWERKS__
        #define _WIN32
      #endif

      #ifdef __DOS__
        #define __RADDOS__
      #endif

      #ifdef __386__
        #define __RAD32__
      #endif

      #ifdef _Windows    //For Borland
        #ifdef __WIN32__
          #define WIN32
        #else
          #define __WINDOWS__
        #endif
      #endif

      #ifdef _WINDOWS    //For MS
        #ifndef _WIN32
          #define __WINDOWS__
        #endif
      #endif

      #ifdef _WIN32
        #ifdef _XBOX
          #define __RADXBOX__
        #else
          #define __RADNT__
        #endif
        #define __RADWIN__
        #define __RAD32__
      #else
        #ifdef __NT__
          #ifdef _XBOX
            #define __RADXBOX__
          #else
            #define __RADNT__
          #endif
          #define __RADWIN__
          #define __RAD32__
        #else
          #ifdef __WINDOWS_386__
            #define __RADWIN__
            #define __RADWINEXT__
            #define __RAD32__
          #else
            #ifdef __WINDOWS__
              #define __RADWIN__
              #define __RAD16__
            #else
              #ifdef WIN32
                #ifdef _XBOX
                  #define __RADXBOX__
                #else
                  #define __RADNT__
                #endif
                #define __RADWIN__
                #define __RAD32__
              #endif
            #endif
          #endif
        #endif
      #endif

      #define __RADLITTLEENDIAN__
      #ifdef __WATCOMC__
        #define RADINLINE
      #else
        #define RADINLINE __inline
      #endif
    #endif

    #if (!defined(__RADDOS__) && !defined(__RADWIN__) && !defined(__RADMAC__) && !defined(__RADNGC__) && !defined(__RADXBOX__))
      #error "RAD.H did not detect your platform.  Define __DOS__, __WINDOWS__, WIN32, macintosh, or powerc."
    #endif

    #ifdef __RADFINAL__
      #define RADTODO(str) { char __str[0]=str; }
    #else
      #define RADTODO(str)
    #endif

    #ifdef __RADNGC__

      #define RADLINK
      #define RADEXPLINK
      #define RADEXPFUNC RADDEFFUNC
      #define RADASMLINK
      #define PTR4

    #elif defined(__RADMAC__)

      // this define is for CodeWarrior 11's stupid new libs (even though
      //   we don't use longlong's).

      #define __MSL_LONGLONG_SUPPORT__

      #define RADLINK
      #define RADEXPLINK

      #ifdef __CFM68K__
        #ifdef __RADINDLL__
          #define RADEXPFUNC RADDEFFUNC __declspec(export)
        #else
          #define RADEXPFUNC RADDEFFUNC __declspec(import)
        #endif
      #else
        #define RADEXPFUNC RADDEFFUNC
      #endif
      #define RADASMLINK

    #else

      #ifdef __RADNT__
        #ifndef _WIN32
          #define _WIN32
        #endif
        #ifndef WIN32
          #define WIN32
        #endif
      #endif

      #ifdef __RADWIN__
        #ifdef __RAD32__
          #ifdef __RADXBOX__

             #define RADLINK __stdcall
             #define RADEXPLINK __stdcall
             #define RADEXPFUNC RADDEFFUNC

          #else
            #ifdef __RADNT__

              #define RADLINK __stdcall
              #define RADEXPLINK __stdcall

              #ifdef __RADINEXE__
                #define RADEXPFUNC RADDEFFUNC
              #else
                #ifndef __RADINDLL__
                  #define RADEXPFUNC RADDEFFUNC __declspec(dllimport)
                  #ifdef __BORLANDC__
                    #if __BORLANDC__<=0x460
                      #undef RADEXPFUNC
                      #define RADEXPFUNC RADDEFFUNC
                    #endif
                  #endif
                #else
                  #define RADEXPFUNC RADDEFFUNC __declspec(dllexport)
                #endif
              #endif
            #else
              #define RADLINK __pascal
              #define RADEXPLINK __far __pascal
              #define RADEXPFUNC RADDEFFUNC
            #endif
          #endif
        #else
          #define RADLINK __pascal
          #define RADEXPLINK __far __pascal __export
          #define RADEXPFUNC RADDEFFUNC
        #endif
      #else
        #define RADLINK __pascal
        #define RADEXPLINK __pascal
        #define RADEXPFUNC RADDEFFUNC
      #endif

      #define RADASMLINK __cdecl

    #endif

    #ifndef __RADXBOX__
      #ifdef __RADWIN__
        #ifndef _WINDOWS
          #define _WINDOWS
        #endif
      #endif
    #endif

    #ifndef RADDEFFUNC

      #ifdef __cplusplus
        #define RADDEFFUNC extern "C"
        #define RADDEFSTART extern "C" {
        #define RADDEFEND }
        #define RADDEFINEDATA extern "C"
        #define RADDECLAREDATA extern "C"
        #define RADDEFAULT( val ) =val
      #else
        #define RADDEFFUNC
        #define RADDEFSTART
        #define RADDEFEND
        #define RADDEFINEDATA
        #define RADDECLAREDATA extern
        #define RADDEFAULT( val )
      #endif

    #endif

    #ifdef __RADNGC__
      #define RAD_ATTRIBUTE_ALIGN(num) __attribute__ ((aligned (num)))
    #else
      #ifdef __RADX86__
        #ifdef __WATCOMC__
          #define RAD_ATTRIBUTE_ALIGN(num) 
        #else
          #define RAD_ATTRIBUTE_ALIGN(num) __declspec(align(num))
        #endif
      #else
        #define RAD_ATTRIBUTE_ALIGN(num) 
      #endif
    #endif

    #ifdef __RADX86__
      #ifdef __WATCOMC__
        #define RAD_ALIGN_TYPE double
        #define RAD_ALIGN_DEF 0.0
      #else
        #define RAD_ALIGN_TYPE double __declspec(align(8))
        #define RAD_ALIGN_DEF 0.0
      #endif
    #else
      #define RAD_ALIGN_TYPE double
      #define RAD_ALIGN_DEF 0.0
    #endif

    #define RAD_ALIGN_ADD_TYPE(var) RAD_ALIGN_TYPE var##align = RAD_ALIGN_DEF

    #define S8 signed char
    #define U8 unsigned char
    #define U32 unsigned long
    #define S32 signed long
    #define F32 float
    #define F64 double

    #if defined(__MWERKS__) || defined(__MRC__) || defined( GEKKO )
      #define U64 unsigned long long
      #define S64 signed long long
    #else
      #define U64 unsigned __int64
      #define S64 signed __int64
    #endif

    #ifdef __RAD32__
      #define PTR4
      #define U16 unsigned short
      #define S16 signed short
    #else
      #define PTR4 __far
      #define U16 unsigned int
      #define S16 signed int
    #endif

    #ifndef RAD_NO_LOWERCASE_TYPES

      #ifdef __RADNGC__

        // Unfortunately dolphin\types.h typedefs the
        // same types that we use.
        // So we use the typedefs for this platform.

        #include <dolphin\types.h>

      #else

        #define u8 U8
        #define s8 S8
        #define u16 U16
        #define s16 S16
        #define u32 U32
        #define s32 S32
        #define u64 U64
        #define s64 S64
        #define f32 F32
        #define f64 F64

      #endif

    #endif

  #endif

#endif
