//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XSystemBase.cpp
//  Description: System base class implementation.
//
//  History:
//  - August 8, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <IEntitySystem.h>
#include "XSystemBase.h"
#include "PlayerSystem.h"
#include "WeaponSystemEx.h"
#include "XVehicleSystem.h"
//#include "ScriptObjectEntity.h"
#include "ScriptObjectPlayer.h"
#include "ScriptObjectSpectator.h"			// ScriptObjectSpectator
#include "ScriptObjectAdvCamSystem.h"		// ScriptObjectAdvCamSystem
#include "ScriptObjectVehicle.h"
#include "UISystem.h"
#include "XPlayer.h"
#include "XVehicle.h"
#include "Spectator.h"									// CSpectator
#include "AdvCamSystem.h"								// CAdvCamSystem
#include <IXMLDOM.h>
#include <IAISystem.h>
#include <IAgent.h>
#include <I3DEngine.h>
#include <ISound.h>
#include <ICryPak.h>
#include <IMovieSystem.h>
#include "ScriptObjectSynched2DTable.h"	// CScriptObjectSynched2DTable
#include "Synched2DTable.h"							// CSynched2DTable


//////////////////////////////////////////////////////////////////////////
void CXSystemBase::SMissionInfo::SetLevelFolder( const char *szLevelDir )
{
	sLevelFolder = szLevelDir;
	std::replace( sLevelFolder.begin(),sLevelFolder.end(),'\\','/' );
	int pos = sLevelFolder.rfind('/');
	if (pos >= 0)
	{
		sLevelName = sLevelFolder.substr(pos+1);
	}
	else
    sLevelName = sLevelFolder;
}

///////////////////////////////////////////////
CXSystemBase::CXSystemBase(CXGame *pGame,ILog *pLog)
{
	m_pGame = pGame;
	m_pSystem = pGame->GetSystem();
	m_pLog = pLog;
	m_pEntitySystem = (IEntitySystem *)m_pSystem->GetIEntitySystem();
	m_pConsole = m_pSystem->GetIConsole();
//	m_cRandomSeed=0;
}

///////////////////////////////////////////////
CXSystemBase::~CXSystemBase()
{
	m_pGame = NULL;
	m_pEntitySystem = NULL;
}

///////////////////////////////////////////////
IEntity* CXSystemBase::GetEntity(WORD wID)
{
	return m_pEntitySystem->GetEntity(wID);
}

///////////////////////////////////////////////
IEntity*	CXSystemBase::GetEntity(const char *sEntity)
{
	return m_pEntitySystem->GetEntity(sEntity);
}

///////////////////////////////////////////////
IEntityIt *CXSystemBase::GetEntities()
{
	return m_pEntitySystem->GetEntityIterator();
}

///////////////////////////////////////////////
bool CXSystemBase::EntityExists(WORD id)
{
	return GetEntity(id) ? true : false;
}

//!get the local player entity
IEntity *CXSystemBase::GetLocalPlayer()
{
	if(m_pGame->IsClient())
	{
		EntityId nID = m_pGame->m_pClient->GetPlayerId();
		return m_pEntitySystem->GetEntity(nID);
	}
	return NULL;
}

void CXSystemBase::InitRegistry(const char *szLevelDir)
{
	CPlayerSystem *pPlayerSystem = m_pGame->GetPlayerSystem();
	CVehicleSystem *pVehicleSystem = m_pGame->GetVehicleSystem();
	CWeaponSystemEx *pWeaponSystemEx = m_pGame->GetWeaponSystemEx();	// m10

	IEntityClassRegistry *pEntityClassRegistry = m_pGame->GetClassRegistry();

	// Enumerate entity classes.
	EntityClass *entCls = NULL;
	pEntityClassRegistry->MoveFirst();
	do {
		entCls = pEntityClassRegistry->Next();
		if (entCls)
		{
			const char* entity_type = entCls->strGameType.c_str();
			EntityClassId ClassId = entCls->ClassId;
			if(strcmp("Player",entity_type)==0)
				pPlayerSystem->AddPlayerClass(ClassId);

			if(strcmp("Vehicle",entity_type)==0)
				pVehicleSystem->AddVehicleClass(ClassId);

			if(strcmp("Projectile",entity_type)==0)
			{
				// cannot be loaded at that point - other scripts must be loaded before
				pWeaponSystemEx->AddProjectileClass(ClassId);
			}
		}
	} while (entCls);
}

//////////////////////////////////////////////////////////////////////////
// let's load the language table for this mission
bool CXSystemBase::LoadLanguageTable(const char *szLevelDir,const char *szMissionName)
{
	char szLanguageFile[512];
	if (!szMissionName || !m_pGame)
		return (false);
	
	//sprintf(szLanguageFile,"%s_%s",szLevelDir,szMissionName);
	//sprintf(szLanguageFile,"%s_%s",szLevelDir,szMissionName);
	sprintf(szLanguageFile,"%s.xml",szLevelDir );

	return (m_pGame->m_StringTableMgr.LoadStringTable(szLanguageFile));		
}


//////////////////////////////////////////////////////////////////////////
bool CXSystemBase::LoadLevelEntities( SMissionInfo &missionInfo  )
{
	bool bSpawn = true;

	m_setLevelEntities.clear();
	m_ChildParentMap.clear();
	m_setPlayerEntities.clear();

	// [marco] clear the previous mission, otherwise if the new script
	// is missing it will erroneuosly reuse the previous mission script!
	m_pGame->GetScriptSystem()->SetGlobalToNull("Mission");

	XDOM::IXMLDOMNodePtr pScriptName = missionInfo.pMissionXML->getAttribute("Script");						
	if (pScriptName)
	{
		//char szMissionScript[512];
		//sprintf(szMissionScript,"%s",pScriptName->getText());							
		//if (!m_pGame->GetScriptSystem()->ExecuteFile(szMissionScript,false))
		
		if (!m_pGame->GetScriptSystem()->ExecuteFile(pScriptName->getText(),false))
		{
			m_pLog->Log("Cannot load mission script %s", pScriptName->getText());
		}
		else
		{
			m_pGame->GetScriptSystem()->BeginCall("Mission","OnInit");
			m_pGame->GetScriptSystem()->EndCall();
		}															
	}
	LoadXMLNode( missionInfo.pMissionXML,bSpawn );
	BindChildren();

	if (m_pGame && m_pSystem)
	{
		// [Anton] allow entities to restore links between them
		IEntityItPtr pEntities = m_pSystem->GetIEntitySystem()->GetEntityIterator();
		pEntities->MoveFirst();
		IEntity *pEnt=NULL;
		while((pEnt=pEntities->Next())!=NULL)
			pEnt->PostLoad();		
	}

	return true;
}

bool CXSystemBase::LoadMaterials(XDOM::IXMLDOMDocument *doc)
{
	//////////////////////////////////////////////////////////////////////////
	// Load default materials.
	m_pGame->m_XSurfaceMgr.LoadDefaults();
	//////////////////////////////////////////////////////////////////////////

	XDOM::IXMLDOMDocumentPtr pDoc;
	XDOM::IXMLDOMNodeListPtr pNodes;
	pDoc=doc;//m_pSystem->CreateXMLDocument();
	
#if !defined(LINUX64)
	if(pDoc!=NULL)
#else
	if(pDoc!=0)
#endif
	{
		///////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////
		//LOAD TERRAIN SURFACES
		///////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////
		pNodes=pDoc->getElementsByTagName("SurfaceTypes");
		XDOM::IXMLDOMNodePtr pNode;
		int nSurfaceID=0;
#if !defined(LINUX64)
		if((pNodes!=NULL) && pNodes->length())
#else
		if((pNodes!=0) && pNodes->length())
#endif
		{
			pNodes->reset();
			while(pNode=pNodes->nextNode())
			{
				XDOM::IXMLDOMNodeListPtr pSurfaceTypeList;
				XDOM::IXMLDOMNodePtr pSurface;
				pSurfaceTypeList=pNode->getChildNodes();
				pSurfaceTypeList->reset();
				
				while(pSurface=pSurfaceTypeList->nextNode())
				{
					if(string(pSurface->getName())==string("SurfaceType"))
					{
						XDOM::IXMLDOMNodePtr pMaterial;
						pMaterial=pSurface->getAttribute("Material");
#if !defined(LINUX64)
						if(pMaterial!=NULL)
#else
						if(pMaterial!=0)
#endif
						{
							string sMaterial=pMaterial->getText();
							if(!sMaterial.length()){
								m_pGame->m_XSurfaceMgr.SetTerrainSurface("mat_default",nSurfaceID);
							}
							else{
								m_pGame->m_XSurfaceMgr.SetTerrainSurface(sMaterial,nSurfaceID);
							}
							nSurfaceID++;							
						}
					}
				}
			}
		}
		m_pGame->m_XSurfaceMgr.InitPhisicalSurfaces();
		//return true;
	}
	return true;
}

void CXSystemBase::LoadXMLNode(XDOM::IXMLDOMNode *pInputNode, bool bSpawn)
{
	XDOM::IXMLDOMNodeListPtr pEDNodes;
	XDOM::IXMLDOMNodePtr pNode;

	XDOM::IXMLDOMNodeListPtr pEquipPackList = pInputNode->getElementsByTagName("EquipPacks");
	XDOM::IXMLDOMNodePtr pPackListNode;
	if (pEquipPackList)
	{
		pEquipPackList->reset();
#if !defined(LINUX64)
		while ((pPackListNode = pEquipPackList->nextNode()) != NULL)
#else
		while ((pPackListNode = pEquipPackList->nextNode()) != 0)
#endif
		{
			XDOM::IXMLDOMNodeListPtr pPackList = pPackListNode->getElementsByTagName("EquipPack");
			XDOM::IXMLDOMNodePtr pPack;
			if (pPackList)
			{
				pPackList->reset();
#if !defined(LINUX64)
				while ((pPack = pPackList->nextNode()) != NULL)
#else
				while ((pPack = pPackList->nextNode()) != 0)
#endif
					m_pGame->AddEquipPack(pPack);
			}
		}
	}
	UINT iCurWeapon = 0;
	XDOM::IXMLDOMNodeListPtr pWeaponsList = pInputNode->getElementsByTagName("Weapons");
	if (pWeaponsList)
	{
		pWeaponsList->reset();
		XDOM::IXMLDOMNodePtr pWeapons=pWeaponsList->nextNode();
		if (pWeapons)
		{
			XDOM::IXMLDOMNodeListPtr pUsedList = pWeapons->getElementsByTagName("Used");
			if (pUsedList)
			{
				pUsedList->reset();
				XDOM::IXMLDOMNodePtr pUsed=pUsedList->nextNode();
				if (pUsed)
				{
					XDOM::IXMLDOMNodeListPtr pWeaponList = pUsed->getElementsByTagName("Weapon");
					XDOM::IXMLDOMNodePtr pUsedWeapon;
					if (pWeaponList)
					{
						pWeaponList->reset();
#if !defined(LINUX64)
						while((pUsedWeapon = pWeaponList->nextNode())!=NULL)
#else
						while((pUsedWeapon = pWeaponList->nextNode())!=0)
#endif
						{
							XDOM::IXMLDOMNodePtr pWeaponName;
							XDOM::IXMLDOMNodePtr pID;
							pWeaponName = pUsedWeapon->getAttribute("Name");
							pID = pUsedWeapon->getAttribute("id");
							int iID = 0;
							if (pID)
							{
								iID = atoi(pID->getText());
								ASSERT(m_pSystem->GetIEntitySystem()->IsIDUsed(iID) == false);
							}
							if(pWeaponName)
							{
								TRACE("Weapon Available: %s", pWeaponName->getText());
								if (bSpawn)
									if (!m_pGame->GetWeaponSystemEx()->AddWeapon(pWeaponName->getText()))
									{
										TRACE("Can't spawn / register weapon '%s'", pWeaponName->getText());
									}
							}
						}
					}
				}
			}
		}
	}

		/*
	//////////////////////////////////////////////////////////////////////////
	// Load Music
	//////////////////////////////////////////////////////////////////////////
	
	if (pInputNode->getAttribute("MusicScript"))
	{
		const char *pszFilename = NULL;
		pszFilename = pInputNode->getAttribute("MusicScript")->getText();
		if (!m_pSystem->LoadMusicDataFromLUA(pszFilename))
			m_pLog->Log("WARNING: Cannot load music-script %s !", pszFilename);
	}
	*/

	///////////////////////////////////////////////////////////////////////////////////////
	// Load weapons
	///////////////////////////////////////////////////////////////////////////////////////
  
	if (pInputNode->getAttribute("PlayerEquipPack"))
	{
		const char *pszEP = NULL;
		pszEP = pInputNode->getAttribute("PlayerEquipPack")->getText();
		m_pGame->SetPlayerEquipPackName(pszEP);
	}
	

	
	XDOM::IXMLDOMNodeListPtr pObjectsTagList;
	XDOM::IXMLDOMNodePtr pObjectsTag;

	int iRespawnCount = 0;

	pObjectsTagList = pInputNode->getElementsByTagName("Objects");
	if (!pObjectsTagList) return;

	pObjectsTagList->reset();
	pObjectsTag =pObjectsTagList->nextNode(); 

	///////////////////////////////////////////////////////////////////////////////////////
	//	GET TAG POINTS
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	XDOM::IXMLDOMNodeListPtr pNodes;

	pNodes=pObjectsTag->getElementsByTagName("Object");
	if(pNodes)
	{
		XDOM::IXMLDOMNodePtr pNode;
		pNodes->reset();
		while(pNode=pNodes->nextNode())
		{
			XDOM::IXMLDOMNodePtr pType;
			XDOM::IXMLDOMNodePtr pName;
			XDOM::IXMLDOMNodePtr pPos;
			pType=pNode->getAttribute("Type");
			pName=pNode->getAttribute("Name");
			pPos=pNode->getAttribute("Pos");
			XDOM::IXMLDOMNodePtr pAngles = pNode->getAttribute("Angles");
			Vec3 angles;
			if (pAngles)
				angles = StringToVector(pAngles->getText());
			else 
				angles(0,0,0);
#if !defined(LINUX64)
			if((pType!=NULL) && (pName!=NULL) && (pPos!=NULL))
#else
			if((pType!=0) && (pName!=0) && (pPos!=0))
#endif
			{
				// <<FIXME>> fixed height... maybe should be sent in ??
				Vec3 pos = StringToVector(pPos->getText());
				if (!stricmp(pType->getText(),"TagPoint"))
				{
					ITagPoint *pPoint = m_pGame->CreateTagPoint(pName->getText(),pos, angles);
					//FIXME - check fo case of multople tagPoints with same name - see m_pGame->CreateTagPoint
					if(pPoint)
					{
						IAIObject *pObject = m_pSystem->GetAISystem()->CreateAIObject(AIOBJECT_WAYPOINT, (void*) pPoint);
						
						if(pObject)
						{
							AIObjectParameters params;
							params.fEyeHeight = 1.7f;
							pObject->ParseParameters(params);

							pObject->SetPos(pos);
							pObject->SetAngles(Vec3(0,0,0));
							pObject->SetName(pName->getText());
						}
					}
				}
				else if (!stricmp(pType->getText(),"Respawn"))
				{
					char name[50];
					if(!pName->getText())
						sprintf(name,"Respawn%d",iRespawnCount);
					else
						strcpy(name,pName->getText());
					ITagPoint *pPoint = m_pGame->CreateTagPoint(name,pos, angles);
					//FIXME - check fo case of multople tagPoints with same name - see m_pGame->CreateTagPoint
					if(pPoint)
					{
						if(!m_pGame->m_bEditor)
						{
							AddRespawnPoint(pPoint);
						}
						iRespawnCount++;
					}
				}
				else if (!stricmp(pType->getText(),"Shape"))
				{
					///////////////////////////////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////////////////////////////
					//	GET AREAS / SHAPES
					///////////////////////////////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////////////////////////////
					XDOM::IXMLDOMNodePtr pAreaID;
					int		areaID;
					XDOM::IXMLDOMNodePtr pGroupID;
					int		groupID=-1;
					XDOM::IXMLDOMNodePtr pAreaWidth;
					float		areaWidth = 0.0f;
					XDOM::IXMLDOMNodePtr pAreaHeight;
					float		areaHeight = 0.0f;

					XDOM::IXMLDOMNodeListPtr pPointsTagList;
					XDOM::IXMLDOMNodePtr pPointsTag;
					XDOM::IXMLDOMNodeListPtr pEntitiesTagList;
					XDOM::IXMLDOMNodePtr pEntitiesTag;

					pAreaID=pNode->getAttribute("AreaId");
					pName=pNode->getAttribute("Entity");
					areaID = atoi(pAreaID->getText());
					pGroupID=pNode->getAttribute("GroupId");
					if (pGroupID)
						groupID = atoi(pGroupID->getText());
					pAreaWidth=pNode->getAttribute("Width");
#if !defined(LINUX64)
					if(pAreaWidth!=NULL)
#else
					if(pAreaWidth!=0)
#endif
					{
						areaWidth = (float)atof(pAreaWidth->getText());
						if(areaWidth<0)
							areaWidth = 0.0f;
					}
					pAreaHeight=pNode->getAttribute("Height");
#if !defined(LINUX64)
					if(pAreaHeight!=NULL)
#else
					if(pAreaHeight!=0)
#endif
					{
						areaHeight = (float)atof(pAreaHeight->getText());
						if(areaHeight<0)
							areaHeight = 0.0f;
					}


					pPointsTagList = pNode->getElementsByTagName("Points");
					if (!pPointsTagList) continue;

					pEntitiesTagList = pNode->getElementsByTagName("Entities");
					if (!pEntitiesTagList) continue;

					pEntitiesTagList->reset();
					pEntitiesTag = pEntitiesTagList->nextNode(); 
					XDOM::IXMLDOMNodeListPtr pTheEntities;
					pTheEntities=pEntitiesTag->getElementsByTagName("Entity");
					std::vector<string>	entitiesName;
					entitiesName.clear();
					if(pTheEntities)
					{
						XDOM::IXMLDOMNodePtr pTheEntity;
						pTheEntities->reset();

						while(pTheEntity=pTheEntities->nextNode())
						{
							pName=pTheEntity->getAttribute("Name");
							entitiesName.push_back(pName->getText());
						}
					}

					pPointsTagList->reset();
					pPointsTag = pPointsTagList->nextNode(); 

					XDOM::IXMLDOMNodeListPtr pThePoints;
					pThePoints=pPointsTag->getElementsByTagName("Point");

					if(pThePoints)
					{
						XDOM::IXMLDOMNodePtr pThePoint;
						pThePoints->reset();
						int sz = pThePoints->length();
						int	cntr=0;
						Vec3 *borderPoints = new Vec3[sz];

						while(pThePoint=pThePoints->nextNode())
						{
							pPos=pThePoint->getAttribute("Pos");
							Vec3 pos = StringToVector(pPos->getText());
							borderPoints[cntr++] = StringToVector(pPos->getText());
						}

						m_pGame->CreateArea(borderPoints, sz, entitiesName, areaID, groupID, areaWidth, areaHeight);
						delete borderPoints;
					}
				}
				else if (!stricmp(pType->getText(),"AreaBox"))
				{
					///////////////////////////////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////////////////////////////
					//	GET AREAS / BOXES
					///////////////////////////////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////////////////////////////
					XDOM::IXMLDOMNodePtr pAreaID;
					int		areaID;
					XDOM::IXMLDOMNodePtr pGroupID;
					int		groupID=-1;
					XDOM::IXMLDOMNodePtr pEdgeWidth;
					float		edgeWidth = 0.0f;
					XDOM::IXMLDOMNodePtr pAreaWidth;
					float		areaWidth = 0.0f;
					XDOM::IXMLDOMNodePtr pAreaHeight;
					float		areaHeight = 0.0f;
					XDOM::IXMLDOMNodePtr pAreaLength;
					float		areaLength = 0.0f;
					XDOM::IXMLDOMNodePtr pPos;
					Vec3		Pos(0.0f, 0.0f, 0.0f);
					XDOM::IXMLDOMNodePtr pAngles;
					Vec3		Angles(0.0f, 0.0f, 0.0f);

					XDOM::IXMLDOMNodeListPtr pEntitiesTagList;
					XDOM::IXMLDOMNodePtr pEntitiesTag;

					pAreaID=pNode->getAttribute("AreaId");
					if (pAreaID)
						areaID = atoi(pAreaID->getText());
					pGroupID=pNode->getAttribute("GroupId");
					if (pGroupID)
						groupID = atoi(pGroupID->getText());
					pPos=pNode->getAttribute("Pos");
					if (pPos)
						Pos=StringToVector(pPos->getText());
					pAngles=pNode->getAttribute("Angles");
					if (pAngles)
						Angles=StringToVector(pAngles->getText());
					pEdgeWidth=pNode->getAttribute("FadeInZone");
#if !defined(LINUX64)
					if(pEdgeWidth!=NULL)
#else
					if(pEdgeWidth!=0)
#endif
					{
						edgeWidth = (float)atof(pEdgeWidth->getText());
						if(edgeWidth<0)
							edgeWidth = 0.0f;
					}
					pAreaWidth=pNode->getAttribute("Width");
					if (pAreaWidth)
						areaWidth=(float)atof(pAreaWidth->getText());
					pAreaHeight=pNode->getAttribute("Height");
					if (pAreaHeight)
						areaHeight=(float)atof(pAreaHeight->getText());
					pAreaLength=pNode->getAttribute("Length");
					if (pAreaLength)
						areaLength=(float)atof(pAreaLength->getText());
					pEntitiesTagList = pNode->getElementsByTagName("Entities");
					if (!pEntitiesTagList)
						continue;
					pEntitiesTagList->reset();
					pEntitiesTag = pEntitiesTagList->nextNode(); 
					XDOM::IXMLDOMNodeListPtr pTheEntities;
					pTheEntities=pEntitiesTag->getElementsByTagName("Entity");
					std::vector<string>	entitiesName;
					entitiesName.clear();
					if(pTheEntities)
					{
						XDOM::IXMLDOMNodePtr pTheEntity;
						pTheEntities->reset();

						while(pTheEntity=pTheEntities->nextNode())
						{
							pName=pTheEntity->getAttribute("Name");
							entitiesName.push_back(pName->getText());
						}
					}
					Vec3 MinBox;
					Vec3 MaxBox;
					Vec3 size;
					size.x = areaWidth;
					size.y = areaLength;
					size.z = areaHeight;
					MinBox = -size/2;
					MaxBox = size/2;
					MinBox.z = 0.0f;
					MaxBox.z = size.z;
					Matrix44 TM=Matrix34::CreateRotationXYZ( Deg2Rad(Angles), Pos );	//set rotation and translation in one function call
					TM	=	GetTransposed44(TM); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

					//TM.RotateDegree(Angles);
					//TM.Translate(Pos);
					m_pGame->CreateArea(MinBox, MaxBox, TM, entitiesName, areaID, groupID, edgeWidth);
				}
				else if (!stricmp(pType->getText(),"AreaSphere"))
				{
					///////////////////////////////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////////////////////////////
					//	GET AREAS / SPHERES
					///////////////////////////////////////////////////////////////////////////////////////
					///////////////////////////////////////////////////////////////////////////////////////
					XDOM::IXMLDOMNodePtr pAreaID;
					int		areaID;
					XDOM::IXMLDOMNodePtr pGroupID;
					int		groupID=-1;
					XDOM::IXMLDOMNodePtr pEdgeWidth;
					float		edgeWidth = 0.0f;
					XDOM::IXMLDOMNodePtr pRadius;
					float		fRadius = 0.0f;
					XDOM::IXMLDOMNodePtr pPos;
					Vec3		Pos(0.0f, 0.0f, 0.0f);

					XDOM::IXMLDOMNodeListPtr pEntitiesTagList;
					XDOM::IXMLDOMNodePtr pEntitiesTag;

					pAreaID=pNode->getAttribute("AreaId");
					if (pAreaID)
						areaID = atoi(pAreaID->getText());
					pGroupID=pNode->getAttribute("GroupId");
					if (pGroupID)
						groupID = atoi(pGroupID->getText());
					pPos=pNode->getAttribute("Pos");
					if (pPos)
						Pos=StringToVector(pPos->getText());
					pEdgeWidth=pNode->getAttribute("FadeInZone");
#if !defined(LINUX64)
					if(pEdgeWidth!=NULL)
#else
					if(pEdgeWidth!=0)
#endif
					{
						edgeWidth = (float)atof(pEdgeWidth->getText());
						if(edgeWidth<0)
							edgeWidth = 0.0f;
					}
					pRadius=pNode->getAttribute("Radius");
					if (pRadius)
						fRadius=(float)atof(pRadius->getText());
					pEntitiesTagList = pNode->getElementsByTagName("Entities");
					if (!pEntitiesTagList)
						continue;
					pEntitiesTagList->reset();
					pEntitiesTag = pEntitiesTagList->nextNode(); 
					XDOM::IXMLDOMNodeListPtr pTheEntities;
					pTheEntities=pEntitiesTag->getElementsByTagName("Entity");
					std::vector<string>	entitiesName;
					entitiesName.clear();
					if(pTheEntities)
					{
						XDOM::IXMLDOMNodePtr pTheEntity;
						pTheEntities->reset();

						while(pTheEntity=pTheEntities->nextNode())
						{
							pName=pTheEntity->getAttribute("Name");
							entitiesName.push_back(pName->getText());
						}
					}
					m_pGame->CreateArea(Pos, fRadius, entitiesName, areaID, groupID, edgeWidth);
				}
				else if (!stricmp(pType->getText(),"AIAnchor"))
				{
						XDOM::IXMLDOMNodePtr pId;
						int anchorID;
							pId=pNode->getAttribute("AnchorId");
							
							if( pId )	// anchpr should have ID
							{
								anchorID=atoi(pId->getText());

								if( anchorID>= 0 )
								{
									XDOM::IXMLDOMNodePtr pAreaRadius;
									pAreaRadius = pNode->getAttribute("Area");
									IAIObject *pObject = m_pSystem->GetAISystem()->CreateAIObject(anchorID, NULL);
									pObject->SetPos(StringToVector(pPos->getText()));
									//alberto
#if !defined(LINUX64)
									if(pAngles!=NULL)
#else
									if(pAngles!=0)
#endif
										pObject->SetAngles(StringToVector(pAngles->getText()));
									else
										pObject->SetAngles(Vec3(0,0,0));
									pObject->SetName(pName->getText());
								if(pAreaRadius)
									pObject->SetRadius((float)atof(pAreaRadius->getText()));

								}
								else
								{
								// problem with name-ID mapping
								// see MasterCD\SCRIPTS\AI\Anchor.lua
									m_pLog->Log("AIAnchor ID is negative Anchor name:  '%s'", pName->getText());
								}
							}
							else
							{
							// no anchor ID saved?
							// see MasterCD\SCRIPTS\AI\Anchor.lua
								m_pLog->Log("AIAnchor ID is missing Anchor name:  '%s'", pName->getText());
							}


				}
				else if (!stricmp(pType->getText(),"AIPath"))
				{
						IAISystem *pAISystem = m_pSystem->GetAISystem();
						XDOM::IXMLDOMNodeListPtr pPointsTagList;
						XDOM::IXMLDOMNodePtr pPointsTag;

						pName=pNode->getAttribute("Name");
						if (!pAISystem->CreatePath(pName->getText()))
							CryError("[AIERROR] DUPLICATE PATH NAME FOUND [%s]. Change it in the editor and re-export.",pName->getText());


						pPointsTagList = pNode->getElementsByTagName("Points");
						if (!pPointsTagList) continue;

						pPointsTagList->reset();
						pPointsTag = pPointsTagList->nextNode(); 

						XDOM::IXMLDOMNodeListPtr pThePoints;

						pThePoints=pPointsTag->getElementsByTagName("Point");

						if(pThePoints)
						{
							XDOM::IXMLDOMNodePtr pThePoint;
							pThePoints->reset();

							while(pThePoint=pThePoints->nextNode())
							{
								pPos=pThePoint->getAttribute("Pos");
								Vec3 pos = StringToVector(pPos->getText());
								pAISystem->AddPointToPath(pos,pName->getText());
							}
						}

				}
				else if (!stricmp(pType->getText(),"ForbiddenArea"))
				{
						IAISystem *pAISystem = m_pSystem->GetAISystem();
						XDOM::IXMLDOMNodeListPtr pPointsTagList;
						XDOM::IXMLDOMNodePtr pPointsTag;

						pName=pNode->getAttribute("Name");
						if (!pAISystem->CreatePath(pName->getText(),AREATYPE_FORBIDDEN))
							CryError("[AIERROR] FORBIDDEN AREA NAME FOUND [%s]. Change it in the editor and re-export.",pName->getText());


						pPointsTagList = pNode->getElementsByTagName("Points");
						if (!pPointsTagList) continue;

						pPointsTagList->reset();
						pPointsTag = pPointsTagList->nextNode(); 

						XDOM::IXMLDOMNodeListPtr pThePoints;

						pThePoints=pPointsTag->getElementsByTagName("Point");

						if(pThePoints)
						{
							XDOM::IXMLDOMNodePtr pThePoint;
							pThePoints->reset();

							while(pThePoint=pThePoints->nextNode())
							{
								pPos=pThePoint->getAttribute("Pos");
								Vec3 pos = StringToVector(pPos->getText());
								pAISystem->AddPointToPath(pos,pName->getText(),AREATYPE_FORBIDDEN);
							}
						}
				}
				else if (!stricmp(pType->getText(),"AINavigationModifier"))
				{
						IAISystem *pAISystem = m_pSystem->GetAISystem();
						XDOM::IXMLDOMNodeListPtr pPointsTagList;
						XDOM::IXMLDOMNodePtr pPointsTag;
						XDOM::IXMLDOMNodePtr pHeight;

						pName=pNode->getAttribute("Name");
						pHeight=pNode->getAttribute("Height");

						if (!pAISystem->CreatePath(pName->getText(),AREATYPE_NAVIGATIONMODIFIER,(float)atof(pHeight->getText())))
							CryError("[AIERROR] DUPLICATE NAVIGATION MODIFIER NAME FOUND [%s]. Change it in the editor and re-export.",pName->getText());

						pPointsTagList = pNode->getElementsByTagName("Points");
						if (!pPointsTagList) continue;

						pPointsTagList->reset();
						pPointsTag = pPointsTagList->nextNode(); 

						XDOM::IXMLDOMNodeListPtr pThePoints;

						pThePoints=pPointsTag->getElementsByTagName("Point");

						if(pThePoints)
						{
							XDOM::IXMLDOMNodePtr pThePoint;
							pThePoints->reset();

							while(pThePoint=pThePoints->nextNode())
							{
								pPos=pThePoint->getAttribute("Pos");
								Vec3 pos = StringToVector(pPos->getText());
								pAISystem->AddPointToPath(pos,pName->getText(),AREATYPE_NAVIGATIONMODIFIER);
							}
						}

				}
				else if (!stricmp(pType->getText(),"AIHorizontalOcclusionPlane"))
				{
						IAISystem *pAISystem = m_pSystem->GetAISystem();
						XDOM::IXMLDOMNodeListPtr pPointsTagList;
						XDOM::IXMLDOMNodePtr pPointsTag;
						
						pName=pNode->getAttribute("Name");
						
						if (!pAISystem->CreatePath(pName->getText(),AREATYPE_OCCLUSION_PLANE))
							CryError("[AIERROR] DUPLICATE OCCLUSION PLANE NAME FOUND [%s]. Change it in the editor and re-export.",pName->getText());

						pPointsTagList = pNode->getElementsByTagName("Points");
						if (!pPointsTagList) continue;

						pPointsTagList->reset();
						pPointsTag = pPointsTagList->nextNode(); 

						XDOM::IXMLDOMNodeListPtr pThePoints;

						pThePoints=pPointsTag->getElementsByTagName("Point");

						if(pThePoints)
						{
							XDOM::IXMLDOMNodePtr pThePoint;
							pThePoints->reset();

							while(pThePoint=pThePoints->nextNode())
							{
								pPos=pThePoint->getAttribute("Pos");
								Vec3 pos = StringToVector(pPos->getText());
								pAISystem->AddPointToPath(pos,pName->getText(),AREATYPE_OCCLUSION_PLANE);
							}
						}

				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////
	//SPAWN ENTIES
	///////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////	
	// do not load entities when loading from a savegame,
	// they will be deleted anyway	
	if (bSpawn && !m_pGame->m_bIsLoadingLevelFromFile)
	{		
		pNodes=pObjectsTag->getElementsByTagName("Entity");
		if(pNodes)
		{
			XDOM::IXMLDOMNodePtr pNode;
			pNodes->reset();
			while(pNode=pNodes->nextNode())
			{				
				CEntityStreamData eData;
				// if pNode is NULL, it should spawn from CEntityStreamData 
				// otherwise it fills CEntityStreamData with info for spawning the entity
				SpawnEntityFromXMLNode(pNode,&eData);
			}
		}
	}

}

//////////////////////////////////////////////////////////////////////////
bool CXSystemBase::SpawnEntityFromXMLNode(XDOM::IXMLDOMNodePtr pNode,CEntityStreamData *pData)
{
	XDOM::IXMLDOMNodePtr pEntityClass;
	XDOM::IXMLDOMNodePtr pName;
	XDOM::IXMLDOMNodePtr pPos;
	XDOM::IXMLDOMNodePtr pAngles;
	XDOM::IXMLDOMNodePtr pId;
	XDOM::IXMLDOMNodePtr pParentId;
	XDOM::IXMLDOMNodePtr pCastShadowVolume;
	XDOM::IXMLDOMNodePtr pSelfShadowing;
	XDOM::IXMLDOMNodePtr pCastShadowMaps;
	XDOM::IXMLDOMNodePtr pRecvShadowMaps;
	XDOM::IXMLDOMNodePtr pMaterial;
	XDOM::IXMLDOMNodePtr pPreCalcShadows;
	XDOM::IXMLDOMNodePtr pScale;
  XDOM::IXMLDOMNodePtr pViewDistRatio;
  XDOM::IXMLDOMNodePtr pLodRatio;
  XDOM::IXMLDOMNodePtr pUpdateVisLevel;
	XDOM::IXMLDOMNodePtr pPhysicalState;
	XDOM::IXMLDOMNodePtr pHiddenInGame;
	XDOM::IXMLDOMNodePtr pSkipOnLowSpec;

	pEntityClass=pNode->getAttribute("EntityClass");
	pName=pNode->getAttribute("Name");
	pPos=pNode->getAttribute("Pos");
	pAngles=pNode->getAttribute("Angles");
	pId=pNode->getAttribute("EntityId");
	pParentId=pNode->getAttribute("ParentId");
	pCastShadowVolume = pNode->getAttribute("CastShadows");				
	pSelfShadowing = pNode->getAttribute("SelfShadowing");				
	pCastShadowMaps = pNode->getAttribute("CastShadowMaps");				
	pRecvShadowMaps = pNode->getAttribute("RecvShadowMaps");				
	pMaterial = pNode->getAttribute("Material");				
	pPreCalcShadows = pNode->getAttribute("PreCalcShadows");
	pScale = pNode->getAttribute("Scale");
  pViewDistRatio = pNode->getAttribute("ViewDistRatio");
  pLodRatio = pNode->getAttribute("LodRatio");
  pUpdateVisLevel = pNode->getAttribute("UpdateVisLevel");
	pPhysicalState = pNode->getAttribute("PhysicsState");
	pHiddenInGame = pNode->getAttribute("HiddenInGame");
	pSkipOnLowSpec = pNode->getAttribute("SkipOnLowSpec"); 


	// [marco] check if this entity should be skipped on low spec config
	if (!m_pGame->IsMultiplayer()) // [marco] not in multiplayer or it will screw up on different machines config!
	{	
#if !defined(LINUX64)
		if (pSkipOnLowSpec!=NULL && (atoi(pSkipOnLowSpec->getText()) > 0))
#else
		if (pSkipOnLowSpec!=0 && (atoi(pSkipOnLowSpec->getText()) > 0))
#endif
		{
			ICVar *pCvar=m_pConsole->GetCVar("sys_skiponlowspec");
			if (pCvar && pCvar->GetIVal()) return true;
		}
	}
#if !defined(LINUX64)
	if((pEntityClass!=NULL) && (pName!=NULL) && (pPos!=NULL))
#else
	if((pEntityClass!=0) && (pName!=0) && (pPos!=0))
#endif
	{
//		TRACE("entity instance desc %s %s %s",pEntityClass->getText(),pName->getText(),pPos->getText());
		EntityClass *pClass=m_pGame->GetClassRegistry()->GetByClass(pEntityClass->getText());
		if (pClass)
		{
			_SmartScriptObject pScriptTable(m_pGame->GetScriptSystem(),true);
			m_pGame->GetScriptSystem()->GetGlobalValue(pClass->strClassName.c_str(),pScriptTable);
			ICVar *pDetailVar = m_pConsole->GetCVar("e_EntitySuppressionLevel");
			int nDetailID;
			if (pScriptTable->GetValue("ENTITY_DETAIL_ID",nDetailID))
			{
				if (nDetailID<pDetailVar->GetIVal())
					return false;
			}
			CEntityDesc ed(0,pClass->ClassId);

			if(!m_pGame->IsLoadingLevelFromFile())
			{							
				ed.name = pName->getText();
#if !defined(LINUX64)
				if (pPos != NULL)
#else
				if (pPos != 0)
#endif
				{
					ed.pos = StringToVector(pPos->getText());
				}
				ed.netPresence = false;
#if !defined(LINUX64)
				if(pId!=NULL)
#else
				if(pId!=0)
#endif
				{
					ed.id=atoi(pId->getText());
				}
#if !defined(LINUX64)
				if(pAngles!=NULL)
#else
				if(pAngles!=0)
#endif
				{
					Vec3 vAngles=StringToVector(pAngles->getText());
					ed.angles = vAngles;
				}
				else
					ed.angles = Vec3(0,0,0);
#if !defined(LINUX64)
				if (pScale != NULL)
#else
				if (pScale != 0)
#endif
				{
					Vec3 scale(1,1,1);
					scale = StringToVector(pScale->getText());
					ed.scale = scale.x;
				}

				//XDOM::IXMLDOMNodeListPtr pProps =  pNode->getElementsByTagName("Properties");
				//if (pProps)
				ed.pUserData = (void *) pNode;
				IEntity *entity = SpawnEntity(ed);							
				if (!entity)
				{
					GameWarning("!Could not load entity '%s'", ed.name.c_str());
					return (false);
				}
				m_setLevelEntities.insert(entity->GetId());

				// shadow volumes
#if !defined(LINUX64)
				if (pCastShadowVolume != NULL)
#else
				if (pCastShadowVolume != 0)
#endif
				{
					if ( atoi(pCastShadowVolume->getText()) != 0) 
						entity->SetRndFlags(ERF_CASTSHADOWVOLUME,true);
				}
#if !defined(LINUX64)
				if (pSelfShadowing != NULL)
#else
				if (pSelfShadowing != 0)
#endif
				{
					if ( atoi(pSelfShadowing->getText()) != 0) 
						entity->SetRndFlags(ERF_SELFSHADOW,true);
				}

				// shadow maps
#if !defined(LINUX64)
				if (pCastShadowMaps != NULL)
#else
				if (pCastShadowMaps != 0)
#endif
				{
					if ( atoi(pCastShadowMaps->getText()) != 0) 
						entity->SetRndFlags(ERF_CASTSHADOWMAPS,true);
				}
#if !defined(LINUX64)
				if (pRecvShadowMaps != NULL)
#else
				if (pRecvShadowMaps != 0)
#endif
				{
					if ( atoi(pRecvShadowMaps->getText()) != 0) 
						entity->SetRndFlags(ERF_RECVSHADOWMAPS,true);
				}
#if !defined(LINUX64)
				if (pPreCalcShadows != NULL)
#else
				if (pPreCalcShadows != 0)
#endif
				{
					if ( atoi(pPreCalcShadows->getText()) != 0) 
						entity->SetRndFlags(ERF_CASTSHADOWINTOLIGHTMAP,true);
				}
#if !defined(LINUX64)
				if (pMaterial != NULL)
#else
				if (pMaterial != 0)
#endif
				{
					IMatInfo *pMtl = m_pSystem->GetI3DEngine()->FindMaterial( pMaterial->getText() );
					if (pMtl)
					{
						entity->SetMaterial( pMtl );
					}
				}
#if !defined(LINUX64)
				if(pParentId!=NULL)
#else
				if(pParentId!=0)
#endif
				{
					int parentId = atoi(pParentId->getText());
					m_ChildParentMap[ed.id] = parentId;
				}

				XDOM::IXMLDOMNodeListPtr pEventTargets =  pNode->getElementsByTagName("EventTargets");
				if (pEventTargets)
				{
					pEventTargets->reset();
					XDOM::IXMLDOMNodePtr pEventTargetsNode;
					while (pEventTargetsNode = pEventTargets->nextNode())
					{
						SetEntityEvents( entity,pEventTargetsNode->getChildNodes() );
					}
				}

        if(pViewDistRatio)
          entity->SetViewDistRatio((int)atof(pViewDistRatio->getText()));

        if(pLodRatio)
          entity->SetLodRatio((int)atof(pLodRatio->getText()));

				if (pPhysicalState)
				{
					const char *str = pPhysicalState->getText();
					entity->SetPhysicsState( str );
				}

        if(pUpdateVisLevel)
          entity->SetUpdateVisLevel((EEntityUpdateVisLevel)atoi(pUpdateVisLevel->getText()));

				// Entity initially hidden in the game.
				if (pHiddenInGame)
				{
					if (atoi(pHiddenInGame->getText()) > 0)
					entity->Hide(true);
				}

				return (true);
      }
		}
	}

	return (false);
}

//////////////////////////////////////////////////////////////////////////
void CXSystemBase::SetEntityProperties( IEntity *entity,XDOM::IXMLDOMNode * pEntityTag )
{
	
	// setup first property table
	{
		XDOM::IXMLDOMNodeListPtr pProps =  pEntityTag->getElementsByTagName("Properties");
    if (pProps)
    {
      _SmartScriptObject pObj(m_pGame->GetScriptSystem(),true);
      entity->GetScriptObject()->GetValue("Properties",*pObj);

      XDOM::IXMLDOMNodePtr pPropNode;
      pProps->reset();
      while (pPropNode = pProps->nextNode())
      {	
        XDOM::IXMLDOMNodeListPtr pAttrList = pPropNode->getChildNodes();
        RecursiveSetEntityProperties(&pObj, pAttrList);
      }
    }
	}

	// set up second property table
	{
		XDOM::IXMLDOMNodeListPtr pProps =  pEntityTag->getElementsByTagName("Properties2");
		if (pProps)
		{
			_SmartScriptObject pObj(m_pGame->GetScriptSystem(),true);
			entity->GetScriptObject()->GetValue("PropertiesInstance",*pObj);

			XDOM::IXMLDOMNodePtr pPropNode;
			pProps->reset();
			while (pPropNode = pProps->nextNode())
			{	
				XDOM::IXMLDOMNodeListPtr pAttrList = pPropNode->getChildNodes();
				RecursiveSetEntityProperties(&pObj, pAttrList);
			}
		}
	}
	entity->SendScriptEvent(ScriptEvent_Reset,0);
}

void CXSystemBase::RecursiveSetEntityProperties(_SmartScriptObject *pRoot, XDOM::IXMLDOMNodeList* pAttrList)
{
		if (pAttrList)
		{
			XDOM::IXMLDOMNodePtr pAttribute;
			pAttrList->reset();
			while (pAttribute=pAttrList->nextNode())
			{
				if (pAttribute->getNodeType() == XDOM::NODE_ATTRIBUTE)
				{
					const XMLCHAR *str = pAttribute->getText();
					if (stricmp(str,"true") == 0) // handle boolean.
						str = "1";
					
					if ((*pRoot)->GetValueType(pAttribute->getName()) == svtNumber)
							(* pRoot)->SetValue(pAttribute->getName(), (float) atof(str));
					else if ((*pRoot)->GetValueType(pAttribute->getName()) == svtString)
							(* pRoot)->SetValue(pAttribute->getName(),str );
					else if ((*pRoot)->GetValueType(pAttribute->getName()) == svtObject)
					{
						float f1,f2,f3;
						if (sscanf(str, "%f,%f,%f",&f1,&f2,&f3)== 3)
						{
							//f1 /= 255.0f;
							//f2 /= 255.0f;
							//f3 /= 255.0f;
							_SmartScriptObject pSubtable(m_pGame->GetScriptSystem(),true);
							(*pRoot)->GetValue(pAttribute->getName(),*pSubtable);
							// check if have x member.
							float temp;
							if (pSubtable->GetValue( "x",temp ))
							{
								pSubtable->SetValue("x",f1);
								pSubtable->SetValue("y",f2);
								pSubtable->SetValue("z",f3);
							}
							else
							{
								pSubtable->SetAt(1,f1);
								pSubtable->SetAt(2,f2);
								pSubtable->SetAt(3,f3);
							}
						}
					}
					else
						m_pLog->Log("[LEVELDATA:WARNING] Property  %s found in property table that was not string, or float, but yet no subtable. SKIPPED!!",pAttribute->getName());
						
				}
				else if (pAttribute->getNodeType() == XDOM::NODE_ELEMENT)
				{
					XDOM::IXMLDOMNodeListPtr pRecurse = pAttribute->getChildNodes();
					pRecurse->reset();

					_SmartScriptObject pObj(m_pGame->GetScriptSystem(),true);
					if ((* pRoot)->GetValue(pAttribute->getName(), pObj))
						RecursiveSetEntityProperties(&pObj, pRecurse);
				}
			}
		}
	
}

//////////////////////////////////////////////////////////////////////////
void CXSystemBase::SetEntityEvents( IEntity *entity,XDOM::IXMLDOMNodeList* pEventsNode)
{
	eventTargetVec eventTargets;
	
	XDOM::IXMLDOMNodePtr pEventTargetNode;
	pEventsNode->reset();
	while (pEventTargetNode = pEventsNode->nextNode())
	{
		XDOM::IXMLDOMNodePtr pEventNode = pEventTargetNode->getAttribute( "Event" );
		XDOM::IXMLDOMNodePtr pTargetNode = pEventTargetNode->getAttribute( "Target" );
		XDOM::IXMLDOMNodePtr pSourceEventNode = pEventTargetNode->getAttribute( "SourceEvent" );
		if (!pEventNode)
			continue;
		if (!pTargetNode)
			continue;
		if (!pSourceEventNode)
			continue;
		
		EntityEventTarget et;
		et.target = atoi(pTargetNode->getText());
		et.event = pEventNode->getText();
		et.sourceEvent = pSourceEventNode->getText();
		eventTargets.push_back(et);
	}
	
	IScriptObject *scriptObject = entity->GetScriptObject();
	if (!scriptObject)
		return;

	IScriptSystem *scriptSystem = m_pGame->GetScriptSystem();
	_SmartScriptObject pEvents( scriptSystem,false );

	scriptObject->SetValue( "Events",*pEvents );
	
	eventTargetVecIt i;
	std::set<string> sourceEvents;
	for (i=eventTargets.begin();i!=eventTargets.end();i++)
	{
		EntityEventTarget &event=(*i);
		sourceEvents.insert(event.sourceEvent);
	}


	for (std::set<string>::iterator it = sourceEvents.begin(); it != sourceEvents.end(); it++)
	{
		_SmartScriptObject pTrgEvents( scriptSystem,false );

		string sourceEvent = *it;

		pEvents->SetValue( sourceEvent.c_str(),*pTrgEvents );

		// Put target events to table.
		int trgEventIndex = 1;
		for (i=eventTargets.begin();i!=eventTargets.end();i++)
		{
			EntityEventTarget &et=(*i);			
			if (et.sourceEvent == sourceEvent)
			{
				_SmartScriptObject pTrgEvent( scriptSystem,false );

				pTrgEvents->SetAt( trgEventIndex,*pTrgEvent );
				trgEventIndex++;
				//pTrgEvent->SetAt( 1,et.target.c_str() );
				pTrgEvent->SetAt( 1,et.target );
				pTrgEvent->SetAt( 2,et.event.c_str() );
			}
		}
	}	

}

//////////////////////////////////////////////////////////////////////////
void CXSystemBase::StartLoading(bool bEditor)
{
	IConsole *pConsole=m_pConsole;
	IRenderer *pRenderer=m_pSystem->GetIRenderer();
	m_pPrevConsoleImg=pConsole->GetImage();

	//they wanna change it from script
	/*
	m_pLoadingImg=pRenderer->EF_LoadTexture("Textures\\Console\\LoadingConsole",FT_NOREMOVE,0,eTT_Base);
	if (m_pLoadingImg)
	{
  	pConsole->SetImage(m_pLoadingImg);
		pConsole->StaticBackground(true);
	}
	*/
	pConsole->StaticBackground(true);

	//////////////////////////////////////////////////////////////////////////
	// Silence everything.
	//////////////////////////////////////////////////////////////////////////
	if (m_pSystem->GetISoundSystem())
		m_pSystem->GetISoundSystem()->Silence();
	if (m_pSystem->GetIMusicSystem())
		m_pSystem->GetIMusicSystem()->Silence();
	//////////////////////////////////////////////////////////////////////////
	
	if (!bEditor)
	{
		if(m_pSystem->GetISoundSystem())
			m_pSystem->GetISoundSystem()->Mute(true);

		m_pGame->DeleteMessage("Switch"); // no switching during loading
	}
}

void CXSystemBase::EndLoading(bool bEditor)
{
	IConsole *pConsole=m_pConsole;
	IRenderer *pRenderer=m_pSystem->GetIRenderer();

	if (m_pPrevConsoleImg && m_pPrevConsoleImg!=pConsole->GetImage())
		pConsole->SetImage(m_pPrevConsoleImg,true);

	pConsole->StaticBackground(false);

	if (!bEditor)
		if(m_pSystem->GetISoundSystem())
			m_pSystem->GetISoundSystem()->Mute(false);

  pRenderer->PostLoad();
	if(!bEditor)
	{
		m_pEntitySystem->SetPrecacheResourcesMode( true );
		m_pSystem->GetI3DEngine()->OnLevelLoaded();
		m_pEntitySystem->SetPrecacheResourcesMode( false );
	}

	m_pSystem->GetITimer()->Reset();	// reset timer (cause problems?)

	m_pSystem->GetITimer()->Update();	// refresh frametime - because the former frame was used for loading
	m_pSystem->UpdateScriptSink();		// update _time and _frametime

	// Reset system Camera, (This camera will not render anything until set to correct values)
	m_pSystem->GetViewCamera().SetPos( Vec3(0,0,0) );
	m_pSystem->GetViewCamera().SetAngle( Vec3(0,0,0) );

	//will be removed from script
	//pRenderer->RemoveTexture(m_pLoadingImg);
	m_pLoadingImg=NULL;
}

void CXSystemBase::BindChildren()
{
	for(std::map< int, int >::const_iterator	itr=m_ChildParentMap.begin(); itr!=m_ChildParentMap.end(); itr++)
	{
		IEntity*	parent =(IEntity *) CXSystemBase::GetEntity(itr->second);
		if (!parent)
			continue;
		// [kirill] need to keep the piosition - it's actually relative position
		IEntity*	child =(IEntity *) CXSystemBase::GetEntity(itr->first);
		Vec3	pos = child->GetPos();
		parent->Bind(itr->first);
//		child->SetPos( Vec3(0,0,0), false );
		child->SetPos( pos, false );

	}
//	m_ChildParentMap.clear();
}

//////////////////////////////////////////////////////////////////////////
void CXSystemBase::GetMission( XDOM::IXMLDOMDocument *doc,const char *sRequestedMission,SMissionInfo &missionInfo )
{
	XDOM::IXMLDOMDocumentPtr pDoc=doc;
	XDOM::IXMLDOMNodeListPtr pNodes;
	//pDoc=m_pSystem->CreateXMLDocument();

	missionInfo.dwProgressBarRange = 0;

	int missionsfound = 0;

	char lastmission[256];
#if !defined(LINUX64)
	if(pDoc!=NULL)
#else
	if(pDoc!=0)
#endif
	{
		XDOM::IXMLDOMNodeListPtr pMissionTagList;
		XDOM::IXMLDOMNodePtr pMissionTag;
		pMissionTagList = pDoc->getElementsByTagName("Missions");
		if (pMissionTagList)
		{				
			pMissionTagList->reset();
			pMissionTag = pMissionTagList->nextNode();
			XDOM::IXMLDOMNodeListPtr pMissionList;
			pMissionList = pMissionTag->getElementsByTagName("Mission");
			if (pMissionList)
			{
				pMissionList->reset();
				XDOM::IXMLDOMNodePtr pMission;
				while (pMission = pMissionList->nextNode())
				{						 
					// load mission script, if such exists
					XDOM::IXMLDOMNodePtr pName = pMission->getAttribute("Name");
					XDOM::IXMLDOMNodePtr pProgressBarRange = pMission->getAttribute("ProgressBarRange");
					XDOM::IXMLDOMNodePtr pMissionFileName = pMission->getAttribute("File");
					if (pMissionFileName)
					{
						missionInfo.pMissionXML = m_pSystem->CreateXMLDocument();
						string sMissionFile = missionInfo.sLevelFolder + "/" + string(pMissionFileName->getText());
						missionInfo.sMissionFilename = sMissionFile;
						if (!missionInfo.pMissionXML->load(sMissionFile.c_str()))
						{
							missionInfo.pMissionXML = NULL;
						}
					}

					if (pName)
					{
						missionInfo.sMissionName = pName->getText();
						strcpy(lastmission,pName->getText());
					}
					if (pProgressBarRange)
						missionInfo.dwProgressBarRange = atoi(pProgressBarRange->getText());
					missionsfound++;

					if(stricmp(sRequestedMission,pName->getText())==0)
					{
						m_pLog->UpdateLoadingScreen("$3 Mission found: %s",lastmission);
						break;
					}
				}
			}
		}
	}

	if (missionsfound > 0)
	{
		return;
	}

	if (stricmp(sRequestedMission, missionInfo.sMissionName.c_str())!=0)
		m_pLog->Log("[ERROR] Map does not contain mission %s, using %s instead.",sRequestedMission,missionInfo.sMissionName.c_str());
}

//////////////////////////////////////////////////////////////////////////
// Do common for Client and Server things when opening a new level.
//////////////////////////////////////////////////////////////////////////
bool CXSystemBase::LoadLevelCommon( SMissionInfo &missionInfo )
{
	// Start time of level loading.
	CTimeValue time0 = m_pSystem->GetITimer()->GetCurrTimePrecise();
	AutoSuspendTimeQuota AutoSuspender(m_pSystem->GetStreamEngine());
		
	string sPreviousLevelFolder = m_pGame->m_currentLevelFolder;
	m_pGame->m_currentLevel = missionInfo.sLevelName;
	m_pGame->m_currentMission = missionInfo.sMissionName;
	m_pGame->m_currentLevelFolder = missionInfo.sLevelFolder;

	// Make level loading image. (ex: Levels\Training\loadscreen_training.dds)
	string sLoadingScreenTexture = missionInfo.sLevelFolder + "/loadscreen_" + missionInfo.sLevelName + ".dds";

	m_pSystem->GetIConsole()->Clear();
	m_pSystem->GetIConsole()->SetScrollMax(600);
	m_pSystem->GetIConsole()->ShowConsole(true);

	m_pSystem->GetIConsole()->SetLoadingImage( sLoadingScreenTexture.c_str() );
	m_pSystem->GetIConsole()->ResetProgressBar(0x7fffffff);
	m_pSystem->GetILog()->UpdateLoadingScreen("");	// just to draw the console

	if (missionInfo.bEditor)
	{
		//////////////////////////////////////////////////////////////////////////
		if (m_pSystem->GetIProcess()->GetFlags() != PROC_3DENGINE)
		{
			m_pSystem->SetIProcess(m_pGame->m_p3DEngine);
			m_pSystem->GetIProcess()->SetFlags(PROC_3DENGINE);
		}
		m_pGame->Reset();

		OnReadyToLoadLevel( missionInfo );

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		// Start loading common stuff.
		//////////////////////////////////////////////////////////////////////////

		//init the entity registry
		InitRegistry( missionInfo.sLevelName.c_str() );
		LoadLanguageTable( missionInfo.sLevelName.c_str(),missionInfo.sMissionName.c_str() );

		///////////////////////////////////////////////////////////////////////////////////////
		// INITIALIZE AI SYSTEM
		IAISystem *pAISystem = m_pSystem->GetAISystem();	
		if (pAISystem)
		{
			pAISystem->Init(m_pSystem, missionInfo.sLevelName.c_str(), missionInfo.sMissionName.c_str() );
			IScriptSystem *pScriptSystem = m_pGame->GetScriptSystem();
			if (!pScriptSystem->ExecuteFile("Scripts/AI/aiconfig.lua"))
			{
				GameWarning( "[AISYSTEM] Cannot load AI CONFIGURATION FILE" );
			}
			//////////////////////////////////////////////////////////////////////////
			// Initialize AI Autobalance.
			//////////////////////////////////////////////////////////////////////////
			pAISystem->GetAutoBalanceInterface()->SetMultipliers(
				m_pGame->cv_game_Accuracy->GetFVal(),
				m_pGame->cv_game_Aggression->GetFVal(),
				m_pGame->cv_game_Health->GetFVal()
				);
		}

		// Init Weapon system.
		if (m_pGame->GetWeaponSystemEx())
			m_pGame->GetWeaponSystemEx()->Init(m_pGame, false);

		return true;
	}

	// Open Paks for this level.
	string sPaks = missionInfo.sLevelFolder + "/*.pak";
	// Open Pak file for this level. 
	if (!m_pGame->OpenPacks(sPaks.c_str()))
	//if (!m_pSystem->GetIPak()->OpenPacks( sPaks.c_str() ))
	{
		// Pak1 not found.
		//CryWarning( VALIDATOR_MODULE_GAME,VALIDATOR_WARNING,"Level Packs %s Not Found",sPaks.c_str() );
		// try to open from the mod folder, if any
	}

	//	m_pSystem->GetISoundSystem()->SetMasterVolume(0);
	string sEPath = missionInfo.sLevelFolder + "/LevelData.xml";
	missionInfo.pLevelDataXML = m_pSystem->CreateXMLDocument();
	if(!missionInfo.pLevelDataXML->load(sEPath.c_str()))
	{
		m_pLog->Log("[ERROR] Cannot Load %s",sEPath.c_str());
		return false;
	}
	string sMissionName = missionInfo.sMissionName;

	if (sMissionName.empty())
		GetMission(missionInfo.pLevelDataXML,"",missionInfo);
	else
		GetMission(missionInfo.pLevelDataXML,sMissionName.c_str(),missionInfo);

	// No mission XML.
	if (!missionInfo.pMissionXML)
	{
		GameWarning( "No mission XML File!" );
		return false;
	}

	// At this point Mission name could have changed, if requeste done wasnt found.
	m_pGame->m_currentMission = missionInfo.sMissionName;

	missionInfo.m_dwLevelDataCheckSum = missionInfo.pLevelDataXML->getCheckSum();
	missionInfo.m_dwMissionCheckSum = missionInfo.pMissionXML->getCheckSum();
	m_wCheckSum = missionInfo.m_dwLevelDataCheckSum + missionInfo.m_dwMissionCheckSum;

	//////////////////////////////////////////////////////////////////////////
	// Reset console.
	//////////////////////////////////////////////////////////////////////////
	if (missionInfo.dwProgressBarRange == 0)
		missionInfo.dwProgressBarRange = 500;

	unsigned int dwProgressBarRange = missionInfo.dwProgressBarRange;
	if (missionInfo.bEditor)
	{
		if (dwProgressBarRange > 50) 
			dwProgressBarRange -= 50; // Editor uses ~50 not level cgfs.
	}
	else
	{	
		// add 15% of loading screen to compensate for the time needed at the end of loading
		dwProgressBarRange+=(int)((float)(dwProgressBarRange)*0.15f);
	}

	m_pSystem->GetIConsole()->ResetProgressBar(dwProgressBarRange);
	m_pSystem->GetIConsole()->TickProgressBar();

	//////////////////////////////////////////////////////////////////////////
	if (m_pGame->m_pClient && m_pSystem->GetIProcess()->GetFlags()!=PROC_3DENGINE)
	{
		m_pSystem->SetIProcess(m_pGame->m_p3DEngine);
		m_pSystem->GetIProcess()->SetFlags(PROC_3DENGINE);
	}

	//////////////////////////////////////////////////////////////////////////
	// Reset old stuff.
	//////////////////////////////////////////////////////////////////////////
	m_pGame->Reset();

	m_pLog->Log("missionInfo.sLevelFolder=%s",missionInfo.sLevelFolder.c_str());		// debug
	OnReadyToLoadLevel( missionInfo );

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Start loading common stuff.
	//////////////////////////////////////////////////////////////////////////
	//load the materials names
	if(!LoadMaterials(missionInfo.pLevelDataXML))
		return false;

	// reload the previously unloaded models since the materials are now reloaded
	if (m_pGame->m_pUISystem)
	{
		m_pGame->m_pUISystem->ReloadAllModels();
	}

	//init the entity registry
	InitRegistry( missionInfo.sLevelName.c_str() );
	LoadLanguageTable( missionInfo.sLevelName.c_str(),missionInfo.sMissionName.c_str() );

	//////////////////////////////////////////////////////////////////////////
	// Start by loading a level in 3D Engine.
	//////////////////////////////////////////////////////////////////////////
	if (!m_pGame->m_p3DEngine->LoadLevel( missionInfo.sLevelFolder.c_str(),missionInfo.sMissionName.c_str() ))
	{
		return false;
	}
	//////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////
	// INITIALIZE AI SYSTEM
	IAISystem *pAISystem = m_pSystem->GetAISystem();	
	if (pAISystem)
	{
		pAISystem->Init(m_pSystem, missionInfo.sLevelName.c_str(), missionInfo.sMissionName.c_str() );
		IScriptSystem *pScriptSystem = m_pGame->GetScriptSystem();
		if (!pScriptSystem->ExecuteFile("Scripts/AI/aiconfig.lua"))
		{
			GameWarning( "[AISYSTEM] Cannot load AI CONFIGURATION FILE" );
		}
		//////////////////////////////////////////////////////////////////////////
		// Initialize AI Autobalance.
		//////////////////////////////////////////////////////////////////////////
		pAISystem->GetAutoBalanceInterface()->SetMultipliers(
			m_pGame->cv_game_Accuracy->GetFVal(),
			m_pGame->cv_game_Aggression->GetFVal(),
			m_pGame->cv_game_Health->GetFVal()
			);
	}

	// Init Weapon system.
	if (m_pGame->GetWeaponSystemEx())
		m_pGame->GetWeaponSystemEx()->Init(m_pGame, false);

	//////////////////////////////////////////////////////////////////////////
	// Load Movie Data.
	//////////////////////////////////////////////////////////////////////////
	string sMovieDataXml = missionInfo.sLevelFolder + "/moviedata.xml";
	if (m_pSystem->GetIMovieSystem())
		m_pSystem->GetIMovieSystem()->Load( sMovieDataXml.c_str(),missionInfo.sMissionName.c_str() );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Load level entities.
	//////////////////////////////////////////////////////////////////////////
	//load the entities from leveldata.xml
	if (!LoadLevelEntities( missionInfo ))
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Triangulation must be loaded after loading of entities.
	//////////////////////////////////////////////////////////////////////////
	if (pAISystem)
		pAISystem->LoadTriangulation( missionInfo.sLevelFolder.c_str(),missionInfo.sMissionName.c_str() );

	//////////////////////////////////////////////////////////////////////////
	// Load Level Music.
	//////////////////////////////////////////////////////////////////////////
	LoadMusic( missionInfo );

	// Set global console variable to current level.
	m_pGame->g_LevelName->Set(missionInfo.sLevelName.c_str());

	//////////////////////////////////////////////////////////////////////////
	// Close paks opened before, for previous level.
	//////////////////////////////////////////////////////////////////////////
	if (stricmp(sPreviousLevelFolder.c_str(),missionInfo.sLevelFolder.c_str()) != 0)
	{ 
		if (!sPreviousLevelFolder.empty())
		{		
			//m_pLog->Log("PREVIOUSLEVEL:%s,missionlevelfolder=%s,%s",sPreviousLevelFolder.c_str(),missionInfo.sLevelFolder.c_str());
			string sClosePaks = sPreviousLevelFolder + "/*.pak";
			// Open Pak file for this level.
			//m_pSystem->GetIPak()->ClosePacks( sClosePaks.c_str() );
			m_pGame->ClosePacks( sClosePaks.c_str() );		
		}
	}
	//////////////////////////////////////////////////////////////////////////

	if (m_pGame->IsMultiplayer())
		AddMPProtectedFiles( missionInfo );

	// hack, to get the progressbar to the end...
	for (int i = 0; i < (int)(dwProgressBarRange); i++)
		m_pConsole->TickProgressBar();

	//////////////////////////////////////////////////////////////////////////
	// Log to file level loading time.
	//////////////////////////////////////////////////////////////////////////
	CTimeValue timeLoad = m_pSystem->GetITimer()->GetCurrTimePrecise() - time0;
	// Log level load times.
	m_pLog->LogToFile( "\001 Level %s loaded in %.3f seconds",missionInfo.sLevelName.c_str(),timeLoad.GetSeconds() );
	//////////////////////////////////////////////////////////////////////////

	m_pGame->GetSystem()->GetIEntitySystem()->PauseTimers(false,true);	

	return true;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CXSystemBase::LoadMusic( SMissionInfo &musicInfo )
{
	// First load music from local file if it is exist.
	IMusicSystem *pMusicSystem = GetISystem()->GetIMusicSystem();

	if(!pMusicSystem)
		return;									// there might be no music system e.g. dedicated server

	string szLevelDir = musicInfo.sLevelFolder;
	string sFilename = szLevelDir + "/Music/Level.xml";

	pMusicSystem->Pause(true);
	bool bLevelMusicExist = false;
	{
		FILE *file = GetISystem()->GetIPak()->FOpen(sFilename.c_str(),"rb");
		if (file)
		{
			bLevelMusicExist = true;
			GetISystem()->GetIPak()->FClose(file);
		}
	}
	if (bLevelMusicExist)
		pMusicSystem->LoadFromXML( sFilename.c_str(),false );


	XDOM::IXMLDOMNodeListPtr pLibsTagList;
	XDOM::IXMLDOMNodePtr pLibsTag;
	pLibsTagList = musicInfo.pLevelDataXML->getElementsByTagName("MusicLibrary");
	if (pLibsTagList)
	{
		pLibsTagList->reset();
		pLibsTag = pLibsTagList->nextNode();
		if (pLibsTag)
		{
			XDOM::IXMLDOMNodeListPtr pLibsList = pLibsTag->getElementsByTagName("Library");
			if (pLibsList)
			{
				pLibsList->reset();
				XDOM::IXMLDOMNodePtr pLib;
				while (pLib = pLibsList->nextNode())
				{						 
					// load mission script, if such exists
					XDOM::IXMLDOMNodePtr pFile = pLib->getAttribute("File");
					if (pFile)
					{
						//const char *sLibName = pName->getText();
						// Make file name from library name.
						//sFilename = string("Music\\") + sLibName + ".xml";
						sFilename = pFile->getText();
						sFilename = szLevelDir + "/Music/" + sFilename;
						CryLogComment( "Loading Music Library: %s",sFilename.c_str() );
						// Load music library.
						pMusicSystem->LoadFromXML( sFilename.c_str(),true );
					}
				}
			}
		}
	}
	pMusicSystem->Pause(false);
}


//////////////////////////////////////////////////////////////////////////
class CTableClone : public IScriptObjectDumpSink
{
public:
	CTableClone(IScriptSystem *pScriptSystem,IScriptObject *pDest,IScriptObject *pSrc)
	{
		m_pScriptSystem=pScriptSystem;
		m_pDest=pDest;
		m_pSrc=pSrc;
		m_pDest->Clone(m_pSrc);
	}
	void OnElementFound(int nIdx,ScriptVarType type){/*ignore non string indexed values*/};
	void OnElementFound(const char *sName,ScriptVarType type)
	{
		if(type==svtObject)
		{
			_SmartScriptObject pT(m_pScriptSystem,true);
			if(m_pSrc->GetValue(sName,pT))
			{
				_SmartScriptObject pNew(m_pScriptSystem);
				pNew->Clone(pT);
				m_pDest->SetValue(sName,(IScriptObject *)pNew);
			}
		}
	}
	IScriptObject *m_pDest;
	IScriptObject *m_pSrc;
	IScriptSystem *m_pScriptSystem;
};

void CXSystemBase::OnSpawn(IEntity *ent, CEntityDesc & ed)
{
	ILog *pLog=m_pLog;
	if (pLog->GetVerbosityLevel()>5)
		pLog->Log("Spawning entity classname=%s,name=%s,type=%d,id=%d",ed.className.c_str(),ed.name.c_str(),(int)ed.ClassId,ed.id);

	EntityClass *pClass=m_pGame->GetClassRegistry()->GetByClassId(ent->GetClassId());
	if (!pClass)
	{
		GameWarning( "Trying to spawn entity from an unknown class : %d",(int)ent->GetClassId() );
		return;
	}

	if (!pClass->strClassName.c_str())
	{
		GameWarning( "This entity class has no script TABLE defined : %d",(int)ent->GetClassId());
		return;
	}

	ent->SetClassName(pClass->strClassName.c_str());
	
	m_pSystem->CreateEntityScriptBinding(ent);

	// property table stuff
	// first clone the property table
	ASSERT(ent->GetScriptObject());
	{
		_SmartScriptObject pProperties(m_pGame->GetScriptSystem());
		_SmartScriptObject pTemp(m_pGame->GetScriptSystem(),true);
		if (ent->GetScriptObject()->GetValue("Properties",*pTemp))
		{
			CTableClone tc(m_pGame->GetScriptSystem(),pProperties,pTemp);
			pTemp->Dump(&tc);
			ent->GetScriptObject()->SetValue("Properties",*pProperties);
		}

		_SmartScriptObject pPropertiesInstance(m_pGame->GetScriptSystem());
		if (ent->GetScriptObject()->GetValue("PropertiesInstance",*pTemp))
		{
			CTableClone tc(m_pGame->GetScriptSystem(),pPropertiesInstance,pTemp);
			pTemp->Dump(&tc);
			ent->GetScriptObject()->SetValue("PropertiesInstance",*pPropertiesInstance);
		}
	}

	// then just parse out the properties
	if (ed.pUserData)
	{
		SetEntityProperties(ent, (XDOM::IXMLDOMNode*) ed.pUserData);
	}
	//<<FIXME>> aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaalberto
	if(ed.pProperties)
	{
		ent->GetScriptObject()->SetValue("Properties",ed.pProperties);
	}
	if(ed.pPropertiesInstance)
	{
		ent->GetScriptObject()->SetValue("PropertiesInstance",ed.pPropertiesInstance);
	}
	//SET THE entity_type/////////////////////////////////
	if (m_pGame->GetPlayerSystem()->IsPlayerClass(ent->GetClassId()))
	{
		ent->GetScriptObject()->SetValue("entity_type","player");
		m_setPlayerEntities.insert(EntitiesSetItor::value_type(ent->GetId()));
	}
	else if (m_pGame->GetVehicleSystem()->IsVehicleClass(ent->GetClassId()))  
	{
		ent->GetScriptObject()->SetValue("entity_type","vehicle");
	}
	else if (ent->GetClassId()==SPECTATOR_CLASS_ID)  
	{
		ent->GetScriptObject()->SetValue("entity_type","spectator");
	}
	else if (ent->GetClassId()==ADVCAMSYSTEM_CLASS_ID)  
	{
		ent->GetScriptObject()->SetValue("entity_type","advcamsystem");
	}
	else if (ent->GetClassId()==SYNCHED2DTABLE_CLASS_ID)  
	{
		ent->GetScriptObject()->SetValue("entity_type","synched2dtable");
	}
	else 
	{
		ent->GetScriptObject()->SetValue("entity_type","basic");
	}
	//////////////////////////////////////////////////////
}

void CXSystemBase::OnSpawnContainer( CEntityDesc &ed,IEntity *pEntity )
{

	ILog *pLog=m_pLog;
	if (pLog->GetVerbosityLevel()>5)
	{
		//Timur, Excessive log.
		//pLog->Log("Spawning container for entity classname=%s,name=%s,type=%d,id=%d,model=%s",ed.className.c_str(),ed.name.c_str(),(int)ed.ClassId,ed.id,ed.sModel.c_str());
	}

	if (ed.ClassId==SPECTATOR_CLASS_ID)
	{
//		if (pLog->GetVerbosityLevel()>5)
//			pLog->Log("Spawning SPECTATOR CLASS ID container");

		CSpectator *pSpectator=new CSpectator(m_pGame);
		pSpectator->SetEntity(pEntity);

		EntityClass *pClass=m_pGame->GetClassRegistry()->GetByClassId(ed.ClassId);
		FIXME_ASSERT(pClass);
		FIXME_ASSERT(pClass->strClassName.c_str());

		ed.className = pClass->strClassName;


		CScriptObjectSpectator *pSSpectator = new CScriptObjectSpectator();
		pSSpectator->Create(m_pGame->GetScriptSystem(),pSpectator);
		pSpectator->SetScriptObject(pSSpectator->GetScriptObject());
		pEntity->SetContainer(pSpectator);
	}
	else if (m_pGame->GetPlayerSystem()->IsPlayerClass(ed.ClassId))
	{
//		if (pLog->GetVerbosityLevel()>5)
//			pLog->Log("Spawning PLAYER CLASS ID container");

		// create player container
		CPlayer *pPlayer = new CPlayer(m_pGame);
		pPlayer->SetEntity(pEntity);


		// get the table name from the entity registry (like "Player")
		EntityClass *pClass=m_pGame->GetClassRegistry()->GetByClassId(ed.ClassId);
		FIXME_ASSERT(pClass);
		FIXME_ASSERT(pClass->strClassName.c_str());
		//ent->SetClassName(pClass->strClassName);
		ed.className = pClass->strClassName;

		// create the containers script object
		CScriptObjectPlayer *pSPlayer = new CScriptObjectPlayer();
		pSPlayer->Create(m_pGame->GetScriptSystem());
		pSPlayer->SetPlayer(pPlayer);
		pPlayer->SetScriptObject(pSPlayer->GetScriptObject());

		if(ed.sModel.length())
		{
			pPlayer->SetPlayerModel(ed.sModel.c_str());
		}
		else
		{
			if (m_pGame->IsMultiplayer())
			{
				pPlayer->SetPlayerModel(m_pGame->mp_model->GetString());
			}
			else
			{
				pPlayer->SetPlayerModel(m_pGame->p_model->GetString());
			}
		}

		pEntity->SetContainer(pPlayer);
		pPlayer->SetColor(ed.vColor);
	}
	else if (m_pGame->GetVehicleSystem()->IsVehicleClass(ed.ClassId))  
	{
//		if (pLog->GetVerbosityLevel()>5)
//			pLog->Log("Spawning VEHICLE CLASS ID CONTAINER");

		// create vehicle container
		CVehicle *pVehicle = new CVehicle(m_pGame);

		// get the script file name from the global table weapon system
		EntityClass *pClass=m_pGame->GetClassRegistry()->GetByClassId(ed.ClassId);
		if (!pClass)
			m_pLog->Log("[ERROR] Trying to spawn vehicle from an unknown class : %d",(int)ed.ClassId);
		FIXME_ASSERT(pClass);
		FIXME_ASSERT(pClass->strClassName.c_str());
		ed.className = pClass->strClassName;

		// create the containers script object 
		CScriptObjectVehicle *pSVehicle = new CScriptObjectVehicle();
		pSVehicle->Create(m_pGame->GetScriptSystem(),m_pSystem->GetIEntitySystem());
		pSVehicle->SetVehicle(pVehicle);  
		pVehicle->SetScriptObject(pSVehicle->GetScriptObject());
		pEntity->SetContainer(pVehicle);
		pEntity->SetNetPresence(true);
		ed.netPresence = true;
	}
	else if (ed.ClassId==ADVCAMSYSTEM_CLASS_ID)
	{
//		if (pLog->GetVerbosityLevel()>5)
//			pLog->Log("Spawning ADVCAM SYSTEM CLASS ID container");

		CAdvCamSystem *pAdvCamSystem=new CAdvCamSystem(m_pGame);
		pAdvCamSystem->SetEntity(pEntity);

		EntityClass *pClass=m_pGame->GetClassRegistry()->GetByClassId(ed.ClassId);
		FIXME_ASSERT(pClass);
		FIXME_ASSERT(pClass->strClassName.c_str());

		ed.className = pClass->strClassName;

		CScriptObjectAdvCamSystem *pSAdvCamSystem = new CScriptObjectAdvCamSystem();
		pSAdvCamSystem->Create(m_pGame->GetScriptSystem(),pAdvCamSystem);
		pAdvCamSystem->SetScriptObject(pSAdvCamSystem->GetScriptObject());
		pEntity->SetContainer(pAdvCamSystem);
	}
	else if (ed.ClassId==SYNCHED2DTABLE_CLASS_ID)
	{
		CSynched2DTable *pSynched2DTable=new CSynched2DTable(m_pGame);
		pSynched2DTable->SetEntity(pEntity);

		EntityClass *pClass=m_pGame->GetClassRegistry()->GetByClassId(ed.ClassId);
		FIXME_ASSERT(pClass);
		FIXME_ASSERT(pClass->strClassName.c_str());

		ed.className = pClass->strClassName;

		CScriptObjectSynched2DTable *pSSynched2DTable = new CScriptObjectSynched2DTable();
		pSSynched2DTable->Create(m_pGame->GetScriptSystem(),pSynched2DTable);
		pSynched2DTable->SetScriptObject(pSSynched2DTable->GetScriptObject());
		pEntity->SetContainer(pSynched2DTable);
	}
}

void CXSystemBase::OnRemove(IEntity *ent)
{
	if (m_pGame->GetPlayerSystem()->IsPlayerClass(ent->GetClassId()))
	{
		EntitiesSetItor It=m_setPlayerEntities.find(ent->GetId());
		if (It!=m_setPlayerEntities.end())
			m_setPlayerEntities.erase(It);
	}
}

//////////////////////////////////////////////////////////////////////////
void CXSystemBase::AddMPProtectedFiles( SMissionInfo &missionInfo )
{
	// Add protected files to defence wall.
	INetwork *pNetwork = m_pSystem->GetINetwork();
	if (pNetwork)
	{
		pNetwork->ClearProtectedFiles();
		// Files to be protected for this level.

		pNetwork->AddProtectedFile( string(missionInfo.sLevelFolder+"/LevelData.xml").c_str() );
		pNetwork->AddProtectedFile( missionInfo.sMissionFilename.c_str() );
	}
}

int CXSystemBase::GetTeamScore(int nTeamId)
{
	TeamsMapItor itor=m_mapTeams.find(nTeamId);
	return ((itor!=m_mapTeams.end())?itor->second.nScore:-1);
}

int CXSystemBase::GetTeamFlags(int nTeamId)
{
	TeamsMapItor itor=m_mapTeams.find(nTeamId);
	return ((itor!=m_mapTeams.end())?itor->second.nFlags:-1);
}

bool CXSystemBase::GetTeamName(int nTeamId,char *ret)
{
	TeamsMapItor itor=m_mapTeams.find(nTeamId);
	if(itor!=m_mapTeams.end())
	{
		assert(ret);
		assert(itor->second.sName.size()<256);
		strcpy(ret,itor->second.sName.c_str());
		return true;
	}
	return false;
}

int	CXSystemBase::GetEntityTeam(int nEntity)
{
	EntitiesSetItor eitr;
	for(TeamsMapItor itor=m_mapTeams.begin();itor!=m_mapTeams.end();++itor)
	{
		eitr=itor->second.m_setEntities.find(nEntity);
		if(eitr!=itor->second.m_setEntities.end())
		{
			return itor->second.nID;
		}
	}
	return -1;
}

int CXSystemBase::GetTeamId(const char *name)
{
	for(TeamsMapItor itor=m_mapTeams.begin();itor!=m_mapTeams.end();++itor)
	{
		if(itor->second.sName==name)
		{
			return itor->second.nID;
		}
	}
	return -1;
}

int CXSystemBase::GetTeamMembersCount(int nTeamId)
{
	TeamsMapItor itor=m_mapTeams.find(nTeamId);
	if(itor!=m_mapTeams.end())
	{
		return itor->second.m_setEntities.size();
	}
	return 0;
}

int CXSystemBase::AddTeam(const char *sTeam, int nTeamId)
{
	if(nTeamId==-1) //generate a new team id
	{
		TeamsMapItor itor;
		for(int i=1;i<MAXTEAMS;i++)
		{
			if(m_mapTeams.find(i)==m_mapTeams.end())
			{
				nTeamId=i;
				break;
			}
		}
	}
	Team t(nTeamId,sTeam,0,0);
	m_mapTeams.insert(TeamsMapItor::value_type(nTeamId,t));
	return nTeamId;
}

bool CXSystemBase::ReadTeams(CStream &stm)
{
	BYTE nteams;
	m_mapTeams.clear();
	stm.Read(nteams);
	Team t;
	for(int i=0;i<nteams;i++)
	{
		t.Read(stm);
		m_mapTeams.insert(TeamsMapItor::value_type(t.nID,t));
	}
	return true;
}

void CXSystemBase::SetTeam(EntityId nEntId, int nTeamId)
{
	TeamsMapItor itor;
	if (nTeamId==0xff)
	{
		//CTeam *pTeam=m_pGame->GetTeamManager()->GetEntityTeam(nEntId);
		itor=m_mapTeams.begin();
		while(itor!=m_mapTeams.end())
		{

			itor->second.m_setEntities.erase(nEntId);
			++itor;
		}
		//TRACE("WARNING: Team not found while synchronizing teams (Remove from team) !");
		return;
	}else
	{
		//		CTeam *pTeam=m_pGame->GetTeamManager()->GetTeam((int)nTeamId);
		//if (!pTeam)
		//{
		//TRACE("WARNING: Team not found while synchronizing teams (Add to team) !");
		//return;
		//}
		//pTeam->AddEntity(nEntId);
		TeamsMapItor itor=m_mapTeams.find(nTeamId);
		if(itor==m_mapTeams.end())
		{
			TRACE("WARNING: Team not found while synchronizing scores !");
			return;
		}
		itor->second.m_setEntities.insert(nEntId);

	}
}


void CXSystemBase::RemoveTeam(int nTeamId)
{
	if (nTeamId==0xff)
	{
		//m_pGame->GetTeamManager()->RemoveAllTeams();
		m_mapTeams.clear();
	}else
	{
		//m_pGame->GetTeamManager()->RemoveTeam(nTeamId);
		m_mapTeams.erase(nTeamId);
	}
}

void CXSystemBase::SetTeamScore(int nTeamId, short nScore)
{
	TeamsMapItor itor=m_mapTeams.find(nTeamId);
	if(itor==m_mapTeams.end())
	{
		TRACE("WARNING: Team not found while synchronizing scores !");
		return;
	}
	itor->second.nScore=nScore;
}

void CXSystemBase::SetTeamFlags(int nTeamId, int nFlags)
{
	TeamsMapItor itor=m_mapTeams.find(nTeamId);
	if(itor==m_mapTeams.end())
	{
		TRACE("WARNING: Team not found while synchronizing flags !");
		return;
	}
	itor->second.nFlags=nFlags;
}


bool Team::Write(CStream &stm)
{
	stm.Write((BYTE)nID);
	stm.Write((short)nScore);
	stm.Write((short)nFlags);
	stm.Write(sName);
	stm.Write((BYTE)m_setEntities.size());
	for(EntitiesSetItor itor=m_setEntities.begin();itor!=m_setEntities.end();++itor)
	{
		stm.Write((unsigned short)(*itor));
	}
	return true;
}

bool Team::Read(CStream &stm)
{
	m_setEntities.clear();
	BYTE t;
	short s;
	stm.Read(t);
	nID=t;
	stm.Read(s);
	nScore=s;
	stm.Read(s);
	nFlags=s;
	stm.Read(sName);
	stm.Read(t);
	unsigned short eid;
	for(int i=0;i<t;i++)
	{
		stm.Read(eid);
		m_setEntities.insert(eid);
	}
	return true;
}

