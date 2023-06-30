#pragma once
#include "afxwin.h"

#include "LMCompStructures.h"

//#define DEBUG_OUTPUTS

// CLMCompDialog dialog

class CLMCompDialog : public CDialog, public ICompilerProgress
{
	DECLARE_DYNAMIC(CLMCompDialog)

public:
	CLMCompDialog(CWnd* pParent = NULL);   // standard constructor
	CLMCompDialog(ISystem *pISystem);
	virtual ~CLMCompDialog();

// Dialog Data
	enum { IDD = IDD_LM_COMPILER };

	void RecompileAll();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void Finish(const bool cbErrorsOccured = false);

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnUpdateLightMapGenerationProgress( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateLightMapGenerationMemUsage( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateLightMapGenerationMemUsageStatic( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateBrushInfoEdit( WPARAM wParam, LPARAM lParam );

	afx_msg void OnReleasedCaptureTexelSize( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnReleasedCaptureAngleSlider( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnBnClickedCancel();
	afx_msg void OnShowErrorLog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRecompileLights();

	bool m_bStarted;
	bool m_bCloseDialog;
	bool m_bAutoBackupEnabledSaved;
	int m_iAutoRemindTimeSaved;

public:
	CButton m_cShadows;
	volatile SSharedLMEditorData m_sSharedData;	//the shared data between lightmap compiler and editor during compilation, this should better be realized vbia a pattern, but next in version
#ifdef DEBUG_OUTPUTS
	CButton m_cDbgBorders;
	CButton m_cDontMergePolys;
#endif
	CComboBox m_cTextureSize;
	CEdit m_txtCompilerOutput;
	ISystem *m_pISystem;

	void Output(const char *pszText);
	
	void UpdateGenParams();

	afx_msg void OnBnClickedRecompileAll();
	afx_msg void OnBnClickedRecompileSelection();
	afx_msg void OnBnClickedRecompileChanges();

	CButton m_cUseSunLight;
	CButton m_cSpotAsPointlight;

	CButton m_cSubSampling;
	CButton m_cOcclusion;
  CButton m_cHDR;
};
