// XKeyboard.h: interface for the CXKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XKEYBOARD_H__9C0FD463_ECC8_42DA_8019_378651B00771__INCLUDED_)
#define AFX_XKEYBOARD_H__9C0FD463_ECC8_42DA_8019_378651B00771__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <IInput.h>

#ifdef PS2

#include "Joystick.h"
#include <libusbkb.h>

#endif


struct ILog;
struct ICVar;

struct XAsciiKey
{
	char		lc[4];	// lowercase
	char		uc[4];	// uppercase
	char		ac[4];	// alt gr
	char		cl[4];	// caps lock (differs slightly from uppercase)
};

#define		TOGGLE_CAPSLOCK			(1 << 0)
#define		TOGGLE_NUMLOCK			(1 << 1)
#define		TOGGLE_SCROLLLOCK		(1 << 2)


#define KEYFLAG_BUFFERSIZE 32
class CInput;


class CXKeyboard : 
public IKeyboard  
{
public:
#ifndef PS2
	//bool Init(CInput *,ILog *pLog, LPDIRECTINPUT8 &g_pdi,HINSTANCE hinst,HWND hwnd3);
	bool Init(CInput *,ISystem *pSystem, LPDIRECTINPUT8 &g_pdi,HINSTANCE hinst,HWND hwnd3);
#else
	bool Init(ILog *pLog);
#endif	

	CXKeyboard();
	virtual ~CXKeyboard();
////////////////////////////////////////////////////////////
	void ShutDown();
	//! allow to force a key code value
  void SetKey(int p_key, int value);
	//! allow to force a key code value
  void SetPrevKey(int p_key, int value);
	//! check for key pressed and held
	bool KeyDown(int p_key);
	//! check for key pressed only once
	bool KeyPressed(int p_key);
	//! check if the key has been released
	bool KeyReleased(int p_key);
	//! clear the key status
	void ClearKey(int p_key);
	//! return the code of the key pressed
	int GetKeyPressedCode();
	//! return the name of the key pressed 
	const char *GetKeyPressedName();
	//! return the code of the key down
	int GetKeyDownCode();
	//! return the name of the key down
	const char *GetKeyDownName();
	//! set/unset directinput to exclusive mode
	void SetExclusive(bool value,void *hwnd=0);
	//! wait for a key pressed
	void WaitForKey();
	//! clear the key (pressed) state
	void ClearKeyState();
	//! get the key state
	unsigned char GetKeyState(int nKey);
	//! return the name (ascii name mapped to the virtual keyboard layout) of the key specified in kKeyCode
	//!
	void Update();
/////////////////////////////////////////////////////////////
	int GetModifiers() const { return m_modifiers; };

private:
	static XAsciiKey m_AsciiTable[256];
public:	
	unsigned short DIK2XKEY(unsigned char cCode);
	unsigned char XKEY2DIK(unsigned short nCode);
	unsigned char XKEY2ASCII(unsigned short nCode,int modifiers);
	bool GetOSKeyName(int nKey, wchar_t *szwKeyName, int iBufSize);

	void FeedVirtualKey(int nVirtualKey,long lParam,bool bDown);
	
protected:
	bool	Acquire();
	bool	UnAcquire();
	void	SetupKeyNames();
	void	ProcessKey(int nKey,bool bPressed,unsigned char *cTempKeys);
	unsigned char	m_cKeysState[256];
	unsigned char	m_cOldKeysState[256];	
	unsigned char m_cTempKeys[256];	
	CInput *m_pInput;
#ifdef WIN32	
	LPDIRECTINPUTDEVICE8 m_pKeyboard;	
	HINSTANCE m_hinst;
	HWND	m_hwnd;
	int		m_iToggleState;
#endif	


	
	
	
///////////////////////////////////////////
////////PS2 SPECIFIC DATA /////////////////
///////////////////////////////////////////

#ifdef PS2

	///??
	void	*m_pKeyboard;
	
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
///////////////////////////////////////////
///////////////////////////////////////////

	bool m_bExclusiveMode;
	ISystem	*m_pSystem;
	ILog		*m_pLog;

	ICVar *m_cvBufferedKeys;
	ICVar *m_cvDirectInputKeys;
	int m_modifiers;

#ifdef PS2
public:
	bool	m_Initialized;

	bool IsInit(){return m_Initialized;}
	void SetPad(CJoystick *pJoy){m_pJoy=pJoy;}
private:
	CJoystick *m_pJoy;
	
#endif	

};

#endif // !defined(AFX_XKEYBOARD_H__9C0FD463_ECC8_42DA_8019_378651B00771__INCLUDED_)
