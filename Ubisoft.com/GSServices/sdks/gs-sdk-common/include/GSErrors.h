#ifndef _GSERRORS_H
#define _GSERRORS_H

#include "GSTypes.h"

/////////////////////////////////////////////////////////////////////////////////////
// GSRESULT TYPE

//	It is divided in 3 parts:
//		Severity(bit 31):		1 = error, 0 = success
//		Facility(bit 16 to 30):	identify the system responsible of the error
//		ID(bit 0 to 15):		unique number that represent an error/warning/success
typedef GSint GSRESULT;

/////////////////////////////////////////////////////////////////////////////////////
// MACRO USED TO CHECK GSRESULT

// Return "TRUE" if the GSRESULT is an error
#define GSFAILED(GSR)				((GSRESULT)(GSR)<(GSint)0)
// Return "TRUE" if the GSRESULT is a success
#define GSSUCCEEDED(GSR)			((GSRESULT)(GSR)>=(GSint)0)
// Return the error or success code of a GSRESULT
#define GSRESULT_CODE(GSR)			((GSint)(GSR&0x8000FFFFL))

/////////////////////////////////////////////////////////////////////////////////////
// SUCCESS CODES
#define GSS_OK									((GSint) 0x00000000)			
#define GSS_FALSE								((GSint) 0x00000001)			
#define GSS_TIMEOUT								((GSint) 0x00005000)

#define GSS_KEYBUFFERTOOSMALL					((GSint) 0x00006200)
#define GSS_BUFFERTOOSMALL						((GSint) 0x00006201)
#define GSS_TRIGGERDETECTED						((GSint) 0x00006202)
	
/////////////////////////////////////////////////////////////////////////////////////
// ERROR CODES
#define GSE_ACCESSDENIED						((GSint) 0x80070005)
#define GSE_HANDLE								((GSint) 0x80070006)
#define GSE_OUTOFMEMORY							((GSint) 0x8007000E)
#define GSE_INVALIDARG							((GSint) 0x80070057)
#define GSE_NOTIMPL 							((GSint) 0x80004001)
#define GSE_NOINTERFACE							((GSint) 0x80004002)
#define GSE_POINTER								((GSint) 0x80004003)
#define GSE_ABORT								((GSint) 0x80004004)
#define GSE_FAIL								((GSint) 0x80004005)
#define GSE_NOTGSMODULE							((GSint) 0x80004FFF)
#define GSE_UNEXPECTED							((GSint) 0x8000FFFF)
#define GSE_ALREADYINITIALIZED					((GSint) 0x80005000)

#define GSE_NOTINITIALIZED						((GSint) 0x80005001)	
#define GSE_CANTFINDAPPLICATION					((GSint) 0x80005002)	
#define GSE_CANTLOAD							((GSint) 0x80005003)
#define GSE_TIMEOUT								((GSint) 0x80005004)

#define GSE_BADMODE								((GSint) 0x80006000)
#define GSE_GSISALREADYUSEDBYOTHERGAME			((GSint) 0x80006001)
#define GSE_GAMEALREADYPRELOADED				((GSint) 0x80006100)
#define GSE_STATENOTSUPPORTED					((GSint) 0x80006101)
#define GSE_INVALIDGAMENAME						((GSint) 0x80006102)
#define GSE_NODATACONTAINER						((GSint) 0x80006103)
#define GSE_MESSAGENOTSUPPORTEDINCURRENTSTATE	((GSint) 0x80006104)
#define GSE_INVALIDKEY							((GSint) 0x80006200)
#define GSE_KEYALREADYEXIST						((GSint) 0x80006201)
#define GSE_BUFFERNOTVALID						((GSint) 0x80006202)
#define GSE_INVALIDINDEX						((GSint) 0x80006203)
#define GSE_NOTMASTER							((GSint) 0x80006204)
#define GSE_INVALIDEVENT						((GSint) 0x80006205)
#define GSE_MATCHNOTSTARTEDBYMASTER				((GSint) 0x80006206)
#define GSE_NOREPLY								((GSint) 0x80006207)
#define GSE_GAMENOTINITIATED					((GSint) 0x80006208)
#define GSE_MATCHNOTFINISHED					((GSint) 0x80006209)
#define GSE_MATCHNOTEXIST						((GSint) 0x8000620A)
#define GSE_MATCHSCORESSUBMISSIONALREDYSENT		((GSint) 0x8000620B)
#define GSE_MATCHSCORESSUBMISSIONFAIL			((GSint) 0x8000620C)
#define GSE_DETECTEDNEWVERSION					((GSint) 0x8000620D)
#define GSE_OTHERENDOFPIPECLOSED				((GSint) 0x8000620E)
#define GSE_SOCKETINVALID						((GSint) 0x8000620F)
#define GSE_OPENFILE							((GSint) 0x80006210)
#define GSE_CONNECTERROR						((GSint) 0x80006211)
#define GSE_CURRENTROOMDESTROYED				((GSint) 0x80006212)
#define GSE_SOCKETERROR							((GSint) 0x80006213)
#define GSE_HOSTUNREACHABLE						((GSint) 0x80006214)
#define GSE_ENDOFSTREAM							((GSint) 0x80006215)
#define GSE_ALREADYALLOCATED					((GSint) 0x80006216)
#define GSE_NOTALLOCATED						((GSint) 0x80006217)
#define GSE_INPROGRESS							((GSint) 0x80006218)
#define GSE_DATADOESNTEXIST						((GSint) 0x80006219)
#define GSE_INVALIDUSER							((GSint) 0x8000621A)
#define GSE_INVALIDPWD							((GSint) 0x8000621B)
#define GSE_INVALIDGUID							((GSint) 0x8000621C)
#define GSE_INVALIDPACKAGE						((GSint) 0x8000621D)
#define GSE_INVALIDXML							((GSint) 0x8000621E)
#define GSE_INVALIDCHUNK						((GSint) 0x8000621F)
#define GSE_XCEEDZIP							((GSint) 0x80006220)
#define GSE_DBFAILURE                           ((GSint) 0x80006221)
#define GSE_OUTOFBOUND                          ((GSint) 0x80006222)
#define GSE_BADARG                              ((GSint) 0x80006223)

#endif _GSERRORS_H
