#include "StdAfx.h"
#include <ctype.h>
#include <IScriptSystem.h>
#include <IEntitySystem.h>
#include "soundpresetmgr.h"

CSoundPresetMgr::CSoundPresetMgr()
{
	XmlNodeRef pPropNode;
	m_pSoundTemplateNode=new CXmlNode("UndefinedSound");

	pPropNode=new CXmlNode("Sound");
	pPropNode->setAttr("type", "sound");
	pPropNode->setAttr("value", "");
	m_pSoundTemplateNode->addChild(pPropNode);

	pPropNode=new CXmlNode("Centered");
	pPropNode->setAttr("type", "bool");
	pPropNode->setAttr("value", "0");
	m_pSoundTemplateNode->addChild(pPropNode);

	pPropNode=new CXmlNode("Chance");
	pPropNode->setAttr("type", "int");
	pPropNode->setAttr("value", "0");
	pPropNode->setAttr("min", "0");
	pPropNode->setAttr("max", "1000");
	m_pSoundTemplateNode->addChild(pPropNode);

	pPropNode=new CXmlNode("NoOverlap");
	pPropNode->setAttr("type", "bool");
	pPropNode->setAttr("value", "1");
	m_pSoundTemplateNode->addChild(pPropNode);

	pPropNode=new CXmlNode("Streaming");
	pPropNode->setAttr("type", "bool");
	pPropNode->setAttr("value", "0");
	m_pSoundTemplateNode->addChild(pPropNode);

	pPropNode=new CXmlNode("Timeout");
	pPropNode->setAttr("type", "float");
	pPropNode->setAttr("value", "0");
	pPropNode->setAttr("min", "0");
	pPropNode->setAttr("max", "1000");
	m_pSoundTemplateNode->addChild(pPropNode);

	pPropNode=new CXmlNode("Volume");
	pPropNode->setAttr("type", "int");
	pPropNode->setAttr("value", "255");
	pPropNode->setAttr("min", "0");
	pPropNode->setAttr("max", "255");
	m_pSoundTemplateNode->addChild(pPropNode);
/*
	pPropNode=new CXmlNode("AreaID");
	pPropNode->setAttr("type", "int");
	pPropNode->setAttr("value", "-1");
	pPropNode->setAttr("min", "-1");
	m_pSoundTemplateNode->addChild(pPropNode);
*/
	m_pRootNode=new CXmlNode("SoundPresetDB");
}

CSoundPresetMgr::~CSoundPresetMgr()
{
}

bool CSoundPresetMgr::AddPreset(CString sName)
{
	if (sName.GetLength()<=0)
		return false;
	// check if name is valid
	for (int i=0;i<sName.GetLength();i++)
	{
		char c=sName[i];
		if (!i)
		{
			if (!isalpha(c))
				return false;
		}else
		{
			if (!isalnum(c))
				return false;
		}
	}
	if (m_pRootNode->findChild(sName))
		return false;
	XmlNodeRef pPresetNode=new CXmlNode(sName);
	m_pRootNode->addChild(pPresetNode);
	return true;
}

bool CSoundPresetMgr::DelPreset(CString sName)
{
	XmlNodeRef pPresetNode=m_pRootNode->findChild(sName);
	if (!pPresetNode)
		return false;
	m_pRootNode->removeChild(pPresetNode);
	Reload();
	return true;
}

bool CSoundPresetMgr::AddSound(CString sPreset)
{
	XmlNodeRef pPresetNode=m_pRootNode->findChild(sPreset);
	if (!pPresetNode)
		return false;
	XmlNodeRef pSoundNode=m_pSoundTemplateNode->clone();
	MakeTagUnique(pPresetNode, pSoundNode);
	pPresetNode->addChild(pSoundNode);
	return true;
}

bool CSoundPresetMgr::DelSound(CString sPreset, XmlNodeRef pNode)
{
	XmlNodeRef pPresetNode=m_pRootNode->findChild(sPreset);
	if (!pPresetNode)
		return false;
	while (pNode->getParent()!=pPresetNode)
	{
		pNode=pNode->getParent();
		if (!pNode)
			return false;
	}
	pPresetNode->removeChild(pNode);
	Reload();
	return true;
}

void CSoundPresetMgr::MakeTagUnique(XmlNodeRef pParent, XmlNodeRef pNode)
{
	CString NodeTag=pNode->getTag();
	// make name valid
	for (int i=0;i<NodeTag.GetLength();i++)
	{
		char c=NodeTag[i];
		if (!i)
		{
			if (!isalpha(c))
				c='_';
		}else
		{
			if (!isalnum(c))
				c='_';
		}
		NodeTag.SetAt(i, c);
	}
	int nCounter=2;
	char sCounter[5];
	bool bUnique=false;
	XmlNodeRef pCheckNode;
	do {
		pCheckNode=pParent->findChild(NodeTag);
		if ((pCheckNode!=NULL) && (pCheckNode!=pNode))
		{
			sprintf(sCounter, "%d", nCounter);
			NodeTag=CString(pNode->getTag())+sCounter;
			nCounter++;
		}else
			bUnique=true;
	}while(!bUnique);
	pNode->setTag(NodeTag);
}

bool CSoundPresetMgr::Reload(CString sFilename)
{
	if (!Save() || !Load())
	{
		AfxMessageBox("An error occured while reloading sound-presets. Is Database read-only ?", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}
	return true;
}

bool CSoundPresetMgr::DumpTableRecursive(FILE *pFile, XmlNodeRef pNode, int nTabs)
{
	for (int i=0;i<nTabs;i++) if (fprintf(pFile, "\t")<=0) return false;
	CString sValue;
	if (pNode->getAttr("value", sValue))
	{
		for (int i=0;i<sValue.GetLength();i++)
		{
			if (sValue[i]=='\\')
				sValue.SetAt(i, '/');
		}
		if (fprintf(pFile, "%s = \"%s\"", pNode->getTag(), (const char*)sValue)<=0) return false;
	}else
	{
		if (fprintf(pFile, "%s = {\r\n", pNode->getTag())<=0) return false;
		for (int i=0;i<pNode->getChildCount();i++)
		{
			XmlNodeRef pChild=pNode->getChild(i);
			if (!DumpTableRecursive(pFile, pChild, nTabs+1)) return false;
		}
		for (int i=0;i<nTabs;i++) if (fprintf(pFile, "\t")<=0) return false;
		if (fprintf(pFile, "}")<=0) return false;
	}
	if (nTabs)
		if (fprintf(pFile, ",\r\n")<=0) return false;
	return true;
}

bool CSoundPresetMgr::Save(CString sFilename)
{
	FILE *pFile=fopen(sFilename, "wb");
	if (!pFile)
		return false;
	bool bRes=true;
	bRes=DumpTableRecursive(pFile, m_pRootNode);
	fclose(pFile);
	return bRes;
}

class CSoundPresetSODump : public IScriptObjectDumpSink
{
private:
	IScriptSystem *m_pScriptSystem;
	_SmartScriptObject m_pObj;
	XmlNodeRef m_pNode;
	XmlNodeRef m_pSoundTemplateNode;
	int m_nLevel;
public:
	CSoundPresetSODump(IScriptSystem *pScriptSystem, _SmartScriptObject &pObj, XmlNodeRef pNode, XmlNodeRef pSoundTemplateNode, int nLevel=0) : m_pObj(pScriptSystem,pObj)
	{
		m_pScriptSystem=pScriptSystem;
		m_pNode=pNode;
		m_pSoundTemplateNode=pSoundTemplateNode;
		m_nLevel=nLevel;
	}
private:
	void OnElementFound(const char *sName, ScriptVarType type)
	{
		switch (type)
		{
			case svtObject:
			{
				_SmartScriptObject pObj(m_pScriptSystem, true);
				m_pObj->GetValue(sName, pObj);
				XmlNodeRef pNode;
				if (!m_nLevel)
				{
					pNode=new CXmlNode(sName);
				}else
				{
					pNode=m_pSoundTemplateNode->clone();
					pNode->setTag(sName);
				}
				m_pNode->addChild(pNode);
				CSoundPresetSODump Sink(m_pScriptSystem, pObj, pNode, m_pSoundTemplateNode, m_nLevel+1);
				pObj->Dump(&Sink);
				break;
			}
			case svtString:
			case svtNumber:
			{
				const char *pszValue;
				m_pObj->GetValue(sName, pszValue);
				XmlNodeRef pNode=m_pNode->findChild(sName);
				//ASSERT(pNode!=NULL);
				if (pNode)
					pNode->setAttr("value", pszValue);
			}
		}
	}
	void OnElementFound(int nIdx, ScriptVarType type)
	{
	}
};

bool CSoundPresetMgr::Load(CString sFilename)
{
	ISystem *pSystem=GetIEditor()->GetSystem();
	if (!pSystem)
		return false;
	IScriptSystem *pScriptSystem=pSystem->GetIScriptSystem();
	if (!pScriptSystem)
		return false;
	pScriptSystem->SetGlobalToNull("SoundPresetDB");
	pScriptSystem->ExecuteFile(sFilename, true, true);
	_SmartScriptObject pObj(pScriptSystem, true);
	if (!pScriptSystem->GetGlobalValue("SoundPresetDB", pObj))
		return false;
	m_pRootNode=new CXmlNode("SoundPresetDB");
	CSoundPresetSODump Sink(pScriptSystem, pObj, m_pRootNode, m_pSoundTemplateNode);
	pObj->Dump(&Sink);
	return true;
}

class CSoundPresetUpdateSODump : public IScriptObjectDumpSink
{
private:
	IScriptSystem *m_pScriptSystem;
	_SmartScriptObject m_pObj;
	CString m_sPresetName;
	CString m_sSoundName;
	CString m_sPropName;
	CString m_sPropValue;
	bool m_bCorrectPreset;
	bool m_bCorrectSound;
public:
	CSoundPresetUpdateSODump(IScriptSystem *pScriptSystem, _SmartScriptObject &pObj, CString &sPresetName, CString &sSoundName, CString &sPropName, CString &sPropValue, bool bCorrectPreset=false, bool bCorrectSound=false) : m_pObj(pScriptSystem, pObj)
	{
		m_pScriptSystem=pScriptSystem;
		m_sPresetName=sPresetName;
		m_sSoundName=sSoundName;
		m_sPropName=sPropName;
		m_sPropValue=sPropValue;
		m_bCorrectPreset=bCorrectPreset;
		m_bCorrectSound=bCorrectSound;
	}
private:
	void OnElementFound(const char *sName, ScriptVarType type)
	{
		switch (type)
		{
			case svtObject:
			{
				_SmartScriptObject pObj(m_pScriptSystem, true);
				m_pObj->GetValue(sName, pObj);
				if (m_bCorrectPreset && (strcmp(sName, m_sSoundName)==0))
				{
					CSoundPresetUpdateSODump Sink(m_pScriptSystem, pObj, m_sPresetName, m_sSoundName, m_sPropName, m_sPropValue, true, true);
					pObj->Dump(&Sink);
				}else
				{
					if (strcmp(sName, m_sPresetName)==0)
					{
						CSoundPresetUpdateSODump Sink(m_pScriptSystem, pObj, m_sPresetName, m_sSoundName, m_sPropName, m_sPropValue, true);
						pObj->Dump(&Sink);
					}else
					{
						CSoundPresetUpdateSODump Sink(m_pScriptSystem, pObj, m_sPresetName, m_sSoundName, m_sPropName, m_sPropValue, false);
						pObj->Dump(&Sink);
					}
				}
				break;
			}
			case svtString:
			case svtNumber:
			{
				if (m_bCorrectPreset && m_bCorrectSound && strcmp(sName, m_sPropName)==0)
				{
					for (int i=0;i<m_sPropValue.GetLength();i++)
					{
						if (m_sPropValue[i]=='\\')
							m_sPropValue.SetAt(i, '/');
					}
					m_pObj->SetValue(sName, m_sPropValue);
				}
			}
		}
	}
	void OnElementFound(int nIdx, ScriptVarType type)
	{
	}
};

bool CSoundPresetMgr::UpdateParameter(XmlNodeRef pNode)
{
	CString sPresetName;
	CString sSoundName;
	CString sPropName;
	CString sPropValue;
	XmlNodeRef pParent=pNode->getParent();
	if (!pParent)
		return false;
	if (pParent->getParent())
		sPresetName=pParent->getParent()->getTag();
	else
		sPresetName="";
	sSoundName=pParent->getTag();
	sPropName=pNode->getTag();
	if (!pNode->getAttr("value", sPropValue))
		return false;
	IScriptSystem *pScriptSystem=GetIEditor()->GetSystem()->GetIScriptSystem();
	_SmartScriptObject pObj(pScriptSystem, true);
	if (!pScriptSystem->GetGlobalValue("SoundPresetDB", pObj))
		return false;
	CSoundPresetUpdateSODump Sink(pScriptSystem, pObj, sPresetName, sSoundName, sPropName, sPropValue);
	pObj->Dump(&Sink);
	// lets call OnPropertyChange on every entity-script which uses presets
	IEntitySystem *pEntSys=GetIEditor()->GetSystem()->GetIEntitySystem();
	if (pEntSys)
	{
		IEntityIt *pEntIt=pEntSys->GetEntityIterator();
		if (pEntIt)
		{
			pEntIt->MoveFirst();
			IEntity *pEnt;
			while (pEnt=pEntIt->Next())
			{
				IScriptObject *pScriptObject=pEnt->GetScriptObject();
				if (pScriptObject)
				{
					_SmartScriptObject pObj(pScriptSystem, true);
					if (pScriptObject->GetValue("Properties", pObj))
					{
						const char *pszPreset;
						if (pObj->GetValue("sndpresetSoundPreset", pszPreset))
						{
							if (strcmp(pszPreset, sPresetName)==0)
							{
								pScriptSystem->BeginCall(pEnt->GetEntityClassName(),"OnPropertyChange");
								pScriptSystem->PushFuncParam(pScriptObject);
								pScriptSystem->EndCall();
							}
						}
					}
				}
			}
		}
	}
	return true;
}