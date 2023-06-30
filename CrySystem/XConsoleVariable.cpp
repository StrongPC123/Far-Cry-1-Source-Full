// XConsoleVariable.cpp: implementation of the CXConsoleVariable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XConsole.h"
#include "XConsoleVariable.h"
#include "System.h"

#include <IConsole.h>
#include <ISystem.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXConsoleVariable::CXConsoleVariable(CXConsole *pConsole,IScriptSystem *pSS,const char *sName,int nFlags,int nType, const char *help)
{
	assert(pConsole);

	m_psHelp = help;
	
#if defined(_DEBUG) && !defined(LINUX)
	if(!*help) 
	{
		char buf[100];
		sprintf(buf, "MISSING HELP FOR VAR: %s\n", sName);
		OutputDebugString(buf);
	};
#endif
	
	const char *sTempValue=NULL;
	m_pConsole=pConsole;
	m_pScriptSystem=pSS;
	strcpy(m_sName,sName);
	m_nFlags=nFlags;
	m_nType=nType;

	m_bAutoDelete=true;

	
	m_nValue = &m_localInt;
	m_fValue = &m_localFloat;
	m_sValue = localString;

	memset(m_sValue,0,VAR_STRING_SIZE);

	
	m_bLoadedFromScript=false;
	if (CanGetValueFromScript() && m_pScriptSystem->GetGlobalValue(m_sName,sTempValue))
	{
		m_bLoadedFromScript=true;
		strcpy(m_sValue,sTempValue);
		*m_fValue=(float)(atof(m_sValue));
		*m_nValue=atoi(m_sValue);
	}
	m_hScriptTag=m_pScriptSystem->CreateTaggedValue(m_sName,m_sValue);
}


CXConsoleVariable::CXConsoleVariable(CXConsole *pConsole,IScriptSystem *pSS,const char *sName,void *pVar,int nFlags,int nType, const char *help)
{
	assert(pConsole);

	m_psHelp = help;
	m_pConsole=pConsole;
	m_pScriptSystem=pSS;
	strcpy(m_sName,sName);
	m_nFlags=nFlags;
	m_nType=nType;

	m_nValue = &m_localInt;
	m_fValue = &m_localFloat;
	m_sValue = localString;

	const char *sTempValue=NULL;
	m_bLoadedFromScript=false;
	m_bAutoDelete=false;
	switch(nType)
	{
	case CVAR_STRING:
		m_sValue = (char *)pVar;
		
		if(CanGetValueFromScript() && m_pScriptSystem->GetGlobalValue(sName,sTempValue))
		{
			m_bLoadedFromScript=true;
			strcpy(m_sValue,sTempValue);
			*m_fValue=(float)(atof(sTempValue));
			*m_nValue=atoi(sTempValue);
		}
		m_hScriptTag=m_pScriptSystem->CreateTaggedValue(sName,m_sValue);
	break;
	case CVAR_INT:
		m_nValue=(int *)pVar;

		if(CanGetValueFromScript() && m_pScriptSystem->GetGlobalValue(sName,sTempValue))
		{
			m_bLoadedFromScript=true;
			*m_nValue=atoi(sTempValue);
		}
		m_hScriptTag=m_pScriptSystem->CreateTaggedValue(sName,m_nValue);
		memset(m_sValue,0,VAR_STRING_SIZE);
	break;
	case CVAR_FLOAT:
		m_fValue=(float *)pVar;
		if(CanGetValueFromScript() && m_pScriptSystem->GetGlobalValue(sName,sTempValue))
		{
			m_bLoadedFromScript=true;
			*m_fValue=(float)(atof(sTempValue));
		}
		m_hScriptTag=m_pScriptSystem->CreateTaggedValue(sName,m_fValue);
		memset(m_sValue,0,VAR_STRING_SIZE);
	break;
	default:
		CryError( "<CrySystem> (CXConsoleVariable::CXConsoleVariable) Unknown console variable type" );
		break;
	}
	Refresh();
	
}

//! Changes the variable storage pointer
void CXConsoleVariable::SetSrc (void* pSrc)
{
	if (!pSrc)
		return;

	m_pScriptSystem->RemoveTaggedValue(m_hScriptTag);

	switch (m_nType)
	{
	case CVAR_STRING:
		strcpy ((char*)pSrc, m_sValue );
		m_sValue = (char*)pSrc;
		m_hScriptTag=m_pScriptSystem->CreateTaggedValue(m_sName,m_sValue);
		break;
	case CVAR_INT:
		*(int*)pSrc = *m_nValue;
		m_nValue = (int*)pSrc;
		m_hScriptTag=m_pScriptSystem->CreateTaggedValue(m_sName,m_nValue);
		break;
	case CVAR_FLOAT:
		*(float*)pSrc = *m_fValue;
		m_fValue = (float*)pSrc;
		m_hScriptTag=m_pScriptSystem->CreateTaggedValue(m_sName,m_fValue);
		break;
	}

	m_bAutoDelete = false;
}


CXConsoleVariable::~CXConsoleVariable()
{
	if(m_bAutoDelete)
	{
	}
	else
	{
		switch(m_nType)
		{
		case CVAR_STRING:			
			break;
		case CVAR_INT:
			break;
		case CVAR_FLOAT:
			break;
		default:
			CryError( "<CrySystem> (CXConsoleVariable::~CXConsoleVariable) Unknown console variable type" );
			break;
		}
		
	}
}

//////////////////////////////////////////////////////////////////////////
bool CXConsoleVariable::CanGetValueFromScript()
{
	if (m_nFlags&(VF_CHEAT|VF_READONLY))
	{
		if (!((CSystem*)GetISystem())->IsDevMode())
			return false;
	}
	return true;
}

int CXConsoleVariable::GetIVal()
{
	return (*m_nValue);
}

float CXConsoleVariable::GetFVal()
{
	return (*m_fValue);
}

char *CXConsoleVariable::GetString()
{
	return (m_sValue);
}

//////////////////////////////////////////////////////////////////////////
void CXConsoleVariable::ForceSet(const char* s)
{	
	bool bCheat=false;
	bool bReadOnly=false;
	if (m_nFlags & VF_READONLY)
	{
		m_nFlags&=~VF_READONLY;
		bReadOnly=true;
	}

	if (m_nFlags & VF_CHEAT)
	{
		m_nFlags&=~VF_CHEAT;
		bCheat=true;
	}

	Set(s);
		
	if (bReadOnly)
		m_nFlags|=VF_READONLY;
	if (bCheat)
		m_nFlags|=VF_CHEAT;
}

//////////////////////////////////////////////////////////////////////////
void CXConsoleVariable::Set(const char* s)
{
	if (!m_pConsole->OnBeforeVarChange(this,s))
		return;

	if(m_nValue)
		*m_nValue=atoi(s);
	if(m_fValue)
		*m_fValue=(float)(atof(s));
	if (m_sValue)
	{
		strcpy(m_sValue,s);
	}
}

//////////////////////////////////////////////////////////////////////////
void CXConsoleVariable::Set(float f)
{
	char sTemp[128];
	sprintf(sTemp,"%f",f);
	if (!m_pConsole->OnBeforeVarChange(this,sTemp))
		return;

	if(m_nValue)
		*m_nValue=(int)(f);
	if(m_fValue)
		*m_fValue=f;
	if (m_sValue)
	{
		sprintf(m_sValue,"%f",f);
	}		
}

void CXConsoleVariable::Set(int i)
{
	char sTemp[128];
	sprintf(sTemp,"%i",i);
	if (!m_pConsole->OnBeforeVarChange(this,sTemp))
		return;

	if(m_nValue)
		*m_nValue=i;
	if(m_fValue)
		*m_fValue=(float)(i);
	if (m_sValue)
	{
		sprintf(m_sValue,"%i",i);
	}
}

void CXConsoleVariable::Refresh()
{
	switch(m_nType)
	{
	case CVAR_STRING:
		Set(m_sValue);
		break;
	case CVAR_INT:
		Set(*m_nValue);
		break;
	case CVAR_FLOAT:
		Set(*m_fValue);
		break;
	default:
		CryError( "<CrySystem> (CXConsoleVariable::Refresh) Unknown console variable type" );
		break;
	}
}

void CXConsoleVariable::ClearFlags (int flags)
{
	m_nFlags&=~flags;
}

int CXConsoleVariable::GetFlags()
{
	return m_nFlags;
}

int CXConsoleVariable::SetFlags( int flags )
{
	m_nFlags = flags;
	return m_nFlags ;
}

int CXConsoleVariable::GetType()
{
	return m_nType;
}

const char* CXConsoleVariable::GetName()
{
	return m_sName;
}

const char* CXConsoleVariable::GetHelp()
{
	return m_psHelp;
}

void CXConsoleVariable::Release()
{
	m_pScriptSystem->RemoveTaggedValue(m_hScriptTag);
	m_pConsole->UnregisterVariable(m_sName);
	delete this;
}

void CXConsoleVariable::GetMemoryUsage (class ICrySizer* pSizer)
{
	pSizer->Add (*this);
	//if (m_bAutoDelete || m_nType != CVAR_STRING)
		//pSizer->Add (m_sValue, VAR_STRING_SIZE);
}