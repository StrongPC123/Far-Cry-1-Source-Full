#ifndef _ICONSOLE_H_
#define _ICONSOLE_H_

struct ConsoleBind;
class CXFont;

struct ICVar;

#define     CVAR_INT    1
#define     CVAR_FLOAT  2
#define     CVAR_STRING 3

// if this flag is set during registering a console variable, and the variable exists,
// then the variable will store its value in memory given by src
#define CVF_CHANGE_SOURCE (1u<<16)

#define VF_SERVER_ONCE  			0x00000001
#define VF_CHEAT			   			0x00000002											// stays in the default state when cheats are disabled
#define VF_USERINFO     			0x00000004
#define VF_MODIFIED     			0x00000008
#define VF_SERVER       			0x00000010
#define VF_NONOTIFY     			0x00000020
#define VF_NOCHANGELEV				0x00000040
#define VF_REQUIRE_NET_SYNC		0x00000080											// cannot be changed on client and when connecting the var sent to the client
#define VF_DUMPTODISK					0x00000100
#define VF_SAVEGAME						0x00000200											// stored when saving a savegame
#define VF_NOHELP							0x00000400
#define VF_READONLY						0x00000800											// can not be changed by the user
#define VF_REQUIRE_LEVEL_RELOAD 0x00001000
#define VF_REQUIRE_APP_RESTART  0x00002000

struct ICVarDumpSink
{
	virtual void OnElementFound(ICVar *pCVar) = 0;
};

struct IKeyBindDumpSink
{
	virtual void OnKeyBindFound( const char *sBind,const char *sCommand ) = 0;
};

struct IOutputPrintSink
{
	virtual void Print( const char *inszText )=0;
};

//! Callback class to derive from when you want to recieve callbacks when console var changes.
struct IConsoleVarSink
{
	//! Called by Console before changing console var value, to validate if var can be changed.
	//! @return true if ok to change value, false if should not change value.
	virtual bool OnBeforeVarChange( ICVar *pVar,const char *sNewValue ) = 0;
};

/*! Interface to the engine console.

	The engine console allow to manipulate the internal engine parameters
	and to invoke commands.
	This interface allow external modules to integrate their functionalities
	into the console as commands or variables.

	IMPLEMENTATIONS NOTES:
	The console takes advantage of the script engine to store the console variables,
	this mean that all variables visible through script and console.

*/ 
struct IConsole
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! delete the variable
		NOTE: the variable will automatically unregister itself from the console
	*/
	virtual void Release() = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Crate a new console variable
		@param sName console variable name
		@param sValue default value
		@param nFlag user definded flag, this parameter is used by other subsystems 
			and doesn't affect the console varible (basically of user data)
		@return a pointer to the interface ICVar
		@see ICVar
	*/
	virtual ICVar *CreateVariable(const char *sName,const char *sValue,int nFlags, const char *help = "")=0;
	virtual ICVar *CreateVariable(const char *sName,int iValue,int nFlags, const char *help = "")=0;
	virtual ICVar *CreateVariable(const char *sName,float fValue,int nFlags, const char *help = "")=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Remove a variable from the console
		@param sVarName console variable name
		@param bDelete if true the variable is deleted
		@see ICVar
	*/
	virtual void UnregisterVariable(const char *sVarName,bool bDelete=false) = 0 ;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Set the y coordinate where the console will stop to scroll when is dropped
		@param value y in screen coordinates
	*/
	virtual void SetScrollMax(int value)=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! add output sink (clases which are interested in the output) - order is not guaranteed
		@param inpSink must not be 0 and is not allowed to be added twice
	*/
	virtual void AddOutputPrintSink( IOutputPrintSink *inpSink )=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! remove output sink (clases which are interested in the output) - order is not guaranteed
		@param inpSink must not be 0 and has to be added before
	*/
	virtual void RemoveOutputPrintSink( IOutputPrintSink *inpSink )=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! show/hide the console
		@param specifies if the window must be (true=show,false=hide)
	*/
	virtual void	ShowConsole(bool show)=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Crate a new console variable that store the value in a user defined memory block
		@param sName console variable name
		@param src pointer to the memory that will store the value 
		@param value default value
		@param type type of the value (can be CVAR_INT|CVAR_FLOAT)
		@return the value
		@see ICVar
	*/
	virtual int Register(const char *name, void  *src, float defaultvalue, int flags, int type, const char *help = "")=0;    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Crate a new console variable that store the value in a user defined floating point
		@param sName console variable name
		@param src pointer to the memory that will store the value 
		@param value default value
		@return the value
		@see ICVar
	*/
	virtual float Register(const char *name, float *src, float defaultvalue, int flags=0, const char *help = "")=0;    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Crate a new console variable that store the value in a user defined integer
		@param sName console variable name
		@param src pointer to the memory that will store the value 
		@param value default value
		@return the value
		@see ICVar
	*/
	virtual int Register(const char *name, int   *src, float defaultvalue, int flags=0, const char *help = "")=0;    
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Dump all console-variables to a callback-interface
		@param Callback callback-interface which needs to be called for each element
	*/
	virtual void DumpCVars(ICVarDumpSink *pCallback,unsigned int nFlagsFilter=0 )=0;
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Bind a console command to a key
		@param sCmd console command that must be executed
		@param sRes name of the key to invoke the command
		@param bExecute legacy parameter(will be removed soon)
	*/
	virtual void CreateKeyBind(const char *sCmd,const char *sRes,bool bExecute)=0;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Sets the background-image
		@param pImage background-image
	*/
	virtual void	SetImage(struct ITexPic *pImage,bool bDeleteCurrent)=0;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Retrieves the background-image
		@return background-image
	*/
	virtual struct ITexPic *GetImage()=0;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Sets static/scroll background-mode
		@param bStatic true if static
	*/
	virtual void StaticBackground(bool bStatic)=0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Sets the loading-background-image
	@param pImage background-image
	*/
	virtual void	SetLoadingImage( const char *szFilename )=0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Iterate through the lines - used for dedicated server (truncated if needed)
		@param indwLineNo 0.. counted from the last printed line on
		@param outszBuffer pointer to the destination string buffer (zero terminted afterwards), must not be 0
		@param indwBufferSize 1.. size of the buffer
		@return true=line was returned, false=there are no more lines
	*/

	virtual bool GetLineNo( const DWORD indwLineNo, char *outszBuffer, const DWORD indwBufferSize ) const=0;

	/*! @return current number of lines in the console
	*/
	virtual int GetLineCount() const=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Retrieve a console variable by name
		@param sName variable name
		@param bCaseSensitive true=faster, false=much slower but allowes names with wrong case (use only where performce doesn't matter)
		@return a pointer to the ICVar interface, NULL if is not found
		@see ICVar
	*/
	virtual ICVar* GetCVar( const char *name, const bool bCaseSensitive=true )=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! legacy function */
  virtual CXFont *GetFont()=0; 
	/*! legacy function */
  virtual void Help(const char *command = NULL)=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Read a value from a configuration file (.ini) and return the value
		@param szVarName variable name
		@param szFileName source configuration file
		@param def_val default value (if the variable is not found into the file)
		@return the variable value
	*/
  virtual char *GetVariable( const char * szVarName, const char * szFileName, const char * def_val )=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Read a value from a configuration file (.ini) and return the value
		@param szVarName variable name
		@param szFileName source configuration file
		@param def_val default value (if the variable is not found into the file)
		@return the variable value
	*/
  virtual float GetVariable( const char * szVarName, const char * szFileName, float def_val )=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Print a string in the console and go to the new line
		@param s the string to print
	*/
	virtual void PrintLine(const char *s)=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Append a string in the last console line
		@param s the string to print
	*/
	virtual void PrintLinePlus(const char *s)=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Retreive the status of the console (active/not active)
		@return the variable value(true = active/false = not active)
	*/
	virtual bool GetStatus()=0;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Clear the console text
	*/
	virtual void	Clear()=0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Update the console
	*/
	virtual void	Update()=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Draw the console
	*/
	virtual void	Draw()=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Add a Console command
		@param sName name of the command (ex "connect")
		@param sScriptFunc script buffer the contain the command implementation
			EG "Game.Connect(%1)" the symbol "%1" will be replaced with the command parameter 1
			writing in the console "connect 127.0.0.1" will invoke Game.Connect("127.0.0.1")
		@param indwFlags bitfield consist of VF_ flags (e.g. VF_CHEAT)
	*/
	virtual void AddCommand(const char *sName, const char *sScriptFunc, const DWORD indwFlags=0, const char *help = "") = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Execute a string in the console
		@param command console command
	*/
	virtual void ExecuteString(const char *command,bool bNeedSlash=false,bool bIgnoreDevMode=false) = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Print a message into the log and abort the execution of the application
		@param message error string to print in the log
	*/
	virtual void Exit(const char *command,...) = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Return true if the console is opened
		@return the variable value(true = opened/false = closed)
	*/
	virtual bool IsOpened() = 0;

	//////////////////////////////////////////////////////////////////////////
	// Auto completion.
	//////////////////////////////////////////////////////////////////////////
	virtual int		GetNumVars() = 0;
	virtual void	GetSortedVars( const char **pszArray,size_t numItems ) = 0;
	virtual const char*	AutoComplete( const char* substr ) = 0;
	virtual const char*	AutoCompletePrev( const char* substr ) = 0;
	virtual char *ProcessCompletion( const char *szInputBuffer ) = 0;
	//! 
	virtual void ResetAutoCompletion()=0;
	
	virtual void DumpCommandsVars(char *prefix) = 0;

	//! Calculation of the memory used by the whole console system
	virtual void GetMemoryUsage (class ICrySizer* pSizer) = 0;

	//! Function related to progress bar
	virtual void ResetProgressBar(int nProgressRange) = 0;
	//! Function related to progress bar
	virtual void TickProgressBar() = 0;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Dump all key bindings to a callback-interface
	@param Callback callback-interface which needs to be called for each element
	*/
	virtual void DumpKeyBinds(IKeyBindDumpSink *pCallback )=0;
	virtual const char* FindKeyBind( const char *sCmd ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Console variable sink.
	//////////////////////////////////////////////////////////////////////////
	//! Adds a new console variables sink callback.
	virtual void AddConsoleVarSink( IConsoleVarSink *pSink )=0;
	//! Removes a console variables sink callback.
	virtual void RemoveConsoleVarSink( IConsoleVarSink *pSink )=0;

	//////////////////////////////////////////////////////////////////////////
	// History
	//////////////////////////////////////////////////////////////////////////

	//! \param bUpOrDown true=after pressed "up", false=after pressed "down"
	//! \return 0 if there is no history line or pointer to the null terminated history line
	virtual const char* GetHistoryElement( const bool bUpOrDown )=0;
	//! \param szCommand must not be 0
	virtual void AddCommandToHistory( const char *szCommand )=0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! this interface is the 1:1 "C++ representation"
//! of a console variable.
//! NOTE: a console variable is accessible in C++ trough
//! this interface and in all scripts as global variable
//! (with the same name of the variable in the console)
struct ICVar
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! delete the variable
		NOTE: the variable will automatically unregister itself from the console
	*/
	virtual void Release() = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Return the integer value of the variable
		@return the value
	*/
	virtual int GetIVal() = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Return the float value of the variable
		@return the value
	*/
	virtual float GetFVal() = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Return the string value of the variable
		@return the value
	*/
	virtual char *GetString() = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! set the string value of the variable
		@param s string representation the value
	*/
	virtual void Set(const char* s)=0;

	/*! Force to set the string value of the variable - can be called 
			from inside code only
		@param s string representation the value
	*/
	virtual void ForceSet(const char* s)=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! set the float value of the variable
		@param s float representation the value
	*/
	virtual void Set(float f)=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! set the float value of the variable
		@param s integer representation the value
	*/
	virtual void Set(int i)=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! refresh the values of the variable
	*/
	virtual void Refresh()=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! clear the specified bits in the flag field
	*/
	virtual void ClearFlags (int flags)=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! return the variable's flags 
		@return the variable's flags
	*/
	virtual int GetFlags()=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! Set the variable's flags 
	*/
	virtual int SetFlags( int flags )=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! return the primary variable's type
		@return the primary variable's type
	*/
	virtual int GetType()=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! return the variable's name
		@return the variable's name
	*/
	virtual const char* GetName()=0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*! return the variable's name
		@return the variable's name
	*/
	virtual const char* GetHelp()=0;
	
	
};

#endif //_ICONSOLE_H_
