#include "stdafx.h"
#include "Entity.h"
#include "EntitySystem.h"
#include <stream.h>
#include <IScriptSystem.h>
#include <ITimer.h>
#include <ILog.h>
#include "StreamData.h"					// CStreamData_WorldPos

#ifdef _DEBUG
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif



	

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//PHYSIC or POS if change the pos
//ANGLES if change
//CONTAINER stuff
//
//
bool CEntity::Write(CStream& stm,EntityCloneState *cs)
{
	static CFrameProfiler profiler_EntityNetworkTraffic(GetISystem(),"Entity",PROFILE_NETWORK_TRAFFIC );
	static CFrameProfiler profiler_ContainerNetworkTraffic(GetISystem(),"Container",PROFILE_NETWORK_TRAFFIC );
	static CFrameProfiler profiler_PhysicsNetworkTraffic(GetISystem(),"Physics",PROFILE_NETWORK_TRAFFIC );

	int nSavedBytes = 0;
	int nStartBytes = stm.GetSize();
	CCustomProfilerSection netTrafficEntity(&profiler_EntityNetworkTraffic,&nSavedBytes);
	static CStream stmPhys;

//////////////////////////////////////
//PHYSIC OR POS
//////////////////////////////////////
	bool bNeedAngles=true;
  bool bAngles=false;
  bool bSyncYAngle=true;
	
	WRITE_COOKIE_NO(stm,0x55);

	if (m_bIsBound)
	{
		stm.Write(true);			// bound

		stm.Write(m_idBoundTo);
		stm.Write(m_cBind);

		//[kirill] need to sync angles of bound entities - they can rotate around parents
		Vec3d v3Angles=GetAngles( 1 );
		bAngles=true;
		if(cs)
		{
			bSyncYAngle=cs->m_bSyncYAngle;
			bAngles=cs->m_bSyncAngles;
			if(bAngles)
			{
//				if(!IsEquivalent(cs->m_v3Angles,v3Angles))
					cs->m_v3Angles=v3Angles;
//				 else
//					bAngles=false;
			}
		}
		else
		{
			bAngles=true;
		}
	////////////////////////////////////
		if(bAngles)
		{
			_VERIFY(stm.Write(true));
			// x
			stm.Write((unsigned short)((v3Angles.x*0xFFFF)*(1.f/360.f)));
			// y
			stm.Write(bSyncYAngle);
			if(bSyncYAngle)
				stm.Write((unsigned short)((v3Angles.y*0xFFFF)*(1.f/360.f))); // this component can be skipped for players
			// z
			stm.Write((unsigned short)((v3Angles.z*0xFFFF)*(1.f/360.f)));
		}
		else
		{
			//_VERIFY(stm.Write(false));
			if (!stm.Write(false))
			{			
				CryError("ENTITY %s (Type=%d)",GetName()?GetName():"UNNAMEDENTITY",(int)GetClassId());
			}
		}
	}
	else	// not bound
	{
		stm.Write(false);		// not bound

		bool bSyncPosition=true;

		if(cs)
			bSyncPosition=cs->m_bSyncPosition;

		bool bPhysics = HavePhysics() 
			&& m_physicEnabled 
			&& GetPhysics()->GetType()!=PE_STATIC 
			// [kirill] to enable DeadBodys saving - bug 429
			//		&& GetPhysics()->GetType()!=PE_ARTICULATED
			&& !IsBound();

		if (bPhysics && GetPhysics()->GetType()==PE_LIVING)
		{
			pe_player_dynamics pd;
			GetPhysics()->GetParams(&pd);
			if (!pd.bActive)
			{
				bPhysics = false;
				bSyncPosition = false;
			}
		}

		_VERIFY(stm.Write(bPhysics));
		if (bPhysics)
		{
			CDefaultStreamAllocator sa;
			CStream stmPhys(1024, &sa);
			int nSavedPhysicsBytes = 0;
			int nStartPhysicsBytes = stm.GetSize();
			CCustomProfilerSection netTrafficPhysics(&profiler_PhysicsNetworkTraffic,&nSavedPhysicsBytes);

			float fStepBack=0.0f;
			//all physical entities except the LIVING ENTITY(players) store 
			//the angles into the physical snapshot
			if(GetPhysics()->GetType()!=PE_LIVING)
			{
				bNeedAngles=false;
			}

			if(cs && cs->m_bLocalplayer)
			{
				stm.Write(true);
				fStepBack=cs->m_fWriteStepBack;
			}
			else
				stm.Write(false);

			stmPhys.Reset();
			int nSize;
			if (GetPhysics()->GetStateSnapshot(stmPhys,fStepBack,!cs || cs->m_bOffSync ? 0:ssf_checksum_only))
			{
				nSize = stmPhys.GetSize();
				stm.WritePkd(nSize);	
				stm.Write(stmPhys);
			}
			else
				stm.WritePkd(0);

	//int a = stm.GetSize();
	//TRACE("PHYSICS WRITE SIZE=%d %s", stm.GetSize()-a, GetName());
#ifdef DEBUG_BONES_SYNC
			if (m_pCryCharInstance[0] && m_pCryCharInstance[0]->GetCharacterPhysics())
				m_pCryCharInstance[0]->GetCharacterPhysics()->GetStateSnapshot(stm,0,0);
#endif

			nSavedPhysicsBytes = stm.GetSize() - nStartPhysicsBytes;
		}
		else
		{
			stm.Write(bSyncPosition);
			if (bSyncPosition)
			{
				Vec3d vPos = GetPos( false );
				_VERIFY(stm.WritePkd(CStreamData_WorldPos(vPos)));
			}
		}

		// if the entity has no physics, character might still have physics that needs to be saved (read: ropes)
		if (!GetPhysics())
		{
			IPhysicalEntity *pCharEnt;
			for (int iSlot=0; iSlot<m_nMaxCharNum; iSlot++) if (m_pCryCharInstance[iSlot])
				for(int iAuxPhys=0; pCharEnt=m_pCryCharInstance[iSlot]->GetCharacterPhysics(iAuxPhys); iAuxPhys++)
					pCharEnt->GetStateSnapshot(stm);
		}
	//////////////////////////////////////
	//ANGLES
	//////////////////////////////////////
		WRITE_COOKIE_NO(stm,0x21);
		Vec3d v3Angles=GetAngles( 1 );

		if(bNeedAngles)
		{
			bAngles=true;
			if(cs)
			{
				bSyncYAngle=cs->m_bSyncYAngle;
				bAngles=cs->m_bSyncAngles;

				if(bAngles)
				{
//					if(!IsEquivalent(cs->m_v3Angles,v3Angles))
						cs->m_v3Angles=v3Angles;
//					else
//						bAngles=false;
				}
			}
			else
			{
				bAngles=true;
			}
		}
	////////////////////////////////////
		if(bAngles)
		{
			_VERIFY(stm.Write(true));				// angles on

			// x
			stm.Write((unsigned short)((v3Angles.x*0xFFFF)*(1.f/360.f)));
			// y
			stm.Write(bSyncYAngle);
			if(bSyncYAngle)	
				stm.Write((unsigned short)((v3Angles.y*0xFFFF)*(1.f/360.f))); // this component can be skipped for players
			// z
			stm.Write((unsigned short)((v3Angles.z*0xFFFF)*(1.f/360.f)));
		}
		else
		{
			_VERIFY(stm.Write(false));			// angles off
		}
	}
	WRITE_COOKIE_NO(stm,0x90);

//////////////////////////////////////
//CONTAINER
//////////////////////////////////////
	bool bContainer=false;
	if (m_pContainer && m_pContainer->IsSaveable())	
		bContainer=true;
		
	_VERIFY(stm.Write(bContainer));

	if (bContainer)
	{
		int nSavedContainerBytes = 0;
		int nStartContainerBytes = stm.GetSize();
		CCustomProfilerSection netTrafficContainer(&profiler_ContainerNetworkTraffic,&nSavedContainerBytes);
		_VERIFY(m_pContainer->Write(stm,cs));
		nSavedContainerBytes = stm.GetSize() - nStartContainerBytes;
	}
	WRITE_COOKIE_NO(stm,0x91);

	nSavedBytes = stm.GetSize() - nStartBytes;
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


bool CEntity::Read(CStream& stm,bool bNoUpdate)
{
	bool bPhysics, bContainer,bBound,bSyncPosition;
//	pe_type pt;
	IPhysicalEntity *pPE=NULL;
	IEntity *pEntBound;
//////////////////////////////////////
//PHYSIC OR POS
//////////////////////////////////////
	VERIFY_ENTITY_COOKIE_NO(stm,0x55)

	stm.Read(bBound);
	if (bBound)
	{
		EntityId idBoundTo;
		unsigned char cBind;

		stm.Read(idBoundTo);
		stm.Read(cBind);

		if (!bNoUpdate)
		{
			if (m_bIsBound && m_idBoundTo!=idBoundTo)
			{
				pEntBound = m_pEntitySystem->GetEntity(m_idBoundTo);
				if (pEntBound)
					pEntBound->Unbind(GetId(),m_cBind);
			}
			if (!m_bIsBound)
			{
/*
				pEntBound = m_pEntitySystem->GetEntity(idBoundTo);
				if (pEntBound)
				{
					Vec3	pos = GetPos();
					// need to set position before OnBind is called (it can set some other position)
					pEntBound->Bind(GetId(),cBind, false, true);
//[kirill] this is not needed - when we load bound people, relative positions will be set in OnBind
//					SetPos( pos, false );
				}
				else
				//[kirill] if parent not spown yet - do bind on PostLoad
*/
				//[kirill] let's do all the bindings on PostLoad
				{
					m_idBoundTo = idBoundTo;
					m_cBind = cBind;
				}
			}
		}
		//[kirill] read angles for bound entity
	//////////////////////////////////////
	//ANGLES
	//////////////////////////////////////
		bool bAngles;
		stm.Read(bAngles);
		if(bAngles)
		{
			Vec3d vec;
			//_VERIFY(stm.Read(vec));
			bool bSyncYAngle;
			unsigned short x;
			unsigned short y=0;
			unsigned short z;
			stm.Read(x);
			stm.Read(bSyncYAngle);
			if(bSyncYAngle)
			{
				stm.Read(y);
			}
			stm.Read(z);
			vec.x=((float)x*360)*(1.f/0xFFFF);
			vec.y=((float)y*360)*(1.f/0xFFFF); // this component can be skipped for players
			vec.z=((float)z*360)*(1.f/0xFFFF);
			// My player entity should not accept angles from network.
			if (!bNoUpdate)
				SetAngles(vec, false,(pPE?(pPE->GetType()==PE_LIVING?false:true):false) );
		}
		VERIFY_ENTITY_COOKIE_NO(stm,0x90)
	}
	else
	{
		if (m_bIsBound && !bNoUpdate)
		{
			pEntBound = m_pEntitySystem->GetEntity(m_idBoundTo);
			if (pEntBound)
				pEntBound->Unbind(GetId(),m_cBind);
		}

		stm.Read(bPhysics);
		if (bPhysics)
		{
			// [anton] removed setting m_physicsEnabled, since static entities don't save physics, but they 
			// still have physics enabled; plus, just setting m_physicsEnabled to true or false will not actually enable/disable physics
			//m_physicEnabled = true;
			int nRet=0,nSize;
			long nPos;
	///int a = stm.GetSize();
			pPE=GetPhysics();
			/*if(!pPE)
			{								
				m_pISystem->GetILog()->LogError("ENTITY %s (Type=%d) was saved with physics state, but it doesnt have one",
					GetName() ? GetName():"UNNAMEDENTITY", (int)GetClassId());
				if (!m_pISystem->GetIGame()->IsMultiplayer())
				{
					CryError("Error: ENTITY %s (Type=%d) was saved with physics state, but it doesnt have one",GetName()?GetName():"UNNAMEDENTITY",(int)GetClassId());			
				}
				return false;
			}*/
			bool bHostEntity;
			stm.Read(bHostEntity);

			stm.ReadPkd(nSize);
			nPos = stm.GetReadPos()+nSize;
			if (nSize && pPE)
				nRet = pPE->SetStateFromSnapshot(stm,(bHostEntity ? ssf_compensate_time_diff:0) | (bNoUpdate ? ssf_no_update:0));
			stm.Seek(nPos);

			if (bHostEntity && !bNoUpdate)
			{
				pe_params_flags pf;
				pf.flagsAND = ~pef_update;
				pPE->SetParams(&pf);
			}

			if (!nRet)
			{
				m_pISystem->GetILog()->LogWarning("ENTITY %s (Type=%d) has incorrect physical entity", GetName() ? GetName():"UNNAMEDENTITY", (int)GetClassId());
				/*if (!m_pISystem->GetIGame()->IsMultiplayer())
				{
					CryError("ENTITY %s (Type=%d) has incorrect physical entity", GetName() ? GetName():"UNNAMEDENTITY", GetType());
				}
				return false;*/
			}

#ifdef DEBUG_BONES_SYNC
			if (m_pCryCharInstance[0] && m_pCryCharInstance[0]->GetCharacterPhysics())
			{
				pe_params_articulated_body pab;
				pab.pHost = 0;
				m_pCryCharInstance[0]->GetCharacterPhysics()->SetParams(&pab);
				m_pCryCharInstance[0]->GetCharacterPhysics()->SetStateFromSnapshot(stm,bNoUpdate ? ssf_no_update:0);
			}
#endif

	//TRACE("PHYSICS READ SIZE=%d %s", stm.GetSize()-a, GetName());
		}
		else
		{
			//m_physicEnabled = false;
			stm.Read(bSyncPosition);
			if (bSyncPosition) 
			{
				Vec3d vPos;
//			_VERIFY(stm.Read(vPos));
#if defined(LINUX)
				_VERIFY(stm.ReadPkd(*(IStreamData*)(&CStreamData_WorldPos(vPos))));
#else
				_VERIFY(stm.ReadPkd(CStreamData_WorldPos(vPos)));
#endif
				if (!bNoUpdate)
					SetPos(vPos, false);
			}
		}

		// if the entity has no physics, character might still have physics that needs to be saved (read: ropes)
		if (!GetPhysics())
		{
			IPhysicalEntity *pCharEnt;
			for (int iSlot=0; iSlot<m_nMaxCharNum; iSlot++) if (m_pCryCharInstance[iSlot])
				for(int iAuxPhys=0; pCharEnt=m_pCryCharInstance[iSlot]->GetCharacterPhysics(iAuxPhys); iAuxPhys++)
					pCharEnt->SetStateFromSnapshot(stm,bNoUpdate ? ssf_no_update:0);
		}
	//////////////////////////////////////
	//ANGLES
	//////////////////////////////////////
		VERIFY_ENTITY_COOKIE_NO(stm,0x21)
		bool bAngles;
		stm.Read(bAngles);
		if(bAngles)
		{
			Vec3d vec;
			//_VERIFY(stm.Read(vec));
			bool bSyncYAngle;
			unsigned short x;
			unsigned short y=0;
			unsigned short z;
			stm.Read(x);
			stm.Read(bSyncYAngle);
			if(bSyncYAngle)
			{
				stm.Read(y);
			}
			stm.Read(z);
			vec.x=((float)x*360)*(1.f/0xFFFF);
			vec.y=((float)y*360)*(1.f/0xFFFF); // this component can be skipped for players
			vec.z=((float)z*360)*(1.f/0xFFFF);
			// My player entity should not accept angles from network.
			if (!bNoUpdate)
				SetAngles(vec, false,(pPE?(pPE->GetType()==PE_LIVING?false:true):false) );
			VERIFY_ENTITY_COOKIE_NO(stm,0x90)
		}
		else
		{
			VERIFY_ENTITY_COOKIE_NO(stm,0x90)
		}
	}

//////////////////////////////////////
//CONTAINER
//////////////////////////////////////
	_VERIFY(stm.Read(bContainer));
	if (bContainer)
	{
		if (!m_pContainer)
		{			
			// since there is no check at all for errors during saving/loading,
			// at least print some information if it crashes			
			m_pISystem->GetILog()->LogError("NO CONTAINER FOR ENTITY %s (Type=%d)",GetName()?GetName():"UNNAMEDENTITY",(int)GetClassId());
			if (!m_pISystem->GetIGame()->GetModuleState(EGameMultiplayer))
			{
				CryError("ERROR, NO CONTAINER FOR ENTITY %s (Type=%d)",GetName()?GetName():"UNNAMEDENTITY",(int)GetClassId());	
			}
			return false;
		}
		_VERIFY(m_pContainer);
		_VERIFY(m_pContainer->Read(stm));
	}

	VERIFY_ENTITY_COOKIE_NO(stm,0x91)

	for (int iSlot=0; iSlot<m_nMaxCharNum; iSlot++) if (m_pCryCharInstance[iSlot])
		m_pCryCharInstance[iSlot]->ForceReskin();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
bool CEntity::Save(CStream& stream , IScriptObject *pStream)
{
	if (!Write(stream))
		return false;
	
	/*[marco] must be done before, to be consistent
	with loadlevel!

	Vec3d vPos;
	Vec3d vAng;
	if( IsBound() )				// save relative position if bound
	{
		vPos = GetPos(false);		
		vAng = GetAngles(1);
	}
	else
	{
		vPos = GetPos();
		vAng = GetAngles();
	}
	_VERIFY(stream.Write(vPos));
	_VERIFY(stream.Write(vAng));
	*/


	//<<add here>> for other C++ stuff to save(see what is already in Write)

	//Timur[2/1/2002] Both OnSave and OnLoad must be implemented.
	if (pStream && m_pSaveFunc && m_pLoadFunc)
	{
		m_pScriptSystem->BeginCall( m_pSaveFunc );
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam(pStream);
		m_pScriptSystem->EndCall();
	}

	// [marco] save/load state after loading/saving custom entity props from
	// onload and onsave
	int nState=GetStateIdx();
	_VERIFY(stream.Write(nState));

	return (true);
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
bool CEntity::Load(CStream& stream , IScriptObject *pStream)
{
	if (!Read(stream))
		return false;
	
	/* [marco] must be done before spawnetity, to be consistent
	with loadlevel!
	Vec3d vPos;
	SetPos(Vec3d(0,0,0));
	_VERIFY(stream.Read(vPos));
	SetPos(vPos);	
	SetAngles(Vec3d(0,0,0));
	_VERIFY(stream.Read(vPos));
	SetAngles(vPos);
	*/
	
	//<<add here>> for other C++ stuff to save(see what is already in Read)
	

	//Timur[2/1/2002] Both OnSave and OnLoad must be implemented.
	if (pStream && m_pSaveFunc && m_pLoadFunc)
	{
		m_pScriptSystem->BeginCall( m_pLoadFunc );
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam(pStream);
		m_pScriptSystem->EndCall();
	} 
	
	// [marco] save/load state after loading/saving custom entity props from
	// onload and onsave
	int nState;
	_VERIFY(stream.Read(nState));
	GotoState(nState);
	m_awakeCounter = 4; // give entity a chance to fetch the updated physics state

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
bool CEntity::LoadPATCH1(CStream& stream , IScriptObject *pStream)
{
	if (!Read(stream))
		return false;

	/* [marco] must be done before spawnetity, to be consistent
	with loadlevel!
	Vec3d vPos;
	SetPos(Vec3d(0,0,0));
	_VERIFY(stream.Read(vPos));
	SetPos(vPos);	
	SetAngles(Vec3d(0,0,0));
	_VERIFY(stream.Read(vPos));
	SetAngles(vPos);
	*/

	//<<add here>> for other C++ stuff to save(see what is already in Read)


	//Timur[2/1/2002] Both OnSave and OnLoad must be implemented.
	if (pStream && m_pSaveFunc && m_pLoadFunc)
	{
		//[kirill] we need to support prev version saves
		//[kirill] we need to support prev version saves
		if(m_pLoadPATCH1Func)
			m_pScriptSystem->BeginCall( m_pLoadPATCH1Func );
		else if(m_pLoadRELEASEFunc)
			m_pScriptSystem->BeginCall( m_pLoadRELEASEFunc );
		else
			m_pScriptSystem->BeginCall( m_pLoadFunc );
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam(pStream);
		m_pScriptSystem->EndCall();
	} 

	// [marco] save/load state after loading/saving custom entity props from
	// onload and onsave
	int nState;
	_VERIFY(stream.Read(nState));
	GotoState(nState);
	m_awakeCounter = 4; // give entity a chance to fetch the updated physics state

	return true;
}



///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
bool CEntity::LoadRELEASE(CStream& stream , IScriptObject *pStream)
{
	if (!Read(stream))
		return false;

	/* [marco] must be done before spawnetity, to be consistent
	with loadlevel!
	Vec3d vPos;
	SetPos(Vec3d(0,0,0));
	_VERIFY(stream.Read(vPos));
	SetPos(vPos);	
	SetAngles(Vec3d(0,0,0));
	_VERIFY(stream.Read(vPos));
	SetAngles(vPos);
	*/

	//<<add here>> for other C++ stuff to save(see what is already in Read)


	//Timur[2/1/2002] Both OnSave and OnLoad must be implemented.
	if (pStream && m_pSaveFunc && m_pLoadFunc)
	{
		//[kirill] we need to support prev version saves
		if(m_pLoadRELEASEFunc)
			m_pScriptSystem->BeginCall( m_pLoadRELEASEFunc );
		else
			m_pScriptSystem->BeginCall( m_pLoadFunc );
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam(pStream);
		m_pScriptSystem->EndCall();
	} 

	// [marco] save/load state after loading/saving custom entity props from
	// onload and onsave
	int nState;
	_VERIFY(stream.Read(nState));
	GotoState(nState);
	m_awakeCounter = 4; // give entity a chance to fetch the updated physics state

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
bool CEntity::PostLoad()
{
	//[kirill] bind it all now
	if (!m_bIsBound && m_idBoundTo!=0)
	{
		IEntity *pEntBound;
		pEntBound = m_pEntitySystem->GetEntity(m_idBoundTo);
			if (pEntBound)
			{
				Vec3 pos = GetPos();
				pEntBound->Bind( GetId(), m_cBind, false, true);
//				SetPos( pos, false );
			}
	}
	if (m_physic)
		m_physic->PostSetStateFromSnapshot();

	return true;
}
