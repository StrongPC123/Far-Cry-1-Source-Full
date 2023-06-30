#include "StdAfx.h"
#include <ctype.h>
#include <IScriptSystem.h>
#include <IEntitySystem.h>
#include "eaxpresetmgr.h"

#define ADDNODE(_type, _name, _min, _max, _value) \
	pPropNode=new CXmlNode(#_name); \
	pPropNode->setAttr("type", #_type); \
	pPropNode->setAttr("min", #_min); \
	pPropNode->setAttr("max", #_max); \
	pPropNode->setAttr("value", #_value); \
	m_pParamTemplateNode->addChild(pPropNode);

CEAXPresetMgr::CEAXPresetMgr()
{
	XmlNodeRef pPropNode;
	m_pParamTemplateNode=new CXmlNode("Parameters");
	ADDNODE(int, nEnvironment, 0, 25, 0);	// sets all listener properties (win32/ps2 only)
	ADDNODE(float, fEnvSize, 1.0, 100.0, 7.5);	// environment size in meters (win32 only)
	ADDNODE(float, fEnvDiffusion, 0.0, 1.0, 1.0);	// environment diffusion (win32/xbox)
	ADDNODE(int, nRoom, -10000, 0, -1000);	// room effect level (at mid frequencies) (win32/xbox/ps2)
	ADDNODE(int, nRoomHF, -10000, 0, -100);	// relative room effect level at high frequencies (win32/xbox)
	ADDNODE(int, nRoomLF, -10000, 0, 0);	// relative room effect level at low frequencies (win32 only)
	ADDNODE(float, fDecayTime, 0.1, 20.0, 1.49);	// reverberation decay time at mid frequencies (win32/xbox)
	ADDNODE(float, fDecayHFRatio, 0.1, 2.0, 0.83);	// high-frequency to mid-frequency decay time ratio (win32/xbox)
	ADDNODE(float, fDecayLFRatio, 0.1, 2.0, 1.0);	// low-frequency to mid-frequency decay time ratio (win32 only)
	ADDNODE(int, nReflections, -10000, 1000, -2602);	// early reflections level relative to room effect (win32/xbox)
	ADDNODE(float, fReflectionsDelay, 0.0, 0.3, 0.007);	// initial reflection delay time (win32/xbox)
	{	// early reflections panning vector (win32 only)
		XmlNodeRef pNode=new CXmlNode("fReflectionsPan");
		pPropNode=new CXmlNode("x"); pPropNode->setAttr("type", "float"); pPropNode->setAttr("value", "0");
		pNode->addChild(pPropNode);
		pPropNode=new CXmlNode("y"); pPropNode->setAttr("type", "float"); pPropNode->setAttr("value", "0");
		pNode->addChild(pPropNode);
		pPropNode=new CXmlNode("z"); pPropNode->setAttr("type", "float"); pPropNode->setAttr("value", "0");
		pNode->addChild(pPropNode);
		m_pParamTemplateNode->addChild(pNode);
	}
	ADDNODE(int, nReverb, -10000, 2000, 200);	// late reverberation level relative to room effect (win32/xbox)
	ADDNODE(float, fReverbDelay, 0.0, 0.1, 0.011);	// late reverberation delay time relative to initial reflection (win32/xbox)
	{	// late reverberation panning vector (win32 only)
		XmlNodeRef pNode=new CXmlNode("fReverbPan");
		pPropNode=new CXmlNode("x"); pPropNode->setAttr("type", "float"); pPropNode->setAttr("value", "0");
		pNode->addChild(pPropNode);
		pPropNode=new CXmlNode("y"); pPropNode->setAttr("type", "float"); pPropNode->setAttr("value", "0");
		pNode->addChild(pPropNode);
		pPropNode=new CXmlNode("z"); pPropNode->setAttr("type", "float"); pPropNode->setAttr("value", "0");
		pNode->addChild(pPropNode);
		m_pParamTemplateNode->addChild(pNode);
	}
	ADDNODE(float, fEchoTime, 0.075, 0.25, 0.25);	// echo time (win32 only)
	ADDNODE(float, fEchoDepth, 0.0, 1.0, 0.0);	// echo depth (win32 only)
	ADDNODE(float, fModulationTime, 0.04, 4.0, 0.25);	// modulation time (win32 only)
	ADDNODE(float, fModulationDepth, 0.0, 1.0, 0.0);	// modulation depth (win32 only)
	ADDNODE(float, fAirAbsorptionHF, -100, 0.0, -5.0);	// change in level per meter at high frequencies (win32 only)
	ADDNODE(float, fHFReference, 1000.0, 20000, 5000.0);	// reference high frequency (hz) (win32/xbox)
	ADDNODE(float, fLFReference, 20.0, 1000.0, 250.0);	// reference low frequency (hz) (win32 only)
	ADDNODE(float, fRoomRolloffFactor, 0.0, 10.0, 0.0);	// like CS_3D_SetRolloffFactor but for room effect (win32/xbox)
	ADDNODE(float, fDiffusion, 0.0, 100.0, 100.0);	// Value that controls the echo density in the late reverberation decay. (xbox only)
	ADDNODE(float, fDensity, 0.0, 100.0, 100.0);	// Value that controls the modal density in the late reverberation decay (xbox only)
	ADDNODE(int, nFlags, 0, 100000, 0);
	m_pRootNode=new CXmlNode("EAXPresetDB");
}

CEAXPresetMgr::~CEAXPresetMgr()
{
}

bool CEAXPresetMgr::AddPreset(CString sName)
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
	XmlNodeRef pPresetNode=m_pParamTemplateNode->clone();//new CXmlNode(sName);
	pPresetNode->setTag(sName);
//	pPresetNode->addChild(m_pParamTemplateNode->clone());
	m_pRootNode->addChild(pPresetNode);
	return true;
}

bool CEAXPresetMgr::DelPreset(CString sName)
{
	XmlNodeRef pPresetNode=m_pRootNode->findChild(sName);
	if (!pPresetNode)
		return false;
	m_pRootNode->removeChild(pPresetNode);
	Reload();
	return true;
}

bool CEAXPresetMgr::Reload(CString sFilename)
{
	if (!Save() || !Load())
	{
		AfxMessageBox("An error occured while reloading sound-presets. Is Database read-only ?", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}
	return true;
}

bool CEAXPresetMgr::DumpTableRecursive(FILE *pFile, XmlNodeRef pNode, int nTabs)
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

bool CEAXPresetMgr::Save(CString sFilename)
{
	FILE *pFile=fopen(sFilename, "wb");
	if (!pFile)
		return false;
	bool bRes=true;
	bRes=DumpTableRecursive(pFile, m_pRootNode);
	fclose(pFile);
	return bRes;
}

class CEAXPresetSODump : public IScriptObjectDumpSink
{
private:
	IScriptSystem *m_pScriptSystem;
	_SmartScriptObject m_pObj;
	XmlNodeRef m_pNode;
	XmlNodeRef m_pParamTemplateNode;
	int m_nLevel;
public:
	CEAXPresetSODump(IScriptSystem *pScriptSystem, _SmartScriptObject &pObj, XmlNodeRef pNode, XmlNodeRef pParamTemplateNode, int nLevel=0) : m_pObj(pScriptSystem,pObj)
	{
		m_pScriptSystem=pScriptSystem;
		m_pNode=pNode;
		m_pParamTemplateNode=pParamTemplateNode;
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
				if ((strcmp(sName, "fReflectionsPan")==0) || (strcmp(sName, "fReverbPan")==0))
				{
					// vector
					XmlNodeRef pNode=m_pNode->findChild(sName);
					if (pNode)
					{
						CEAXPresetSODump Sink(m_pScriptSystem, pObj, pNode, m_pParamTemplateNode, m_nLevel+1);
						pObj->Dump(&Sink);
					}
				}else
				{
					XmlNodeRef pNode=m_pParamTemplateNode->clone();
					pNode->setTag(sName);
					m_pNode->addChild(pNode);
					CEAXPresetSODump Sink(m_pScriptSystem, pObj, pNode, m_pParamTemplateNode, m_nLevel+1);
					pObj->Dump(&Sink);
				}
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

bool CEAXPresetMgr::Load(CString sFilename)
{
	ISystem *pSystem=GetIEditor()->GetSystem();
	if (!pSystem)
		return false;
	IScriptSystem *pScriptSystem=pSystem->GetIScriptSystem();
	if (!pScriptSystem)
		return false;
	pScriptSystem->SetGlobalToNull("EAXPresetDB");
	pScriptSystem->ExecuteFile(sFilename, true, true);
	_SmartScriptObject pObj(pScriptSystem, true);
	if (!pScriptSystem->GetGlobalValue("EAXPresetDB", pObj))
		return false;
	m_pRootNode=new CXmlNode("EAXPresetDB");
	CEAXPresetSODump Sink(pScriptSystem, pObj, m_pRootNode, m_pParamTemplateNode);
	pObj->Dump(&Sink);
	return true;
}

class CEAXPresetUpdateSODump : public IScriptObjectDumpSink
{
private:
	IScriptSystem *m_pScriptSystem;
	_SmartScriptObject m_pObj;
	CString m_sPresetName;
	CString m_sPropName;
	CString m_sPropValue;
	bool m_bCorrectPreset;
public:
	CEAXPresetUpdateSODump(IScriptSystem *pScriptSystem, _SmartScriptObject &pObj, CString &sPresetName, CString &sPropName, CString &sPropValue, bool bCorrectPreset=false) : m_pObj(pScriptSystem, pObj)
	{
		m_pScriptSystem=pScriptSystem;
		m_sPresetName=sPresetName;
		m_sPropName=sPropName;
		m_sPropValue=sPropValue;
		m_bCorrectPreset=bCorrectPreset;
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
				if (strcmp(sName, m_sPresetName)==0)
				{
					CEAXPresetUpdateSODump Sink(m_pScriptSystem, pObj, m_sPresetName, m_sPropName, m_sPropValue, true);
					pObj->Dump(&Sink);
				}else
				{
					CEAXPresetUpdateSODump Sink(m_pScriptSystem, pObj, m_sPresetName, m_sPropName, m_sPropValue, false);
					pObj->Dump(&Sink);
				}
				break;
			}
			case svtString:
			case svtNumber:
			{
				if (m_bCorrectPreset && strcmp(sName, m_sPropName)==0)
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

bool CEAXPresetMgr::UpdateParameter(XmlNodeRef pNode)
{
	CString sPresetName;
	CString sSoundName;
	CString sPropName;
	CString sPropValue;
	XmlNodeRef pParent=pNode->getParent();
	if (!pParent)
		return false;
	sPresetName=pParent->getTag();
	sPropName=pNode->getTag();
	if (!pNode->getAttr("value", sPropValue))
		return false;
	IScriptSystem *pScriptSystem=GetIEditor()->GetSystem()->GetIScriptSystem();
	_SmartScriptObject pObj(pScriptSystem, true);
	if (!pScriptSystem->GetGlobalValue("EAXPresetDB", pObj))
		return false;
	CEAXPresetUpdateSODump Sink(pScriptSystem, pObj, sPresetName, sPropName, sPropValue);
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
						if (pObj->GetValue("eaxpresetEAXPreset", pszPreset))
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