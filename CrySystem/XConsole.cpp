// XConsole.cpp: implementation of the CXConsole class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XConsole.h"
#include "XConsoleVariable.h"
#include "Font.h"
#include "System.h"

#include <IInput.h>
#include <ITimer.h>
#include <IScriptSystem.h>
#include <IInput.h>
#include <IRenderer.h>
#include <ISystem.h>
#include <ILog.h>
#include <IProcess.h>
#include <IGame.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXConsole::CXConsole()
{
	m_pFont=NULL;
	m_pRenderer=NULL;
	m_pInput=NULL;
	m_pImage=NULL;
	m_pXFont=NULL;
	m_pScriptSystem=NULL;
	m_nCursorPos=0;
	m_nScrollPos=0;
	m_nScrollMax=600;
	m_nScrollLine=0;
	m_nHistoryPos=-1;
	m_bRepeat=false;
	m_nTabCount=0;
	m_bConsoleActive=false;
	m_sdScrollDir = sdNONE;
	m_pSystem=NULL;
	m_pKeyboard = NULL;
	con_line_buffer_size=NULL;
	m_bStaticBackground=false;
	m_nProgress = 0;
	m_nProgressRange = 0;
	m_nLoadingBarTexID = 0;
	m_nLoadingBackTexID = 0;
}

CXConsole::~CXConsole()
{
	if(!m_mapVariables.empty())
	{
		while (!m_mapVariables.empty())
		{
			m_mapVariables.begin()->second->Release();
		}
		m_mapVariables.clear();
	}

	if(m_pXFont)
	{
	  SAFE_RELEASE(m_pXFont);
		m_pXFont=NULL;
	}
}

void CXConsole::FreeRenderResources()
{
  if (m_pRenderer)
  {
    if (m_nLoadingBarTexID)
		{
      m_pRenderer->RemoveTexture(m_nLoadingBarTexID);
			m_nLoadingBarTexID = -1;
		}
    if (m_nLoadingBackTexID)
		{
      m_pRenderer->RemoveTexture(m_nLoadingBackTexID);
			m_nLoadingBackTexID = -1;
		}
    if (m_pImage)
      m_pImage->Release();
  }
}

//////////////////////////////////////////////////////////////////////////
void CXConsole::Release()
{
	delete this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Initialize the Console
	@param pSystem pointer to the System
*/
void CXConsole::Init(CSystem *pSystem)
{
	m_pSystem=pSystem;
	if (pSystem->GetICryFont())
		m_pFont=pSystem->GetICryFont()->GetFont("Console");
	m_pRenderer=pSystem->GetIRenderer();
	m_pInput=pSystem->GetIInput();   
	m_pTimer=pSystem->GetITimer();
	m_pScriptSystem=pSystem->GetIScriptSystem();
	if (m_pInput)
	{
		m_pKeyboard=m_pInput->GetIKeyboard();
		// Assign this class as input listener.
		m_pInput->AddConsoleEventListener( this );
	}
	con_display_last_messages=CreateVariable("con_display_last_messages","0",0);  // keep default at 1, needed for gameplay
	con_line_buffer_size=CreateVariable("con_line_buffer_size","1000",0);

	if (m_pRenderer)
	{
		m_nLoadingBarTexID = -1;
		m_nLoadingBackTexID = -1;

		ITexPic *pTex = 0;		
	
		pTex = pSystem->GetIRenderer()->EF_LoadTexture("console/loadingbar", FT_NORESIZE | FT_NOMIPS, 0, eTT_Base);
		if (pTex) m_nLoadingBarTexID = pTex->GetTextureID();
	}
	else
	{
		m_nLoadingBarTexID = -1;
		m_nLoadingBackTexID = -1;
	} 
	if(pSystem && pSystem->IsDedicated())
			m_bConsoleActive = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Crate a new console variable
	@param sName console variable name
	@param sValue default value
	@param nFlag user definded flag, this parameter is used by other subsystems 
		and doesn't affect the console varible(besically of user data)
	@see ICVar
*/
ICVar *CXConsole::CreateVariable(const char *sName,const char *sValue,int nFlags, const char *help)
{
	ConsoleVariablesMapItor itor;
	CXConsoleVariable *pCVar;
	itor=m_mapVariables.find(sName);
	
	if(itor!=m_mapVariables.end())
	{
		return itor->second;
	}

	pCVar=new CXConsoleVariable(this,m_pScriptSystem,sName,nFlags,CVAR_STRING, help);
	/*
	char sTemp[200];
	sprintf(sTemp,"%s=%s\n",sName,sValue);
	::OutputDebugString(sTemp);
	*/
	//the script ovveride the .ini file
	if(!pCVar->m_bLoadedFromScript){
//		char *res=GetVariable(sName, "Engine.Ini",sValue);
//	TRACE("%s=%s",sName,res);
		pCVar->Set(sValue);
	}

	m_mapVariables.insert(ConsoleVariablesMapItor::value_type(sName,pCVar));

	return pCVar;
}

//////////////////////////////////////////////////////////////////////////
ICVar *CXConsole::CreateVariable(const char *sName,float fValue,int nFlags, const char *help)
{
	ConsoleVariablesMapItor itor;
	CXConsoleVariable *pCVar;
	itor=m_mapVariables.find(sName);

	if(itor!=m_mapVariables.end())
	{
		return itor->second;
	}
	pCVar=new CXConsoleVariable(this,m_pScriptSystem,sName,nFlags,CVAR_FLOAT, help);
	//the script ovveride the .ini file
	if(!pCVar->m_bLoadedFromScript){
		pCVar->Set(fValue);
	}
	m_mapVariables.insert(ConsoleVariablesMapItor::value_type(sName,pCVar));
	return pCVar;
}

//////////////////////////////////////////////////////////////////////////
ICVar *CXConsole::CreateVariable(const char *sName,int iValue,int nFlags, const char *help)
{
	ConsoleVariablesMapItor itor;
	CXConsoleVariable *pCVar;
	itor=m_mapVariables.find(sName);

	if(itor!=m_mapVariables.end())
	{
		return itor->second;
	}
	pCVar=new CXConsoleVariable(this,m_pScriptSystem,sName,nFlags,CVAR_INT, help);
	//the script ovveride the .ini file
	if(!pCVar->m_bLoadedFromScript){
		pCVar->Set(iValue);
	}
	m_mapVariables.insert(ConsoleVariablesMapItor::value_type(sName,pCVar));
	return pCVar;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Remove a variable from the console
	@param sVarName console variable name
	@param bDelete if true the variable is deleted
	@see ICVar
*/
void CXConsole::UnregisterVariable(const char *sVarName,bool bDelete)
{
	ConsoleVariablesMapItor itor;
	itor=m_mapVariables.find(sVarName);

	if(itor==m_mapVariables.end())
		return;

	ICVar *pCVar=itor->second;

	m_mapVariables.erase(sVarName);

	if(bDelete)
	{
		pCVar->Release();
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Set the y coordinate where the console will stop to scroll when is dropped
	@param value y in screen coordinates
*/
void CXConsole::SetScrollMax(int value)
{
	m_nScrollMax=value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CXConsole::SetImage(ITexPic *pImage,bool bDeleteCurrent)
{
	if (bDeleteCurrent)	
	{
		m_pRenderer=m_pSystem->GetIRenderer();
		if (m_pRenderer)
			m_pRenderer->RemoveTexture(m_pImage);	
	}
		
	m_pImage=pImage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! show/hide the console
	@param specifies if the window must be (true=show,false=hide)
*/
void	CXConsole::ShowConsole(bool show)
{
	SetStatus(show);

	if(m_bConsoleActive) 
    m_sdScrollDir=sdDOWN;		
	else 
    m_sdScrollDir=sdUP;	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Crate a new console variable that store the value in a user defined memory block
	@param sName console variable name
	@param src pointer to the memory that will store the value 
	@param value default value
	@param type type of the value (can be CVAR_INT|CVAR_FLOAT)
	@see ICVar
*/
int CXConsole::Register(const char *sName, void  *src, float value, int flags, int type, const char *help)
{
	int nRes=0;
	float	fRes=0;
	ConsoleVariablesMapItor itor;
	CXConsoleVariable *pICVar;
	itor=m_mapVariables.find(sName);
	
	if(itor!=m_mapVariables.end())
	{
		if (flags & CVF_CHANGE_SOURCE)
			itor->second->SetSrc(src);
		// found such variable; just set its 
		return 0;
	}

	char *sValue=NULL;

	switch(type)
	{
		case CVAR_INT:
			
			pICVar=new CXConsoleVariable(this,m_pScriptSystem,sName,src,flags,CVAR_INT, help);
			if(!pICVar->m_bLoadedFromScript)
			{
//				nRes=(int)(GetVariable(sName, "Engine.Ini", value));
				pICVar->Set((int)nRes);
			}
			
			m_mapVariables.insert(ConsoleVariablesMapItor::value_type(sName,pICVar));
			return nRes;
			break;
		case CVAR_FLOAT:
			pICVar=new CXConsoleVariable(this,m_pScriptSystem,sName,src,flags,CVAR_FLOAT, help);
			if(!pICVar->m_bLoadedFromScript)
			{
//				fRes=GetVariable(sName, "Engine.Ini", value);
				pICVar->Set(fRes);
			}
			
			m_mapVariables.insert(ConsoleVariablesMapItor::value_type(sName,pICVar));
			return ((int)(fRes));
			break;
		default:
			CryError( "<CrySystem> (CXConsole::Register) Unknown console variable type" );
			break;
	}
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Crate a new console variable that store the value in a user defined floating point
	@param sName console variable name
	@param src pointer to the memory that will store the value 
	@param value default value
	@see ICVar
*/
float CXConsole::Register(const char *sName, float *src, float value, int flags, const char *help)
{
//	float nRes=0;
	ConsoleVariablesMapItor itor;
	CXConsoleVariable *pICVar;
	itor=m_mapVariables.find(sName);
	
	if(itor!=m_mapVariables.end())
	{
		if (flags & CVF_CHANGE_SOURCE)
			itor->second->SetSrc(src);
		return itor->second->GetFVal();
	}
  
	pICVar=new CXConsoleVariable(this,m_pScriptSystem,sName,src,flags,CVAR_FLOAT, help);
	if(!pICVar->m_bLoadedFromScript)
	{
//		nRes=GetVariable(sName, "Engine.Ini", value);
		pICVar->Set(value);
	}
	
	m_mapVariables.insert(ConsoleVariablesMapItor::value_type(sName,pICVar));
	return pICVar->GetFVal();//value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Crate a new console variable that store the value in a user defined integer
	@param sName console variable name
	@param src pointer to the memory that will store the value 
	@param value default value
	@see ICVar
*/
int CXConsole::Register(const char *sName, int   *src, float value, int flags, const char *help)
{
//	int nRes=0;
	ConsoleVariablesMapItor itor;
	CXConsoleVariable *pICVar;
	itor=m_mapVariables.find(sName);
	
	if(itor!=m_mapVariables.end())
	{
		if (flags & CVF_CHANGE_SOURCE)
			itor->second->SetSrc(src);
		return itor->second->GetIVal();
	}
	
	pICVar=new CXConsoleVariable(this,m_pScriptSystem,sName,src,flags,CVAR_INT, help);
	if(!pICVar->m_bLoadedFromScript)
	{
//		nRes=(int)(GetVariable(sName, "Engine.Ini", value));
		pICVar->Set(value);
	}
	
	m_mapVariables.insert(ConsoleVariablesMapItor::value_type(sName,pICVar));
	return (int)pICVar->GetIVal();//value;

}

////////////////////////////////////////////////////////
/*! Bind a console command to a key
	@param sCmd console command that must be executed
	@param sRes name of the key to invoke the command
	@param bExecute legacy parameter(will be removed soon)
*/
void CXConsole::CreateKeyBind(const char *sCmd,const char *sRes,bool bExecute)
{
	m_mapBinds.insert(ConsoleBindsMapItor::value_type(sRes,sCmd));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Dump all key binds to a callback-interface
	@param Callback callback-interface which needs to be called for each element
*/
void CXConsole::DumpKeyBinds( IKeyBindDumpSink *pCallback )
{
	for (ConsoleBindsMap::iterator it = m_mapBinds.begin(); it != m_mapBinds.end(); ++it)
	{
		pCallback->OnKeyBindFound( it->first.c_str(),it->second.c_str() );
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* CXConsole::FindKeyBind( const char *sCmd )
{
	ConsoleBindsMap::iterator it = m_mapBinds.find(sCmd);
	if (it != m_mapBinds.end())
		return it->second.c_str();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Dump all console-variables to a callback-interface
	@param Callback callback-interface which needs to be called for each element
*/
void CXConsole::DumpCVars(ICVarDumpSink *pCallback,unsigned int nFlagsFilter)
{
	ConsoleVariablesMapItor It=m_mapVariables.begin();
	while (It!=m_mapVariables.end())
	{
		if((nFlagsFilter==0) || ((nFlagsFilter!=0) && (It->second->GetFlags()&nFlagsFilter) ))
			pCallback->OnElementFound(It->second);
		++It;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Retrieve a console variable by name
	@param sName variable name
	@see ICVar
*/
ICVar* CXConsole::GetCVar( const char *sName, const bool bCaseSensitive )
{
	assert(this);
	assert(sName);

	if(bCaseSensitive)
	{
		// faster

		ConsoleVariablesMapItor itor;
		
		itor=m_mapVariables.find(sName);
		if(itor!=m_mapVariables.end()){
			return itor->second;
		}
	}
	else
	{
		// much slower but allowes names with wrong case (use only where performce doesn't matter)

		ConsoleVariablesMapItor it;

		for(it=m_mapVariables.begin(); it!=m_mapVariables.end(); ++it)
		{
			if(stricmp(it->first.c_str(),sName)==0)
				return it->second;
		}
	}

	return NULL;		// haven't found this name
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Refresh the internal values of a console variable
	@param sVarName variable name
	@see ICVar
*/
void CXConsole::RefreshVariable(string sVarName)
{
	ConsoleVariablesMapItor itor;
	itor=m_mapVariables.find(sVarName);
	if(itor!=m_mapVariables.end()){
		CXConsoleVariable *pCV=itor->second;
		pCV->Refresh();
		if(pCV->GetFlags()&VF_REQUIRE_NET_SYNC){
			m_pSystem->m_pGame->OnSetVar(pCV);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Legacy function
*/
void CXConsole::Help(const char *command)
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Read a value from a configuration file (.ini) and return the value
	@param szVarName variable name
	@param szFileName source configuration file
	@param def_val default value (if the variable is not found into the file)
*/
char *CXConsole::GetVariable( const char * szVarName, const char * szFileName, const char * def_val )
{
	assert( m_pSystem );
	//return m_pSystem->GetIniVar( szVarName, szFileName, def_val);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Read a value from a configuration file (.ini) and return the value
	@param szVarName variable name
	@param szFileName source configuration file
	@param def_val default value (if the variable is not found into the file)
*/
float CXConsole::GetVariable( const char * szVarName, const char * szFileName, float def_val )
{
	assert( m_pSystem );
	//return m_pSystem->GetIniVar( szVarName, szFileName, def_val);
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Retreive the status of the console (active/not active)
*/
bool CXConsole::GetStatus()
{
	return m_bConsoleActive;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Clear the console text
*/
void CXConsole::Clear()
{
	m_dqConsoleBuffer.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Update the console
*/
void CXConsole::Update()
{
	// Repeat GetIRenderer (For Editor).
	if (!m_pSystem)
		return;
	m_pRenderer = m_pSystem->GetIRenderer();

	// Process Key press repeat.
	if (m_bConsoleActive)
	{
		if (m_nRepeatKey > XKEY_NULL)
		{
			float fRepeatSpeed = 40.0;

			if (GetCVar("ui_RepeatSpeed"))
			{
				fRepeatSpeed = (float)GetCVar("ui_RepeatSpeed")->GetIVal();
			}

			float fTime = m_pSystem->GetITimer()->GetAsyncCurTime() * 1000.0f;
			float fNextTimer = (1000.0f / fRepeatSpeed); // repeat speed

			while (fTime - m_nRepeatTimer > fNextTimer)
			{
				ProcessInput(m_nRepeatKey, m_sLastKeyName.c_str());
				m_nRepeatTimer += fNextTimer;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CXConsole::OnInputEvent( const SInputEvent &event )
{
	// Process input event.
	ConsoleBindsMapItor itorBind;

	int nKeyCode = event.key;
	int nModifiers = event.moidifiers;
	const char *sKeyName = event.keyname;
 
	if (event.type == SInputEvent::KEY_RELEASE && m_bConsoleActive)
	{
		m_nRepeatKey = XKEY_NULL;

		if ((nKeyCode == XKEY_RETURN) || (nKeyCode == XKEY_NUMPADENTER))
		{
			ExecuteInputBuffer();
			m_nScrollLine=0; 

			return false;
		}
	}

	if (event.type != SInputEvent::KEY_PRESS)
		return false;

	if (nKeyCode <= XKEY_NULL)
		return false;

	m_nRepeatKey = nKeyCode;
	m_sLastKeyName = sKeyName;

	float fRepeatDelay = 200.0f;
	
	if (GetCVar("ui_RepeatDelay"))
	{
		fRepeatDelay = (float)GetCVar("ui_RepeatDelay")->GetIVal();
	}

	m_nRepeatTimer = m_pTimer->GetAsyncCurTime() * 1000.0f + fRepeatDelay;

	//execute Binds
	if(!m_bConsoleActive)
	{
		itorBind = m_mapBinds.find(sKeyName);
		if(itorBind != m_mapBinds.end())
		{
			ExecuteString(itorBind->second.c_str(),true);
			m_sInputBuffer = "";
			m_nCursorPos=0;
		}
	}
	else
	{
		if (nKeyCode != XKEY_TAB)
			ResetAutoCompletion();

		if (nKeyCode == XKEY_V && (nModifiers&XKEY_MOD_CONTROL) != 0) 
		{
			Paste();
			return false;
		}

		if (nKeyCode == XKEY_C && (nModifiers&XKEY_MOD_CONTROL) != 0) 
		{
			Copy();
			return false;
		}
	}

	switch(nKeyCode)
	{
	case XKEY_TILDE:
		ShowConsole(!GetStatus());
		m_sInputBuffer="";
		m_nCursorPos=0;
		return false;
		////////////////////////////////////////////////////////////
	case XKEY_ESCAPE:
		//switch process or page or other things
		m_sInputBuffer="";
		m_nCursorPos=0;
		if (m_pSystem)
		{
			if (!m_bConsoleActive)
			{
				m_pSystem->GetIGame()->SendMessage("Switch");
			}

			ISystemUserCallback *pCallback = ((CSystem*)m_pSystem)->GetUserCallback();
			if (pCallback)
				pCallback->OnProcessSwitch();
		}
		return false;
		////////////////////////////////////////////////////////////
	default:
		break;
	}

	ProcessInput( nKeyCode,m_pInput->GetKeyName(event.key, event.moidifiers, 1) );

	return false;
}

//////////////////////////////////////////////////////////////////////////
void CXConsole::ProcessInput( int nKeyCode,const char *sKeyName )
{
	if (!m_bConsoleActive)
		return;

	switch(nKeyCode)
	{
		////////////////////////////////////////////////////////////
	case XKEY_BACKSPACE:
		RemoveInputChar(true);
		break;
		////////////////////////////////////////////////////////////
	case XKEY_LEFT:
		if(m_nCursorPos)
			m_nCursorPos--;
		break;
		////////////////////////////////////////////////////////////
	case XKEY_RIGHT:
		if(m_nCursorPos<(int)(m_sInputBuffer.length()))
			m_nCursorPos++;
		break;
		////////////////////////////////////////////////////////////
	case XKEY_UP:
		{
			const char *szHistoryLine=GetHistoryElement(true);		// true=UP

			if(szHistoryLine)
			{
				m_sInputBuffer=szHistoryLine;
				m_nCursorPos=(int)m_sInputBuffer.size();
			}
		}
		break;
		////////////////////////////////////////////////////////////
	case XKEY_DOWN:
		{
			const char *szHistoryLine=GetHistoryElement(false);		// false=DOWN

			if(szHistoryLine)
			{
				m_sInputBuffer=szHistoryLine;
				m_nCursorPos=(int)m_sInputBuffer.size();
			}
		}
		break;
		////////////////////////////////////////////////////////////
	case XKEY_TAB:
		if (
			(!m_pSystem->GetIInput()->KeyDown(XKEY_LALT)) &&
			(!m_pSystem->GetIInput()->KeyDown(XKEY_RALT)) &&
			(!m_pSystem->GetIInput()->KeyDown(XKEY_ALT)))
		{
			m_sInputBuffer = ProcessCompletion(m_sInputBuffer.c_str());
			m_nCursorPos = m_sInputBuffer.size();
		}
		break;
		////////////////////////////////////////////////////////////
	case XKEY_PAGE_UP:
		if(m_nScrollLine<(int)(m_dqConsoleBuffer.size()-2))
			m_nScrollLine++;
		break;
		////////////////////////////////////////////////////////////
	case XKEY_PAGE_DOWN:
		if(m_nScrollLine)
			m_nScrollLine--;
		break;
		////////////////////////////////////////////////////////////
	case XKEY_HOME:
		m_nCursorPos=0;
		break;
		////////////////////////////////////////////////////////////
	case XKEY_END:
		m_nCursorPos=(int)m_sInputBuffer.length();
		break;
		////////////////////////////////////////////////////////////
	case XKEY_DELETE:
		RemoveInputChar(false);
		break;
		////////////////////////////////////////////////////////////   
	default:
		if (sKeyName)
		{
			if(nKeyCode==XKEY_SPACE)
			{
				AddInputChar(' ');
			}
			else{
				if(strlen(sKeyName)!=1)
					return;
				AddInputChar( sKeyName[0] );
			}
		}
		break;
		////////////////////////////////////////////////////////////
	}
	m_nLastKey = nKeyCode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* CXConsole::GetHistoryElement( const bool bUpOrDown )
{
	static string sReturnString;			// is that save enough?

	if(bUpOrDown)
	{
		if(!m_dqHistory.empty())
		{
			if(m_nHistoryPos<(int)(m_dqHistory.size()-1))
			{
				m_nHistoryPos++;
				sReturnString=m_dqHistory[m_nHistoryPos];
				return sReturnString.c_str();
			}
		}
	}
	else
	{		
		if(m_nHistoryPos>0)
		{
			m_nHistoryPos--;
			sReturnString=m_dqHistory[m_nHistoryPos];
			return sReturnString.c_str();
		}
	}

	return 0;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Draw the console
*/
void CXConsole::Draw()
{
	if(!m_pSystem || !m_nScrollMax)
		return;

	if(!m_pRenderer)
	{
		// For Editor.
		m_pRenderer = m_pSystem->GetIRenderer();
	}

	if(!m_pRenderer)
		return;

	if (!m_pFont)
	{
		// For Editor.
		ICryFont *pICryFont=m_pSystem->GetICryFont();

		if(pICryFont)
			m_pFont= m_pSystem->GetICryFont()->GetFont("Default");
	}

	float fCurrTime=m_pTimer->GetCurrTime();

	ScrollConsole();

	if (m_nScrollPos<=0)
	{
//#ifdef _DEBUG
		DrawBuffer(70, "buttonfocus");
//#endif
		return;
	} 

	const double fBlinkTime=CURSOR_TIME*2.0;
	m_bDrawCursor = fmod((double)fCurrTime,fBlinkTime)<0.5;
	 
	if (m_pImage) 
  {
    float time = fCurrTime*0.05f;

//#ifndef _XBOX
		if (!m_nProgressRange)
		{
			if (m_bStaticBackground)
			{
        m_pRenderer->SetState(GS_NODEPTHTEST);
				m_pRenderer->Draw2dImage(0,0,800,600,m_pImage->GetTextureID(), 0.0f, 1.0f, 1.0f, 0.0f);
			}else
			{
        m_pRenderer->SetState(GS_NODEPTHTEST);
				m_pRenderer->TransformTextureMatrix(time,0,0,1);
				m_pRenderer->Draw2dImage(0,(float)(m_nScrollPos-m_nScrollMax),800,(float)(m_nScrollMax),m_pImage->GetTextureID(),4.0f,2.0f);	
				m_pRenderer->ResetTextureMatrix();

        m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
				m_pRenderer->SetMaterialColor(1,1,1,0.5f);

				m_pRenderer->TransformTextureMatrix(time,0,0,-1);
				m_pRenderer->Draw2dImage(0,(float)(m_nScrollPos-m_nScrollMax),800,(float)(m_nScrollMax),m_pImage->GetTextureID(),4.0f,2.0f);	
				m_pRenderer->ResetTextureMatrix();
			}
		}
/*#else
		if(m_bStaticBackground)
			m_pRenderer->Draw2dImage(0,(float)(m_nScrollPos-m_nScrollMax),800,(float)m_nScrollMax,m_pImage->GetTextureID(),4.0f,2.0f);	
		else
			m_pRenderer->Draw2dImage(0,0,800,600,m_pImage->GetTextureID());
#endif		*/

  }

  // draw progress bar
  if(m_nProgressRange)
  {
		float fRcp1024 = 1.0f / 1024.0f;
		float fProgress = min(1.0f, m_nProgress / (float)m_nProgressRange);

		float fTexProgress0 = 51.0f * fRcp1024;
		float fTexProgress1 = (51.0f + fProgress * (1024.0f - 51.0f*2.0f)) * fRcp1024;


    m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
		m_pRenderer->Draw2dImage(0.0, 0.0, 800.0f, 600.0f, m_nLoadingBackTexID, 0.0f, 1.0f, 1.0f, 0.0f);
	  m_pRenderer->Draw2dImage(40, 480, fProgress * (800.0f - 40.0f*2.0f), 13, m_nLoadingBarTexID, fTexProgress0, 1.0f, fTexProgress1, 0.0f);
  }

  int nPrevMode = m_pRenderer->SetPolygonMode(0);
  //if (!m_bStaticBackground)
	DrawBuffer(m_nScrollPos, "console");
  m_pRenderer->SetPolygonMode(nPrevMode);
}

void CXConsole::DrawBuffer(int nScrollPos, const char *szEffect)
{
	if(!m_bConsoleActive && (con_display_last_messages->GetIVal()==0))
		return;
	
	if (m_pFont && m_pRenderer)
  {
	  m_pFont->UseRealPixels(false);
	  m_pFont->SetEffect(szEffect);
	  m_pFont->SetSameSize(true);
	  m_pFont->SetCharWidthScale(1.5f / 3.0f);
	  m_pFont->SetSize(vector2f(14, 14));
	  m_pFont->SetColor(color4f(1,1,1,1));

		m_pFont->UseRealPixels(true);
		float csize = 0.8f * m_pFont->GetCharHeight();
		m_pFont->UseRealPixels(false);

		float ypos = nScrollPos-csize-3.0f;

		float fCharWidth=(m_pFont->GetCharWidth() * m_pFont->GetCharWidthScale());

	/*if (GetFont() && m_pRenderer)
  {
    float fXScale = 0.5f;
    float fYScale = 1.0f;
    CXFont *pFont= GetFont();
    m_pRenderer->SetFontScale(fXScale, fYScale);
    int csize = (int)(pFont->m_charsize*fYScale*(float)(m_pRenderer->GetHeight())/600.0f);
    int nCharWidth = (int)(pFont->m_charsize*fXScale*(float)(m_pRenderer->GetWidth())/800.0f);
    m_pRenderer->SetCurFontColor(Col_White);*/
		
		//int ypos=nScrollPos-csize-3;	

		//Draw the input line
		if(m_bConsoleActive && !m_nProgressRange)
    {
      /*m_pRenderer->DrawString(LINE_BORDER-nCharWidth, ypos, false, ">");
      m_pRenderer->DrawString(LINE_BORDER, ypos, false, m_sInputBuffer.c_str());
		  if(m_bDrawCursor)
			  m_pRenderer->DrawString(LINE_BORDER+nCharWidth*m_nCursorPos, ypos, false, "_");*/

      m_pFont->DrawString((float)(LINE_BORDER-fCharWidth), (float)ypos, ">");
			m_pFont->DrawString((float)LINE_BORDER, (float)ypos, m_sInputBuffer.c_str(),false);
		  
			if(m_bDrawCursor)
			{
				string szCursorLeft(m_sInputBuffer.c_str(), m_sInputBuffer.c_str() + m_nCursorPos);
				int n = m_pFont->GetTextLength(szCursorLeft.c_str(), false);

			  m_pFont->DrawString((float)(LINE_BORDER+(fCharWidth * n)), (float)ypos, "_");
			}
		}
		
		ypos-=csize;
		
		ConsoleBufferRItor ritor;
		ritor=m_dqConsoleBuffer.rbegin();
		int nScroll=0;
		while(ritor!=m_dqConsoleBuffer.rend() && ypos>=0)  
		{
			if(nScroll>=m_nScrollLine)
			{
				const char *buf=ritor->c_str();// GetBuf(k);
				
				if(*buf>0 && *buf<32) buf++;		// to jump over verbosity level character

				if (ypos+csize>0) 
  			  m_pFont->DrawString((float)LINE_BORDER, (float)ypos, buf,false);
					//m_pRenderer->DrawString(LINE_BORDER, ypos, false, buf);
				//CSystem::GetRenderer()->WriteXY(m_font,0,ypos,0.5f,1,1,1,1,1,buf);			
				ypos-=csize;
			}
			nScroll++;
			
			++ritor;
		} //k		
  }
}


bool CXConsole::GetLineNo( const DWORD indwLineNo, char *outszBuffer, const DWORD indwBufferSize ) const
{
	assert(outszBuffer);
	assert(indwBufferSize>0);

	outszBuffer[0]=0;

	ConsoleBuffer::const_reverse_iterator ritor = m_dqConsoleBuffer.rbegin();
  
	ritor+=indwLineNo;

	if(indwLineNo>=m_dqConsoleBuffer.size())
		return false;

	const char *buf=ritor->c_str();// GetBuf(k);
	
	if(*buf>0 && *buf<32) buf++;		// to jump over verbosity level character

	strncpy(outszBuffer,buf,indwBufferSize-1);
	outszBuffer[indwBufferSize-1]=0;
	
	return true;
}

int CXConsole::GetLineCount() const
{
	return m_dqConsoleBuffer.size();
}

void CXConsole::ScrollConsole()
{
	if(!m_pRenderer)
		return;
  
  int nCurrHeight=m_pRenderer->GetHeight();
  
  switch (m_sdScrollDir)
  {
/////////////////////////////////
		case sdDOWN: // The console is scrolling down
      
      // Vlads note: console should go down immediately, otherwise it can look very bad on startup
      //m_nScrollPos+=nCurrHeight/2;			
      m_nScrollPos = m_nScrollMax;
      
      if (m_nScrollPos>m_nScrollMax)
      {
        m_nScrollPos = m_nScrollMax;
        m_sdScrollDir = sdNONE;
      }
      break;
/////////////////////////////////      
    case sdUP: // The console is scrolling up
      
      m_nScrollPos-=nCurrHeight;//2;			
      
      if (m_nScrollPos<0)
      {
        m_nScrollPos = 0;
        m_sdScrollDir = sdNONE;				
      }
      break;
/////////////////////////////////      
    case sdNONE: 
			break;
/////////////////////////////////
  }
}


void CXConsole::AddCommand(const char *sName, const char *sScriptFunc, const DWORD indwFlags, const char *help)
{
#if defined(_DEBUG) && !defined(LINUX)
	if(!*help) 
	{
		char buf[100];
		sprintf(buf, "MISSING HELP FOR COMMAND: %s\n", sName);
		OutputDebugString(buf);
	};
#endif

	XConsoleCommand cmd;
	cmd.m_sName=sName;
	cmd.m_sCommand=sScriptFunc;
	cmd.m_psHelp=help;
	cmd.m_dwFlags=indwFlags;
	m_mapCommands.insert(ConsoleCommandsMapItor::value_type(sName,cmd));
}

bool hasprefix(const char *s, const char *prefix)
{
	while(*prefix) if(*prefix++!=*s++) return false;
	return true;
};

void CXConsole::DumpCommandsVars(char *prefix)
{
	FILE *f = fopen("consolecommandsandvars.txt", "w");
	if(!f) return;
	
	fprintf(f," CHEAT: stays in the default value if cheats are not disabled\n");
	fprintf(f," REQUIRE_NET_SYNC: cannot be changed on client and when connecting it´s sent to the client\n");
	fprintf(f," SAVEGAME: stored when saving a savegame\n");
	fprintf(f," READONLY: can not be changed by the user\n");
	fprintf(f,"-------------------------\n");
	fprintf(f,"\n");

	for(ConsoleCommandsMapItor itrCmd = m_mapCommands.begin(); itrCmd!=m_mapCommands.end(); ++itrCmd)
	{
		XConsoleCommand &cmd = itrCmd->second;

		if(hasprefix(cmd.m_sName.c_str(), prefix))
		{
			string sFlags;

			if(cmd.m_dwFlags&VF_CHEAT)						sFlags+=" CHEAT";
			if(cmd.m_dwFlags&VF_REQUIRE_NET_SYNC)	sFlags+=" REQUIRE_NET_SYNC";
			if(cmd.m_dwFlags&VF_SAVEGAME)					sFlags+=" SAVEGAME";
			if(cmd.m_dwFlags&VF_READONLY)					sFlags+=" READONLY";

			fprintf(f, "command: %s %s\nscript: %s\nhelp: %s\n\n", cmd.m_sName.c_str(), sFlags.c_str(), cmd.m_sCommand.c_str(), cmd.m_psHelp);
		}
	};

	for(ConsoleVariablesMapItor itrVar = m_mapVariables.begin(); itrVar!=m_mapVariables.end(); ++itrVar)
	{
		ICVar *var = itrVar->second;
		char *types[] = { "?", "int", "float", "string", "?" };

		if(hasprefix(var->GetName(), prefix)) 
		{
			string sFlags;

			if(var->GetFlags()&VF_CHEAT)						sFlags+=" CHEAT";
			if(var->GetFlags()&VF_REQUIRE_NET_SYNC)	sFlags+=" REQUIRE_NET_SYNC";
			if(var->GetFlags()&VF_SAVEGAME)					sFlags+=" SAVEGAME";
			if(var->GetFlags()&VF_READONLY)					sFlags+=" READONLY";
	
			fprintf(f, "variable: %s %s\ntype: %s\ncurrent: %s\nhelp: %s\n\n", var->GetName(), sFlags.c_str(), types[var->GetType()], var->GetString(), var->GetHelp());
		}
	};
	
	ConsoleLog( "succesfully wrote list of commands & variables to $3consolecommandsandvars.txt");	
	
	fclose(f);
};

void CXConsole::DisplayHelp( const char *help, const char *name)
{
	if(!*help)
	{
		ConsoleLog("No help available for $3%s", name);
	}
	else
	{
		char *start, *pos;
		for(pos = (char *)help, start = (char *)help; pos = strstr(pos, "\n"); start = ++pos)
		{
			string s = start;
			s.resize(pos-start);
			ConsoleLog("    $3%s", s.c_str());
		};
		ConsoleLog("    $3%s", start);
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Execute a string in the console
	@param command console command
*/
void CXConsole::ExecuteString( const char *command,bool bNeedSlash,bool bIgnoreDevMode )
{
	ILog *pLog=m_pSystem->GetILog();				assert(pLog);
	string sTemp=command;
	string sCommand;
	ConsoleCommandsMapItor itrCmd;
	ConsoleVariablesMapItor itrVar;

	if(sTemp.empty())
		return;
		
///////////////////////////
//Execute as string
	if(command[0]=='#')
	{
		if ( !bIgnoreDevMode ) AddLine(sTemp);//don't put line in console if we are overriding devmode: TRay 3/4/04
		IGame *pGame=m_pSystem->GetIGame();		assert(pGame);

		if (m_pSystem->IsDevMode() || bIgnoreDevMode)
		{
			sTemp=&command[1];
			m_pScriptSystem->ExecuteBuffer(sTemp.c_str(),sTemp.length());
			m_bDrawCursor = 0;
		}
		else
		{
			// Warning.
			// No Cheat warnings. ConsoleWarning("Console execution is cheat protected");
		}
		return;
	}

	string::size_type nPos;
//is a command or console var 
	if(command[0]=='\\' || !bNeedSlash)
	{
		if (GetStatus())
			AddLine(sTemp);
		sTemp=&command[bNeedSlash?1:0];

		if (strnicmp(sTemp.c_str(),"pb_",3 ) == 0)
		{
			ICVar *pVar = GetCVar("sys_punkbuster_loaded");
			if (!pVar)
			{
				ConsoleLog( "PunkBuster is not currently loaded." );
			}
			return;
		}

		if((nPos=sTemp.find_first_of(' '))==string::npos)
		{
			sCommand=sTemp;
		}
		else
		{
			sCommand=sTemp.substr(0,nPos);
		}
		//////////////////////////////////////////
		//Check if is a command
		itrCmd=m_mapCommands.find(sCommand);
		if(itrCmd!=m_mapCommands.end())
		{
			sTemp=command;
			ExecuteCommand((itrCmd->second), sTemp,bIgnoreDevMode);
			return;
		}
		//////////////////////////////////////////
		//Check  if is a variable
		itrVar=m_mapVariables.find(sCommand);
		if(itrVar!=m_mapVariables.end())
		{
			string sScriptCommand;
			sCommand=itrVar->first;
			if(nPos==string::npos)
			{

			}
			else
			{
				sTemp=sTemp.substr(nPos+1);
				
				// "" clears the cvar's contents
				if (sTemp == "\"\"")
				{
					sScriptCommand=sCommand+"=\"\"";
					m_pScriptSystem->ExecuteBuffer(sScriptCommand.c_str(),sScriptCommand.length());
				}
				else
				{
					sScriptCommand=sCommand+"=\""+sTemp+"\"";

					//remove useles spaces
					while((nPos=sTemp.find(' '))!=string::npos)
					{
						sTemp.erase(nPos,1);
					}

					ICVar *v = itrVar->second;

					if(sTemp=="?" && ((v->GetFlags() & VF_NOHELP) == 0))
					{
						ICVar *v = itrVar->second;
						DisplayHelp( v->GetHelp(), sCommand.c_str());
						return;
					}; 

					if(sTemp.length())
						m_pScriptSystem->ExecuteBuffer(sScriptCommand.c_str(),sScriptCommand.length());
				}
			}
			ICVar *pVar=itrVar->second;

			// the following line called AddLine() indirectly
			ConsoleLog("\001%s",(string(pVar->GetName())+"="+string(pVar->GetString())).c_str() );
			return;
		}
	}
	else if(bNeedSlash)
	{
		string tmp=command;
		size_t pp=0;

		AddLine(sTemp);
		while((pp=tmp.find("\""))!=string::npos){
			tmp.replace(pp, 1,"'");
		}
		//<<FIXME>> think a better way to the broadcast string thing
		tmp=string("if(Client)then Client:Say(\"")+tmp+string("\"); end");
		m_pScriptSystem->ExecuteBuffer(tmp.c_str(),tmp.size());
		return;
	}
 
	ConsoleWarning("Unknown command: %s", command);	
}

//////////////////////////////////////////////////////////////////////////
void CXConsole::ExecuteCommand(XConsoleCommand &cmd,string &str,bool bIgnoreDevMode)
{
    std::vector<string> args;

		size_t t=1;

		for(;;) 
		{
			t = str.find_first_of("\\",t);
			if (t==string::npos)break;
			str.replace(t, 1, "\\\\", 2);
			t+=2;
		} 

		for(t=1;;) 
		{
			t = str.find_first_of("\"",t);
			if (t==string::npos)break;
			str.replace(t, 1, "\\\"", 2);
			t+=2;
		} 

    for(char *p = (char *)str.c_str(); *p;)
    {
        while(*p==' ') p++;
        char *s = p;
        while(*p && *p!=' ') p++;
        if(p!=s) args.push_back(string(s, p));
    }
    
    if(args.size()>=2 && args[1]=="?" && ((cmd.m_dwFlags & VF_NOHELP) == 0))
    {
			DisplayHelp( cmd.m_psHelp, cmd.m_sName.c_str() );
			return;
    }

		if((cmd.m_dwFlags&VF_CHEAT)!=0 && !m_pSystem->IsDevMode() && !bIgnoreDevMode)
		{
			// No Log. ConsoleWarning("Command %s is cheat protected.", cmd.m_sName.c_str());
			return;
		}
    
    string buf = cmd.m_sCommand.c_str();

    size_t pp = buf.find("%%");
    if(pp!=string::npos)
    {
        string list = "";
        for(unsigned int i = 1; i<args.size(); i++)
        {
            list += "\""+args[i]+"\"";
            if(i<args.size()-1) list += ",";
        };
		buf.replace(pp, 2, list);          
	}else if((pp = buf.find("%line"))!=string::npos){
		string tmp="\""+str.substr(str.find(" ")+1)+"\"";
		if(args.size()>1)
		{
			buf.replace(pp, 5, tmp);
		}
		else
		{
			buf.replace(pp, 5, "");
		}
	}
    else
    {
        for(unsigned int i = 1; i<=args.size(); i++)
        {
            char pat[10];
            sprintf(pat, "%%%d", i); 
            size_t pos = buf.find(pat);
            if(pos==string::npos)
            {
                if(i!=args.size())
                {
                    ConsoleWarning("Too many arguments for: %s", cmd.m_sName.c_str());
                    return;
                };
            }
            else
            {
                if(i==args.size())
                {
                    ConsoleWarning("Not enough arguments for: %s", cmd.m_sName.c_str());
                    return;
                };
                string arg = "\""+args[i]+"\"";
		        buf.replace(pos, strlen(pat), arg);          
            };
        };
    };

	m_pScriptSystem->ExecuteBuffer(buf.c_str(), buf.length());
	m_bDrawCursor = 0;
    
/*

	string::size_type nPos;
	string sTemp;
	string sParam;
	string sParamList;

	sTemp=str;
	string::npos;
	string sCommandBuf=cmd.sCommand;
	int nParam=0;

	nParam=sCommandBuf.find("%%");
	
	nPos = 0;
	while(sTemp[nPos]==' ') nPos++;
	sTemp=sTemp.substr(nPos);
	nPos=sTemp.find(' ');

	while((nPos!=string::npos))
	{
		sParam=sTemp.substr(0,nPos);
		sParam="\""+sParam+"\"";
		while(sTemp[nPos]==' ') nPos++;
		sTemp=sTemp.substr(nPos);

		sParamList+=sParam+",";
		nPos=sTemp.find(' ');
	}

	if(!sTemp.empty())
	{
		sParam="\""+sTemp+"\"";
		sParamList+=sParam;
	}

	while((nParam=sCommandBuf.find("%%"))!=string::npos)
	{
		sCommandBuf.replace(nParam,2,sParamList.c_str(),sParamList.length());
	}
	
	//TRACE("sCommandBuf %d",sCommandBuf.c_str());

	m_pScriptSystem->ExecuteBuffer(sCommandBuf.c_str(),sCommandBuf.length());
	*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Print a message into the log and abort the execution of the application
	@param message error string to print in the log
*/
void CXConsole::Exit(const char * szExitComments, ...)
{
  char sResultMessageText[4096]="";
  if(szExitComments) 
  { // make result string
    va_list		arglist;    
    va_start(arglist, szExitComments);
    vsprintf(sResultMessageText, szExitComments, arglist);
    va_end(arglist);
  }
  else
    strcpy(sResultMessageText, "No comments from application");

	CryError( sResultMessageText );
}

void CXConsole::ResetAutoCompletion()
{
	m_nTabCount = 0;
	m_sPrevTab = "";
}

char *CXConsole::ProcessCompletion(const char *szInputBuffer)
{
	static string szInput;
	szInput = szInputBuffer;

	int offset = (szInputBuffer[0] == '\\' ? 1 : 0);

	if ((m_sPrevTab.size() > strlen(szInputBuffer + offset)) || strnicmp(m_sPrevTab.c_str(), (szInputBuffer + offset), m_sPrevTab.size()))
	{
		m_nTabCount = 0;
		m_sPrevTab = "";
	}

	if(szInput == "")
	{
		szInput = "\\";
		return (char *)szInput.c_str();
	}


	int nMatch = 0;
	ConsoleCommandsMapItor itrCmds;
	ConsoleVariablesMapItor itrVars;
	bool showlist = !m_nTabCount && m_sPrevTab=="";

	if (m_nTabCount==0)
	{
		if(szInput.size()>0)
			if(szInput[0]=='\\')
				m_sPrevTab=&szInput.c_str()[1];
			else
			{
				m_sPrevTab=szInput;
				szInput="\\"+szInput;
			}

		else
			m_sPrevTab="";
	}
	//try to search in command list

	std::vector<char *> matches;

	itrCmds=m_mapCommands.begin();
	while(itrCmds!=m_mapCommands.end())
	{
		XConsoleCommand &cmd = itrCmds->second;
		if((cmd.m_dwFlags&(VF_CHEAT|VF_READONLY))==0 || m_pSystem->IsDevMode())
		{
			if(strnicmp(m_sPrevTab.c_str(),itrCmds->first.c_str(),m_sPrevTab.length())==0)
			{
				matches.push_back((char *const)itrCmds->first.c_str());
			}
		}
		++itrCmds;
	}

	itrVars=m_mapVariables.begin();
	while(itrVars!=m_mapVariables.end()) 
	{
		CXConsoleVariable *pVar = itrVars->second;
		if((pVar->GetFlags()&(VF_CHEAT|VF_READONLY))==0 || m_pSystem->IsDevMode())
		{
			//if(itrVars->first.compare(0,m_sPrevTab.length(),m_sPrevTab)==0)
			if(strnicmp(m_sPrevTab.c_str(),itrVars->first.c_str(),m_sPrevTab.length())==0)
			{
				matches.push_back((char *const)itrVars->first.c_str());
			}
		}
		++itrVars;
	}

	if (showlist && matches.size()>1) 
	{
		ConsoleInputLog( " " );

		for(std::vector<char *>::iterator i = matches.begin(); i!=matches.end(); ++i)
		{
			// List matching variables
			const char *sVar = *i;
			const char *sValue = "";
			ICVar *pVar = GetCVar( sVar );
			if (pVar)
			{
				sValue = pVar->GetString();

				char szFalgsString[512]="";
				{ // make string containing enabled flags names
					static struct {	long nFlag;	const char * szName; } FlagNames[] =
					{
						VF_CHEAT,"CHEAT",
						VF_REQUIRE_NET_SYNC,"REQUIRE_NET_SYNC",
						VF_REQUIRE_LEVEL_RELOAD,"REQUIRE_LEVEL_RELOAD",
						VF_REQUIRE_APP_RESTART,"REQUIRE_APP_RESTART",
						VF_DUMPTODISK,"DUMPTODISK",
						0,0
					};

					for(int i=0; FlagNames[i].nFlag; i++)
						if(pVar->GetFlags()&FlagNames[i].nFlag)
						{
							if(szFalgsString[0])
								strncat(szFalgsString,", ", sizeof(szFalgsString));
							strncat(szFalgsString,FlagNames[i].szName, sizeof(szFalgsString));
						}
				}

				ConsoleInputLog( "    $3%s = $6%s $5[%s]", sVar,sValue,szFalgsString );
			}
			else
			{
				ConsoleInputLog( "    $3%s", sVar,sValue );
			}
		}
	}

	for(std::vector<char *>::iterator i = matches.begin(); i!=matches.end(); ++i)
	{
		if(m_nTabCount<=nMatch)
		{
			szInput="\\";
			szInput+=*i;
			szInput+=" ";
			m_nTabCount=nMatch+1;
			return (char *)szInput.c_str();
		}
		nMatch++;
	}; 

	if (m_nTabCount>0)
	{
		m_nTabCount=0;
		szInput=string("\\")+m_sPrevTab;
		szInput = ProcessCompletion(szInput.c_str());
	}

	return (char *)szInput.c_str();
}

bool CXConsole::IsOpened()
{
	return m_nScrollPos == m_nScrollMax;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Print a string in the console and go to the new line
	@param s the string to print
*/
void CXConsole::PrintLine(const char *s)
{
	AddLine(s);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Append a string in the last console line
	@param s the string to print
*/
void CXConsole::PrintLinePlus(const char *s)
{
	AddLinePlus(s);
}

void CXConsole::AddLine(string str)
{
	string::size_type nPos;
	while ((nPos=str.find('\n'))!=string::npos)
	{
		str.replace(nPos,1,1,' ');
	}

	while ((nPos=str.find('\r'))!=string::npos)
	{
		str.replace(nPos,1,1,' ');
	}

	//m_pSystem->GetIGame()->GetString

	m_dqConsoleBuffer.push_back(str);

	int nBufferSize=200;
	if(con_line_buffer_size)
	{
		nBufferSize=con_line_buffer_size->GetIVal();
	}
	while(((int)(m_dqConsoleBuffer.size()))>nBufferSize)
	{
		m_dqConsoleBuffer.pop_front();
	}

	// tell everyone who is interested (e.g. dedicated server printout)
	{
		std::vector<IOutputPrintSink *>::iterator it;

		for(it=m_OutputSinks.begin();it!=m_OutputSinks.end();++it)
			(*it)->Print(str.c_str());
	}
}

void CXConsole::ResetProgressBar(int nProgressBarRange)
{
	m_nProgressRange = nProgressBarRange;
	m_nProgress = 0;

	if (nProgressBarRange < 0)
		nProgressBarRange = 0;

	if (!m_nProgressRange)
	{
		if (m_nLoadingBackTexID)
		{
			if (m_pRenderer)
				m_pRenderer->RemoveTexture(m_nLoadingBackTexID);
			m_nLoadingBackTexID = -1;
		}
	}

	ICVar *log_Verbosity = GetCVar("log_Verbosity");

	if ((log_Verbosity) && (!log_Verbosity->GetIVal()))
	{
		Clear();
	}
}

void CXConsole::TickProgressBar()
{	
	if (m_nProgressRange != 0 && m_nProgressRange > m_nProgress)
	{
		m_nProgress++;
		m_pSystem->UpdateLoadingScreen();
	}

	if(m_pSystem->GetIGame())
		m_pSystem->GetIGame()->UpdateDuringLoading();		// network update
}

//////////////////////////////////////////////////////////////////////////
void CXConsole::SetLoadingImage(const char *szFilename )
{
	ITexPic *pTex = 0;		

	pTex = m_pSystem->GetIRenderer()->EF_LoadTexture( szFilename, FT_NORESIZE | FT_NOMIPS, 0, eTT_Base);

	if (!pTex || (pTex && (pTex->GetFlags() & FT_NOTFOUND)))
	{
		pTex = m_pSystem->GetIRenderer()->EF_LoadTexture("console/loadscreen_default", FT_NORESIZE | FT_NOMIPS, 0, eTT_Base);
	}

	if (pTex)
	{
		m_nLoadingBackTexID = pTex->GetTextureID();
	}
	else
	{
		m_nLoadingBackTexID = -1;
	}
}

void CXConsole::AddOutputPrintSink( IOutputPrintSink *inpSink )
{
	assert(inpSink);

	m_OutputSinks.push_back(inpSink);
}

void CXConsole::RemoveOutputPrintSink( IOutputPrintSink *inpSink )
{
	assert(inpSink);

	int nCount=m_OutputSinks.size();

	for(int i=0;i<nCount;i++)
	{
		if(m_OutputSinks[i]==inpSink)
		{
			if(nCount<=1)	m_OutputSinks.clear();
			else
			{
				m_OutputSinks[i]=m_OutputSinks.back();
				m_OutputSinks.pop_back();
			}
			return;
		}
	}

	assert(0);
}


void CXConsole::AddLinePlus(string str)
{
	if(!m_dqConsoleBuffer.size())
   return;

	string::size_type nPos;
	while ((nPos=str.find('\n'))!=string::npos)
		str.replace(nPos,1,1,' ');

	while ((nPos=str.find('\r'))!=string::npos)
		str.replace(nPos,1,1,' ');

	string tmpStr = m_dqConsoleBuffer.back();// += str;

	m_dqConsoleBuffer.pop_back();

  if(tmpStr.size()<256)
    m_dqConsoleBuffer.push_back(tmpStr + str);
  else 
    m_dqConsoleBuffer.push_back(tmpStr);

	// tell everyone who is interested (e.g. dedicated server printout)
	{
		std::vector<IOutputPrintSink *>::iterator it;

		for(it=m_OutputSinks.begin();it!=m_OutputSinks.end();++it)
			(*it)->Print((tmpStr + str).c_str());
	}
}

void CXConsole::AddInputChar(const char c)
{
	if(m_nCursorPos<(int)(m_sInputBuffer.length()))
		m_sInputBuffer.insert(m_nCursorPos,1,c);
	else
		m_sInputBuffer=m_sInputBuffer+c;
	m_nCursorPos++;
}

void CXConsole::ExecuteInputBuffer()
{
	string sTemp=m_sInputBuffer;
	if(m_sInputBuffer.empty())
		return;
	m_sInputBuffer="";
	AddCommandToHistory(sTemp.c_str());
	ExecuteString(sTemp.c_str(),true);
	
	m_nCursorPos=0;
}

void CXConsole::RemoveInputChar(bool bBackSpace)
{
	if(m_sInputBuffer.empty())
		return;

	if(bBackSpace)
	{
		if(m_nCursorPos>0){
			m_sInputBuffer.erase(m_nCursorPos-1,1);
			m_nCursorPos--;
		}
	}
	else
	{
		if(m_nCursorPos<(int)(m_sInputBuffer.length()))
			m_sInputBuffer.erase(m_nCursorPos,1);
	}
}


void CXConsole::AddCommandToHistory( const char *szCommand )
{
	assert(szCommand);

	m_nHistoryPos=-1;

	if(!m_dqHistory.empty())
	{
		// add only if the command is != than the last
		if(m_dqHistory.front()!=szCommand)
			m_dqHistory.push_front(szCommand);
	}
	else
		m_dqHistory.push_front(szCommand);

	while(m_dqHistory.size()>MAX_HISTORY_ENTRIES)
		m_dqHistory.pop_back();
}


void CXConsole::Copy()
{
#if !defined(_XBOX) && !defined(LINUX)
	if (m_sInputBuffer.empty())
		return;

	if (!OpenClipboard(NULL))
		return;

	size_t cbLength = m_sInputBuffer.length();

	HGLOBAL hGlobal;
	LPVOID  pGlobal;

	hGlobal = GlobalAlloc(GHND, cbLength + 1);
	pGlobal = GlobalLock (hGlobal);

	strcpy((char *)pGlobal, m_sInputBuffer.c_str());

	GlobalUnlock(hGlobal);

	EmptyClipboard();
	SetClipboardData(CF_TEXT, hGlobal);
	CloseClipboard();

	return;
#endif
}

void CXConsole::Paste()
{
#if !defined(_XBOX) && !defined(LINUX)
	//TRACE("Paste\n");
	
	static char sTemp[256];
	HGLOBAL hGlobal;
  void*  pGlobal;
	
	
	if (!IsClipboardFormatAvailable(CF_TEXT)) 
		return; 
	if (!OpenClipboard(NULL))
		return;
	
	hGlobal = GetClipboardData(CF_TEXT);
	if(!hGlobal)
		return;

	pGlobal = GlobalLock(hGlobal);
	size_t cbLength = min(sizeof(sTemp)-1, strlen((char *)pGlobal));
	
	strncpy(sTemp, (char *)pGlobal, cbLength);
	sTemp[cbLength] = '\0';
	GlobalUnlock(hGlobal);
	CloseClipboard();
	
	m_sInputBuffer.insert(m_nCursorPos,sTemp,strlen(sTemp));
	m_nCursorPos+=(int)strlen(sTemp);
#endif
}

inline bool less_stricmp( const char* left,const char* right )
{
	return stricmp(left,right) < 0;
}

//////////////////////////////////////////////////////////////////////////
int CXConsole::GetNumVars()
{
	return (int)m_mapVariables.size();
}

//////////////////////////////////////////////////////////////////////////
void CXConsole::GetSortedVars( const char **pszArray,size_t numItems )
{
	assert( pszArray );
	
	if (numItems == 0)
		return;

	size_t i = 0;
	for (ConsoleVariablesMap::iterator it = m_mapVariables.begin(); it != m_mapVariables.end(); it++)
	{
		if (i >= numItems)
			break;
		/*
		if (i > 0)
		{
			// Ignore duplicate strings.
			if (stricmp(pszArray[i-1],it->first.c_str()) == 0)
				continue;
		}
		*/
		pszArray[i] = it->first.c_str();
		i++;
	}
	std::sort( pszArray,pszArray+numItems,less_stricmp );
}

//////////////////////////////////////////////////////////////////////////
const char* CXConsole::AutoComplete( const char* substr )
{
	std::vector<const char*> cmds;
	cmds.resize( GetNumVars() );
	GetSortedVars( &cmds[0],cmds.size() );

	size_t substrLen = strlen(substr);
	
	// If substring is empty return first command.
	if (substrLen==0 && cmds.size()>0)
		return cmds[0];

	for (size_t i = 0; i < cmds.size(); i++)
	{
		size_t cmdlen = strlen(cmds[i]);
		if (cmdlen >= substrLen && memicmp(cmds[i],substr,substrLen) == 0)
		{
			if (substrLen == cmdlen)
			{
				i++;
				if (i < cmds.size())
					return cmds[i];
				return cmds[i-1];
			}
			return cmds[i];
		}
	}
	// Not found.
	return "";
}

//////////////////////////////////////////////////////////////////////////
const char* CXConsole::AutoCompletePrev( const char* substr )
{
	std::vector<const char*> cmds;
	cmds.resize( GetNumVars() );
	GetSortedVars( &cmds[0],cmds.size() );

	// If substring is empty return last command.
	if (strlen(substr)==0 && cmds.size()>0)
		return cmds[cmds.size()-1];

	for (unsigned int i = 0; i < cmds.size(); i++)
	{
		if (stricmp(substr,cmds[i])==0)
		{
			if (i > 0) 
				return cmds[i-1];
			else
				return cmds[0];
		}
	}
	return AutoComplete( substr );
}

size_t sizeOf (const string& str)
{
	return str.capacity()+1;
}

size_t sizeOf (const char* sz)
{
	return sz? strlen(sz)+1:0;
}

size_t sizeOf (const ConsoleBuffer& Buffer)
{
	size_t nSize = 0;
	ConsoleBuffer::const_iterator it = Buffer.begin(), itEnd = Buffer.end();
	for (; it != itEnd; ++it)
	{
		nSize += sizeof(*it) + sizeOf (*it);
	}
	return nSize;
}

//! Calculation of the memory used by the whole console system
void CXConsole::GetMemoryUsage (class ICrySizer* pSizer)
{
	pSizer->AddObject (this, sizeof(*this)
		+ sizeOf (m_dqConsoleBuffer)
		+ sizeOf (m_dqHistory)
		+ sizeOf (m_sInputBuffer)
		+ sizeOf (m_sPrevTab)
		);

	{
		ConsoleCommandsMap::const_iterator it = m_mapCommands.begin(), itEnd = m_mapCommands.end();
		for (;it != itEnd; ++it)
			pSizer->AddObject (&(*it), sizeof(*it) + sizeOf(it->first) + it->second.sizeofThis());
	}

	{
		ConsoleBindsMap::const_iterator it = m_mapBinds.begin(), itEnd = m_mapBinds.end();
		for (; it != itEnd; ++it)
			pSizer->AddObject (&(*it), sizeof(*it) + sizeOf (it->first) + sizeOf (it->second));
	}

	{
		ConsoleVariablesMap::const_iterator it = m_mapVariables.begin(), itEnd = m_mapVariables.end();
		for (; it != itEnd; ++it)
		{
			pSizer->AddObject (&*it, sizeof(*it) + sizeOf(it->first));
			it->second->GetMemoryUsage (pSizer);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CXConsole::ConsoleInputLog( const char *format,... )
{
	va_list args;
	va_start(args,format);
	GetISystem()->GetILog()->LogV( ILog::eInput,format,args );
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
void CXConsole::ConsoleLog( const char *format,... )
{
	va_list args;
	va_start(args,format);
	GetISystem()->GetILog()->LogV( ILog::eAlways,format,args );
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
void CXConsole::ConsoleWarning( const char *format,... )
{
	va_list args;
	va_start(args,format);
	GetISystem()->GetILog()->LogV( ILog::eWarningAlways,format,args );
	va_end(args);
}

//////////////////////////////////////////////////////////////////////////
bool CXConsole::OnBeforeVarChange( ICVar *pVar,const char *sNewValue )
{
	if (!m_consoleVarSinks.empty())
	{
		ConsoleVarSinks::iterator it,next;
		for (it = m_consoleVarSinks.begin(); it != m_consoleVarSinks.end(); it = next)
		{
			next = it; next++;
			if (!(*it)->OnBeforeVarChange(pVar,sNewValue))
				return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CXConsole::AddConsoleVarSink( IConsoleVarSink *pSink )
{
	m_consoleVarSinks.push_back(pSink);
}

//////////////////////////////////////////////////////////////////////////
void CXConsole::RemoveConsoleVarSink( IConsoleVarSink *pSink )
{
	m_consoleVarSinks.remove(pSink);
}
