// Check this in OnConnect in XServerSlot

#if defined(WIN32)
#	define GS_WIN32
#else
#	define GS_LINUX
#endif

#include "GSTypes.h"
#include "define.h"
#include "GSCDKeyDefines.h"

#ifndef __CD_KEY_VALIDATION__
#define __CD_KEY_VALIDATION__

class CDKeyValidation
{
public:
	CDKeyValidation();
	~CDKeyValidation();

	// Server & client
	void Initialize(void);
	void Uninitialize(void);

	// ------- SERVER FUNCTIONS ---------------
	// The server should call this to validate an authorization ID
	//GSCDKeyRequest ValidateUser(PVALIDATION_SERVER_INFO validationServerInfo, GSubyte *auhorizationID,GSchar *gameName);
	GSCDKeyRequest ValidateUser(GSchar szIPAddress[IPADDRESSLENGTH], GSushort usPort, GSubyte *auhorizationID,GSchar *gameName);

	// Server should call this everytime a player quits or for every player when the game ends.  That way that
	// client can use their cd key in other games
	// Yes I know it's a bad design :(
	GSCDKeyRequest ServerDisconnectUser(GSchar szIPAddress[IPADDRESSLENGTH], GSushort usPort, GSubyte *authorizationID);

	// ------- CLIENT FUNCTIONS ---------------
	// Client needs to call this once to get an Activation ID.  You never need to call this more than once.
	// The activation ID will be stored in the registry
	GSCDKeyRequest ActivateCDKey(GSchar szIPAddress[IPADDRESSLENGTH], GSushort usPort, PACTIVATION_INFO activationInfo);
	// This uses the activation ID to get an AUTHORIZATION ID for a game server.  You need to call this
	// before each time you play.  You then send the AUTHORIZATION ID internally to the server
	GSCDKeyRequest AuthorizeActivationID(GSchar szIPAddress[IPADDRESSLENGTH], GSushort usPort,PVALIDATION_INFO validationInfo);

private:
	void Update(void);
	GShandle classHandle;
};

#endif