////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   newleveldialog.h
//  Version:     v1.00
//  Created:     24/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __newleveldialog_h__
#define __newleveldialog_h__

#if _MSC_VER > 1000
#pragma once
#endif


// CNewLevelDialog dialog

class CNewLevelDialog : public CDialog
{
	DECLARE_DYNAMIC(CNewLevelDialog)

public:
	CNewLevelDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewLevelDialog();

// Dialog Data
	enum { IDD = IDD_NEW_LEVEL };

	void SetTerrainResolution( int res );
	void SetTerrainUnits( int units );

	CString GetLevel() const;
	int GetTerrainResolution() const;
	int GetTerrainUnits() const;
	bool IsUseTerrain() const;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedUseTerrain();
	afx_msg void OnCbnSelendokTerrainResolution();
	afx_msg void OnCbnSelendokTerraniUnits();

	void UpdateTerrainInfo();

	DECLARE_MESSAGE_MAP()

public:
	CString m_level;
	BOOL m_useTerrain;
	int m_terrainResolution;
	int m_terrainUnits;
	CStatic m_cTerrainInfo;
	CComboBox m_cTerrainResolution;
	CComboBox m_cTerrainUnits;
};

#endif // __newleveldialog_h__