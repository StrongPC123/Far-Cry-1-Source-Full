
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Input.cpp
//  Description: General input system
//
//	History:
//	-Jan 31,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma warning(disable:4786)
#include <stdio.h>
#include <ILog.h>
#include <IInput.h>
#include <ISystem.h>
#include "Input.h"
//#include "ActionMap.h"
#include "XActionMapManager.h"
//#include "..\Common\Log.h"

#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

#if defined (WIN32) || (WIN64)
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#endif

#ifdef PS2

#include<sifrpc.h>
#include <sifdev.h>


#endif

///////////////////////////////////////////
int g_nKeys[]={
		XKEY_BACKSPACE     ,
		XKEY_TAB           ,
		XKEY_RETURN        ,
		XKEY_CONTROL       ,
		XKEY_ALT           ,
		XKEY_SHIFT         ,
		XKEY_PAUSE         ,
		XKEY_CAPSLOCK      ,
		XKEY_ESCAPE        ,
		XKEY_SPACE         ,
		XKEY_PAGE_DOWN     ,
		XKEY_PAGE_UP       ,
		XKEY_END           ,
		XKEY_HOME          ,
		XKEY_LEFT          ,
		XKEY_UP            ,
		XKEY_RIGHT         ,
		XKEY_DOWN          ,
		XKEY_PRINT         ,
		XKEY_INSERT        ,
		XKEY_DELETE        ,
		XKEY_HELP          ,
		XKEY_0             ,
		XKEY_1             ,
		XKEY_2             ,
		XKEY_3             ,
		XKEY_4             ,
		XKEY_5             ,
		XKEY_6             ,
		XKEY_7             ,
		XKEY_8             ,
		XKEY_9             ,
		XKEY_A             ,
		XKEY_B             ,
		XKEY_C             ,
		XKEY_D             ,
		XKEY_E             ,
		XKEY_F             ,
		XKEY_G             ,
		XKEY_H             ,
		XKEY_I             ,
		XKEY_J             ,
		XKEY_K             ,
		XKEY_L             ,
		XKEY_M             ,
		XKEY_N             ,
		XKEY_O             ,
		XKEY_P             ,
		XKEY_Q             ,
		XKEY_R             ,
		XKEY_S             ,
		XKEY_T             ,
		XKEY_U             ,
		XKEY_V             ,
		XKEY_W             ,
		XKEY_X             ,
		XKEY_Y             ,
		XKEY_Z             ,
		XKEY_TILDE         ,
		XKEY_MINUS         ,
		XKEY_EQUALS        ,
		XKEY_LBRACKET      ,
		XKEY_RBRACKET      ,
		XKEY_BACKSLASH     ,
		XKEY_SEMICOLON     ,
		XKEY_APOSTROPHE    ,
		XKEY_COMMA         ,
		XKEY_PERIOD        ,
		XKEY_SLASH         ,
		XKEY_NUMPAD0       ,
		XKEY_NUMPAD1       ,
		XKEY_NUMPAD2       ,
		XKEY_NUMPAD3       ,
		XKEY_NUMPAD4       ,
		XKEY_NUMPAD5       ,
		XKEY_NUMPAD6       ,
		XKEY_NUMPAD7       ,
		XKEY_NUMPAD8       ,
		XKEY_NUMPAD9       ,
		XKEY_MULTIPLY      ,
		XKEY_ADD           ,
		XKEY_SEPARATOR     ,
		XKEY_SUBTRACT      ,
		XKEY_DECIMAL       ,
		XKEY_DIVIDE        ,
		XKEY_NUMPADENTER   ,
		XKEY_F1            ,
		XKEY_F2            ,
		XKEY_F3            ,
		XKEY_F4            ,
		XKEY_F5            ,
		XKEY_F6            ,
		XKEY_F7            ,
		XKEY_F8            ,
		XKEY_F9            ,
		XKEY_F10           ,
		XKEY_F11           ,
		XKEY_F12           ,
		XKEY_F13           ,
		XKEY_F14           ,
		XKEY_F15           ,
		XKEY_F16           ,
		XKEY_F17           ,
		XKEY_F18           ,
		XKEY_F19           ,
		XKEY_F20           ,
		XKEY_F21           ,
		XKEY_F22           ,
		XKEY_F23           ,
		XKEY_F24           ,
		XKEY_NUMLOCK       ,
		XKEY_SCROLLLOCK    ,
		XKEY_LCONTROL      ,
		XKEY_RCONTROL      ,
		XKEY_LALT          ,
		XKEY_RALT          ,
		XKEY_LSHIFT        ,
		XKEY_RSHIFT        ,
		XKEY_WIN_LWINDOW   ,
		XKEY_WIN_RWINDOW   ,
		XKEY_WIN_APPS      ,
		XKEY_OEM_102       ,
		//,
		XKEY_MOUSE1					,
		XKEY_MOUSE2					,
		XKEY_MOUSE3					,
		XKEY_MOUSE4					,
		XKEY_MOUSE5					,
		XKEY_MOUSE6					,
		XKEY_MOUSE7					,
		XKEY_MOUSE8					,
		XKEY_MWHEEL_UP			,
		XKEY_MWHEEL_DOWN		,
		XKEY_MAXIS_X			,
		XKEY_MAXIS_Y			,

    // Gamepad
    XKEY_GP_A,
    XKEY_GP_B,
    XKEY_GP_X,
    XKEY_GP_Y,
    XKEY_GP_WHITE,
    XKEY_GP_BLACK,
    XKEY_GP_LEFT_TRIGGER,
    XKEY_GP_RIGHT_TRIGGER,

    XKEY_GP_DPAD_UP     ,
    XKEY_GP_DPAD_DOWN   ,
    XKEY_GP_DPAD_LEFT   ,
    XKEY_GP_DPAD_RIGHT  ,
    XKEY_GP_START       ,
    XKEY_GP_BACK        ,
    XKEY_GP_LEFT_THUMB  ,
    XKEY_GP_RIGHT_THUMB ,

    XKEY_GP_STHUMBLUP,
		XKEY_GP_STHUMBLDOWN,
    XKEY_GP_STHUMBLLEFT,
		XKEY_GP_STHUMBLRIGHT,

    XKEY_GP_STHUMBLX,
		XKEY_GP_STHUMBLY,
    XKEY_GP_STHUMBRX,
		XKEY_GP_STHUMBRY,

    XKEY_NULL
};

bool CInput::Init(ISystem *pSystem,HINSTANCE hinst,HWND hwnd,bool dinput)
{
	m_pSystem = pSystem;

#ifndef PS2
	m_pLog=pSystem->GetILog();
  
#ifndef _XBOX
	m_hinst=hinst;
	m_hwnd=hwnd;
	m_postingenable = 1;
		
	//if (dinput)
	{
		m_pLog->Log("Initializing Direct Input\n");

		HRESULT hr = DirectInput8Create(m_hinst,
			DIRECTINPUT_VERSION, 
			IID_IDirectInput8, 
			(void**)&m_g_pdi, NULL); 
		
		
		//HRESULT hr = DirectInputCreate(m_hinst, DIRECTINPUT_VERSION, &m_g_pdi, NULL);	
		if (FAILED(hr) && dinput) 
		{
			m_pLog->Log("Cannot initialize Direct Input\n");		
			return (false);
		}			
	}
	//else
	//	return (true);
	
	m_pLog->Log("Direct Input initialized (CryInputDLL)\n");	
#endif //_XBOX

#ifndef _XBOX
	//if (!m_Keyboard.Init(this,m_pLog,m_g_pdi,hinst,hwnd) && dinput) 
	if (!m_Keyboard.Init(this,m_pSystem,m_g_pdi,hinst,hwnd) && dinput) 
		return (false);
	m_pLog->LogToFile("Keyboard initialized\n");			
	
	m_Mouse.m_pInput = this;
	if (!m_Mouse.Init(m_pSystem,m_g_pdi,hinst,hwnd,dinput) && dinput) 
		return (false);
	m_pLog->Log("Mouse initialized\n");		
	
	if (!m_Joystick.Init(m_pLog)) 
	{
		m_pLog->Log("Cannot initialize joystick\n");
	}		
	else 
		m_pLog->Log("Joystick initialized\n");		
#else //_XBOX

  if (!m_Gamepad.Init(m_pLog)) 
	{
		m_pLog->Log("Cannot initialize Gamepad\n");
	}		
	else 
		m_pLog->Log("Gamepad initialized\n");		
#ifdef DEBUG_KEYBOARD
  if (!m_Keyboard.Init(this, m_pLog))
  {
    m_pLog->Log("Cannot initialize Keyboard\n");
  }
  else
	  m_pLog->LogToFile("Keyboard initialized\n");
#endif //DEBUG_KEYBOARD

#endif //_XBOX

	m_pLog->Log("initializing Key/name Mapping\n");

	int n=0;
	while(g_nKeys[n]!=0)
	{
		m_mapKeyNames.insert(KeyNamesMapItor::value_type(GetKeyName(g_nKeys[n]),g_nKeys[n]));
		//m_pLog->Log("KEY==> %s",GetKeyName(g_nKeys[n]));
		n++;
	}

  m_bPreviousFocus = false;
	m_exclusivelistener = 0;
#endif
	return (true);	
}

////////////////////Morgantini...init PS2 version ///////////////////////
#ifdef PS2

bool CInput::Init(ILog *pLog,ITimer *pTimer)
{

	m_pLog=pLog;
	m_pLog->Log("Initializing PS2 Input\n");

	
	
	if (!m_Mouse.Init(pLog,pTimer))
	{
		m_pLog->Log("Cannot initialize Mouse\n");
		//TO BE CHANGED return (false);
	}
	else
		m_pLog->Log("PS2 Mouse initialized\n");		
				
	//Set the pad on mouse
	m_Mouse.SetPad(&m_Joystick);
	
	
	
	
	if (!m_Keyboard.Init(pLog)) 
	{
		m_pLog->Log("Cannot initialize Keyboard\n");
		//TO BE CHANGED return (false);
	}
	else
		m_pLog->LogToFile("PS2 Keyboard initialized\n");		
	

	if (!m_Joystick.Init(pLog)) 
	{
		m_pLog->Log("Cannot initialize joystick\n");
		//TO BE CHANGED return (false);
	}		
	else 
	{
		
		if(!m_Keyboard.IsInit())
		{
			m_Keyboard.SetPad(&m_Joystick);
			
		}

//TiZ		m_pLog->Log("PS2 Joystick initialized\n");
	}		

	return (true);	

}


#endif


///////////////////////////////////////////
void CInput::ShutDown()
{
	m_pLog->Log("Input Shutdown\n");

#ifndef _XBOX
	m_Keyboard.ShutDown();
	m_Mouse.Shutdown();
	m_Joystick.ShutDown();
#else //_XBOX
  m_Gamepad.ShutDown();
#ifdef DEBUG_KEYBOARD
  m_Keyboard.ShutDown();
#endif //DEBUG_KEYBOARD 
#endif //_XBOX

#ifndef PS2
#ifndef _XBOX
	if (m_g_pdi)
	{
		m_g_pdi->Release();
		m_g_pdi = NULL;
	}	
#endif //_XBOX
#endif
	//
	delete this;
}

///////////////////////////////////////////
void CInput::Update(bool bFocus)
{
	if(bFocus)
  {
		m_console = m_pSystem->GetIConsole();
#ifndef _XBOX
    m_Keyboard.Update();
	  m_Mouse.Update(m_bPreviousFocus); // m_bPreviousFocus used to skip first mouse read after getting focus
	  m_Joystick.Update();
#else //_XBOX
   m_Gamepad.Update();
#ifdef DEBUG_KEYBOARD
   m_Keyboard.Update();
#endif //DEBUG_KEYBOARD 
#endif //_XBOX
  }

  m_bPreviousFocus = bFocus;
}

void CInput::ClearKeyState()
{
	m_Keyboard.ClearKeyState();
	m_Mouse.ClearKeyState();
}

///////////////////////////////////////////////
IActionMapManager* CInput::CreateActionMapManager()
{
	CXActionMapManager *pAMM = new CXActionMapManager(this);
	if(!pAMM)
		return NULL;

	return pAMM;
}
///////////////////////////////////////////////
const char* CInput::GetKeyName(int iKey,int modifiers, bool bGUI)
{
	static char szKeyName[8];	szKeyName[0]=0;

	if (bGUI)
	{
		bool bNumLock = ((GetKeyState(XKEY_NUMLOCK) & 0x01) != 0);

		switch(iKey)
		{
		case XKEY_SPACE:         return " ";
		case XKEY_MULTIPLY:      return "*";
		case XKEY_ADD:           return "+";
		case XKEY_SUBTRACT:      return "-";
		case XKEY_DECIMAL:       return ".";
		case XKEY_DIVIDE:        return "/";
		case XKEY_NUMPAD0:       return (bNumLock ? "0" : "");
		case XKEY_NUMPAD1:       return (bNumLock ? "1" : "");
		case XKEY_NUMPAD2:       return (bNumLock ? "2" : "");
		case XKEY_NUMPAD3:       return (bNumLock ? "3" : "");
		case XKEY_NUMPAD4:       return (bNumLock ? "4" : "");
		case XKEY_NUMPAD5:       return (bNumLock ? "5" : "");
		case XKEY_NUMPAD6:       return (bNumLock ? "6" : "");
		case XKEY_NUMPAD7:       return (bNumLock ? "7" : "");
		case XKEY_NUMPAD8:       return (bNumLock ? "8" : "");
		case XKEY_NUMPAD9:       return (bNumLock ? "9" : "");
		case XKEY_BACKSPACE:
		case XKEY_TAB:
		case XKEY_RETURN:
		case XKEY_PAUSE:
		case XKEY_CAPSLOCK:
		case XKEY_ESCAPE:
		case XKEY_PAGE_DOWN:
		case XKEY_PAGE_UP:
		case XKEY_END:
		case XKEY_HOME:
		case XKEY_LEFT:
		case XKEY_UP:
		case XKEY_RIGHT:
		case XKEY_DOWN:
		case XKEY_PRINT:
		case XKEY_INSERT:
		case XKEY_DELETE:
		case XKEY_HELP:
		case XKEY_SEPARATOR:
		case XKEY_NUMPADENTER:
		case XKEY_F1:
		case XKEY_F2:
		case XKEY_F3:
		case XKEY_F4:
		case XKEY_F5:
		case XKEY_F6:
		case XKEY_F7:
		case XKEY_F8:
		case XKEY_F9:
		case XKEY_F10:
		case XKEY_F11:
		case XKEY_F12:
		case XKEY_F13:
		case XKEY_F14:
		case XKEY_F15:
		case XKEY_F16:
		case XKEY_F17:
		case XKEY_F18:
		case XKEY_F19:
		case XKEY_F20:
		case XKEY_F21:
		case XKEY_F22:
		case XKEY_F23:
		case XKEY_F24:
		case XKEY_NUMLOCK:
		case XKEY_SCROLLLOCK:
		case XKEY_LCONTROL:
		case XKEY_RCONTROL:
		case XKEY_LALT:
		case XKEY_RALT:
		case XKEY_LSHIFT:
		case XKEY_RSHIFT:
		case XKEY_WIN_LWINDOW:
		case XKEY_WIN_RWINDOW:
		case XKEY_WIN_APPS:
		case XKEY_MOUSE1:
		case XKEY_MOUSE2:
		case XKEY_MOUSE3:
		case XKEY_MOUSE4:
		case XKEY_MOUSE5:
		case XKEY_MOUSE6:
		case XKEY_MOUSE7:
		case XKEY_MOUSE8:
		case XKEY_MWHEEL_UP:
		case XKEY_MWHEEL_DOWN:
		case XKEY_MAXIS_X:
		case XKEY_MAXIS_Y:
		case XKEY_GP_A:
		case XKEY_GP_B:
		case XKEY_GP_X:
		case XKEY_GP_Y:
		case XKEY_GP_WHITE:
		case XKEY_GP_BLACK:
		case XKEY_GP_LEFT_TRIGGER:
		case XKEY_GP_RIGHT_TRIGGER:
		case XKEY_GP_DPAD_UP:
		case XKEY_GP_DPAD_DOWN:
		case XKEY_GP_DPAD_LEFT:
		case XKEY_GP_DPAD_RIGHT:
		case XKEY_GP_START:
		case XKEY_GP_BACK:
		case XKEY_GP_LEFT_THUMB:
		case XKEY_GP_RIGHT_THUMB:
		case XKEY_GP_STHUMBLUP:
		case XKEY_GP_STHUMBLDOWN:
		case XKEY_GP_STHUMBLLEFT:
		case XKEY_GP_STHUMBLRIGHT:
		case XKEY_GP_STHUMBLX:
		case XKEY_GP_STHUMBLY:
		case XKEY_GP_STHUMBRX:
		case XKEY_GP_STHUMBRY:
			return "";
		}

		sprintf(szKeyName,"%c", m_Keyboard.XKEY2ASCII(iKey, modifiers));

		return szKeyName;
	}
	else
	{
		switch (iKey)
		{
		case XKEY_NULL:					 return "";
		case XKEY_BACKSPACE:     return "backspace";
		case XKEY_TAB:           return "tab";
		case XKEY_RETURN:        return "return";
		case XKEY_PAUSE:         return "pause";
		case XKEY_CAPSLOCK:      return "capslock";
		case XKEY_ESCAPE:        return "esc";
		case XKEY_SPACE:         return "spacebar";
		case XKEY_PAGE_DOWN:     return "pagedown";
		case XKEY_PAGE_UP:       return "pageup";
		case XKEY_END:           return "end";
		case XKEY_HOME:          return "home";
		case XKEY_LEFT:          return "left";
		case XKEY_UP:            return "up";
		case XKEY_RIGHT:         return "right";
		case XKEY_DOWN:          return "down";
		case XKEY_PRINT:         return "printscreen";
		case XKEY_INSERT:        return "insert";
		case XKEY_DELETE:        return "delete";
		case XKEY_HELP:          return "help";
		case XKEY_NUMPAD0:       return "numpad0";
		case XKEY_NUMPAD1:       return "numpad1";
		case XKEY_NUMPAD2:       return "numpad2";
		case XKEY_NUMPAD3:       return "numpad3";
		case XKEY_NUMPAD4:       return "numpad4";
		case XKEY_NUMPAD5:       return "numpad5";
		case XKEY_NUMPAD6:       return "numpad6";
		case XKEY_NUMPAD7:       return "numpad7";
		case XKEY_NUMPAD8:       return "numpad8";
		case XKEY_NUMPAD9:       return "numpad9";
		case XKEY_MULTIPLY:      return "multiply";
		case XKEY_ADD:           return "add";
		case XKEY_SEPARATOR:     return "separator";
		case XKEY_SUBTRACT:      return "subtract";
		case XKEY_DECIMAL:       return "decimal";
		case XKEY_DIVIDE:        return "divide";
		case XKEY_NUMPADENTER:   return "numpad enter";
		case XKEY_F1:            return "f1";
		case XKEY_F2:            return "f2";
		case XKEY_F3:            return "f3";
		case XKEY_F4:            return "f4";
		case XKEY_F5:            return "f5";
		case XKEY_F6:            return "f6";
		case XKEY_F7:            return "f7";
		case XKEY_F8:            return "f8";
		case XKEY_F9:            return "f9";
		case XKEY_F10:           return "f10";
		case XKEY_F11:           return "f11";
		case XKEY_F12:           return "f12";
		case XKEY_F13:           return "f13";
		case XKEY_F14:           return "f14";
		case XKEY_F15:           return "f15";
		case XKEY_F16:           return "f16";
		case XKEY_F17:           return "f17";
		case XKEY_F18:           return "f18";
		case XKEY_F19:           return "f19";
		case XKEY_F20:           return "f20";
		case XKEY_F21:           return "f21";
		case XKEY_F22:           return "f22";
		case XKEY_F23:           return "f23";
		case XKEY_F24:           return "f24";
		case XKEY_NUMLOCK:       return "numlock";
		case XKEY_SCROLLLOCK:    return "scrolllock";
		case XKEY_LCONTROL:      return "lctrl";
		case XKEY_RCONTROL:      return "rctrl";
		case XKEY_LALT:          return "lalt";
		case XKEY_RALT:          return "ralt";
		case XKEY_LSHIFT:        return "lshift";
		case XKEY_RSHIFT:        return "rshift";
		case XKEY_WIN_LWINDOW:   return "lwin";
		case XKEY_WIN_RWINDOW:   return "rwin";
		case XKEY_WIN_APPS:      return "apps";
		case XKEY_MOUSE1:					return "mouse1";
		case XKEY_MOUSE2:					return "mouse2";
		case XKEY_MOUSE3:					return "mouse3";
		case XKEY_MOUSE4:					return "mouse4";
		case XKEY_MOUSE5:					return "mouse5";
		case XKEY_MOUSE6:					return "mouse6";
		case XKEY_MOUSE7:					return "mouse7";
		case XKEY_MOUSE8:					return "mouse8";
		case XKEY_MWHEEL_UP:			return "mwheelup";
		case XKEY_MWHEEL_DOWN:		return "mwheeldown";
		case XKEY_MAXIS_X:				return "maxisx";
		case XKEY_MAXIS_Y:				return "maxisy";

		case XKEY_GP_A:          return "gp_a";
		case XKEY_GP_B:          return "gp_b";
		case XKEY_GP_X:          return "gp_x";
		case XKEY_GP_Y:          return "gp_y";
		case XKEY_GP_WHITE:      return "gp_white";
		case XKEY_GP_BLACK:      return "gp_black";
		case XKEY_GP_LEFT_TRIGGER:    return "gp_left_trigger";
		case XKEY_GP_RIGHT_TRIGGER:   return "gp_right_trigger";

		case XKEY_GP_DPAD_UP:       return "gp_dpad_up";
		case XKEY_GP_DPAD_DOWN:     return "gp_dpad_down";
		case XKEY_GP_DPAD_LEFT:     return "gp_dpad_left";
		case XKEY_GP_DPAD_RIGHT:    return "gp_dpad_right";
		case XKEY_GP_START:         return "gp_start";
		case XKEY_GP_BACK:          return "gp_back";
		case XKEY_GP_LEFT_THUMB:    return "gp_left_thumb";
		case XKEY_GP_RIGHT_THUMB:   return "gp_right_thumb";

		case XKEY_GP_STHUMBLUP:     return "gp_sthumblup";
		case XKEY_GP_STHUMBLDOWN:   return "gp_sthumbldown";
		case XKEY_GP_STHUMBLLEFT:   return "gp_sthumblleft";
		case XKEY_GP_STHUMBLRIGHT:  return "gp_sthumblright";

		case XKEY_GP_STHUMBLX:    return "gp_sthumblx";
		case XKEY_GP_STHUMBLY:    return "gp_sthumbly";
		case XKEY_GP_STHUMBRX:    return "gp_sthumbrx";
		case XKEY_GP_STHUMBRY:    return "gp_sthumbry";
		}

		sprintf(szKeyName,"%c", m_Keyboard.XKEY2ASCII(iKey, modifiers));

		return szKeyName;
	}

	return "";
}

bool CInput::GetOSKeyName(int nKey, wchar_t *szwKeyName, int iBufSize)
{
	if (IS_KEYBOARD_KEY(nKey))
	{
		return m_Keyboard.GetOSKeyName(nKey, szwKeyName, iBufSize);
	}
	else
	{
		if (IS_MOUSE_KEY(nKey))
		{
			return m_Mouse.GetOSKeyName(nKey, szwKeyName, iBufSize);
		}
	}
	return false;
}

int CInput::VK2XKEY(int nKey)
{
//#ifdef DEBUG_KEYBOARD
	switch(nKey)
   {
      case VK_ESCAPE:        return XKEY_ESCAPE;
      case '1':             return XKEY_1;
      case '2':             return XKEY_2;
      case '3':             return XKEY_3;
      case '4':             return XKEY_4;
      case '5':             return XKEY_5;
      case '6':             return XKEY_6;
      case '7':             return XKEY_7;
      case '8':             return XKEY_8;
      case '9':             return XKEY_9;
      case '10':             return XKEY_0;
      case VK_SUBTRACT:         return XKEY_MINUS;
      case VK_OEM_PLUS:        return XKEY_EQUALS;
      case VK_BACK:          return XKEY_BACKSPACE;
      case VK_TAB:           return XKEY_TAB;
      case 'Q':             return XKEY_Q;
      case 'W':             return XKEY_W;
      case 'E':             return XKEY_E;
      case 'R':             return XKEY_R;
      case 'T':             return XKEY_T;
      case 'Y':             return XKEY_Y;
      case 'U':             return XKEY_U;
      case 'I':             return XKEY_I;
      case 'O':             return XKEY_O;
      case 'P':             return XKEY_P;
      case VK_OEM_4:      return XKEY_LBRACKET;
      case VK_OEM_6:      return XKEY_RBRACKET;
      case VK_RETURN:        return XKEY_RETURN;
      case VK_LCONTROL:      return XKEY_LCONTROL;
      case 'A':             return XKEY_A;
      case 'S':             return XKEY_S;
      case 'D':             return XKEY_D;
      case 'F':             return XKEY_F;
      case 'G':             return XKEY_G;
      case 'H':             return XKEY_H;
      case 'J':             return XKEY_J;
      case 'K':             return XKEY_K;
      case 'L':             return XKEY_L;
      case VK_OEM_1:     return XKEY_SEMICOLON;
      case VK_OEM_7:    return XKEY_APOSTROPHE;
      case VK_OEM_3:         return XKEY_TILDE; 
      case VK_LSHIFT:        return XKEY_LSHIFT;
      case VK_OEM_5:     return XKEY_BACKSLASH;
      case 'Z':             return XKEY_Z;
      case 'X':             return XKEY_X;
      case 'C':             return XKEY_C;
      case 'V':             return XKEY_V;
      case 'B':             return XKEY_B;
      case 'N':             return XKEY_N;
      case 'M':             return XKEY_M;
      case VK_OEM_COMMA:         return XKEY_COMMA;
      case VK_OEM_PERIOD:        return XKEY_PERIOD;
      case VK_OEM_2:         return XKEY_SLASH;
      case VK_RSHIFT:        return XKEY_RSHIFT;
      case VK_MULTIPLY:      return XKEY_MULTIPLY;
      case VK_LMENU:          return XKEY_LALT;
      case VK_SPACE:         return XKEY_SPACE;
      case VK_CAPITAL:      return XKEY_CAPSLOCK;
      case VK_F1:            return XKEY_F1;
      case VK_F2:            return XKEY_F2;
      case VK_F3:            return XKEY_F3;
      case VK_F4:            return XKEY_F4;
      case VK_F5:            return XKEY_F5;
      case VK_F6:            return XKEY_F6;
      case VK_F7:            return XKEY_F7;
      case VK_F8:            return XKEY_F8;
      case VK_F9:            return XKEY_F9;
      case VK_F10:           return XKEY_F10;
      case VK_NUMLOCK:       return XKEY_NUMLOCK;
      case VK_SCROLL:        return XKEY_SCROLLLOCK;
      case VK_NUMPAD7:       return XKEY_NUMPAD7;
      case VK_NUMPAD8:       return XKEY_NUMPAD8;
      case VK_NUMPAD9:       return XKEY_NUMPAD9;
      case VK_NUMPAD4:       return XKEY_NUMPAD4;
      case VK_NUMPAD5:       return XKEY_NUMPAD5;
      case VK_NUMPAD6:       return XKEY_NUMPAD6;
      case VK_ADD:           return XKEY_ADD;
      case VK_NUMPAD1:       return XKEY_NUMPAD1;
      case VK_NUMPAD2:       return XKEY_NUMPAD2;
      case VK_NUMPAD3:       return XKEY_NUMPAD3;
      case VK_NUMPAD0:       return XKEY_NUMPAD0;
      case VK_DECIMAL:       return XKEY_DECIMAL;
      case VK_F11:           return XKEY_F11;
      case VK_F12:           return XKEY_F12;
      case VK_F13:           return XKEY_F13;
      case VK_F14:           return XKEY_F14;
      case VK_F15:           return XKEY_F15;
      case VK_KANA:          return 0;
      case VK_CONVERT:       return 0;
      //case VK_NOCONVERT:     return 0;
//      case VK_YEN:           return 0;
//      case VK_NUMPADEQUALS:  return 0;
//      case VK_CIRCUMFLEX:    return 0;
//      case VK_AT:            return 0;
//      case VK_COLON:         return 0;
//      case VK_UNDERLINE:     return 0;
 //     case VK_KANJI:         return 0;
//      case VK_STOP:          return 0;
//      case VK_AX:            return 0;
//      case VK_UNLABELED:     return 0;
      //case VK_NUMPADENTER:   return XKEY_NUMPADENTER;
      case VK_RCONTROL:      return XKEY_RCONTROL;
//      case VK_NUMPADCOMMA:   return XKEY_SEPARATOR;
      case VK_DIVIDE:        return XKEY_DIVIDE;
//      case VK_SYSRQ:         return XKEY_PRINT;
      case VK_RMENU:          return XKEY_RALT;
      case VK_PAUSE:         return XKEY_PAUSE;
      case VK_HOME:          return XKEY_HOME;
      case VK_UP:            return XKEY_UP;
      case VK_PRIOR:          return XKEY_PAGE_UP;
      case VK_LEFT:          return XKEY_LEFT;
      case VK_RIGHT:         return XKEY_RIGHT;
      case VK_END:           return XKEY_END;
      case VK_DOWN:          return XKEY_DOWN;
      case VK_NEXT:          return XKEY_PAGE_DOWN;
      case VK_INSERT:        return XKEY_INSERT;
      case VK_DELETE:        return XKEY_DELETE;
      case VK_LWIN:          return XKEY_WIN_LWINDOW;
      case VK_RWIN:          return XKEY_WIN_RWINDOW;
      case VK_APPS:          return XKEY_WIN_APPS;
      case VK_OEM_102:       return XKEY_OEM_102;
			case VK_OEM_MINUS:		 return XKEY_MINUS;	
   };
//#endif // DEBUG_KEYBOARD


   return XKEY_NULL;
}

//////////////////////////////////////////////////////////////////////////
int CInput::GetKeyID(const char *sName)
{
	if (!sName)
	{
		return XKEY_NULL;
	}

	KeyNamesMapItor itor;

	char sTemp[256];
	strcpy(sTemp,sName);	

	_strlwr(sTemp);

	itor=m_mapKeyNames.find(sTemp);

	if(itor!=m_mapKeyNames.end())
		return itor->second;

	return XKEY_NULL;
}

//think if can be better
//////////////////////////////////////////////////////////////////////////
const char *CInput::GetXKeyPressedName()
{
	if(MousePressed(XKEY_MOUSE1))
	{
		return GetKeyName(XKEY_MOUSE1);
	}
	if(MousePressed(XKEY_MOUSE2))
	{
		return GetKeyName(XKEY_MOUSE2);
	}
	if(MousePressed(XKEY_MOUSE3))
	{
		return GetKeyName(XKEY_MOUSE3);
	}
	if(MousePressed(XKEY_MOUSE4))
	{
		return GetKeyName(XKEY_MOUSE4);
	}
	if(MousePressed(XKEY_MOUSE5))
	{
		return GetKeyName(XKEY_MOUSE5);
	}
	if(MousePressed(XKEY_MOUSE6))
	{
		return GetKeyName(XKEY_MOUSE6);
	}
	if(MousePressed(XKEY_MOUSE7))
	{
		return GetKeyName(XKEY_MOUSE7);
	}
	if(MousePressed(XKEY_MOUSE8))
	{
		return GetKeyName(XKEY_MOUSE8);
	}
	if(MousePressed(XKEY_MWHEEL_DOWN))
	{
		return GetKeyName(XKEY_MWHEEL_DOWN);
	}
	if(MousePressed(XKEY_MWHEEL_UP))
	{
		return GetKeyName(XKEY_MWHEEL_UP);
	}
	return GetKeyPressedName();
}

//////////////////////////////////////////////////////////////////////////
void CInput::AddEventListener( IInputEventListener *pListener )
{
	// Add new listener to list if not added yet.
	if (std::find(m_listeners.begin(),m_listeners.end(),pListener) == m_listeners.end())
	{
		m_listeners.push_back( pListener );
	}
}

//////////////////////////////////////////////////////////////////////////
void CInput::RemoveEventListener( IInputEventListener *pListener )
{
	// Remove listener if it is in list.
	Listeners::iterator it = std::find(m_listeners.begin(),m_listeners.end(),pListener);
	if (it != m_listeners.end())
	{
		m_listeners.erase( it );
		//m_listeners.erase( std::remove(m_listeners.begin(),m_listeners.end(),pListener),m_listeners.end() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CInput::AddConsoleEventListener( IInputEventListener *pListener )
{
	if (std::find(m_consolelisteners.begin(),m_consolelisteners.end(),pListener) == m_consolelisteners.end())
	{
		m_consolelisteners.push_back( pListener );
	}
}

//////////////////////////////////////////////////////////////////////////
void CInput::RemoveConsoleEventListener( IInputEventListener *pListener )
{
	Listeners::iterator it = std::find(m_consolelisteners.begin(),m_consolelisteners.end(),pListener);
	if (it != m_consolelisteners.end())
	{
		m_consolelisteners.erase( it );
	}
}

//////////////////////////////////////////////////////////////////////////
void CInput::EnableEventPosting ( bool bEnable )
{
	m_postingenable = bEnable;
}

//////////////////////////////////////////////////////////////////////////
void CInput::PostInputEvent( const SInputEvent &event )
{
	if (m_postingenable)
	{
		BroadcastEvent( event );
	}
}

//////////////////////////////////////////////////////////////////////////
void CInput::BroadcastEvent( const SInputEvent &event )
{
	if (!m_postingenable)
	{
		return;
	}
	
	for (Listeners::const_iterator it = m_consolelisteners.begin(); it != m_consolelisteners.end(); ++it)
	{
		if ((*it)->OnInputEvent( event ))
			break;
	}

	if (m_exclusivelistener)
	{
		m_exclusivelistener->OnInputEvent( event );

		return;
	}

	// Send this event to all listeners.
	for (Listeners::const_iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		if ((*it)->OnInputEvent( event ))
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CInput::SetExclusiveListener( IInputEventListener *pListener )
{
	m_exclusivelistener = pListener;
}

//////////////////////////////////////////////////////////////////////////
IInputEventListener *CInput::GetExclusiveListener()
{
	return m_exclusivelistener;
}

//////////////////////////////////////////////////////////////////////////
int CInput::GetModifiers() const
{
	return m_Keyboard.GetModifiers();
}