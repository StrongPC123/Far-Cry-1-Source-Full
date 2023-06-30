// ScriptObjectSystem.h: interface for the CScriptObjectSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTSYSTEM_H__AE30A606_B76F_45EA_8187_B97044772DD0__INCLUDED_)
#define AFX_SCRIPTOBJECTSYSTEM_H__AE30A606_B76F_45EA_8187_B97044772DD0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>

struct ISystem;
struct ILog;
struct ISoundSystem;
struct IRenderer;
struct IConsole;
struct IInput;
struct ITimer;
struct IEntitySystem;
struct I3DEngine;
class IPhysicalWorld;

/*! This class implements script-functions for exposing the System functionalities

	REMARKS:
	this object doesn't have a global mapping(is not present as global variable into the script state)
		
	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/

class CScriptObjectSystem :
public _ScriptableEx<CScriptObjectSystem>
{ 
public:
	CScriptObjectSystem();
	virtual ~CScriptObjectSystem();
	void Init(IScriptSystem *pScriptSystem,ISystem *pSystem);
public:
	int CreateDownload(IFunctionHandler *pH);
	int LoadFont(IFunctionHandler *pH); //string (return void)
	int ExecuteCommand(IFunctionHandler *pH); //string (return void)
	int LogToConsole(IFunctionHandler *pH); //string (return void)
	int ClearConsole(IFunctionHandler *pH); // void (return void)
	int GetConsoleKeyName(IFunctionHandler *pH); // void (return string)
	int Log(IFunctionHandler *pH); // void (string))
	int LogAlways(IFunctionHandler *pH); // void (string))
	int Warning(IFunctionHandler *pH); //string (return void)
	int Error(IFunctionHandler *pH); //string (return void)
	int GetCurrTime(IFunctionHandler *pH); //void (return float)
	int GetCurrAsyncTime(IFunctionHandler *pH); //void (return float)
	int GetFrameTime(IFunctionHandler *pH); //void (return float)
	int GetLocalOSTime(IFunctionHandler *pH); //void (return float)
	int ShowConsole(IFunctionHandler *pH); //int (return void)
//	int PostMessage(IFunctionHandler *pH); //string (return void)
	int GetEntity(IFunctionHandler *pH); //int (return obj)
	int GetEntities(IFunctionHandler *pH); //int (return obj[])
	int GetEntitiesInRadius(IFunctionHandler *pH); // table of entities
	int GetTeamMembers(IFunctionHandler *pH); // table of entities
	int GetEntityByName(IFunctionHandler *pH); //char * (return obj)
	int LoadAnimatedTexture(IFunctionHandler *pH); //char *,int (return int)
	int LoadTexture(IFunctionHandler *pH); //char *, int (return int)
	int LoadObject(IFunctionHandler *pH); 
	int DrawSprite(IFunctionHandler *pH); //char * vector int(return int)
	int DrawLabelImage(IFunctionHandler *pH); //Vec3d,float,int (return void)
	int DeformTerrain(IFunctionHandler *pH);//Vector,float,int(return void)
	int ScreenToTexture(IFunctionHandler *pH); //void(return void)
	int LoadImage(IFunctionHandler *pH); //const char* (return int)
	int FreeImage(IFunctionHandler *pH); //int (return int)
	int DrawTriStrip(IFunctionHandler *pH);
	int DrawLine(IFunctionHandler *pH);
	int Draw2DLine(IFunctionHandler *pH);
	int DrawImage(IFunctionHandler *pH); //int,int,int,int,int,int (return void)
	int DrawImageColor(IFunctionHandler *pH); //int,int,int,int,int,int,float,float,float,float (return void)
	int DrawImageColorCoords(IFunctionHandler *pH); //int,int,int,int,int,int,float,float,float,float, float, float, float, float (return void)
	int DrawImageCoords(IFunctionHandler *pH); //int,int,int,int,int,int,float,float,float,float, float, float, float, float (return void)
	int SetWorldColorRatio(IFunctionHandler *pH);	//float
  int SetGammaDelta(IFunctionHandler *pH);	//float
	int DrawRectShader(IFunctionHandler *pH);	//const char*,int,int,int,int
  int SetScreenShader(IFunctionHandler *pH);//const char*

  // tiago: added
  int SetScreenFx(IFunctionHandler *pH);//const char*, int
  int SetScreenFxParamInt(IFunctionHandler *pH);//const char*, const char*, int
  int SetScreenFxParamFloat(IFunctionHandler *pH);//const char*, const char*, float
  int GetScreenFx(IFunctionHandler *pH);//const char*
  int GetScreenFxParamInt(IFunctionHandler *pH);//const char*, const char*
  int GetScreenFxParamFloat(IFunctionHandler *pH);//const char*, const char*    
  int SetScissor(IFunctionHandler *pH);//int, int, int, int

	// CW: added for script based system analysis
	int GetCPUQuality( IFunctionHandler *pH );
	int GetGPUQuality( IFunctionHandler *pH );
	int GetSystemMem( IFunctionHandler *pH );
	int GetVideoMem( IFunctionHandler *pH );
	int IsPS20Supported( IFunctionHandler *pH );
	int IsHDRSupported( IFunctionHandler *pH );

	int RemoveEntity(IFunctionHandler *pH); //int (return void)
	int SpawnEntity(IFunctionHandler *pH); //int (return int)
	int ActivateLight(IFunctionHandler *pH); //name, activate
//	int ActivateMainLight(IFunctionHandler *pH); //pos, activate
	int SetSkyBox(IFunctionHandler *pH); //szShaderName, fBlendTime, bUseWorldBrAndColor
	int SetWaterVolumeOffset(IFunctionHandler *pH); //szShaderName, fBlendTime, bUseWorldBrAndColor
	int MeasureTime(IFunctionHandler *pH);
	int IsValidMapPos(IFunctionHandler *pH);
	int EnableMainView(IFunctionHandler *pH);
	int EnableOceanRendering(IFunctionHandler *pH); // int
	int	ScanDirectory(IFunctionHandler *pH);
	int DebugStats(IFunctionHandler *pH);
	int ViewDistanceSet(IFunctionHandler *pH);
	int ViewDistanceGet(IFunctionHandler *pH);
	int SetFogEnd(IFunctionHandler *pH);
	int SetFogStart(IFunctionHandler *pH);
	int SetFogColor(IFunctionHandler *pH);
	int GetFogEnd(IFunctionHandler *pH);
	int GetFogStart(IFunctionHandler *pH);
	int GetFogColor(IFunctionHandler *pH);
	int GetWorldColor(IFunctionHandler *pH);
	int SetWorldColor(IFunctionHandler *pH);  
	int GetOutdoorAmbientColor(IFunctionHandler *pH);
	int SetOutdoorAmbientColor(IFunctionHandler *pH);
	int SetBFCount(IFunctionHandler *pH);
	int GetBFCount(IFunctionHandler *pH);
	int SetGrasshopperCount(IFunctionHandler *pH);
	int GetGrasshopperCount(IFunctionHandler *pH);
	int SetGrasshopperCGF(IFunctionHandler *pH);
	int GetTerrainElevation(IFunctionHandler *pH);
	int SetSkyFade(IFunctionHandler *pH);
//	int SetIndoorColor(IFunctionHandler *pH);
	int	ActivatePortal(IFunctionHandler *pH);
	int DumpMMStats(IFunctionHandler *pH);
	int EnumAAFormats(IFunctionHandler *pH);
	int EnumDisplayFormats(IFunctionHandler *pH);
	int IsPointIndoors(IFunctionHandler *pH);
	int SetConsoleImage(IFunctionHandler *pH);
	int ProjectToScreen(IFunctionHandler *pH);
	int EnableHeatVision(IFunctionHandler *pH);
	int ShowDebugger(IFunctionHandler *pH);
	int FrameProfiler(IFunctionHandler *pH);
	int DumpMemStats (IFunctionHandler *pH);
	int DumpWinHeaps (IFunctionHandler *pH);
	int Break(IFunctionHandler *pH);
	int DumpCommandsVars(IFunctionHandler *pH);
	int GetViewCameraPos(IFunctionHandler *pH);
	int RayWorldIntersection(IFunctionHandler *pH);
	int RayTraceCheck(IFunctionHandler *pH);
	int BrowseURL(IFunctionHandler *pH);
	int IsDevModeEnable(IFunctionHandler* pH);
	int SaveConfiguration(IFunctionHandler *pH);
	int SetSystemShaderRenderFlags(IFunctionHandler *pH);

	static void InitializeTemplate(IScriptSystem *pSS);
	static void ReleaseTemplate();

private:
	ISystem *							m_pSystem;
	ILog *								m_pLog;
	ISoundSystem *				m_pSoundSytem;
	IRenderer *						m_pRenderer;
	IConsole *						m_pConsole;
	ITimer *							m_pTimer;
	IEntitySystem *				m_pEntitySystem;
	I3DEngine *						m_p3DEngine;
	IPhysicalWorld *			m_pPhysicalWorld;

	ICVar *e_deformable_terrain; //!< the Cvar is created in cry3dengine, this is just a pointer

	//log a tring to console and/or to disk with support for different languages
	void	LogString(IFunctionHandler *pH,bool bToConsoleOnly);

	//Vlad is too lazy to add this to 3DEngine - so I have to put it here. It should 
	//not be here, but I have to put it somewhere.....
	float	m_SkyFadeStart;		// when fogEnd less - start to fade sky to fog
	float	m_SkyFadeEnd;			// when fogEnd less - no sky, just fog

	static IScriptObject *m_pScriptTimeTable;

public:
	int ApplyForceToEnvironment(IFunctionHandler * pH);
	//int IndoorSoundAllowed(IFunctionHandler * pH);
	//int ApplyStormToEnvironment(IFunctionHandler * pH);
};

#endif // !defined(AFX_SCRIPTOBJECTSYSTEM_H__AE30A606_B76F_45EA_8187_B97044772DD0__INCLUDED_)
