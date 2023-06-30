// XKeyboard.cpp: implementation of the CXKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Input.h"
#include <ILog.h>
#include <stdio.h>
#ifdef	PS2
#else
#include <dinput.h>
#endif
#include "XKeyboard.h"

#include <ISystem.h>
#include <IConsole.h>
#include "Input.h"

/////////////////////PS2 SPECIFIC//////////////////////////////////////////
#ifdef PS2

static const char notImplemented[]={"Not Yet Implemented"};

static int NumKeyb=2;
static int result=0;
#endif


#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
XAsciiKey CXKeyboard::m_AsciiTable[256];

CXKeyboard::CXKeyboard()
{
	m_pKeyboard = NULL;
	m_bExclusiveMode=false;
	m_cvBufferedKeys=NULL;
	m_cvDirectInputKeys=NULL;
	m_pSystem=NULL;
	m_modifiers = 0;
	m_iToggleState = 0;
}

CXKeyboard::~CXKeyboard()
{

}

#ifdef PS2

bool CXKeyboard::Init(ILog *pLog)
{

	//No joystick emulation at start.....
	m_pJoy=0;

	m_Initialized=false;
	
	//Init log pointer
	m_pLog=pLog;
	
	//Init the usb keyboard system
	if(sceUsbKbInit(&NumKeyb)!=USBKB_OK)
	{
		return false;
	}
	
	//Check if keyborard is connected
	if(sceUsbKbGetInfo(&keyinfo)!=USBKB_OK)
	{
		return false;
	}
	
	//Wait for getting info
	sceUsbKbSync(USBKB_WAIT,&result);
	
	
	//Check if there is at least one keyboard connected
	if(keyinfo.now_connect==0)
	{
		return false;
	}
	
	
	//Find the first connected keyboad
	for(int counter=0;counter<keyinfo.max_connect;counter++)
	{
		if(keyinfo.status[counter])
		{
			m_KeyboardNumber=counter;
			break;
		}
	}
	
	//Read character as ASCII directly
	sceUsbKbSetCodeType(m_KeyboardNumber,USBKB_CODETYPE_RAW);
	
	
	//Set the read mode
	sceUsbKbSetReadMode(m_KeyboardNumber,USBKB_RMODE_INPUTCHAR);
	


	//Setup key names
	/////SetupKeyNames();

	m_modifiers = 0;

	m_Initialized=true;
	return true;

}

#else
//////////////////////////////////////////////////////////////////////////
bool CXKeyboard::Init(CInput *pInput,ISystem *pSystem, LPDIRECTINPUT8 &g_pdi,HINSTANCE hinst,HWND hwnd)
{
	m_pSystem=pSystem;
	m_pLog=pSystem->GetILog();
	m_pInput=pInput;
	m_hinst=hinst;

	m_hwnd=hwnd;
	
	HRESULT hr;
	DIPROPDWORD dipdw = {{sizeof(DIPROPDWORD), sizeof(DIPROPHEADER), 0, DIPH_DEVICE}, KEYFLAG_BUFFERSIZE};

	m_pLog->LogToFile("Initializing Keyboard\n");

	m_cvBufferedKeys = m_pSystem->GetIConsole()->CreateVariable("i_bufferedkeys", "1",0,
		"Toggles key buffering.\n"
		"Usage: i_bufferedkeys [0/1]\n"
		"Default is 0 (off). Set to 1 to process buffered key strokes.");

	m_cvDirectInputKeys = m_pSystem->GetIConsole()->CreateVariable("i_dinputkeys", "1",VF_DUMPTODISK,
		"Toggles use of directX for keys data retrieval.\n"
		"Usage: i_dinputkeys [0/1]\n"
		"Default is 1 (on). Set to 0 to process keys events from windows (under Win32).");

	if (g_pdi)
	{

		hr = g_pdi->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL);
		if (FAILED(hr)) 
		{
			m_pLog->LogToFile("Cannot Create Keyboard Device\n");
			return false;
		}				

		hr = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
		if (FAILED(hr)) 
		{
			m_pLog->LogToFile("Cannot Set Keyboard Data Format\n");
			return false;
		}				

		if (m_hwnd != 0)
		{
			hr = m_pKeyboard->SetCooperativeLevel(m_hwnd, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND|DISCL_NOWINKEY);
			if (FAILED(hr)) 
			{
				m_pLog->LogToFile("Cannot set Keyboard Cooperative level %0xX\n",hr );
				return false;
			}						
		}

		hr = m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);	
		if (FAILED(hr)) 
		{
			m_pLog->LogToFile("Cannot Set Di Buffer Size\n");
			return false;
		}
	}
 	
	memset(m_cKeysState, 0, sizeof(m_cKeysState));
	memset(m_cOldKeysState, 0, sizeof(m_cOldKeysState));

	Acquire();	
  SetupKeyNames();

	return true;
}
#endif //PS2

//////////////////////////////////////////////////////////////////////////
void CXKeyboard::SetExclusive(bool value,void *hwnd)
{ 

	if (m_bExclusiveMode==value && hwnd == m_hwnd)
		return;

#ifdef WIN32		
	HRESULT hr;

	if (hwnd)
		m_hwnd = (HWND)hwnd;

	HWND wind=m_hwnd;

	// flush the buffer when we switch focus
	//DWORD dwItems = INFINITE; 

	// dwItems = Number of elements flushed
	//hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &dwItems, 0); 

	/*
	if (SUCCEEDED(hr)) 
	{ 
			// Buffer successfully flushed...I hope :)			
			if (hr == DI_BUFFEROVERFLOW) 
			{ 
					// Buffer had overflowed. 
			} 
	} 
	*/

	UnAcquire();
 
	m_iToggleState = 0;

	// get the current toggle key states
	if (::GetKeyState(VK_NUMLOCK) & 0x01)
	{
		m_iToggleState |= TOGGLE_NUMLOCK;
		m_cKeysState[DIK_NUMLOCK] |= 0x01;
	}
	else
	{
		m_cKeysState[DIK_NUMLOCK] &= ~0x01;
	}

	if (::GetKeyState(VK_CAPITAL) & 0x01)
	{
		m_iToggleState |= TOGGLE_CAPSLOCK;
		m_cKeysState[DIK_CAPSLOCK] |= 0x01;
	}
	else
	{
		m_cKeysState[DIK_CAPSLOCK] &= ~0x01;
	}

	if (::GetKeyState(VK_SCROLL) & 0x01)
	{
		m_iToggleState |= TOGGLE_SCROLLLOCK;
		m_cKeysState[DIK_SCROLL] |= 0x01;
	}
	else
	{
		m_cKeysState[DIK_SCROLL] &= ~0x01;
	}

	if (m_cvDirectInputKeys->GetIVal())
	{	
		if (value)
		{		
			hr = m_pKeyboard->SetCooperativeLevel(wind,DISCL_FOREGROUND|DISCL_EXCLUSIVE|DISCL_NOWINKEY);

			if (FAILED(hr))
			{
				m_pLog->LogToFile("Cannot Set Keyboard Exclusive Mode\n");
				return;
			}
		}
		else
		{
			hr = m_pKeyboard->SetCooperativeLevel(wind,DISCL_FOREGROUND|DISCL_NONEXCLUSIVE|DISCL_NOWINKEY);
			if(FAILED(hr))
			{
				m_pLog->LogToFile("Cannot Set Keyboard Non-Exclusive Mode\n");
				return;
			}				
		}
	}
#endif

	if (!Acquire()) 
		return;

	m_bExclusiveMode = value;	
}

//////////////////////////////////////////////////////////////////////////
bool CXKeyboard::Acquire()
{
#ifdef PS2
	return true;
#else
	m_modifiers = 0;
	if (m_cvDirectInputKeys->GetIVal())
	{	
		if (m_pKeyboard && m_pKeyboard->Acquire()) 
			return (true);
		return (false);	
	}
	return (true);
#endif
}

//////////////////////////////////////////////////////////////////////////
bool CXKeyboard::UnAcquire()
{
#ifdef PS2
	return true;
#else
	m_modifiers = 0;
	if (m_cvDirectInputKeys->GetIVal())
	{	
		if (m_pKeyboard && m_pKeyboard->Unacquire()) 
			return (true);
		return (false);
	}
	return (true);
#endif	
}

//////////////////////////////////////////////////////////////////////////
unsigned short CXKeyboard::DIK2XKEY(unsigned char cCode )
{
#ifndef PS2
   switch(cCode)
   {
      case DIK_ESCAPE:        return XKEY_ESCAPE;
      case DIK_1:             return XKEY_1;
      case DIK_2:             return XKEY_2;
      case DIK_3:             return XKEY_3;
      case DIK_4:             return XKEY_4;
      case DIK_5:             return XKEY_5;
      case DIK_6:             return XKEY_6;
      case DIK_7:             return XKEY_7;
      case DIK_8:             return XKEY_8;
      case DIK_9:             return XKEY_9;
      case DIK_0:             return XKEY_0;
      case DIK_MINUS:         return XKEY_MINUS;
      case DIK_EQUALS:        return XKEY_EQUALS;
      case DIK_BACK:          return XKEY_BACKSPACE;
      case DIK_TAB:           return XKEY_TAB;
      case DIK_Q:             return XKEY_Q;
      case DIK_W:             return XKEY_W;
      case DIK_E:             return XKEY_E;
      case DIK_R:             return XKEY_R;
      case DIK_T:             return XKEY_T;
      case DIK_Y:             return XKEY_Y;
      case DIK_U:             return XKEY_U;
      case DIK_I:             return XKEY_I;
      case DIK_O:             return XKEY_O;
      case DIK_P:             return XKEY_P;
      case DIK_LBRACKET:      return XKEY_LBRACKET;
      case DIK_RBRACKET:      return XKEY_RBRACKET;
      case DIK_RETURN:        return XKEY_RETURN;
      case DIK_LCONTROL:      return XKEY_LCONTROL;
      case DIK_A:             return XKEY_A;
      case DIK_S:             return XKEY_S;
      case DIK_D:             return XKEY_D;
      case DIK_F:             return XKEY_F;
      case DIK_G:             return XKEY_G;
      case DIK_H:             return XKEY_H;
      case DIK_J:             return XKEY_J;
      case DIK_K:             return XKEY_K;
      case DIK_L:             return XKEY_L;
      case DIK_SEMICOLON:     return XKEY_SEMICOLON;
      case DIK_APOSTROPHE:    return XKEY_APOSTROPHE;
      case DIK_GRAVE:         return XKEY_TILDE;
      case DIK_LSHIFT:        return XKEY_LSHIFT;
      case DIK_BACKSLASH:     return XKEY_BACKSLASH;
      case DIK_Z:             return XKEY_Z;
      case DIK_X:             return XKEY_X;
      case DIK_C:             return XKEY_C;
      case DIK_V:             return XKEY_V;
      case DIK_B:             return XKEY_B;
      case DIK_N:             return XKEY_N;
      case DIK_M:             return XKEY_M;
      case DIK_COMMA:         return XKEY_COMMA;
      case DIK_PERIOD:        return XKEY_PERIOD;
      case DIK_SLASH:         return XKEY_SLASH;
      case DIK_RSHIFT:        return XKEY_RSHIFT;
      case DIK_MULTIPLY:      return XKEY_MULTIPLY;
      case DIK_LALT:          return XKEY_LALT;
      case DIK_SPACE:         return XKEY_SPACE;
      case DIK_CAPSLOCK:      return XKEY_CAPSLOCK;
      case DIK_F1:            return XKEY_F1;
      case DIK_F2:            return XKEY_F2;
      case DIK_F3:            return XKEY_F3;
      case DIK_F4:            return XKEY_F4;
      case DIK_F5:            return XKEY_F5;
      case DIK_F6:            return XKEY_F6;
      case DIK_F7:            return XKEY_F7;
      case DIK_F8:            return XKEY_F8;
      case DIK_F9:            return XKEY_F9;
      case DIK_F10:           return XKEY_F10;
      case DIK_NUMLOCK:       return XKEY_NUMLOCK;
      case DIK_SCROLL:        return XKEY_SCROLLLOCK;
      case DIK_NUMPAD7:       return XKEY_NUMPAD7;
      case DIK_NUMPAD8:       return XKEY_NUMPAD8;
      case DIK_NUMPAD9:       return XKEY_NUMPAD9;
      case DIK_SUBTRACT:      return XKEY_SUBTRACT;
      case DIK_NUMPAD4:       return XKEY_NUMPAD4;
      case DIK_NUMPAD5:       return XKEY_NUMPAD5;
      case DIK_NUMPAD6:       return XKEY_NUMPAD6;
      case DIK_ADD:           return XKEY_ADD;
      case DIK_NUMPAD1:       return XKEY_NUMPAD1;
      case DIK_NUMPAD2:       return XKEY_NUMPAD2;
      case DIK_NUMPAD3:       return XKEY_NUMPAD3;
      case DIK_NUMPAD0:       return XKEY_NUMPAD0;
      case DIK_DECIMAL:       return XKEY_DECIMAL;
      case DIK_F11:           return XKEY_F11;
      case DIK_F12:           return XKEY_F12;
      case DIK_F13:           return XKEY_F13;
      case DIK_F14:           return XKEY_F14;
      case DIK_F15:           return XKEY_F15;
      case DIK_KANA:          return 0;
      case DIK_CONVERT:       return 0;
      case DIK_NOCONVERT:     return 0;
      case DIK_YEN:           return 0;
      case DIK_NUMPADEQUALS:  return 0;
      case DIK_CIRCUMFLEX:    return 0;
      case DIK_AT:            return 0;
      case DIK_COLON:         return 0;
      case DIK_UNDERLINE:     return 0;
      case DIK_KANJI:         return 0;
      case DIK_STOP:          return 0;
      case DIK_AX:            return 0;
      case DIK_UNLABELED:     return 0;
      case DIK_NUMPADENTER:   return XKEY_NUMPADENTER;
      case DIK_RCONTROL:      return XKEY_RCONTROL;
      case DIK_NUMPADCOMMA:   return XKEY_SEPARATOR;
      case DIK_DIVIDE:        return XKEY_DIVIDE;
      case DIK_SYSRQ:         return XKEY_PRINT;
      case DIK_RALT:          return XKEY_RALT;
      case DIK_PAUSE:         return XKEY_PAUSE;
      case DIK_HOME:          return XKEY_HOME;
      case DIK_UP:            return XKEY_UP;
      case DIK_PGUP:          return XKEY_PAGE_UP;
      case DIK_LEFT:          return XKEY_LEFT;
      case DIK_RIGHT:         return XKEY_RIGHT;
      case DIK_END:           return XKEY_END;
      case DIK_DOWN:          return XKEY_DOWN;
      case DIK_PGDN:          return XKEY_PAGE_DOWN;
      case DIK_INSERT:        return XKEY_INSERT;
      case DIK_DELETE:        return XKEY_DELETE;
      case DIK_LWIN:          return XKEY_WIN_LWINDOW;
      case DIK_RWIN:          return XKEY_WIN_RWINDOW;
      case DIK_APPS:          return XKEY_WIN_APPS;
      case DIK_OEM_102:       return XKEY_OEM_102;
   }

   #else
   
   	///////CONVERT USB  CODE IN ENGINE CODE (IF POSSIBLE)


   switch (cCode)
   {
	case kBACKSPACE:  		return XKEY_BACKSPACE;
	case kTAB:	   			return XKEY_TAB;        
	case kENTER	:   		return XKEY_RETURN;     
	//TO DO case -1; 				return XKEY_PAUSE;     //TO DO 	
	case kCAPSLOCK:			return XKEY_CAPSLOCK;   	
	case kESC:				return XKEY_ESCAPE;   
	case kSPACE:			return XKEY_SPACE;      
	case kPGDN:				return XKEY_PAGE_DOWN;  
	case kPGUP:				return XKEY_PAGE_UP;    
	case kEND:				return XKEY_END;        
	case kHOME:				return XKEY_HOME;       
	case kLARROW:			return XKEY_LEFT;       
	case kUARROW:			return XKEY_UP;         
	case kRARROW:			return XKEY_RIGHT;      
	case kDARROW:			return XKEY_DOWN;       
	case kCTRLPRTSC:		return XKEY_PRINT;      
	case kINS:				return XKEY_INSERT;     
	case kDEL:				return XKEY_DELETE;     
	//TO DO case -1: 				return XKEY_HELP;       //TO DO 
	case k0:		return XKEY_0;           
	case k1:		return XKEY_1;          
	case k2:		return XKEY_2;          
	case k3:		return XKEY_3;          
	case k4:		return XKEY_4;          
	case k5:		return XKEY_5;          
	case k6:		return XKEY_6;          
	case k7:		return XKEY_7;          
	case k8:		return XKEY_8;          
	case k9:		return XKEY_9;          
	case kA:		return XKEY_A;          
	case kB:		return XKEY_B;          
	case kC:		return XKEY_C;          
	case kD:		return XKEY_D;          
	case kE:		return XKEY_E;          
	case kF:		return XKEY_F;          
	case kG:		return XKEY_G;          
	case kH:		return XKEY_H;          
	case kI:		return XKEY_I;          
	case kJ:		return XKEY_J;          
	case kK:		return XKEY_K;          
	case kL:		return XKEY_L;          
	case kM:		return XKEY_M;          
	case kN:		return XKEY_N;          
	case kO:		return XKEY_O;          
	case kP:		return XKEY_P;          
	case kQ:		return XKEY_Q;          
	case kR:		return XKEY_R;          
	case kS:		return XKEY_S;          
	case kT:		return XKEY_T;          
	case kU:		return XKEY_U;          
	case kV:		return XKEY_V;          
	case kW:		return XKEY_W;          
	case kX:		return XKEY_X;          
	case kY:		return XKEY_Y;          
	case kZ:		return XKEY_Z;          
	//TO DO case -1; 				return XKEY_TILDE;      //TO DO 
	case kMINUS:			return XKEY_MINUS;      
	case kEQUAL:			return XKEY_EQUALS;     
	case kLBRACKET:			return XKEY_LBRACKET;   
	case kRBRACKET:			return XKEY_RBRACKET;   
	case kBACKSLASH:		return XKEY_BACKSLASH;  
	case kSEMICOLON:		return XKEY_SEMICOLON;  		
	case kAPOSTROPHE:		return XKEY_APOSTROPHE; 
	case kCOMMA:			return XKEY_COMMA; 	
	case kPERIOD:			return XKEY_PERIOD;     	
	case kSLASH:			return XKEY_SLASH;      	
	case kKEYPAD0:			return XKEY_NUMPAD0;
	case kKEYPAD1:			return XKEY_NUMPAD1;    		
	case kKEYPAD2:			return XKEY_NUMPAD2;    		
	case kKEYPAD3:			return XKEY_NUMPAD3;    		
	case kKEYPAD4:			return XKEY_NUMPAD4;    		
	case kKEYPAD5:			return XKEY_NUMPAD5;    		
	case kKEYPAD6:			return XKEY_NUMPAD6;    		
	case kKEYPAD7:			return XKEY_NUMPAD7;    		
	case kKEYPAD8:			return XKEY_NUMPAD8;    		
	case kKEYPAD9:			return XKEY_NUMPAD9;    			
	case kKEYPADSTAR:		return XKEY_MULTIPLY;     			
	case kKEYPADPLUS:		return XKEY_ADD;    			
	//TO DO case -1;					return XKEY_SEPARATOR; 			
	case kKEYPADMINUS:		return XKEY_SUBTRACT;   
	//TO DO case -1;					return XKEY_DECIMAL;    //TO DO 
	case kKEYPADSLASH:		return XKEY_DIVIDE;     			
	case kKEYPADENTER:		return XKEY_NUMPADENTER;			
	case kF1:				return XKEY_F1;         
	case kF2:				return XKEY_F2;         
	case kF3:				return XKEY_F3;         
	case kF4:				return XKEY_F4;         
	case kF5:				return XKEY_F5;         
	case kF6:				return XKEY_F6;         
	case kF7:				return XKEY_F7;         
	case kF8:				return XKEY_F8;         
	case kF9:				return XKEY_F9;         
	case kF10:				return XKEY_F10;        
	case kF11:				return XKEY_F11;        
	case kF12:				return XKEY_F12;        
	//TO DO case -1;		return XKEY_F13;        
	//TO DO case -1;		return XKEY_F14;        
	//TO DO case -1;		return XKEY_F15;        
	case kNUMLOCK:			return XKEY_NUMLOCK;    
	case kSCROLLLOCK:		return XKEY_SCROLLLOCK; 
	case kLEFTCTRL:			return XKEY_LCONTROL;  	
	case kRIGHTCTRL:		return XKEY_RCONTROL;   
    case kLEFTALT:			return XKEY_LALT;       
    case kRIGHTALT:			return XKEY_RALT;       
    case kLEFTSHIFT:		return XKEY_LSHIFT;     
    case kRIGHTSHIFT:		return XKEY_RSHIFT;     
    //TO DO case -1;		return XKEY_WIN_LWINDOW;
  	//TO DO case -1;		return XKEY_WIN_RWINDOW;
   	//TO DO case -1;		return XKEY_WIN_APPS;   
   	//TO DO case -1;		return XKEY_OEM_102;   	
   	}
   	
   	return XKEY_NULL;
   
   #endif

   return XKEY_NULL;
}

//////////////////////////////////////////////////////////////////////////
unsigned char CXKeyboard::XKEY2DIK(unsigned short nCode )
{
#ifndef PS2
   switch (nCode)
   {
      case XKEY_BACKSPACE:     return DIK_BACK;
      case XKEY_TAB:           return DIK_TAB;
      case XKEY_RETURN:        return DIK_RETURN;
      //XKEY_CONTROL:
      //XKEY_ALT:
      //XKEY_SHIFT:
      case XKEY_PAUSE:         return DIK_PAUSE;
      case XKEY_CAPSLOCK:      return DIK_CAPSLOCK;
      case XKEY_ESCAPE:        return DIK_ESCAPE;
      case XKEY_SPACE:         return DIK_SPACE;
      case XKEY_PAGE_DOWN:     return DIK_PGDN;
      case XKEY_PAGE_UP:       return DIK_PGUP;
      case XKEY_END:           return DIK_END;
      case XKEY_HOME:          return DIK_HOME;
      case XKEY_LEFT:          return DIK_LEFT;
      case XKEY_UP:            return DIK_UP;
      case XKEY_RIGHT:         return DIK_RIGHT;
      case XKEY_DOWN:          return DIK_DOWN;
      case XKEY_PRINT:         return DIK_SYSRQ;
      case XKEY_INSERT:        return DIK_INSERT;
      case XKEY_DELETE:        return DIK_DELETE;
      case XKEY_HELP:          return 0;
      case XKEY_0:             return DIK_0;
      case XKEY_1:             return DIK_1;
      case XKEY_2:             return DIK_2;
      case XKEY_3:             return DIK_3;
      case XKEY_4:             return DIK_4;
      case XKEY_5:             return DIK_5;
      case XKEY_6:             return DIK_6;
      case XKEY_7:             return DIK_7;
      case XKEY_8:             return DIK_8;
      case XKEY_9:             return DIK_9;
      case XKEY_A:             return DIK_A;
      case XKEY_B:             return DIK_B;
      case XKEY_C:             return DIK_C;
      case XKEY_D:             return DIK_D;
      case XKEY_E:             return DIK_E;
      case XKEY_F:             return DIK_F;
      case XKEY_G:             return DIK_G;
      case XKEY_H:             return DIK_H;
      case XKEY_I:             return DIK_I;
      case XKEY_J:             return DIK_J;
      case XKEY_K:             return DIK_K;
      case XKEY_L:             return DIK_L;
      case XKEY_M:             return DIK_M;
      case XKEY_N:             return DIK_N;
      case XKEY_O:             return DIK_O;
      case XKEY_P:             return DIK_P;
      case XKEY_Q:             return DIK_Q;
      case XKEY_R:             return DIK_R;
      case XKEY_S:             return DIK_S;
      case XKEY_T:             return DIK_T;
      case XKEY_U:             return DIK_U;
      case XKEY_V:             return DIK_V;
      case XKEY_W:             return DIK_W;
      case XKEY_X:             return DIK_X;
      case XKEY_Y:             return DIK_Y;
      case XKEY_Z:             return DIK_Z;
      case XKEY_TILDE:         return DIK_GRAVE;
      case XKEY_MINUS:         return DIK_MINUS;
      case XKEY_EQUALS:        return DIK_EQUALS;
      case XKEY_LBRACKET:      return DIK_LBRACKET;
      case XKEY_RBRACKET:      return DIK_RBRACKET;
      case XKEY_BACKSLASH:     return DIK_BACKSLASH;
      case XKEY_SEMICOLON:     return DIK_SEMICOLON;
      case XKEY_APOSTROPHE:    return DIK_APOSTROPHE;
      case XKEY_COMMA:         return DIK_COMMA;
      case XKEY_PERIOD:        return DIK_PERIOD;
      case XKEY_SLASH:         return DIK_SLASH;
      case XKEY_NUMPAD0:       return DIK_NUMPAD0;
      case XKEY_NUMPAD1:       return DIK_NUMPAD1;
      case XKEY_NUMPAD2:       return DIK_NUMPAD2;
      case XKEY_NUMPAD3:       return DIK_NUMPAD3;
      case XKEY_NUMPAD4:       return DIK_NUMPAD4;
      case XKEY_NUMPAD5:       return DIK_NUMPAD5;
      case XKEY_NUMPAD6:       return DIK_NUMPAD6;
      case XKEY_NUMPAD7:       return DIK_NUMPAD7;
      case XKEY_NUMPAD8:       return DIK_NUMPAD8;
      case XKEY_NUMPAD9:       return DIK_NUMPAD9;
      case XKEY_MULTIPLY:      return DIK_MULTIPLY;
      case XKEY_ADD:           return DIK_ADD;
      case XKEY_SEPARATOR:     return DIK_NUMPADCOMMA;
      case XKEY_SUBTRACT:      return DIK_SUBTRACT;
      case XKEY_DECIMAL:       return DIK_DECIMAL;
      case XKEY_DIVIDE:        return DIK_DIVIDE;
      case XKEY_NUMPADENTER:   return DIK_NUMPADENTER;
      case XKEY_F1:            return DIK_F1;
      case XKEY_F2:            return DIK_F2;
      case XKEY_F3:            return DIK_F3;
      case XKEY_F4:            return DIK_F4;
      case XKEY_F5:            return DIK_F5;
      case XKEY_F6:            return DIK_F6;
      case XKEY_F7:            return DIK_F7;
      case XKEY_F8:            return DIK_F8;
      case XKEY_F9:            return DIK_F9;
      case XKEY_F10:           return DIK_F10;
      case XKEY_F11:           return DIK_F11;
      case XKEY_F12:           return DIK_F12;
      case XKEY_F13:           return DIK_F13;
      case XKEY_F14:           return DIK_F14;
      case XKEY_F15:           return DIK_F15;
      case XKEY_NUMLOCK:       return DIK_NUMLOCK;
      case XKEY_SCROLLLOCK:    return DIK_SCROLL;
      case XKEY_LCONTROL:      return DIK_LCONTROL;
      case XKEY_RCONTROL:      return DIK_RCONTROL;
      case XKEY_LALT:          return DIK_LALT;
      case XKEY_RALT:          return DIK_RALT;
      case XKEY_LSHIFT:        return DIK_LSHIFT;
      case XKEY_RSHIFT:        return DIK_RSHIFT;
      case XKEY_WIN_LWINDOW:   return DIK_LWIN;
      case XKEY_WIN_RWINDOW:   return DIK_RWIN;
      case XKEY_WIN_APPS:      return DIK_APPS;
      case XKEY_OEM_102:       return DIK_OEM_102;
      
   };

  #else
  
  	///////CONVERT ENGINE's CODE IN USB (FOR PS2) CODE (IF POSSIBLE)


   switch (nCode)
   {
	case XKEY_BACKSPACE:  		return kBACKSPACE;
	case XKEY_TAB:        		return kTAB;	
	case XKEY_RETURN:     		return kENTER	;
	case XKEY_PAUSE:      		return -1; 				//TO DO 	
	case XKEY_CAPSLOCK:   		return kCAPSLOCK;			
	case XKEY_ESCAPE:     		return kESC;	
	case XKEY_SPACE:      		return kSPACE;
	case XKEY_PAGE_DOWN:  		return kPGDN;
	case XKEY_PAGE_UP:    		return kPGUP;
	case XKEY_END:        		return kEND;	
	case XKEY_HOME:       		return kHOME;	
	case XKEY_LEFT:       		return kLARROW;
	case XKEY_UP:         		return kUARROW;
	case XKEY_RIGHT:      		return kRARROW;
	case XKEY_DOWN:       		return kDARROW;
	case XKEY_PRINT:      		return kCTRLPRTSC;
	case XKEY_INSERT:     		return kINS;
	case XKEY_DELETE:     		return kDEL;	
	case XKEY_HELP:       		return -1; 				//TO DO 
	case XKEY_0:          		return k0;
	case XKEY_1:          		return k1;
	case XKEY_2:          		return k2;
	case XKEY_3:          		return k3;
	case XKEY_4:          		return k4;
	case XKEY_5:          		return k5;
	case XKEY_6:          		return k6;
	case XKEY_7:          		return k7;
	case XKEY_8:          		return k8;
	case XKEY_9:          		return k9;
	case XKEY_A:          		return kA;
	case XKEY_B:          		return kB;
	case XKEY_C:          		return kC;
	case XKEY_D:          		return kD;
	case XKEY_E:          		return kE;
	case XKEY_F:          		return kF;
	case XKEY_G:          		return kG;
	case XKEY_H:          		return kH;
	case XKEY_I:          		return kI;
	case XKEY_J:          		return kJ;
	case XKEY_K:          		return kK;
	case XKEY_L:          		return kL;
	case XKEY_M:          		return kM;
	case XKEY_N:          		return kN;
	case XKEY_O:          		return kO;
	case XKEY_P:          		return kP;
	case XKEY_Q:          		return kQ;
	case XKEY_R:          		return kR;
	case XKEY_S:          		return kS;
	case XKEY_T:          		return kT;
	case XKEY_U:          		return kU;
	case XKEY_V:          		return kV;
	case XKEY_W:          		return kW;
	case XKEY_X:          		return kX;
	case XKEY_Y:          		return kY;
	case XKEY_Z:          		return kZ;
	case XKEY_TILDE:      		return -1; 				//TO DO 
	case XKEY_MINUS:      		return kMINUS;
	case XKEY_EQUALS:     		return kEQUAL;
	case XKEY_LBRACKET:   		return kLBRACKET;
	case XKEY_RBRACKET:   		return kRBRACKET;
	case XKEY_BACKSLASH:  		return kBACKSLASH;
	case XKEY_SEMICOLON:  		return kSEMICOLON;				
	case XKEY_APOSTROPHE: 		return kAPOSTROPHE;		
	case XKEY_COMMA:      		return kCOMMA;				
	case XKEY_PERIOD:     		return kPERIOD;				
	case XKEY_SLASH:      		return kSLASH;				
	case XKEY_NUMPAD0:    		return kKEYPAD0;				
	case XKEY_NUMPAD1:    		return kKEYPAD1;				
	case XKEY_NUMPAD2:    		return kKEYPAD2;				
	case XKEY_NUMPAD3:    		return kKEYPAD3;				
	case XKEY_NUMPAD4:    		return kKEYPAD4;				
	case XKEY_NUMPAD5:    		return kKEYPAD5;				
	case XKEY_NUMPAD6:    		return kKEYPAD6;				
	case XKEY_NUMPAD7:    		return kKEYPAD7;				
	case XKEY_NUMPAD8:    		return kKEYPAD8;				
	case XKEY_NUMPAD9:    		return kKEYPAD9;					
	case XKEY_MULTIPLY:   		return kKEYPADSTAR;		  			
	case XKEY_ADD:        		return kKEYPADPLUS;		 			
	case XKEY_SEPARATOR:  		return -1;				//TO DO 			
	case XKEY_SUBTRACT:   		return kKEYPADMINUS; 	
	case XKEY_DECIMAL:    		return -1;				//TO DO 
	case XKEY_DIVIDE:     		return kKEYPADSLASH;				
	case XKEY_NUMPADENTER:		return kKEYPADENTER;				
	case XKEY_F1:         		return kF1;	
	case XKEY_F2:         		return kF2;	
	case XKEY_F3:         		return kF3;	
	case XKEY_F4:         		return kF4;	
	case XKEY_F5:         		return kF5;	
	case XKEY_F6:         		return kF6;	
	case XKEY_F7:         		return kF7;	
	case XKEY_F8:         		return kF8;	
	case XKEY_F9:         		return kF9;	
	case XKEY_F10:        		return kF10;
	case XKEY_F11:        		return kF11;
	case XKEY_F12:        		return kF12;
	case XKEY_F13:        		return -1;
	case XKEY_F14:        		return -1;
	case XKEY_F15:        		return -1;
	case XKEY_NUMLOCK:    		return kNUMLOCK;
	case XKEY_SCROLLLOCK: 		return kSCROLLLOCK;
	case XKEY_LCONTROL:  		return kLEFTCTRL;
	case XKEY_RCONTROL:   		return kRIGHTCTRL;
    case XKEY_LALT:       		return kLEFTALT;
    case XKEY_RALT:       		return kRIGHTALT;
    case XKEY_LSHIFT:     		return kLEFTSHIFT;
    case XKEY_RSHIFT:     		return kRIGHTSHIFT;
    case XKEY_WIN_LWINDOW:		return -1;
  	case XKEY_WIN_RWINDOW:		return -1;
   	case XKEY_WIN_APPS:   		return -1;
   	case XKEY_OEM_102:   		return -1;
   	}
   	
   	
   	return -1;
   	
  #endif //PS2 SPECIFIC

   return 0;
}

//////////////////////////////////////////////////////////////////////////
unsigned char CXKeyboard::XKEY2ASCII(unsigned short nCode,int modifiers)
{
#ifndef PS2

	if ((modifiers & XKEY_MOD_CONTROL) && (modifiers & XKEY_MOD_ALT))
	{
		return m_AsciiTable[XKEY2DIK(nCode)].ac[0];
	}
	else if ((modifiers & XKEY_MOD_CAPSLOCK) != 0)
	{
		return m_AsciiTable[XKEY2DIK(nCode)].cl[0];
	}
	else if ((modifiers & XKEY_MOD_SHIFT) != 0)
	{
		return m_AsciiTable[XKEY2DIK(nCode)].uc[0];
	}
	return m_AsciiTable[XKEY2DIK(nCode)].lc[0];
#else

	/////TO DO PS2 VERSION
	return 0;

#endif
}

bool CXKeyboard::GetOSKeyName(int nKey, wchar_t *szwKeyName, int iBufSize)
{
	if (IS_KEYBOARD_KEY(nKey) && m_pKeyboard)
	{
		int iDIK = XKEY2DIK(nKey);

		if (iDIK)
		{
			DIDEVICEOBJECTINSTANCE dido;

			ZeroMemory(&dido, sizeof(DIDEVICEOBJECTINSTANCE));
			dido.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);

			if (m_pKeyboard->GetObjectInfo(&dido, iDIK, DIPH_BYOFFSET) != DI_OK)
			{
				return false;
			}

			wchar_t szwMyKeyName[256]={0};
      
			swprintf(szwMyKeyName, L"%S", dido.tszName);
			wcsncpy(szwKeyName, szwMyKeyName, iBufSize);

			return true;
		}
		return false;
	}
	return false;
}

#ifdef PS2
//////////////////////////////////////////////////////////////////////////
inline MapVirtualKey(int x, int y)
{
	return x;
}

//////////////////////////////////////////////////////////////////////////
inline ToAscii(unsigned long vKeyCode, int k, unsigned char sKState[], unsigned short ascii[], int val )
{
	return vKeyCode;
}

#endif //PS2


//////////////////////////////////////////////////////////////////////////
void CXKeyboard::SetupKeyNames()
{
#ifndef PS2
	////////////////////////////////////////////////////////////
	for (int k=0;k<256;k++) 
	{
    memset(m_AsciiTable[k].lc,0,sizeof(m_AsciiTable[k].lc));
		memset(m_AsciiTable[k].uc,0,sizeof(m_AsciiTable[k].uc));
		memset(m_AsciiTable[k].ac,0,sizeof(m_AsciiTable[k].ac));
		memset(m_AsciiTable[k].cl,0,sizeof(m_AsciiTable[k].cl));
	}
	
	unsigned char sKState[256];
	unsigned short ascii[2];
	int nResult;
	unsigned int vKeyCode;

	for (k=0;k<256;k++)
	{
		vKeyCode = MapVirtualKeyEx( k, 1, GetKeyboardLayout(0) );

		// lower case
		{
			memset(sKState,0,sizeof(sKState));
		
			ascii[0] = ascii[1] = 0;
			nResult = ToAsciiEx( vKeyCode, k, sKState, ascii, 0, GetKeyboardLayout(0) );
		
			if (nResult == 2)
				m_AsciiTable[k].lc[0] = (char)ascii[1] ? ascii[1] : (ascii[0] >> 8);
			else if (nResult == 1)
				m_AsciiTable[k].lc[0] = (char)ascii[0];
		}			
	
		// upper case
		{
			ascii[0] = ascii[1] = 0;
			sKState[VK_SHIFT] = 0x80;
			nResult = ToAsciiEx( vKeyCode, k, sKState, ascii, 0, GetKeyboardLayout(0) );

			if (nResult == 2)
				m_AsciiTable[k].uc[0] = (char)ascii[1] ? ascii[1] : (ascii[0] >> 8);
			else if (nResult == 1)
				m_AsciiTable[k].uc[0] = (char)ascii[0];
		}

		// alternate
		{
			ascii[0] = ascii[1] = 0;
			memset(sKState,0,sizeof(sKState));
			sKState[VK_CONTROL] = 0x80;
			sKState[VK_MENU] = 0x80;
			sKState[VK_LCONTROL] = 0x80;
			sKState[VK_LMENU] = 0x80;

			nResult = ToAsciiEx( vKeyCode, k, sKState, ascii, 0, GetKeyboardLayout(0) );
			m_AsciiTable[k].ac[0] = (char)ascii[0];
		}

		// caps lock
		{
			ascii[0] = ascii[1] = 0;
			memset(sKState,0,sizeof(sKState));
//			sKState[VK_SHIFT] = 0x80;
			sKState[VK_CAPITAL] = 0x01;

			nResult = ToAsciiEx( vKeyCode, k, sKState, ascii, 0, GetKeyboardLayout(0) );

			if (nResult == 2)
				m_AsciiTable[k].cl[0] = (char)ascii[1] ? ascii[1] : (ascii[0] >> 8);
			else if (nResult == 1)
				m_AsciiTable[k].cl[0] = (char)ascii[0];
		}
	}

	memset(sKState,0,sizeof(sKState));

	for (int k = 0; k < 256; k++)
	{
		vKeyCode = MapVirtualKeyEx( k, 1, GetKeyboardLayout(0) );

		sKState[k] = 0x81;

		ascii[0] = ascii[1] = 0;
		if (m_AsciiTable[k].lc[0] == 0)
		{
			nResult = ToAsciiEx( vKeyCode, k, sKState, ascii, 0, GetKeyboardLayout(0) );
			
			if (nResult == 2)
				m_AsciiTable[k].lc[0] = (char)ascii[1] ? ascii[1] : (ascii[0] >> 8);
			else if (nResult == 1)
				m_AsciiTable[k].lc[0] = (char)ascii[0];
		}

		ascii[0] = ascii[1] = 0;
		if (m_AsciiTable[k].uc[0] == 0)
		{
			sKState[VK_SHIFT] = 0x80;
			ToAsciiEx( vKeyCode, k, sKState, ascii, 0, GetKeyboardLayout(0) );
			sKState[VK_SHIFT] = 0;

			if (nResult == 2)
				m_AsciiTable[k].uc[0] = (char)ascii[1] ? ascii[1] : (ascii[0] >> 8);
			else if (nResult == 1)
				m_AsciiTable[k].uc[0] = (char)ascii[0];
		}

		ascii[0] = ascii[1] = 0;
		if (m_AsciiTable[k].cl[0] == 0)
		{
			sKState[VK_CAPITAL] = 0x80;
			ToAsciiEx( vKeyCode, k, sKState, ascii, 0, GetKeyboardLayout(0) );
			sKState[VK_CAPITAL] = 0;

			if (nResult == 2)
				m_AsciiTable[k].cl[0] = (char)ascii[1] ? ascii[1] : (ascii[0] >> 8);
			else if (nResult == 1)
				m_AsciiTable[k].cl[0] = (char)ascii[0];
		}

		sKState[k] = 0;
	}



#else

	////PS2 SPECIFIC CODE
	
#endif 
}

//////////////////////////////////////////////////////////////////////////
void CXKeyboard::FeedVirtualKey(int nVirtualKey,long lParam,bool bDown)
{
	if (m_cvDirectInputKeys->GetIVal())
		return; // use dx8

	if (!m_bExclusiveMode)
		return; // windows has no focus
	
	// hardcoded windows system key values(?) 
	// couldn't find them
	if (nVirtualKey==0x10) // SHIFT
	{
		if (lParam & 0x00100000)
			nVirtualKey=VK_RSHIFT;
		else
			nVirtualKey=VK_LSHIFT;
	}
	else
	if (nVirtualKey==0x11)
	{
		if (lParam & 0x01000000) // ok
			nVirtualKey=VK_RCONTROL;
		else
			nVirtualKey=VK_LCONTROL;
	}
	else
	if (nVirtualKey==0x12)
	{
		if (lParam & 0x01000000)
			nVirtualKey=VK_RMENU;
		else
			nVirtualKey=VK_LMENU;
	}

	int cKey=XKEY2DIK(m_pInput->VK2XKEY(nVirtualKey));
	m_cTempKeys[cKey]=m_cKeysState[cKey];
	
	ProcessKey(cKey,bDown,m_cTempKeys);

	m_cOldKeysState[cKey]=m_cKeysState[cKey];		
	m_cKeysState[cKey]=m_cTempKeys[cKey];

	//m_pLog->Log("Keypressed=%s",GetKeyDownName());
}

//////////////////////////////////////////////////////////////////////////
void CXKeyboard::ProcessKey(int cKey,bool bPressed,unsigned char *cTempKeys)
{
	if (bPressed)
	{
		cTempKeys[cKey] |= 0x80;
	}
	else
	{
		cTempKeys[cKey] &= ~0x80;
	}		

	if (bPressed)
	{
		if (cKey == DIK_LSHIFT) m_modifiers |= XKEY_MOD_LSHIFT;
		if (cKey == DIK_RSHIFT) m_modifiers |= XKEY_MOD_RSHIFT;
		if (cKey == DIK_LCONTROL) m_modifiers |= XKEY_MOD_LCONTROL;
		if (cKey == DIK_RCONTROL) m_modifiers |= XKEY_MOD_RCONTROL;
		if (cKey == DIK_LALT) m_modifiers |= XKEY_MOD_LALT;
		if (cKey == DIK_RALT) m_modifiers |= XKEY_MOD_RALT;

		if (cKey == DIK_CAPSLOCK)
		{
			if (m_iToggleState & TOGGLE_CAPSLOCK)
			{
				m_iToggleState &= ~TOGGLE_CAPSLOCK;
				cTempKeys[cKey] &= ~0x01;
			}
			else
			{
				m_iToggleState |= TOGGLE_CAPSLOCK;
				cTempKeys[cKey] |= 0x01;
			}
		}
		if (cKey == DIK_NUMLOCK)
		{
			if (m_iToggleState & TOGGLE_NUMLOCK)
			{
				m_iToggleState &= ~TOGGLE_NUMLOCK;
				cTempKeys[cKey] &= ~0x01;
			}
			else
			{
				m_iToggleState |= TOGGLE_NUMLOCK;
				cTempKeys[cKey] |= 0x01;
			}
		}
		if (cKey == DIK_SCROLL)
		{
			if (m_iToggleState & TOGGLE_SCROLLLOCK)
			{
				m_iToggleState &= ~TOGGLE_SCROLLLOCK;
				cTempKeys[cKey] &= ~0x01;
			}
			else
			{
				m_iToggleState |= TOGGLE_SCROLLLOCK;
				cTempKeys[cKey] |= 0x01;
			}
		}
	}
	else if (m_cKeysState[cKey] & 0x80)
	{
		if (cKey == DIK_LSHIFT) m_modifiers &= ~XKEY_MOD_LSHIFT;
		if (cKey == DIK_RSHIFT) m_modifiers &= ~XKEY_MOD_RSHIFT;
		if (cKey == DIK_LCONTROL) m_modifiers &= ~XKEY_MOD_LCONTROL;
		if (cKey == DIK_RCONTROL) m_modifiers &= ~XKEY_MOD_RCONTROL;
		if (cKey == DIK_LALT) m_modifiers &= ~XKEY_MOD_LALT;
		if (cKey == DIK_RALT) m_modifiers &= ~XKEY_MOD_RALT;
	}
	else
	{
		return;
	}

	if(m_iToggleState & TOGGLE_CAPSLOCK)
		m_modifiers |= XKEY_MOD_CAPSLOCK;
	else
		m_modifiers &= ~XKEY_MOD_CAPSLOCK;

	// Post Input events.
	SInputEvent event;
	event.key = DIK2XKEY( (unsigned char)cKey );
	if (bPressed)
		event.type = SInputEvent::KEY_PRESS;
	else
		event.type = SInputEvent::KEY_RELEASE;
		//event.timestamp = rgdod[k].dwTimeStamp;
	event.timestamp = 0;
	event.moidifiers = m_modifiers;
	event.keyname = m_pInput->GetKeyName( event.key,event.moidifiers );

	// if alt+tab was pressed
	// auto-release it, because we lost the focus on the press, so we don't get the up message
	if ((event.key == XKEY_TAB) && (event.type == SInputEvent::KEY_PRESS) && (cTempKeys[DIK_LALT] & 0x80))
	{
		//m_pLog->Log("ALT TAB DETECTED!");
		m_pInput->PostInputEvent( event );

		event.type = SInputEvent::KEY_RELEASE;
		m_pInput->PostInputEvent( event );
		cTempKeys[DIK_TAB] = 0;
		cTempKeys[DIK_LALT] = 0;
	}
	else
	{
		m_pInput->PostInputEvent( event );
	}
}

//////////////////////////////////////////////////////////////////////////
void CXKeyboard::Update()
{

	if (!m_cvDirectInputKeys->GetIVal())
		return; // use windows messages

#ifndef PS2
	HRESULT hr;    	
	DIDEVICEOBJECTDATA rgdod[256];	
	DWORD dwItems = 256;

	while (m_pKeyboard)
	{
		if (!m_cvBufferedKeys->GetIVal())
		{
			memset(m_cTempKeys,0,256);
			hr = m_pKeyboard->GetDeviceState(sizeof(m_cTempKeys),m_cTempKeys);
		}
		else
		{	
			// read buffered data and keep the previous values otherwise keys 
			//hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),rgdod,&dwItems,DIGDD_PEEK); //0);
			hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),rgdod,&dwItems,0 ); //0);

			memcpy(m_cTempKeys,m_cKeysState,sizeof(m_cKeysState));
		}

		if (SUCCEEDED(hr))
		{
			if (m_cvBufferedKeys->GetIVal())
			{
				// go through all buffered items
				for (unsigned int k=0; k < dwItems; k++)
				{
					int cKey = rgdod[k].dwOfs;
					bool bPressed = ((rgdod[k].dwData & 0x80) != 0);

					ProcessKey(cKey,bPressed,m_cTempKeys);
				}
			}
						
			memcpy(m_cOldKeysState,m_cKeysState,sizeof(m_cOldKeysState));
			memcpy(m_cKeysState,m_cTempKeys,sizeof(m_cKeysState));
			
			break;
		}
		else 
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
		{			
			if (FAILED(hr = m_pKeyboard->Acquire()))					
			break;
		}
		/*
		else 
		if (hr==DI_BUFFEROVERFLOW)
		{
			m_pLog->LogToConsole("DX8 Buffer Overflow");
		}
		//else
		*/
		else
		break;
	} 		  
#else
	
	////////////////////////////////////////////////
	////PS2 SPECIFIC UPDATE CODE ///////////////////
	////////////////////////////////////////////////
	
	//Check if keyboard is connected
	if(sceUsbKbGetInfo(&keyinfo)!=USBKB_OK)
	{
		return;
	}
		
	
	//Check if there is at least one keyboard connected
	if(keyinfo.now_connect)
	{

		USBKBDATA_t keydata; 

		//Wait for getting info
		sceUsbKbSync(USBKB_WAIT,&result);
		
		//Read from first keyboard
		sceUsbKbRead(m_KeyboardNumber,&keydata);
		
		//Wait for read
		sceUsbKbSync(USBKB_WAIT,0);
		
		//Copy the old keyboard situatiom
		memcpy(ASCIPRESSEDOLD,ASCIPRESSED,sizeof(ASCIPRESSED));
		
		
		//Reset array
		memset(ASCIPRESSED,0,sizeof(ASCIPRESSED));
				
		//Read the pressed key in the array
		for(int counter=0;counter<keydata.len;counter++)
		{	
			ASCIPRESSED[keydata.keycode[counter]]=1;

		}
	}	
////////////////////////////////////////////////
////END PS2 SPECIFIC UPDATE CODE ///////////////
////////////////////////////////////////////////
		
		
			  
#endif

}

//////////////////////////////////////////////////////////////////////////
void CXKeyboard::ShutDown()
{
#ifndef PS2
	m_pLog->LogToFile("Keyboard Shutdown\n");
	UnAcquire();
	if (m_pKeyboard) m_pKeyboard->Release();
	m_pKeyboard=NULL;
#else

	//PS2 SPECIFIC CODE.
	


#endif
}

//////////////////////////////////////////////////////////////////////////
void CXKeyboard::SetKey(int p_key, int value)
{

}

//////////////////////////////////////////////////////////////////////////
void CXKeyboard::SetPrevKey(int p_key, int value)
{

}

//////////////////////////////////////////////////////////////////////////
bool CXKeyboard::KeyDown(int p_key)
{
#ifdef PS2
	if(!m_Initialized)
		return false;
#endif

	unsigned char cDik=XKEY2DIK(p_key);

#ifndef PS2
	return ((m_cKeysState[cDik] & 0x80) != 0);

#else

	if(cDik<0)
		return false;

	//PS2 SPECIFIC CODE
	if(ASCIPRESSED[cDik])
		return true;
	else
		return false;


	

#endif

}

//////////////////////////////////////////////////////////////////////////
bool CXKeyboard::KeyPressed(int p_key)
{
	unsigned char cDik=XKEY2DIK(p_key);

#ifndef PS2
	if(((m_cKeysState[cDik] & 0x80) != 0) && ((m_cOldKeysState[cDik] & 0x80) == 0))
		return true; 
	else return false;

#else


	if(cDik<0)
		return false;

	//PS2 SPECIFIC CODE
	if(ASCIPRESSED[cDik]&&!ASCIPRESSEDOLD[cDik])
		return true;
	else
		return false;

#endif


}

//////////////////////////////////////////////////////////////////////////
bool CXKeyboard::KeyReleased(int p_key)
{
	unsigned char cDik=XKEY2DIK(p_key);

#ifndef PS2
	return ((m_cKeysState[cDik] & 0x80) == 0) && ((m_cOldKeysState[cDik] & 0x80) != 0);

#else

	if(cDik<0)
		return false;

	//PS2 SPECIFIC CODE
	if(!ASCIPRESSED[cDik]&&ASCIPRESSEDOLD[cDik])
		return true;
	else
		return false;

	
#endif
}

//////////////////////////////////////////////////////////////////////////
void CXKeyboard::ClearKey(int p_key)
{
	unsigned char cDik=XKEY2DIK(p_key);
#ifndef PS2
	if (cDik<256 && cDik >=0)
	{
		m_cOldKeysState[cDik]=m_cKeysState[cDik];
		m_cKeysState[cDik]=NULL;
	}
#else
	
	//PS2 SPECIFIC CODE
	
	if(cDik<0)
		return;
		
	ASCIPRESSEDOLD[cDik]=ASCIPRESSED[cDik];
	ASCIPRESSED[cDik]=0;

#endif
}

//////////////////////////////////////////////////////////////////////////
int CXKeyboard::GetKeyPressedCode()
{
#ifndef PS2

	for (int k=0;k<256;k++) 
	{
		int nXKey=DIK2XKEY(k);

		if(nXKey==XKEY_NULL)
			continue;

		if (KeyPressed(nXKey)) 
			return nXKey;		
	}
	return -1;

#else

	for (int k=0;k<256;k++) 
	{
		int nXKey=DIK2XKEY(k);
		if (KeyPressed(nXKey)) 
			return nXKey;		
	}
	return -1;

#endif
}

//////////////////////////////////////////////////////////////////////////
const char *CXKeyboard::GetKeyPressedName()
{
	int key=GetKeyPressedCode();
	if (key==-1) 
    return (NULL);

	return m_pInput->GetKeyName(key);
}

//////////////////////////////////////////////////////////////////////////
int CXKeyboard::GetKeyDownCode()
{
#ifndef PS2
	for (int k=0;k<256;k++) 
		if (KeyDown(DIK2XKEY(k))) 
			return DIK2XKEY(k);		
	return -1;
#else


	//PS2 SPECIFIC CODE
	
	for (int k=0;k<256;k++) 
	{
		int Code=DIK2XKEY(k);
		
		if(Code==XKEY_NULL)
			continue;
		
		
		if(KeyDown(Code))
			return Code;
		
	
	}
	
	return -1;


#endif

}

//////////////////////////////////////////////////////////////////////////
const char *CXKeyboard::GetKeyDownName()
{
	int key=GetKeyDownCode();
	if (key==-1) 
    return (NULL);

	return m_pInput->GetKeyName(key);
}

//////////////////////////////////////////////////////////////////////////
void CXKeyboard::WaitForKey()
{
#ifndef PS2
	CryError( "<CryInput> CXKeyboard::WaitForKey() invalid function call" );
#endif	
}

//////////////////////////////////////////////////////////////////////////
void CXKeyboard::ClearKeyState()
{
#ifndef PS2
	memset(m_cKeysState,0,sizeof(m_cKeysState));
#else
	memset(ASCIPRESSED,0,sizeof(ASCIPRESSED));
#endif
	m_modifiers = 0;
}

unsigned char CXKeyboard::GetKeyState(int nKey)
{
	nKey = XKEY2DIK(nKey);

	if (nKey >= 0)
		return m_cKeysState[nKey];
	return 0;
}
