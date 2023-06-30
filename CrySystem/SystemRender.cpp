
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
// 
//	File: SystemRender.cpp
//  Description: CryENGINE system core 
// 
//	History:
//	-Jan 6,2004: split from system.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "System.h"

#ifndef _XBOX
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
#else
#include <xtl.h>
#endif

#include <IRenderer.h>
#include "Log.h"
#include "XConsole.h"
#include <I3DEngine.h>
#include <IAISystem.h>

#include "CrySizerStats.h"
#include "CrySizerImpl.h"

#ifdef WIN32
#include "luadebugger/luadbginterface.h"
#include "luadebugger/LuaDbg.h"
#endif

#if !defined(LINUX)
#include <ddraw.h>
extern HRESULT GetDXVersion( DWORD* pdwDirectXVersion, TCHAR* strDirectXVersion, int cchDirectXVersion );
#endif

extern int g_nPrecaution;

#if !defined(LINUX)
/////////////////////////////////////////////////////////////////////////////////
int CSystem::AutoDetectRenderer(char *Vendor, char *Device)
{
  HRESULT hr;
  LPDIRECTDRAW7 pDD;
  HINSTANCE hDDInstance;
  int nRenderer = R_DX9_RENDERER;

  // Dynamically load the DLL.
  typedef HRESULT (WINAPI *DD_CREATE_FUNC)(GUID* lpGUID, void* lplpDD, REFIID iid,IUnknown* pUnkOuter);
  typedef HRESULT (WINAPI *DD_ENUM_FUNC  )(LPDDENUMCALLBACKEXA lpCallback,void* lpContext,DWORD dwFlags);
  DD_CREATE_FUNC ddCreateFunc;
  DD_ENUM_FUNC ddEnumFunc;
  hDDInstance = CryLoadLibrary(_T("ddraw.dll"));
  if( hDDInstance == NULL )
    return nRenderer;
  ddCreateFunc = (DD_CREATE_FUNC)GetProcAddress( hDDInstance, "DirectDrawCreateEx"     );
  ddEnumFunc   = (DD_ENUM_FUNC  )GetProcAddress( hDDInstance, "DirectDrawEnumerateExA" );
  if( !ddCreateFunc || !ddEnumFunc )
    return nRenderer;
  if(FAILED(hr=ddCreateFunc(NULL,&pDD,IID_IDirectDraw7,NULL)))
  {
    FreeLibrary(hDDInstance);
    return nRenderer;
  }

  DDDEVICEIDENTIFIER2 DeviceIdentifier;
  memset(&DeviceIdentifier,0,sizeof(DeviceIdentifier));
  if(FAILED(hr=pDD->GetDeviceIdentifier(&DeviceIdentifier,0)))
  {
    pDD->Release();
    FreeLibrary(hDDInstance);
    return nRenderer;
  }
  bool bUnsupported = false;
  switch(DeviceIdentifier.dwVendorId)
  {
    // 3DFX
    case 0x1142:
    case 0x10d9:
    case 0x121a:
      strcpy(Vendor, "3DFX");
      bUnsupported = true;
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0001: strcpy(Device, "Voodoo 1"); break;
        case 0x0002: strcpy(Device, "Voodoo 2"); break;
        case 0x0003:
        case 0x0004: strcpy(Device, "Banshee");  break;
        case 0x0005: strcpy(Device, "Voodoo 3"); break;
        case 0x0007:
        case 0x0009: strcpy(Device, "Voodoo 4"); break;
        case 0x643d: strcpy(Device, "Rush (Alliance)"); break;
        case 0x8626: strcpy(Device, "Rush (Macronix)"); break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x104a:
      strcpy(Vendor, "ST Microelectronics");
      bUnsupported = true;
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0010: strcpy(Device, "Kyro I/II"); break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x1002:
      strcpy(Vendor, "ATI");
      nRenderer = R_DX9_RENDERER;
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x4158: strcpy(Device, "Mach 32"); bUnsupported = true; break;
        case 0x4337: strcpy(Device, "IGP 340M"); bUnsupported = true; break;
        case 0x4354: strcpy(Device, "Mach 64");  bUnsupported = true; break;
        case 0x4358: strcpy(Device, "Mach 64"); bUnsupported = true; break;
        case 0x4554: strcpy(Device, "Mach 64"); bUnsupported = true; break;
        case 0x4654: strcpy(Device, "Mach 64"); bUnsupported = true; break;
        case 0x4742: strcpy(Device, "Rage Pro"); bUnsupported = true; break;
        case 0x4744: strcpy(Device, "Rage Pro"); bUnsupported = true; break;
        case 0x4747: strcpy(Device, "Rage Pro"); bUnsupported = true; break;
        case 0x4749: strcpy(Device, "Rage Pro"); bUnsupported = true; break;
        case 0x474c: strcpy(Device, "Rage XC"); bUnsupported = true; break;
        case 0x474d: strcpy(Device, "Rage XL"); bUnsupported = true; break;
        case 0x474e: strcpy(Device, "Rage XC"); bUnsupported = true; break;
        case 0x474f: strcpy(Device, "Rage XL"); bUnsupported = true; break;
        case 0x4750: strcpy(Device, "Rage Pro"); bUnsupported = true; break;
        case 0x4751: strcpy(Device, "Rage Pro"); bUnsupported = true; break;
        case 0x4752: strcpy(Device, "Rage XL"); bUnsupported = true; break;
        case 0x4753: strcpy(Device, "Rage XC"); bUnsupported = true; break;
        case 0x4754: strcpy(Device, "Rage II"); bUnsupported = true; break;
        case 0x4755: strcpy(Device, "Rage II+"); bUnsupported = true; break;
        case 0x4756: strcpy(Device, "Rage IIC"); bUnsupported = true; break;
        case 0x4757: strcpy(Device, "Rage IIC"); bUnsupported = true; break;
        case 0x4758: strcpy(Device, "Mach 64"); bUnsupported = true; break;
        case 0x4759: strcpy(Device, "Rage IIC"); bUnsupported = true; break;
        case 0x475a: strcpy(Device, "Rage IIC"); bUnsupported = true; break;
        case 0x4c42: strcpy(Device, "Rage LT Pro"); bUnsupported = true; break;
        case 0x4c44: strcpy(Device, "Rage LT Pro"); bUnsupported = true; break;
        case 0x4c47: strcpy(Device, "Rage LT"); bUnsupported = true; break;
        case 0x4c49: strcpy(Device, "Rage LT Pro"); bUnsupported = true; break;
        case 0x4c50: strcpy(Device, "Rage LT Pro"); bUnsupported = true; break;
        case 0x4c51: strcpy(Device, "Rage LT Pro"); bUnsupported = true; break;
        case 0x4c45: strcpy(Device, "Rage Mobility"); bUnsupported = true; break;
        case 0x4c46: strcpy(Device, "Rage Mobility"); bUnsupported = true; break;
        case 0x4c4d: strcpy(Device, "Rage Mobility"); bUnsupported = true; break;
        case 0x4c4e: strcpy(Device, "Rage Mobility"); bUnsupported = true; break;
        case 0x4c52: strcpy(Device, "Rage Mobility"); bUnsupported = true; break;
        case 0x4c53: strcpy(Device, "Rage Mobility"); bUnsupported = true; break;
        case 0x4c54: strcpy(Device, "Rage Mobility"); bUnsupported = true; break;
        case 0x4d46: strcpy(Device, "Rage Mobility 128"); bUnsupported = true; break;
        case 0x4d4c: strcpy(Device, "Rage Mobility 128"); bUnsupported = true; break;
        case 0x5041: strcpy(Device, "Rage 128 Pro PCI"); bUnsupported = true; break;
        case 0x5042: strcpy(Device, "Rage 128 Pro AGP2X"); bUnsupported = true; break;
        case 0x5043: strcpy(Device, "Rage 128 Pro AGP4X"); bUnsupported = true; break;
        case 0x5044: strcpy(Device, "Rage 128 Pro PCI TMDS"); bUnsupported = true; break;
        case 0x5045: strcpy(Device, "Rage 128 Pro AGP2X TMDS"); bUnsupported = true; break;
        case 0x5046: strcpy(Device, "Rage 128 Pro AGP4X TMDS"); bUnsupported = true; break;
        case 0x5047: strcpy(Device, "Rage 128 Pro PCI"); bUnsupported = true; break;
        case 0x5048: strcpy(Device, "Rage 128 Pro AGP2X"); bUnsupported = true; break;
        case 0x5049: strcpy(Device, "Rage 128 Pro AGP4X"); bUnsupported = true; break;
        case 0x504a: strcpy(Device, "Rage 128 Pro PCI TMDS"); bUnsupported = true; break;
        case 0x504b: strcpy(Device, "Rage 128 Pro AGP2X TMDS"); bUnsupported = true; break;
        case 0x504c: strcpy(Device, "Rage 128 Pro AGP4X TMDS"); bUnsupported = true; break;
        case 0x504d: strcpy(Device, "Rage 128 Pro PCI"); bUnsupported = true; break;
        case 0x504e: strcpy(Device, "Rage 128 Pro AGP2X"); bUnsupported = true; break;
        case 0x504f: strcpy(Device, "Rage 128 Pro AGP4X"); bUnsupported = true; break;
        case 0x5050: strcpy(Device, "Rage 128 Pro PCI TMDS"); bUnsupported = true; break;
        case 0x5051: strcpy(Device, "Rage 128 Pro AGP2X TMDS"); bUnsupported = true; break;
        case 0x5052: strcpy(Device, "Rage 128 Pro AGP4X TMDS"); bUnsupported = true; break;
        case 0x5053: strcpy(Device, "Rage 128 Pro PCI"); bUnsupported = true; break;
        case 0x5054: strcpy(Device, "Rage 128 Pro AGP2X"); bUnsupported = true; break;
        case 0x5055: strcpy(Device, "Rage 128 Pro AGP4X"); bUnsupported = true; break;
        case 0x5056: strcpy(Device, "Rage 128 Pro PCI TMDS"); bUnsupported = true; break;
        case 0x5057: strcpy(Device, "Rage 128 Pro AGP2X TMDS"); bUnsupported = true; break;
        case 0x5058: strcpy(Device, "Rage 128 Pro AGP4X TMDS"); bUnsupported = true; break;
        case 0x5245: strcpy(Device, "Rage 128 GL PCI"); bUnsupported = true; break;
        case 0x5246: strcpy(Device, "Rage 128 GL AGP"); bUnsupported = true; break;
        case 0x5247: strcpy(Device, "Rage 128"); bUnsupported = true; break;
        case 0x524b: strcpy(Device, "Rage 128 VR PCI"); bUnsupported = true; break;
        case 0x524c: strcpy(Device, "Rage 128 VR AGP"); bUnsupported = true; break;
        case 0x5345: strcpy(Device, "Rage 128 4X PCI"); bUnsupported = true; break;
        case 0x5346: strcpy(Device, "Rage 128 4X AGP2X"); bUnsupported = true; break;
        case 0x5347: strcpy(Device, "Rage 128 4X AGP4X"); bUnsupported = true; break;
        case 0x5348: strcpy(Device, "Rage 128 4X"); bUnsupported = true; break;
        case 0x534b: strcpy(Device, "Rage 128 4X PCI"); bUnsupported = true; break;
        case 0x534c: strcpy(Device, "Rage 128 4X AGP2X"); bUnsupported = true; break;
        case 0x534d: strcpy(Device, "Rage 128 4X AGP4X"); bUnsupported = true; break;
        case 0x534e: strcpy(Device, "Rage 128 4X"); bUnsupported = true; break;
        case 0x5354: strcpy(Device, "Mach 64"); bUnsupported = true; break;
        case 0x5446: strcpy(Device, "Rage 128 Pro Ultra GL AGP"); bUnsupported = true; break;
        case 0x544c: strcpy(Device, "Rage 128 Pro Ultra VR AGP"); bUnsupported = true; break;
        case 0x5452: strcpy(Device, "Rage 128 Pro Ultra4XL VR-R AGP"); bUnsupported = true; break;
        case 0x5453: strcpy(Device, "Rage 128 Pro"); bUnsupported = true; break;
        case 0x5454: strcpy(Device, "Rage 128 Pro"); bUnsupported = true; break;
        case 0x5455: strcpy(Device, "Rage 128 Pro"); bUnsupported = true; break;
        case 0x5654: strcpy(Device, "Mach 64"); bUnsupported = true; break;
        case 0x5655: strcpy(Device, "Mach 64"); bUnsupported = true; break;
        case 0x5656: strcpy(Device, "Mach 64"); bUnsupported = true; break;
        case 0x4c57: strcpy(Device, "Radeon Mobility 7500"); bUnsupported = true; break;
        case 0x4c58: strcpy(Device, "Radeon Mobility 7500"); bUnsupported = true; break;
        case 0x4c59: strcpy(Device, "Radeon Mobility VE"); bUnsupported = true; break;
        case 0x4c5a: strcpy(Device, "Radeon Mobility VE"); bUnsupported = true; break;

        case 0x4c64: strcpy(Device, "Radeon Mobility 9000");  break;
        case 0x4c66: strcpy(Device, "Radeon Mobility 9000");  break;
        case 0x4966: strcpy(Device, "Radeon 9000");  break;
        case 0x496e: strcpy(Device, "Radeon 9000 - Secondary");  break;
        case 0x514d: strcpy(Device, "Radeon 9100");  break;
        case 0x5834: strcpy(Device, "Radeon 9100 IGP");  break;
        case 0x4242: strcpy(Device, "Radeon 8500 DV");  break;
        case 0x4152: strcpy(Device, "Radeon 9600");  break;
        case 0x4172: strcpy(Device, "Radeon 9600 - Secondary");  break;
        case 0x4164: strcpy(Device, "Radeon 9500 - Secondary");  break;
        case 0x4144: strcpy(Device, "Radeon 9500");  break;
        case 0x4e45: strcpy(Device, "Radeon 9500 Pro / 9700");  break;
        case 0x4150: strcpy(Device, "Radeon 9600 Pro");  break;
        case 0x4151: strcpy(Device, "Radeon 9600");  break;
        case 0x4170: strcpy(Device, "Radeon 9600 Pro - Secondary");  break;
        case 0x4171: strcpy(Device, "Radeon 9600 - Secondary");  break;
        case 0x4e46: strcpy(Device, "Radeon 9600 TX");  break;
        case 0x4e66: strcpy(Device, "Radeon 9600 TX - Secondary");  break;
        case 0x4e44: strcpy(Device, "Radeon 9700 Pro");  break;
        case 0x4e64: strcpy(Device, "Radeon 9700 Pro - Secondary");  break;
        case 0x4e65: strcpy(Device, "Radeon 9500 Pro / 9700 - Secondary");  break;
        case 0x4e49: strcpy(Device, "Radeon 9800");  break;
        case 0x4e69: strcpy(Device, "Radeon 9800 - Secondary");  break;
        case 0x4148: strcpy(Device, "Radeon 9800");  break;
        case 0x4168: strcpy(Device, "Radeon 9800 - Secondary");  break;
        case 0x4e48: strcpy(Device, "Radeon 9800 Pro");  break;
        case 0x4e68: strcpy(Device, "Radeon 9800 Pro - Secondary");  break;
        case 0x4e4a: strcpy(Device, "Radeon 9800 XT");  break;
        case 0x4e6a: strcpy(Device, "Radeon 9800 XT - Secondary");  break;
        case 0x5960: strcpy(Device, "Radeon 9200 Pro");  break;
        case 0x5940: strcpy(Device, "Radeon 9200 Pro - Secondary");  break;
        case 0x5961: strcpy(Device, "Radeon 9200");  break;
        case 0x5941: strcpy(Device, "Radeon 9200 - Secondary");  break;
        case 0x5964: strcpy(Device, "Radeon 9200SE");  break;
        case 0x5144: strcpy(Device, "Radeon 7200");  break;
        case 0x5145: strcpy(Device, "Radeon");  break;
        case 0x5146: strcpy(Device, "Radeon");  break;
        case 0x5147: strcpy(Device, "Radeon");  break;
        case 0x5148: strcpy(Device, "Radeon FireGL");  break;
        case 0x5157: strcpy(Device, "Radeon 7500");  break;
        case 0x5159: strcpy(Device, "Radeon 7000 VE");  break;
        case 0x515a: strcpy(Device, "Radeon VE");  break;
        case 0x516c: strcpy(Device, "Radeon");  break;
        case 0x514c: strcpy(Device, "Radeon 8500");  break;
        case 0x514e: strcpy(Device, "Radeon 8500");  break;
        case 0x514f: strcpy(Device, "Radeon 8500");  break;
        case 0x4136: strcpy(Device, "IGP 320");  break;
        case 0x4137: strcpy(Device, "IGP 340");  break;

        case 0x4A49: strcpy(Device, "Radeon X800 Pro");  break;
        case 0x4A4A: strcpy(Device, "Radeon X800 SE");  break;
        case 0x4A4B: strcpy(Device, "Radeon X800");  break;
        case 0x4A4C: strcpy(Device, "Radeon X800 Series");  break;
        case 0x4A50: strcpy(Device, "Radeon X800 XT");  break;
        case 0x4A69: strcpy(Device, "Radeon X800 Pro Secondary");  break;
        case 0x4A6A: strcpy(Device, "Radeon X800 SE Secondary");  break;
        case 0x4A6b: strcpy(Device, "Radeon X800 Secondary");  break;
        case 0x4A6C: strcpy(Device, "Radeon X800 Series Secondary");  break;
        case 0x4A70: strcpy(Device, "Radeon X800 XT Secondary");  break;

        default: strcpy(Device, "Unknown");
      }
  	  break;

    case 0x104c:
    case 0x10ba:
    case 0x3d3d:
    case 0x1048:
      strcpy(Vendor, "3D Labs");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0001: strcpy(Device, "GLiNT 300SX"); bUnsupported = true; break;
        case 0x0002: strcpy(Device, "GLiNT 500TX"); bUnsupported = true; break;
        case 0x0003: strcpy(Device, "GLiNT"); bUnsupported = true; break;
        case 0x0004: strcpy(Device, "Permedia"); bUnsupported = true; break;
        case 0x0005: strcpy(Device, "Permedia"); bUnsupported = true; break;
        case 0x0006: strcpy(Device, "GLiNT MX"); bUnsupported = true; break;
        case 0x0007: strcpy(Device, "Permedia 2"); bUnsupported = true; break;
        case 0x0008: strcpy(Device, "GLiNT G1"); bUnsupported = true; break;
        case 0x0009: strcpy(Device, "Permedia 2"); bUnsupported = true; break;
        case 0x000b: strcpy(Device, "Oxygen Series R3"); bUnsupported = true; break;
        case 0x000d: strcpy(Device, "Oxygen Series R4"); bUnsupported = true; break;
        case 0x000e: strcpy(Device, "Oxygen Series Gamma2"); bUnsupported = true; break;
        case 0x0100: strcpy(Device, "Permedia 2"); bUnsupported = true; break;
        case 0x0301: strcpy(Device, "Permedia 2"); bUnsupported = true; break;
        case 0x1004: strcpy(Device, "Permedia"); bUnsupported = true; break;
        case 0x3d04: strcpy(Device, "Permedia 1"); bUnsupported = true; break;
        case 0x3d07: strcpy(Device, "Permedia 2"); bUnsupported = true; break;
        case 0x8901: strcpy(Device, "GLiNT"); bUnsupported = true; break;

        case 0x000a: strcpy(Device, "Permedia 3"); break;
        case 0x000c: strcpy(Device, "Permedia 4"); break;
        default: strcpy(Device, "Unknown");
      }
  	  break;

    case 0x1039:
      strcpy(Vendor, "SiS");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0204: strcpy(Device, "6215"); bUnsupported = true; break;
        case 0x0205: strcpy(Device, "6205"); bUnsupported = true; break;
        case 0x0305: strcpy(Device, "305"); bUnsupported = true; break;
        case 0x6306: strcpy(Device, "530"); bUnsupported = true; break;
        case 0x6326: strcpy(Device, "6326"); bUnsupported = true; break;
        case 0x6325: strcpy(Device, "650"); bUnsupported = true; break;

        case 0x0325: strcpy(Device, "315");       break;
        case 0x0330: strcpy(Device, "Xabre 330"); break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x5333:
      strcpy(Vendor, "S3");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x9102: strcpy(Device, "Savage 2000"); bUnsupported = true; break;
        case 0x8a20: strcpy(Device, "Savage 3D"); bUnsupported = true; break;
        case 0x8a21: strcpy(Device, "Savage 3D S3"); bUnsupported = true; break;
        case 0x8a22: strcpy(Device, "Savage 3D S4"); bUnsupported = true; break;
        case 0x8a23: strcpy(Device, "Savage 3D S4"); bUnsupported = true; break;
        case 0x8a25: strcpy(Device, "Savage4 ProSavage"); bUnsupported = true; break;
        case 0x8a26: strcpy(Device, "ProSavage"); bUnsupported = true; break;
        case 0x8c10: strcpy(Device, "Savage MX"); bUnsupported = true; break;
        case 0x8c12: strcpy(Device, "Savage IX"); bUnsupported = true; break;
        case 0x8c22: strcpy(Device, "SuperSavage 128 MX"); bUnsupported = true; break;
        case 0x8c2a: strcpy(Device, "SuperSavage 128 IX"); bUnsupported = true; break;
        case 0x8c2b: strcpy(Device, "SuperSavage 128 IX DDR"); bUnsupported = true; break;
        case 0x8c2c: strcpy(Device, "SuperSavage IX"); bUnsupported = true; break;
        case 0x8c2d: strcpy(Device, "SuperSavage IX DDR"); bUnsupported = true; break;
        case 0x8c2e: strcpy(Device, "SuperSavage IXC SDR"); bUnsupported = true; break;
        case 0x8c2f: strcpy(Device, "SuperSavage IXC DDR"); bUnsupported = true; break;
        case 0x8d04: strcpy(Device, "ProSavage"); bUnsupported = true; break;
        case 0x5631: strcpy(Device, "Virge"); bUnsupported = true; break;
        case 0x8811: strcpy(Device, "Trio 64"); bUnsupported = true; break;
        case 0x8812: strcpy(Device, "Trio 64"); bUnsupported = true; break;
        case 0x8814: strcpy(Device, "Trio 64 Plus"); bUnsupported = true; break;
        case 0x8815: strcpy(Device, "Aurora 128"); bUnsupported = true; break;
        case 0x883d: strcpy(Device, "Virge"); bUnsupported = true; break;
        case 0x8880: strcpy(Device, "Virge"); bUnsupported = true; break;
        case 0x88c0: strcpy(Device, "Virge"); bUnsupported = true; break;
        case 0x88c1: strcpy(Device, "Virge"); bUnsupported = true; break;
        case 0x88d0: strcpy(Device, "Virge"); bUnsupported = true; break;
        case 0x88d1: strcpy(Device, "Virge"); bUnsupported = true; break;
        case 0x88f0: strcpy(Device, "Virge"); bUnsupported = true; break;
        case 0x8901: strcpy(Device, "Trio 64 DX"); bUnsupported = true; break;
        case 0x8904: strcpy(Device, "Trio 3D"); bUnsupported = true; break;
        case 0x8a01: strcpy(Device, "Virge DXGX"); bUnsupported = true; break;
        case 0x8a10: strcpy(Device, "Virge GX2"); bUnsupported = true; break;
        case 0x8a13: strcpy(Device, "Trio3D"); bUnsupported = true; break;
        case 0x8c00: strcpy(Device, "Virge MX"); bUnsupported = true; break;
        case 0x8c01: strcpy(Device, "Virge MX"); bUnsupported = true; break;
        case 0x8c02: strcpy(Device, "Virge MXC"); bUnsupported = true; break;
        case 0x8c03: strcpy(Device, "Virge MX"); bUnsupported = true; break;
        case 0x8d02: strcpy(Device, "Graphics Twister"); bUnsupported = true; break;
        default: strcpy(Device, "Unknown");
      }
  	  break;

    case 0x102b:
      strcpy(Vendor, "Matrox");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0518: strcpy(Device, "Millennium"); bUnsupported = true; break;
        case 0x0519: strcpy(Device, "Millennium"); bUnsupported = true; break;
        case 0x051a: strcpy(Device, "Mystique"); bUnsupported = true; break;
        case 0x051b: strcpy(Device, "Millennium II"); bUnsupported = true; break;
        case 0x051f: strcpy(Device, "Millennium II"); bUnsupported = true; break;
        case 0x0d10: strcpy(Device, "Mystique"); bUnsupported = true; break;
        case 0x1000: strcpy(Device, "G100 PCI"); bUnsupported = true; break;
        case 0x1001: strcpy(Device, "G100 AGP"); bUnsupported = true; break;
        case 0x0520: strcpy(Device, "G200 PCI"); bUnsupported = true; break;
        case 0x0521: strcpy(Device, "G200 AGP"); bUnsupported = true; break;
        case 0x1525: strcpy(Device, "Fusion G450"); bUnsupported = true; break;
        case 0x0525: strcpy(Device, "G400/450"); bUnsupported = true; break;
        case 0x2007: strcpy(Device, "Mistral"); bUnsupported = true; break;

        case 0x2527: strcpy(Device, "G550"); break;
        case 0x1527: strcpy(Device, "Fusion G800"); break;
        case 0x0527: strcpy(Device, "Parhelia 128"); break;
        default: strcpy(Device, "Unknown");
      }
  	  break;

    case 0x14Af:
      strcpy(Vendor, "Guillemot");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x5810: strcpy(Device, "TNT2"); bUnsupported = true; break;
        case 0x5820: strcpy(Device, "TNT2 Ultra"); bUnsupported = true; break;
        case 0x5620: strcpy(Device, "TNT2 M64"); bUnsupported = true; break;
        case 0x5020: strcpy(Device, "GeForce 256"); bUnsupported = true; break;
        case 0x5008: strcpy(Device, "TNT Vanta"); bUnsupported = true; break;
        case 0x4D20: strcpy(Device, "TNT2 M64"); bUnsupported = true; break;
        default: strcpy(Device, "Unknown");
      }
  	  break;

    case 0x10b4:
    case 0x12d2:
    case 0x10de:
      strcpy(Vendor, "nVidia");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x1b1d: strcpy(Device, "Riva 128"); bUnsupported = true; break;
        case 0x0008: strcpy(Device, "NV 1"); bUnsupported = true; break;
        case 0x0009: strcpy(Device, "NV 1"); bUnsupported = true; break;
        case 0x0010: strcpy(Device, "NV 2"); bUnsupported = true; break;
        case 0x0018: strcpy(Device, "Riva 128"); bUnsupported = true; break;
        case 0x0019: strcpy(Device, "Riva 128 ZX"); bUnsupported = true; break;
        case 0x0020: strcpy(Device, "TNT"); bUnsupported = true; break;
        case 0x002e: strcpy(Device, "TNT Vanta"); bUnsupported = true; break;
        case 0x002f: strcpy(Device, "TNT Vanta"); bUnsupported = true; break;
        case 0x00a0: strcpy(Device, "TNT2 Aladdin"); bUnsupported = true; break;
        case 0x0028: strcpy(Device, "Riva TNT2/TNT2 Pro"); bUnsupported = true; break;
        case 0x0029: strcpy(Device, "TNT2 Ultra"); bUnsupported = true; break;
        case 0x002a: strcpy(Device, "TNT2"); bUnsupported = true; break;
        case 0x002b: strcpy(Device, "TNT2"); bUnsupported = true; break;
        case 0x002c: strcpy(Device, "TNT Vanta/Vanta LT"); bUnsupported = true; break;
        case 0x002d: strcpy(Device, "TNT2 M64/M64 Pro"); bUnsupported = true; break;

		case 0x0040: strcpy(Device, "GeForce 6800 Ultra"); break;
		case 0x0041: strcpy(Device, "GeForce 6800"); break;
        case 0x0042:
        case 0x0043:
        case 0x0044:
        case 0x0045:
        case 0x0046:
        case 0x0047:
        case 0x0048:
        case 0x004a:
        case 0x004b:
        case 0x004c:
        case 0x004d:
        case 0x004f:
                     strcpy(Device, "NV40");  break;

		case 0x0049:
		case 0x004e:
			strcpy(Device, "NV40GL");  break;

		case 0x00f9: strcpy(Device, "GeForce 6800 Ultra"); break;

        case 0x00fa: strcpy(Device, "GeForce PCX 5750"); break;
        case 0x00fb: strcpy(Device, "GeForce PCX 5900"); break;
        case 0x00fc: strcpy(Device, "GeForce PCX 5300"); break;
        case 0x00fd: strcpy(Device, "Quadro PCI-E Series"); break;
		case 0x00fe: strcpy(Device, "Quadro FX 1300"); break;
        case 0x00ff: strcpy(Device, "GeForce PCX 4300"); break;

        case 0x0100: strcpy(Device, "GeForce 256");  break;
        case 0x0101: strcpy(Device, "GeForce 256 DDR");  break;
        case 0x0102: strcpy(Device, "GeForce 256 Ultra");  break;
        case 0x0103: strcpy(Device, "GeForce 256 Quadro");  break;
        case 0x0110: strcpy(Device, "GeForce2 MX/MX 400");  break;
        case 0x0111: strcpy(Device, "GeForce2 MX 100/200");  break;
        case 0x0112: strcpy(Device, "GeForce2 Go");  break;
        case 0x0113: strcpy(Device, "Quadro2 MXR/EX");  break;
        case 0x0150: strcpy(Device, "GeForce2 GTS/GeForce2 Pro");  break;
        case 0x0151: strcpy(Device, "GeForce2 Ti");  break;
        case 0x0152: strcpy(Device, "GeForce2 Ultra");  break;
        case 0x0153: strcpy(Device, "Quadro2 Pro");  break;
        case 0x0170: strcpy(Device, "GeForce4 MX 460");  break;
        case 0x0171: strcpy(Device, "GeForce4 MX 440");  break;
        case 0x0172: strcpy(Device, "GeForce4 MX 420");  break;
        case 0x0173: strcpy(Device, "GeForce4 MX 440-SE");  break;
        case 0x0174: strcpy(Device, "GeForce4 Go 440");  break;
        case 0x0175: strcpy(Device, "GeForce4 Go 420");  break;
        case 0x0176: strcpy(Device, "GeForce4 Go 420");  break;
        case 0x0177: strcpy(Device, "GeForce4 460 Go");  break;
        case 0x0178: strcpy(Device, "Quadro4 550 XGL");  break;
        case 0x0179: strcpy(Device, "GeForce4 Go 440");  break;
        case 0x017a: strcpy(Device, "Quadro NVS");  break;
        case 0x017b: strcpy(Device, "Quadro 550 XGL");  break;
        case 0x017c: strcpy(Device, "Quadro4 500 GoGL");  break;
        case 0x017d: strcpy(Device, "GeForce4 410 Go 16M");  break;
        case 0x0181: strcpy(Device, "GeForce4 MX 440 with AGP8X");  break;
        case 0x0182: strcpy(Device, "GeForce4 MX 440SE with AGP8X");  break;
        case 0x0183: strcpy(Device, "GeForce4 MX 420 with AGP8X");  break;
        case 0x0188: strcpy(Device, "Quadro4 580 XGL");  break;
        case 0x018a: strcpy(Device, "Quadro NVS with AGP8X");  break;
        case 0x018b: strcpy(Device, "Quadro4 380 XGL");  break;
        case 0x01a0: strcpy(Device, "GeForce2 Integrated GPU (nForce)");  break;
        case 0x01f0: strcpy(Device, "GeForce4 MX Integrated GPU (nForce2)");  break;

        case 0x0200: strcpy(Device, "GeForce3");  break;
        case 0x0201: strcpy(Device, "GeForce3 Ti200");  break;
        case 0x0202: strcpy(Device, "GeForce3 Ti500");  break;
        case 0x0203: strcpy(Device, "Quadro DCC");  break;
        case 0x0250: strcpy(Device, "GeForce4 Ti 4600");  break;
        case 0x0251: strcpy(Device, "GeForce4 Ti 4400");  break;
        case 0x0253: strcpy(Device, "GeForce4 Ti 4200");  break;
        case 0x0258: strcpy(Device, "Quadro4 900 XGL");  break;
        case 0x0259: strcpy(Device, "Quadro4 750 XGL");  break;
        case 0x025b: strcpy(Device, "Quadro4 700 XGL");  break;
        case 0x0280: strcpy(Device, "GeForce4 Ti 4800");  break;
        case 0x0281: strcpy(Device, "GeForce4 Ti4200 with AGP8X");  break;
        case 0x0282: strcpy(Device, "GeForce4 Ti 4800 SE");  break;
        case 0x0288: strcpy(Device, "Quadro4 980 XGL");  break;
        case 0x0289: strcpy(Device, "Quadro4 780 XGL");  break;
        case 0x02a0: strcpy(Device, "GeForce3 XBOX");  break;

        case 0x0301: strcpy(Device, "GeForce FX 5800 Ultra");  break;
        case 0x0302: strcpy(Device, "GeForce FX 5800");  break;
        case 0x0308: strcpy(Device, "Quadro FX 2000");  break;
        case 0x0309: strcpy(Device, "Quadro FX 1000");  break;
		case 0x030a: strcpy(Device, "ICE FX 2000");  break;
        case 0x0311: strcpy(Device, "GeForce FX 5600 Ultra");  break;
        case 0x0312: strcpy(Device, "GeForce FX 5600");  break;
        case 0x0313: strcpy(Device, "NV31");  break;
        case 0x0314: strcpy(Device, "GeForce FX 5600XT");  break;
        case 0x031a: strcpy(Device, "GeForce FX Go5600");  break;
        case 0x0321: strcpy(Device, "GeForce FX 5200 Ultra");  break;
        case 0x0322: strcpy(Device, "GeForce FX 5200");  break;
        case 0x0323: strcpy(Device, "GeForce FX 5200SE");  break;
        case 0x0324: strcpy(Device, "GeForce FX Go5200");  break;
        case 0x0325: strcpy(Device, "GeForce FX Go5250");  break;
        case 0x0328: strcpy(Device, "GeForce FX Go5200 32M/64M");  break;
        case 0x032a: strcpy(Device, "Quadro NVS 280 PCI");  break;
        case 0x032b: strcpy(Device, "Quadro FX 500");  break;
        case 0x032c: strcpy(Device, "GeForce FX Go53xx Series");  break;
        case 0x032d: strcpy(Device, "GeForce FX Go5100");  break;
        case 0x032f: strcpy(Device, "NV34GL");  break;
        case 0x0330: strcpy(Device, "GeForce FX 5900 Ultra");  break;
        case 0x0331: strcpy(Device, "GeForce FX 5900");  break;
        case 0x0332: strcpy(Device, "GeForce FX 5900XT");  break;
        case 0x0333: strcpy(Device, "GeForce FX 5950 Ultra");  break;
        case 0x0338: strcpy(Device, "Quadro FX 3000");  break;
        case 0x0341: strcpy(Device, "GeForce FX 5700 Ultra");  break;
        case 0x0342: strcpy(Device, "GeForce FX 5700");  break;
        case 0x0343: strcpy(Device, "GeForce FX 5700LE");  break;
        case 0x0344: strcpy(Device, "GeForce FX 5700VE");  break;
        case 0x0345: strcpy(Device, "NV36");  break;
        case 0x0347: strcpy(Device, "GeForce FX Go5700");  break;
        case 0x0348: strcpy(Device, "GeForce FX Go5700");  break;
        case 0x0349: strcpy(Device, "NV36M Pro");  break;
        case 0x034b: strcpy(Device, "NV36MAP");  break;
        case 0x034c: strcpy(Device, "Quadro FX Go1000");  break;
        case 0x034e: strcpy(Device, "Quadro FX 1100");  break;
        case 0x034f: strcpy(Device, "NV36GL");  break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x8086:
      strcpy(Vendor, "Intel");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x7121: strcpy(Device, "810"); bUnsupported = true; break;
        case 0x7123: strcpy(Device, "810"); bUnsupported = true; break;
        case 0x7125: strcpy(Device, "810e"); bUnsupported = true; break;
        case 0x7127: strcpy(Device, "810"); bUnsupported = true; break;
        case 0x1132: strcpy(Device, "815"); bUnsupported = true; break;
        case 0x7800: strcpy(Device, "740"); bUnsupported = true; break;
        case 0x1240: strcpy(Device, "752"); bUnsupported = true; break;

        case 0x3577: strcpy(Device, "830"); break;
        case 0x2562: strcpy(Device, "845"); break;
        case 0x2572: strcpy(Device, "865G"); break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x1033:
      strcpy(Vendor, "VideoLogic");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0046: strcpy(Device, "PowerVR"); bUnsupported = true; break;
        case 0x0067: strcpy(Device, "PowerVR2"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x1023:
      strcpy(Vendor, "Trident");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x8420: strcpy(Device, "CyberBlade i7"); bUnsupported = true; break;
        case 0x8820: strcpy(Device, "CyberBlade XP"); bUnsupported = true; break;
        case 0x9320: strcpy(Device, "Cyber9320"); bUnsupported = true; break;
        case 0x9388: strcpy(Device, "Cyber9388"); bUnsupported = true; break;
        case 0x9397: strcpy(Device, "Cyber9397"); bUnsupported = true; break;
        case 0x939A: strcpy(Device, "Cyber9397 DVD"); bUnsupported = true; break;
        case 0x9440: strcpy(Device, "Cyber"); bUnsupported = true; break;
        case 0x9520: strcpy(Device, "Cyber9520"); bUnsupported = true; break;
        case 0x9525: strcpy(Device, "Cyber9520 DVD"); bUnsupported = true; break;
        case 0x9540: strcpy(Device, "CyberBlade E4"); bUnsupported = true; break;
        case 0x9660: strcpy(Device, "Cyber9385"); bUnsupported = true; break;
        case 0x9750: strcpy(Device, "975"); bUnsupported = true; break;
        case 0x9754: strcpy(Device, "9753"); bUnsupported = true; break;
        case 0x9850: strcpy(Device, "3D Image"); bUnsupported = true; break;
        case 0x9880: strcpy(Device, "Blade 3D"); bUnsupported = true; break;
        case 0x9910: strcpy(Device, "CyberBlade XP"); bUnsupported = true; break;
        case 0x9930: strcpy(Device, "CyberBlade XPm"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x105d:
      strcpy(Vendor, "Number Nine");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x493d: strcpy(Device, "Revolution 3D"); bUnsupported = true; break;
        case 0x5348: strcpy(Device, "Revolution IV"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x126f:
      strcpy(Vendor, "Silicon Motion");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x720 : strcpy(Device, "Lynx 3DM"); bUnsupported = true; break;
        case 0x820 : strcpy(Device, "Lynx 3D"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x1013:
      strcpy(Vendor, "Cirrus");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0038: strcpy(Device, "GD7548"); bUnsupported = true; break;
        case 0x00a0: strcpy(Device, "GD5430"); bUnsupported = true; break;
        case 0x00a8: strcpy(Device, "GD5434"); bUnsupported = true; break;
        case 0x00ac: strcpy(Device, "GD5436"); bUnsupported = true; break;
        case 0x00b8: strcpy(Device, "GD5446"); bUnsupported = true; break;
        case 0x00bc: strcpy(Device, "GD5480"); bUnsupported = true; break;
        case 0x00d0: strcpy(Device, "CL5462"); bUnsupported = true; break;
        case 0x00d4: strcpy(Device, "GD5464"); bUnsupported = true; break;
        case 0x00d6: strcpy(Device, "GD5465"); bUnsupported = true; break;
        case 0x0301: strcpy(Device, "GD5446"); bUnsupported = true; break;
        case 0x1100: strcpy(Device, "CL6729"); bUnsupported = true; break;
        case 0x1202: strcpy(Device, "GD7543"); bUnsupported = true; break;
        case 0x6001: strcpy(Device, "CL4610"); bUnsupported = true; break;
        case 0x6003: strcpy(Device, "CL4614"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x1163:
      strcpy(Vendor, "Rendition");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0001: strcpy(Device, "Verite 1000"); bUnsupported = true; break;
        case 0x2000: strcpy(Device, "Verite 2100"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x110b:
      strcpy(Vendor, "Chromatic");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0004: strcpy(Device, "MPact"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x100e:
      strcpy(Vendor, "Weitek");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x9001: strcpy(Device, "P9000"); bUnsupported = true; break;
        case 0x9100: strcpy(Device, "P9100"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x0e11:
      strcpy(Vendor, "Compaq");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x3032: strcpy(Device, "QVision"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x1011:
      strcpy(Vendor, "Digital");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0004: strcpy(Device, "TGA"); bUnsupported = true; break;
        case 0x000d: strcpy(Device, "TGA2"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x10c8:
      strcpy(Vendor, "NeoMagic");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x0001: strcpy(Device, "128"); bUnsupported = true; break;
        case 0x0002: strcpy(Device, "128"); bUnsupported = true; break;
        case 0x0003: strcpy(Device, "128ZV"); bUnsupported = true; break;
        case 0x0004: strcpy(Device, "128XD"); bUnsupported = true; break;
        case 0x0005: strcpy(Device, "256AV"); bUnsupported = true; break;
        case 0x0006: strcpy(Device, "256ZX"); bUnsupported = true; break;
        case 0x0016: strcpy(Device, "256XL"); bUnsupported = true; break;
        case 0x0025: strcpy(Device, "256AV"); bUnsupported = true; break;
        case 0x0083: strcpy(Device, "128ZV"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x100c:
      strcpy(Vendor, "Tseng Labs");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x3202: strcpy(Device, "ET4000"); bUnsupported = true; break;
        case 0x3205: strcpy(Device, "ET4000"); bUnsupported = true; break;
        case 0x3206: strcpy(Device, "ET4000"); bUnsupported = true; break;
        case 0x3207: strcpy(Device, "ET4000"); bUnsupported = true; break;
        case 0x3208: strcpy(Device, "ET6000"); bUnsupported = true; break;
        case 0x4702: strcpy(Device, "ET6300"); bUnsupported = true; break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    case 0x18ca:
      strcpy(Vendor, "XGI");
      switch (DeviceIdentifier.dwDeviceId)
      {
        case 0x40: strcpy(Device, "Volary V8 DUO Ultra"); break;
        default:     strcpy(Device, "Unknown");
      }
  	  break;

    default:
      strcpy(Vendor, "Unknown");
      break;
  }

  pDD->Release();
  FreeLibrary(hDDInstance);

  if (bUnsupported)
    return -1;

  //Check DX version
  DWORD dwDirectXVersion = 0;
  TCHAR strDirectXVersion[10];
  TCHAR strResult[128];

  hr = GetDXVersion( &dwDirectXVersion, strDirectXVersion, 10 );
  if( SUCCEEDED(hr) )
  {
    if( dwDirectXVersion > 0 )
    {
      sprintf(strResult, TEXT("DirectX %s installed"), strDirectXVersion);
      if (dwDirectXVersion < 0x90000)
        nRenderer = R_GL_RENDERER;
    }
    else
    {
      strncpy(strResult, TEXT("DirectX not installed"), 128 );
      nRenderer = R_GL_RENDERER;
    }
    strResult[127] = 0;
  }
  else
  {
    sprintf( strResult, TEXT("Unknown version of DirectX installed"), hr );
    nRenderer = R_GL_RENDERER;
    strResult[127] = 0;
  }
  GetILog()->LogToFile("System: INFO: %s\n", strResult);

  return nRenderer;
}
#endif

/////////////////////////////////////////////////////////////////////////////////
void CSystem::CreateRendererVars()
{
	// load renderer settings from engine.ini
	//m_rWidth = GetIConsole()->CreateVariable("r_Width", "800", VF_DUMPTODISK);
	//m_rHeight = GetIConsole()->CreateVariable("r_Height", "600", VF_DUMPTODISK);
	m_rWidth = GetIConsole()->CreateVariable("r_Width", "1024", VF_DUMPTODISK,
		"Sets the display width, in pixels. Default is 1024.\n"
		"Usage: r_Width [800/1024]");
	m_rHeight = GetIConsole()->CreateVariable("r_Height", "768", VF_DUMPTODISK,
		"Sets the display height, in pixels. Default is 768.\n"
		"Usage: r_Height [600/768]");
	m_rColorBits = GetIConsole()->CreateVariable("r_ColorBits", "32", VF_DUMPTODISK,
		"Sets the color resolution, in bits per pixel. Default is 32.\n"
		"Usage: r_ColorBits [32/24/16/8]");
	m_rDepthBits = GetIConsole()->CreateVariable("r_DepthBits", "32", VF_DUMPTODISK);
	m_rStencilBits = GetIConsole()->CreateVariable("r_StencilBits", "8", VF_DUMPTODISK);	
	m_rDriver= GetIConsole()->CreateVariable("r_Driver", "Direct3D9", VF_DUMPTODISK,
		"Sets the renderer driver. Default is 'Direct3D9'.\n"
		"Usage: r_Driver Direct3D9");
#ifdef _DEBUG
	m_rFullscreen = GetIConsole()->CreateVariable("r_Fullscreen", "0", VF_DUMPTODISK,
		"Toggles fullscreen mode. Default is 1 (fullscreen).\n"
		"Usage: r_Fullscreen [0/1]\n"
		"In debug mode, the display is automatically set to windowed mode.");
	m_rDisplayInfo = GetIConsole()->CreateVariable("r_DisplayInfo", "1", VF_DUMPTODISK,
		"Toggles debugging information display. Default is 0 (off)."
		"Usage: r_DisplayInfo [0/1]\n"
		"In debug mode, the information is automatically displayed.");
#else
	m_rFullscreen = GetIConsole()->CreateVariable("r_Fullscreen", "1", VF_DUMPTODISK,
		"Toggles fullscreen mode. Default is 1 (fullscreen).\n"
		"Usage: r_Fullscreen [0/1]\n"
		"In debug mode, the display is automatically set to windowed mode.");
	m_rDisplayInfo = GetIConsole()->CreateVariable("r_DisplayInfo", "0", VF_DUMPTODISK,
		"Toggles debugging information display. Default is 0 (off)."
		"Usage: r_DisplayInfo [0/1]\n"
		"In debug mode, the information is automatically displayed.");
#endif
}

//////////////////////////////////////////////////////////////////////
static IRenderer *g_pRenderer;
static void g_DrawLine(float *v1,float *v2)
{
	Vec3 V1, V2;
  //V1.Set(v1[0],);
  //V2.Set(v2);
	V1.Set(v1[0],v1[1],v1[2]);
	V2.Set(v2[0],v2[1],v2[2]);
  g_pRenderer->DrawLine(V1,V2);
}

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderBegin()
{
	FUNCTION_PROFILER( this,PROFILE_SYSTEM );

	if (m_bIgnoreUpdates)
		return;

	//////////////////////////////////////////////////////////////////////
	//start the rendering pipeline
	if (m_pRenderer) 
		m_pRenderer->BeginFrame();
}

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderEnd()
{
	FUNCTION_PROFILER( this,PROFILE_SYSTEM );

	if (m_bIgnoreUpdates)
		return;

	if (!m_pRenderer)
		return;

	if (m_pConsole)
  {
    m_pConsole->Draw();
    m_Time.MeasureTime("3ConsDraw");
  }

	RenderStats();
	RenderStatistics ();
	//////////////////////////////////////////////////////////////////////
	//draw labels
	m_pRenderer->FlushTextMessages();

	// Flush render data and swap buffers.
	m_pRenderer->Update();
	m_Time.MeasureTime("3RendFlush");
}

//////////////////////////////////////////////////////////////////////////
void CSystem::UpdateLoadingScreen()
{
	if (!m_bEditor)
	{
		if (GetIRenderer()->EF_Query(EFQ_RecurseLevel) <= 0)
		{
			RenderBegin();
			GetIConsole()->Draw();
			RenderEnd();
		}
	}
	// This happens during loading, give windows opportunity to process window messages.
#ifdef WIN32
	if (m_hWnd && ::IsWindow((HWND)m_hWnd))
	{
		MSG msg;
    // Don't make any steps while 3D device is lost
    while (true)
    {
      while (PeekMessage(&msg, (HWND)m_hWnd, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      int *nLost = (int *)GetIRenderer()->EF_Query(EFQ_DeviceLost, 0);
      if (!*nLost)
        break;
    }
	}
#endif
}

//! Renders the statistics; this is called from RenderEnd, but if the 
//! Host application (Editor) doesn't employ the Render cycle in ISystem,
//! it may call this method to render the essential statistics
//////////////////////////////////////////////////////////////////////////
void CSystem::RenderStatistics ()
{
	// Render profile info.
	m_FrameProfileSystem.Render();
	RenderMemStats();

	int profValue = m_sys_profile->GetIVal();
	if (profValue != m_profile_old)
	{
		m_profile_old = profValue;
		if (profValue)
		{
			// Turn on frame profiler.
			if (!m_FrameProfileSystem.IsEnabled())
			{
				if (profValue > 0)
					m_FrameProfileSystem.Enable(true,true);
				else
					m_FrameProfileSystem.Enable(true,false); // Not display.
			}
			if (profValue < 0)
				profValue = -profValue;
			m_FrameProfileSystem.SetDisplayQuantity( (CFrameProfileSystem::EDisplayQuantity)(profValue-1) );
		}
		else if (m_FrameProfileSystem.IsEnabled())
		{
			// Turn off frame profiler.
			m_FrameProfileSystem.Enable(false,false);
		}
	}
	if (m_FrameProfileSystem.IsEnabled())
	{
		static string sSysProfileFilter;
		if (stricmp(m_sys_profile_filter->GetString(),sSysProfileFilter.c_str()) != 0)
		{
			sSysProfileFilter = m_sys_profile_filter->GetString();
			m_FrameProfileSystem.SetSubsystemFilter( sSysProfileFilter.c_str() );
		}
		m_FrameProfileSystem.SetHistogramScale( m_sys_profile_graphScale->GetFVal() );
		m_FrameProfileSystem.SetDrawGraph( m_sys_profile_graph->GetIVal() != 0 );
		m_FrameProfileSystem.SetNetworkProfiler( m_sys_profile_network->GetIVal() != 0 );
		m_FrameProfileSystem.SetPeakTolerance( m_sys_profile_peak->GetFVal() );
		m_FrameProfileSystem.SetPageFaultsGraph( m_sys_profile_pagefaultsgraph->GetIVal() != 0 );
	}
	static int memProfileValueOld = 0;
	int memProfileValue = m_sys_profile_memory->GetIVal();
	if (memProfileValue != memProfileValueOld)
	{
		memProfileValueOld = memProfileValue;
		m_FrameProfileSystem.EnableMemoryProfile( memProfileValue!=0 );
	}
}

//////////////////////////////////////////////////////////////////////
void CSystem::Render()
{
	if (m_bIgnoreUpdates)
		return;

	//check what is the current process 
	if (!m_pProcess)
		return; //should never happen

	//check if the game is in pause or
	//in menu mode
	//bool bPause=false;
	//if (m_pProcess->GetFlags() & PROC_MENU)
	//	bPause=true;

	FUNCTION_PROFILER( this,PROFILE_SYSTEM );

	//////////////////////////////////////////////////////////////////////
	//draw	
  if (m_pProcess && (m_pProcess->GetFlags() & PROC_3DENGINE))
  {	
		if (!IsEquivalent(m_ViewCamera.GetPos(),Vec3(0,0,0),VEC_EPSILON))		
		{
			if (m_pI3DEngine)
				m_pI3DEngine->SetCamera(m_ViewCamera);

			m_pProcess->Draw();		
						
			if (m_pAISystem)		
				m_pAISystem->DebugDraw(g_pRenderer);		
		}
  }
  else
  {
    if (m_pProcess)	
      m_pProcess->Draw();		
  }

	//////////////////////////////////////////////////////////////////////
	//draw physics helpers
	int id = m_pRenderer->GetFrameID();

	if (m_pIPhysicalWorld)
	{
		g_pRenderer = GetIRenderer();
		g_pRenderer->SetMaterialColor(0.5f,1.0f,0.9f,1.0f);
		m_pIPhysicalWorld->DrawPhysicsHelperInformation(g_DrawLine);

		if (m_pIPhysicalWorld->GetPhysVars()->bProfileEntities)
		{
			phys_profile_info *pInfos;
			int i,nEnts = m_pIPhysicalWorld->GetEntityProfileInfo(pInfos);
			float fColor[4] = { 0.3f,0.6f,1.0f,1.0f };
			for(i=0;i<nEnts;i++) 
				g_pRenderer->Draw2dLabel( 10.0f,60.0f+i*12.0f, 1.0f, fColor,false,
				 "%.2fms/%d %s (id %d)",CFrameProfilerTimer::TicksToSeconds(pInfos[i].nTicks)*1000.0f,pInfos[i].nCalls,pInfos[i].pName,pInfos[i].id );
		}
	}
	 
  // ProcessSleep()
  {
  #ifdef WIN32

    WIN_HWND hRendWnd = GetIRenderer()->GetHWND();
    if(!hRendWnd)
      return;

    static bool bSleep=0;
    WIN_HWND hActiveWnd = ::GetActiveWindow();
    char sBuff[128]="";
    GetWindowText( (HWND)hRendWnd,sBuff,128);
    if(hActiveWnd != hRendWnd && strncmp(sBuff,"- Far Cry -",11)==0)
    {
#if !defined(LINUX)
      Sleep(50);
#endif      
//      if(!bSleep)
//				GetILog()->LogToConsole("Sleep ON");
      bSleep = true;
    }
    else
    {
//      if(bSleep)
//        GetILog()->LogToConsole("Sleep OFF");
      bSleep = false;
    }

  #endif      
  }
}

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderMemStats()
{
	// check for the presence of the system
	if (!m_pConsole || !m_pRenderer || !m_cvMemStats->GetIVal())
	{
		SAFE_DELETE(m_pMemStats);
		return;
	}

	TickMemStats();

	assert (m_pMemStats);
	// render the statistics
	{
		CrySizerStatsRenderer StatsRenderer (this, m_pMemStats, m_cvMemStatsMaxDepth->GetIVal(), m_cvMemStatsThreshold->GetIVal());
		StatsRenderer.render((m_pRenderer->GetFrameID()+2)%m_cvMemStats->GetIVal() <= 1);
	}
}

//////////////////////////////////////////////////////////////////////////
void CSystem::RenderStats()
{
	if (!m_pConsole || !m_pI3DEngine)
		return;

	// Draw engine stats
	//ICVar * pDispInfoCVar = m_pConsole->GetCVar("r_DisplayInfo");
	if (m_rDisplayInfo->GetIVal())
  {
    // Draw 3dengine stats and get last text cursor position
    float nTextPosX=101-20, nTextPosY=-2, nTextStepY=3;
    m_pI3DEngine->DisplayInfo(nTextPosX, nTextPosY, nTextStepY);

    // Draw non 3dengine stats
	  if (m_rDisplayInfo->GetIVal()==2)
	  {
		  size_t nSoundCurrMem,nSoundMaxMem;
		  m_pRenderer->TextToScreen( nTextPosX-15, nTextPosY+=nTextStepY, "SysMem %.1f mb",  
        float(DumpMMStats(NULL))/1024.f);

			if (m_pISound)
			{
				m_pISound->GetSoundMemoryUsageInfo(nSoundCurrMem,nSoundMaxMem);	
				m_pRenderer->TextToScreen( nTextPosX-18, nTextPosY+=nTextStepY, "Sound %2.1f/%2.1f mb",
					(float)(nSoundCurrMem)/(1024.0f*1024.0f), (float)(nSoundMaxMem)/(1024.0f*1024.0f));
			}
	  }
/*
		float fLogic=0, fRender=0, fSum=0;
		m_Time.GetBasicStats(fLogic, fRender, fSum);
		m_pRenderer->TextToScreen( nTextPosX-23, nTextPosY+=nTextStepY, "Think=%.1f Draw=%.1f ms",  fLogic, fRender);
*/
    m_Time.MeasureTime("3SysStats");
  }
}
