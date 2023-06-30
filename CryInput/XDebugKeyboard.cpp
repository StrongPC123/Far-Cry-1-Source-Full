// XDebugKeyboard.cpp
//
//////////////////////////////////////////////////////////////////////

#ifdef DEBUG_KEYBOARD


#include "stdafx.h"
#include "Input.h"
#include <ILog.h>
#include <stdio.h>

#include <xkbd.h>

#include "XDebugKeyboard.h"

#include <ISystem.h>

#pragma comment (lib, "xkbdd.lib")


#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif


inline MapVirtualKey(int x, int y)
{
	return x;
}

inline ToAscii(unsigned long vKeyCode, int k, unsigned char sKState[], unsigned short ascii[], int val )
{
	return vKeyCode;
}


namespace{
  const DWORD g_dwNumToPort[] = {XDEVICE_PORT0, XDEVICE_PORT1, XDEVICE_PORT2, XDEVICE_PORT3};
}

#define NumToPort(X) g_dwNumToPort[X]


XAsciiKey CXDebugKeyboard::m_AsciiTable[256];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//XAsciiKey CXKeyboard::m_AsciiTable[256];

CXDebugKeyboard::CXDebugKeyboard()
{
  memset(m_cKeysState, 0, sizeof(m_cKeysState));
	memset(m_cOldKeysState, 0, sizeof(m_cOldKeysState));
  memset(m_bKeys, 0, sizeof(m_bKeys));
  m_hDebugKeyboard = NULL;
  m_pInput = 0;
}

CXDebugKeyboard::~CXDebugKeyboard()
{

}

bool CXDebugKeyboard::Init(CInput * pInput, ILog *pLog)
{
  char szBuffer[10240];

  m_pInput = pInput;
  m_pLog=pLog;


  m_pLog->LogToFile("Initializing Debug Keyboard\n");

/*
  XDEVICE_PREALLOC_TYPE sDeviceTypes[] =
  {
      { XDEVICE_TYPE_DEBUG_KEYBOARD, 1 },
  };

  // Initialize all four gamepads and the debug keyboard
  XInitDevices(sizeof(sDeviceTypes) / sizeof(XDEVICE_PREALLOC_TYPE), sDeviceTypes);
*/

  DWORD dwReturn;
  XINPUT_DEBUG_KEYQUEUE_PARAMETERS xdkp =
  {
    XINPUT_DEBUG_KEYQUEUE_FLAG_KEYDOWN |
    XINPUT_DEBUG_KEYQUEUE_FLAG_KEYUP,
    25, 500, 50
  };

  // Init the debug keyboard
  dwReturn = XInputDebugInitKeyboardQueue(&xdkp);
  assert(dwReturn == ERROR_SUCCESS);

  DWORD dwDeviceMask = XGetDevices( XDEVICE_TYPE_DEBUG_KEYBOARD );

  // Open the devices
  for( DWORD i=0; i < XGetPortCount(); i++ )
  {
      if( dwDeviceMask & (1<<i) ) 
      {
          // Now open the device
          XINPUT_POLLING_PARAMETERS pollValues;
          pollValues.fAutoPoll       = TRUE;
          pollValues.fInterruptOut   = TRUE;
          pollValues.bInputInterval  = 32;  
          pollValues.bOutputInterval = 32;
          pollValues.ReservedMBZ1    = 0;
          pollValues.ReservedMBZ2    = 0;

          m_hDebugKeyboard  = XInputOpen( XDEVICE_TYPE_DEBUG_KEYBOARD, i, 
                                            XDEVICE_NO_SLOT, &pollValues );
	        if (m_hDebugKeyboard != NULL)
	        {
		        sprintf(szBuffer, "INPUT: Keyboard successfully opened\n");
		        OutputDebugString(szBuffer);
	        }
	        else
	        {
		        sprintf(szBuffer, "INPUT: ERROR: Can't open keyboard \n");
		        OutputDebugString(szBuffer);
	        }
      }
  }



	//Acquire();	
  SetupKeyNames();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::SetExclusive(bool value,void *hwnd)
{ 

}



//////////////////////////////////////////////////////////////////////////
unsigned short CXDebugKeyboard::DIK2XKEY(unsigned char cCode )
{

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
      //case DIK_CIRCUMFLEX:    return 0;
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
  return XKEY_NULL;
}

//////////////////////////////////////////////////////////////////////////
unsigned char CXDebugKeyboard::XKEY2DIK(unsigned short nCode )
{
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

  	///////CONVERT ENGINE's CODE IN USB (FOR PS2) CODE (IF POSSIBLE)

/*
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
   	
  */ 	
   	return -1;
}

//////////////////////////////////////////////////////////////////////////
unsigned char CXDebugKeyboard::XKEY2ASCII(unsigned short nCode)
{
        unsigned char dik = 0;

        char startchar = 'a';
        if(KeyDown(XKEY_LSHIFT) || KeyDown(XKEY_RSHIFT))
        {
          startchar = 'A';
        }
        if(XKEY_A <= nCode && nCode <=XKEY_Z)
          dik = nCode - XKEY_A + startchar;

  if(KeyDown(XKEY_LSHIFT) || KeyDown(XKEY_RSHIFT))
  {
    switch(nCode)
    {
      case XKEY_TILDE       :dik = '¬' ;break;
      case XKEY_EQUALS      :dik = '+' ;break;
      case XKEY_LBRACKET    :dik = '{' ;break;
      case XKEY_RBRACKET    :dik = '}' ;break;
      case XKEY_BACKSLASH   :dik = '|';break;
      case XKEY_SEMICOLON   :dik = ':' ;break;
      case XKEY_APOSTROPHE  :dik = '"';break;
      case XKEY_COMMA       :dik = '<' ;break;
      case XKEY_ADD         :dik = '+' ;break;
      case XKEY_SUBTRACT    :dik = '-' ;break;
      case XKEY_MINUS       :dik = '_' ;break;
      case XKEY_DECIMAL     :dik = '.' ;break;
      case XKEY_DIVIDE      :dik = '?' ;break;
      case XKEY_SLASH  :dik = '?' ;break;
      case XKEY_BACKSPACE   :dik = 9   ;break;
      case XKEY_RETURN      :dik = 13  ;break;

      case XKEY_0 :dik = ')'       ;break;
      case XKEY_1 :dik = '!'       ;break;
      case XKEY_2 :dik = '@'       ;break;
      case XKEY_3 :dik = '#'       ;break;
      case XKEY_4 :dik = '$'       ;break;
      case XKEY_5 :dik = '%'       ;break;
      case XKEY_6 :dik = '^'       ;break;
      case XKEY_7 :dik = '&'       ;break;
      case XKEY_8 :dik = '*'       ;break;
      case XKEY_9 :dik = '('       ;break;

    }
  }
  else
  {
    if(XKEY_0 <= nCode && nCode <=XKEY_9)
      dik = nCode - XKEY_0 + '0';

    switch(nCode)
    {
      case XKEY_TILDE       :dik = '`' ;break;
      case XKEY_EQUALS      :dik = '=' ;break;
      case XKEY_LBRACKET    :dik = '[' ;break;
      case XKEY_RBRACKET    :dik = ']' ;break;
      case XKEY_BACKSLASH   :dik = '\\';break;
      case XKEY_SEMICOLON   :dik = ';' ;break;
      case XKEY_APOSTROPHE  :dik = '\'';break;
      case XKEY_COMMA       :dik = ',' ;break;
      case XKEY_ADD         :dik = '+' ;break;
      case XKEY_SUBTRACT    :dik = '-' ;break;
      case XKEY_MINUS       :dik = '-' ;break;
      case XKEY_DECIMAL     :dik = '.' ;break;
      case XKEY_DIVIDE      :dik = '/' ;break;
      case XKEY_SLASH  :dik = '/' ;break;
      case XKEY_BACKSPACE   :dik = 9   ;break;
      case XKEY_RETURN      :dik = 13  ;break;
    }
  }
  if(dik)
  {
    return dik;
  }

	if (KeyDown(XKEY_LSHIFT) || KeyDown(XKEY_RSHIFT))
	{
		return m_AsciiTable[XKEY2DIK(nCode)].uc[0];
	}

	return m_AsciiTable[XKEY2DIK(nCode)].lc[0];
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::SetupKeyNames()
{
  ///////////////////////////////////////////////////////////
	for (int k=0;k<256;k++) 
	{
    memset(m_AsciiTable[k].lc,0,sizeof(m_AsciiTable[k].lc));
		memset(m_AsciiTable[k].uc,0,sizeof(m_AsciiTable[k].uc));
	}
	
	unsigned char sKState[256];
	unsigned short ascii[2];
	int nResult;
	unsigned int vKeyCode;
	for (k=0;k<256;k++)
	{
		memset(sKState,0,sizeof(sKState));
		vKeyCode = MapVirtualKey( k, 1 );
		
		ascii[0] = ascii[1] = 0;
		nResult=ToAscii( vKeyCode, k, sKState, ascii, 0 );
		
		
		
		if ( nResult == 2 )
			m_AsciiTable[k].lc[0] =(char) ascii[1] ? ascii[1] : ( ascii[0] >> 8 );
    else if ( nResult == 1 )
			m_AsciiTable[k].lc[0] =(char) ascii[0];
    //::OutputDebugString(m_AsciiTable[k].lc); ::OutputDebugString("\n");
		
		ascii[0] = ascii[1] = 0;
    sKState[VK_SHIFT] = 0x80;
		
		nResult=ToAscii( vKeyCode, k, sKState, ascii, 0 );
		
		if ( nResult == 2 )
			m_AsciiTable[k].uc[0] =(char) ascii[1] ? ascii[1] : ( ascii[0] >> 8 );
    else if ( nResult == 1 )
			m_AsciiTable[k].uc[0] =(char) ascii[0];
    //::OutputDebugString(m_AsciiTable[k].uc); ::OutputDebugString("\n");
	}	
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::Update()
{
  //BOOL bReturn;
  DWORD dwInsertions = 0, dwRemovals = 0;
	unsigned int i;
  char szBuffer[10240];
  DWORD dwReturn;

	// Query the debug keyboard

	XINPUT_DEBUG_KEYSTROKE sKeyStroke;

	// Query changes
	XGetDeviceChanges(XDEVICE_TYPE_DEBUG_KEYBOARD, &dwInsertions, &dwRemovals);

	// Detect keyboard insertions
	for (i=0; i<4; i++)
	{
		if (dwInsertions & (1 << i))
		{
			// A controller was inserted
			sprintf(szBuffer, "INPUT: Keyboard %i inserted\n", i);
			OutputDebugString(szBuffer);

			// Open the keyboard on the port
      XINPUT_POLLING_PARAMETERS pollValues;
      pollValues.fAutoPoll       = TRUE;
      pollValues.fInterruptOut   = TRUE;
      pollValues.bInputInterval  = 32;  
      pollValues.bOutputInterval = 32;
      pollValues.ReservedMBZ1    = 0;
      pollValues.ReservedMBZ2    = 0;

			m_hDebugKeyboard = XInputOpen(XDEVICE_TYPE_DEBUG_KEYBOARD, NumToPort(i), XDEVICE_NO_SLOT, &pollValues);

			if (m_hDebugKeyboard != NULL)
			{
				sprintf(szBuffer, "INPUT: Keyboard %i successfully opened\n", i);
				OutputDebugString(szBuffer);
			}
			else
			{
				sprintf(szBuffer, "INPUT: ERROR: Can't open keyboard %i\n", i);
				OutputDebugString(szBuffer);
			}
		}
	}

	// Detect keyboard removal
	for (i=0; i<4; i++)
	{
		if (dwRemovals & (1 << i))
		{
			// A keyboard was removed
			sprintf(szBuffer, "INPUT: Keyboard %i removed\n", i);
			OutputDebugString(szBuffer);

			// Close the controller
			XInputClose(m_hDebugKeyboard);
			m_hDebugKeyboard = NULL;
		}
	}

	// Poll for input when we got a keyboard connected and initializd
	
  if (m_hDebugKeyboard)
	{
    memcpy(m_cOldKeysState,m_cKeysState,sizeof(m_cOldKeysState));
     
    // Query the keyboard state
		dwReturn = XInputDebugGetKeystroke(&sKeyStroke);

		// Error ?
		if (dwReturn == ERROR_SUCCESS)
		{

			// We got a key up or key down event ?

      DWORD dik =-1;

      switch(sKeyStroke.VirtualKey)
      {

        case VK_TAB         :dik = XKEY_TAB         ;break;
        case VK_BACK        :dik = XKEY_BACKSPACE   ;break;
        case VK_RETURN      :dik = XKEY_RETURN      ;break;
        case VK_SHIFT       :dik = XKEY_SHIFT       ;break;
        case VK_CONTROL     :dik = XKEY_CONTROL     ;break;
        case VK_PAUSE       :dik = XKEY_PAUSE       ;break;


        case VK_ESCAPE      :dik = XKEY_ESCAPE      ;break;

        case VK_SPACE       :dik = XKEY_SPACE       ;break;


        case VK_END         :dik = XKEY_END         ;break;
        case VK_HOME        :dik = XKEY_HOME        ;break;
        case VK_LEFT        :dik = XKEY_LEFT        ;break;
        case VK_UP          :dik = XKEY_UP          ;break;
        case VK_RIGHT       :dik = XKEY_RIGHT       ;break;
        case VK_DOWN        :dik = XKEY_DOWN        ;break;

        case VK_PRINT       :dik = XKEY_PRINT       ;break;


        case VK_INSERT      :dik = XKEY_INSERT      ;break;
        case VK_DELETE      :dik = XKEY_DELETE      ;break;
        case VK_HELP        :dik = XKEY_HELP        ;break;

        //case  VK_LALT       :dik = XKEY_LALT        ;break;
        //case  VK_RALT       :dik = XKEY_RALT        ;break;

        case  VK_LSHIFT     :dik = XKEY_LSHIFT      ;break;
        case  VK_RSHIFT     :dik = XKEY_RSHIFT      ;break;
        case  VK_LCONTROL   :dik = XKEY_LCONTROL    ;break;
        case  VK_RCONTROL   :dik = XKEY_RCONTROL    ;break;


        case VK_NUMPAD0    :  dik = XKEY_NUMPAD0    ;break;
        case VK_NUMPAD1    :  dik = XKEY_NUMPAD1    ;break;
        case VK_NUMPAD2    :  dik = XKEY_NUMPAD2    ;break;
        case VK_NUMPAD3    :  dik = XKEY_NUMPAD3    ;break;
        case VK_NUMPAD4    :  dik = XKEY_NUMPAD4    ;break;
        case VK_NUMPAD5    :  dik = XKEY_NUMPAD5    ;break;
        case VK_NUMPAD6    :  dik = XKEY_NUMPAD6    ;break;
        case VK_NUMPAD7    :  dik = XKEY_NUMPAD7    ;break;
        case VK_NUMPAD8    :  dik = XKEY_NUMPAD8    ;break;
        case VK_NUMPAD9    :  dik = XKEY_NUMPAD9    ;break;
        case VK_MULTIPLY   :  dik = XKEY_MULTIPLY   ;break;
        case VK_ADD        :  dik = XKEY_ADD        ;break;
        case VK_SEPARATOR  :  dik = XKEY_SEPARATOR  ;break;
        case VK_SUBTRACT   :  dik = XKEY_SUBTRACT   ;break;
        case VK_DECIMAL    :  dik = XKEY_DECIMAL    ;break;
        case VK_DIVIDE     :  dik = XKEY_DIVIDE     ;break;
        case VK_F1         :  dik = XKEY_F1         ;break;
        case VK_F2         :  dik = XKEY_F2         ;break;
        case VK_F3         :  dik = XKEY_F3         ;break;
        case VK_F4         :  dik = XKEY_F4         ;break;
        case VK_F5         :  dik = XKEY_F5         ;break;
        case VK_F6         :  dik = XKEY_F6         ;break;
        case VK_F7         :  dik = XKEY_F7         ;break;
        case VK_F8         :  dik = XKEY_F8         ;break;
        case VK_F9         :  dik = XKEY_F9         ;break;
        case VK_F10        :  dik = XKEY_F10        ;break;
        case VK_F11        :  dik = XKEY_F11        ;break;
        case VK_F12        :  dik = XKEY_F12        ;break;
        case VK_F13        :  dik = XKEY_F13        ;break;
        case VK_F14        :  dik = XKEY_F14        ;break;
        case VK_F15        :  dik = XKEY_F15        ;break;
        case VK_F16        :  dik = XKEY_F16        ;break;
        case VK_F17        :  dik = XKEY_F17        ;break;
        case VK_F18        :  dik = XKEY_F18        ;break;
        case VK_F19        :  dik = XKEY_F19        ;break;
        case VK_F20        :  dik = XKEY_F20        ;break;
        case VK_F21        :  dik = XKEY_F21        ;break;
        case VK_F22        :  dik = XKEY_F22        ;break;
        case VK_F23        :  dik = XKEY_F23        ;break;
        case VK_F24        :  dik = XKEY_F24        ;break;
      }
      
      if(dik != -1)
        m_cKeysState[dik] = (sKeyStroke.Flags & XINPUT_DEBUG_KEYSTROKE_FLAG_KEYUP)? false: true;

      dik = 0;

      if('a' <= sKeyStroke.Ascii && sKeyStroke.Ascii <='z')
        dik = XKEY_A + sKeyStroke.Ascii - 'a';

      if('A' <= sKeyStroke.Ascii && sKeyStroke.Ascii <='Z')
        dik = XKEY_A + sKeyStroke.Ascii - 'A';

      if('0' <= sKeyStroke.Ascii && sKeyStroke.Ascii <='9')
        dik = XKEY_0 + sKeyStroke.Ascii - '0';
      switch(sKeyStroke.Ascii)
      {
        case '`' :dik = XKEY_TILDE       ;break;
        case '=' :dik = XKEY_EQUALS      ;break;
        case '[' :dik = XKEY_LBRACKET    ;break;
        case ']' :dik = XKEY_RBRACKET    ;break;
        case '\\' :dik = XKEY_BACKSLASH  ;break;
        case ';' :dik = XKEY_SEMICOLON   ;break;
        case '\'' :dik = XKEY_APOSTROPHE ;break;
        case ',' :dik = XKEY_COMMA       ;break;
        case '-' :dik = XKEY_MINUS       ;break;
        case '.' :dik = XKEY_DECIMAL     ;break;
        case '/' :dik = XKEY_SLASH       ;break;

        case '¬' :dik = XKEY_TILDE       ;break;
        case '+' :dik = XKEY_EQUALS      ;break;
        case '{' :dik = XKEY_LBRACKET    ;break;
        case '}' :dik = XKEY_RBRACKET    ;break;
        case '|' :dik = XKEY_BACKSLASH  ;break;
        case ':' :dik = XKEY_SEMICOLON   ;break;
        case '"' :dik = XKEY_APOSTROPHE ;break;
        case '<' :dik = XKEY_COMMA       ;break;
        case '_' :dik = XKEY_MINUS       ;break;
        case '>' :dik = XKEY_DECIMAL     ;break;
        case '?' :dik = XKEY_SLASH       ;break;

        case ')' :dik = XKEY_0       ;break;
        case '!' :dik = XKEY_1       ;break;
        case '@' :dik = XKEY_2       ;break;
        case '#' :dik = XKEY_3       ;break;
        case '$' :dik = XKEY_4       ;break;
        case '%' :dik = XKEY_5       ;break;
        case '^' :dik = XKEY_6       ;break;
        case '&' :dik = XKEY_7       ;break;
        case '*' :dik = XKEY_8       ;break;
        case '(' :dik = XKEY_9       ;break;
      }
      if(dik!=0)
        m_cKeysState[dik] = (sKeyStroke.Flags & XINPUT_DEBUG_KEYSTROKE_FLAG_KEYUP)? false: true;
		}
		else
		{
			// We got no success, and the keyboard buffer is not empty ?
			if (dwReturn != ERROR_HANDLE_EOF)
			  OutputDebugString("INPUT: Error while reading from debug keyboard");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::ShutDown()
{
	if (m_hDebugKeyboard)
	{
		XInputClose(m_hDebugKeyboard);
		m_hDebugKeyboard = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::SetKey(int p_key, int value)
{
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::SetPrevKey(int p_key, int value)
{
}

//////////////////////////////////////////////////////////////////////////
bool CXDebugKeyboard::KeyDown(int p_key)
{
	return ((m_cKeysState[p_key]) != 0);
}

//////////////////////////////////////////////////////////////////////////
bool CXDebugKeyboard::KeyPressed(int p_key)
{
  if(p_key == XKEY_MINUS)
  {
    if(m_cKeysState[p_key]!=0)
    {
      int ga=1;
    }

    int gamedev=1;
  }

  return ((m_cKeysState[p_key]) != 0 && m_cOldKeysState[p_key] == 0);
  /*
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

*/
  return false;
}

//////////////////////////////////////////////////////////////////////////
bool CXDebugKeyboard::KeyReleased(int p_key)
{
  return ((m_cKeysState[p_key] ) == 0) && ((m_cOldKeysState[p_key] ) != 0);
  /*
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
  */
  return false;
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::ClearKey(int p_key)
{
  m_cOldKeysState[p_key]=m_cKeysState[p_key];
  m_cKeysState[p_key] = 0;
  /*
	unsigned char cDik=XKEY2DIK(p_key);
#ifndef PS2
	m_cOldKeysState[cDik]=m_cKeysState[cDik];
	m_cKeysState[cDik]=NULL;
#else
	
	//PS2 SPECIFIC CODE
	
	if(cDik<0)
		return;
		
	ASCIPRESSEDOLD[cDik]=ASCIPRESSED[cDik];
	ASCIPRESSED[cDik]=0;

#endif
  */
 
}

//////////////////////////////////////////////////////////////////////////
int CXDebugKeyboard::GetKeyPressedCode()
{
	for (int k=0;k<256;k++) 
	{
		int nXKey=DIK2XKEY(k);

		if(nXKey==XKEY_NULL)
			continue;

		if (KeyPressed(nXKey)) 
			return nXKey;		
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
const char *CXDebugKeyboard::GetKeyPressedName()
{
	int key=GetKeyPressedCode();
	if (key==-1) 
    return (NULL);

	return m_pInput->GetKeyName(key);
}

//////////////////////////////////////////////////////////////////////////
int CXDebugKeyboard::GetKeyDownCode()
{
	for (int k=0;k<256;k++) 
		if (KeyDown(DIK2XKEY(k))) 
			return DIK2XKEY(k);		
	return -1;
}

//////////////////////////////////////////////////////////////////////////
const char *CXDebugKeyboard::GetKeyDownName()
{
	int key=GetKeyDownCode();
	if (key==-1) 
    return (NULL);

	return m_pInput->GetKeyName(key);

}

//////////////////////////////////////////////////////////////////////////
const char *CXDebugKeyboard::EnumerateKey(int nKeyCode)
{
	return m_pInput->GetKeyName(nKeyCode);
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::WaitForKey()
{
	CryError( "<CryInput> CXKeyboard::WaitForKey() invalid function call" );
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::ClearKeyState()
{
	memset(m_cKeysState,0,sizeof(m_cKeysState));
}


#endif // DEBUG_KEYBOARD