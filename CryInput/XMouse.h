// XMouse.h: interface for the CXMouse class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XMOUSE_H__E79C3125_5BF0_499D_B76E_6C1070554E8F__INCLUDED_)
#define AFX_XMOUSE_H__E79C3125_5BF0_499D_B76E_6C1070554E8F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IInput.h>
struct ITimer;
struct ILog;

#ifdef PS2
#include "Joystick.h"
#endif

struct ICVar;

#define XMOUSE_MAX_MOUSE_EVENTS		12				// e.g. mouse buttons, axis, ..

class CXMouse : 
public IMouse  
{
public:
	CXMouse()
	{ 
		m_exmode=false;
		m_pMouse = NULL;
		m_kInertia=0;
		m_fAccelerationScale=0.5f;
		m_bAcceleration=false;
		i_mouse_buffered = NULL;
	}
	virtual ~CXMouse(){
	}

	class CInput* m_pInput;
	
#ifdef WIN32
	bool	Init(ISystem *pSystem,LPDIRECTINPUT8 &g_pdi,HINSTANCE hinst,HWND hwnd,bool dinput);
#else
	
	////////////////////////////////////////////////////
	///////////////PS2 SPECIFIC INIT ///////////////////
	////////////////////////////////////////////////////
	#ifdef PS2
	
		bool m_Initialized;
	
		bool	Init(ISystem *pSystem);
		bool IsInit(){return m_Initialized;}
	
	#endif
	////////////////////////////////////////////////////
	///////////////END PS2 SPECIFIC ////////////////////
	////////////////////////////////////////////////////
	
#endif

	void	Shutdown();
	void	Update(bool bPrevFocus);

	void	ClearKeyState();

	bool	MouseDown(int p_numButton);
	bool	MousePressed(int p_numButton);
	bool	MouseDblClick(int p_numButton);
	bool	MouseReleased(int p_numButton);
	bool GetOSKeyName(int nKey, wchar_t *szwKeyName, int iBufSize);
	
//  int	  GetMouseWheelRotation(bool reset = true); // get wheel state for aplication
  void  SetMouseWheelRotation(int value); // set wheel state from app message loop

	bool	SetExclusive(bool value,void *hwnd=0);

	inline float	GetDeltaX() { return m_Deltas[0]*(m_fSensitivity*m_fSensitivityScale); }
	inline float	GetDeltaY() { return m_Deltas[1]*(m_fSensitivity*m_fSensitivityScale); }
	inline float	GetDeltaZ()
	{
		m_wheelChecked = true;
		return m_Deltas[2]*(m_fSensitivity*m_fSensitivityScale);
	}

	void SetSensitvity(float fSensitivity)
	{
		if(fSensitivity>0)
			m_fSensitivity=fSensitivity/100;
		else
			m_fSensitivity=0.0f;
	}
	float GetSensitvity()
	{
		return m_fSensitivity*100;
	}
	void SetSensitvityScale(float fSensScale)
	{
		m_fSensitivityScale=fSensScale;
	}
	float GetSensitvityScale()
	{
		return m_fSensitivityScale;
	}
	inline void SetInertia(float kinertia) { m_kInertia=kinertia; }

	void SetVScreenX(float fX);
	void SetVScreenY(float fY);
	float GetVScreenX();
	float GetVScreenY();
private:
	bool	Acquire();
	bool	UnAcquire();
	int XKEY2IDX(int nKey);

	void PostEvent( int key,SInputEvent::EType type,float value=0,unsigned int timestamp=0 );

	//smooth movement & mouse accel
	void CapDeltas(float cap);
	void SmoothDeltas(float accel,float decel=0.0f);
	
#ifdef WIN32
	LPDIRECTINPUTDEVICE8     m_pMouse;	
#else
	void  *					m_pMouse;
#endif	

	HINSTANCE				m_hinst;
	HWND						m_hwnd;	

#ifndef PS2
	POINT						m_previouspoint;
#endif

	ICVar *					i_mouse_buffered;
	//! mouse accel cvars
	ICVar *					i_mouse_accel;
	ICVar *					i_mouse_accel_max;
	//! mouse smooth cvar
	ICVar *					i_mouse_smooth;
	//! mouse mirror cvar
	ICVar *					i_mouse_mirror;

	float						m_fDblClickTime;

	float						m_Deltas[5];
	float						m_OldDeltas[2];//!< just old x,y, to smooth movement.

	float						m_DeltasInertia[2];
	float						m_kInertia;
	unsigned char		m_Events[XMOUSE_MAX_MOUSE_EVENTS];				//!<
	unsigned char		m_oldEvents[XMOUSE_MAX_MOUSE_EVENTS];			//!<
	float						m_fLastRelease[XMOUSE_MAX_MOUSE_EVENTS];	//!<
	bool						m_bDblClick[XMOUSE_MAX_MOUSE_EVENTS];			//!<
	
	bool						m_exmode;	
	bool						m_dinput;
	bool						m_wheelChecked;
	
	float						m_fVScreenX;
	float						m_fVScreenY;

	ILog *					m_pLog;
	ITimer *				m_pTimer;
	ISystem *				m_pSystem;

	float						m_fSensitivity;
	float 					m_fSensitivityScale;
	float 					m_fAccelerationScale;
	bool						m_bAcceleration;

	
	
	
//////////////////////////////////////////////////////////////////
//////////////PS2 SPECIFIC////////////////////////////////////////
//////////////////////////////////////////////////////////////////
	
#ifdef PS2

	int m_MousePort;
	
	
	int XPos;
	int YPos;
	int XOldPos;
	int YOldPos;
	
	



	public:
	
		//Used to simulate mouse input with pad (if needed)
		void SetPad(CJoystick *pJoy){m_pJoy=pJoy;}
	
	private:
	
		//If no mouse connected, use pad to simulate it
		CJoystick *m_pJoy;
	
#endif


//////////////////////////////////////////////////////////////////
//////////////END PS2 SPECIFIC////////////////////////////////////
//////////////////////////////////////////////////////////////////

};

#endif // !defined(AFX_XMOUSE_H__E79C3125_5BF0_499D_B76E_6C1070554E8F__INCLUDED_)
