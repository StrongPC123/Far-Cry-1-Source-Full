// XGamepad.h: interface for the CXGamepad class.
// Sergiy Shaykin (sergiys@crytek.de)
//////////////////////////////////////////////////////////////////////

#ifndef _XGAMEPAD_H_
#define _XGAMEPAD_H_

#include <IInput.h>

struct ILog;

class CInput;

class CXGamepad : public IGamepad
{
public:
	bool Init(ILog *pLog);

	CXGamepad();
	virtual ~CXGamepad();
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
	//! return the name (key name list) of the key specified in kKeyCode
	const char *EnumerateKey(int nKeyCode);
	//! set/unset directinput to exclusive mode
	void SetExclusive(bool value,void *hwnd=0);
	//! wait for a key pressed
	void WaitForKey();
	//! clear the key (pressed) state
	void ClearKeyState();
	//! return the name (ascii name mapped to the virtual keyboard layout) of the key specified in kKeyCode
	//const char *GetKeyName(unsigned short nKey);
	//!
	void Update();

  float GetDeltaX();
  float GetDeltaY();

/////////////////////////////////////////////////////////////
private:

bool IsControllerReady(UINT iNumController) const
		{ assert(iNumController < MAX_XBOX_CONTROLLERS); return m_hGamepads[iNumController] != NULL; };

bool AnyControllersReady() const 
		{ UINT i; for (i=0; i<MAX_XBOX_CONTROLLERS; i++) if (m_hGamepads[i]) return true; return false; };

int GetPrimaryController() const
		{ UINT i; for (i=0; i<MAX_XBOX_CONTROLLERS; i++) if (m_hGamepads[i]) return i; return -1; };

SHORT Ignore(SHORT iVal) const 
{ 
  if (abs(iVal) < XBOX_ANALOGSTICK_DEADZONE)
    return 0; 
  else 
  {
    if(iVal < 0)
      return iVal + XBOX_ANALOGSTICK_DEADZONE;
    else
      return iVal - XBOX_ANALOGSTICK_DEADZONE;
  }
};


bool IsDigitalButtonPressed(const eDigitalButton eWhichButton, const bool bOnce,
									  const eController eWhichController) const;

float GetAnalogButtonValue(const eAnalogButton eWhichButton, const eController eWhichController) const;

void GetAnalogStickValue(float& fX, float& fY, const eSide eWhichStick, 
								   const eController eWhichController) const;

void ReadAnalogStickValue(const unsigned __int8 iControllerID, const eSide eWhichSide, 
		const bool bExponential, float& fX, float& fY) const;

	//static XAsciiKey m_AsciiTable[256];



XINPUT_STATE m_xbStatus[MAX_XBOX_CONTROLLERS];
XINPUT_STATE m_xbLastStatus[MAX_XBOX_CONTROLLERS];
float m_fLX, m_fLY, m_fRX, m_fRY;



public:	
	//unsigned short DIK2XKEY(unsigned char cCode);
	//unsigned char XKEY2DIK(unsigned short nCode);
	//unsigned char XKEY2ASCII(unsigned short nCode);
	
protected:
  /*
	bool Acquire();
	bool UnAcquire();
	void SetupKeyNames();
  */

  HANDLE m_hGamepads[MAX_XBOX_CONTROLLERS];

	unsigned char	m_cKeysState[256];
	unsigned char	m_cOldKeysState[256];	
	CInput *m_pInput;

  bool m_bExponentialStick;

  bool m_bExclusiveMode;
	ILog *m_pLog;
};

#endif _XGAMEPAD_H_
