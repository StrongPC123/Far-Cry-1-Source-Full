////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   customfiledialog.h
//  Version:     v1.00
//  Created:     3/9/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Custom Dialog for opening files.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __customfiledialog_h__
#define __customfiledialog_h__
#pragma once

#include "XTToolkit.h"
#include "Controls\CustomComboBoxEx.h"
#include "Controls\PreviewModelCtrl.h"
#include "Util\FileUtil.h"

// forward declaration.
struct ISound;

//////////////////////////////////////////////////////////////////////////
class CFilesListCtrl : public CXTListCtrl
{
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg UINT OnGetDlgCode();

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////
/*! CCustomFileDialog Custom file selection dialog.
*/
class CCustomFileDialog : public CXTResizeDialog
{
	DECLARE_DYNAMIC(CCustomFileDialog)

public:
	/*! FileOpenDialogParams structure specify parameters for CutsomFileDialog.
	*/
	struct OpenParams
	{
		ECustomFileType filetype; //! Type of file to open.
		CString filter;			//! Filter for opening files.
		CString initialDir; //! Directory to start in, leave empty for default.
		CString initialFile; //! Select initial file.
		bool bMultiSelection;//! Allow to select miltiple files.

		OpenParams()
		{
			filetype = EFILE_TYPE_ANY;
			bMultiSelection = false;
		}
	};

	CCustomFileDialog( OpenParams &fp,CWnd* pParent = NULL);   // standard constructor
	virtual ~CCustomFileDialog();

	// Get selected filename (Not include path);
	CString GetFileName() const;
	CString GetFilePath() const;

	// Get number of selected files.
	int GetSelectedCount() const;
	CString GetSelectedFile( int nIndex ) const;

// Dialog Data
	enum { IDD = IDD_FILE_BROWSER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	//////////////////////////////////////////////////////////////////////////
	// METHODS.
	//////////////////////////////////////////////////////////////////////////
	void FillFiles();
	bool ScanDirectory( const CString &dir );
	void AddFilteredFiles( const CString &searchFilter );

	void SetCurrentDir( const CString &dir,bool bRememberPrevious=true );
	void SetCurrentFile( const CString &file );
	void SelectItem( int item );
	void RefreshDirectory();
	void ParseFilter( const CString &filter );
	void ActivateItem( int );
	void PreviewItem( int item );
	void PreviewOff();
	void LoadHistory();
	void SaveHistory();
	void FillLookinControl();
	void InitShortcuts();

	//////////////////////////////////////////////////////////////////////////
	// MESSAGE HANDLERS.
	//////////////////////////////////////////////////////////////////////////
	afx_msg void OnBnClickedBack();
	afx_msg void OnNMDblclkFilelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemChangedFilelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedUp();
	afx_msg void OnCbnSelendokHistory();
	afx_msg void OnCbnSelendokFilestype();
	afx_msg void OnCbnSelendokFilename();
	afx_msg void OnLvnKeydownFilelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelendokLookin();
	afx_msg void OnCbenEndeditFilename(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnOutbarNotify( WPARAM lParam, LPARAM wParam);
	afx_msg void OnBnClickedPreviewOn();


	//////////////////////////////////////////////////////////////////////////
	// CONTROLS.
	//////////////////////////////////////////////////////////////////////////
	CComboBox m_historyCtrl;
	CXTComboBoxEx m_lookinCtrl;
	CFilesListCtrl m_fileListCtrl;
	CXTOutlookBar m_shortcutsCtrl;
	CCustomComboBoxEx<CXTComboBoxEx> m_filenameCtrl;
	CComboBox m_filesTypeCtrl;
	CCustomButton m_btnBack;
	CCustomButton m_btnUp;
	CStatic m_fileInfoCtrl;
	CImageList m_imageListFiles;
	CButton m_previewOn;

	//////////////////////////////////////////////////////////////////////////
	// Preview
	//////////////////////////////////////////////////////////////////////////
	
	CPreviewModelCtrl m_previewCtrl;
	CStatic m_previewImageCtrl;
	CBitmap m_previewBitmap;
	CRect m_rcPreview;
	_smart_ptr<ISound> m_pSound;
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// VARIABLES.
	//////////////////////////////////////////////////////////////////////////
	OpenParams &m_fp;
	ICryPak *m_pIPak;

	// Current directory description.
	struct FileInfo
	{
		CString filename;
		unsigned attrib;
		time_t time_create;    /* -1 for FAT file systems */
		time_t time_access;    /* -1 for FAT file systems */
		time_t time_write;
		_fsize_t size;
	};
	static bool FileInfoCompare( const FileInfo &f1,const FileInfo &f2 );

	std::vector<FileInfo> m_files;
	CString m_currentDir;
	CString m_currentFile;

	// List of multiple selected files.
	std::vector<CString> m_selectedFiles;

	//////////////////////////////////////////////////////////////////////////
	// File Filter.
	//////////////////////////////////////////////////////////////////////////
	struct FileFilter
	{
		CString description;
		std::vector<CString> filters; // Actual array of filters.
	};
	std::vector<FileFilter> m_fileFilters;
	// Current filter.
	int m_currentFilter;
	CString m_overrideFilter;

	std::vector<CString> m_directoryStack;

	//////////////////////////////////////////////////////////////////////////
	// Shortcuts.
	//////////////////////////////////////////////////////////////////////////
	struct ShortcutInfo
	{
		CString directory;
		CString name;
		UINT nIconID;
		ShortcutInfo() { nIconID = 0; };
		ShortcutInfo( const char *sName,const char *sDir,UINT nIcon )
		{
			name = sName;
			directory = sDir;
			nIconID = nIcon;
		}
	};
	typedef std::vector<ShortcutInfo> Shortcuts;
	Shortcuts m_shortcuts[EFILE_TYPE_LAST];

	//////////////////////////////////////////////////////////////////////////
	// Maps extenstions to icons.
	std::map<CString,UINT> m_extToIcons;

	//////////////////////////////////////////////////////////////////////////
	// History.
	//////////////////////////////////////////////////////////////////////////
	static bool m_bPreviewOn;
};

#endif // __customfiledialog_h__