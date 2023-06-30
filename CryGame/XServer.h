//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XServer.h
//  Description: XServer class.
//
//  History:
//  - August 3, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_XSERVER_H
#define GAME_XSERVER_H

#if _MSC_VER > 1000
# pragma once
#endif

#include "XNetwork.h"
#include "XServerRules.h"
#include "XSnapshot.h"
#include "INetwork.h"					// IServerSlotFactory
#include "ScriptObjectServer.h"
#include <map>
#include <IConsole.h>

#include "XServerSlot.h"			// XServerSlot

// Broadcast flags organisation (from low byte to top):
// 32 bits  -  16 bits - Main flags
//              8 bits - Message
//				8 bits - Id of a slot (id 0 is the localhost)
#define BROADCAST_RELIABLE				0x0000								// Send a reliable packet
#define BROADCAST_UNRELIABLE			0x0001								// Send an unreliable packet
#define BROADCAST_EXCLUDE					0x0002								// Exclude the specified slots
#define BROADCAST_MESSAGE					0x0004								// Add a message at the begining of the stream
#define BROADCAST_SLOTID(id)			(((BYTE)(id))<<16)		// Specify a slot
#define BROADCAST_MSG(msg)				(((BYTE)(msg))<<24)		// Specify a message

#define GETBROADCAST_SLOTID(data)	((BYTE)(((data)>>16)&0xFF))
#define GETBROADCAST_MSG(data)		((BYTE)(((data)>>24)&0xFF))

#define MAXPLAYERS_LIMIT					(32)


typedef std::multimap<string, ITagPoint *>	RespawnPointMap;


class BannedID
{
public:
	BannedID() { memset(vBanID, 0, 64); bSize = 0; };
	BannedID(const unsigned char *vByteArr, unsigned char bArrSize, const string &szPlayerName) { memset(vBanID, 0, 64); memcpy(vBanID, vByteArr, min(bArrSize, 64)); szName = szPlayerName; bSize = bArrSize; };
	BannedID(const BannedID &NewBannedID) { szName = NewBannedID.szName; memset(vBanID, 0, 64); memcpy(vBanID, NewBannedID.vBanID, NewBannedID.bSize); bSize = NewBannedID.bSize; };
	virtual ~BannedID() {};

	bool operator ==(const BannedID &arg) const {
		if (bSize != arg.bSize)
			return 0;
		return (memcmp(arg.vBanID, vBanID, bSize) == 0);
	}
	bool operator !=(const BannedID &arg) const {
		if (bSize != arg.bSize)
			return 1;
		return (memcmp(arg.vBanID, vBanID, bSize) != 0);
	}

	unsigned char	vBanID[64];
	unsigned char bSize;
	string				szName;
};


typedef std::vector<BannedID>					BannedIDList;
typedef BannedIDList::iterator				BannedIDListItor;

typedef std::vector<unsigned int>			BannedIPList;
typedef BannedIPList::iterator				BannedIPListItor;


///////////////////////////////////////////////
/*! this class represent a client on the server-side.This mean that 
for every connected client a respective CXServerSlot exists.
*/

class CXServer : public IServerSlotFactory, public IEntitySystemSink, public IServerSecuritySink
{
	// the respawn points
	RespawnPointMap m_vRespawnPoints;
	RespawnPointMap::iterator	m_CurRespawnPoint;

public:
  typedef std::map<BYTE, CXServerSlot*>		XSlotMap;	

	//! constructor
	CXServer(class CXGame *pGame, WORD nPort, const char *szName, bool listen);
	//! destructor
	virtual ~CXServer();
	//!
	void DrawNetStats(IRenderer *pRenderer);

	//! /return true=server creation was successful, false otherwise
	bool IsOK() { return m_bOK; }

	//! /return pointer to the ServerSlot or 0 if there is no server slot for this id
	CXServerSlot *GetServerSlotByEntityId( const EntityId inId ) const
	{
		for(XSlotMap::const_iterator itor=m_mapXSlots.begin();itor!=m_mapXSlots.end();++itor)
		{
			CXServerSlot *slot=itor->second;

			if(slot->GetPlayerId()==inId)
				return itor->second;
		}

		return 0;
	}

	//! /return pointer to the ServerSlot or 0 if there is no server slot for this id
	CXServerSlot *GetServerSlotByIP( unsigned int clientIP ) const;

	// IServerSlotFactory /////////////////////////////////
	virtual bool CreateServerSlot(IServerSlot *pIServerSlot);
	virtual bool GetServerInfoStatus(string &szServerStatus);
	virtual bool GetServerInfoStatus(string &szName, string &szGameType, string &szMap, string &szVersion, bool *pbPassword, int *piPlayers, int *piMaxPlayers);
	virtual bool GetServerInfoRules(string &szServerRules);
	virtual bool GetServerInfoPlayers(string *vszStrings[4], int &nStrings);
	virtual bool ProcessXMLInfoRequest( const char *sRequest,const char *sRespone,int nResponseMaxLength );

	///////////////////////////////////////////////////////
	// IEntitySystemSink /////////////////////////////////
	void OnSpawnContainer( CEntityDesc &ed,IEntity *pEntity );
	void OnSpawn(IEntity *ent,  CEntityDesc &ed);
	void OnRemove(IEntity *ent);
	void OnBind(EntityId id,EntityId child,unsigned char param)
	{
		BindEntity(id,child,param);
	}
	void OnUnbind(EntityId id,EntityId child,unsigned char param)
	{
		UnbindEntity(id,child,param);
	}
	///////////////////////////////////////////////////////

	void RegisterSlot(CXServerSlot *pSlot);

	//! is called by the XServerSlot itself during destruction
	//! don't call from anywhere else - a call cycle is likely 
	void UnregisterXSlot(DWORD nClientID);

	void ClearSlots();
    int GetNumPlayers();

	void OnClientMsgText(EntityId sender,CStream &stm);

	void Update();
	void UpdateXServerNetwork();


	// Message broadcast
	void BroadcastUnreliable(XSERVERMSG msg, CStream &stmIn,int nExclude=-1);
	virtual void BroadcastReliable(XSERVERMSG msg, CStream &stmIn,bool bSecondaryChannel);

	void BroadcastText(const char *sText, float fLifeTime = DEFAULT_TEXT_LIFETIME);
	void BroadcastCommand(const char *sCmd);
	void BroadcastCommand(const char *sCmd, const Vec3 &invPos, const Vec3 &invNormal, const EntityId inId, const unsigned char incUserByte );

	void BindEntity(EntityId idParent,EntityId idChild,unsigned char cParam);
	void UnbindEntity(EntityId idParent,EntityId idChild,unsigned char cParam);
	void SyncVariable(ICVar *p);
	void SyncAIState(void);
	XSlotMap &GetSlotsMap(){ return m_mapXSlots; }

	// Infos retriving
	const char *GetName()	{ return sv_name->GetString(); }
	const WORD  GetPort()	{ return m_ServerInfos.nPort; }

	// return the current context
	bool GetContext(SXGameContext &ctxOut);

	void	AddRespawnPoint(ITagPoint *pPoint);
	void	RemoveRespawnPoint(ITagPoint *pPoint);
	// get a random respawn point
	ITagPoint* GetFirstRespawnPoint();
	ITagPoint* GetNextRespawnPoint();
	ITagPoint* GetPrevRespawnPoint();
  ITagPoint* GetRespawnPoint(char *name);
	ITagPoint* GetRandomRespawnPoint(const char *sFilter=NULL);

	//!
	void AddToTeam(const char *sTeam,int eid);
	//!
	void RemoveFromTeam(int eid);
	//!
	void AddTeam(const char *sTeam);
	//!
	void RemoveTeam(const char *sTeam);
	//!
	void SetTeamScore(const char *sTeam,int score);
	//! request a script hash value from all connected clients (return-packet is then verified)
	//! - used for lua cheat protection
	//! \param Entity entity id, INVALID_WID for globals
	//! \param szPath e.g. "cnt.table1"
	//! \param szKey e.g. "luaFunc"
	void SendRequestScriptHash( EntityId Entity, const char *szPath, const char *szKey );
	//!
	void SetTeamFlags(const char *sTeam,int flags);
	//!
	unsigned MemStats();
	//!
	unsigned int GetSchedulingDelay();
	//!
	unsigned int GetMaxUpdateRate() const;
	//!
	void ClearRespawnPoints(){m_vRespawnPoints.clear();}
	//!
	CXServerRules* GetRules() { return &m_ServerRules; };
	//!
  void OnMapChanged();
	//!
	bool IsIDBanned(const BannedID &ID);
	//!
	void BanID(const BannedID &ID);
	//!
	void UnbanID(const BannedID &ID);
	//! \return true during destruction (to avoid recursive calls), false otherwise
	bool IsInDestruction() const;

	// interface IServerSecuritySink -------------------------------------------------------------

	virtual bool IsIPBanned(const unsigned int dwIP);
	virtual void BanIP(const unsigned int dwIP);
	virtual void UnbanIP(const unsigned int dwIP);
	virtual void CheaterFound( const unsigned int dwIP,int type,const char *sMsg );
	virtual bool GetSlotInfo(  const unsigned int dwIP,SSlotInfo &info , int nameOnly );

	// -------------------------------------------------------------------------------------------

	bool GetServerInfo();

	bool									m_bOK;										//!< true=server creation was successful, false otherwise
	bool									m_bListen;								//!< server accepts non-local connections
	struct IServer*				m_pIServer;								//!<

	struct IXSystem*			m_pISystem;								//!< The system interface

	struct ITimer *				m_pTimer;									//!< timer interface to avoid thousands of GetTimer()
	class CXGame *				m_pGame;									//!< the game

	// ID Generator
	
	SXServerInfos					m_ServerInfos;						//!< server infos
	SXGameContext					m_GameContext;						//!< the current game context
	CXServerRules					m_ServerRules;						//!< server rules
	
	// snapshot
	CScriptObjectServer		m_ScriptObjectServer;			//!<

	// console variables
	ICVar *								sv_name;									//!< server name (shown in the serverlist)
	ICVar *								sv_password;							//!< "" if not used
	ICVar *								sv_maxplayers;						//!<
	ICVar *								sv_maxrate;								//!< bitspersecond, Internet, maximum for all player, value is for one player
	ICVar *								sv_maxrate_lan;						//!< bitspersecond, LAN, maximum for all player, value is for one player
	ICVar *								sv_netstats;							//!<
	ICVar *								sv_max_scheduling_delay;	//!<
	ICVar *								sv_min_scheduling_delay;	//!<
	
	CXNetworkStats				m_NetStats;								//!< for network statistics (count and size per packet type)

	static const char *GetMsgName( XSERVERMSG inValue );

	bool									m_bIsLoadingLevel;				//!< true during loading of the level (used to disable synchronized spawning of entities and to make client waiting during that time)

	BannedIDList					m_vBannedIDList;					//!<
	BannedIPList					m_vBannedIPList;					//!<

	void SaveBanList(bool bSaveID = true, bool bSaveIP = true);
	void LoadBanList(bool bLoadID = true, bool bLoadIP = true);

private: // --------------------------------------------------------------------------

	XSlotMap							m_mapXSlots;							//!<
	bool									m_bInDestruction;					//!< only true during destruction (to avoid recursive calls)

	ICVar *								sv_maxupdaterate;					//!< is limiting the updaterate of the clients
};

#endif // GAME_XSERVER_H
