// XConsole.h: interface for the CXConsole class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XCONSOLE_H__BA902011_5C47_4954_8E09_68598456912D__INCLUDED_)
#define AFX_XCONSOLE_H__BA902011_5C47_4954_8E09_68598456912D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IConsole.h>
#include <IInput.h>

//forward declaration
struct IIpnut;
struct IScriptSystem;
class CSystem;

#define	CURSOR_TIME		1.0f/2.0f // half second

#define MAX_HISTORY_ENTRIES 50
#define LINE_BORDER 10

enum ScrollDir
{
	sdDOWN,
	sdUP,
	sdNONE
};

struct XConsoleCommand
{
	string				m_sName;				//!< Console command name
	string				m_sCommand;			//!< lua code that is executed when this cammand is invoked
	const char *	m_psHelp;				//!< optional help string - can be shown in the console with "<commandname> ?"
	DWORD					m_dwFlags;			//!< bitmask consist of flag starting with VF_ e.g. VF_CHEAT

	size_t sizeofThis ()const {return sizeof(*this) + m_sName.capacity()+1 + m_sCommand.capacity()+1;}
};

typedef std::deque<string> ConsoleBuffer;
typedef ConsoleBuffer::iterator ConsoleBufferItor;
typedef ConsoleBuffer::reverse_iterator ConsoleBufferRItor;

typedef std::map<string,XConsoleCommand> ConsoleCommandsMap;
typedef ConsoleCommandsMap::iterator ConsoleCommandsMapItor;

typedef std::map<string,string> ConsoleBindsMap;
typedef ConsoleBindsMap::iterator ConsoleBindsMapItor;

struct string_nocase_lt 
{
	bool operator()( const string &s1,const string &s2 ) const 
	{
		return stricmp(s1.c_str(),s2.c_str()) < 0;
	}
};

class CXConsoleVariable;
typedef std::map<string,CXConsoleVariable*,string_nocase_lt> ConsoleVariablesMap;
typedef ConsoleVariablesMap::iterator ConsoleVariablesMapItor;

//forward declarations
struct ITexPic;
struct IRenderer;


/*! engine console implementation
	@see IConsole
*/
class CXConsole : public IConsole,public IInputEventListener
{
public:
	//! constructor
	CXConsole();
	//! destructor
	virtual ~CXConsole();

	//!
	void Init(CSystem *pSystem);
	//!
	void SetStatus(bool bActive){ m_bConsoleActive=bActive;}
	//<<FIXME>> legacy must be removed
	void SetFont(CXFont *p){ m_pXFont=p; }	
	//!
	void SetScriptSystem(IScriptSystem *pScriptSystem)
	{
		m_pScriptSystem=pScriptSystem;
	}
	//!
	void DrawBuffer(int nScrollPos, const char *szEffect);
	//!
	void RefreshVariable(string sVarName);
	//!
  void FreeRenderResources();
	//!
	void Copy();
	//!
	void Paste();


	// interface IConsole ---------------------------------------------------------

	virtual void Release();
	virtual ICVar *CreateVariable(const char *sName,const char *sValue,int nFlags, const char *help = "");
	virtual ICVar *CreateVariable(const char *sName,int iValue,int nFlags, const char *help = "");
	virtual ICVar *CreateVariable(const char *sName,float fValue,int nFlags, const char *help = "");
	virtual void UnregisterVariable(const char *sVarName,bool bDelete=false);
	virtual void SetScrollMax(int value);
	virtual void AddOutputPrintSink( IOutputPrintSink *inpSink );
	virtual void RemoveOutputPrintSink( IOutputPrintSink *inpSink );
	virtual void ShowConsole(bool show);
	virtual int Register(const char *name, void  *src, float defaultvalue, int flags, int type, const char *help = "");    
	virtual float Register(const char *name, float *src, float defaultvalue, int flags=0, const char *help = "");    
	virtual int Register(const char *name, int   *src, float defaultvalue, int flags=0, const char *help = "");    
	virtual void DumpCVars(ICVarDumpSink *pCallback,unsigned int nFlagsFilter=0);
	virtual void DumpKeyBinds(IKeyBindDumpSink *pCallback );
	virtual void CreateKeyBind(const char *sCmd,const char *sRes,bool bExecute);
	virtual const char* FindKeyBind( const char *sCmd );
	virtual inline void	SetImage(ITexPic *pImage,bool bDeleteCurrent); 
	virtual inline ITexPic *GetImage() { return m_pImage; }
	virtual void StaticBackground(bool bStatic) { m_bStaticBackground=bStatic; }
	virtual bool GetLineNo( const DWORD indwLineNo, char *outszBuffer, const DWORD indwBufferSize ) const;
	virtual int GetLineCount() const;
	virtual ICVar *GetCVar( const char *name, const bool bCaseSensitive=true );
  virtual CXFont *GetFont(){ return m_pXFont;} 
	virtual void Help(const char *command = NULL);
  virtual char *GetVariable( const char * szVarName, const char * szFileName, const char * def_val );
  virtual float GetVariable( const char * szVarName, const char * szFileName, float def_val );
	virtual void PrintLine(const char *s);
  virtual void PrintLinePlus(const char *s);
	virtual bool GetStatus();
	virtual void Clear();
	virtual void Update();
	virtual void Draw();
 	virtual void AddCommand(const char *sName, const char *sScriptFunc, const DWORD indwFlags=0, const char *help = "");
	virtual void ExecuteString(const char *command,bool bNeedSlash=false,bool bIgnoreDevMode=false);
	virtual void Exit(const char *command,...);
	virtual bool IsOpened();
	virtual int GetNumVars();
	virtual void GetSortedVars( const char **pszArray,size_t numItems );
	virtual const char*	AutoComplete( const char* substr );
	virtual const char*	AutoCompletePrev( const char* substr );
	virtual char *ProcessCompletion( const char *szInputBuffer);
	virtual void ResetAutoCompletion();
	virtual void DumpCommandsVars(char *prefix);
	virtual void GetMemoryUsage (class ICrySizer* pSizer);
	virtual void ResetProgressBar(int nProgressRange);
	virtual void TickProgressBar();
	virtual void SetLoadingImage(const char *szFilename);
	virtual void AddConsoleVarSink( IConsoleVarSink *pSink );
	virtual void RemoveConsoleVarSink( IConsoleVarSink *pSink );
	virtual const char* GetHistoryElement( const bool bUpOrDown );
	virtual void AddCommandToHistory( const char *szCommand );

	// interface IInputEventListener ------------------------------------------------------------------

	virtual bool OnInputEvent( const SInputEvent &event );

	// interface IConsoleVarSink ----------------------------------------------------------------------

	virtual bool OnBeforeVarChange( ICVar *pVar,const char *sNewValue );

protected: // ----------------------------------------------------------------------------------------

	void ProcessInput( int nKeyCode,const char *sKeyName );
	void AddLine(string str);
  void AddLinePlus(string str);
	void AddInputChar(const char c);
	void RemoveInputChar(bool bBackSpace);
	void ExecuteInputBuffer();
	void ExecuteCommand(XConsoleCommand &cmd,string &str,bool bIgnoreDevMode = false );
 
	void ScrollConsole();

	void ConsoleInputLog( const char *szFormat,... );
	void ConsoleLog( const char *szFormat,... );
	void ConsoleWarning( const char *szFormat,... );
	void DisplayHelp( const char *help, const char *name );

private: // ----------------------------------------------------------

	ConsoleBuffer										m_dqConsoleBuffer;
	ConsoleBuffer										m_dqHistory;

	bool														m_bStaticBackground;
	int															m_nLoadingBackTexID;
	int															m_nLoadingBarTexID;
	int															m_nProgress;
	int															m_nProgressRange;

	string													m_sInputBuffer;

	string													m_sPrevTab;
	int															m_nTabCount;

	ConsoleCommandsMap							m_mapCommands;						//!<
	ConsoleBindsMap									m_mapBinds;								//!<
	ConsoleVariablesMap							m_mapVariables;						//!<
	std::vector<IOutputPrintSink *>	m_OutputSinks;						//!< objects in this vector are not released

	typedef std::list<IConsoleVarSink*>	ConsoleVarSinks;
	ConsoleVarSinks									m_consoleVarSinks;

	int															m_nScrollPos;
	int															m_nScrollMax;
	int															m_nScrollLine;
	int															m_nHistoryPos;
	int															m_nCursorPos;
	ITexPic *												m_pImage;

	bool														m_bRepeat;
	float														m_nRepeatTimer;
	int															m_nLastKey;
	int															m_nRepeatKey;
	string													m_sLastKeyName;

	ScrollDir												m_sdScrollDir;

	bool														m_bDrawCursor;

	bool														m_bConsoleActive;

	//<<FIXME>> legacy must be removed
	CXFont *												m_pXFont;
	ICVar *													con_display_last_messages;
	ICVar *													con_line_buffer_size;

	CSystem	*												m_pSystem;
	IFFont *												m_pFont;
	IRenderer *											m_pRenderer;
	IInput *												m_pInput;
	IKeyboard *											m_pKeyboard;
	ITimer *												m_pTimer;
	IScriptSystem *									m_pScriptSystem;
};

#endif // !defined(AFX_XCONSOLE_H__BA902011_5C47_4954_8E09_68598456912D__INCLUDED_)
