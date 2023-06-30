#include "StdAfx.h"
#include "animationserializer.h"
#include "cryeditdoc.h"
#include "mission.h"
#include "IMovieSystem.h"

#include "Util\PakFile.h"

CAnimationSerializer::CAnimationSerializer(void)
{
}

CAnimationSerializer::~CAnimationSerializer(void)
{
}

//////////////////////////////////////////////////////////////////////////
void CAnimationSerializer::SaveSequence( IAnimSequence *seq,const char *szFilePath, bool bSaveEmpty )
{
	assert( seq != 0 );
	XmlNodeRef sequenceNode = new CXmlNode( "Sequence" );
	seq->Serialize( sequenceNode,false, false );
	sequenceNode->saveToFile( szFilePath ); 
}

IAnimSequence* CAnimationSerializer::LoadSequence( const char *szFilePath )
{
	IAnimSequence *seq = 0;
	XmlParser parser;
	XmlNodeRef sequenceNode = parser.parse( szFilePath );
	if (sequenceNode)
	{
		seq = GetIEditor()->GetMovieSystem()->LoadSequence( sequenceNode );
	}
	return seq;
}

//////////////////////////////////////////////////////////////////////////
void CAnimationSerializer::SaveAllSequences( const char *szPath,CPakFile &pakFile )
{
	IMovieSystem *movSys = GetIEditor()->GetMovieSystem();
	XmlNodeRef movieNode=new CXmlNode("MovieData");
	for (int i=0;i<GetIEditor()->GetDocument()->GetMissionCount();i++)
	{
		CMission *pMission=GetIEditor()->GetDocument()->GetMission(i);
		pMission->ExportAnimations(movieNode);
	}
	string sFilename=string(szPath)+"MovieData.xml";
	//movieNode->saveToFile(sFilename.c_str());

	XmlString xml = movieNode->getXML();
	CMemFile file;
	file.Write( xml.c_str(),xml.length() );
	pakFile.UpdateFile( sFilename.c_str(),file );
}

//////////////////////////////////////////////////////////////////////////
void CAnimationSerializer::LoadAllSequences( const char *szPath )
{
	CString dir = Path::AddBackslash(szPath);
	std::vector<CFileUtil::FileDesc> files;
	CFileUtil::ScanDirectory( dir,"*.seq",files,false );

	for (int i = 0; i < files.size(); i++)
	{
		// Construct the full filepath of the current file
		LoadSequence( dir + files[i].filename );
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimationSerializer::SerializeSequences( XmlNodeRef &xmlNode,bool bLoading )
{
	if (bLoading)
	{
		// Load.
		IMovieSystem *movSys = GetIEditor()->GetMovieSystem();
		movSys->RemoveAllSequences();
		int num = xmlNode->getChildCount();
		for (int i = 0; i < num; i++)
		{
			XmlNodeRef seqNode = xmlNode->getChild(i);
			movSys->LoadSequence( seqNode );
		}
	}
	else
	{
		// Save.
		IMovieSystem *movSys = GetIEditor()->GetMovieSystem();
		ISequenceIt *It=movSys->GetSequences();
		IAnimSequence *seq=It->first();
		while (seq)
		{
			XmlNodeRef seqNode = xmlNode->newChild( "Sequence" );
			seq->Serialize( seqNode,false );
			seq=It->next();
		}
		It->Release();
	}
}