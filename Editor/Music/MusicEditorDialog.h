////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   MaterialDialog.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __musiceditordialog_h__
#define __musiceditordialog_h__
#pragma once

#include "BaseLibraryDialog.h"
#include "Controls\SplitterWndEx.h"
#include "Controls\PropertyCtrl.h"

class CMusicThemeLibItem;
class CMusicManager;

// forward declartions of DynamicMusic structures.
struct SMusicTheme;
struct SMusicPatternSet;
struct SPatternDef;
struct SMusicMood;
struct SMusicData;
struct ISound;

/** Dialog which hosts entity prototype library.
*/
class CMusicEditorDialog : public CBaseLibraryDialog
{
	DECLARE_DYNAMIC(CMusicEditorDialog)
public:
	CMusicEditorDialog( CWnd *pParent );
	~CMusicEditorDialog();

	// Called every frame.
	void Update();

	virtual UINT GetDialogMenuID();


protected:
	enum ETreeItemType
	{
		EITEM_THEME,
		EITEM_MOOD,
		EITEM_PATTERN_SET,
		EITEM_LAYER,
		EITEM_PATTERN,
	};
	enum ELayerType
	{
		ELAYER_MAIN = 0,
		ELAYER_RHYTHMIC = 1,
		ELAYER_INCIDENTAL = 2,
	};
	//! Description of item in tree control.
	struct ItemDesc
	{
		ItemDesc *pParentItem;
		ETreeItemType type;
		ELayerType layerType;
		CMusicThemeLibItem *pItem;
		SMusicTheme *m_pTheme;
		SMusicMood *m_pMood;
		SMusicPatternSet *m_pPatternSet;
		SPatternDef *m_pPattern;
		//////////////////////////////////////////////////////////////////////////
		ItemDesc() : pParentItem(0),type(EITEM_THEME),layerType(ELAYER_MAIN),pItem(0),m_pTheme(0),
			m_pMood(0),m_pPatternSet(0),m_pPattern(0) {};
		ItemDesc( ItemDesc &item,ELayerType lt ) : pParentItem(&item),type(EITEM_LAYER),layerType(lt),pItem(item.pItem),m_pTheme(item.m_pTheme),
			m_pMood(item.m_pMood),m_pPatternSet(item.m_pPatternSet),m_pPattern(item.m_pPattern) {};
		ItemDesc( ItemDesc &item ) : pParentItem(&item),type(item.type),layerType(item.layerType),pItem(item.pItem),m_pTheme(item.m_pTheme),
			m_pMood(item.m_pMood),m_pPatternSet(item.m_pPatternSet),m_pPattern(item.m_pPattern) {};
	};

	void DoDataExchange(CDataExchange* pDX);
	BOOL OnInitDialog();
	
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	
	afx_msg void OnPlay();
	afx_msg void OnUpdatePlay( CCmdUI* pCmdUI );
	afx_msg void OnStop();
	afx_msg void OnLoadFromLua();

	afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNotifyTreeRClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNotifyTreeDblClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCopy();
	afx_msg void OnPaste();
	afx_msg void OnSelChangedItemTree(NMHDR* pNMHDR, LRESULT* pResult);

	// File tree
	afx_msg void OnNotifyFileTreeClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectFileTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFilesBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileTreeKillFocus(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnAddItem();
	afx_msg void OnAddSubItem();
	virtual afx_msg void OnRemoveItem();
	virtual afx_msg void OnRenameItem();
	virtual afx_msg void OnLoadLibrary();

	//////////////////////////////////////////////////////////////////////////
	// Some functions can be overriden to modify standart functionality.
	//////////////////////////////////////////////////////////////////////////
	virtual void InitToolbar();
	virtual HTREEITEM InsertItemToTree( CBaseLibraryItem *pItem,HTREEITEM hParent );
	virtual void SelectItem( CBaseLibraryItem *item,bool bForceReload=false );
	virtual void SetActive( bool bActive );

	//////////////////////////////////////////////////////////////////////////
	void OnUpdateProperties( IVariable *var );
	void DropToItem( HTREEITEM hItem,HTREEITEM hSrcItem );

	HTREEITEM InsertItem_Theme( CMusicThemeLibItem *pItem );
	HTREEITEM InsertItem_Mood( ItemDesc *pParent,SMusicMood *pMood,HTREEITEM parentItem=0 );
	HTREEITEM InsertItem_PatternSet( ItemDesc *pParent,SMusicPatternSet *pPatternSet,HTREEITEM parentItem=0 );
	HTREEITEM InsertItem_Pattern( ItemDesc *pParent,SPatternDef *pPattern,HTREEITEM parentItem=0 );
	ItemDesc* GetCurrentItem();

	virtual void ReloadItems();
	void SelectItem( ItemDesc *pItemDesc );
	void SetModified();

	//////////////////////////////////////////////////////////////////////////
	// Adding
	//////////////////////////////////////////////////////////////////////////
	SMusicTheme* NewTheme( const char *sBaseName=NULL );
	SMusicMood* NewMood( SMusicTheme *pTheme,const char *sBaseName=NULL,bool bNewPatternSet=true );
	SMusicPatternSet* NewPatternSet( SMusicMood *pMood );
	SPatternDef* NewPattern( SMusicPatternSet *pPatternSet,ELayerType layer,const char *sBaseName=NULL,XmlNodeRef &node=XmlNodeRef(0) );
	CString MakeUniqPatternName( const CString &baseName );
	CString MakeUniqThemeName( const CString &baseName );
	CString MakeUniqMoodName( SMusicTheme *pTheme,const CString &baseName );
	void CalcProbabilities( SMusicPatternSet *pPatternSet );
	void PastePatternSet( SMusicMood *pMood,XmlNodeRef &node );
	SMusicMood* PasteMood( SMusicTheme *pTheme,XmlNodeRef &node );
	void PasteTheme( XmlNodeRef &node );

	//////////////////////////////////////////////////////////////////////////
	// Deleting
	//////////////////////////////////////////////////////////////////////////
	void DeletePattern( ItemDesc *pItemDesc );
	void DeletePatternSet( ItemDesc *pItemDesc );
	void DeleteMood( ItemDesc *pItemDesc );
	void DeleteTheme( ItemDesc *pItemDesc );

	//////////////////////////////////////////////////////////////////////////
	// IDocListener listener implementation
	//////////////////////////////////////////////////////////////////////////
	virtual	void OnNewDocument();
	virtual	void OnLoadDocument();
	virtual void OnCloseDocument();
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Music files tree.
	//////////////////////////////////////////////////////////////////////////
	void OnRealodMusicFiles();
	void LoadMusicFiles( std::vector<CString> &musicFiles );
	void FillFileTree();
	void PlaySound( const CString &filename );
	//////////////////////////////////////////////////////////////////////////

	DECLARE_MESSAGE_MAP()

	//////////////////////////////////////////////////////////////////////////
	// Variables.
	//////////////////////////////////////////////////////////////////////////
	
	IMusicSystem *m_pMusicSystem;

	CSplitterWndEx m_wndVSplitter;
	CSplitterWndEx m_wndHSplitter;
	
	CTreeCtrl m_filesTreeCtrl;
	CPropertyCtrl m_propsCtrl;
	CVarBlockPtr m_vars;

	CImageList m_imageList;
	CImageList m_filesImageList;

	// Material manager.
	CMusicManager *m_pMusicManager;
	SMusicData *m_pMusicData;

	bool m_bPlaying;
	bool m_bHandleDeleteItem;

	HTREEITEM m_hDropItem;
	HTREEITEM m_hDraggedItem;

	// All music files.
	CString m_musicFilesPath;
	std::map<HTREEITEM,CString> m_musicFilesMap;
	_smart_ptr<ISound> m_pSound;

	//////////////////////////////////////////////////////////////////////////
	// Maps to Tree items.
	//////////////////////////////////////////////////////////////////////////
	std::map<SMusicTheme*,HTREEITEM> m_ThemeToItemMap;
	std::map<SMusicMood*,HTREEITEM> m_MoodToItemMap;
	std::map<SMusicPatternSet*,HTREEITEM> m_PatternSetToItemMap;
	std::map<SPatternDef*,HTREEITEM> m_PatternToItemMap;
	typedef std::map<CString,SPatternDef*,stl::less_stricmp<CString> > PatternsMap;
	PatternsMap m_PatternsMap;

	ETreeItemType m_lastItemType;
};

#endif // __musiceditordialog_h__

/*

[-] Theme files.
	[-] Moods { Bridges }
		[-] Sneaking
			[-] PatternSet0 { MinTimeout=50, MaxTimeout=60 }
				[-] MainLayer { Probability = 100 }
					-- Pattern0 { File,Volume,FadePos,Probability=50 }
					-- Pattern1 { File,Volume,FadePos,Probability=50 }
					-- Pattern2 { File,Volume,FadePos,Probability=50 }
					-- Pattern3 { File,Volume,FadePos,Probability=50 }
				[-] RhythmicLayer { Probability = 100 }
					-- Pattern0 { File,Volume,FadePos,Probability=50 }
					-- Pattern1 { File,Volume,FadePos,Probability=50 }
					-- Pattern2 { File,Volume,FadePos,Probability=50 }
					-- Pattern3 { File,Volume,FadePos,Probability=50 }
				[-] IncidentalLayer { Probability = 100 }
					-- Pattern0 { File,Volume,FadePos,Probability=50 }
					-- Pattern1 { File,Volume,FadePos,Probability=50 }
					-- Pattern2 { File,Volume,FadePos,Probability=50 }
					-- Pattern3 { File,Volume,FadePos,Probability=50 }
				[+] PatternSet1 { MinTimeout=50, MaxTimeout=60 }
				[+] PatternSet2
		[+] Suspense
		[+] Nearsuspense
		[+] Alert
		[+] Victory


 */