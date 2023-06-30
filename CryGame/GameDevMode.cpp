
//////////////////////////////////////////////////////////////////////
//
//	Game source code (c) Crytek 2001-2003
//	
//	File: GameMisc.cpp
//  
//	History:
//	-October	31,2003: created
//	
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

#include "Game.h"
#include "XNetwork.h"
#include "XServer.h"
#include "XClient.h"
#include "UIHud.h"
#include "XPlayer.h"
#include "PlayerSystem.h"
#include "XServer.h"
#include "WeaponSystemEx.h"
#include "ScriptObjectGame.h"
#include "ScriptObjectInput.h"
#include <IEntitySystem.h>

#include "UISystem.h"
#include "ScriptObjectUI.h"
#include "TimeDemoRecorder.h"

//////////////////////////////////////////////////////////////////////////
void CXGame::DevModeInit()
{
#ifdef WIN32
	for (int i = 0; i < 255; i++)
	{
		// Reset Async state of all keys.
		GetAsyncKeyState(i);
	}
#endif WIN32
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::IsDevModeEnable()
{
	// if there already a server we stick to the setting during creation
	if(IsMultiplayer())
	{
		if(m_pSystem->GetForceNonDevMode())
			return false;
	}

	// otherwise with get the info from the console variable
	if (!m_pCVarCheatMode || (strcmp(m_pCVarCheatMode->GetString(),"DEVMODE")!=0))
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CXGame::DevModeUpdate()
{
#ifdef WIN32

	m_pTimeDemoRecorder->Update();

	if(m_pSystem->GetIRenderer()->GetHWND() != ::GetActiveWindow())
		return;

	// Check if special developmnt keys where pressed.
	bool bCtrl = (GetAsyncKeyState(VK_CONTROL) & (1<<15)) != 0;
	bool bShift = (GetAsyncKeyState(VK_SHIFT) & (1<<15)) != 0;

	int key = 0;
	for (int i = 0; i < 8; i++)
	{
		if (GetAsyncKeyState(VK_F1+i)&1)
		{
			key = i+1;
			break;
		}
	}

	// try also old F9/F10 keys
	if (key == 0)
	{
		if(GetAsyncKeyState(VK_F9) & 1)
		{
			key = 1;
			bCtrl = true;
		}
		else if(GetAsyncKeyState(VK_F10) & 1)
		{
			key = 1;
			bShift = true;
		}
	}

	if (key != 0)
	{
		if (bCtrl)
		{
			// Save current player position.
			DevMode_SavePlayerPos(key-1);
		} else if (bShift)
		{
			// Load current player position.
			DevMode_LoadPlayerPos(key-1);
		}
	}

	bool bCancel = GetAsyncKeyState(VK_CANCEL) & 1;
	bool bTimeDemoKey = GetAsyncKeyState(VK_SNAPSHOT) & 1;

	if (bCancel)
	{
		if (m_pTimeDemoRecorder->IsRecording())
		{
			// Stop and save
			StopRecording();
		}
		// Toggle start/stop of demo recording.
		if (m_pTimeDemoRecorder->IsPlaying())
		{
			// Stop playing.
			StopDemoPlay();
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// Time demo on/off
	//////////////////////////////////////////////////////////////////////////
	if (bCtrl && bTimeDemoKey)
	{
		if (!m_pTimeDemoRecorder->IsRecording())
		{
			// Start record.
			StartRecording( g_timedemo_file->GetString() );
		}
	}
	if (bShift && bTimeDemoKey)
	{
		if (!m_pTimeDemoRecorder->IsPlaying())
		{
			// Load and start playing.
			StartDemoPlay( g_timedemo_file->GetString() );
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void CXGame::DevMode_SavePlayerPos( int index,const char *sTagName,const char *sDescription )
{
#ifdef WIN32
	if (index < 0 && index > 11)
		return;

	if (!IsDevModeEnable())
		return;

	Vec3 tagLocations[12];
	Vec3 tagAngles[12];
	memset( tagAngles,0,sizeof(tagAngles) );
	memset( tagLocations,0,sizeof(tagLocations) );

	String filename = m_currentLevelFolder + "/tags.txt";
	if (sTagName)
		filename = m_currentLevelFolder + "/" + sTagName + ".tagpoint";

	const char *desc = "";
	if (sDescription)
		desc = sDescription;

	// Load tag locations from file.
	FILE *f = fopen( filename.c_str(),"rt" ); // Dont change this to CryPak
	if (f)
	{
		for (int i = 0; i < 12; i++)
		{
			float x=0,y=0,z=0,ax=0,ay=0,az=0;
			fscanf( f,"%f,%f,%f,%f,%f,%f\n",&x,&y,&z,&ax,&ay,&az );
			tagLocations[i] = Vec3(x,y,z);
			tagAngles[i] = Vec3(ax,ay,az);
		}
		fclose(f);
	}

	tagLocations[index] = m_pSystem->GetViewCamera().GetPos();
	tagAngles[index] = m_pSystem->GetViewCamera().GetAngles();

	SetFileAttributes(filename.c_str(), 0);

	f = fopen( filename.c_str(),"wt" ); // Dont change this to CryPak
	if (f)
	{
		for (int i = 0; i < 12; i++)
		{
			fprintf( f,"%f,%f,%f,%f,%f,%f\n",
				tagLocations[i].x,tagLocations[i].y,tagLocations[i].z,
				tagAngles[i].x,tagAngles[i].y,tagAngles[i].z);
		}
		fprintf( f,"%s\n",desc );
		fclose(f);
	}
	else
	{
		GameWarning( "Cannot overwrite Tag point file %s not found (Check if read-only)",filename.c_str() );
	}

	if (sTagName && strlen(sTagName) > 0)
	{
		//////////////////////////////////////////////////////////////////////////
		// Also save to Editor supported comment object .grp.
		//////////////////////////////////////////////////////////////////////////
		XmlNodeRef root = m_pSystem->CreateXmlNode( "Objects" );
		XmlNodeRef node = root->newChild( "Object" );
		node->setAttr( "Pos",tagLocations[index] );
		node->setAttr( "Angles",tagAngles[index] );
		node->setAttr( "Type","Comment" );
		if (sDescription)
		{
			node->setAttr( "Name",sDescription );
			node->setAttr( "Comment",sDescription );
		}
		else
			node->setAttr( "Name",sTagName );

		//node->setAttr( "ColorRGB",RGB(255,0,0) );
		filename = m_currentLevelFolder + "/" + sTagName + ".grp";
		root->saveToFile( filename.c_str() );
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
void CXGame::DevMode_LoadPlayerPos( int index,const char *sTagName )
{
#ifdef WIN32
	if (index < 0 && index > 11)
		return;
	IEntity *pPlayer = GetMyPlayer();
	if (!pPlayer)
		return;

	if (!IsDevModeEnable())
		return;

	Vec3 tagLocations[12];
	Vec3 tagAngles[12];
	memset( tagAngles,0,sizeof(tagAngles) );
	memset( tagLocations,0,sizeof(tagLocations) );

	char desc[1024];
	String filename = m_currentLevelFolder + "/tags.txt";
	if (sTagName)
		filename = m_currentLevelFolder + "/" + sTagName + ".tagpoint";
	// Load tag locations from file.
	FILE *f = fopen( filename.c_str(),"rt" );	// Dont change this to CryPak
	if (f)
	{
		for (int i = 0; i < 12; i++)
		{
			strcpy( desc,"" );
			float x=0,y=0,z=0,ax=0,ay=0,az=0;
			fscanf( f,"%f,%f,%f,%f,%f,%f,%s\n",&x,&y,&z,&ax,&ay,&az,desc );
			tagLocations[i] = Vec3(x,y,z);
			tagAngles[i] = Vec3(ax,ay,az);
		}
		fscanf( f,"%s\n",desc );
		if (strlen(desc) > 0)
			GameWarning( "%s",desc );
		fclose(f);
	}
	else
	{
		GameWarning( "Tag file %s not found",filename.c_str() );
	}

	Vec3 p = tagLocations[index];
	Vec3 a = tagAngles[index];
	if (!p.IsZero())
	{
		m_pSystem->GetViewCamera().SetPos(p);
		pe_player_dimensions dim;
		if (pPlayer->GetPhysics())
		{
			dim.heightEye = 0;
			pPlayer->GetPhysics()->GetParams( &dim );
			p.z = p.z - dim.heightEye;
		}
		pPlayer->SetPos( p );
	}
	if (!a.IsZero())
	{
		m_pSystem->GetViewCamera().SetAngle(a);
		SetViewAngles( a );
	}
#endif
}
