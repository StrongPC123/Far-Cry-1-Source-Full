
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Keyboard.h
//
//	History:
//	-Jan 31,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef KEYBOARD_H
#define KEYBOARD_H

/*
===========================================
The Keyboard interface Class
Requires Directx 8
===========================================
*/
#include <IInput.h>
#ifdef PS2
#include <libusbkb.h>
#include <sifdev.h>
#include <sifrpc.h>
#include "Joystick.h"
#endif

struct ILog;
struct ICVar;

//struct AsciiKey
//{
//	char lc[32];
//	char uc[32];
//};
//////////////////////////////////////////////////////////////////////
class CKeyboard :
public IKeyboard
{
public:

	CKeyboard() { m_exmode=false; } 

#ifdef WIN32	
	bool	Init(ILog *pLog,LPDIRECTINPUT8 &g_pdi,HINSTANCE hinst,HWND hwnd);		
#else
	bool	Init(ILog *pLog);		
#endif	
	
	void	ShutDown();
	void	Update();

    inline  void    SetKey(int p_key,int value)
    {
			#ifdef WIN32
        m_Keys[p_key]=value;
			#endif
    }

    inline  void    SetPrevKey(int p_key,int value)
    {
			#ifdef WIN32
        m_oldKeys[p_key]=value;
			#endif
    }

	bool	KeyDown(int p_key);
	bool	KeyPressed(int p_key);
	bool	KeyReleased(int p_key);
	void ClearKey(int p_key);
	int		GetKeyPressedCode();
	const char *GetKeyPressedName();

	int		GetKeyDownCode();
	const char *GetKeyDownName();

	void	SetExclusive(bool value,void *hwnd=0);

	void	WaitForKey();

	void	ClearKeyState();
  static  void    SetupKeyNames();
  static  AsciiKey	m_AsciiTable[256];
	static  char	m_keynames[256][32];

private:	

	bool	Acquire();
	bool	UnAcquire();

#ifdef WIN32	
	LPDIRECTINPUTDEVICE8     m_g_pKeyboard;	
	HINSTANCE		m_hinst;
	HWND			m_hwnd;	
#endif	

///////////////////////////////////////////
////////PS2 SPECIFIC DATA /////////////////
///////////////////////////////////////////

#ifdef PS2

	//Keyboard's ID 
	int m_KeyboardNumber;

	unsigned char ASCIPRESSED[255];
	unsigned char ASCIPRESSEDOLD[255];
	
	//Just one key pressed a time (as the PC version) to be updated
	char m_KeyPressedName;

	//Keyboard info
	USBKBINFO_t keyinfo;

#endif

///////////////////////////////////////////
////////PS2 SPECIFIC FUNCTIONS ////////////
///////////////////////////////////////////	
	
#ifdef PS2
	public:
	
		void SetPad(CJoystick *pJoy){m_pJoy=pJoy;}
		int ConvertCode(int Code);
		
	private:
	
		//CARLONE....IF NO MOUSE....USE JOYSTICK TO SIMULATE IT
		CJoystick *m_pJoy;
			
#endif
	
///////////////////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////

	ICVar *m_cvBufferedKeys;

	unsigned char	m_Keys[256],m_oldKeys[256];	
	int				m_KeyTable[256]; 	
	bool			m_exmode;		

	char			m_tempstring[256];

	ILog *m_pLog;
};

#endif