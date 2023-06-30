#ifndef __NETINTERFACE_H__
#define __NETINTERFACE_H__

#include "GSTypes.h"

enum INTERFACE_TYPE {
    IT_LAN,
    IT_INTERNET,
    IT_ALL
};

//===================================================================================================
// MEMBER:       GetNetInterface
// AUTHOR:       Pierre-Luc Rigaux
// CREATION:     May 2001
//
// DESCRIPTION:  Get the local IP Address and the Net Mask of the first Eternet device
//===================================================================================================
// INPUT:  null
// OUPUTS: IP Address and netmask in a string format
// RESULT: True on success, false else
// known bug: this funtion takes the first ip interface (we don't know if it is the good one)
//===================================================================================================
GSbool GetNetInterface(GSchar* szIPAddress, GSchar* szNetMask,
                       INTERFACE_TYPE type = IT_ALL, GSuint uiIndex = 0);

//===================================================================================================
// MEMBER:       ResolveBroadcast
// AUTHOR:       Guillaume Plante
// CREATION:     May 2001
//
// DESCRIPTION:  Resolve the broadcast address on the network from the netmask and local ip
//               that has been specified by the user. If they are valid, the broadcast is resolve
//               with these ip but if they arent, the function tries to resolve it with the
//               detected local ip address and net mask, in this case the function return false.
//===================================================================================================
// INPUT:  szLocalAddrees = local address in a string format
// INPUT:  szNetmask = local netmask in a string format
// OUPUTS: szBroadcastAddress : network broadcast ip address in a string format
// RESULT: True on success, false else
//===================================================================================================
GSbool ResolveBroadcast(GSchar *szLocalAddress,GSchar *szNetmask,GSchar *szBroadcastAddress);

//===================================================================================================
// MEMBER:       ResolveBroadcast
// AUTHOR:       Guillaume Plante
// CREATION:     May 2001
//
// DESCRIPTION:  Check if a ip address is valid by comparing it with each of the detected
//               address on the local machine.
//===================================================================================================
// INPUT:  szLocalAddrees = local address in a string format
// RESULT: True on success, false else
//===================================================================================================
GSbool IPIsValid(GSchar *szIPAddress);


#endif
