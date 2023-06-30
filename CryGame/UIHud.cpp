//////////////////////////////////////////////////////////////////////
//
//	FarCry Game Source code
//	
//	File:Hud.cpp
//  Description: Player Hud implementation
//
//	History:
//	-June 03,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "UIHud.h"
#include <IFont.h>
#include <IScriptSystem.h>

//////////////////////////////////////////////////////////////////////////
CUIHud::CUIHud(CXGame *pGame,ISystem *pISystem)
{	
	m_pGame=pGame;
	m_pISystem = pISystem;
	m_init = false;
	m_pHudScriptObj=NULL;
	m_pScriptSystem=m_pISystem->GetIScriptSystem();

	ICryFont *pICryFont=m_pISystem->GetICryFont();

	if(pICryFont)
	{
		m_pFont= pICryFont->GetFont("Default");
		m_pFont->SetEffect("default");
	}
	else m_pFont=0;
}

//////////////////////////////////////////////////////////////////////////
CUIHud::~CUIHud()
{
	ShutDown();
	if(m_pHudScriptObj)
		m_pHudScriptObj->Release();
}

bool CUIHud::Reset()
{
	char filename[512];
	sprintf(filename,"Scripts/$GT$/Hud/%s", m_pISystem->GetIConsole()->GetCVar("cl_hud_name")->GetString());
		
	if (!m_pGame->ExecuteScript(filename,true))
	{
		m_pISystem->GetILog()->Log("Cannot load script %s", filename);
		return false;
	}
	
	if (!m_pGame->ExecuteScript("Scripts/$GT$/Hud/scoreboard.lua",true))
	{
		m_pISystem->GetILog()->Log("Cannot load script Scripts/$GT$/Hud/scoreboard.lua");
		return false;
	}
	
	// initialize hud
	m_pHudScriptObj=m_pScriptSystem->CreateEmptyObject();
	if(!m_pScriptSystem->GetGlobalValue("Hud",m_pHudScriptObj))
		return false;

	m_pScriptSystem->BeginCall("Hud","OnInit");
	m_pScriptSystem->PushFuncParam(m_pHudScriptObj);
	m_pScriptSystem->EndCall();
	return true;

}
// create and initialize the hud
//////////////////////////////////////////////////////////////////////////
bool CUIHud::Init(IScriptSystem *pScriptSystem)
{
	if (m_init)
		return true;

	
	m_init=true;//Reset();
		
	return (true);	
}

//////////////////////////////////////////////////////////////////////////
void CUIHud::SetFont(const char *pszFontName, const char *pszEffectName)
{
	m_pFont=m_pISystem->GetICryFont()->GetFont(pszFontName);
	if (!m_pFont)
	{
		m_pFont=m_pISystem->GetICryFont()->GetFont("Default");
		m_pFont->SetEffect("default");
	}else
		m_pFont->SetEffect(pszEffectName);
}

// write a number on the screen using special textures numbers
//////////////////////////////////////////////////////////////////////////
void CUIHud::WriteNumber(int px, int py, int number, float r, float g, float b, float a, float xsize/* =1 */, float ysize/* =1 */)
{
	m_pFont->Reset();
	vector2f hsize(xsize,ysize);
	m_pFont->SetSize(hsize);
	color4f hcolor(r,g,b,a);
	m_pFont->SetColor(hcolor);

	char szText[16];

	sprintf(szText, "%d", number);

	m_pFont->DrawString((float)(px),(float)(py),szText);
}

//////////////////////////////////////////////////////////////////////////
void CUIHud::WriteString(int px, int py, const wchar_t *swStr, float r, float g, float b, float a, float xsize/* =1 */, float ysize/* =1 */, float fWrapWidth)
{
	m_pFont->Reset();
	vector2f hsize (xsize,ysize);
	m_pFont->SetSize(hsize);
	color4f hcolor(r,g,b,a);
	m_pFont->SetColor(hcolor);

	if (fWrapWidth > 0)
	{
		m_pFont->DrawWrappedStringW((float)(px),(float)(py), fWrapWidth, swStr );
	}
	else
	{
		m_pFont->DrawStringW((float)(px),(float)(py),swStr );
	}
}

//////////////////////////////////////////////////////////////////////////
//void CUIHud::WriteStringFixed(int px, int py, char *pszStr, float r, float g, float b, float a,float xsize/* =1 */, float ysize/* =1 */, float fWidthScale)
void CUIHud::WriteStringFixed(int px, int py, const wchar_t *swStr, float r, float g, float b, float a,float xsize/* =1 */, float ysize/* =1 */, float fWidthScale)
{
	m_pFont->Reset();
	m_pFont->SetSameSize(TRUE);
	m_pFont->SetCharWidthScale(fWidthScale);
	vector2f hsize(xsize,ysize);
	m_pFont->SetSize(hsize);
	color4f hcolor(r,g,b,a);
	m_pFont->SetColor(hcolor);
	m_pFont->DrawStringW((float)(px),(float)(py),swStr );
}

// update the hud every frame
//////////////////////////////////////////////////////////////////////////
bool CUIHud::Update()
{
	if (!m_init)
		return false;
	m_pScriptSystem->BeginCall("Hud","OnUpdate");
	m_pScriptSystem->PushFuncParam(m_pHudScriptObj);
	m_pScriptSystem->EndCall();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CUIHud::ShutDown()
{
	if (!m_init)
		return; 
	
	m_pScriptSystem->BeginCall("Hud","OnShutdown");
	m_pScriptSystem->PushFuncParam(m_pHudScriptObj);
	m_pScriptSystem->EndCall();
}
