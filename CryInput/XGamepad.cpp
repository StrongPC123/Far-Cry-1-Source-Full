#include "stdafx.h"
#include "Input.h"
#include <ILog.h>

#include <Xtl.h>

#include "XGamepad.h"

namespace{
  const DWORD g_dwNumToPort[] = {XDEVICE_PORT0, XDEVICE_PORT1, XDEVICE_PORT2, XDEVICE_PORT3};
}

#define NumToPort(X) g_dwNumToPort[X]


CXGamepad::CXGamepad()
{
  memset(m_hGamepads, NULL, sizeof(HANDLE) * MAX_XBOX_CONTROLLERS);
  memset(m_xbStatus, NULL, sizeof(XINPUT_STATE) * MAX_XBOX_CONTROLLERS);
  memset(m_xbLastStatus, NULL, sizeof(XINPUT_STATE) * MAX_XBOX_CONTROLLERS);
  //memset(m_bKeys, false, sizeof(bool) * 256);
  //m_hDebugKeyboard = NULL;
  m_bExponentialStick = true;
  m_fLX = m_fLY = m_fRX = m_fRY = 0.0f;
}


CXGamepad::~CXGamepad()
{
  ShutDown();
}


bool CXGamepad::Init(ILog *pLog)
{
  XDEVICE_PREALLOC_TYPE sDeviceTypes[] =
  {
      { XDEVICE_TYPE_GAMEPAD, MAX_XBOX_CONTROLLERS },
#ifdef DEBUG_KEYBOARD
      { XDEVICE_TYPE_DEBUG_KEYBOARD, 1 },
#endif //DEBUG_KEYBOARD
  };

  // Initialize all four gamepads and the debug keyboard
  XInitDevices(sizeof(sDeviceTypes) / sizeof(XDEVICE_PREALLOC_TYPE), sDeviceTypes);


  return true;
}


void CXGamepad::ShutDown()
{
  unsigned int i;

  // Close all opened gamepad handles
  for (i=0; i<sizeof(m_hGamepads) / sizeof(HANDLE); i++)
  {
    if (m_hGamepads[i])
      XInputClose(m_hGamepads[i]);
  }
}



void CXGamepad::Update()
{
	BOOL bReturn;
	DWORD dwInsertions = 0, dwRemovals = 0;
	unsigned int i;
#ifdef _DEBUG
	char szBuffer[10024 /*256*/];
#endif
	DWORD dwReturn;

	// Query controller changes
	bReturn = XGetDeviceChanges(XDEVICE_TYPE_GAMEPAD, &dwInsertions, &dwRemovals);

	// Detect controler insertion
	for (i=0; i<MAX_XBOX_CONTROLLERS; i++)
	{
		if (dwInsertions & (1 << i))
		{
			// A controller was inserted
#ifdef _DEBUG
			sprintf(szBuffer, "INPUT: Controller %i inserted\n", i);
			OutputDebugString(szBuffer);
#endif

			// Open the device on the port
			m_hGamepads[i] = XInputOpen(XDEVICE_TYPE_GAMEPAD, NumToPort(i), XDEVICE_NO_SLOT, NULL);

#ifdef _DEBUG
			if (m_hGamepads[i] != NULL)
			{
				sprintf(szBuffer, "INPUT: Controller %i successfully opened\n", i);
				OutputDebugString(szBuffer);
			}
			else
			{
				sprintf(szBuffer, "INPUT: ERROR: Can't open controller %i\n", i);
				OutputDebugString(szBuffer);
			}
#endif
		}
	}

	// Detect controller removal
	for (i=0; i<MAX_XBOX_CONTROLLERS; i++)
	{
		if (dwRemovals & (1 << i))
		{
			// A controller was removed
#ifdef _DEBUG
			sprintf(szBuffer, "INPUT: Controller %i removed\n", i);
			OutputDebugString(szBuffer);
#endif
			// Close the controller
			XInputClose(m_hGamepads[i]);
			m_hGamepads[i] = NULL;
		}
	}

	// Retrieve the state
	for (i=0; i<MAX_XBOX_CONTROLLERS; i++)
	{
		if (IsControllerReady(i))
		{
			// Save the old state
			m_xbLastStatus[i] = m_xbStatus[i];

			// Get the new one
			dwReturn = XInputGetState(m_hGamepads[i], &m_xbStatus[i]);

#ifdef _DEBUG
			if (dwReturn == ERROR_DEVICE_NOT_CONNECTED)
			{
				sprintf(szBuffer, "INPUT: Can't get state of controller %i because it was" \
					" removed and the removal has not been handled yet\n", i);
				OutputDebugString(szBuffer);
			}
			else if (dwReturn != ERROR_SUCCESS)
			{
				sprintf(szBuffer, "INPUT: ERROR: Can't get state of controller %i" \
					", error %i\n", i, dwReturn);
				OutputDebugString(szBuffer);
			}

			/*
			if (m_xbLastStatus[i].dwPacketNumber != m_xbStatus[i].dwPacketNumber)
			{
				//ASH: get rid of this.
				sprintf(szBuffer, "packet: %d\n"
					"got input: controller - %d\n"
					"up: %d, down: %d, left: %d, right: %d\n"
					"start: %d, back: %d, left thumb: %d, right thumb: %d\n"
					"a: %d, b: %d, x: %d, y: %d, black: %d, white: %d\n"
					"left trigger: %d, right trigger: %d\n"
					"LX: %d, LY: %d, RX: %d, RY: %d\n",
					(int) (m_xbStatus[i].dwPacketNumber),
					i,
					(int) (m_xbStatus[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP),
					(int) (m_xbStatus[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN),
					(int) (m_xbStatus[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT),
					(int) (m_xbStatus[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT),
					(int) (m_xbStatus[i].Gamepad.wButtons & XINPUT_GAMEPAD_START),
					(int) (m_xbStatus[i].Gamepad.wButtons & XINPUT_GAMEPAD_BACK),
					(int) (m_xbStatus[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB),
					(int) (m_xbStatus[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB),
					(int) (m_xbStatus[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_A]),
					(int) (m_xbStatus[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_B]),
					(int) (m_xbStatus[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_X]),
					(int) (m_xbStatus[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_Y]),
					(int) (m_xbStatus[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_BLACK]),
					(int) (m_xbStatus[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_WHITE]),
					(int) (m_xbStatus[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER]),
					(int) (m_xbStatus[i].Gamepad.bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER]),
					(int) (m_xbStatus[i].Gamepad.sThumbLX), 
					(int) (m_xbStatus[i].Gamepad.sThumbLY),
					(int) (m_xbStatus[i].Gamepad.sThumbRX), 
					(int) (m_xbStatus[i].Gamepad.sThumbRY));
				OutputDebugString(szBuffer);
			}
			*/


#endif
		}
	}
}



bool CXGamepad::IsDigitalButtonPressed(const eDigitalButton eWhichButton, const bool bOnce,
									  const eController eWhichController) const
{
	//////////////////////////////////////////////////////////////////////
	// Returns the status of a digital button
	//////////////////////////////////////////////////////////////////////

	unsigned int i, iLastController = eWhichController;
	bool bPressed = false;

	if (!AnyControllersReady())
		return false;

	if (eWhichButton == NULL)
		return false;

	// Should we check all controllers ?
	if (eWhichController == eAll)
	{
		for (i=0; i<MAX_XBOX_CONTROLLERS; i++)
		{
			if (IsControllerReady(i))
			{
				// Button eWhichButton pressed on controller i ?
				if (m_xbStatus[i].Gamepad.wButtons & eWhichButton)
				{
					// Button pressed, save the current controller as the last one
					iLastController = i;
					bPressed = true;
					break;
				}
			}
		}
	}

	// Should we check the primary controller ?
	if (eWhichController == ePrimary)
	{
		// Query the key state on the primary controller and prepare to check
		// if the key was already pressed during the last update
		iLastController = GetPrimaryController();
		bPressed = (m_xbStatus[GetPrimaryController()].Gamepad.wButtons & eWhichButton) != NULL;
	}
	else if (eWhichController != eAll)
	{
		// Check the specified controller
		if (IsControllerReady(eWhichController))
			bPressed = (m_xbStatus[eWhichController].Gamepad.wButtons & eWhichButton) != NULL;
		else
			// Specified controller not ready
			bPressed = false;
	}
	
	// We can just return if we are not in single trigger mode or the value is false
	if (bPressed == false || bOnce == false)
		return bPressed;

	if (bOnce)	
	{
		// Key is pressed and we are in single trigger mode
		if ((m_xbLastStatus[iLastController].Gamepad.wButtons & eWhichButton) != NULL)
			// Button press already processed
			bPressed = false;
	}

	return bPressed;
}



float CXGamepad::GetAnalogButtonValue(const eAnalogButton eWhichButton, 
									 const eController eWhichController) const
{
	//////////////////////////////////////////////////////////////////////
	// Returns the status of an analog button as a value between 0 and 1
	//////////////////////////////////////////////////////////////////////

	unsigned int i;
	float fAccumValue = 0.0f;

	if (!AnyControllersReady())
		return 0.0f;

	// Should we check all controllers ?
	if (eWhichController == eAll)
	{
		for (i=0; i<MAX_XBOX_CONTROLLERS; i++)
		{
			// Add the value of the button if the controller is ready
			if (IsControllerReady(i))
				fAccumValue += (float) m_xbStatus[i].Gamepad.bAnalogButtons[eWhichButton] / 255.0f;
		}

		// Return clamped value
		return __min(fAccumValue, 1.0f);
	}

	// Should we check the primary controller ?
	if (eWhichController == ePrimary)
		return m_xbStatus[GetPrimaryController()].Gamepad.bAnalogButtons[eWhichButton] / 255.0f;

	// Check the specified controller
	if (IsControllerReady(eWhichController))
		return m_xbStatus[eWhichController].Gamepad.bAnalogButtons[eWhichButton] / 255.0f;
	else
		// Specified controller not ready
		return 0.0f;
}



void CXGamepad::GetAnalogStickValue(float& fX, float& fY, const eSide eWhichStick, 
								   const eController eWhichController) const
{
	//////////////////////////////////////////////////////////////////////
	// Check the axis of an analog stick, value is between -1 and +1
	//////////////////////////////////////////////////////////////////////

	unsigned int i;
	float fAccumValueX = 0.0f;
	float fAccumValueY = 0.0f;
	//float fValX, fValY;

	if (!AnyControllersReady())
	{
		fX = 0.0f;
		fY = 0.0f;
		return;
	}

	// Should we check all controllers ?
	if (eWhichController == eAll)
	{
		for (i=0; i<MAX_XBOX_CONTROLLERS; i++)
		{
			// Add the value of the button if the controller is ready
			if (IsControllerReady(i))
			{
				if (eWhichStick == eLeft)
				{
					// Left stick
					fAccumValueX += Ignore(m_xbStatus[i].Gamepad.sThumbLX) / 32768.0f;
					fAccumValueY += Ignore(m_xbStatus[i].Gamepad.sThumbLY) / 32768.0f;
				}
				else if (eWhichStick == eRight)
				{
					// Right stick
					fAccumValueX += Ignore(m_xbStatus[i].Gamepad.sThumbRX) / 32768.0f;
					fAccumValueY += Ignore(m_xbStatus[i].Gamepad.sThumbRY) / 32768.0f;
				}
			}
		}

		// Clamp
		if (fAccumValueX < -1.0f)
			fAccumValueX = -1.0;
		if (fAccumValueX > 1.0f)
			fAccumValueX = 1.0;
		if (fAccumValueY < -1.0f)
			fAccumValueY = -1.0;
		if (fAccumValueY > 1.0f)
			fAccumValueY = 1.0;

		// Return clamped value
		fX = fAccumValueX;
		fY = fAccumValueY;
		return;
	}

	// Should we check the primary controller ?
	if (eWhichController == ePrimary)
	{
		// Read out the controller
		ReadAnalogStickValue(GetPrimaryController(), eWhichStick, 
			m_bExponentialStick, fX, fY);
		return;
	}
		
	// Check the specified controller
	if (IsControllerReady(eWhichController))
	{
		if (eWhichStick == eLeft)
		{
			// Left stick
			fX = Ignore(m_xbStatus[eWhichController].Gamepad.sThumbLX) / 32768.0f;
			fY = Ignore(m_xbStatus[eWhichController].Gamepad.sThumbLY) / 32768.0f;
		}
		else if (eWhichStick == eRight)
		{
			// Right stick
			fX = Ignore(m_xbStatus[eWhichController].Gamepad.sThumbRX) / 32768.0f;
			fY = Ignore(m_xbStatus[eWhichController].Gamepad.sThumbRY) / 32768.0f;
		}
		
		return;
	}
	else
	{
		// Specified controller not ready
		fX = 0.0f;
		fY = 0.0f;
	}
}


void CXGamepad::ReadAnalogStickValue(const unsigned __int8 iControllerID, 
									const eSide eWhichSide, 
									const bool bExponential,
									float& fX, float& fY) const
{
	//////////////////////////////////////////////////////////////////////
	// Read the axis values of one of the analog sticks. Perform linear
	// or exponential attenuation, take deadzone into account
	//////////////////////////////////////////////////////////////////////

	float fValX, fValY;

	// Abort when controller is not inserted / opened
	if (iControllerID + 1 >= MAX_XBOX_CONTROLLERS || IsControllerReady(iControllerID) == false)
	{
		fX = 0.0f;
		fY = 0.0f;
		return;
	}

	if (bExponential)
	{
		// Exponential change of intensity

		if (eWhichSide == eRight)
		{
			// Read values from the right stick and scale to 0 - 50
			fValX = Ignore(m_xbStatus[iControllerID].Gamepad.sThumbRX) / 32768.0f * 50.0f;
			fValY = Ignore(m_xbStatus[iControllerID].Gamepad.sThumbRY) / 32768.0f * 50.0f;
		}
		else
		{
			// Read values from the left stick and scale to 0 - 50
			fValX = Ignore(m_xbStatus[iControllerID].Gamepad.sThumbLX) / 32768.0f * 50.0f;
			fValY = Ignore(m_xbStatus[iControllerID].Gamepad.sThumbLY) / 32768.0f * 50.0f;
		}
		
		// Attenuate
		fX = fValX * fValX / 2500.0f;
		fY = fValY * fValY / 2500.0f;

		// Restore sign
		if (fValX < 0.0f)
			fX = -fX;
		if (fValY < 0.0f)
			fY = -fY;
	}
	else
	{
		// Linear change of intensity

		if (eWhichSide == eRight)
		{
			// Read from the right side
			fX = Ignore(m_xbStatus[iControllerID].Gamepad.sThumbRX) / 32768.0f;
			fY = Ignore(m_xbStatus[iControllerID].Gamepad.sThumbRY) / 32768.0f;
		}
		else
		{
			// Read from the left side
			fX = Ignore(m_xbStatus[iControllerID].Gamepad.sThumbLX) / 32768.0f;
			fY = Ignore(m_xbStatus[iControllerID].Gamepad.sThumbLY) / 32768.0f;
		}
	}
}


namespace{
  eDigitalButton but_dig[] = { eDigitalUp, eDigitalDown, eDigitalLeft, eDigitalRight, eStart, eBack, eLeftStick, eRightStick };
  eAnalogButton but_an[] = {eA,eB,eX,eY, eBlack, eWhite, eLeftTrigger, eRightTrigger };
}

bool CXGamepad::KeyDown(int p_key)
{
  if(p_key == XKEY_MAXIS_X || p_key == XKEY_MAXIS_Y)
    return true;

  if(XKEY_GP_A <= p_key && p_key <= XKEY_GP_RIGHT_TRIGGER)
  {
    int b = ( p_key>> 24) - (XKEY_GP_A >> 24);
    return (GetAnalogButtonValue(but_an[b], eAll) > 0.4f);
  }

  if(XKEY_GP_DPAD_UP <= p_key && p_key <= XKEY_GP_RIGHT_THUMB)
  {
    int b = ( p_key>> 24) - (XKEY_GP_DPAD_UP >> 24);
    return IsDigitalButtonPressed(but_dig[b], false, eAll);
  }

  if(XKEY_GP_STHUMBLUP <= p_key && p_key <= XKEY_GP_STHUMBLRIGHT)
  {
    if (m_xbLastStatus[0].dwPacketNumber != m_xbStatus[0].dwPacketNumber)
      GetAnalogStickValue(m_fRX, m_fRY, eLeft, eAll);
    switch(p_key)
    {
      case XKEY_GP_STHUMBLUP:
        return (m_fRY >  0.4f);
      case XKEY_GP_STHUMBLDOWN:
        return (m_fRY < -0.4f);
      case XKEY_GP_STHUMBLLEFT:
        return (m_fRX < -0.4f);
      case XKEY_GP_STHUMBLRIGHT:
        return (m_fRX >  0.4f);
    }
  }
  return false;
}




bool CXGamepad::KeyPressed(int p_key)
{
  if(XKEY_GP_A <= p_key && p_key <= XKEY_GP_RIGHT_TRIGGER)
  {
    int b = ( p_key>> 24) - (XKEY_GP_A >> 24);
    return (GetAnalogButtonValue(but_an[b], eAll) > 0.4f);
  }

  if(XKEY_GP_DPAD_UP <= p_key && p_key <= XKEY_GP_RIGHT_THUMB)
  {
    int b = ( p_key>> 24) - (XKEY_GP_DPAD_UP >> 24);
    return IsDigitalButtonPressed(but_dig[b], true, eAll);
  }
  return false;
}


bool CXGamepad::KeyReleased(int p_key)
{
  return false;
}

float	CXGamepad::GetDeltaX()
{
  if (m_xbLastStatus[0].dwPacketNumber != m_xbStatus[0].dwPacketNumber)
    GetAnalogStickValue(m_fLX, m_fLY, eRight, eAll);
  return m_fLX * 5;
}

float	CXGamepad::GetDeltaY()
{
  if (m_xbLastStatus[0].dwPacketNumber != m_xbStatus[0].dwPacketNumber)
    GetAnalogStickValue(m_fLX, m_fLY, eRight, eAll);
  return m_fLY * 5;
}




