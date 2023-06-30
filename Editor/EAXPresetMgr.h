#pragma once

#define EAX_PRESETS_FILENAME	"Scripts/Sounds/EAXPresetDB.lua"

class CEAXPresetMgr
{
protected:
	XmlNodeRef m_pRootNode;
	XmlNodeRef m_pParamTemplateNode;
protected:
	bool DumpTableRecursive(FILE *pFile, XmlNodeRef pNode, int nTabs=0);
public:
	CEAXPresetMgr();
	virtual ~CEAXPresetMgr();
	XmlNodeRef GetRootNode() { return m_pRootNode; }
	bool AddPreset(CString sName);
	bool DelPreset(CString sName);
	bool Save(CString sFilename=EAX_PRESETS_FILENAME);
	bool Load(CString sFilename=EAX_PRESETS_FILENAME);
	bool Reload(CString sFilename=EAX_PRESETS_FILENAME);
	bool UpdateParameter(XmlNodeRef pNode);
	void MakeTagUnique(XmlNodeRef pParent, XmlNodeRef pNode);
};
