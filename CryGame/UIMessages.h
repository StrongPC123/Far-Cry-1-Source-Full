//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - UI System Message Definitions
//
// History:
//  - [25/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#pragma once



//! Convert a 2d float coordinate into an 2d integer packed in a dword [xxxx:yyyy]
#define UIM_PACK_COORD(x, y)		((((short)(x)) << 16) | ((short)(y)))

//! Get coordingates from packed unsigned int
#define UIM_GET_X_FLOAT(packed)		((float)((packed) >> 16))
//! Get coordingates from packed unsigned int
#define UIM_GET_Y_FLOAT(packed)		((float)((packed) & 0xffff))
//! Get coordingates from packed unsigned int
#define UIM_GET_X(packed)			((short)((packed) >> 16))
//! Get coordingates from packed unsigned int
#define UIM_GET_Y(packed)			((short)((packed) & 0xffff))


// Convert a float to a int without casting, so it can be recovered back
inline unsigned int UIM_FLOAT_TO_DWORD(float fValue)
{
	static float fValueToDW;
	fValueToDW = fValue;
	
	return *((unsigned int *)(&fValueToDW));
}

// Convert a int to a float without casting, so it can be recovered back
inline float UIM_DWORD_TO_FLOAT(unsigned int iValue)
{
	static unsigned int iValueToFloat = iValue;
	iValueToFloat = iValue;

	return *((float *)(&iValueToFloat));
}


//------------------------------------------------------------------------------------------------- 
// keyboard events
//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 

//! a key was pressed
//! wParam contains the unicode char (wchar_t)
//! lParam contais the xkey code
#define UIM_KEYDOWN				(0x0001)	

//! a key was released
//! wParam contains the unicode char (wchar_t)
//! lParam contais the xkey code
#define UIM_KEYUP				(0x0002)

//! a key was pressed
//! wParam contains the unicode char (wchar_t)
//! lParam contais the xkey code
#define UIM_KEYPRESSED			(0x0003)



//------------------------------------------------------------------------------------------------- 
// mouse events
//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 

//! a mouse button was pressed
//! wParam contains the packed integer absolute position [xxxx:yyyy]
//! lParam contains the button xkey code
#define UIM_MOUSEDOWN			(0x0010)

//! a mouse button was released
//! wParam contains the packed integer absolute position [xxxx:yyyy]
//! lParam contains the button xkey code
#define UIM_MOUSEUP				(0x0011)

//! mouse cursor moved over the widget surface
//! wParam contains the old packed integer relative position [xxxx:yyyy]
//! lParam contains the new packed integer relative position [xxxx:yyyy]
#define UIM_MOUSEOVER			(0x0012)

//! mouse cursor moved
//! wParam contains the old packed integer absolute position [xxxx:yyyy]
//! lParam contains the new packed integer absolute position [xxxx:yyyy]
#define UIM_MOUSEMOVE			(0x0013)

//! left mouse button pressed
//! wParam contains the packed integer relative position [xxxx:yyyy]
//! lParam contains the button xkey code
#define UIM_LBUTTONDOWN			(0x0014)

//! left mouse button released
//! wParam contains the packed integer relative position [xxxx:yyyy]
//! lParam contains the button xkey code
#define UIM_LBUTTONUP			(0x0015)

//! left mouse button click
//! wParam contains the packed integer relative position [xxxx:yyyy]
//! lParam contains the button xkey code
#define UIM_LBUTTONCLICK		(0x0016)

//! left mouse button doubleclick
//! wParam contains the packed integer relative position [xxxx:yyyy]
//! lParam contains the button xkey code
#define UIM_LBUTTONDBLCLICK		(0x0017)

//! right mouse button pressed
//! wParam contains the packed integer relative position [xxxx:yyyy]
//! lParam contains the button xkey code
#define UIM_RBUTTONDOWN			(0x0018)

//! right mouse button released
//! wParam contains the packed integer relative position [xxxx:yyyy]
//! lParam contains the button xkey code
#define UIM_RBUTTONUP			(0x0019)

//! right mouse button click
//! wParam contains the packed integer relative position [xxxx:yyyy]
//! lParam contains the button xkey code
#define UIM_RBUTTONCLICK		(0x001a)

//! right mouse button double click
//! wParam contains the packed integer relative position [xxxx:yyyy]
//! lParam contains the button xkey code
#define UIM_RBUTTONDBLCLICK		(0x001b)

//! mouse enters the widget
#define UIM_MOUSEENTER			(0x001c)

//! mouse leaves the widget
#define UIM_MOUSELEAVE			(0x001d)



//------------------------------------------------------------------------------------------------- 
// widget events
//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 

//! widget got focus
//! send after the widget got the input focus
#define UIM_GOTFOCUS			(0x0020)

//! widget lost focus
//! send after the widget lost the input focus
#define UIM_LOSTFOCUS			(0x0021)

//! widget changed size
//! sent after the widget changed size
#define UIM_SIZED				(0x0022)

//! widget changed position
//! sent after the widget moved
#define UIM_MOVED				(0x0023)

//! widget changed its content/state
#define UIM_CHANGED				(0x0024)



//------------------------------------------------------------------------------------------------- 
// widget control
//------------------------------------------------------------------------------------------------- 
//------------------------------------------------------------------------------------------------- 

//! change size
//! wParam should contain new floating point width
//! lParam should contain new floating point height
#define UIM_SIZE				(0x0100)

//! change position
//! wParam should contain new floating x coordinate relative to parent
//! lParam should contain new floating y coordinate relative to parent
#define UIM_MOVE				(0x0101)

//! widget should draw itself
//! wParam contains the rendering pass [0..1]
//! lParam should be 0
#define UIM_DRAW				(0x0102)

//! widget should init
//! wParam should be 0
//! lParam should be 0
#define UIM_INIT				(0x0103)

//! call widget OnCommand		
//! wParam should be 0
//! lParam should be 0
#define UIM_COMMAND				(0x0104)

//! widget should destroy itself
//! wParam should be 0
//! lParam should be 0
#define UIM_DESTROY				(0x0105)


//-------------------------------------------------------------------------------------------------