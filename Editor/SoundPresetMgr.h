#pragma once

#define SOUND_PRESETS_FILENAME	"Scripts/Sounds/PresetDB.lua"

class CSoundPresetMgr
{
protected:
	XmlNodeRef m_pRootNode;
	XmlNodeRef m_pSoundTemplateNode;
protected:
	bool DumpTableRecursive(FILE *pFile, XmlNodeRef pNode, int nTabs=0);
public:
	CSoundPresetMgr();
	virtual ~CSoundPresetMgr();
	XmlNodeRef GetRootNode() { return m_pRootNode; }
	bool AddPreset(CString sName);
	bool DelPreset(CString sName);
	bool AddSound(CString sPreset);
	bool DelSound(CString sPreset, XmlNodeRef pNode);
	bool Save(CString sFilename=SOUND_PRESETS_FILENAME);
	bool Load(CString sFilename=SOUND_PRESETS_FILENAME);
	bool Reload(CString sFilename=SOUND_PRESETS_FILENAME);
	bool UpdateParameter(XmlNodeRef pNode);
	void MakeTagUnique(XmlNodeRef pParent, XmlNodeRef pNode);
};
