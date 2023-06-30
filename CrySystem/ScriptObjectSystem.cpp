// ScriptObjectSystem.cpp: implementation of the CScriptObjectSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "System.h"
#include "ScriptObjectSystem.h"
#include <ICryAnimation.h>
#include "ScriptObjectEntity.h"
#include <ScriptObjectVector.h>
#include <IFont.h>
#include <ILog.h>
#include <IRenderer.h>
#include <I3DEngine.h>
#include <IInput.h>
#include <IEntitySystem.h>
#include <ITimer.h>
#include <IConsole.h>
#include <IAISystem.h>
#include <IAgent.h>
#include <ISound.h>
#include <IGame.h>									// IGame
#include <ICryPak.h>
#if !defined(LINUX)
	#include "ddraw.h"
#endif
#include "HTTPDownloader.h"
#include <time.h>

#ifdef WIN32
#include <ShellAPI.h>
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

_DECLARE_SCRIPTABLEEX(CScriptObjectSystem)

// modes of ScanDirectory function
#define SCANDIR_ALL 0
#define SCANDIR_FILES 1
#define SCANDIR_SUBDIRS 2

/*
 	nMode
 		R_BLEND_MODE__ZERO__SRC_COLOR					        1
		R_BLEND_MODE__SRC_COLOR__ZERO					        2
		R_BLEND_MODE__SRC_COLOR__ONE_MINUS_SRC_COLOR	3
		R_BLEND_MODE__SRC_ALPHA__ONE_MINUS_SRC_ALPHA	4
		R_BLEND_MODE__ONE__ONE                        5
		R_BLEND_MODE__DST_COLOR__SRC_COLOR            6
		R_BLEND_MODE__ZERO__ONE_MINUS_SRC_COLOR       7
		R_BLEND_MODE__ONE__ONE_MINUS_SRC_COLOR	      8
		R_BLEND_MODE__ONE__ZERO                       9
		R_BLEND_MODE__ZERO__ZERO                     10
		R_BLEND_MODE__ONE__ONE_MINUS_SRC_ALPHA       11
		R_BLEND_MODE__SRC_ALPHA__ONE                 12
		R_BLEND_MODE__ADD_SIGNED                     14
*/

static unsigned int sGetBlendState(int nMode)
{
  unsigned int nBlend;
  switch(nMode)
  {
    case 1:
      nBlend = GS_BLSRC_ZERO | GS_BLDST_SRCCOL;
      break;
    case 2:
    case 3:
      assert(0);
      break;
    case 4:
      nBlend = GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;
      break;
    case 5:
      nBlend = GS_BLSRC_ONE | GS_BLDST_ONE;
      break;
    case 6:
      nBlend = GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL;
      break;
    case 7:
      nBlend = GS_BLSRC_ZERO | GS_BLDST_ONEMINUSSRCCOL;
      break;
    case 8:
      nBlend = GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCCOL;
      break;
    case 9:
      nBlend = GS_BLSRC_ONE | GS_BLDST_ZERO;
      break;
    case 10:
      nBlend = GS_BLSRC_ZERO | GS_BLDST_ZERO;
      break;
    case 11:
      nBlend = GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCALPHA;
      break;
    case 12:
      nBlend = GS_BLSRC_SRCALPHA | GS_BLDST_ONE;
      break;
    case 14:
      nBlend = GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL;
      break;
    default:
      assert(0);
  }
  return nBlend;
}

IScriptObject* CScriptObjectSystem::m_pScriptTimeTable = NULL;

CScriptObjectSystem::CScriptObjectSystem()
{
	m_SkyFadeStart = 1000;
	m_SkyFadeEnd = 200;
}

CScriptObjectSystem::~CScriptObjectSystem()
{
}

/////////////////////////////////////////////////////////
/*! Initializes the script-object and makes it available for the scripts.
		@param pScriptSystem Pointer to the ScriptSystem-interface
		@param pSystem Pointer to the System-interface
		@see IScriptSystem
		@see ISystem
*/
void CScriptObjectSystem::Init(IScriptSystem *pScriptSystem, ISystem *pSystem)
{
	m_pSystem = pSystem;
	m_pLog = m_pSystem->GetILog();
	m_pSoundSytem = m_pSystem->GetISoundSystem();
	m_pRenderer = m_pSystem->GetIRenderer();
	m_pConsole = m_pSystem->GetIConsole();
	m_pTimer = m_pSystem->GetITimer();
	m_pEntitySystem = (IEntitySystem *) m_pSystem->GetIEntitySystem();
	m_p3DEngine =m_pSystem->GetI3DEngine();
	m_pPhysicalWorld =m_pSystem->GetIPhysicalWorld();

	pScriptSystem->SetGlobalValue("ENTITYPROP_CASTSHADOWS",		ENTITYPROP_CASTSHADOWS);
	pScriptSystem->SetGlobalValue("ENTITYPROP_DONOTCHECKVIS",	ENTITYPROP_DONOTCHECKVIS);

	pScriptSystem->SetGlobalValue("SCANDIR_ALL",SCANDIR_ALL);
	pScriptSystem->SetGlobalValue("SCANDIR_FILES",SCANDIR_FILES);
	pScriptSystem->SetGlobalValue("SCANDIR_SUBDIRS",SCANDIR_SUBDIRS);

	e_deformable_terrain = m_pSystem->GetIConsole()->GetCVar("e_deformable_terrain");

	InitGlobal(pScriptSystem,"System",this);
}


void CScriptObjectSystem::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectSystem>::InitializeTemplate(pSS);

	m_pScriptTimeTable = pSS->CreateObject();

	REG_FUNC(CScriptObjectSystem,CreateDownload);
	REG_FUNC(CScriptObjectSystem,LoadFont);
	REG_FUNC(CScriptObjectSystem,ExecuteCommand);
	REG_FUNC(CScriptObjectSystem,LogToConsole);
	REG_FUNC(CScriptObjectSystem,LogAlways);
	REG_FUNC(CScriptObjectSystem,ClearConsole);
	REG_FUNC(CScriptObjectSystem,GetConsoleKeyName);
	REG_FUNC(CScriptObjectSystem,Log);
	REG_FUNC(CScriptObjectSystem,Warning);
	REG_FUNC(CScriptObjectSystem,Error);
	REG_FUNC(CScriptObjectSystem,GetCurrTime);
	REG_FUNC(CScriptObjectSystem,GetCurrAsyncTime);
	REG_FUNC(CScriptObjectSystem,GetFrameTime);
	REG_FUNC(CScriptObjectSystem,GetLocalOSTime);
	//REG_FUNC(CScriptObjectSystem,PostMessage);
	REG_FUNC(CScriptObjectSystem,DrawLabelImage);
	REG_FUNC(CScriptObjectSystem,GetEntity);//<<FIXME>> move to server
	REG_FUNC(CScriptObjectSystem,GetEntities);//<<FIXME>> move to server
	REG_FUNC(CScriptObjectSystem,GetEntitiesInRadius);
	REG_FUNC(CScriptObjectSystem,GetTeamMembers);
	//REG_FUNC(CScriptObjectSystem,GetMyPlayer);
	REG_FUNC(CScriptObjectSystem,GetEntityByName);//<<FIXME>> remove
	REG_FUNC(CScriptObjectSystem,LoadAnimatedTexture);
	REG_FUNC(CScriptObjectSystem,LoadTexture);
	REG_FUNC(CScriptObjectSystem,LoadObject);
	REG_FUNC(CScriptObjectSystem,DrawSprite);
	REG_FUNC(CScriptObjectSystem,DeformTerrain);
	//REG_FUNC(CScriptObjectSystem,SetSurfacePairParameters);
	REG_FUNC(CScriptObjectSystem,ScreenToTexture);
	REG_FUNC(CScriptObjectSystem,LoadImage);
	REG_FUNC(CScriptObjectSystem,FreeImage);
//	REG_FUNC(CScriptObjectSystem,UnloadImage);
	REG_FUNC(CScriptObjectSystem,DrawLine);
	REG_FUNC(CScriptObjectSystem,Draw2DLine);
	REG_FUNC(CScriptObjectSystem,DrawImage);
	REG_FUNC(CScriptObjectSystem,DrawImageColor);
	REG_FUNC(CScriptObjectSystem,DrawImageCoords);
	REG_FUNC(CScriptObjectSystem,DrawImageColorCoords);
	REG_FUNC(CScriptObjectSystem,DrawTriStrip);
	REG_FUNC(CScriptObjectSystem,SetWorldColorRatio);
	REG_FUNC(CScriptObjectSystem,SetGammaDelta);
	REG_FUNC(CScriptObjectSystem,DrawRectShader);
	REG_FUNC(CScriptObjectSystem,SetScreenShader);

	REG_FUNC(CScriptObjectSystem,ShowConsole);

  // tiago: added
  REG_FUNC(CScriptObjectSystem,SetScreenFx);
  REG_FUNC(CScriptObjectSystem,SetScreenFxParamInt);
  REG_FUNC(CScriptObjectSystem,SetScreenFxParamFloat);
  REG_FUNC(CScriptObjectSystem,GetScreenFx);
  REG_FUNC(CScriptObjectSystem,GetScreenFxParamInt);
  REG_FUNC(CScriptObjectSystem,GetScreenFxParamFloat);
  REG_FUNC(CScriptObjectSystem,SetScissor);

  // CW: added for script based system analysis
  REG_FUNC( CScriptObjectSystem, GetCPUQuality );
  REG_FUNC( CScriptObjectSystem, GetGPUQuality );
  REG_FUNC( CScriptObjectSystem, GetSystemMem );
  REG_FUNC( CScriptObjectSystem, GetVideoMem );
  REG_FUNC( CScriptObjectSystem, IsPS20Supported );
	REG_FUNC( CScriptObjectSystem, IsHDRSupported );

	REG_FUNC(CScriptObjectSystem,ActivateLight);
//	REG_FUNC(CScriptObjectSystem,ActivateMainLight);
	REG_FUNC(CScriptObjectSystem,SetSkyBox);
	REG_FUNC(CScriptObjectSystem,SetWaterVolumeOffset);
	REG_FUNC(CScriptObjectSystem,MeasureTime);
	REG_FUNC(CScriptObjectSystem,IsValidMapPos);
	REG_FUNC(CScriptObjectSystem,EnableMainView);
	REG_FUNC(CScriptObjectSystem,EnableOceanRendering);
	REG_FUNC(CScriptObjectSystem,ScanDirectory);
	REG_FUNC(CScriptObjectSystem,DebugStats);
	REG_FUNC(CScriptObjectSystem,ViewDistanceSet);
	REG_FUNC(CScriptObjectSystem,ViewDistanceGet);
	REG_FUNC(CScriptObjectSystem,SetFogEnd);
	REG_FUNC(CScriptObjectSystem,SetFogStart);
	REG_FUNC(CScriptObjectSystem,SetFogColor);
	REG_FUNC(CScriptObjectSystem,GetFogEnd);
	REG_FUNC(CScriptObjectSystem,GetFogStart);
	REG_FUNC(CScriptObjectSystem,GetFogColor);
	REG_FUNC(CScriptObjectSystem,ApplyForceToEnvironment);
	REG_FUNC(CScriptObjectSystem,GetWorldColor);
	REG_FUNC(CScriptObjectSystem,SetWorldColor);
	REG_FUNC(CScriptObjectSystem,GetOutdoorAmbientColor);
	REG_FUNC(CScriptObjectSystem,SetOutdoorAmbientColor);
	REG_FUNC(CScriptObjectSystem,SetBFCount);
	REG_FUNC(CScriptObjectSystem,GetBFCount);
	REG_FUNC(CScriptObjectSystem,SetGrasshopperCount);
	REG_FUNC(CScriptObjectSystem,GetGrasshopperCount);
	REG_FUNC(CScriptObjectSystem,SetGrasshopperCGF);
	REG_FUNC(CScriptObjectSystem,GetTerrainElevation);
	REG_FUNC(CScriptObjectSystem,SetSkyFade);
//	REG_FUNC(CScriptObjectSystem,SetIndoorColor);
	REG_FUNC(CScriptObjectSystem,ActivatePortal);
	REG_FUNC(CScriptObjectSystem,DumpMMStats);
	REG_FUNC(CScriptObjectSystem,EnumDisplayFormats);
	REG_FUNC(CScriptObjectSystem,EnumAAFormats);
	REG_FUNC(CScriptObjectSystem,IsPointIndoors);
	REG_FUNC(CScriptObjectSystem,SetConsoleImage);
	REG_FUNC(CScriptObjectSystem,ProjectToScreen);
	REG_FUNC(CScriptObjectSystem,EnableHeatVision);
	REG_FUNC(CScriptObjectSystem,ShowDebugger);
  REG_FUNC(CScriptObjectSystem,FrameProfiler);
	//REG_FUNC(CScriptObjectSystem,IndoorSoundAllowed);
	//REG_FUNC(CScriptObjectSystem,ApplyStormToEnvironment);
	REG_FUNC(CScriptObjectSystem,DumpMemStats);
	REG_FUNC(CScriptObjectSystem,DumpWinHeaps);
	REG_FUNC(CScriptObjectSystem,Break);
	REG_FUNC(CScriptObjectSystem,DumpCommandsVars);
	REG_FUNC(CScriptObjectSystem,GetViewCameraPos);
	REG_FUNC(CScriptObjectSystem,RayWorldIntersection);
	REG_FUNC(CScriptObjectSystem,BrowseURL);
	REG_FUNC(CScriptObjectSystem,IsDevModeEnable);
	REG_FUNC(CScriptObjectSystem,RayTraceCheck);
	REG_FUNC(CScriptObjectSystem,SaveConfiguration);
	REG_FUNC(CScriptObjectSystem,SetSystemShaderRenderFlags);
}

//////////////////////////////////////////////////////////////////////////
void CScriptObjectSystem::ReleaseTemplate()
{
	SAFE_RELEASE( m_pScriptTimeTable );
	_ScriptableEx<CScriptObjectEntity>::ReleaseTemplate();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

int CScriptObjectSystem::ShowDebugger(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pSystem->ShowDebugger(NULL, 0, "Invoked From User");
	return pH->EndFunction();
}

int CScriptObjectSystem::FrameProfiler(IFunctionHandler *pH)
{
    bool on = false;
    bool display = true;
    char *prefix = "";
	if(pH->GetParamCount()>0)
	{
		pH->GetParam(1, on);
		if(pH->GetParamCount()>1)
		{
			pH->GetParam(2, display);
			if(pH->GetParamCount()>2)
			{
				pH->GetParam(3, prefix);
			};
		};
	};
    m_pSystem->SetFrameProfiler(on, display, prefix);
    return pH->EndFunction();
};

int CScriptObjectSystem::DumpMemStats (IFunctionHandler *pH)
{
	m_pSystem->DumpMemoryUsageStatistics();
	return pH->EndFunction();
}

int CScriptObjectSystem::DumpWinHeaps (IFunctionHandler *pH)
{
	m_pSystem->DumpWinHeaps();
	return pH->EndFunction();
}

/*! Creates a download object
		@return download object just created
*/
int CScriptObjectSystem::CreateDownload(IFunctionHandler *pH)
{
	// this cast is a hack, because i don't want to change the ISystem interface at this point
	CSystem *pSystem = static_cast<CSystem *>(m_pSystem);
//#if !defined(LINUX)
	if (pSystem)
	{
		CHTTPDownloader *pDL = pSystem->m_pDownloadManager->CreateDownload();

		return pH->EndFunction(pDL->GetScriptObject());
	}
//#endif
	return pH->EndFunctionNull();
}

/*! Loads a font and makes it available for future-selection
		@param name of font-xml-file (no suffix)
*/
int CScriptObjectSystem::LoadFont(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *pszName;
	pH->GetParam(1, pszName);

	ICryFont *pICryFont=m_pSystem->GetICryFont();

	if(pICryFont)
	{
		string szFontPath = "languages/";
		const char *szLanguage = 0;

		/*
		m_pScriptSystem->GetGlobalValue("g_language", szLanguage);

		if (!szLanguage || !strlen(szLanguage))
		{
			szFontPath += "english";
		}
		else
		{
			szFontPath += szLanguage;
		}
		*/

		szFontPath += "fonts/";
		szFontPath += pszName;
		szFontPath += ".xml";

		IFFont *pIFont = pICryFont->NewFont(pszName);

		if (!pIFont->Load(szFontPath.c_str()))
		{
			m_pLog->Log((string("Error loading digital font from ")+szFontPath).c_str());
		}
	}
	return pH->EndFunction();
}

//-------------------------------------------------------------------------------------------------
int CScriptObjectSystem::ExecuteCommand(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	char *szCmd;

	if (pH->GetParam(1, szCmd))
	{
		m_pConsole->ExecuteString(szCmd);
	}

	return pH->EndFunction();
}



/*! Write a string to the console
		@param String to write
		@see CScriptObjectSystem::Log
*/
int CScriptObjectSystem::LogToConsole(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	LogString(pH,true);

	return (pH->EndFunction());
}

/*! log even with log verbosity 0 - without <LUA>
@param String to write
*/
int CScriptObjectSystem::LogAlways(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *sParam=NULL;

	pH->GetParam(1,sParam);

	if(sParam)
		CryLogAlways("%s",sParam);

	return pH->EndFunction();
}

int CScriptObjectSystem::GetConsoleKeyName(IFunctionHandler *pH)
{
	if (m_pSystem->GetIInput())
	{
		return pH->EndFunction(m_pSystem->GetIInput()->GetKeyName(XKEY_TILDE));
	}

	return pH->EndFunctionNull();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::Warning(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *sParam = "";
	if (pH->GetParam(1,sParam))
	{
		m_pSystem->Warning( VALIDATOR_MODULE_SCRIPTSYSTEM,VALIDATOR_WARNING,0,NULL,"%s",sParam );
	}

	return (pH->EndFunction());
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::Error(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *sParam = "";
	if (pH->GetParam(1,sParam))
	{
		m_pSystem->Warning( VALIDATOR_MODULE_SCRIPTSYSTEM,VALIDATOR_ERROR,0,NULL,"%s",sParam );
	}

	return (pH->EndFunction());
}

/*! Clear the console
*/
int CScriptObjectSystem::ClearConsole(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	m_pConsole->Clear();

	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Write a message into the log file and the console
		@param String to write
		@see stuff
*/
int CScriptObjectSystem::Log(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	LogString(pH,false);

	return (pH->EndFunction());
}

/////////////////////////////////////////////////////////////////////////////////
//log a string to the console and or to the file with support for different
//languages
void CScriptObjectSystem::LogString(IFunctionHandler *pH,bool bToConsoleOnly)
{
	const char *sParam=NULL;
	string szText;

	//get the text
	/*ScriptVarType varType=pH->GetParamType(1);

	if (varType==svtString)
	{
		//this is a string, set directly
		pH->GetParam(1,sParam);
	}
	else
	if (varType==svtNumber)
	{
		int nStringID;
		//this is an id of a string table for different languages
		if (pH->GetParam(1,nStringID))
		{
			szText="MUST BE PUT BACK"; //m_pGame->m_StringTableMgr.EnumString(nStringID);
			sParam=szText.c_str();
		}
	}*/

	pH->GetParam(1,sParam);

	if (sParam)
  {
    // add the "<Lua> " prefix to understand that this message
		// has been called from a script function
    char sLogMessage[1024];

		if(sParam[0]<=5 && sParam[0]!=0)
		{
			sLogMessage[0] = sParam[0];
			strcpy(&sLogMessage[1], "<Lua> ");
			strncat(sLogMessage, &sParam[1], sizeof(sLogMessage)-6);
		}
		else
		{
			strcpy(sLogMessage, "<Lua> ");
			strncat(sLogMessage, sParam, sizeof(sLogMessage)-6);
		}

		sLogMessage[sizeof(sLogMessage)-1]=0;

		if (bToConsoleOnly)
			m_pLog->LogToConsole(sLogMessage);
		else
			m_pLog->Log(sLogMessage);
  }

}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::SetConsoleImage(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *pszName;
	bool bRemoveCurrent;
	pH->GetParam(1, pszName);
	pH->GetParam(2, bRemoveCurrent);

	//remove the previous image
	//ITexPic *pPic=m_pConsole->GetImage();
	//pPic->Release(false); //afaik true removes the ref counter
	//m_pConsole->SetImage(NULL); //remove the image

	//load the new image
	ITexPic *pPic=m_pRenderer->EF_LoadTexture(pszName,FT_NOREMOVE,0,eTT_Base);
	m_pConsole->SetImage(pPic,bRemoveCurrent);

	return pH->EndFunction();
}

/*!set a named checkpoint for the profiler
	@param sLabel name of the checkpoint
*/
int CScriptObjectSystem::MeasureTime(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sLabel;
	if(pH->GetParam(1,sLabel))
		m_pTimer->MeasureTime(sLabel);
	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::GetCurrTime(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	float fTime = m_pTimer->GetCurrTime();
	return pH->EndFunction(fTime);
}

int CScriptObjectSystem::GetCurrAsyncTime(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	float fTime = m_pTimer->GetAsyncCurTime();
	return pH->EndFunction(fTime);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::GetFrameTime(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	float fTime = m_pTimer->GetFrameTime();
	return pH->EndFunction(fTime);
}
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::GetLocalOSTime(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	//! Get time.
#if defined(LINUX)
	time_t long_time = time( NULL );
	struct tm *newtime = localtime( &long_time ); /* Convert to local time. */
#else
	__time64_t long_time;
	_time64( &long_time );                /* Get time as long integer. */
	struct tm *newtime = _localtime64( &long_time ); /* Convert to local time. */
#endif

	float ftime = 0;
	if (newtime)
	{
		m_pScriptTimeTable->BeginSetGetChain();
		m_pScriptTimeTable->SetValueChain("sec",newtime->tm_sec);
		m_pScriptTimeTable->SetValueChain("min",newtime->tm_min);
		m_pScriptTimeTable->SetValueChain("hour",newtime->tm_hour);
		m_pScriptTimeTable->SetValueChain("isdst",newtime->tm_isdst);
		m_pScriptTimeTable->SetValueChain("mday",newtime->tm_mday);
		m_pScriptTimeTable->SetValueChain("wday",newtime->tm_wday);
		m_pScriptTimeTable->SetValueChain("mon",newtime->tm_mon);
		m_pScriptTimeTable->SetValueChain("yday",newtime->tm_yday);
		m_pScriptTimeTable->SetValueChain("year",newtime->tm_year);
		m_pScriptTimeTable->EndSetGetChain();
	}
	return pH->EndFunction(m_pScriptTimeTable);
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::ShowConsole(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	int nParam = 0;
	pH->GetParam(1, nParam);
  m_pConsole->ShowConsole(nParam != 0);
	return pH->EndFunction();
}
/*
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::PostMessage(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sParam;
	IProcess *pProcess = m_pSystem->GetIProcess();
	pH->GetParam(1, sParam);
	m_pGame->SendMessage(sParam);
	return pH->EndFunction();
}
*/
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!Get an entity by id
	@param nID the entity id
*/
int CScriptObjectSystem::GetEntity(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	IEntity *pEntity;
	IScriptObject *pObject;
	int nID;
	pH->GetParam(1,nID);
	pEntity=m_pEntitySystem->GetEntity(nID);
	if(pEntity){
		pObject=pEntity->GetScriptObject();
		return pH->EndFunction(pObject);
	}
	else
	{
		return pH->EndFunctionNull();
	}
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!return a all entities currently present in the level
	@return a table filled with all entities currently present in the level
*/
int CScriptObjectSystem::GetEntities(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	_SmartScriptObject pObj(m_pScriptSystem);
	int k = 0;

	IEntityItPtr pIIt = m_pEntitySystem->GetEntityIterator();
	IEntity *pEntity = NULL;

	while(pEntity = pIIt->Next())
	{
		if (pEntity->GetScriptObject())
		{
			pObj->SetAt(k, pEntity->GetScriptObject());
			k++;
		}
	}
	return pH->EndFunction(*pObj);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!get the classid of a certain entity class
	@param sClassName the name of the class
	@return [if succeded]the id of the class specified by sClassName [if failed]return nil
*/

#if !defined(XBOX) && !defined(PS2) && (defined(WIN32) || defined(LINUX))
	#if !defined(LINUX)
		#include <io.h>
	#endif
	inline bool Filter(struct __finddata64_t& fd, int nScanMode)
	{
		if (!strcmp(fd.name, ".") || !strcmp(fd.name, ".."))
			return false;

		switch (nScanMode)
		{
		case SCANDIR_ALL:
			return true;
		case SCANDIR_SUBDIRS:
			return 0 != (fd.attrib & _A_SUBDIR);
		case SCANDIR_FILES:
			return 0 == (fd.attrib & _A_SUBDIR);
		default:
			return false;
		}
	}

	inline bool Filter(struct _finddata_t& fd, int nScanMode)
	{
		if (!strcmp(fd.name, ".") || !strcmp(fd.name, ".."))
			return false;

		switch (nScanMode)
		{
		case SCANDIR_ALL:
			return true;
		case SCANDIR_SUBDIRS:
			return 0 != (fd.attrib & _A_SUBDIR);
		case SCANDIR_FILES:
			return 0 == (fd.attrib & _A_SUBDIR);
		default:
			return false;
		}
	}
#endif

/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::ScanDirectory(IFunctionHandler *pH)
{
	if (pH->GetParamCount()<1)
		return pH->EndFunctionNull();

	_SmartScriptObject pObj(m_pScriptSystem);
	int k=0;

	const char *pszFolderName;
	if (!pH->GetParam(1,pszFolderName))
		return pH->EndFunction(*pObj);

	int nScanMode=SCANDIR_SUBDIRS;
	int nInPack = 0;
	if (pH->GetParamCount()>1)
		pH->GetParam(2, nScanMode);
	if (pH->GetParamCount()>2)
		pH->GetParam(3, nInPack);

	if (!nInPack)
	{
#if !defined(XBOX) && !defined(PS2)

		struct __finddata64_t c_file;
		intptr_t hFile;

		// Find first file in current directory
#if defined(WIN32)
		if ((hFile = _findfirst64( (string(pszFolderName) + "\\*.*").c_str(), &c_file )) == -1L)
#elif defined(LINUX)
		if ((hFile = _findfirst64( (string(pszFolderName) + "/*").c_str(), &c_file )) == -1)
#endif
		{
			return (pH->EndFunction(*pObj));
		}
		else
		{
			do
			{
				if (Filter (c_file, nScanMode))
				{
					pObj->SetAt(k,c_file.name);
					k++;
				}
			}
			while(_findnext64(hFile, &c_file)==0);

			_findclose(hFile);
		}
#endif
	}
	else
	{
		_finddata_t c_file;
		intptr_t hFile;

		if ((hFile = m_pSystem->GetIPak()->FindFirst((string(pszFolderName) + "\\*.*").c_str(), &c_file)) == -1L)
		{
			return pH->EndFunction(*pObj);
		}
		else
		{
			do
			{
				if (Filter (c_file, nScanMode))
				{
					pObj->SetAt(k, c_file.name);
					k++;
				}
			}
			while(m_pSystem->GetIPak()->FindNext(hFile, &c_file)==0);

			m_pSystem->GetIPak()->FindClose(hFile);
		}
	}

	return pH->EndFunction(*pObj);
}

/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::DrawLabelImage(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);

	CScriptObjectVector oVec(m_pScriptSystem,true);
	float fSize;
	USER_DATA nTextureId=0;
	int nCookie=0;

	pH->GetParam(1,*oVec);
	pH->GetParam(2,fSize);

	pH->GetParamUDVal(3,nTextureId,nCookie);
	if(nTextureId && (nCookie==USER_DATA_TEXTURE))
	{
		if (m_pRenderer)
			m_pRenderer->DrawLabelImage(oVec.Get(),fSize,nTextureId);
	}
	return (pH->EndFunction());
}


/////////////////////////////////////////////////////////////////////////////////
/*!return all entities contained in a certain radius
		@param oVec center of the sphere
		@param fRadius length of the radius
		@return a table filled with all entities contained in the specified radius
*/
int CScriptObjectSystem::GetEntitiesInRadius(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	float fRadius;
	Vec3 v3Origin;
	std::vector<IEntity*> ents;
	std::vector<IEntity*>::iterator itor;
	int k = 0;

	pH->GetParam(1,*oVec);
	pH->GetParam(2,fRadius);
	v3Origin=oVec.Get();
	m_pEntitySystem->GetEntitiesInRadius( v3Origin,fRadius,ents );
	itor=ents.begin();

	if (!ents.empty())
	{
		_SmartScriptObject pObj(m_pScriptSystem);
		while(itor!=ents.end())
		{
			pObj->SetAt(k,(*itor)->GetScriptObject());
			k++;
			++itor;
		}
		return pH->EndFunction(*pObj);
	}

	return pH->EndFunctionNull();
}

/////////////////////////////////////////////////////////////////////////////////
/*!Get all entities members of a certain team
	@param nTeamId id of the team
	@return a table filled with all members entities
*/
int CScriptObjectSystem::GetTeamMembers(IFunctionHandler *pH)
{
/*	CHECK_PARAMETERS(1);
	int nTeamId;
	pH->GetParam(1, nTeamId);
	_SmartScriptObject pObj(m_pScriptSystem);
//	CTeamMgr *pTeamMgr=m_pGame->GetTeamManager();
	IXSystem *pSys=NULL;
	if(m_pGame->m_pServer){
		pSys=m_pGame->m_pServer->m_pISystem;
	}else if(m_pGame->m_pClient){
		pSys=m_pGame->m_pClient->m_pISystem;
	}
	IEntityIt *pIt=m_pEntitySystem->GetEntityIterator();
	IEntity *pEntity=NULL;
	while ((pEntity=pIt->Next())!=NULL)
	{
		//CTeam *pTeam=pTeamMgr->GetEntityTeam(pEntity->GetId());
		if (pSys->GetEntityTeam(pEntity->GetId())==nTeamId)
		{
			pObj->SetAt(pEntity->GetId(), pEntity->GetScriptObject());
		}
	}
	pIt->Release();
	return pH->EndFunction(*pObj);*/
return pH->EndFunctionNull();
}



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*![DEBUG ONLY]return an entity by his name
*/
int CScriptObjectSystem::GetEntityByName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	IEntity *pEntity;
	IScriptObject *pObject;
	const char *sEntityName;
	pH->GetParam(1,sEntityName);
	pEntity=m_pEntitySystem->GetEntity(sEntityName);
	if(pEntity)
	{
		pObject=pEntity->GetScriptObject();
		return pH->EndFunction(pObject);
	}
	return pH->EndFunctionNull();

}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*int CScriptObjectSystem::RemoveTexture(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int nID;
	pH->GetParam(1,nID);
	m_pRenderer->RemoveTexture(nID);
	return pH->EndFunction();
}*/
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!Load a set of textures in order to create an animated texture
	@param sFormat Name of the texture to load
	@param nCount number of frame to load
	@return the texture id if succeded nil if failed
*/
int CScriptObjectSystem::LoadAnimatedTexture(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *sFormat;
	int nCount,nTid;
	pH->GetParam(1,sFormat);
	pH->GetParam(2,nCount);
	nTid=m_pRenderer->LoadAnimatedTexture(sFormat,nCount);
	if(nTid)
	{
		USER_DATA ud=m_pScriptSystem->CreateUserData((int)nTid,USER_DATA_TEXTURE);
		return pH->EndFunction(ud);
	}
	return pH->EndFunctionNull();
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!load a texture
	@param szFileName the texture file path
	@return the texture id if succeded nil if failed
*/
int CScriptObjectSystem::LoadTexture(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(1);
	const char *szFileName;
	int nTid=0;
	pH->GetParam(1,szFileName);

	int nLoadAsCubeMap=0;
	bool bClamp = false;
	if (pH->GetParamCount()>=2)
	{
		pH->GetParam(2,nLoadAsCubeMap);
		pH->GetParam(3,bClamp);
	}

	ITexPic * pPic = 0;
	if(nLoadAsCubeMap)
		pPic = m_pSystem->GetIRenderer()->EF_LoadTexture((char*)szFileName, 0, FT2_FORCECUBEMAP, eTT_Cubemap);
	else
		pPic = m_pRenderer->EF_LoadTexture((char*)szFileName,FT_CLAMP | FT_NOREMOVE,0,eTT_Base);

	if (pPic && pPic->IsTextureLoaded())
	{
		nTid=pPic->GetTextureID();
		m_pSystem->GetIRenderer()->SetTexture(nTid);
		m_pSystem->GetIRenderer()->SetTexClampMode(bClamp);

		if(nLoadAsCubeMap)
			return pH->EndFunction(nTid);


		USER_DATA ud=m_pScriptSystem->CreateUserData((int)nTid,USER_DATA_TEXTURE);
		return pH->EndFunction(ud);
	}
	return pH->EndFunctionNull();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!load a CGF geometry(model)
	@param szFileName the texture file path
	@return the obj id if succeded nil if failed
*/
int CScriptObjectSystem::LoadObject(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *szFileName=NULL;
	int nTid=0;
	pH->GetParam(1,szFileName);


	//nTid=m_pRenderer->LoadTexture(sFileName);
	if (!szFileName || strlen(szFileName) == 0)
	{
		m_pSystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,0,
			"Script method System:LoadObject(filename) called with Empty filename" );
		return pH->EndFunctionNull();
	}

	IStatObj *pObj = m_p3DEngine->MakeObject(szFileName);
	if (pObj)
	{
		//nTid=pPic->GetTextureID();
		USER_DATA ud=m_pScriptSystem->CreateUserData((INT_PTR)pObj,USER_DATA_OBJECT);
		return pH->EndFunction(ud);
	}
	return pH->EndFunctionNull();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::DrawSprite(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
//	const char *sImage;
	Vec3 v3Origin;
//	int nMode;
	CScriptObjectVector oVec(m_pScriptSystem,true);
/*	CImage *pImage;

	pH->GetParam(1,sImage);
	pH->GetParam(2,*oVec);
	pH->GetParam(3,nMode);

	pImage=m_pRenderer->FindImage(sImage);

	if(pImage)
	{
		v3Origin=oVec.Get();

		if (nMode!=0)
		{
			m_pRenderer->EnableBlend(true);
			m_pRenderer->SetBlendMode(nMode);
		}
		else
		{
			m_pRenderer->Project3DSprite(v3Origin,pImage);
		}
	}*/
	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!create a terrain deformation at the given point
	this function is called when a projectile or a grenade explode
	@param oVec explosion position
	@param fSize explosion radius
*/
int CScriptObjectSystem::DeformTerrain(IFunctionHandler *pH)
{
	if (pH->GetParamCount() < 3)
		return pH->EndFunction();

	CScriptObjectVector oVec(m_pScriptSystem,true);
    Vec3 v3Pos;//,v3SysDir;
    float fSize;
    USER_DATA nTid;
	int nCookie=0;
	pH->GetParam(1,*oVec);
	v3Pos=oVec.Get();
	pH->GetParam(2,fSize);
	pH->GetParamUDVal(3,nTid,nCookie);
	if(nTid && (nCookie==USER_DATA_TEXTURE))
	{
		bool bDeform = true;

		// somebody specified a parameter, so let's take a look at it
		if (pH->GetParamCount() > 3)
			pH->GetParam(4, bDeform);

		//check if e_deformable_terrain for some reason is NULL, maybe at the init time it wasnt already created.
		if (!e_deformable_terrain)
			e_deformable_terrain = m_pSystem->GetIConsole()->GetCVar("e_deformable_terrain");

		// always make it false, when e_deformable_terrain is set to 0
		if (e_deformable_terrain && e_deformable_terrain->GetIVal()==0)
			bDeform = false;

		m_p3DEngine->OnExplosion(v3Pos,Vec3d(0,0,-1),fSize,nTid, bDeform);
	}
	return pH->EndFunction();
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::ScreenToTexture(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pRenderer->ScreenToTexture();
	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!load a texture
	@param szFileName the texture file path
	@return the texture id if succeded nil if failed
*/
int CScriptObjectSystem::LoadImage(IFunctionHandler *pH)
{
	if (pH->GetParamCount() < 1)
	{
		CHECK_PARAMETERS(1);

		return pH->EndFunction();
	}
	const char *sFileName;
	int nTid=0;
	bool bClamp = false;
	bool bRemovable = false;
	pH->GetParam(1,sFileName);

	if (pH->GetParamCount() > 1)
	{
		pH->GetParam(2, bClamp);
	}

	if (pH->GetParamCount() > 2)
	{
		pH->GetParam(3, bRemovable);
	}

	//nTid=m_pRenderer->LoadTexture(sFileName);
	ITexPic * pPic = m_pRenderer->EF_LoadTexture((char *)sFileName, (bRemovable ? 0 : FT_NOREMOVE) | FT_NORESIZE, 0, eTT_Base);

  if (pPic && pPic->IsTextureLoaded())
	{
		nTid=pPic->GetTextureID();
		m_pRenderer->SetTexture(nTid);
		m_pRenderer->SetTexClampMode(bClamp);
		USER_DATA ud=m_pScriptSystem->CreateUserData((int)nTid,USER_DATA_TEXTURE);
		return pH->EndFunction(ud);
	}

	return pH->EndFunctionNull();
}

int CScriptObjectSystem::FreeImage(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	INT_PTR nTid = -1;
	int nCookie = 0;

	pH->GetParamUDVal(1, nTid, nCookie);

	if (nTid != -1)
	{
		m_pRenderer->RemoveTexture(nTid);
	}

	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////



int CScriptObjectSystem::DrawLine(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(6);
	CScriptObjectVector pPt1(m_pScriptSystem,true);
	CScriptObjectVector pPt2(m_pScriptSystem,true);
	float r,g,b,a;
	pH->GetParam(1,pPt1);
	pH->GetParam(2,pPt2);
	pH->GetParam(3,r);
	pH->GetParam(4,g);
	pH->GetParam(5,b);
	pH->GetParam(6,a);
	CFColor cfc(r,g,b,a);
	m_pRenderer->DrawLineColor(pPt1.Get(),cfc,pPt2.Get(),cfc);
	return pH->EndFunction();
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::Draw2DLine(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(8);
	Vec3 p1(0,0,0),p2(0,0,0);
	float r,g,b,a;
	pH->GetParam(1,p1.x);
	pH->GetParam(2,p1.y);
	pH->GetParam(3,p2.x);
	pH->GetParam(4,p2.y);
	pH->GetParam(5,r);
	pH->GetParam(6,g);
	pH->GetParam(7,b);
	pH->GetParam(8,a);
	CFColor cfc(r,g,b,a);

  m_pRenderer->Set2DMode(true,800,600);
	m_pRenderer->SetState(GS_NODEPTHTEST);

  m_pRenderer->DrawLineColor(p1,cfc,p2,cfc);

	m_pRenderer->SetState(GS_DEPTHWRITE);
	m_pRenderer->Set2DMode(false, 800,600);

	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!draw an sprite on screen
	@param nTid texture id
	@param nPx x position(the screen is normalized at 800x600)
	@param nPy y position(the screen is normalized at 800x600)
	@param w width
	@param h height
	@param nMode blending mode (the number must be specified)
		R_BLEND_MODE__ZERO__SRC_COLOR					        1
		R_BLEND_MODE__SRC_COLOR__ZERO					        2
		R_BLEND_MODE__SRC_COLOR__ONE_MINUS_SRC_COLOR	3
		R_BLEND_MODE__SRC_ALPHA__ONE_MINUS_SRC_ALPHA	4
		R_BLEND_MODE__ONE__ONE                        5
		R_BLEND_MODE__DST_COLOR__SRC_COLOR            6
		R_BLEND_MODE__ZERO__ONE_MINUS_SRC_COLOR       7
		R_BLEND_MODE__ONE__ONE_MINUS_SRC_COLOR	      8
		R_BLEND_MODE__ONE__ZERO                       9
		R_BLEND_MODE__ZERO__ZERO                     10
		R_BLEND_MODE__ONE__ONE_MINUS_SRC_ALPHA       11
		R_BLEND_MODE__SRC_ALPHA__ONE                 12
		R_BLEND_MODE__ADD_SIGNED                     14
*/
int CScriptObjectSystem::DrawImage(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(6);

	USER_DATA nTid;
	int nPx;
	int nPy;
	int w;
	int h;
	int nMode;
	//pH->GetParam(1,nTid);
	int nCookie=0;
	pH->GetParamUDVal(1,nTid,nCookie);
	if(nTid && (nCookie==USER_DATA_TEXTURE))
	{
		pH->GetParam(2,nPx);
		pH->GetParam(3,nPy);
		pH->GetParam(4,w);
		pH->GetParam(5,h);
		pH->GetParam(6,nMode);

		if (nMode!=0)
    	m_pRenderer->SetState(GS_NODEPTHTEST | sGetBlendState(nMode));

		m_pRenderer->Draw2dImage((float)nPx,(float)nPy,(float)w,(float)h,nTid, 0.f, 1.f, 1.f, 0.f);

		if (nMode!=0)
    	m_pRenderer->SetState(GS_DEPTHWRITE);
	}
	return pH->EndFunction();
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!draw an sprite on screen
	@param nTid texture id
	@param nPx x position(the screen is normalized at 800x600)
	@param nPy y position(the screen is normalized at 800x600)
	@param w width
	@param h height
	@param nMode blending mode (the number must be specified)
		R_BLEND_MODE__ZERO__SRC_COLOR					        1
		R_BLEND_MODE__SRC_COLOR__ZERO					        2
		R_BLEND_MODE__SRC_COLOR__ONE_MINUS_SRC_COLOR	3
		R_BLEND_MODE__SRC_ALPHA__ONE_MINUS_SRC_ALPHA	4
		R_BLEND_MODE__ONE__ONE                        5
		R_BLEND_MODE__DST_COLOR__SRC_COLOR            6
		R_BLEND_MODE__ZERO__ONE_MINUS_SRC_COLOR       7
		R_BLEND_MODE__ONE__ONE_MINUS_SRC_COLOR	      8
		R_BLEND_MODE__ONE__ZERO                       9
		R_BLEND_MODE__ZERO__ZERO                     10
		R_BLEND_MODE__ONE__ONE_MINUS_SRC_ALPHA       11
		R_BLEND_MODE__SRC_ALPHA__ONE                 12
		R_BLEND_MODE__ADD_SIGNED                     14
	@param r red component
	@param g green component
	@param b blue component
	@param a alpha component
*/
int CScriptObjectSystem::DrawImageColor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(10);
	USER_DATA nTid;
	float nPx;
	float nPy;
	float w;
	float h;
	int nMode;
	float r,g,b,a;
	//pH->GetParam(1,nTid);
	int nCookie=0;
	pH->GetParamUDVal(1,nTid,nCookie);
	if(nTid && (nCookie==USER_DATA_TEXTURE))
	{
		pH->GetParam(2,nPx);
		pH->GetParam(3,nPy);
		pH->GetParam(4,w);
		pH->GetParam(5,h);
		pH->GetParam(6,nMode);
		pH->GetParam(7,r);
		pH->GetParam(8,g);
		pH->GetParam(9,b);
		pH->GetParam(10,a);

		if (nMode!=0)
    	m_pRenderer->SetState(GS_NODEPTHTEST | sGetBlendState(nMode));

		m_pRenderer->Draw2dImage(nPx,nPy,w,h,nTid,0,1,1,0,0,r,g,b,a);

		if (nMode!=0)
    	m_pRenderer->SetState(GS_DEPTHWRITE);
	}
	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!draw an sprite on screen
@param nTid texture id
@param nPx x position(the screen is normalized at 800x600)
@param nPy y position(the screen is normalized at 800x600)
@param w width
@param h height
@param nMode blending mode (the number must be specified)
R_BLEND_MODE__ZERO__SRC_COLOR					        1
R_BLEND_MODE__SRC_COLOR__ZERO					        2
R_BLEND_MODE__SRC_COLOR__ONE_MINUS_SRC_COLOR	3
R_BLEND_MODE__SRC_ALPHA__ONE_MINUS_SRC_ALPHA	4
R_BLEND_MODE__ONE__ONE                        5
R_BLEND_MODE__DST_COLOR__SRC_COLOR            6
R_BLEND_MODE__ZERO__ONE_MINUS_SRC_COLOR       7
R_BLEND_MODE__ONE__ONE_MINUS_SRC_COLOR	      8
R_BLEND_MODE__ONE__ZERO                       9
R_BLEND_MODE__ZERO__ZERO                     10
R_BLEND_MODE__ONE__ONE_MINUS_SRC_ALPHA       11
R_BLEND_MODE__SRC_ALPHA__ONE                 12
R_BLEND_MODE__ADD_SIGNED                     14
@param u1 1st u component of the texcoords
@param v1 1st v component of the texcoords
@param u2 2nd u component of the texcoords
@param v2 2nd v component of the texcoords
*/
int CScriptObjectSystem::DrawImageCoords(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(10);

	USER_DATA nTid;
	int nPx;
	int nPy;
	int w;
	int h;
	int nMode;
	float u1, v1, u2, v2;
	//pH->GetParam(1,nTid);
	int nCookie=0;
	pH->GetParamUDVal(1,nTid,nCookie);
	if(nTid && (nCookie==USER_DATA_TEXTURE))
	{
		pH->GetParam(2,nPx);
		pH->GetParam(3,nPy);
		pH->GetParam(4,w);
		pH->GetParam(5,h);
		pH->GetParam(6,nMode);
		pH->GetParam(7,u1);
		pH->GetParam(8,v1);
		pH->GetParam(9,u2);
		pH->GetParam(10,v2);
		if (nMode!=0)
    	m_pRenderer->SetState(GS_NODEPTHTEST | sGetBlendState(nMode));

 		m_pRenderer->Draw2dImage((float)nPx,(float)nPy,(float)w,(float)h,nTid, u1, v1, u2, v2);

		if (nMode!=0)
    	m_pRenderer->SetState(GS_DEPTHWRITE);
	}
	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*!draw an sprite on screen
	@param nTid texture id
	@param nPx x position(the screen is normalized at 800x600)
	@param nPy y position(the screen is normalized at 800x600)
	@param w width
	@param h height
	@param nMode blending mode (the number must be specified)
		R_BLEND_MODE__ZERO__SRC_COLOR					        1
		R_BLEND_MODE__SRC_COLOR__ZERO					        2
		R_BLEND_MODE__SRC_COLOR__ONE_MINUS_SRC_COLOR	3
		R_BLEND_MODE__SRC_ALPHA__ONE_MINUS_SRC_ALPHA	4
		R_BLEND_MODE__ONE__ONE                        5
		R_BLEND_MODE__DST_COLOR__SRC_COLOR            6
		R_BLEND_MODE__ZERO__ONE_MINUS_SRC_COLOR       7
		R_BLEND_MODE__ONE__ONE_MINUS_SRC_COLOR	      8
		R_BLEND_MODE__ONE__ZERO                       9
		R_BLEND_MODE__ZERO__ZERO                     10
		R_BLEND_MODE__ONE__ONE_MINUS_SRC_ALPHA       11
		R_BLEND_MODE__SRC_ALPHA__ONE                 12
		R_BLEND_MODE__ADD_SIGNED                     14
	@param r red component
	@param g green component
	@param b blue component
	@param a alpha component
	@param u1 1st u component of the texcoords
	@param v1 1st v component of the texcoords
	@param u2 2nd u component of the texcoords
	@param v2 2nd v component of the texcoords
*/
int CScriptObjectSystem::DrawImageColorCoords(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(14);
	USER_DATA nTid;
	float nPx;
	float nPy;
	float w;
	float h;
	int nMode;
	float r,g,b,a;
	float u1, v1, u2, v2;
	int nCookie=0;
	pH->GetParamUDVal(1,nTid,nCookie);
	if(nTid && (nCookie==USER_DATA_TEXTURE))
	{
		//pH->GetParam(1,nTid);
		pH->GetParam(2,nPx);
		pH->GetParam(3,nPy);
		pH->GetParam(4,w);
		pH->GetParam(5,h);
		pH->GetParam(6,nMode);
		pH->GetParam(7,r);
		pH->GetParam(8,g);
		pH->GetParam(9,b);
		pH->GetParam(10,a);
		pH->GetParam(11,u1);
		pH->GetParam(12,v1);
		pH->GetParam(13,u2);
		pH->GetParam(14,v2);

		if (nMode!=0)
    	m_pRenderer->SetState(GS_NODEPTHTEST | sGetBlendState(nMode));

		m_pRenderer->Draw2dImage(nPx,nPy,w,h,nTid,u1,v1,u2,v2,0,r,g,b,a);

		if (nMode!=0)
    	m_pRenderer->SetState(GS_DEPTHWRITE);
	}
	return pH->EndFunction();
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::DrawTriStrip(IFunctionHandler *pH)
{
#if !defined(LINUX)
#define _MAX_VTXS 10
	USER_DATA nTid;
	int nCookie=0;
	int nMode=0;
	float a,r,g,b;
	pH->GetParamUDVal(1,nTid,nCookie);
	struct _vtx_{
		float x,y,z;
		unsigned char c[4];
		float u,v;
	};
	pH->GetParam(4,r);
	pH->GetParam(5,g);
	pH->GetParam(6,b);
	pH->GetParam(7,a);
	_SmartScriptObject vtxs(m_pScriptSystem,true);
	if(((nTid && (nCookie==USER_DATA_TEXTURE))) && pH->GetParam(2,nMode)  && pH->GetParam(3,vtxs))
	{
		_SmartScriptObject vtx(m_pScriptSystem,true);
		_vtx_ v[_MAX_VTXS];
		int nvtxs=0;
		vtxs->BeginIteration();
		while(vtxs->MoveNext() && nvtxs<_MAX_VTXS)
		{
			if(vtxs->GetCurrent(vtx))
			{
				v[nvtxs].z=0;
				v[nvtxs].c[0]=unsigned char(r*0xFF);
				v[nvtxs].c[1]=unsigned char(g*0xFF);
				v[nvtxs].c[2]=unsigned char(b*0xFF);
				v[nvtxs].c[3]=unsigned char(a*0xFF);
				vtx->GetValue("x",v[nvtxs].x);
				vtx->GetValue("y",v[nvtxs].y);
				vtx->GetValue("u",v[nvtxs].u);
				vtx->GetValue("v",v[nvtxs].v);
				nvtxs++;
			}
		}
		vtxs->EndIteration();
		if(nvtxs)
		{
			CVertexBuffer vb(&v,VERTEX_FORMAT_P3F_COL4UB_TEX2F,nvtxs);
			m_pRenderer->Set2DMode(true,800,600);
			m_pRenderer->SetTexture(nTid);
      unsigned int nState = GS_NODEPTHTEST;
			m_pRenderer->SetCullMode(R_CULL_DISABLE);
			if (nMode!=0)
			{
        nState |= sGetBlendState(nMode);
        m_pRenderer->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
			}
      m_pRenderer->SetState(nState);
			m_pRenderer->DrawTriStrip(&vb,nvtxs);
			if (nMode!=0){
				m_pRenderer->SetState(GS_DEPTHWRITE);
        m_pRenderer->SetColorOp(eCO_REPLACE, eCO_REPLACE, DEF_TEXARG0, DEF_TEXARG0);
			}
			//m_pRenderer->SetCullMode(R_CULL_DISABLE);
			m_pRenderer->Set2DMode(false,0,0);
		}

	}
#endif
	return pH->EndFunction();
#undef _MAX_VTXS
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::DrawRectShader(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(9);
	const char *pszShaderName;
	float x, y, w, h, r, g, b, a;
	pH->GetParam(1, pszShaderName);
	pH->GetParam(2, x);
	pH->GetParam(3, y);
	pH->GetParam(4, w);
	pH->GetParam(5, h);
	pH->GetParam(6, r);
	pH->GetParam(7, g);
	pH->GetParam(8, b);
	pH->GetParam(9, a);
  IShader *sh = m_pRenderer->EF_LoadShader((char *)pszShaderName, eSH_Screen);
  if (!(sh->GetFlags() & EF_NOTFOUND))
  {
    m_pRenderer->EF_StartEf();
    CFColor col(r,g,b,a);
    m_pRenderer->EF_DrawEf(sh, x, y, w, h, col);
    m_pRenderer->Set2DMode(true,800,600);
    m_pRenderer->EF_EndEf2D(true);
    m_pRenderer->Set2DMode(false,800,600);
  }
	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::SetScreenShader(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *pszShaderName;

  pH->GetParam(1, pszShaderName);

  m_pSystem->GetI3DEngine()->SetScreenShader(pszShaderName);

  return pH->EndFunction();
}

// activate screen fx
int CScriptObjectSystem::SetScreenFx(IFunctionHandler *pH)
{
  CHECK_PARAMETERS(2);
  const char *pszEffectName;
  int iActive;

  pH->GetParam(1, pszEffectName);
  pH->GetParam(2, iActive);

  m_pSystem->GetI3DEngine()->SetScreenFx(pszEffectName, iActive);

  return pH->EndFunction();
}

// set screen fx parameter, pass an integer value..
int CScriptObjectSystem::SetScreenFxParamInt(IFunctionHandler *pH)
{
  CHECK_PARAMETERS(3);
  const char *pszEffectName,
             *pszEffectParam;
  int         iValue;

  // <<NOTE>> check 3dScreenEffects for a list of effects names and respective parameters

  pH->GetParam(1, pszEffectName);
  pH->GetParam(2, pszEffectParam);
  pH->GetParam(3, iValue);

  m_pSystem->GetI3DEngine()->SetScreenFxParam(pszEffectName, pszEffectParam, &iValue);

  return pH->EndFunction();
}

// set screen fx parameter, pass a float value...
int CScriptObjectSystem::SetScreenFxParamFloat(IFunctionHandler *pH)
{
  CHECK_PARAMETERS(3);
  const char *pszEffectName,
             *pszEffectParam;
  float       fValue;

  // <<NOTE>> check 3dScreenEffects for a list of effects names and respective parameters

  pH->GetParam(1, pszEffectName);
  pH->GetParam(2, pszEffectParam);
  pH->GetParam(3, fValue);

  m_pSystem->GetI3DEngine()->SetScreenFxParam(pszEffectName, pszEffectParam, &fValue);

  return pH->EndFunction();
}

// Get screen effect state (enabled/disabled)
int CScriptObjectSystem:: GetScreenFx(IFunctionHandler *pH)
{
  CHECK_PARAMETERS(1);
  const char *pszEffectName;
  pH->GetParam(1, pszEffectName);

  // <<NOTE>> check 3dScreenEffects for a list of effects names

  // state is: 1 active, 0 disabled, -1 effect doens't exist
  int iState=m_pSystem->GetI3DEngine()->GetScreenFx(pszEffectName);
  if(iState>=0)
  {
    return pH->EndFunction(iState);
  }

  return pH->EndFunctionNull();
}

// Get screen effect parameter value
int CScriptObjectSystem::GetScreenFxParamInt(IFunctionHandler *pH)
{
  CHECK_PARAMETERS(2);
  const char *pszEffectName,
             *pszEffectParam;

  // <<NOTE>> check 3dScreenEffects for a list of effects names and respective parameters
  pH->GetParam(1, pszEffectName);
  pH->GetParam(2, pszEffectParam);

  void *pValue=0;
  // iResult is: 0 in case value effect name or parameter doens't exist, 1 in case of sucess
  int iResult=m_pSystem->GetI3DEngine()->GetScreenFxParam(pszEffectName, pszEffectParam, pValue);
  if(iResult)
  {
    int iValue=*((int*)pValue);
    return pH->EndFunction(iValue);
  }

  return pH->EndFunctionNull();
}

// Get screen effect parameter value
int CScriptObjectSystem::GetScreenFxParamFloat(IFunctionHandler *pH)
{
  CHECK_PARAMETERS(2);
  const char *pszEffectName,
             *pszEffectParam;

  // <<NOTE>> check 3dScreenEffects for a list of effects names and respective parameters
  pH->GetParam(1, pszEffectName);
  pH->GetParam(2, pszEffectParam);

  void *pValue=0;
  // iResult is: 0 in case value effect name or parameter doens't exist, 1 in case of sucess
  int iResult=m_pSystem->GetI3DEngine()->GetScreenFxParam(pszEffectName, pszEffectParam, pValue);
  if(iResult)
  {
    float fValue=*((float*)pValue);
    return pH->EndFunction(fValue);
  }

  return pH->EndFunctionNull();
}

// set scissoring screen area
int CScriptObjectSystem::SetScissor(IFunctionHandler *pH)
{
  CHECK_PARAMETERS(4);
  int x, y, w, h;

  pH->GetParam(1, x);
  pH->GetParam(2, y);
  pH->GetParam(3, w);
  pH->GetParam(4, h);

  m_pSystem->GetIRenderer()->SetScissor(m_pSystem->GetIRenderer()->ScaleCoordX((float)x),
                                        m_pSystem->GetIRenderer()->ScaleCoordY((float)y),
                                        m_pSystem->GetIRenderer()->ScaleCoordX((float)w),
                                        m_pSystem->GetIRenderer()->ScaleCoordY((float)h));

  return pH->EndFunction();
}

int CScriptObjectSystem::ActivateLight(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *pszLightName;
	bool bActive;
	pH->GetParam(1, pszLightName);
	pH->GetParam(2, bActive);
	m_p3DEngine->ActivateLight(pszLightName, bActive);
	return pH->EndFunction();
}
/*
int CScriptObjectSystem::ActivateMainLight(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	bool bActive;
	CScriptObjectVector oVec(m_pScriptSystem);
	if(pH->GetParam(1,*oVec))
	{
		pH->GetParam(2, bActive);
		m_p3DEngine->GetBuildingManager()->ActivateMainLight(oVec.Get(), bActive);
	}
	return pH->EndFunction();
}	*/

int CScriptObjectSystem::SetSkyBox(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	const char *pszShaderName;
	float fBlendTime;
	bool bUseWorldBrAndColor;
	pH->GetParam(1, pszShaderName);
	pH->GetParam(2, fBlendTime);
	pH->GetParam(3, bUseWorldBrAndColor);
	m_p3DEngine->SetSkyBox(pszShaderName);// not supported now: fBlendTime, bUseWorldBrAndColor);
	return pH->EndFunction();
}

int CScriptObjectSystem::IsValidMapPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	bool bValid=false;

	if(pH->GetParam(1,*oVec))
	{
		int nTerrainSize=m_p3DEngine->GetTerrainSize();
		float fOut=(float)(nTerrainSize+500);
		Vec3 v=oVec.Get();
		if(v.x<-500 || v.y<-500 || v.z>500 || v.x>fOut || v.y>fOut)
			bValid=false;
		else
			bValid=true;
	}
	return pH->EndFunction(bValid);
}


int CScriptObjectSystem::EnableMainView(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

  assert(0); // feature unimplemented

  /*bool bEnable;
	pH->GetParam(1,bEnable);
	if (m_p3DEngine)
		m_p3DEngine->EnableMainViewRendering(bEnable);*/

	return pH->EndFunction();
}

int CScriptObjectSystem::DebugStats(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	bool cp;
	pH->GetParam(1,cp);
	m_pSystem->DebugStats(cp, false);
	return pH->EndFunction();
}


//
//------------------------------------------------------------------------------
int CScriptObjectSystem::ViewDistanceSet(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float		viewDist;
	pH->GetParam(1,viewDist);
	if(viewDist<20)
		viewDist = 20;
	m_p3DEngine->SetMaxViewDistance(viewDist);
	return pH->EndFunction();
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::ViewDistanceGet(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	float	fDist = m_p3DEngine->GetMaxViewDistance( );
	return pH->EndFunction( fDist );
}


//
//------------------------------------------------------------------------------
int CScriptObjectSystem::SetFogEnd(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float		fogEnd;
	pH->GetParam(1,fogEnd);
	m_p3DEngine->SetFogEnd( fogEnd );

	float	alpha;

	if( m_SkyFadeStart != m_SkyFadeEnd )
	{
		if( fogEnd<m_SkyFadeEnd )
			alpha = 0.0f;
		else if( fogEnd<m_SkyFadeStart )
			alpha = 1.0f - (m_SkyFadeStart - fogEnd)/(m_SkyFadeStart-m_SkyFadeEnd);
		else
			alpha = 1.0f;
	}
	else
		alpha = 1.0f;

	m_p3DEngine->SetSkyBoxAlpha( alpha );

	return pH->EndFunction();
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::SetFogStart(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float		fogStart;
	pH->GetParam(1,fogStart);
	m_p3DEngine->SetFogStart( fogStart );
	return pH->EndFunction();
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::SetFogColor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	CScriptObjectColor oColor(m_pScriptSystem,true);
  Vec3 v3Color;
	pH->GetParam(1,*oColor);
	v3Color=oColor.Get();
	m_p3DEngine->SetFogColor( v3Color );
	return pH->EndFunction();
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::GetFogEnd(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	float		fogEnd;
	fogEnd = m_p3DEngine->GetFogEnd( );
	return pH->EndFunction(fogEnd);
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::GetFogStart(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	float		fogStart;
	fogStart = m_p3DEngine->GetFogStart( );
	return pH->EndFunction(fogStart);
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::GetFogColor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	Vec3 v3Color;

	CScriptObjectColor oColor(m_pScriptSystem);
//	vec=m_pEntity->GetPos(false);
	v3Color=m_p3DEngine->GetFogColor( );
	oColor=v3Color;
	return pH->EndFunction(*oColor);
}


//
//------------------------------------------------------------------------------
int CScriptObjectSystem::ApplyForceToEnvironment(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(3);
	CScriptObjectVector oPos(m_pScriptSystem,true);
	float	force;
	float	radius;

	pH->GetParam(1,*oPos);
	pH->GetParam(2, radius);
	pH->GetParam(3,force);

	Vec3 pos;
	pos = oPos.Get();

	m_p3DEngine->ApplyForceToEnvironment(pos,radius,force);

	return pH->EndFunction();
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::GetWorldColor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	Vec3 v3Color;

	CScriptObjectColor oColor(m_pScriptSystem);
//	vec=m_pEntity->GetPos(false);
	v3Color=m_p3DEngine->GetWorldColor( false );
//	v3Color *= 255.0f;
	oColor=v3Color;
	return pH->EndFunction(*oColor);
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::SetWorldColor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	CScriptObjectColor oColor(m_pScriptSystem,true);
  Vec3 v3Color;
	pH->GetParam(1,*oColor);
	v3Color=oColor.Get();
//	v3Color /= 255.0f;
	m_p3DEngine->SetWorldColor( v3Color );
	return pH->EndFunction();
}
//------------------------------------------------------------------------------
int CScriptObjectSystem::GetOutdoorAmbientColor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	Vec3 v3Color;

	CScriptObjectColor oColor(m_pScriptSystem);

	v3Color=m_p3DEngine->GetOutdoorAmbientColor();

	oColor=v3Color;
	return pH->EndFunction(*oColor);
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::SetOutdoorAmbientColor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	CScriptObjectColor oColor(m_pScriptSystem,true);
  Vec3 v3Color;
	pH->GetParam(1,*oColor);
	v3Color=oColor.Get();

	m_p3DEngine->SetOutdoorAmbientColor( v3Color );
	return pH->EndFunction();
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::SetBFCount(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int	bfNumber;
	pH->GetParam(1,bfNumber);
	m_p3DEngine->SetBFCount( bfNumber );
//bfNumber = m_p3DEngine->GetBFCount( );
	return pH->EndFunction();
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::GetBFCount(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	int	bfNumber;
	bfNumber = m_p3DEngine->GetBFCount( );
	return pH->EndFunction(bfNumber);
}


//
//------------------------------------------------------------------------------
int CScriptObjectSystem::SetGrasshopperCount(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int	bfNumber;
	pH->GetParam(1,bfNumber);
	m_p3DEngine->SetGrasshopperCount( bfNumber );
//	m_p3DEngine->SetBFCount( bfNumber );
//bfNumber = m_p3DEngine->GetGrasshopperCount();
	return pH->EndFunction();
}

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::GetGrasshopperCount(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	int	bfNumber;
	bfNumber = m_p3DEngine->GetGrasshopperCount( );
	return pH->EndFunction(bfNumber);
}


//
//------------------------------------------------------------------------------
int CScriptObjectSystem::SetGrasshopperCGF(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	IEntity *pEntity;
	int nID;
	pH->GetParam(1,nID);
	pEntity=m_pEntitySystem->GetEntity(nID);
	if(pEntity){
		CEntityObject obj;
		int		slot=0;
		while(pEntity->GetEntityObject(slot, obj))
		{
			m_p3DEngine->SetGrasshopperCGF(slot++, obj.object);
		}
		for( ;slot<4; )
			m_p3DEngine->SetGrasshopperCGF(slot++, NULL);
	}
	return pH->EndFunction();
}



//
//------------------------------------------------------------------------------

//
//------------------------------------------------------------------------------
int CScriptObjectSystem::GetTerrainElevation(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	CScriptObjectVector oVec(m_pScriptSystem,true);
  Vec3 v3Pos;//,v3SysDir;
	pH->GetParam(1,*oVec);
	v3Pos=oVec.Get();
	float	elevation;

	elevation = m_p3DEngine->GetTerrainElevation(v3Pos.x, v3Pos.y);
	return pH->EndFunction( elevation );
}


//
//------------------------------------------------------------------------------
int CScriptObjectSystem::SetSkyFade(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	pH->GetParam(1,m_SkyFadeStart);
	pH->GetParam(2,m_SkyFadeEnd);

	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/*int CScriptObjectSystem::SetIndoorColor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	CScriptObjectColor oColor(m_pScriptSystem,true);
  Vec3 v3Color;
	pH->GetParam(1,*oColor);
	v3Color=oColor.Get();
	m_p3DEngine->GetBuildingManager()->SetIndoorAmbientColor( v3Color );
	return pH->EndFunction();
}	*/

/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::ActivatePortal(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);

	CScriptObjectVector oPos(m_pScriptSystem,true);
  Vec3 vPos;
	int nID;
	pH->GetParam(1,*oPos);
	vPos=oPos.Get();
	bool bActivate;
	pH->GetParam(2,bActivate);
	pH->GetParam(3,nID);

	IEntity *pEnt = m_pEntitySystem->GetEntity(nID);

//	m_p3DEngine->GetBuildingManager()->ActivatePortal(vPos,bActivate,pEnt);
	m_p3DEngine->ActivatePortal(vPos,bActivate,pEnt);
	ISoundSystem *pSS=m_pSystem->GetISoundSystem();

	if (pSS)
	{
		// recompute the sound occlusion to an opened or closed portal
		pSS->RecomputeSoundOcclusion(false,true);
	}

	return (pH->EndFunction());
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//
//------------------------------------------------------------------------------
int CScriptObjectSystem::SetWorldColorRatio(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	float vRatio;
	pH->GetParam(1, vRatio);

//	m_AmbientColorRatio = vRatio;
//	Vec3 v3Color;

//	v3Color=m_p3DEngine->GetWorldColor( );
//	m_p3DEngine->SetWorldColor( v3Color*m_AmbientColorRatio );
  m_p3DEngine->SetWorldColorRatio(vRatio);

	return pH->EndFunction();
}

//
//------------------------------------------------------------------------------


int CScriptObjectSystem::DumpMMStats(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pSystem->DumpMMStats(true);
	//m_pScriptSystem->GetMemoryStatistics(NULL);
	m_pLog->Log("***SCRIPT GC COUNT [%d kb]",m_pScriptSystem->GetCGCount());
	return pH->EndFunction();
}



int CScriptObjectSystem::EnumDisplayFormats(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pLog->Log("Enumerating display settings...");
	_SmartScriptObject pDispArray(m_pScriptSystem);
	TArray<SDispFormat> Formats;
	int i;
	m_pRenderer->EnumDisplayFormats(Formats, false);
	for (i=0; i<Formats.Num(); i++)
	{
		SDispFormat *pForm = &Formats[i];
		_SmartScriptObject pDisp(m_pScriptSystem);
		pDisp->SetValue("width", pForm->m_Width);
		pDisp->SetValue("height", pForm->m_Height);
		pDisp->SetValue("bpp", pForm->m_BPP);

		// double check for multiple entries of the same resolution/color depth -- CW
		bool bInsert( true );
		for( int j( 0 ); j < pDispArray->Count(); ++j )
		{
			_SmartScriptObject pDispCmp( m_pScriptSystem );
			if( false != pDispArray->GetAt( j + 1, pDispCmp ) )
			{
				int iWidthCmp( 0 );
				pDispCmp->GetValue( "width", iWidthCmp );

				int iHeightCmp( 0 );
				pDispCmp->GetValue( "height", iHeightCmp );

				int iBppCmp( 0 );
				pDispCmp->GetValue( "bpp", iBppCmp );

				if( pForm->m_Width == iWidthCmp &&
					pForm->m_Height == iHeightCmp &&
					pForm->m_BPP == iBppCmp )
				{
					bInsert = false;
					break;
				}
			}
		}
		if( false != bInsert )
		{
			pDispArray->PushBack( pDisp );
		}
	}

	if(Formats.Num()==0)				// renderer is not doing his job
	{
		{
			_SmartScriptObject pDisp(m_pScriptSystem);
			pDisp->SetValue("width", 640);
			pDisp->SetValue("height", 480);
			pDisp->SetValue("bpp", 32);
			pDispArray->SetAt(1, pDisp);
		}
		{
			_SmartScriptObject pDisp(m_pScriptSystem);
			pDisp->SetValue("width", 800);
			pDisp->SetValue("height", 600);
			pDisp->SetValue("bpp", 32);
			pDispArray->SetAt(2, pDisp);
		}
		{
			_SmartScriptObject pDisp(m_pScriptSystem);
			pDisp->SetValue("width", 1024);
			pDisp->SetValue("height", 768);
			pDisp->SetValue("bpp", 32);
			pDispArray->SetAt(3, pDisp);
		}
	}

  m_pRenderer->EnumDisplayFormats(Formats, true);

	return pH->EndFunction(pDispArray);
}

int CScriptObjectSystem::EnumAAFormats(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pLog->Log("Enumerating FSAA modes...");
	_SmartScriptObject pAAArray(m_pScriptSystem);
	TArray<SAAFormat> AAFormats;

	m_pRenderer->EnumAAFormats(AAFormats, false);

	for (int i=0; i<AAFormats.Num(); i++)
	{
		SAAFormat *pAAForm = &AAFormats[i];
		_SmartScriptObject pAA(m_pScriptSystem);

		pAA->SetValue("desc", pAAForm->szDescr);
		pAA->SetValue("samples", pAAForm->nSamples);
		pAA->SetValue("quality", pAAForm->nQuality);

		pAAArray->PushBack( pAA );
	}

	m_pRenderer->EnumAAFormats(AAFormats, true);

	return pH->EndFunction(pAAArray);
}

int CScriptObjectSystem::IsPointIndoors(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	bool bInside=false;

	if(pH->GetParam(1,*oVec))
	{
		Vec3 vPos=oVec.Get();
		I3DEngine *p3dEngine=m_pSystem->GetI3DEngine();
		if (p3dEngine)
			bInside = p3dEngine->GetVisAreaFromPos(vPos)!=0;
	}
	return pH->EndFunction(bInside);
}

int CScriptObjectSystem::SetGammaDelta(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	float fDelta = 0;
	pH->GetParam(1, fDelta);

  m_pSystem->GetIRenderer()->SetGammaDelta(fDelta);

	return pH->EndFunction();
}

int CScriptObjectSystem::ProjectToScreen(IFunctionHandler *pH)
{
	CScriptObjectVector pVec(m_pScriptSystem,false);
	if(pH->GetParam(1,pVec))
	{
		bool nomodelview;
		Vec3 v=pVec.Get();
		if(pH->GetParam(2,nomodelview))
		{

			CCamera c;
			c.SetAngle(Vec3(0,0,0));
			c.Update();
			Matrix44 t=c.GetVCMatrixD3D9();

			m_pRenderer->PushMatrix();

			m_pRenderer->LoadMatrix(&t);
			float rx,ry,rz,zz=0;

			m_pRenderer->ProjectToScreen(v.x,v.y,v.z,&rx,&ry,&rz);
			m_pRenderer->PopMatrix();

			v.x=rx*8.0f;
			v.y=ry*6.0f;
			v.z=zz;
			pVec=v;
		}
		else
		{
		float rx,ry,rz,zz;
		zz=v.z;
		m_pRenderer->ProjectToScreen(v.x,v.y,v.z,&rx,&ry,&rz);
		v.x=rx*8.0f;
		v.y=ry*6.0f;
		v.z=zz;
		pVec=v;
		}
	}

	return pH->EndFunction();
}

int CScriptObjectSystem::EnableHeatVision(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int nEnable=0;
	if(pH->GetParam(1, nEnable))
	{
		m_pSystem->GetI3DEngine()->EnableHeatVision(nEnable>0);
	}
	return pH->EndFunction();
}

/*
int CScriptObjectSystem::IndoorSoundAllowed(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(2);
	CScriptObjectVector pVecOrigin(m_pScriptSystem,false);
	CScriptObjectVector pVecDestination(m_pScriptSystem,false);

	IGraph *pGraph = m_pSystem->GetAISystem()->GetNodeGraph();

	Vec3 start = pVecOrigin.Get();
	Vec3 end = pVecDestination.Get();

	if ((start.x<0)||(start.x>2000.f)||(start.y<0)||(start.y>2000.f))
	{
		m_pLog->Log("\001[CRASHWARNING] Unnaturally high value placed for sound detection");
		return pH->EndFunction(true); // there is no sound occlusion in outdoor
	}

	if ((end.x<0)||(end.x>2000.f)||(end.y<0)||(end.y>2000.f))
	{
		m_pLog->Log("\001[CRASHWARNING] Unnaturally high value placed for sound detection");
		return pH->EndFunction(true); // there is no sound occlusion in outdoor
	}

	GraphNode *pStart = pGraph->GetEnclosing(pVecOrigin.Get());


	if (pStart->nBuildingID<0)
		return pH->EndFunction(true); // there is no sound occlusion in outdoor

	GraphNode *pEnd = pGraph->GetEnclosing(pVecDestination.Get());

	if (pStart->nBuildingID!=pEnd->nBuildingID)
		return pH->EndFunction(false); // if in 2 different buildings we cannot hear the sound for sure

	// make the real indoor sound occlusion check (doors, sectors etc.)
	return (m_pSystem->GetI3DEngine()->IsVisAreasConnected(pStart->pArea,pEnd->pArea,1,false));


	//IIndoorBase *pIndoor = m_p3DEngine->GetBuildingManager();
	//if (pIndoor)
	{
		// make the real indoor sound occlusion check (doors, sectors etc.)
		return pH->EndFunction(pIndoor->IsSoundPotentiallyHearable(pStart->nBuildingID,pStart->nSector,pEnd->nSector));
	//}

	return pH->EndFunction(true);
}
*/


int CScriptObjectSystem::EnableOceanRendering(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(1);
  bool bOcean=true, bShore=true;
  if(pH->GetParam(1,bOcean) && pH->GetParam(2,bShore))
	  if (m_p3DEngine)
		  m_p3DEngine->EnableOceanRendering(bOcean,bShore);

  return pH->EndFunction();
}

int CScriptObjectSystem::Break(IFunctionHandler *pH)
{
#ifdef WIN32
	CryError("CScriptObjectSystem:Break");
#endif
	return pH->EndFunction();
}

int CScriptObjectSystem::DumpCommandsVars(IFunctionHandler *pH)
{
	char *arg = "";
	if(pH->GetParamCount()>0) pH->GetParam(1,arg);
	m_pSystem->GetIConsole()->DumpCommandsVars(arg);
	return pH->EndFunction();
}

int CScriptObjectSystem::SetWaterVolumeOffset(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);

	const char *pszWaterVolumeName=0;
	float fWaterOffsetX=0;
	float fWaterOffsetY=0;
	float fWaterOffsetZ=0;

	pH->GetParam(1, pszWaterVolumeName);
	pH->GetParam(2, fWaterOffsetX);
	pH->GetParam(3, fWaterOffsetY);
	pH->GetParam(4, fWaterOffsetZ);

	IWaterVolume * pWaterVolume = pszWaterVolumeName ? m_p3DEngine->FindWaterVolumeByName(pszWaterVolumeName) : 0;
	if(!pWaterVolume)
	{
		m_pSystem->Warning( VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, 0, 0,
			"CScriptObjectSystem::SetWaterVolumeOffset: Water volume not found: %s", pszWaterVolumeName ? pszWaterVolumeName : "Name is not set");
		return pH->EndFunction(false);
	}

	pWaterVolume->SetPositionOffset(Vec3d(fWaterOffsetX,fWaterOffsetY,fWaterOffsetZ));

	return pH->EndFunction();
}

int CScriptObjectSystem::GetViewCameraPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	CCamera &Camera=m_pSystem->GetViewCamera();
	CScriptObjectVector oVec(m_pScriptSystem);
	oVec.Set(Camera.GetPos());
	return pH->EndFunction(*oVec);
}

int CScriptObjectSystem::RayWorldIntersection(IFunctionHandler *pH)
{
	assert(pH->GetParamCount()>=3 && pH->GetParamCount()<=6);

	CScriptObjectVector vPos(m_pScriptSystem, true);
	CScriptObjectVector vDir(m_pScriptSystem, true);

	int nMaxHits,iEntTypes=ent_all;

	pH->GetParam(1, *vPos);
	pH->GetParam(2, *vDir);
	pH->GetParam(3, nMaxHits);
	pH->GetParam(4, iEntTypes);

	if (nMaxHits>10)
		nMaxHits=10;

	ray_hit RayHit[10];

	//filippo: added support for skip certain entities.
	int skipId1 = -1;
	int skipId2 = -1;

	IPhysicalEntity	*skipPhys1 = NULL;
	IPhysicalEntity	*skipPhys2 = NULL;

	pH->GetParam(5, skipId1);
	pH->GetParam(6, skipId2);

	if (skipId1!=-1)
	{
		IEntity	*skipEnt1 = m_pEntitySystem->GetEntity(skipId1);
		if (skipEnt1)
			skipPhys1 = skipEnt1->GetPhysics();
	}

	if (skipId2!=-1)
	{
		IEntity	*skipEnt2 = m_pEntitySystem->GetEntity(skipId2);
		if (skipEnt2)
			skipPhys2 = skipEnt2->GetPhysics();
	}

	Vec3 src = vPos.Get();
	Vec3 dst = vDir.Get()-src;

	int nHits=m_pPhysicalWorld->RayWorldIntersection(src, dst, iEntTypes,
		geom_colltype0<<rwi_colltype_bit | rwi_stop_at_pierceable, RayHit, nMaxHits,skipPhys1,skipPhys2);

	_SmartScriptObject pObj(m_pScriptSystem);

	for (int i=0;i<nHits;i++)
	{
		_SmartScriptObject pHitObj(m_pScriptSystem);
		CScriptObjectVector vOutPos(m_pScriptSystem);
		CScriptObjectVector vOutNormal(m_pScriptSystem);
		ray_hit &Hit=RayHit[i];
		vOutPos.Set(Hit.pt);
		pHitObj->SetValue("pos", *vOutPos);
		vOutNormal.Set(Hit.n);
		pHitObj->SetValue("normal", *vOutNormal);
		pHitObj->SetValue("dist", Hit.dist);
		pHitObj->SetValue("surface", Hit.surface_idx);
		pObj->SetAt(i+1, *pHitObj);
	}

	return pH->EndFunction(*pObj);
}


//-------------------------------------------------------------------------------------------------
//
int CScriptObjectSystem::RayTraceCheck(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);
	CScriptObjectVector vPos(m_pScriptSystem, true);
	Vec3 src, dst;
	int	skipId1, skipId2;

	pH->GetParam(1, *vPos);
	src = vPos.Get();
	pH->GetParam(2, *vPos);
	dst = vPos.Get();
	pH->GetParam(3, skipId1);
	pH->GetParam(4, skipId2);

	IEntity	*skipEnt1 = m_pEntitySystem->GetEntity(skipId1);
	IEntity	*skipEnt2 = m_pEntitySystem->GetEntity(skipId2);
	IPhysicalEntity	*skipPhys1=NULL;
	IPhysicalEntity	*skipPhys2=NULL;

	if(skipEnt1) skipPhys1 = skipEnt1->GetPhysics();
	if(skipEnt2) skipPhys2 = skipEnt2->GetPhysics();

	ray_hit RayHit;
	//TODO? add an extraparam to specify what kind of objects to check? now its world and static
	int nHits=m_pPhysicalWorld->RayWorldIntersection(src, dst-src, ent_static|ent_terrain,	rwi_ignore_noncolliding |  rwi_stop_at_pierceable, &RayHit, 1, skipPhys1, skipPhys2 );

	return pH->EndFunction((bool)(nHits==0));
}



//-------------------------------------------------------------------------------------------------
int CScriptObjectSystem::BrowseURL(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	char *szURL;
	pH->GetParam(1, szURL);

	// for security reasons, check if it really a url
	if (strlen(szURL) >= 10)
	{
		// check for http and : as in http://
		// : might be on position 5, for https://

		if (!strncmp("http", szURL, 4) && ((szURL[4] == ':') || (szURL[5] == ':')))
		{
#ifdef WIN32

			ShellExecute(0, "open", szURL, 0, 0, SW_MAXIMIZE);

#else

#pragma message("WARNING: CScriptObjectSystem::BrowseURL() not implemented on this platform!")

#endif
		}
	}

	return pH->EndFunctionNull();
}
//////////////////////////////////////////////////////////////////////////

int CScriptObjectSystem::GetCPUQuality( IFunctionHandler* pH )
{
	CHECK_PARAMETERS( 0 );

	// return cpu quality -- that is, clock speed of CPU in MHz
	float fMHz( (float) ( 1e-6 / m_pSystem->GetSecondsPerCycle() ) );
	if( false != IsAMD64() )
	{
		fMHz *= 1.5f; // AMD64 with 2 GHz is equal to a 3.0 GHz machine
		fMHz = ( fMHz < 3000.0f ) ? 3000.0f : fMHz;
	}

	int iSysSpec( 0 );
	if( fMHz < 1600.0f )
	{
		// < 1.7 GHz (1.9 GHz = safety net) is minimum spec
		iSysSpec = 0;
	}
	else if( fMHz < 2500.0f )
	{
		// < 2.6 GHz (2.5 GHz = safety net) is medium spec
		iSysSpec = 1;
	}
	else if( fMHz < 2900.0f )
	{
		// < 3.0 GHz (2.9 GHz = safety net) is high spec
		iSysSpec = 2;
	}
	else
	{
		// else very high spec
		iSysSpec = 3;
	}

	return( pH->EndFunction( iSysSpec ) );
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::GetGPUQuality( IFunctionHandler* pH )
{
	CHECK_PARAMETERS( 0 );
	static int s_iGPUQuality( -1 );
#if !defined(LINUX)
	if( -1 == s_iGPUQuality )
	{
		HMODULE hDDraw( LoadLibrary( "ddraw.dll" ) );
		if( 0 != hDDraw )
		{
			s_iGPUQuality = 666; // Unknown card

			typedef HRESULT (WINAPI *FP_DirectDrawCreateEx) ( GUID* lpGUID, void* lplpDD, REFIID iid, IUnknown* pUnkOuter );
			FP_DirectDrawCreateEx fpDirectDrawCreateEx( (FP_DirectDrawCreateEx) GetProcAddress( hDDraw, "DirectDrawCreateEx" ) );

			LPDIRECTDRAW7 pDD7( 0 );
			if( SUCCEEDED( fpDirectDrawCreateEx( 0, &pDD7, IID_IDirectDraw7, 0 ) ) )
			{
				DDDEVICEIDENTIFIER2 DeviceIdentifier;
				memset( &DeviceIdentifier, 0, sizeof( DeviceIdentifier ) );
				if( SUCCEEDED( pDD7->GetDeviceIdentifier( &DeviceIdentifier, 0 ) ) )
				{
					switch( DeviceIdentifier.dwVendorId )
					{
					case 0x1002: // ATI
						{
							switch( DeviceIdentifier.dwDeviceId )
							{
								case 0x4c64: s_iGPUQuality = 0; break; // Radeon Mobility 9000
								case 0x4c66: s_iGPUQuality = 0; break; // Radeon Mobility 9000
								case 0x4966: s_iGPUQuality = 0; break; // Radeon 9000
								case 0x496e: s_iGPUQuality = 0; break; // Radeon 9000 - Secondary
								case 0x514d: s_iGPUQuality = 0; break; // Radeon 9100
								case 0x5834: s_iGPUQuality = 0; break; // Radeon 9100 IGP
								case 0x4242: s_iGPUQuality = 0; break; // Radeon 8500 DV
								case 0x4152: s_iGPUQuality = 1; break; // Radeon 9600
								case 0x4172: s_iGPUQuality = 1; break; // Radeon 9600 - Secondary
								case 0x4164: s_iGPUQuality = 1; break; // Radeon 9500 - Secondary
								case 0x4144: s_iGPUQuality = 2; break; // Radeon 9500
								case 0x4e45: s_iGPUQuality = 2; break; // Radeon 9500 Pro / 9700
								case 0x4150: s_iGPUQuality = 1; break; // Radeon 9600 Pro
								case 0x4151: s_iGPUQuality = 1; break; // Radeon 9600
								case 0x4170: s_iGPUQuality = 1; break; // Radeon 9600 Pro - Secondary
								case 0x4171: s_iGPUQuality = 1; break; // Radeon 9600 - Secondary
								case 0x4e46: s_iGPUQuality = 1; break; // Radeon 9600 TX
								case 0x4e66: s_iGPUQuality = 1; break; // Radeon 9600 TX - Secondary
								case 0x4e44: s_iGPUQuality = 3; break; // Radeon 9700 Pro
								case 0x4e64: s_iGPUQuality = 3; break; // Radeon 9700 Pro - Secondary
								case 0x4e65: s_iGPUQuality = 2; break; // Radeon 9500 Pro / 9700 - Secondary
								case 0x4e49: s_iGPUQuality = 3; break; // Radeon 9800
								case 0x4e69: s_iGPUQuality = 3; break; // Radeon 9800 - Secondary
								case 0x4148: s_iGPUQuality = 3; break; // Radeon 9800
								case 0x4168: s_iGPUQuality = 3; break; // Radeon 9800 - Secondary
								case 0x4e48: s_iGPUQuality = 3; break; // Radeon 9800 Pro
								case 0x4e68: s_iGPUQuality = 3; break; // Radeon 9800 Pro - Secondary
								case 0x4e4a: s_iGPUQuality = 3; break; // Radeon 9800 XT
								case 0x4e6a: s_iGPUQuality = 3; break; // Radeon 9800 XT - Secondary
								case 0x5960: s_iGPUQuality = 1; break; // Radeon 9200 Pro
								case 0x5940: s_iGPUQuality = 1; break; // Radeon 9200 Pro - Secondary
								case 0x5961: s_iGPUQuality = 0; break; // Radeon 9200
								case 0x5941: s_iGPUQuality = 1; break; // Radeon 9200 - Secondary
								case 0x5964: s_iGPUQuality = 1; break; // Radeon 9200SE
								case 0x514c: s_iGPUQuality = 0; break; // Radeon 8500
								case 0x514e: s_iGPUQuality = 0; break; // Radeon 8500
								case 0x514f: s_iGPUQuality = 0; break; // Radeon 8500
				        case 0x4136: s_iGPUQuality = 0; break; // IGP 320
				        case 0x4137: s_iGPUQuality = 0; break; // IGP 340
				        case 0x4a49: s_iGPUQuality = 3; break; // Radeon X800 Pro
				        case 0x4a4a: s_iGPUQuality = 3; break; // Radeon X800 SE
				        case 0x4a4b: s_iGPUQuality = 3; break; // Radeon X800
				        case 0x4a4c: s_iGPUQuality = 3; break; // Radeon X800 Series
				        case 0x4a50: s_iGPUQuality = 3; break; // Radeon X800 XT
				        case 0x4a69: s_iGPUQuality = 3; break; // Radeon X800 Pro Secondary
				        case 0x4a6a: s_iGPUQuality = 3; break; // Radeon X800 SE Secondary
				        case 0x4a6b: s_iGPUQuality = 3; break; // Radeon X800 Secondary
				        case 0x4a6c: s_iGPUQuality = 3; break; // Radeon X800 Series Secondary
				        case 0x4a70: s_iGPUQuality = 3; break; // Radeon X800 XT Secondary
							}
						}

					case 0x10b4: // NVIDIA
					case 0x12d2:
					case 0x10de:
						{
							switch( DeviceIdentifier.dwDeviceId )
							{
								case 0x0152: s_iGPUQuality = 0; break; // GeForce2 Ultra
								case 0x0153: s_iGPUQuality = 0; break; // Quadro2 Pro
								case 0x0170: s_iGPUQuality = 0; break; // GeForce4 MX 460
								case 0x0171: s_iGPUQuality = 0; break; // GeForce4 MX 440
								case 0x0172: s_iGPUQuality = 0; break; // GeForce4 MX 420
								case 0x0173: s_iGPUQuality = 0; break; // GeForce4 MX 440-SE
								case 0x0174: s_iGPUQuality = 0; break; // GeForce4 Go 440
								case 0x0175: s_iGPUQuality = 0; break; // GeForce4 Go 420
								case 0x0176: s_iGPUQuality = 0; break; // GeForce4 Go 420
								case 0x0178: s_iGPUQuality = 0; break; // Quadro4 550 XGL
								case 0x0179: s_iGPUQuality = 0; break; // GeForce4 Go 440
								case 0x017a: s_iGPUQuality = 0; break; // Quadro NVS
								case 0x017b: s_iGPUQuality = 0; break; // Quadro 550 XGL
								case 0x0181: s_iGPUQuality = 0; break; // GeForce4 MX 440 with AGP8X
								case 0x0182: s_iGPUQuality = 0; break; // GeForce4 MX 440SE with AGP8X
								case 0x0183: s_iGPUQuality = 0; break; // GeForce4 MX 420 with AGP8X
								case 0x0188: s_iGPUQuality = 0; break; // Quadro4 580 XGL
								case 0x018a: s_iGPUQuality = 0; break; // Quadro NVS with AGP8X
								case 0x018b: s_iGPUQuality = 0; break; // Quadro4 380 XGL
								case 0x01f0: s_iGPUQuality = 0; break; // GeForce4 MX Integrated GPU (nForce2)
								case 0x0200: s_iGPUQuality = 0; break; // GeForce3
								case 0x0201: s_iGPUQuality = 0; break; // GeForce3 Ti200
								case 0x0202: s_iGPUQuality = 0; break; // GeForce3 Ti500
								case 0x0203: s_iGPUQuality = 0; break; // Quadro DCC
								case 0x0250: s_iGPUQuality = 1; break; // GeForce4 Ti 4600
								case 0x0251: s_iGPUQuality = 1; break; // GeForce4 Ti 4400
								case 0x0253: s_iGPUQuality = 0; break; // GeForce4 Ti 4200
								case 0x0258: s_iGPUQuality = 0; break; // Quadro4 900 XGL
								case 0x0259: s_iGPUQuality = 0; break; // Quadro4 750 XGL
								case 0x025b: s_iGPUQuality = 0; break; // Quadro4 700 XGL
								case 0x0280: s_iGPUQuality = 1; break; // GeForce4 Ti 4800
								case 0x0281: s_iGPUQuality = 0; break; // GeForce4 Ti4200 with AGP8X
								case 0x0282: s_iGPUQuality = 0; break; // GeForce4 Ti 4800 SE
								case 0x0288: s_iGPUQuality = 0; break; // Quadro4 980 XGL
								case 0x0289: s_iGPUQuality = 0; break; // Quadro4 780 XGL
								case 0x02a0: s_iGPUQuality = 0; break; // GeForce3 XBOX
								case 0x0301: s_iGPUQuality = 1; break; // GeForce FX 5800 Ultra
								case 0x0302: s_iGPUQuality = 1; break; // GeForce FX 5800
								case 0x0308: s_iGPUQuality = 1; break; // Quadro FX 2000
								case 0x0309: s_iGPUQuality = 1; break; // Quadro FX 1000
								case 0x030a: s_iGPUQuality = 1; break; // ICE FX 2000
								case 0x0311: s_iGPUQuality = 1; break; // GeForce FX 5600 Ultra
								case 0x0312: s_iGPUQuality = 1; break; // GeForce FX 5600
								case 0x0313: s_iGPUQuality = 1; break; // NV31
								case 0x0314: s_iGPUQuality = 1; break; // GeForce FX 5600XT
								case 0x031a: s_iGPUQuality = 1; break; // GeForce FX Go5600
								case 0x0321: s_iGPUQuality = 1; break; // GeForce FX 5200 Ultra
								case 0x0322: s_iGPUQuality = 1; break; // GeForce FX 5200
								case 0x0323: s_iGPUQuality = 1; break; // GeForce FX 5200SE
								case 0x032b: s_iGPUQuality = 1; break; // Quadro FX 500
								case 0x032f: s_iGPUQuality = 1; break; // NV34GL
								case 0x0330: s_iGPUQuality = 3; break; // GeForce FX 5900 Ultra
								case 0x0331: s_iGPUQuality = 2; break; // GeForce FX 5900
								case 0x0332: s_iGPUQuality = 2; break; // NV35
								case 0x0333: s_iGPUQuality = 3; break; // GeForce FX 5950 Ultra
								case 0x0338: s_iGPUQuality = 2; break; // Quadro FX 3000
								case 0x0341: s_iGPUQuality = 2; break; // GeForce FX 5700 Ultra
								case 0x0342: s_iGPUQuality = 2; break; // GeForce FX 5700
								case 0x034e: s_iGPUQuality = 1; break; // Quadro FX 1100
								case 0x0040:													 // GeForce 6800 Ultra
								case 0x0041:													 // GeForce 6800
								case 0x0042:
								case 0x0043:
								case 0x0044:
								case 0x0045:
								case 0x0046:
								case 0x0047:
								case 0x0048:
								case 0x0049:													 // NV40GL
								case 0x004a:
								case 0x004b:
								case 0x004c:
								case 0x004d:
								case 0x004e:													 // NV40GL
								case 0x004f: s_iGPUQuality = 3; break; // NV40 (GeForce ???)
								case 0x00f9: s_iGPUQuality = 3; break; // GeForce 6800 Ultra
								case 0x00fa: s_iGPUQuality = 2; break; // GeForce PCX 5750
								case 0x00fb: s_iGPUQuality = 2; break; // GeForce PCX 5900
								case 0x00fc: s_iGPUQuality = 1; break; // GeForce PCX 5300
								case 0x00fd: s_iGPUQuality = 2; break; // Quadro PCI-E Series
								case 0x00fe: s_iGPUQuality = 2; break; // Quadro FX 1300
								case 0x00ff: s_iGPUQuality = 1; break; // GeForce PCX 4300
							}
						}
					}
				}
				pDD7->Release();
			}

			FreeLibrary( hDDraw );
		}
	}

  // Hack: No gpu detected ? Could be newer gpu, check for pixel shaders support
  if(s_iGPUQuality==666)
  {
    if(m_pRenderer->GetFeatures() & RFT_HW_PS20)
    {
      s_iGPUQuality=2;
    }
    else
    if(m_pRenderer->GetFeatures() & RFT_HW_TS)
    {
      s_iGPUQuality=1;
    }
    else
    {
      s_iGPUQuality=0;
    }
  }  

#endif
	// return gpu quality level in range [0..3]
	return( pH->EndFunction( s_iGPUQuality ) );
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::GetSystemMem( IFunctionHandler* pH )
{
	CHECK_PARAMETERS( 0 );

	MEMORYSTATUS sMemStat;
	GlobalMemoryStatus( &sMemStat );
	// return size of total physical memory in MB
	int iSysMemInMB( sMemStat.dwTotalPhys >> 20 );

	return( pH->EndFunction( iSysMemInMB ) );
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::GetVideoMem( IFunctionHandler* pH )
{
	CHECK_PARAMETERS( 0 );
	static DWORD s_dwTotalVideoMemory( 0xFFFFFFFF );
#if !defined(LINUX)
	if( 0xFFFFFFFF == s_dwTotalVideoMemory )
	{
		s_dwTotalVideoMemory = 0;

		HMODULE hDDraw( LoadLibrary( "ddraw.dll" ) );
		if( 0 != hDDraw )
		{
			typedef HRESULT (WINAPI *FP_DirectDrawCreateEx) ( GUID* lpGUID, void* lplpDD, REFIID iid, IUnknown* pUnkOuter );
			FP_DirectDrawCreateEx fpDirectDrawCreateEx( (FP_DirectDrawCreateEx) GetProcAddress( hDDraw, "DirectDrawCreateEx" ) );

			LPDIRECTDRAW7 pDD7( 0 );
			if( SUCCEEDED( fpDirectDrawCreateEx( 0, &pDD7, IID_IDirectDraw7, 0 ) ) )
			{
				DDSCAPS2 sDDSCaps2;
				ZeroMemory( &sDDSCaps2, sizeof( sDDSCaps2 ) );
				sDDSCaps2.dwCaps = DDSCAPS_LOCALVIDMEM;

				HRESULT hr( pDD7->GetAvailableVidMem( &sDDSCaps2, &s_dwTotalVideoMemory, 0 ) );
				assert( S_OK == hr );

				pDD7->Release();
			}

			FreeLibrary( hDDraw );
		}
	}
#endif
	// return size of available video memory in MB
	return( pH->EndFunction( (int) ( s_dwTotalVideoMemory >> 20 ) ) );
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::IsPS20Supported( IFunctionHandler* pH )
{
	CHECK_PARAMETERS( 0 );
	bool bPS20( 0 != ( m_pSystem->GetIRenderer()->GetFeatures() & RFT_HW_PS20 ) );
	return( pH->EndFunction( false != bPS20 ? 1 : 0 ) );
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::IsHDRSupported( IFunctionHandler* pH )
{
	CHECK_PARAMETERS(0);

	if (m_pSystem->GetIRenderer()->GetFeatures() & RFT_HW_HDR)
	{
		return pH->EndFunction((int)1);
	}
	else
	{

		return pH->EndFunctionNull();
	}
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::IsDevModeEnable(IFunctionHandler* pH)
{
	CHECK_PARAMETERS( 0 );

	// check if we're running in devmode (cheat mode)
	// to check if we are allowed to enable certain scripts
	// function facilities (god mode, fly mode etc.)

	IGame *pGame=m_pSystem->GetIGame();				assert(pGame);

	return pH->EndFunction( pGame->GetModuleState( EGameDevMode ) );
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::SaveConfiguration(IFunctionHandler* pH)
{
	CHECK_PARAMETERS( 0 );

	m_pSystem->SaveConfiguration();

	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSystem::SetSystemShaderRenderFlags(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	const char *pszShaderName=NULL;
	const char *pszFlagName=NULL;
	bool bEnable = false;
	pH->GetParam(1, pszShaderName);
	pH->GetParam(2, pszFlagName);
	pH->GetParam(3, bEnable);

	typedef struct
	{
		int dwFlagValue;
		char * szFlagName;
	} flagsDesc;

	static flagsDesc arrFlags[] =
	{
		{DLD_TERRAIN_WATER, "DrawWater"},
		{DLD_TERRAIN, "DrawTerrain"},
		{DLD_DETAIL_TEXTURES, "DrawDetailTextures"},
		{DLD_DETAIL_OBJECTS, "DrawDetailObjects"},
		{DLD_FAR_SPRITES, "DrawFarSprites"},
		{DLD_STATIC_OBJECTS, "DrawStaticObjects"},
		{DLD_ENTITIES, "DrawEntities"},
		{DLD_PARTICLES, "DrawParticles"},
		{DLD_TERRAIN_LIGHT, "UseLights"},
		{DLD_TERRAIN_FULLRES, "FullDetailTerrain"},
		{0,0}
	};

	int dwFlag = 0;
	for(int i=0; arrFlags[i].dwFlagValue; i++)
		if(stricmp(arrFlags[i].szFlagName, pszFlagName)==0)
			dwFlag |= arrFlags[i].dwFlagValue;

	IShader * pShader = m_pRenderer->EF_LoadShader(pszShaderName,eSH_World,EF_SYSTEM);
	if(bEnable)
		pShader->SetRenderFlags(pShader->GetRenderFlags() | dwFlag);
	else
		pShader->SetRenderFlags(pShader->GetRenderFlags() & ~dwFlag);

	return pH->EndFunction();
}