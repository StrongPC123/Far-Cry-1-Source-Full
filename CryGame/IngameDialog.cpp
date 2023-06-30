// IngameDialog.cpp: implementation of the CIngameDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IngameDialog.h"

#define EDGESIZE		8.0f

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CIngameDialog::CIngameDialog()
{
	m_fX=0.0f;
	m_fY=0.0f;
	m_fW=0.0f;
	m_fH=0.0f;
	m_sText="";
	m_pRenderer=NULL;
	m_pFont=NULL;
	m_nFillId=0;
	m_bInited=false;
}
 
//////////////////////////////////////////////////////////////////////////
CIngameDialog::~CIngameDialog()
{
}

//////////////////////////////////////////////////////////////////////////
// INit the dialog; must be called before all other function of this class.
bool CIngameDialog::Init(CIngameDialogMgr *pMgr, int nId, ISystem *pSystem, int nFillId, const char *pszFontName, const char *pszEffectName, int nSize, string sText,wstring swText, float fTimeout)
{
	if (m_bInited || (!pSystem) || (sText.empty() && swText.empty()))
		return (false);
	m_pRenderer=pSystem->GetIRenderer();
	if (!m_pRenderer)
		return (false);
	if ((!m_pRenderer->GetWidth()) || (!m_pRenderer->GetHeight()))
		return false;
	m_sEffect=pszEffectName;

	ICryFont *pCryFont = pSystem->GetICryFont();

	if(!pCryFont)					// is 0 on the dedicated server
		return false;

	m_pFont=pCryFont->GetFont(pszFontName);
	if (m_pFont)
		m_pFont->SetEffect(m_sEffect.c_str());
	else	
	{
		m_pFont=pSystem->GetICryFont()->GetFont("Default");
		m_pFont->SetEffect("IngameDlg");
	}
	m_nSize=nSize;
	vector2f hsize ((float)m_nSize, (float)m_nSize);
	m_pFont->SetSize(hsize);
	// calculate size of box
	m_pFont->SetSameSize(false);
	m_pFont->SetCharWidthScale(1.0f);
	bool b=m_pFont->GetSameSize();
	float f=m_pFont->GetCharWidthScale();
	vector2f Size;
	if (!sText.empty())
		Size=m_pFont->GetTextSize(sText.c_str());	
	else
		Size=m_pFont->GetTextSizeW(swText.c_str());

	m_fW=Size.x*(800.0f/m_pRenderer->GetWidth());
	m_fH=Size.y*(600.0f/m_pRenderer->GetHeight());
	m_pMgr=pMgr;
	m_nId=nId;
	m_nFillId=nFillId;
	m_sText=sText;
	m_swText=swText;
	m_fTimeout=fTimeout;
	m_bInited=true;
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CIngameDialog::SetPos(float x, float y)
{
	m_fX=x;
	m_fY=y;
}

//////////////////////////////////////////////////////////////////////////
bool CIngameDialog::Update()
{
	if (!m_bInited)
		return false;
	vector2f hsize ((float)m_nSize, (float)m_nSize);
	m_pFont->SetSize(hsize);
	m_pFont->SetEffect(m_sEffect.c_str());
	m_pFont->SetSameSize(false);
	m_pFont->SetCharWidthScale(1.0f);
	// draw fill
  m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);

  // disabled, it was rendering strange black rectangle at beggining of missions
	//m_pRenderer->Draw2dImage(m_fX, m_fY, EDGESIZE+EDGESIZE+m_fW, EDGESIZE+EDGESIZE+m_fH, m_nFillId, 0, 1, 1, 0, 0, 1.0f, 1.0f, 1.0f, 1.0f);

  m_pRenderer->SetState(GS_NODEPTHTEST);
	// draw text
	color4f hcolor(1.0f, 1.0f, 1.0f, 1.0f);
	m_pFont->SetColor(hcolor, -1);
	m_pFont->DrawString(m_fX+EDGESIZE,m_fY+EDGESIZE, m_sText.c_str());
	return true;
}
 
//////////////////////////////////////////////////////////////////////////
CIngameDialogMgr::CIngameDialogMgr()
{
	m_nNextId=0;
	m_nDefaultFillId=-1;
	m_pRenderer=NULL;
	m_pTimer=NULL;
}

//////////////////////////////////////////////////////////////////////////
CIngameDialogMgr::~CIngameDialogMgr()
{
	// delete all dialogs
	std::list<SIGDId*>::iterator lstDialogsIt;
	lstDialogsIt=m_lstDialogs.begin();
	while (lstDialogsIt!=m_lstDialogs.end())
	{
		delete ((*lstDialogsIt)->pDialog);
		delete (*lstDialogsIt);
		lstDialogsIt=m_lstDialogs.erase(lstDialogsIt);
	}
	// remove default fill-texture
	if (m_pRenderer && (m_nDefaultFillId>=0))
		m_pRenderer->RemoveTexture(m_nDefaultFillId);
}

//////////////////////////////////////////////////////////////////////////
// Adds a new dialog on the screen.
int CIngameDialogMgr::AddDialog(ISystem *pSystem, int nFillId, const char *pszFontName, const char *pszEffectName, int nSize, string sText,wstring swText, float fTimeout)
{
	if (!m_pRenderer)
		m_pRenderer=pSystem->GetIRenderer();
	if (!m_pTimer)
		m_pTimer=pSystem->GetITimer();
	string sFontName="default";
	string sEffectName="IngameDlg";
	// load default fill-texture if not loaded yet
	if (m_pRenderer && (m_nDefaultFillId<0))
	{
		m_nDefaultFillId=m_pRenderer->LoadTexture("textures/gui/igdlg_fill");
	}
	if (pszFontName && strlen(pszFontName))
		sFontName=pszFontName;
	if (pszEffectName && strlen(pszEffectName))
		sEffectName=pszEffectName;
	if (nFillId<0)
		nFillId=m_nDefaultFillId;
	// create dialog and put in list
	SIGDId *pIGDId=new SIGDId();
	pIGDId->pDialog=new CIngameDialog();
	pIGDId->nId=m_nNextId++;
	pIGDId->pDialog->Init(this, pIGDId->nId, pSystem, nFillId, sFontName.c_str(), sEffectName.c_str(), nSize, sText,swText, fTimeout);
	m_lstDialogs.push_back(pIGDId);
	return pIGDId->nId;
}

//////////////////////////////////////////////////////////////////////////
// Removes a dialog from the screen and destroys it.
void CIngameDialogMgr::RemoveDialog(int nId)
{
	// find and remove dialog
	std::list<SIGDId*>::iterator lstDialogsIt;
	lstDialogsIt=m_lstDialogs.begin();
	while (lstDialogsIt!=m_lstDialogs.end())
	{
		SIGDId *pIGDId=(*lstDialogsIt);
		if (pIGDId->nId==nId)
		{
			delete (pIGDId->pDialog);
			delete pIGDId;
			m_lstDialogs.erase(lstDialogsIt);
			return;
		}
		lstDialogsIt++;
	}
}

//////////////////////////////////////////////////////////////////////////
// Updates and draws all dialogs on the screen. Should be called every frame.
void CIngameDialogMgr::Update()
{
	float y=55.0f;
	std::list<SIGDId*>::iterator lstDialogsIt;
	lstDialogsIt=m_lstDialogs.begin();
	while (lstDialogsIt!=m_lstDialogs.end())
	{
		SIGDId *pIGDId=(*lstDialogsIt);
		if (pIGDId->pDialog->m_fTimeout)
		{
			pIGDId->pDialog->m_fTimeout-=m_pTimer->GetFrameTime();
			if (pIGDId->pDialog->m_fTimeout<=0.0f)
			{
				delete (pIGDId->pDialog);
				delete pIGDId;
				lstDialogsIt=m_lstDialogs.erase(lstDialogsIt);
				continue;
			}
		}
		pIGDId->pDialog->SetPos(10.0f, y);
		pIGDId->pDialog->Update();
		y+=pIGDId->pDialog->GetHeight()+EDGESIZE+EDGESIZE+5.0f;
		lstDialogsIt++;
	}
}