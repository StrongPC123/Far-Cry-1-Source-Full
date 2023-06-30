#pragma once


// platform: windows ascii/unicode (is compiled to nil on other plattforms)
//
// * make sure ::CoInitialize(NULL); is called before this class is used and ( don't forget ::CoUninitialize(); )
// * SourceSafe has to be installed
// * This is written as gloabal function to make it easy to include in MFC and non MFC projects
//
// written by AlbertoD, MartinM
//
// dependencies: none

//! get info about the last SourceSafe action for a specifed file
//! relative path form inszFileName is extracted, combined with inszSSProject to geth the SS path
//! (e.g. "$/AssMan/AssManShellExt/AssManMenu.cpp")
//! SS connection is opened and closed in this call 
//! \param inszSourceSafePath e.g. "\\\\server1\\vss\\srcsafe.ini"
//! \param inszSSProject inszSSProject!=0, e.g. "$/AssMan"
//! \param inszDirProject inszDirProject!=0, e.g. "c:\\mastercd\\AssMan"
//! \param inszFileName inszFileName!=0, e.g. "c:\\mastercd\\AssMan\\AssManShellExt\\AssManMenu.cpp"
//! \param outszName outszName!=0, [0..indwBufferSize-1]
//! \param outszComment outszComment!=0, [0..indwBufferSize-1]
//! \param outszDate outszDate!=0, [0..indwBufferSize-1]
//! \param indwBufferSize >0
//! \return true=success, false otherwise (output parameters are set to empty strings)
bool _GetSSFileInfo( const char *inszSourceSafePath, const char *inszSSProject, const char *inszDirProject, const char *inszFileName, 
	char *outszName, char *outszComment, char *outszDate, const unsigned int innBufferSize );
