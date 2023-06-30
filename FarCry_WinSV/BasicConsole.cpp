#include "stdafx.h"


//#ifdef NONWIN32TESTCOMPILE


#include "BasicConsole.h"

#include <vector>										// STL vector<>, used by IGame, why?

#include <stdlib.h>
#include <stdio.h>

#if !defined(LINUX)
	#include <conio.h>									// getch() 
#else
	extern bool g_OnQuit;
#endif

#include <platform.h>								// string

#include "Cry_Math.h"
#include <Cry_Camera.h>

#include <IRenderer.h>
#include <ILog.h>
#include <ISystem.h>
#include <ITimer.h>
#include <IInput.h>


#include <IConsole.h>								// IOutputPrintSink
#include <IGame.h>									// IGame
#include "DedicatedServer.h"				// InitDedicatedServer

char			g_szInputLine[256]="";		//!<
int				g_iCursorPos=0;						//!<
bool			g_bInputVisible=false;		//!< true=visible, false=hidden



//
void HideInputLine(const bool cRemovePrompt = true)
{
	g_bInputVisible=false;

	int i;
	int iLen=(int)strlen(g_szInputLine);

	if(cRemovePrompt) 
		iLen += 2;
	// remove string and prompt
	for(i=0;i<iLen;++i)
		printf("%c",8);							// backspace

	// clear line
	for(i=0;i<iLen;++i)
		printf(" ");								// space

	for(i=0;i<iLen;++i)
		printf("%c",8);							// go to beginning of line
}

//
void ShowInputLine()
{
	g_bInputVisible=true;
	printf("> %s",g_szInputLine);
#if defined(LINUX)
	fflush(stdout);
#endif
}


//
void ChangeInputLine( const char *szNewLine)
{
	HideInputLine();
	strcpy(g_szInputLine,szNewLine);
	g_iCursorPos=strlen(g_szInputLine);
	ShowInputLine();
}

// -----------------------------------------------------------------


class COutputPrintSinkCON :public IOutputPrintSink
{
public:
	//! constructor
	COutputPrintSinkCON() {};
	//! destructor
	virtual ~COutputPrintSinkCON() {};

	// interface IOutputPrintSink ------------------------

	virtual void Print( const char *inszText )
	{
		const char *szStripped = Strip(inszText);

		if(g_bInputVisible)
		{
			// while the user console input line is shown
			HideInputLine();
			printf("%s\n",szStripped);
			ShowInputLine();
		}
		else 
		{
			// during executing a console command (console input line is hidden)
			printf("%s\n",szStripped);
		}
	}
};

// -----------------------------------------------------------------

#if defined(LINUX)
void ClearAndSetCursorToStart(const bool cShowPrompt = true)
{
	printf("\033[2K"); //clear current line
	printf("\033[%dD",cShowPrompt?g_iCursorPos : g_iCursorPos+2); //position cursor to the start
	g_iCursorPos=0;
	fflush(stdout);
}

void ClearInputLine(const bool bResetInputLine = true)
{
	if(g_iCursorPos == 0)
		return;//already empty
	ClearAndSetCursorToStart(false);
	if(bResetInputLine)
		strcpy(g_szInputLine,"");
	ShowInputLine();								// show prompt
}

#endif
 
#if defined(LINUX)
void DisplayFrameRate(const float cFrameRate)
{
	printf("\033[s");//save cursor pos	
	printf("\033[0;0H");
	printf(" FPS");
	printf("\033[5m");
	printf(":");
	fflush(stdout); 
	printf("\033[0m");
	fflush(stdout);
	printf("%4.2f   \n",cFrameRate);
	printf("             \n");
	printf("\033[u");//restore cursor pos
	printf("\033[s");//save cursor pos	
	fflush(stdout);
}

void ResetFrameRate()
{
	printf("\033[s");//save cursor pos	
	printf("\033[0;0H");
	for(int i=0; i<2; ++i)
		printf("             \n");
	printf("\033[u");//restore cursor pos
	printf("\033[s");//save cursor pos	
	fflush(stdout);
}
#endif


#if !defined(LINUX)
#include <windows.h>
#endif
#if defined(LINUX)
	int MainCON( const char *szCmdLine, const bool cIsDaemon)
#else
	int MainCON( const char *szCmdLine)
#endif
{
	COutputPrintSinkCON			OutputSinkCON;			//!<

// how to add the console window to win32 application http://phoenix.liunet.edu/~mdevi/win32gui/Win32Apps.htm
	AllocConsole();										// Creates a new Console Window, if one has not already been created
#if !defined(LINUX)
	freopen("CONIN$","rb",stdin);			// reopen stdin handle as console window input
	freopen("CONOUT$","wb",stdout);		// reopen stout handle as console window output
	freopen("CONOUT$","wb",stderr);		// reopen stderr handle as console window output
#endif

	printf("Initializing...\n");

	InitDedicatedServer_System(szCmdLine);	// is executing early commands (e.g. -DEVMODE, -IP)

	// redirect console output
	IConsole *pConsole = GetISystem()->GetIConsole();

	pConsole->AddOutputPrintSink(&OutputSinkCON);

	PrintDedicatedServerStatus();

	InitDedicatedServer_Game(szCmdLine);		// is executing the console commands specified in szCmdLine

	PrintWelcomeMessage();

	bool bRelaunch;

	ShowInputLine();												// show prompt

#if defined(LINUX)
	//store server start time into linux system variable to make sure it exists just once
	struct timeval t;
	gettimeofday( &t, NULL );

	float frameRate = 0.f;
	ICVar *	psvDisplayInfo=	pConsole->GetCVar("r_DisplayInfo");		
	bool frameRateDisplayed = false;
	bool frameRateDisplayedLastFrame = false;

	unsigned long long lastSecond = t.tv_sec; 

	bool bFirstTime = true;	//for sleep 
	bool bLoop = true;
	unsigned char cursorPressed = 0;
	static const unsigned char scCursorUp = 1, scCursorDown = 2;//constants for cursor key processing
	printf("\033[s");//save cursor pos
	fflush(stdout);
#endif

	// main loop
	while( GetISystem()->GetIGame()->Run( bRelaunch ) )
	{
#if defined(LINUX)
		if(!cIsDaemon)
		{
			bLoop = true;//only used for continue statements
			cursorPressed = 0;	//reset
			while(kbhit() && bLoop)
			{
				char c=readch();//internal stuff going on, can't use getch itself even it exists under linux
				if((int)c == 27)
				{
					bool bEscapeSequStarted = false;
					while(kbhit())
					{
						c=readch();
						if(c == '[')
						{
							bEscapeSequStarted = true;
							break;
						}
					}
					if(bEscapeSequStarted)
					{
						c=readch();
						switch(c)
						{
							case 'A': 
								cursorPressed = scCursorUp;
								break;
							case 'B': 
								cursorPressed = scCursorDown;
								break;
							default:
								break;
						}
					}
					while(kbhit()){c = readch();}//flush input buffer
						//is escape sequence
					bLoop = false;
					printf("\033[u");//restore cursor pos
					fflush(stdout);
					if(!cursorPressed)//continue if no cursor has been pressed
						continue;
				}
				// tab (auto completion)   
				if(c==-1)
				{
					readch();
					continue;
				}
				// tab (auto completion)
				if(c != 9)
					pConsole->ResetAutoCompletion();
				if(c==9)
				{
					g_bInputVisible=false;
					printf("\033[u");//restore cursor pos
					fflush(stdout);
					char *szResult=pConsole->ProcessCompletion(g_szInputLine);
					if(*szResult=='\\')
					{
						ClearAndSetCursorToStart(false);
						strcpy(g_szInputLine,szResult+1);
						g_iCursorPos=strlen(g_szInputLine);
						ShowInputLine();
					}
				}
				else	
				if(cursorPressed)
				{
					switch(cursorPressed)
					{
					case scCursorUp:					// cursor up
						{
							const char *szHistoryLine=pConsole->GetHistoryElement(true);		// true=UP

							if(szHistoryLine)
								ChangeInputLine(szHistoryLine);
						}
						break;
					case scCursorDown:					// cursor down
						{
							const char *szHistoryLine=pConsole->GetHistoryElement(false);		// false=DOWN

							if(szHistoryLine)
								ChangeInputLine(szHistoryLine);
						}
						break;
					}
				}
				else
				// usual key    
				if(c>=32 && c<127)   
				{
					if(g_iCursorPos<200)
					{ 
						g_szInputLine[g_iCursorPos++]=c;
						g_szInputLine[g_iCursorPos]=0; 
					}
				}
				else
				// backspace
				if(c==8 || c==127) 
				{
					if(g_iCursorPos > 0)
					{
						printf("\b \b");								// clear character and go back
						fflush(stdout);
						g_szInputLine[--g_iCursorPos]=0; 
					}
				} 
				else
				// return (execute command)  
				if(c==13 || c==10)
				{
					printf("\033[1A");
					printf("\033[2K"); //clear current line
					printf("> ");
					fflush(stdout);
					char szInputLine[256];
					g_bInputVisible=false;

					strcpy(szInputLine,g_szInputLine);
					strcpy(g_szInputLine,"");				// clear it early to avoid wrong console printout after execution

					const string cCommandLine(szInputLine);

					if(cCommandLine == "quit")
					{
						g_OnQuit = true;
					}

					pConsole->ExecuteString(szInputLine);
	
					pConsole->AddCommandToHistory(szInputLine);
					ClearAndSetCursorToStart(false); 
					ShowInputLine();								// show prompt
					g_iCursorPos=0;
					fflush(stdout);
				}
				else
				{
					printf("\033[u");//restore cursor pos
					fflush(stdout);
				}
				printf("\033[s");//save cursor pos
				fflush(stdout);
			}
		}
#else
		while(kbhit())
		{
			unsigned char c=getch();
			// tab (auto completion)
			if(c==9)
			{
				// remove line
				HideInputLine();

				char *szResult=pConsole->ProcessCompletion(g_szInputLine);
				if(*szResult=='\\')
				{
					strcpy(g_szInputLine,szResult+1);
//					printf("%s",&g_szInputLine[g_iCursorPos]);
					g_iCursorPos=strlen(g_szInputLine);
				}

				ShowInputLine();											// show prompt with autocompleted string
			}
			else pConsole->ResetAutoCompletion();

			// special keys (second value in the input buffer nees different treatment)
			if(c==224)
			{
				unsigned char c2=getch();

				switch(c2)
				{
					case 'H':					// cursor up
						{
							const char *szHistoryLine=pConsole->GetHistoryElement(true);		// true=UP

							if(szHistoryLine)
								ChangeInputLine(szHistoryLine);
						}
						break;
					case 'P':					// cursor down
						{
							const char *szHistoryLine=pConsole->GetHistoryElement(false);		// false=DOWN

							if(szHistoryLine)
								ChangeInputLine(szHistoryLine);
						}
						break;
				}

				continue;
			}
			// usual key
			if(c>=32)
			{
				if(g_iCursorPos<200)
				{
					g_szInputLine[g_iCursorPos++]=c;
					printf("%c",c);											// show character
					g_szInputLine[g_iCursorPos]=0;
				}
			}
			// backspace
			if(c==8 && g_iCursorPos>0)
			{
				g_szInputLine[--g_iCursorPos]=0;
				printf("%c %c",8,8);								// clear character and go back
			}
			// return (execute command)
			if(c==13 || c==10)
			{
				char szInputLine[256];
				
				HideInputLine(false);										// hide prompt

				strcpy(szInputLine,g_szInputLine);
				strcpy(g_szInputLine,"");						// clear it early to avoid wrong console printout after execution

				pConsole->ExecuteString(szInputLine);
				pConsole->AddCommandToHistory(szInputLine);
				ShowInputLine();										// show prompt
				g_iCursorPos=0;
			}
		}
#endif
#if defined(LINUX)
		if(!cIsDaemon)
		{
			gettimeofday( &t, NULL );
			if(t.tv_sec > lastSecond)
			{
				//get new framerate
				frameRate = GetISystem()->GetITimer()->GetFrameRate();
				if(psvDisplayInfo && psvDisplayInfo->GetIVal() == 1)
				{
					frameRateDisplayed = true;   
					DisplayFrameRate(frameRate);
					frameRateDisplayedLastFrame = true;
				}
				else
				{
					if(frameRateDisplayedLastFrame)
						ResetFrameRate();
					frameRateDisplayedLastFrame = false;
				}
				lastSecond = t.tv_sec;
			}
		}
		SleepIfNeeded(bFirstTime);
#else
		SleepIfNeeded();
#endif
	}

	PrintGoodbyeMessage();

	DeInitDedicatedServer();

	FreeConsole();    // Close a console window

#if defined(LINUX)
	if(frameRateDisplayed)
		ResetFrameRate();
#endif

	return 1;
}

//#endif // NONWIN32TESTCOMPILE
