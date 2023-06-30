#ifndef __FILE_TREE_H__
#define __FILE_TREE_H__

#pragma once

#include "_TinyMain.h"
#include "_TinyWindow.h"
#include "_TinyFileEnum.h"
#include "_TinyImageList.h"

#include <vector>
#include <string>

#include "..\resource.h"

class CFileTree : public _TinyTreeView {
public:
	CFileTree() { m_iID = 0; };
	virtual ~CFileTree() { };

	BOOL Create(const _TinyRect& rcWnd, _TinyWindow *pParent = NULL, UINT iID = NULL) {
		m_iID = iID;
		_TinyVerify(_TinyTreeView::Create(iID, WS_CHILD | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | 
			WS_VISIBLE, WS_EX_CLIENTEDGE, &rcWnd, pParent));
		m_cTreeIcons.CreateFromBitmap(MAKEINTRESOURCE(IDB_TREE_VIEW), 16);
		SetImageList(m_cTreeIcons.GetHandle());

		return TRUE;
	};

	void ScanFiles(char *pszRootPath) {
		HTREEITEM hRoot = _TinyTreeView::AddItemToTree(pszRootPath, NULL, NULL, 2);
		EnumerateScripts(hRoot, pszRootPath);
		Expand(hRoot);
	};

	const char * GetCurItemFileName() {
		HTREEITEM hCurSel = NULL;
		LPARAM nIdx;
		hCurSel = _TinyTreeView::GetSelectedItem();
		if (!hCurSel)
			return NULL;
		nIdx = _TinyTreeView::GetItemUserData(hCurSel);
		if ((size_t) nIdx > m_vFileNameTbl.size() - 1)
			return NULL;
		return m_vFileNameTbl[nIdx].c_str();
	};

protected:
	_TinyImageList m_cTreeIcons;
	INT m_iID;
	std::vector<string> m_vFileNameTbl;

	void EnumerateScripts(HTREEITEM hRoot, char *pszEnumPath) {
		_TinyFileEnum cScripts;
		__finddata64_t sCurFile;
		char szPath[_MAX_PATH];
		char szFullFilePath[_MAX_PATH];
		HTREEITEM hItem = NULL;

		if (!cScripts.StartEnumeration(pszEnumPath, "*", &sCurFile))
			return;

		while (cScripts.GetNextFile(&sCurFile)) {
			if (_stricmp(sCurFile.name, "..") == 0)
				continue;
			if (_stricmp(sCurFile.name, ".") == 0)
				continue;

			sprintf(szFullFilePath, "%s%s", pszEnumPath, sCurFile.name);
			m_vFileNameTbl.push_back(szFullFilePath);

			if (sCurFile.attrib & _A_SUBDIR) {
				hItem = AddItemToTree(sCurFile.name, (LPARAM) m_vFileNameTbl.size() - 1, hRoot, 0);
				sprintf(szPath, "%s%s\\", pszEnumPath, sCurFile.name);
				EnumerateScripts(hItem, szPath);
			} else {
				if (strstr(_strlwr(sCurFile.name), ".lua"))
					hItem = AddItemToTree(sCurFile.name, (LPARAM) m_vFileNameTbl.size() - 1, hRoot, 1);
			}
		}
	};
};

#endif