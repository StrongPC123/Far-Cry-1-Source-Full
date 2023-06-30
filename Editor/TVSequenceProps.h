////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tvsequenceprops.h
//  Version:     v1.00
//  Created:     23/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __tvsequenceprops_h__
#define __tvsequenceprops_h__
#include "afxwin.h"

#if _MSC_VER > 1000
#pragma once
#endif

struct IAnimSequence;

// CTVSequenceProps dialog

class CTVSequenceProps : public CDialog
{
	DECLARE_DYNAMIC(CTVSequenceProps)

public:
	CTVSequenceProps( IAnimSequence *seq,CWnd* pParent = NULL);   // standard constructor
	virtual ~CTVSequenceProps();

// Dialog Data
	enum { IDD = IDD_TV_SEQ_PROPS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	IAnimSequence *m_sequence;


	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRescaleTime();
	virtual void OnOK();

private:
	CEdit m_nameEdit;
	CButton m_alwaysPlayingBtn;
	CButton m_cutSceneBtn;
	CButton m_NoHUDBtn;
	CButton m_NoPlayerBtn;
	CButton m_NoPhysicsBtn;
	CButton m_NoAIBtn;
	CButton m_16To9;
	CButton m_NoSoundsBtn;
	CNumberCtrl m_startTime;	
	CNumberCtrl m_endTime;
	CNumberCtrl m_length;
	int m_outOfRange;
};

#endif // __tvsequenceprops_h__
