#include "stdafx.h"
#include <IEntitySystem.h>
#include "Game.h"
#include "Synched2DTable.h"									// CSynched2DTable
#include "XPlayer.h"												// CPlayer
#include "Cry_Math.h"												// TMatrix_tpl
#include <algorithm>

CSynched2DTable::CSynched2DTable(CXGame *pGame)
{
	m_pScriptObject=NULL;
	m_pGame=pGame;
}

//////////////////////////////////////////////////////////////////////////
// Initialize the AdvCamSystem-container.
bool CSynched2DTable::Init()
{
	IEntity *entity = GetEntity();
	entity->GetScriptObject()->SetValue("type", "Synched2DTable");

	entity->SetNeedUpdate(true);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CSynched2DTable::Update()
{
	if(!GetISystem()->GetIGame()->GetModuleState(EGameServer))
		return;	// only needed on the server, does nothing on the client

	IXGame *pXGame = GetIXGame( GetISystem()->GetIGame() );			assert(pXGame);
	CXServer *pXServer = pXGame->GetServer();										assert(pXServer);

	IServer *pIServer = pXServer->m_pIServer;										assert(pIServer);

	// check for disconnected clients
	{
		TDirtyListsMap::iterator itMap;

		for(itMap=m_ServerslotDirtylist.begin();itMap!=m_ServerslotDirtylist.end();)
		{
			uint8 ucId=itMap->first;

			if(!pIServer->GetServerSlotbyID(ucId))
			{
				// disconnected client
				m_ServerslotDirtylist.erase(itMap++);
			}
			else
				++itMap;
		}
	}

	// check for new clients
	{
		// use uint32 not uin8 otherwise the for loop would not be able to teminate if we have 256 players
		uint32 dwMaxClientID = (uint32)pIServer->GetMaxClientID();

		for(uint32 dwId=0;dwId<=dwMaxClientID;++dwId)
		{
			IServerSlot *pSlot=pIServer->GetServerSlotbyID(dwId);

			if(!pSlot)
				continue;		// disconnected

			if(!m_ServerslotDirtylist.count(dwId))
				InsertServerSlot(dwId);			// new connected client
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void CSynched2DTable::OnSetAngles( const Vec3d &ang )
{
}


//////////////////////////////////////////////////////////////////////////
IScriptObject *CSynched2DTable::GetScriptObject()
{
	return m_pScriptObject;
}


//////////////////////////////////////////////////////////////////////////
void CSynched2DTable::SetScriptObject(IScriptObject *object)
{
	m_pScriptObject=object;
}


//////////////////////////////////////////////////////////////////////////
// Save upcast.
bool CSynched2DTable::QueryContainerInterface(ContainerInterfaceType desired_interface, void **ppInterface )
{
/*	if (desired_interface == CIT_IADVCAMSYSTEM)
	{
		*ppInterface = (void *) this;
		return true;
	}
	else
*/	
	{
		*ppInterface = 0;
		return false;
	}	
}

void CSynched2DTable::GetEntityDesc( CEntityDesc &desc ) const
{
}


bool CSynched2DTable::STableEntry::Write( CStream &stm )
{
	bool bFloat = m_fValue!=FLT_MAX;

	stm.Write(bFloat);

	if(bFloat)
		return stm.Write(m_fValue);
	 else
		return stm.Write(m_sValue);
}

bool CSynched2DTable::STableEntry::Read( CStream &stm )
{
	bool bFloat;

	if(!stm.Read(bFloat))
		return false;

	if(bFloat)
		return stm.Read(m_fValue);
	else
	{
		m_fValue=FLT_MAX;

		return stm.Read(m_sValue);
	}
}


bool CSynched2DTable::Write(CStream &stm,EntityCloneState *cs)
{
	assert(cs);

	uint8 ucClientId = cs->m_pServerSlot->GetID();

	// true=send new packet, false=resend old packet
  bool bSendNewPacket = cs->m_pServerSlot->OccupyLazyChannel();
	bool SendOverLazyChannel=cs->m_pServerSlot->ShouldSendOverLazyChannel();

	if(!SendOverLazyChannel)
	{
		if(!stm.Write(false))		// last one
			return false;

		return true;
	}

	TDirtyLists &list = m_ServerslotDirtylist[ucClientId].m_DirtyList;

	if(m_ServerslotDirtylist[ucClientId].m_bPendingPacket && bSendNewPacket)
	{
		// got acknowledge from client so remove this
		if(!list.empty())//[MG]pop_front remark: first element must not be empty, had a crash here, so trying to fix it with this...
			list.pop_front();

		// todo: remove
//		GetISystem()->GetILog()->Log("CSynched2DTable got acknowledge from client so remove this");
	}

	if(!list.empty())
	{
		SDirtyItem &item = list.front();

		m_ServerslotDirtylist[ucClientId].m_bPendingPacket=true;

		if(!stm.Write(true))			// one item
			return false;

		bool bServerLazyState= cs->m_pServerSlot->GetServerLazyChannelState();

		// todo: remove
//		GetISystem()->GetILog()->Log("bServerLazyState write %s",bServerLazyState?"true":"false");

		if(!stm.Write(bServerLazyState))
			return false;

		if(!stm.Write(item.m_ucX))
			return false;

		if(!stm.Write(item.m_ucY))
			return false;

		if(item.m_ucX==0xff)
		{
			// whole line
			uint32 dwColumnCount=m_EntryTable.GetColumnCountY(item.m_ucY);

			// todo: remove
//			GetISystem()->GetILog()->Log("CSynched2DTable ucColumnCount write %d %d",(int)item.m_ucY,dwColumnCount);

			if(!stm.Write((uint8)dwColumnCount))
				return false;

			for(uint32 dwX=0;dwX<dwColumnCount;++dwX)
			{
				STableEntry Value = m_EntryTable.GetXY(dwX,item.m_ucY);

				if(!Value.Write(stm))
					return false;
			}
		}
		else
		{
			// one item
			STableEntry Value = m_EntryTable.GetXY(item.m_ucX,item.m_ucY);

			if(!Value.Write(stm))
				return false;
		}
	}

	if(!stm.Write(false))		// last one
		return false;

	return true;
}



bool CSynched2DTable::Read(CStream &stm)
{
	for(;;)
	{
		bool bItem;

		if(!stm.Read(bItem))			// if there are items
			return false;

		if(!bItem)
			break;

		bool bServerLazyState;

		if(!stm.Read(bServerLazyState))
			return false;

		// todo: remove
//		GetISystem()->GetILog()->Log("bServerLazyState read %s",bServerLazyState?"true":"false");

		bool bIgnoreResentPacket=false;

		if(m_pGame->GetClient()->GetLazyChannelState()==bServerLazyState)
			bIgnoreResentPacket=true;
		else
		{
			m_pGame->GetClient()->LazyChannelAcknowledge();

			// todo: remove
//			GetISystem()->GetILog()->Log("LazyChannelAcknowledge ->%s",m_pGame->GetClient()->GetLazyChannelState()?"true":"false");
		}

		uint8 ucX,ucY;

		if(!stm.Read(ucX))
			return false;

		if(!stm.Read(ucY))
			return false;

		if(ucX==0xff) // (0xff is used to mark the whole line dirty)
		{
			// one line
			uint8 ucColumnCount;

			if(!stm.Read(ucColumnCount))
				return false;

			// todo: remove
//			GetISystem()->GetILog()->Log("CSynched2DTable ucColumnCount read %d %d",(int)ucY,(int)ucColumnCount);

			for(uint32 dwX=0;dwX<(uint32)ucColumnCount;++dwX)
			{
				STableEntry Value;

				if(!Value.Read(stm))
					return false;

				// todo: remove
//				GetISystem()->GetILog()->Log("CSynched2DTable receive %d %d %.2f",(int)dwX,(int)ucY,fValue);

				if(!bIgnoreResentPacket)
					m_EntryTable.SetXY(dwX,ucY,Value);
			}
		}
		else
		{
			// one entry
			STableEntry Value;

			if(!Value.Read(stm))
				return false;

			// todo: remove
//			GetISystem()->GetILog()->Log("CSynched2DTable receive %d %d %.2f",(int)ucX,(int)ucY,fValue);

			if(!bIgnoreResentPacket)
				m_EntryTable.SetXY(ucX,ucY,Value);
		}
	}

	return true;
}




void CSynched2DTable::OnEntityNetworkUpdate( const EntityId &idViewerEntity, const Vec3d &v3dViewer, uint32 &inoutPriority, 
																						EntityCloneState &inoutCloneState) const
{
	inoutCloneState.m_bSyncAngles=false;
	inoutCloneState.m_bSyncYAngle=false;
	inoutCloneState.m_bSyncPosition=false;

	// todo: set inoutPriority set based on not synched data count

	inoutPriority=0xffff;
}


void CSynched2DTable::MarkDirty( const uint8 ucPlayerId )
{
	TDirtyLists &list=m_ServerslotDirtylist[ucPlayerId].m_DirtyList;

	list.clear();

	uint32 dwLines=m_EntryTable.m_Lines.size();

	for(uint32 dwI=0;dwI<dwLines;++dwI)
		list.push_back(SDirtyItem(dwI));
}


void CSynched2DTable::MarkDirtyXY( const uint8 ucX, const uint8 ucY, const uint8 ucPlayerId )
{
	TDirtyLists &list=m_ServerslotDirtylist[ucPlayerId].m_DirtyList;

	SDirtyItem newEntry(ucX,ucY);

	if(std::find(list.begin(),list.end(),newEntry) == list.end())		// if if wasn't in the list already
		list.push_back(newEntry);
}


void CSynched2DTable::MarkDirtyY( const uint8 ucY, const uint8 ucPlayerId )
{
	TDirtyLists &list=m_ServerslotDirtylist[ucPlayerId].m_DirtyList;

	SDirtyItem newEntry(0xff,ucY);

	if(std::find(list.begin(),list.end(),newEntry) == list.end())		// if if wasn't in the list already
		list.push_back(newEntry);
}


void CSynched2DTable::MarkDirtyXY( const uint8 ucX, const uint8 ucY )
{
	TDirtyListsMap::iterator itMap;

	for(itMap=m_ServerslotDirtylist.begin();itMap!=m_ServerslotDirtylist.end();++itMap)
	{
		uint8 ucId=itMap->first;

		MarkDirtyXY(ucX,ucY,ucId);
	}
}


void CSynched2DTable::MarkDirtyY( const uint8 ucY )
{
	TDirtyListsMap::iterator itMap;

	for(itMap=m_ServerslotDirtylist.begin();itMap!=m_ServerslotDirtylist.end();++itMap)
	{
		uint8 ucId=itMap->first;

		MarkDirtyY(ucY,ucId);
	}
}



void CSynched2DTable::SetEntryXYFloat( const uint32 uiX, const uint32 uiY, const float fValue )
{
	if(!GetISystem()->GetIGame()->GetModuleState(EGameServer))
	{
		assert(0);	// only call this on the server
		return;			// to prevent crash
	}

	const STableEntry OldValue = m_EntryTable.GetXY(uiX,uiY);

	// apply only changed values
	if(!OldValue.IsFloat() || OldValue.GetFloat()!=fValue)
	{
		m_EntryTable.SetXY(uiX,uiY,fValue);
		MarkDirtyXY(uiX,uiY);
	}
}


void CSynched2DTable::SetEntryXYString( const uint32 uiX, const uint32 uiY, const string &sValue )
{
	if(!GetISystem()->GetIGame()->GetModuleState(EGameServer))
	{
		assert(0);	// only call this on the server
		return;			// to prevent crash
	}

	const STableEntry OldValue = m_EntryTable.GetXY(uiX,uiY);

	// apply only changed values
	if(OldValue.IsFloat() || OldValue.GetString()!=sValue)
	{
		m_EntryTable.SetXY(uiX,uiY,sValue.c_str());
		MarkDirtyXY(uiX,uiY);
	}
}



void CSynched2DTable::SetEntriesYFloat( const uint32 uiY, const float fValue )
{
	if(!GetISystem()->GetIGame()->GetModuleState(EGameServer))
	{
		assert(0);	// only call this on the server
		return;			// to prevent crash
	}

	uint32 dwColumns = m_EntryTable.GetColumnCountY(uiY);

	for(uint32 iX=0;iX<dwColumns;++iX)
		m_EntryTable.SetXY(iX,uiY,fValue);

	MarkDirtyY(uiY);
}



void CSynched2DTable::SetEntriesYString( const uint32 uiY, const string &sValue )
{
	if(!GetISystem()->GetIGame()->GetModuleState(EGameServer))
	{
		assert(0);	// only call this on the server
		return;			// to prevent crash
	}

	uint32 dwColumns = m_EntryTable.GetColumnCountY(uiY);

	for(uint32 iX=0;iX<dwColumns;++iX)
		m_EntryTable.SetXY(iX,uiY,sValue.c_str());

	MarkDirtyY(uiY);
}

void CSynched2DTable::InsertServerSlot( const uint8 ucId )
{
	m_ServerslotDirtylist[ucId]=SDirtylist();		// create element
	MarkDirty(ucId);
}







uint32 CSynched2DTable::GetLineCount() const
{
	return m_EntryTable.m_Lines.size();
}


uint32 CSynched2DTable::GetColumnCount() const
{
	return m_EntryTable.GetColumnCount();
}

