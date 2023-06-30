#include "StdAfx.h"
#include "SourceSafeHelper.h"


#if defined(WIN32) && !defined(WIN64)			// windows specific implementation

#include <objbase.h>
#include <comdef.h>
#include "TCHAR.h"					// _T()



#import "SSAPI.DLL" no_namespace

#pragma comment (lib, "ole32.lib")


#endif	// WIN32

// 
bool _GetSSFileInfo( const char *inszSourceSafePath, const char *inszSSProject, const char *inszDirProject, const char *inszFileName, 
	char *outszName, char *outszComment, char *outszDate, const unsigned int innBufferSize )
{
	assert(innBufferSize>0);

	// to make sure the result is empty if this function failes
	outszName[0]=0;outszComment[0]=0;outszDate[0]=0;


#if !(defined(WIN32) && !defined(WIN64))			// non windows specific implementation
	return false;		// this plattform is not supporting SourceSafeInfo
#endif	// WIN32

#if defined(WIN32) && !defined(WIN64)			// windows specific implementation


	char sSSFilePath[_MAX_PATH];		// max SS path size (_MAX_PATH might be not right here)

	sprintf(sSSFilePath,"%s/%s",inszSSProject,inszFileName);

	// if path is absolute, remove leading part
	if(_strnicmp(inszDirProject,inszFileName,strlen(inszDirProject))==0)
	{
		char cSeperator=inszFileName[strlen(inszDirProject)];

		if(cSeperator=='/' || cSeperator=='\\')
			sprintf(sSSFilePath,"%s/%s",inszSSProject,&inszFileName[strlen(inszDirProject)+1]);
	}

	// replace '\' by '/'
	{
		char *p=sSSFilePath;

		while(*p)
		{
			if(*p=='\\') *p='/';
			p++;
		}
	}

	try
	{
		IVSSDatabasePtr pDatabase;
		IVSSItemPtr pIRootItem;

		pDatabase.CreateInstance(_T("SourceSafe"));
		pDatabase->Open(inszSourceSafePath, _T(""), _T(""));								// open ( sourcesafe, username, password )
		pIRootItem = pDatabase->GetVSSItem(sSSFilePath, VARIANT_FALSE);		// specify file
		IVSSVersionsPtr pVersions=pIRootItem->GetVersions(0);
		IEnumVARIANTPtr pEnum=pVersions->_NewEnum();
		_variant_t var;
		pEnum->Next(1,&var,NULL);
		IVSSVersionPtr pVer=var;
		
		_bstr_t name=pVer->GetUsername();
		_bstr_t comment=pVer->GetComment();
		DATE date=pVer->GetDate();

		BSTR wdatestring;

		_bstr_t t;

		// this may cause a problem:
		// LOCALE_SYSTEM_DEFAULT or LOCALE_USER_DEFAULT is used (output varies from windows settings)
		VarBstrFromDate(date,0,VAR_FOURDIGITYEARS|VAR_CALENDAR_GREGORIAN|VAR_DATEVALUEONLY,&wdatestring);
		t.Attach(wdatestring);		
		char datestring[256];
		strcpy(datestring,(TCHAR *)t);
		
		//if(WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,(LPCWSTR)wdatestring,-1,datestring,256,NULL,NULL)==0)
			//return false;

		if(strncpy(outszName,(TCHAR *)name,innBufferSize)==0)return false;
		if(strncpy(outszComment,(TCHAR *)comment,innBufferSize)==0)return false;
		if(strncpy(outszDate,(TCHAR *)datestring,innBufferSize)==0)return false;
		//::SysFreeString(wdatestring);
	}
	catch(_com_error &e)
	{
		// error handling (return false
		_bstr_t error=e.Description();
		return false;
	}

#endif	// WIN32

	return true;
}

