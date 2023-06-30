#include "CdKeyValidation.h"
#include "GSCDKeyInterface.h"
#include "UbisoftMemory.h"
#define CD_KEY_VALIDATION_PORT 43421

CDKeyValidation::CDKeyValidation()
{
	classHandle=0;
}

CDKeyValidation::~CDKeyValidation()
{
}

// Server & client
void CDKeyValidation::Initialize(void)
{
	classHandle=GSCDKey_Initialize(CD_KEY_VALIDATION_PORT);
}

void CDKeyValidation::Uninitialize(void)
{
	GSCDKey_Uninitialize(classHandle);
}

// ------- SERVER FUNCTIONS ---------------
// The server should call this to validate an authorization ID
GSCDKeyRequest CDKeyValidation::ValidateUser(GSchar szIPAddress[IPADDRESSLENGTH], GSushort usPort, GSubyte *auhorizationID,GSchar *gameName)
{
	/*
	*	GSCDKeyRequest __stdcall GSCDKey_ValidateUser(GShandle pGSCDKeyHandle,
	PVALIDATION_SERVER_INFO psValidationServerInfo,
	GSubyte *pucAuhorizationID,GSchar *szGameName,
	GSushort usTimeout = 3);
	 */

	return 0;
}

// Server should call this everytime a player quits or for every player when the game ends.  That way that
// client can use their cd key in other games
// Yes I know it's a bad design :(
GSCDKeyRequest CDKeyValidation::ServerDisconnectUser(GSchar szIPAddress[IPADDRESSLENGTH], GSushort usPort,	GSubyte *authorizationID)
{
	/*
	GSCDKeyRequest __stdcall GSCDKey_DisconnectUser(GShandle pGSCDKeyHandle,
		PVALIDATION_SERVER_INFO psValidationServerInfo,
		GSubyte *pucAuhorizationID);
		*/

	return 0;
}

// ------- CLIENT FUNCTIONS ---------------
// Client needs to call this once to get an Activation ID.  You never need to call this more than once.
// The activation ID will be stored in the registry
GSCDKeyRequest CDKeyValidation::ActivateCDKey(GSchar szIPAddress[IPADDRESSLENGTH], GSushort usPort, PACTIVATION_INFO activationInfo)
{
	/*
	*	GSCDKeyRequest __stdcall GSCDKey_RequestActivation(GShandle pGSCDKeyHandle,
	PVALIDATION_SERVER_INFO psValidationServerInfo,
	PACTIVATION_INFO psActivationInfo,
	GSushort usTimeout = 6);
	 */

	return 0;
}

// This uses the activation ID to get an AUTHORIZATION ID for a game server.  You need to call this
// before each time you play.  You then send the AUTHORIZATION ID internally to the server
GSCDKeyRequest CDKeyValidation::AuthorizeActivationID(GSchar szIPAddress[IPADDRESSLENGTH], GSushort usPort,PVALIDATION_INFO validationInfo)
{
	/*
	GSCDKeyRequest __stdcall GSCDKey_RequestAuthorization(GShandle pGSCDKeyHandle,
	PVALIDATION_SERVER_INFO psValidationServerInfo,
	PVALIDATION_INFO psValidationInfo,
	GSushort usTimeout = 3);
	*/

	return 0;
}