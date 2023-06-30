// PluginManager.h: interface for the CPluginManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLUGINMANAGER_H__9BF63ADF_3ADE_4384_8110_A026C96F0ABB__INCLUDED_)
#define AFX_PLUGINMANAGER_H__9BF63ADF_3ADE_4384_8110_A026C96F0ABB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IEditorImpl.h"
#include <list>

// List of plugins
typedef std::list<IPlugin *> PluginList;
typedef PluginList::iterator PluginIt;

// Event IDs assoicated with event handlers
typedef std::map<int, IUIEvent *> EventHandlerMap;
typedef EventHandlerMap::iterator EventHandlerIt;

// Plugins associated with ID / handler maps
typedef std::map<IPlugin *, EventHandlerMap> PluginEventMap;
typedef PluginEventMap::iterator PluginEventIt;

// UI IDs associated with plugin pointers. When a plugin UI element is
// activated, the ID is used to determine which plugin should handle
// the event
typedef std::map<uint8, IPlugin *> UIIDPluginMap;
typedef UIIDPluginMap::iterator UIIDPluginIt;

class CPluginManager  
{
public:
	CPluginManager();
	virtual ~CPluginManager();

	// Actions
	bool LoadAllPlugins(CString strPath);
	void NewDocument();
	bool CanAllPluginsExitNow();
	bool CallExport(const char * pszGamePath);

	void AddHandlerForCmdID(IPlugin *pIPluginOfHandler, uint8 iCmdID, IUIEvent *pIEvt);

	// Plugin access
	PluginList * GetPluginList() { return &m_lPlugins; };
	IPlugin * GetPluginByGUID(const char * pszGUID);
	IPlugin * GetPluginByUIID(uint8 iUserInterfaceID);
	IUIEvent * GetEventByIDAndPluginID(uint8 iPluginID, uint8 iEvtID);

	IPlugin* GetAssociatedPlugin() const { return m_pIAssociatedPlugin; };
	uint8 GetAssociatedPluginUIID() const { return m_iAssociatedPluginUIID; }

protected:
	// Interface needs to know which plugin it is dealing with
	void SetAssociatedPlugin(IPlugin * pIPlugin,uint8 iUIID);

	IPlugin * m_pIAssociatedPlugin;
	uint8 m_iAssociatedPluginUIID;

	void CleanUp();

	PluginList m_lPlugins;

	PluginEventMap m_mPluginEventMap;

	UIIDPluginMap m_mUIIDPluginMap;
};

#endif // !defined(AFX_PLUGINMANAGER_H__9BF63ADF_3ADE_4384_8110_A026C96F0ABB__INCLUDED_)
