
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Joystick.h
//
//	History:
//	-Jan 31,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef JOYSTICK_H
#define JOYSTICK_H

#define JOY_DIR_UP		0
#define JOY_DIR_DOWN	1
#define JOY_DIR_LEFT	2
#define JOY_DIR_RIGHT	3
#define JOY_DIR_NONE	-1

#define JOY_UPDATE_TIME		1.0f/10.0f //10 times for second


#include "Cry_Math.h"

#ifdef PS2

#include <libpad.h>


/////BUTTON'S ID FOR PS2

#define PS2_BUTTON_SQUARE   1
#define PS2_BUTTON_CROSS    2
#define PS2_BUTTON_TRIANGLE 3
#define PS2_BUTTON_CIRCLE   4
#define PS2_BUTTON_L1       5
#define PS2_BUTTON_L2       6
#define PS2_BUTTON_R1       7
#define PS2_BUTTON_R2       8
#define PS2_BUTTON_START    9
#define PS2_BUTTON_SELECT   10

#define PS2_DIR_LEFT        1
#define PS2_DIR_RIGHT       2
#define PS2_DIR_UP          3
#define PS2_DIR_DOWN        4



#endif

/*
===========================================
The Joystick interface Class
===========================================
*/
struct ILog;
//////////////////////////////////////////////////////////////////////
class CJoystick
{
public:
	CJoystick();
	~CJoystick();

	bool	Init(ILog *pLog);
	void	Update();			
	void	ShutDown();

	int		GetNumButtons();	
	bool	IsButtonPressed(int buttonnum);
	int		GetDir();
	int		GetHatDir();

	// get analog direction (two analog sticks)
	Vec3	GetAnalog1Dir(unsigned int joystickID) const;
	Vec3	GetAnalog2Dir(unsigned int joystickID) const;

private:	

	bool	m_initialized;
	bool	m_hatswitch;

	int		m_numbuttons;	
	float	m_joytime;
	
	unsigned char m_buttons[8];
	unsigned char m_dirs[4];
	unsigned char m_hatdirs[4];	

	unsigned int	m_numjoysticks;	//!<	Number of available joysticks
	// this is for returning analog controller directional input. Input is in the range -1..1 for the x and y axis. z unused
	Vec3	*m_vAnalog1Dir;
	Vec3	*m_vAnalog2Dir;

	ILog *m_pLog;

	
	
/////////////////////////////////////////////////////////////////
///////////////PS2 SPECIFIC CODE ////////////////////////////////
/////////////////////////////////////////////////////////////////	
	
#ifdef PS2

	//PS2 INTERFACE
	public:
	
	
	bool ButtonPressed(int Type);
	bool ButtonHasBeenPressed(int Type);
	
	
	bool DigitalCrossDirection(int Type);
	unsigned char LeftStickX();
	unsigned char LeftStickY();
	unsigned char RightStickX();
	unsigned char RightStickY();
	
	

	//PS2 "INTERNAL" UTILITY 
	private:
	
	void GetAnalog(unsigned char *pData);
	void GetDigital(unsigned char *pData);
	void GetButton(unsigned char *pData);
	
	//Convert the PS2 pad information in PC style format(used by the interface).
	void ConvertPadInformation();

	
	//The type of Joystick
	int JoyType;
	
	
	//For analogic stick
	unsigned char m_RightX;
	unsigned char m_RightY;
	
	unsigned char m_LeftX;
	unsigned char m_LeftY;
	
	//For digital stick
	bool m_Left;
	bool m_Right;
	bool m_Up;
	bool m_Down;
	
	//Digital button
	
	bool m_L1;
	bool m_L2;
	bool m_R1;
	bool m_R2;
	
	bool m_Square;
	bool m_Circle;
	bool m_Triangle;
	bool m_Cross;
	
	bool m_Start;
	bool m_Select;
	
	bool m_TriggerL;
	bool m_TriggerR;
	
	
	
	
	
	bool m_old_L1;
	bool m_old_L2;
	bool m_old_R1;
	bool m_old_R2;
	
	bool m_old_Square;
	bool m_old_Circle;
	bool m_old_Triangle;
	bool m_old_Cross;
	
	bool m_old_Start;
	bool m_old_Select;
	
	bool m_old_TriggerL;
	bool m_old_TriggerR;
	
	//USED TO SIMULATE MOUSE/////////////////
	
	float m_fMouseScreenX;
	float m_fMouseScreenY;
	
	float m_fMouseDeltaX;
	float m_fMouseDeltaY;
	
	public:
	
	float GetVScreenX();
	float GetVScreenY();
	
	float GetDeltaX();
	float GetDeltaY();
	
	
	void SetScreenX(float x){m_MouseScreenX=x;}
	void SetScreenY(float y){m_MouseScreenY=y;}
	
	
	
	//END MOUSE SIMULATION //////////////////
	
	
	///USED TO SIMULATE THE KEYBOARD ////////

	bool KeyDown(int p_key);
	bool KeyPressed(int p_key);
	
	////////////////////////////////////////
	
	bool HasControl;
	bool HasMovControl(){return HasControl;}
	
	
	
	
	

#endif

/////////////////////////////////////////////////////////////////
///////////////END PS2 SPECIFIC CODE ////////////////////////////
/////////////////////////////////////////////////////////////////

};

#endif
