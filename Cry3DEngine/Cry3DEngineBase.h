////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cry3denginebase.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Access to external stuff used by 3d engine. Most 3d engine classes 
//               are derived from this base class to access other interfaces
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _Cry3DEngineBase_h_
#define _Cry3DEngineBase_h_

struct ISystem;
struct IRenderer;
struct ILog;
class  IPhysicalWorld;
struct ITimer;
struct IConsole;
struct I3DEngine;
struct CVars;
struct CVisAreaManager;

struct Cry3DEngineBase
{
  static ISystem * m_pSys;
  static IRenderer * m_pRenderer;
  static ITimer * m_pTimer;
  static ILog * m_pLog;
  static IPhysicalWorld * m_pPhysicalWorld;
  static IConsole * m_pConsole;
  static I3DEngine * m_p3DEngine;
  static CVars * m_pCVars;
  static ICryPak * m_pCryPak;
  static int m_nRenderStackLevel;
  static int m_dwRecursionDrawFlags[2];
	static int m_nRenderFrameID;
	static bool m_bProfilerEnabled;
	static float m_fPreloadStartTime;

  static int m_CpuFlags;
  static double m_SecondsPerCycle;
	static ESystemConfigSpec m_configSpec;
  static ESystemConfigSpec m_LightConfigSpec;

	static bool m_bIgnoreFakeMaterialsInCGF;
	static bool m_bEditorMode;

  static ISystem	       * GetSystem() { return m_pSys; }
  static IRenderer	     * GetRenderer() { return m_pRenderer; }
  static ITimer         * GetTimer() { return m_pTimer; }
  static ILog           * GetLog() { return m_pLog; }
  static IPhysicalWorld * GetPhysicalWorld() { return m_pPhysicalWorld;}
  static IConsole       * GetConsole() { return m_pConsole; }
  static I3DEngine      * Get3DEngine() { return m_p3DEngine; }
  static CVars          * GetCVars() { return m_pCVars; }
  static CVisAreaManager* GetVisAreaManager();
  static ICryPak        * GetPak() { return m_pCryPak; }

	static int GetFrameID() { return m_nRenderFrameID; };

  CCamera & GetViewCamera() ;
  float GetCurTimeSec();
	float GetCurAsyncTimeSec();
  void UpdateLoadingScreen(const char *command,...);
  void UpdateLoadingScreenPlus(const char *command,...);

	// Validator warning.
	static void Warning( int flags,const char *file,const char *format,... );
	
	CCObject * GetIdentityCCObject() 
	{
		CCObject * pCCObject = GetRenderer()->EF_GetObject(true);
		pCCObject->m_Matrix.SetIdentity();
		return pCCObject;
	}
};

#endif // _Cry3DEngineBase_h_
