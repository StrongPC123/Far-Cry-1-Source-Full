////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   StatCGFCompiler.cpp
//  Version:     v1.00
//  Created:     5/11/2002 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <DbgHelp.h>
#include "ConvertContext.h"
#include "iconfig.h"
#include "StatCGFCompiler.h"
#define ISystem IRCLog
#include "meshidx.h"
#include "statcgfshadvol.h"

CStatCFGCompiler::CStatCFGCompiler(void)
{
}

CStatCFGCompiler::~CStatCFGCompiler(void)
{
}

// function for creating this from outside (without including StatCGFCompiler.h)
IConvertor* NewStatCGFCompiler()
{
	return new CStatCFGCompiler();
}

void CStatCFGCompiler::FindDependencies( CIndexedMesh * pIndexedMesh, ConvertContext &cc )
{
	cc.pLog->Log("Finding dependencies");

	for(int m=0; m<pIndexedMesh->m_lstMatTable.Count(); m++)
	{
		CMatInfo &ref=pIndexedMesh->m_lstMatTable[m];

		const char *szMatName=ref.GetName();
		const char *szScriptName=ref.sScriptMaterial;

		CString sourceFile = cc.getSourcePath();

		cc.pRC->AddDependencyMaterial(sourceFile.GetString(),szMatName,szScriptName);		// material name

		if(!ref.pMatEnt)continue;

		// texture path names
#define ADD_MAP(MAP) if (ref.pMatEnt->map_##MAP.name[0]) cc.pRC->AddDependencyFile(sourceFile.GetString(),ref.pMatEnt->map_##MAP.name);
		ADD_MAP(a);
		ADD_MAP(d);
		ADD_MAP(o);
		ADD_MAP(b);
		ADD_MAP(s);
		ADD_MAP(g);
		ADD_MAP(detail);
		ADD_MAP(e);
		ADD_MAP(subsurf);
		ADD_MAP(displ);
#undef ADD_MAP
	}
}

FILETIME UnixTimeToFileTime(time_t t)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
	return (FILETIME&)ll;
}


// returns the file modification time
FILETIME GetModificationTime(FILE* hFile)
{
	struct _stat st;
	_fstat(_fileno(hFile), &st);
#ifdef _DEBUG
	const char* szTest = ctime (&st.st_mtime);
#endif
	return UnixTimeToFileTime (st.st_mtime);
}


bool CStatCFGCompiler::GetSourceFileTime(const char * szFileName, FILETIME & fileTime)
{
	FILE* f = fopen (szFileName, "rb");
	if (!f)
		return false;

	fileTime = GetModificationTime(f);
	fclose (f);
	return true;
}

void CStatCFGCompiler::GetFileParams( ConvertContext &cc, CString & sGeomName, 
																			bool & bStripify, bool & bLoadAdditinalInfo, bool & bKeepInLocalSpace)
{
	cc.config->Get("GeomName",sGeomName); // subobject name
	cc.config->Get("Stripify",bStripify); // sort for vertex ceache
	cc.config->Get("LoadAdditinalInfo",bLoadAdditinalInfo); // load geom names, helpers, lightsources
	cc.config->Get("KeepInLocalSpace",bKeepInLocalSpace); // do not transform vertices by node matrix
}

bool CStatCFGCompiler::Process( ConvertContext &cc )
{
	try
	{
		CString sGeomName;
		bool bStripify=0, bLoadAdditinalInfo=0, bKeepInLocalSpace=0;
		GetFileParams( cc, sGeomName, bStripify, bLoadAdditinalInfo, bKeepInLocalSpace);
		if (!cc.bQuiet)
		{
			cc.pLog->Log("Conversion params:");
			cc.pLog->Log("  GeomName = %s", sGeomName[0] ? sGeomName : "None");
			cc.pLog->Log("  Stripify = %s", bStripify ? "Yes" : "No");
			cc.pLog->Log("  LoadAdditinalInfo = %s", bLoadAdditinalInfo ? "Yes" : "No");
			cc.pLog->Log("  KeepInLocalSpace = %s", bKeepInLocalSpace ? "Yes" : "No");
		}

		CString sourceFile = cc.getSourcePath();
		CString outputFile = cc.getOutputPath();

		const char *sInFile = sourceFile.GetString();
		
		FILETIME fileTime;
		if(!GetSourceFileTime( sourceFile.GetString(), fileTime))
		{
			remove( outputFile.GetString() );
			return false;
		}

		// Check if .cga.
		if (stricmp(Path::GetExt(cc.sourceFile).GetString(),"cga") == 0)
		{
			// Load CGA.
			ProcessCGA( cc,sourceFile,outputFile,fileTime,bStripify );
		}
		else
		{
			// Load Normal CGF.
			ProcessCGF( cc,sourceFile,outputFile,fileTime,sGeomName,bStripify,bLoadAdditinalInfo,bKeepInLocalSpace );
		}
	}
	catch(char*)
	{
		Beep(1000,1000);
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CStatCFGCompiler::ProcessCGA( ConvertContext &cc,CString &sourceFile,CString &outputFile,FILETIME fileTime,bool bStripify )
{
	CString outputFileNoExt = Path::RemoveExtension(cc.sourceFile);
	CString outputGeomFile;

	int nLoadedTrisCount=0;
	CIndexedMesh * pIndexedMesh = new CIndexedMesh( cc.pLog,sourceFile.GetString(),0,&nLoadedTrisCount,true,true );
	if (pIndexedMesh)
	{
		for (int i = 0; i < (int)pIndexedMesh->m_lstGeomNames.size(); i++)
		{
			CString sOrgGeomName = pIndexedMesh->m_lstGeomNames[i];
			CString sGeomName = sOrgGeomName;
			sGeomName.Replace( '\\','_' );
			sGeomName.Replace( '/','_' );
			outputGeomFile.Format( "%s_%s_%d_%d_%d.ccgf",outputFileNoExt.GetString(), sGeomName.GetString(),(int)bStripify,1,1 );
			cc.outputFile = outputGeomFile;
			outputGeomFile = cc.getOutputPath();
			const char *sgeom = outputGeomFile.GetString();
			ProcessCGF( cc,sourceFile,outputGeomFile,fileTime,sOrgGeomName,bStripify,true,true );
		}
	}
	delete pIndexedMesh;
}

//////////////////////////////////////////////////////////////////////////
void CStatCFGCompiler::ProcessCGF( ConvertContext &cc,CString &sourceFile,CString &outputFile,FILETIME fileTime,CString sGeomName,
								bool bStripify, bool bLoadAdditinalInfo, bool bKeepInLocalSpace )
{
	// load source cgf
	int nLoadedTrisCount=0;
	CIndexedMesh * pIndexedMesh = new CIndexedMesh( cc.pLog, sourceFile.GetString(), sGeomName[0] ? sGeomName.GetString() : 0,
		&nLoadedTrisCount, bLoadAdditinalInfo, bKeepInLocalSpace);
	pIndexedMesh->CalcTangentSpace();

	// if geom name was specified - save empty file to let the engine know that this geom does not exits
	// since passing wrong geom name is valid operation
	if(!nLoadedTrisCount && !sGeomName[0])
		cc.pLog->ThrowError(" No faces found");

	// find dependencies (material names, texture path names)
	FindDependencies( pIndexedMesh, cc );

	// compile data
	CSimpleStatObj StatObj( cc.pLog, pIndexedMesh, sourceFile.GetString() );
	CSimpleLeafBuffer LeafBuffer(cc.pLog, pIndexedMesh, bStripify, 
		pIndexedMesh->m_lstGeomNames.Count()>0 && strstr(pIndexedMesh->m_lstGeomNames[0],"cloth")!=0);
	CStatCGFShadVol StatCGFShadVol(cc.pLog, pIndexedMesh);

	int nPos = 0;

	// get data size
	if(nLoadedTrisCount)
	{
		StatObj.Serialize(nPos, 0, true);
		LeafBuffer.Serialize(nPos, 0, true, outputFile.GetString() );
		StatCGFShadVol.Serialize(nPos, 0, true);
	}

	// allocate mem buffer
	uchar * pData = new uchar[nPos+sizeof(CCGFHeader)];

	// make header
	CCGFHeader fileHeader;
	fileHeader.nDataSize = nPos;
	fileHeader.nFacesInCGFNum = nLoadedTrisCount;
	fileHeader.SourceFileTime = fileTime;
	fileHeader.vBoxMin = pIndexedMesh->m_vBoxMin;
	fileHeader.vBoxMax = pIndexedMesh->m_vBoxMax;
	strcpy(fileHeader.szVersion,CCGF_FILE_VERSION);
	if(StatObj.IsPhysicsExist())
		fileHeader.dwFlags |= CCGFHF_PHYSICS_EXIST;

	memcpy(pData,&fileHeader,sizeof(CCGFHeader));

	nPos = sizeof(fileHeader);

	// save to new file
	if(nLoadedTrisCount)
	{
		StatObj.Serialize(nPos, pData, true);
		LeafBuffer.Serialize(nPos, pData, true, outputFile.GetString() );
		StatCGFShadVol.Serialize(nPos, pData, true);
	}

	// create folder for object
	CString srtDirName = cc.getOutputFolderPath();
	int nFind = -1;
	while(1)
	{
		nFind = srtDirName.Find('\\', nFind+1);
		if(nFind<0)
			break;

		CString strSubDirName = srtDirName.Left(nFind);
		CreateDirectory(strSubDirName.GetString(),NULL);
	}

	cc.pLog->Log("Writing CCGF: %s", outputFile.GetString() );
	FILE * f = fopen(outputFile.GetString(),"wb");
	if(f)
	{
		size_t nWriten = fwrite(pData,1,nPos,f);
		fclose(f);
		if(nWriten == nPos)
			cc.pLog->Log("  %d bytes saved", nPos);
		else
			cc.pLog->ThrowError(" Error writing output file");
	}
	else
		cc.pLog->ThrowError(" Error opening output file");

	delete pIndexedMesh;
	delete pData;
}

////////////////////////////////////////////////////////////
//
// !!! PLZ NEVER CHANGE THIS FILE WITHOUT ASKING VLAD !!!
//
////////////////////////////////////////////////////////////

bool CStatCFGCompiler::GetOutputFile( ConvertContext &cc )
{
  bool bStripify=0, bLoadAdditinalInfo=0, bKeepInLocalSpace=0; CString sGeomName;
	GetFileParams( cc, sGeomName, bStripify, bLoadAdditinalInfo, bKeepInLocalSpace);

  CString outputFileNoExt = Path::ReplaceExtension( cc.sourceFile, "" );

	char szCurDir[MAX_PATH]="";
	GetCurrentDirectory(MAX_PATH,szCurDir);
	CString curFolderName = szCurDir;

	cc.outputFolder = cc.masterFolder + CString(CCGF_CACHE_DIR_NAME) + "\\" + cc.outputFolder;
	//CString outputDirName=Path::ReplacePath(curFolderName, curFolderName + "\\" + CCGF_CACHE_DIR_NAME, outputFileNoExt);

	char szOutputFileName[1024];
  sprintf(szOutputFileName,
    "%s_%s_%d_%d_%d.ccgf",
    outputFileNoExt.GetString(), sGeomName.GetString(), 
		(int)bStripify, (int)bLoadAdditinalInfo, (int)bKeepInLocalSpace);

	//specify output path
	cc.outputFile = szOutputFileName;
	return true;
}

//////////////////////////////////////////////////////////////////////////
int CStatCFGCompiler::GetNumPlatforms() const
{
	return 4;
}

//////////////////////////////////////////////////////////////////////////
Platform CStatCFGCompiler::GetPlatform( int index ) const
{
	switch (index)
	{
	case 0:	return PLATFORM_PC;
	case 1:	return PLATFORM_XBOX;
	//case 2:	return PLATFORM_PS2;
	//case 3:	return PLATFORM_GAMECUBE;
	};
	//assert(0);
	return PLATFORM_UNKNOWN;
}

DWORD CStatCFGCompiler::GetTimestamp() const
{
	return GetTimestampForLoadedLibrary(g_hInst);
}

CStatCFGCompiler::Error::Error (const char* szFormat, ...)
{
	char szBuffer[0x800];
	va_list arg;
	va_start(arg,szFormat);
	_vsnprintf (szBuffer, sizeof(szBuffer), szFormat, arg);
	va_end(arg);
	this->m_strReason = szBuffer;
}

CStatCFGCompiler::Error::Error (int nCode)
{
	char szBuffer[36];
	sprintf (szBuffer, "Generic Error #%d", nCode);
	this->m_strReason = szBuffer;
}

int CStatCFGCompiler::SetTexType(TextureMap3 *tm)
{
  if (tm->type == TEXMAP_CUBIC)
    return eTT_Cubemap;
  else
  if (tm->type == TEXMAP_AUTOCUBIC)
    return eTT_AutoCubemap;
  return eTT_Base;
}
