
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Input.h
//
//	History:
//	-Jan 31,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef INPUT_H
#define INPUT_H

#if _MSC_VER > 1000
# pragma once
#endif

#ifndef _XBOX
#ifdef WIN32
#include <dinput.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#else
#include <xtl.h>
#endif

//////////////////////////////////////////////////////////////////////

#include <IInput.h>
#include <IConsole.h>
#include <map>
#include <string>
#include <queue>


// PC devices
#ifndef _XBOX
#include "XKeyboard.h"
#endif //_XBOX
#include "XMouse.h"
#include "Joystick.h"

// XBox devices
#ifdef _XBOX
#include "XGamepad.h"
#include "XDebugKeyboard.h"
#endif // _XBOX




/*
===========================================
The Input interface Class
Win32 code requires Directx 8
===========================================
*/
struct ILog;

typedef std::map<string,int> KeyNamesMap;
typedef KeyNamesMap::iterator KeyNamesMapItor;

typedef std::queue<int> VirtualKeyQueue;
//////////////////////////////////////////////////////////////////////
class CInput :
public IInput
{
private:
	
public:

	CInput() 
	{ 
		m_console = 0;
		m_pSystem = NULL;
		m_pLog=NULL; 
#if !defined(_XBOX) && !defined(PS2)
		m_g_pdi = NULL; 
#endif
	}
	bool	Init(ISystem *pSystem,HINSTANCE hinst, HWND hwnd, bool usedinput);

#ifdef PS2

	bool	Init(ISystem *pSystem);
	
#endif

	void	ShutDown();
	void	Update(bool bFocus);
	void	ClearKeyState();

#ifndef _XBOX
#ifndef PS2
	inline	void	SetMouseExclusive(bool exclusive,void *hwnd=0) { m_Mouse.SetExclusive(exclusive,hwnd); }
#else
	inline	void	SetMouseExclusive(bool exclusive,void *hwnd=0) { } //PS2 version....do nothing
#endif
	
	void AddEventListener( IInputEventListener *pListener );
	void RemoveEventListener( IInputEventListener *pListener );
	void EnableEventPosting ( bool bEnable );
	void PostInputEvent( const SInputEvent &event );

	virtual void AddConsoleEventListener( IInputEventListener *pListener );
	virtual void RemoveConsoleEventListener( IInputEventListener *pListener );

	void SetExclusiveListener( IInputEventListener *pListener );
	IInputEventListener *GetExclusiveListener();

	int GetModifiers() const;

	inline	void	SetKeyboardExclusive(bool exclusive,void *hwnd=0) { m_Keyboard.SetExclusive(exclusive,hwnd); }
	inline	bool	KeyDown(int p_key)		{ return (m_Keyboard.KeyDown(p_key)); }
	inline	bool	KeyPressed(int p_key)	{ return (m_Keyboard.KeyPressed(p_key)); }
	inline	bool	KeyReleased(int p_key)	{ return (m_Keyboard.KeyReleased(p_key)); }
	inline	bool	MouseDown(int p_numButton)		{ return (m_Mouse.MouseDown(p_numButton)); }
	inline	bool	MousePressed(int p_numButton)	{ return (m_Mouse.MousePressed(p_numButton)); }
	inline	bool	MouseDblClick(int p_numButton) { return (m_Mouse.MouseDblClick(p_numButton)); }
	inline	bool	MouseReleased(int p_numButton)	{ return (m_Mouse.MouseReleased(p_numButton)); }
	inline	float	MouseGetDeltaX() { return (m_Mouse.GetDeltaX()); }
	inline	float	MouseGetDeltaY() { return (m_Mouse.GetDeltaY()); }
	inline	float	MouseGetDeltaZ() { return (m_Mouse.GetDeltaZ()); }
	inline  float	MouseGetVScreenX() { return m_Mouse.GetVScreenX(); }
	inline  float	MouseGetVScreenY() { return m_Mouse.GetVScreenY(); }
	inline  void	SetMouseInertia(float kinertia) { m_Mouse.SetInertia(kinertia); }
	inline	bool	JoyButtonPressed(int p_numButton) { return (m_Joystick.IsButtonPressed(p_numButton)); }	
	inline	int		JoyGetDir()		{ return (m_Joystick.GetDir()); }	
	inline	int		JoyGetHatDir()  { return (m_Joystick.GetHatDir()); }		
	inline	Vec3	JoyGetAnalog1Dir(unsigned int joystickID) const  { return (m_Joystick.GetAnalog1Dir(joystickID)); }		
	inline	Vec3	JoyGetAnalog2Dir(unsigned int joystickID) const  { return (m_Joystick.GetAnalog2Dir(joystickID)); }		
  inline  CXKeyboard *GetKeyboard()        { return (&m_Keyboard); }	
	inline  CXMouse * GetMouse()        { return (&m_Mouse); }	
	inline	int		GetKeyPressedCode()		{ return (m_Keyboard.GetKeyPressedCode()); }
	inline	const char	*GetKeyPressedName()	{ return (m_Keyboard.GetKeyPressedName()); }
	inline	int		GetKeyDownCode()		{ return (m_Keyboard.GetKeyDownCode()); }
	inline	const char	*GetKeyDownName()		{ return (m_Keyboard.GetKeyDownName()); }
	inline	void	WaitForKey()			{ m_Keyboard.WaitForKey(); }    
	inline	unsigned char	GetKeyState(int nKey) { return m_Keyboard.GetKeyState(nKey); };
	//Interface specifics
	inline  IKeyboard *GetIKeyboard()        { return (&m_Keyboard); }	
	inline  IMouse * GetIMouse()        { return (&m_Mouse); }	
#endif // _XBOX
#ifdef _XBOX
  inline	void	SetMouseExclusive(bool exclusive,void *hwnd=0) { }
	inline	void	SetKeyboardExclusive(bool exclusive,void *hwnd=0) {  }
	inline	bool	KeyDown(int p_key)		{ return m_Keyboard.KeyDown(p_key); }
	inline	bool	KeyPressed(int p_key)	{ return m_Keyboard.KeyPressed(p_key); }
	inline	bool	KeyReleased(int p_key)	{ return m_Keyboard.KeyReleased(p_key); }
	inline	bool	MouseDown(int p_numButton)		{ return m_Gamepad.KeyDown(p_numButton); }
	inline	bool	MousePressed(int p_numButton)	{ return 0; }
	inline	bool	MouseDblClick(int p_numButton) { return 0; }
	inline	bool	MouseReleased(int p_numButton)	{ return 0; }
	inline	float	MouseGetDeltaX() { return m_Gamepad.GetDeltaX(); }
	inline	float	MouseGetDeltaY() { return m_Gamepad.GetDeltaY(); }
	inline	float	MouseGetDeltaZ() { return 0; }
	inline  int		MouseGetVScreenX() { return 0; }
	inline  int		MouseGetVScreenY() { return 0; }
	inline  void	SetMouseInertia(float kinertia) { 0; }
	inline	bool	JoyButtonPressed(int p_numButton) { return 0; }	
	inline	int		JoyGetDir()		{ return 0; }	// never called
	inline	int		JoyGetHatDir()  { return 0; } // never called
	inline	Vec3	JoyGetAnalog1Dir(unsigned int joystickID) const  { return Vec3(); }
	inline	Vec3	JoyGetAnalog2Dir(unsigned int joystickID) const  { return Vec3(); }
  //inline  CXKeyboard *GetKeyboard()        { return 0; }	//never called
  inline  CXMouse * GetMouse(){ return 0; } //never called

  inline	int		GetKeyPressedCode()		{ return (m_Keyboard.GetKeyPressedCode()); }
	inline	const char	*GetKeyPressedName()	{ return (m_Keyboard.GetKeyPressedName()); }
	inline	int		GetKeyDownCode()		{ return (m_Keyboard.GetKeyDownCode()); }
	inline	const char	*GetKeyDownName()		{ return (m_Keyboard.GetKeyDownName()); }
	inline	void	WaitForKey()			{ m_Keyboard.WaitForKey(); }    

	//Interface specifics
	inline  IKeyboard *GetIKeyboard()        { return &m_Keyboard; }
	inline  IMouse * GetIMouse()        { return 0; }	

  //! return the Xbox gamepad interface 
  inline  IGamepad * GetIGamepad() {return &m_Gamepad;}

#endif// _XBOX

	const char *GetKeyName(int nKey, int modifiers=0, bool bGUI=0);
	bool GetOSKeyName(int nKey, wchar_t *szwKeyName, int iBufSize);
	int GetKeyID(const char *sName);
	IActionMapManager* CreateActionMapManager();
	const char *GetXKeyPressedName();
	int VK2XKEY(int nKey);
	void EnableBufferedInput(bool bEnable)
	{
		m_bBufferedInput=bEnable;
		if(m_bBufferedInput==false)
		{
			while(!m_qVKQueue.empty())
			{
				m_qVKQueue.pop();
			}
		}
	}
	int GetBufferedKey()
	{
		if(!m_qVKQueue.empty())
		{
			int nRet=m_qVKQueue.front();
		}
		return -1;
	}
	const char* GetBufferedKeyName()
	{
		return GetKeyName(GetBufferedKey());
	}
	void PopBufferedKey()
	{
		if(!m_qVKQueue.empty())
			m_qVKQueue.pop();
	}
	void FeedVirtualKey(int nVirtualKey,long lParam,bool bDown)
	{
		m_Keyboard.FeedVirtualKey(nVirtualKey,lParam,bDown);
		//if(m_bBufferedInput)
		//	m_qVKQueue.push(nVirtualKey);
	}
private:
	void BroadcastEvent( const SInputEvent &event );
	
	//////////////////////////////////////////////////////////////////////////

	IConsole *m_console;
#ifndef _XBOX
	CXKeyboard	m_Keyboard;
	CXMouse		m_Mouse;
	CJoystick	m_Joystick;
#else  //_XBOX
  CXGamepad m_Gamepad;
#ifdef DEBUG_KEYBOARD
  CXDebugKeyboard m_Keyboard;
#endif //DEBUG_KEYBOARD 
#endif // _XBOX

	ILog *m_pLog;	
#if !defined(_XBOX) && !defined(PS2)
	LPDIRECTINPUT8	m_g_pdi;	
	HINSTANCE		m_hinst;
	HWND			m_hwnd;
#endif	
	struct IXBInput *m_pIInput;
	KeyNamesMap m_mapKeyNames;
  bool m_bPreviousFocus; // used to skip first mouse read after getting focus (removes random movements)
	bool m_bBufferedInput;
	VirtualKeyQueue m_qVKQueue;
	ISystem *m_pSystem;

	typedef std::vector<IInputEventListener*> Listeners;
	Listeners m_listeners;
	Listeners m_consolelisteners;
	IInputEventListener *m_exclusivelistener;

	bool			m_postingenable;
};

#endif
