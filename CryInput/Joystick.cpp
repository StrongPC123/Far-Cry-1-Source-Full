//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Joystick.cpp
//  Description: joystick function support
//
//	History:
//	-Jan 31,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include <ILog.h>
#include "Joystick.h"

#ifdef WIN32
#include "Mmsystem.h"
#pragma comment(lib, "winmm.lib")
#endif

#ifdef PS2

#include <sifdev.h>
#define TRESHOLD 30

extern "C" void PS2GDRV_AckGrabScreenshot();
//Statics 

//To use the DMA
static u_long128 pad_dma_buf[scePadDmaBufferMax] __attribute__((aligned (64)));


//MOUSE EMULATION

#define MOUSE_THRESOLD		 60
#define DELTA_SCREEN   	     5
#define MOUSE_DELTA_SPEED_MAX 16.0f



//KEYBOARD EMULATION
#define FORWARD_KEY		17
#define BACKWARD_KEY    31  


#endif


#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

//////////////////////////////////////////////////////////////////////
CJoystick::CJoystick()
{
	m_pLog=NULL;
	memset(m_dirs,0,sizeof(m_dirs));
	memset(m_buttons,0,sizeof(m_buttons));
	memset(m_hatdirs,0,sizeof(m_hatdirs));

	m_numbuttons=0;
	m_hatswitch=false;
	m_joytime=0;
	m_numjoysticks = 0;

	m_vAnalog1Dir = NULL;
	m_vAnalog2Dir = NULL;

	//PS2 SPECIFIC initialization
#ifdef PS2

	// No "direction" at start
	m_RightX=m_RightY=m_LeftX=m_LeftY=0;	
	
	// Flag to keep track of different used pad (only dual analog supported right now)
	JoyType=0;
	
	// Mouse simulation
	m_fMouseScreenX=0;
	m_fMouseScreenY=0;
	
	m_fMouseDeltaX=0;
	m_fMouseDeltaY=0;
	
	HasControl=false;	
#endif

	m_initialized=false;
}

CJoystick::~CJoystick()
{
	delete [] m_vAnalog1Dir;
	delete [] m_vAnalog2Dir;
}

///////////////////////////////////////////
bool CJoystick::Init(ILog *pLog)
{
	m_pLog=pLog;

	//WINDOWS SPECIFIC INIT

#ifdef WIN32	
	JOYCAPS joycaps;

	JOYINFOEX	joyinfo;
	MMRESULT	result;

	/* check to see if a joystick driver is loaded */	
	int numJoy = joyGetNumDevs();
	m_numjoysticks = 0;
	
	if (numJoy)
	{
		m_pLog->LogToFile("Found %d joystick devices\n", numJoy);

		for (int i = 0; i < numJoy; ++i)
		{
			/* make sure a joystick is plugged in */
			joyinfo.dwSize = sizeof(JOYINFOEX);
			joyinfo.dwFlags = JOY_RETURNALL;
			result = joyGetPosEx(i, &joyinfo);
			if (result != JOYERR_NOERROR)
			{
				m_pLog->LogToFile("Joystick %d not plugged in!\n", i);
				continue;
			}

			// we have an attached joystick, so count up.
			m_numjoysticks++;

			/* find out what the joystick can do */
			result = joyGetDevCaps(i, &joycaps, sizeof(JOYCAPS));
			if (result == JOYERR_NOERROR)
			{
				m_numbuttons = joycaps.wNumButtons;
				m_pLog->LogToFile("Num joy buttons=%d\n", m_numbuttons);
				m_pLog->LogToFile("Joystick name=%s\n", joycaps.szPname);

				if (joycaps.wCaps & JOYCAPS_HASPOV) 
					m_hatswitch=true;
				else 
					m_hatswitch=false;
				if (m_hatswitch) 
					m_pLog->LogToFile("Joystick has hatswitch\n");
			}
		}

		m_initialized=(m_numjoysticks!=0);

		// allocate memory for the analog sticks
		m_vAnalog1Dir = new Vec3[m_numjoysticks];
		m_vAnalog2Dir = new Vec3[m_numjoysticks];
		return (true);
	}

	m_pLog->LogToFile("Cannot find joystick device\n");	
	return (false);
#endif

//PS2 SPECIFIC INIT

#ifdef PS2

	int valRet;
	int JoyID,JoyExID;

	//Init the controller system
	scePadInit(0);
	
	
	//Open pad port.....PAD 0 in PORT 0
	valRet = scePadPortOpen(0,0,pad_dma_buf);
	
	if(!valRet)
	{
		m_pLog->LogToFile("Can't open PS2 pad 0 port 0");
		return false;
			
	}
	
	
	/////////////Wait pad to be stable
	int padState;
	while((padState=scePadGetState(0,0))==scePadStateExecCmd);
	
	if(padState!=scePadStateStable)
	{
		m_pLog->LogToFile("The pad is not stable.......");	
		return false;
	}	


			
	/////////////Get pad type
	
	JoyID = scePadInfoMode(0, 0, InfoModeCurID, 0 );
	
	if (JoyID==0)
	{
		m_pLog->LogToFile("Can't get pad type!!!");	
		return false;
			
	}	

	
	JoyExID=0;
	JoyExID = scePadInfoMode(0, 0, InfoModeCurExID,0);
	if (JoyExID>0) 
	{
		JoyID = JoyExID;
		
	}
	
	
	//Save the joystick type in Joystick class
	JoyType=JoyID;
	
	
	//LOCK THE ANALOG MODE
	scePadSetMainMode(0,0,1,0);
	
	
	
	//INITIALIZATION WAS OK
	m_initialized=true;
	return true;
			

#endif


}

///////////////////////////////////////////
void CJoystick::Update()
{	

	//Check if joystick was initialized

	if (!m_initialized) 
		return;

	//WARNING remember
	//use a time check like this one
	//do not call the update every frame
	//
	//float currtime=CSystem::GetTimer()->GetCurrTime();
	//if (currtime-m_joytime<JOY_UPDATE_TIME) 
	//	return;	

	//m_joytime=currtime;

#ifdef WIN32	
	/* get the current status */

	JOYINFOEX	joyinfo;
	MMRESULT	result;

	joyinfo.dwSize = sizeof(JOYINFOEX);
	joyinfo.dwFlags = JOY_RETURNALL;
	result = joyGetPosEx(JOYSTICKID1, &joyinfo);

	if (result != JOYERR_NOERROR) 
		return;			

	memset(m_dirs,0,sizeof(m_dirs));
	memset(m_buttons,0,sizeof(m_buttons));
	m_vAnalog1Dir[0].Set(0,0,0);
	m_vAnalog2Dir[0].Set(0,0,0);
		
	if (joyinfo.dwXpos < 16384)			
		m_dirs[JOY_DIR_LEFT] = 1;
	else 
	if (joyinfo.dwXpos > 65535-16384)	
		m_dirs[JOY_DIR_RIGHT] = 1;

	if (joyinfo.dwYpos < 16384)			
		m_dirs[JOY_DIR_UP] = 1;
	else 
	if(joyinfo.dwYpos > 65535-16384)	
		m_dirs[JOY_DIR_DOWN] = 1;

	if (m_hatdirs)
	{
		memset(m_hatdirs,0,sizeof(m_hatdirs));	
		if (joyinfo.dwPOV == JOY_POVLEFT)		m_hatdirs[JOY_DIR_LEFT]=1;
		if (joyinfo.dwPOV == JOY_POVRIGHT)		m_hatdirs[JOY_DIR_RIGHT]=1;
		if (joyinfo.dwPOV == JOY_POVFORWARD)	m_hatdirs[JOY_DIR_UP]=1;		
		if(joyinfo.dwPOV == JOY_POVBACKWARD)	m_hatdirs[JOY_DIR_DOWN]=1;		
	}

	m_buttons[0]= (unsigned char)(joyinfo.dwButtons & JOY_BUTTON1);
	m_buttons[1]= (unsigned char)(joyinfo.dwButtons & JOY_BUTTON2);
	m_buttons[2]= (unsigned char)(joyinfo.dwButtons & JOY_BUTTON3);
	m_buttons[3]= (unsigned char)(joyinfo.dwButtons & JOY_BUTTON4);
	m_buttons[4]= (unsigned char)(joyinfo.dwButtons & JOY_BUTTON5);
	m_buttons[5]= (unsigned char)(joyinfo.dwButtons & JOY_BUTTON6);
	m_buttons[6]= (unsigned char)(joyinfo.dwButtons & JOY_BUTTON7);
	m_buttons[7]= (unsigned char)(joyinfo.dwButtons & JOY_BUTTON8);

	// this is a bit ugly, but the input system doesn't deserve better
	int iAnalog1X = joyinfo.dwXpos - 32768;
	int iAnalog1Y = joyinfo.dwYpos - 32768;
	int iAnalog2X = joyinfo.dwZpos - 32768;
	int iAnalog2Y = joyinfo.dwRpos - 32768;

	if (abs(iAnalog1X) < 100) iAnalog1X = 0;
	if (abs(iAnalog1Y) < 100) iAnalog1Y = 0;
	if (abs(iAnalog2X) < 100) iAnalog2X = 0;
	if (abs(iAnalog2Y) < 100) iAnalog2Y = 0;

	m_vAnalog1Dir[0].x = iAnalog1X/32768.0f;
	m_vAnalog1Dir[0].y = iAnalog1Y/32768.0f;
	m_vAnalog1Dir[0].z = 0.0f;
	m_vAnalog2Dir[0].x = iAnalog2X/32768.0f;
	m_vAnalog2Dir[0].y = iAnalog2Y/32768.0f;
	m_vAnalog2Dir[0].z = 0.0f;

	if (m_numjoysticks > 1)
	{
		m_vAnalog1Dir[1].Set(0,0,0);
		m_vAnalog2Dir[1].Set(0,0,0);

		joyinfo.dwSize = sizeof(JOYINFOEX);
		joyinfo.dwFlags = JOY_RETURNALL;
		result = joyGetPosEx(JOYSTICKID2, &joyinfo);

		if (result != JOYERR_NOERROR) 
			return;			
		
		iAnalog1X = joyinfo.dwXpos - 32768;
		iAnalog1Y = joyinfo.dwYpos - 32768;
		iAnalog2X = joyinfo.dwZpos - 32768;
		iAnalog2Y = joyinfo.dwRpos - 32768;

		if (abs(iAnalog1X) < 100) iAnalog1X = 0;
		if (abs(iAnalog1Y) < 100) iAnalog1Y = 0;
		if (abs(iAnalog2X) < 100) iAnalog2X = 0;
		if (abs(iAnalog2Y) < 100) iAnalog2Y = 0;

		m_vAnalog1Dir[1].x = iAnalog1X/32768.0f;
		m_vAnalog1Dir[1].y = iAnalog1Y/32768.0f;
		m_vAnalog1Dir[1].z = 0.0f;
		m_vAnalog2Dir[1].x = iAnalog2X/32768.0f;
		m_vAnalog2Dir[1].y = iAnalog2Y/32768.0f;
		m_vAnalog2Dir[1].z = 0.0f;
	}
#endif	

//PS2 SPECIFIC UPDATE FUNCTION

#ifdef PS2

	int readReturn=0;
	unsigned char JoyData[32];
	int PadState;
	
	
	//Check that the pad is connected
	PadState=scePadGetState(0,0);
	
	
	//Elaborate the input only if the pad is connected
	if(PadState==scePadStateStable)
	{
	
		//Read the joypad information
		readReturn=scePadRead(0,0, JoyData );
		if(readReturn!=0)
		{
			//Check what kind of joystick is plugged...now just Dual Analog is supported
		
			switch(JoyType)
			{
			
				///////NOT SUPPORTED PAD//////////////////
				
				case 2:  //"NeGi-CON"
					
					break;

				case 3: //"GunCON(Konami)"
				
					break;
					
				///////////////////////////////////////////

				
				
				
				//N.B: Only ANALOG AND STANDARD SUPPORTED!!!!!
				
				case 4: //"STANDARD"
				case 7: //"ANALOG"
				
					//Just for now...do the same for STANDARD AND ANALOG.....
					
					GetAnalog(JoyData);
					GetDigital(JoyData);
					GetButton(JoyData);
					
					ConvertPadInformation();
			
					if(m_TriggerR && m_TriggerL)	
						PS2GDRV_AckGrabScreenshot();
			
					break;
				
				
				
				
					
					
				///////NOT SUPPORTED PAD//////////////////
					
				case 5: //"JOYSTICK"
					
					break;

				case 6: //"GunCON(NAMCO)"
			
					break;
				
				case 0x100: //"TSURI-CON"
					
					break;
					
				case 0x300: //"JOG-CON"
					
					break;
					
				///////////////////////////////////////////

			}
			
		}
		else
		{
			 m_pLog->LogToFile("Joystick ERROR!!! Can't read Joystick information\n");
		}
			
		
		
	}
	else
	{
		 m_pLog->LogToFile("Joystick ERROR!!! Pad not connected\n");
	}
		
	
	
	
	//////////////////////////////////////////////////////////////////////////////////////////
	//////// N.B: MOUSE SIMULATION USED (By game & menu) OLY IF MOUSE NOT CONNECTED //////////
	//////////////////////////////////////////////////////////////////////////////////////////


	//////////////MOUSE SIMULATION ///////////////////////////////////////////////////////////
	
	m_MouseDeltaX=0;
	m_MouseDeltaY=0;

	
	//Use left stick to simulate the mouse
	
	
	////Pointer contro false
	HasControl=false;
	
	
	if(m_LeftX> 127+MOUSE_THRESOLD)
	{
		//The pad ()instead of mouse) has the control over the "movement"
		HasControl=true;
	
		m_fMouseScreenX+=DELTA_SCREEN;
		
		if(m_fMouseScreenX > 800)
			m_fMouseScreenX=800;
	}
	

	if(m_LeftX< 127-MOUSE_THRESOLD)
	{
		//The pad ()instead of mouse) has the control over the "movement"
		HasControl=true;
	
	
		m_fMouseScreenX-=DELTA_SCREEN;
		
		if(m_fMouseScreenX<0)
			m_fMouseScreenX=0;			
	}
	
	
	if(m_LeftY> 127+MOUSE_THRESOLD )
	{
		//The pad ()instead of mouse) has the control over the "movement"
		HasControl=true;
		
		m_fMouseScreenY+=DELTA_SCREEN;
		
		if(m_fMouseScreenY>600)
			m_fMouseScreenY=600;
	}
	
	if(m_LeftY< 127-MOUSE_THRESOLD)
	{
		//The pad ()instead of mouse) has the control over the "movement"
		HasControl=true;
	
		if(m_fMouseScreenY<0)
			m_fMouseScreenY=0;
		m_fMouseScreenY-=DELTA_SCREEN;		
	}
	

	
	//////////////////MOSUE DELTA SIMULATION///////////////////////////////////
	
	
	float factor;
	float finalDelta=MOUSE_DELTA_SPEED_MAX;
	
	

	
	if(m_RightX> 127+MOUSE_THRESOLD)
	{
	
	
		factor=(((float)m_RightX)-127.0f)/127;
		m_MouseDeltaX= finalDelta*factor;	
	
		
	}
	

	if(m_RightX< 127-MOUSE_THRESOLD)
	{	
		factor=(127.0f-((float)m_RightX))/127;
		m_MouseDeltaX= -finalDelta*factor;
	}
	
	
	if(m_RightY> 127+MOUSE_THRESOLD )
	{
	
		factor=(((float)m_RightY)-127.0f)/127;
		
		m_MouseDeltaY= finalDelta*factor;
			
	}
	
	if(m_RightY< 127-MOUSE_THRESOLD)
	{
	
		factor=(127.0f-((float)m_RightY))/127;
		m_MouseDeltaY= -finalDelta*factor;
		
	}
	
	
	

	//////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////  END MOUSE SIMULATION /////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////




#endif

}

///////////////////////////////////////////
int CJoystick::GetDir()
{
	if (!m_initialized) 
		return (JOY_DIR_NONE);
	
	for (int k=0;k<4;k++) 
		if (m_dirs[k]) 
			return (k);
		
	return (JOY_DIR_NONE);
}

///////////////////////////////////////////
int CJoystick::GetHatDir()
{
	if (!m_initialized) 
		return (JOY_DIR_NONE);
	
	for (int k=0;k<4;k++) 
		if (m_hatdirs[k]) 
			return (k);
		
	return (JOY_DIR_NONE);
}

///////////////////////////////////////////
Vec3 CJoystick::GetAnalog1Dir(unsigned int joystickID) const
{
	if (joystickID < m_numjoysticks)
		return m_vAnalog1Dir[joystickID];
	else
		return Vec3(0,0,0);
}
///////////////////////////////////////////
Vec3 CJoystick::GetAnalog2Dir(unsigned int joystickID) const
{
	if (joystickID < m_numjoysticks)
		return m_vAnalog2Dir[joystickID];
	else
		return Vec3(0,0,0);
}


///////////////////////////////////////////
int CJoystick::GetNumButtons()
{
	return (m_numbuttons);
}

///////////////////////////////////////////
bool CJoystick::IsButtonPressed(int buttonnum)
{
	if (!m_initialized || buttonnum>m_numbuttons) 
		return (false);	
	if (m_buttons[buttonnum]) 
		return (true);
	return (false);
}

///////////////////////////////////////////
void CJoystick::ShutDown()
{
	m_pLog->LogToFile("Joystick Shutdown\n");
	m_initialized=false;

#ifdef PS2
	
	//Close Pad 0 port 0
	scePadPortClose(0,0);
	
	
#endif

}

//////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// PS2 SPECIFIC CODE///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef PS2

void CJoystick::GetAnalog(unsigned char *pData)
{

	//Get Joystick analog values
	
	//Left stick
	m_RightX=pData[4];
	m_RightY=pData[5];
	
	
	//Right stick
	m_LeftX=pData[6];
	m_LeftY=pData[7];
	

}

void CJoystick::GetDigital(unsigned char *pData)
{
	//get Joystick digital "cross" values

	m_Left=m_Right=m_Up=m_Down=false;
	unsigned char tempFlags;
	
	
	tempFlags=pData[2];	
	tempFlags= ~tempFlags;
	
	if(tempFlags&1<<7)
	{
		m_Left=true;	
	}
	
	if(tempFlags&1<<5)
	{
		m_Right=true;	
	}
	
	if(tempFlags&1<<4)
	{
		m_Up=true;	
	}
	
	if(tempFlags&1<<6)
	{
		m_Down=true;	
	}


}

void CJoystick::GetButton(unsigned char *pData)
{

	//Get PS2 button pression (digital not analog...)
	unsigned int tempFlags;
	

	
	///SWAP BUTTON STATUS
	
	m_old_L1=m_L1;
	m_old_L2=m_L2;
	m_old_R1=m_R1;
	m_old_R2=m_R2;
	
	m_old_Square=m_Square;
	m_old_Circle=m_Circle;
	m_old_Triangle=m_Triangle;
	m_old_Cross=m_Cross;
	
	m_old_Start=m_Start;
	m_old_Select=m_Select;
	
	m_old_TriggerL=m_TriggerL;
	m_old_TriggerR=m_TriggerR;
	



	m_L1=m_L2=m_R1=m_R2=m_Square=m_Circle=m_Triangle=m_Cross=m_Start=m_Select=m_TriggerR=m_TriggerR=false;
	

	tempFlags=0xffff ^ ( ( pData[2] << 8 ) | pData[3] );
	
	//Changed all this "if" to be SONY compilant
	if(tempFlags&SCE_PADL1)
	{
		m_L1=true;	
	}
	
	if(tempFlags&SCE_PADL2)
	{
		m_L2=true;	
	}
	
	if(tempFlags&SCE_PADR1)
	{
		m_R1=true;	
	}
	
	if(tempFlags&SCE_PADR2)
	{
		m_R2=true;	
	}
	
	
	
	
	
	if(tempFlags&SCE_PADRleft)
	{
		m_Square=true;	
	}
	
	if(tempFlags&SCE_PADRright)
	{
		m_Circle=true;	
	}
	
	if(tempFlags&SCE_PADRup)
	{
		m_Triangle=true;	
	}
	
	if(tempFlags&SCE_PADRdown)
	{
		m_Cross=true;	
	}
	
	
	//Start and select
//?!?!	tempFlags=pData[2];
//?!?!?	tempFlags=~tempFlags;
	
	
	if(tempFlags&SCE_PADstart)
	{
		m_Start=true;	
	}
	
	if(tempFlags&SCE_PADi)
	{
		m_TriggerL=true;	
	}
	
	
	if(tempFlags&SCE_PADj)
	{
		m_TriggerR=true;	
	}
	
	


}



void CJoystick::ConvertPadInformation()
{


	//Init arrys
	memset(m_dirs,0,sizeof(m_dirs));
	memset(m_buttons,0,sizeof(m_buttons));
	
	//This is not supported by PS2 - always false -
	memset(m_hatdirs,0,sizeof(m_hatdirs));


	//Digital directions
	
	//Left
	if(m_Left)
	{
		m_dirs[JOY_DIR_LEFT] = 1;
	}
	else if(m_LeftX<(125-TRESHOLD))
	{
		m_dirs[JOY_DIR_LEFT] = 1;	
	
	}
	
	//Right
	if(m_Right)
	{
		m_dirs[JOY_DIR_RIGHT] = 1;
	}
	else if(m_LeftX>(125+TRESHOLD))
	{
		m_dirs[JOY_DIR_RIGHT] = 1;	
	
	}
	
	
	//UP
	if(m_Up)
	{
		m_dirs[JOY_DIR_UP] = 1;
	}
	else if(m_LeftY<(125-TRESHOLD))
	{
		m_dirs[JOY_DIR_UP] = 1;	
	
	}
	
	//DOWN
	if(m_Down)
	{
		m_dirs[JOY_DIR_DOWN] = 1;
	}
	else if(m_LeftY>(125+TRESHOLD))
	{
		m_dirs[JOY_DIR_DOWN] = 1;	
	
	}
	
	
	//NOW THE BUTTONS
	

	if(m_Circle)
		m_buttons[0]=1;
		
	if(m_Triangle)
		m_buttons[1]=1;
	
	if(m_Square)
		m_buttons[2]=1;
		
	if(m_Cross)
		m_buttons[3]=1;
		
	if(m_L1)
		m_buttons[4]=1;
		
	if(m_L2)
		m_buttons[5]=1;
		
	if(m_R1)
		m_buttons[6]=1;
		
	if(m_R2)
		m_buttons[7]=1;
		 
	
}


////////////////PS2 SPECIFIC PUBLIC INTERFACE//////////////////////////////////////////


bool CJoystick::ButtonHasBeenPressed(int Type)
{
	
	switch (Type)
	{
	
		case PS2_BUTTON_SQUARE:
			return (m_Square&&!m_old_Square);
		break;
		
		case PS2_BUTTON_CROSS:
			return (m_Cross&&!m_old_Cross);
		break;
		
		case PS2_BUTTON_TRIANGLE:
			return (m_Triangle&&!m_old_Triangle);
		break;
		
		case PS2_BUTTON_CIRCLE:
			return (m_Circle&&!m_old_Circle);
		break;
		
		case PS2_BUTTON_L1:
			return (m_L1&&!m_old_L1);
		break;
		
		case PS2_BUTTON_L2:
			return (m_L2&&!m_old_L2);
		break;
		
		case PS2_BUTTON_R1:
			return (m_R1&&!m_old_R1);
		break;
		
		case PS2_BUTTON_R2:
			return (m_R2&&!m_old_R2);
		break;
		
		case PS2_BUTTON_START:
			return (m_Start&&!m_old_Start);
		break;
		
		case PS2_BUTTON_SELECT :
			return (m_Select&&!m_old_Select);
		break;
	
	
	}
		
	return false;
}





bool CJoystick::ButtonPressed(int Type)
{
	
	switch (Type)
	{
	
		case PS2_BUTTON_SQUARE:
			return m_Square;
		break;
		
		case PS2_BUTTON_CROSS:
			return m_Cross;
		break;
		
		case PS2_BUTTON_TRIANGLE:
			return m_Triangle;
		break;
		
		case PS2_BUTTON_CIRCLE:
			return m_Circle;
		break;
		
		case PS2_BUTTON_L1:
			return m_L1;
		break;
		
		case PS2_BUTTON_L2:
			return m_L2;
		break;
		
		case PS2_BUTTON_R1:
			return m_R1;
		break;
		
		case PS2_BUTTON_R2:
			return m_R2;
		break;
		
		case PS2_BUTTON_START:
			return m_Start;
		break;
		
		case PS2_BUTTON_SELECT :
			return m_Select;
		break;
	
	
	}
	
	
	
	return false;
		

}


bool CJoystick::DigitalCrossDirection(int Type)
{
	switch (Type)
	{
	
	
		case PS2_DIR_LEFT:
			return m_Left;	
		break;  
		
		case PS2_DIR_RIGHT:
			return m_Right;
		break; 
		
		case PS2_DIR_UP:
			return m_Up;
		break;    
		
		case PS2_DIR_DOWN:
			return m_Down;
		break;  
	}
	

}


unsigned char CJoystick::LeftStickX()
{
	return m_LeftX;
}


unsigned char CJoystick::LeftStickY()
{
	return m_LeftY;
}


unsigned char CJoystick::RightStickX()
{
	return m_RightX;
}


unsigned char CJoystick::RightStickY()
{
	return m_RightY;
}




/////SIMULATE MOUSE WITH JOYPAD

float CJoystick::GetVScreenX()
{
	return m_fMouseScreenX+0.5f;
}		 

float CJoystick::GetVScreenY()
{
	return m_fMouseScreenY+0.5f;
}			 


float  CJoystick::GetDeltaX()
{
	return m_MouseDeltaX;
}

float  CJoystick::GetDeltaY()
{
	return m_MouseDeltaY;
}





///USED TO SIMULATE THE KEYBOARD ////////

bool CJoystick::KeyDown(int p_key)
{

	switch(p_key)
	{
		case FORWARD_KEY:
		
		if(m_LeftY<127-MOUSE_THRESOLD)
			return true;
		else
			return false;
		
		break;
		
		case BACKWARD_KEY:
		
		if(m_LeftY>127+MOUSE_THRESOLD)
			return true;
		else
			return false;
		
		break;
	
		default:
			return false;
	}

	
}



bool CJoystick::KeyPressed(int p_key)
{


	switch(p_key)
	{
		case FORWARD_KEY:
		
		if(m_LeftY<127-MOUSE_THRESOLD)
			return true;
		else
			return false;
		
		break;
		
		case BACKWARD_KEY:
	
		
		if(m_LeftY>127+MOUSE_THRESOLD)
			return true;
		else
			return false;
		
		break;

	
		default:
			return false;

	}

	


}

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////END PS2 SPECIFIC CODE////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


#endif

