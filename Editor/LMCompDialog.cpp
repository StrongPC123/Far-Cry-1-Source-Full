// LMCompDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CryEdit.h"
#include "LMCompDialog.h"
#include "LightmapGen.h"

#include <I3DEngine.h>
#include "IEntitySystem.h"

#include "Objects\ObjectManager.h"
#include "Objects\BrushObject.h"

#include "Settings.h"

#include "resource.h"
#include ".\lmcompdialog.h"
#include "./LightmapCompiler/IndoorLightPatches.h"

// CLMCompDialog dialog

#define WM_UPDATE_LIGHTMAP_GENERATION_PROGRESS ( WM_USER + 1 )
#define WM_UPDATE_LIGHTMAP_GENERATION_MEMUSAGE ( WM_USER + 2 )
#define WM_UPDATE_LIGHTMAP_GENERATION_MEMUSAGE_STATIC ( WM_USER + 3 )
#define WM_UPDATE_GLM_NAME_EDIT ( WM_USER + 4 )

IMPLEMENT_DYNAMIC(CLMCompDialog, CDialog)
CLMCompDialog::CLMCompDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLMCompDialog::IDD, pParent)
{
}

CLMCompDialog::CLMCompDialog(ISystem *pISystem)
	: CDialog(CLMCompDialog::IDD, NULL)
{
	m_pISystem = pISystem;
	m_sSharedData.uiProgressMessage		= WM_UPDATE_LIGHTMAP_GENERATION_PROGRESS;
	m_sSharedData.uiMemUsageMessage		= WM_UPDATE_LIGHTMAP_GENERATION_MEMUSAGE;
	m_sSharedData.uiMemUsageStatic		= WM_UPDATE_LIGHTMAP_GENERATION_MEMUSAGE_STATIC;
	m_sSharedData.uiGLMNameEdit			= WM_UPDATE_GLM_NAME_EDIT;
	m_bStarted = false;
	m_bCloseDialog = false;
	// turn auto reminder and backup off as long as modal dialog is up
	m_bAutoBackupEnabledSaved = gSettings.autoBackupEnabled;
	gSettings.autoBackupEnabled = false;
	m_iAutoRemindTimeSaved = gSettings.autoRemindTime;
	gSettings.autoRemindTime = 0;
}

CLMCompDialog::~CLMCompDialog()
{
	// set auto reminder and backup back to the state it was set before
	gSettings.autoBackupEnabled = m_bAutoBackupEnabledSaved;
	gSettings.autoRemindTime = m_iAutoRemindTimeSaved;
}

void CLMCompDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_CHECK_SHADOWS, m_cShadows);
  DDX_Control(pDX, IDC_LM_RESOLUTION, m_cTextureSize);
  DDX_Control(pDX, IDC_COMPILER_OUTPUT, m_txtCompilerOutput);
  DDX_Control(pDX, IDC_USE_SUN_LIGHT, m_cUseSunLight);
  DDX_Control(pDX, IDC_SPOTASPOINT, m_cSpotAsPointlight);
  DDX_Control(pDX, IDC_SUB_SAMPLING, m_cSubSampling);
  DDX_Control(pDX, IDC_OCCLUSION, m_cOcclusion);
#ifdef DEBUG_OUTPUTS
  DDX_Control(pDX, IDC_DBG_BORDERS, m_cDbgBorders);
  DDX_Control(pDX, IDC_DONT_MERGE_POLYS, m_cDontMergePolys);
#endif
  DDX_Control(pDX, IDC_HDR, m_cHDR);
}


BEGIN_MESSAGE_MAP(CLMCompDialog, CDialog)
	ON_BN_CLICKED(ID_RECOMPILE_ALL, OnBnClickedRecompileAll)
	ON_BN_CLICKED(ID_RECOMPILE_SELECTION, OnBnClickedRecompileSelection)
	ON_BN_CLICKED(ID_RECOMPILE_CHANGES, OnBnClickedRecompileChanges)
	ON_BN_CLICKED(ID_CANCEL, OnBnClickedCancel)
	ON_MESSAGE( WM_UPDATE_LIGHTMAP_GENERATION_PROGRESS, OnUpdateLightMapGenerationProgress ) 
	ON_MESSAGE( WM_UPDATE_LIGHTMAP_GENERATION_MEMUSAGE, OnUpdateLightMapGenerationMemUsage ) 
	ON_MESSAGE( WM_UPDATE_LIGHTMAP_GENERATION_MEMUSAGE_STATIC, OnUpdateLightMapGenerationMemUsageStatic ) 
	ON_MESSAGE( WM_UPDATE_GLM_NAME_EDIT, OnUpdateBrushInfoEdit ) 
	ON_NOTIFY( NM_RELEASEDCAPTURE, IDC_TEXEL_SIZE_SLIDER, OnReleasedCaptureTexelSize )
	ON_NOTIFY( NM_RELEASEDCAPTURE, IDC_ANGLE_SLIDER, OnReleasedCaptureAngleSlider )
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(ID_RECOMPILE_LIGHTS, OnBnClickedRecompileLights)
END_MESSAGE_MAP()

DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	char ***pppText = reinterpret_cast<char ***> (dwCookie);
	char **ppText = *pppText;
	LONG iLen = strlen(* ppText) /*- 1*/;
	*pcb = __min(cb, iLen);
	if (*pcb == 0)
	{
		delete pppText;
		return 0;
	}
	memcpy(pbBuff, (* ppText), *pcb);
	*ppText += *pcb;
	return 0;
};

BOOL SetText(const char *pszText, CRichEditCtrl *pEdit)
{
	EDITSTREAM strm;
	const char ***pppText = new const char **;
	*pppText = &pszText;
	strm.dwCookie = (DWORD_PTR) pppText;
	strm.dwError = 0;
	strm.pfnCallback = EditStreamCallback;
	pEdit->SendMessage(EM_LIMITTEXT, 0x7FFFFFF, 0);
	return pEdit->SendMessage(EM_STREAMIN, SF_RTF | SFF_SELECTION, (LPARAM) &strm);
};

void CLMCompDialog::Output(const char *pszText)
{
	m_txtCompilerOutput.ReplaceSel(pszText);
	m_txtCompilerOutput.UpdateWindow();
}

// CLMCompDialog message handlers

BOOL CLMCompDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	LMGenParam sParam = CLightmapGen::GetGenParam();
	
	m_cShadows.SetCheck(sParam.m_bComputeShadows ? BST_CHECKED : 0);


#ifdef DEBUG_OUTPUTS
	m_cDbgBorders.SetCheck(sParam.m_bDebugBorders ? BST_CHECKED : 0);
	m_cDontMergePolys.SetCheck(sParam.m_bDontMergePolys ? BST_CHECKED : 0);
#endif

	m_cUseSunLight.SetCheck(sParam.m_bUseSunLight ? BST_CHECKED : 0);
	m_cSpotAsPointlight.SetCheck(sParam.m_bSpotAsPointlight ? BST_CHECKED : 0);
		
	m_cTextureSize.AddString("64");
	m_cTextureSize.AddString("128");
	m_cTextureSize.AddString("256");
	m_cTextureSize.AddString("512");
	m_cTextureSize.AddString("1024");
	m_cTextureSize.AddString("2048");

	char szBuffer[128];

	sprintf(szBuffer, "%i", sParam.m_iTextureResolution);
	m_cTextureSize.SetCurSel(m_cTextureSize.FindString(0, szBuffer));

	// init progress bar ctrl
	{
		CProgressCtrl* pCtrl( (CProgressCtrl*) GetDlgItem( IDC_LM_GEN_PROGRESS ) );
		assert( 0 != pCtrl );
		pCtrl->SetRange( 0, 100 );
		pCtrl->SetPos( 0 );
	}
	// init progress bar ctrl
	{
		CProgressCtrl* pCtrl( (CProgressCtrl*) GetDlgItem( IDC_LM_MEMUSAGE ) );
		assert( 0 != pCtrl );
		pCtrl->SetRange( 0, 1000 );
		pCtrl->SetPos( 0 );
	}

	// init texel size slider
	{
		CSliderCtrl* pCtrl( (CSliderCtrl*) GetDlgItem( IDC_TEXEL_SIZE_SLIDER ) );
		assert( 0 != pCtrl );
		pCtrl->SetRange( 0, 6 );
		CStatic* pStatic( (CStatic*) GetDlgItem( IDC_TEXEL_SIZE ) );
		assert( 0 != pStatic );
		switch(static_cast<unsigned int>(sParam.m_fTexelSize * 100.0f))
		{
		case 25:
			pCtrl->SetPos(0);
			pStatic->SetWindowText("0.25");
			break;
		case 30:
			pCtrl->SetPos(1);
			pStatic->SetWindowText("0.30");
			break;
		case 50:
			pCtrl->SetPos(2);
			pStatic->SetWindowText("0.50");
			break;
		case 75:
			pCtrl->SetPos(3);
			pStatic->SetWindowText("0.75");
			break;
		case 100:
			pCtrl->SetPos(4);
			pStatic->SetWindowText("1.00");
			break;
		case 150:
			pCtrl->SetPos(5);
			pStatic->SetWindowText("1.50");
			break;
		case 200:
			pCtrl->SetPos(6);
			pStatic->SetWindowText("2.00");
			break;
		default:
			pCtrl->SetPos(0);
			pStatic->SetWindowText("0.25");
			break;
		}
	}
	(sParam.m_iSubSampling == 9)?m_cSubSampling.SetCheck(BST_CHECKED):m_cSubSampling.SetCheck(0);
		
	m_cOcclusion.SetCheck((sParam.m_bGenOcclMaps)?BST_CHECKED:0);
	m_cHDR.SetCheck((sParam.m_bHDR)?BST_CHECKED:0);

	{
		CSliderCtrl* pAngleCtrl( (CSliderCtrl*) GetDlgItem( IDC_ANGLE_SLIDER ) );
		assert( 0 != pAngleCtrl );
		pAngleCtrl->SetRange( 0, 90/5 );
		CStatic* pAngleStatic( (CStatic*) GetDlgItem( IDC_ANGLE ) );
		assert( 0 != pAngleStatic );
		char text[4];
		sprintf(text, "%i", sParam.m_uiSmoothingAngle);
		pAngleCtrl->SetPos(sParam.m_uiSmoothingAngle / 5);
		pAngleStatic->SetWindowText(text);
	}

 	m_txtCompilerOutput.SetLimitText(2048 * 1024);
	m_sSharedData.hwnd = m_hWnd;

	CButton* pButton( (CButton*) GetDlgItem( ID_RECOMPILE_ALL ) );
	assert( 0 != pButton );
	pButton->SetFocus();

	ICVar* pCvar = GetISystem()->GetIConsole()->GetCVar("e_light_maps_occlusion");
	if(pCvar && pCvar->GetIVal() == 1)
		m_cOcclusion.ShowWindow(true);
	else
		m_cOcclusion.ShowWindow(false);

  pCvar = GetISystem()->GetIConsole()->GetCVar("r_HDRRendering");
  if(pCvar)
    m_cHDR.ShowWindow(true);
  else
    m_cHDR.ShowWindow(false);

	return TRUE;
}

void CLMCompDialog::UpdateGenParams()
{
	m_bStarted = true;	
	m_sSharedData.bCancelled = false;
	std::vector<IEntityRender *> nodes;
	std::vector<const CDLight *> vLights;
	I3DEngine *pI3DEngine = m_pISystem->GetI3DEngine();
	const CDLight *pLights = NULL;
	char szBuffer[128];
	LMGenParam sParam;

	// Retrieve dialog parameters
	sParam.m_bComputeShadows = (m_cShadows.GetCheck() == BST_CHECKED);

#ifdef DEBUG_OUTPUTS
	sParam.m_bDebugBorders = (m_cDbgBorders.GetCheck() == BST_CHECKED);
	sParam.m_bDontMergePolys = (m_cDontMergePolys.GetCheck() == BST_CHECKED);
#else
	sParam.m_bDebugBorders = false;		sParam.m_bDontMergePolys = false;
#endif

	sParam.m_iSubSampling = (m_cSubSampling.GetCheck() == BST_CHECKED)?9:1;//only support 9x or 1x

	sParam.m_bGenOcclMaps = (m_cOcclusion.GetCheck() == BST_CHECKED)?true:false;
	sParam.m_bHDR = (m_cHDR.GetCheck() == BST_CHECKED)?true:false;

	CSliderCtrl* pAngleCtrl( (CSliderCtrl*) GetDlgItem( IDC_ANGLE_SLIDER ) );
	assert( 0 != pAngleCtrl );
	CStatic* pAngleStatic( (CStatic*) GetDlgItem( IDC_ANGLE ) );
	assert( 0 != pAngleStatic );
	sParam.m_uiSmoothingAngle = pAngleCtrl->GetPos() * 5;
	char text[4];
	sprintf(text, "%i", sParam.m_uiSmoothingAngle);
	pAngleStatic->SetWindowText(text);

	CStatic* pStatic( (CStatic*) GetDlgItem( IDC_TEXEL_SIZE ) );
	assert( 0 != pStatic );
	CSliderCtrl* pCtrl( (CSliderCtrl*) GetDlgItem( IDC_TEXEL_SIZE_SLIDER ) );
	assert( 0 != pCtrl );
	switch(pCtrl->GetPos())
	{
	case 0:
		sParam.m_fTexelSize = 0.25f;
		pStatic->SetWindowText("0.25");
		break;
	case 1:
		sParam.m_fTexelSize = 0.30f;
		pStatic->SetWindowText("0.3");
		break;
	case 2:
		sParam.m_fTexelSize = 0.5f;
		pStatic->SetWindowText("0.50");
		break;
	case 3:
		sParam.m_fTexelSize = 0.75f;
		pStatic->SetWindowText("0.75");
		break;
	case 4:
		sParam.m_fTexelSize = 1.00f;
		pStatic->SetWindowText("1.00");
		break;
	case 5:
		sParam.m_fTexelSize = 1.50f;
		pStatic->SetWindowText("1.50");
		break;
	case 6:
		sParam.m_fTexelSize = 2.00f;
		pStatic->SetWindowText("2.00");
		break;
	default:
		sParam.m_fTexelSize = 0.25f;
		pStatic->SetWindowText("0.25");
		break;
	}

	m_cTextureSize.GetLBText(m_cTextureSize.GetCurSel(), szBuffer);
	sParam.m_iTextureResolution = atoi(szBuffer);

	sParam.m_bUseSunLight		= (m_cUseSunLight.GetCheck() == BST_CHECKED);
	sParam.m_bSpotAsPointlight	= (m_cSpotAsPointlight.GetCheck() == BST_CHECKED);
		
	CLightmapGen::SetGenParam(sParam);
} 
 
void CLMCompDialog::Finish(const bool cbErrorsOccured)
{
	m_bStarted = false;
	if(	m_bCloseDialog == true)
		CDialog::OnOK();
	CButton* pCtrl( (CButton*) GetDlgItem( IDOK ) );
	assert( 0 != pCtrl );
	pCtrl->SetActiveWindow();		
	pCtrl->SetFocus();
	if(cbErrorsOccured)
		OnShowErrorLog();
}

afx_msg void CLMCompDialog::OnBnClickedRecompileAll()
{
	// Recompute All
	UpdateGenParams();
	CLightmapGen cLightmapGen(&m_sSharedData);
	Finish(cLightmapGen.GenerateAll(GetIEditor(), (ICompilerProgress *) this));
}

afx_msg void CLMCompDialog::OnBnClickedRecompileSelection()
{
	// Recompute Selection Only
	UpdateGenParams();
	CLightmapGen cLightmapGen(&m_sSharedData);
	Finish(cLightmapGen.GenerateSelected(GetIEditor(), (ICompilerProgress *) this));
}

afx_msg void CLMCompDialog::OnBnClickedRecompileChanges()
{
	// Recompute Changes
	UpdateGenParams();
	CLightmapGen cLightmapGen(&m_sSharedData);
	Finish(cLightmapGen.GenerateChanged(GetIEditor(), (ICompilerProgress *) this));
}

afx_msg LRESULT 
CLMCompDialog::OnUpdateLightMapGenerationProgress( WPARAM wParam, LPARAM lParam )
{
	CProgressCtrl* pCtrl( (CProgressCtrl*) GetDlgItem( IDC_LM_GEN_PROGRESS ) );
	assert( 0 != pCtrl );
	pCtrl->SetPos( (int) wParam );

	return( 0 );
}

afx_msg LRESULT 
CLMCompDialog::OnUpdateLightMapGenerationMemUsage( WPARAM wParam, LPARAM lParam )
{
	CProgressCtrl* pCtrl( (CProgressCtrl*) GetDlgItem( IDC_LM_MEMUSAGE ) );
	assert( 0 != pCtrl );
	pCtrl->SetPos( (int) wParam );

	return( 0 );
}

afx_msg LRESULT 
CLMCompDialog::OnUpdateLightMapGenerationMemUsageStatic( WPARAM wParam, LPARAM lParam )
{
	CStatic* pCtrl( (CStatic*) GetDlgItem( IDC_STATIC_MEMUSAGE ) );
	assert( 0 != pCtrl );
	char pText[4];
	sprintf(pText,"%d",wParam);
	pCtrl->SetWindowText(pText);

	return( 0 );
}

afx_msg LRESULT 
CLMCompDialog::OnUpdateBrushInfoEdit( WPARAM wParam, LPARAM lParam )
{
	CEdit* pEditBrushName( (CEdit*) GetDlgItem( IDC_EDIT_BRUSHNAME ) );			assert( 0 != pEditBrushName );
	CEdit* pEditBrushType( (CEdit*) GetDlgItem( IDC_EDIT_BRUSHTYPE ) );			assert( 0 != pEditBrushType );

	if(wParam == 0 || lParam == 0)
	{
		pEditBrushType->SetWindowText(" ");
		pEditBrushName->SetWindowText(" ");
		return( 0 );
	}

	const CRadMesh* pMesh	= reinterpret_cast<CRadMesh*>(wParam);
	IStatObj* pIGeom		= reinterpret_cast<IStatObj*>(lParam);

	pEditBrushType->SetWindowText(pIGeom->GetFileName());
	pEditBrushName->SetWindowText(pMesh->m_sGLMName);

	return( 0 );
}

void CLMCompDialog::OnBnClickedCancel()
{
	if(m_bStarted == false)
	{
		CDialog::OnOK();
	}
	m_sSharedData.bCancelled = true;
	m_bCloseDialog = true;
	Output("\r\nCancel pressed - please wait...\r\n\r\n");
}

void CLMCompDialog::OnShowErrorLog()
{
	CButton* pErrorLog( (CButton*) GetDlgItem( IDC_CHECK_ERROR_LOG ) );
	if(!pErrorLog ||	pErrorLog->GetCheck() != BST_CHECKED)
		return;
	CFileUtil::EditTextFile( "LightmapError.log" );
}

afx_msg void 
CLMCompDialog::OnReleasedCaptureTexelSize( NMHDR* pNMHDR, LRESULT* pResult )
{
	CSliderCtrl* pCtrl( (CSliderCtrl*) GetDlgItem( IDC_TEXEL_SIZE_SLIDER ) );
	assert( 0 != pCtrl );
	CStatic* pStatic( (CStatic*) GetDlgItem( IDC_TEXEL_SIZE ) );
	assert( 0 != pStatic );
	switch(pCtrl->GetPos())
	{
	case 0:
		pStatic->SetWindowText("0.25");
		break;
	case 1:
		pStatic->SetWindowText("0.3");
		break;
	case 2:
		pStatic->SetWindowText("0.50");
		break;
	case 3:
		pStatic->SetWindowText("0.75");
		break;
	case 4:
		pStatic->SetWindowText("1.00");
		break;
	case 5:
		pStatic->SetWindowText("1.50");
		break;
	case 6:
		pStatic->SetWindowText("2.00");
		break;
	default:
		pStatic->SetWindowText("0.25");
		break;
	}
	*pResult = 0;
}

afx_msg void 
CLMCompDialog::OnReleasedCaptureAngleSlider( NMHDR* pNMHDR, LRESULT* pResult )
{
	CSliderCtrl* pCtrl( (CSliderCtrl*) GetDlgItem( IDC_ANGLE_SLIDER ) );
	assert( 0 != pCtrl );
	CStatic* pStatic( (CStatic*) GetDlgItem( IDC_ANGLE ) );
	assert( 0 != pStatic );
	char text[4];
	sprintf(text, "%i", pCtrl->GetPos()*5);
	pStatic->SetWindowText(text);
	*pResult = 0;
}

void CLMCompDialog::OnBnClickedOk()
{
	if(m_bStarted == false)
	{
		CDialog::OnOK();
	}
	m_bCloseDialog = true;
	Output("\r\nClose pressed - please wait...\r\n\r\n");
}
 
//////////////////////////////////////////////////////////////////////////
void CLMCompDialog::RecompileAll()
{
	OnBnClickedRecompileAll();
}

void CLMCompDialog::OnBnClickedRecompileLights()
{
	// Recompute All
	UpdateGenParams();
	CLightmapGen cLightmapGen(&m_sSharedData);
	LMGenParam params = cLightmapGen.GetGenParam();
	params.m_bOnlyExportLights = true;
	cLightmapGen.SetGenParam( params );
	const bool cbErrorsOccured = cLightmapGen.GenerateAll(GetIEditor(), (ICompilerProgress *) this);
	params.m_bOnlyExportLights = false;
	cLightmapGen.SetGenParam( params );
	Finish(cbErrorsOccured);	
}
