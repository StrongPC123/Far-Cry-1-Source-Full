#include "StdAfx.h"
#include "AseParser.h"

#include "ASETokens.h"

#include <IMovieSystem.h>

//////////////////////////////////////////////////////////////////////////
CASEParser::CASEParser()
{
	m_currentPos = 0;
	m_animObject = 0;
	m_node = 0;
}

//////////////////////////////////////////////////////////////////////////
float CASEParser::TickToTime( int ticks )
{
	return (float)ticks / (m_scene.ticksPerFrame*m_scene.frameSpeed);
}

//////////////////////////////////////////////////////////////////////////
bool CASEParser::Compare( const char* token, const char* id )
{
	if (!token)
		return FALSE;

	return stricmp(token, id) == 0;
}

//////////////////////////////////////////////////////////////////////////
char* CASEParser::GetToken()
{
	static char str[512];
	char* p;
	int ch;
	BOOL quoted = FALSE;

	p = str;

	// Skip white space
	while (isspace(ch = GetChar()) && !IsEof());

	// Are we at end of file?
	if (IsEof()) 
	{
		return NULL;
	}

	if (ch == '\"') 
	{
		quoted = TRUE;
	}

	// Read everything that is not white space into the token buffer
	do 
	{
		*p = ch;
		p++;
		ch = GetChar();
		if (ch == '\"') 
		{
			quoted = FALSE;
		}
	} while ((quoted || !isspace(ch)) && !IsEof());

	*p = '\0';

	if (str[0] == '\0')
		return NULL;

	return str;
}

// Skip to the end of this block.
bool CASEParser::SkipBlock()
{
	char* token;
	int level = 1;
	do 
	{
		token = GetToken();
		if (Compare(token, "{"))
			level++;
		else if (Compare(token, "}"))
			level--;
	} while (level > 0);

	return true;
}

//////////////////////////////////////////////////////////////////////////
Vec3d CASEParser::GetVec3()
{
	char* token;
	Vec3d p;

	token = GetToken();
	p.x = (float)atof(token);
	token = GetToken();
	p.y = (float)atof(token);
	token = GetToken();
	p.z = (float)atof(token);

	return p;
}

//////////////////////////////////////////////////////////////////////////
Quat CASEParser::GetQuat()
{
	char* token;
	Quat q;

	token = GetToken();
	q.x = (float)atof(token);
	token = GetToken();
	q.y = (float)atof(token);
	token = GetToken();
	q.z = (float)atof(token);
	token = GetToken();
	q.w = (float)atof(token);

	return q;
}

//////////////////////////////////////////////////////////////////////////
float CASEParser::GetFloat()
{
	char* token;
	float f;

	token = GetToken();
	f = (float)atof(token);

	return f;
}

//////////////////////////////////////////////////////////////////////////
int CASEParser::GetInt()
{
	char* token;
	int i;

	token = GetToken();
	i = atoi(token);

	return i;
}

//////////////////////////////////////////////////////////////////////////
const char* CASEParser::GetString()
{
	static std::string str;

	str = GetToken();
	str = str.substr( 1,str.length()-2 );

	return str.c_str();
}

//////////////////////////////////////////////////////////////////////////
CAnimObject::Node* CASEParser::FindNode( const char *sNodeName )
{
	NodeMap::iterator it = m_nodeMap.find(sNodeName);
	if (it == m_nodeMap.end())
		return 0;
	return it->second;
}

//////////////////////////////////////////////////////////////////////////
Vec3 CASEParser::ScalePosition( const Vec3 &pos )
{
	return pos * (1.0f/100.0f);
}

//////////////////////////////////////////////////////////////////////////
void CASEParser::ParseGeomNode()
{
	Vec3 nodePos(0,0,0);
	Vec3 nodeScale(1,1,1);
	Vec3 nodeRotAxis(0,0,0);
	float nodeRotAngle = 0;

	// New node.
	m_node = 0;
	int level = 0;
	const char *str;
	char *token;
	do 
	{
		token = GetToken();

		if (Compare(token, "{"))
			level++;
		else if (Compare(token, "}"))
			level--;
		else if (Compare(token,ASE_NODE_NAME))
		{
			if (level == 1)
			{
				str = GetString();
				m_node = m_animObject->CreateNode(str);
				m_nodeMap[str] = m_node;
			}
			continue;
		}
		if (!m_node)
			continue;
		if (Compare(token,ASE_NODE_PARENT))
		{
			str = GetString();
			m_node->m_parent = FindNode(str);
		}
		else if (Compare(token,ASE_TM_POS))
		{
			nodePos = ScalePosition(GetVec3());
		}
		else if (Compare(token,ASE_TM_SCALE))
		{
			nodeScale = GetVec3();
		}
		else if (Compare(token,ASE_TM_ROTANGLE))
		{
			nodeRotAngle = GetFloat();
		}
		else if (Compare(token,ASE_TM_ROTAXIS))
		{
			nodeRotAxis = GetVec3();
		}
		else if (Compare(token,ASE_TM_ANIMATION))
		{
			ParseTMAnimation();
		}
	} while (level > 0);

	if (m_node)
	{
		Quat q;
		q.SetFromAngleAxis( nodeRotAngle,nodeRotAxis );
		m_node->m_pos = nodePos;
		m_node->m_scale = nodeScale;
		m_node->m_rotate = q;
		
		Matrix tm;
		q.GetMatrix(tm);
		tm.ScaleMatrix( nodeScale.x,nodeScale.y,nodeScale.z );
		tm.SetTranslation( nodePos );
		tm.Invert();
		if (m_node->m_parent)
			m_node->m_invOrigTM = tm * m_node->m_parent->m_invOrigTM;
		else
			m_node->m_invOrigTM = tm;
	}
}

//////////////////////////////////////////////////////////////////////////
void CASEParser::ParseTMAnimation()
{
	std::vector<ITcbKey> posTableTcb;
	std::vector<ITcbKey> rotTableTcb;
	std::vector<ITcbKey> scaleTableTcb;

	// Optimize allocations.
	posTableTcb.reserve( 10 );
	rotTableTcb.reserve( 10 );
	scaleTableTcb.reserve( 10 );
	// Parse node animation.
	Quat prevQ;
	prevQ.Identity();
	int level = 0;
	char *token;
	do 
	{
		token = GetToken();

		if (Compare(token, "{"))
			level++;
		else if (Compare(token, "}"))
			level--;
		else if (Compare(token,ASE_CONTROL_POS_TCB))
		{
			// Tcb Position controller.
			m_node->m_posTrack = GetIEditor()->GetMovieSystem()->CreateTrack( ATRACK_TCB_VECTOR );
			m_node->m_posTrack->SetFlags( ATRACK_CYCLE );
		}
		else if (Compare(token,ASE_CONTROL_ROT_TCB))
		{
			// Tcb Position controller.
			m_node->m_rotTrack = GetIEditor()->GetMovieSystem()->CreateTrack( ATRACK_TCB_QUAT );
			m_node->m_rotTrack->SetFlags( ATRACK_CYCLE );
		}
		else if (Compare(token,ASE_CONTROL_SCALE_TCB))
		{
			// Tcb Position controller.
			m_node->m_scaleTrack = GetIEditor()->GetMovieSystem()->CreateTrack( ATRACK_TCB_VECTOR );
			m_node->m_scaleTrack->SetFlags( ATRACK_CYCLE );
		}
		/*********************************************************************/
		//
		// Controller keys
		//
		/*********************************************************************/
		else if (Compare(token, ASE_POS_SAMPLE) || Compare(token, ASE_POS_KEY)) 
		{
			/*
			PosKeeper* p = new PosKeeper;
			p->t = GetInt();
			p->val = GetVec3();

			// Bezier tangents
			p->inTan = p->val;
			p->outTan = p->val;
			p->flags = 0;

			p->type = kBez;

			posTable.Append(1, &p, 5);
			*/
		}
		else if (Compare(token, ASE_BEZIER_POS_KEY)) 
		{
			/*
			PosKeeper* p = new PosKeeper;
			p->t = GetInt();
			p->val = GetVec3();

			// Bezier tangents
			p->inTan = GetVec3();
			p->outTan = GetVec3();
			p->flags = GetInt();

			p->type = kBez;

			posTable.Append(1, &p, 5);
			*/
		}
		else if (Compare(token, ASE_TCB_POS_KEY)) 
		{
			ITcbKey key;
			key.time = TickToTime(GetInt());
			key.SetVec3( ScalePosition(GetVec3()) );

			key.tens = GetFloat();
			key.cont = GetFloat();
			key.bias = GetFloat();
			key.easeto = GetFloat();
			key.easefrom = GetFloat();

			posTableTcb.push_back(key);
		}
		else if (Compare(token, ASE_TCB_ROT_KEY)) 
		{
			ITcbKey key;
			key.time = TickToTime(GetInt());
			Vec3 axis = GetVec3();
			float angle = GetFloat();

			key.tens = GetFloat();
			key.cont = GetFloat();
			key.bias = GetFloat();
			key.easeto = GetFloat();
			key.easefrom = GetFloat();

			Quat q;
			q.SetFromAngleAxis( angle,axis );
			// Rotation in ASE file is relative, so we must convert it to absolute.
			q = q * prevQ;
			prevQ = q;
			key.SetQuat(q);

			rotTableTcb.push_back(key);
		}
		else if (Compare(token, ASE_TCB_SCALE_KEY)) 
		{
			ITcbKey key;
			key.time = TickToTime(GetInt());
			key.SetVec3( GetVec3() );

			key.tens = GetFloat();
			key.cont = GetFloat();
			key.bias = GetFloat();
			key.easeto = GetFloat();
			key.easefrom = GetFloat();

			scaleTableTcb.push_back(key);
		}
	} while (level > 0);

	if (m_node->m_posTrack)
	{
		m_node->m_posTrack->SetNumKeys(posTableTcb.size());
		for (int i = 0; i < posTableTcb.size(); i++)
		{
			m_node->m_posTrack->SetKey( i,&posTableTcb[i] );
		}
	}
	if (m_node->m_rotTrack)
	{
		m_node->m_rotTrack->SetNumKeys(rotTableTcb.size());
		for (int i = 0; i < rotTableTcb.size(); i++)
		{
			m_node->m_rotTrack->SetKey( i,&rotTableTcb[i] );
		}

		/*
		FILE *file = fopen( "Objects\\sampl.txt","wt" );
		prevQ.Identity();
		// Sample rot track.
		for (int t = 0; t < 14400; t+= 160)
		{
			Quat qval;
			m_node->m_rotTrack->GetValue( t,qval );
			
			Quat q = qval / prevQ;
			prevQ = qval;

			q = q.GetAngleAxis();
			fprintf( file,"%d: %.4f,%.4f,%.4f,%.4f\n",t,q.x,q.y,q.z,q.w );
		}
		fclose(file);
		*/
	}
	if (m_node->m_scaleTrack)
	{
		m_node->m_scaleTrack->SetNumKeys(scaleTableTcb.size());
		for (int i = 0; i < scaleTableTcb.size(); i++)
		{
			m_node->m_scaleTrack->SetKey( i,&scaleTableTcb[i] );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Get scene parameters from the file
void CASEParser::ParseSceneParams()
{
	char* token;
	int level = 0;
	do 
	{
		token = GetToken();
		if (Compare(token, ASE_FIRSTFRAME)) 
		{
			m_scene.firstFrame = GetInt();
		}
		else if (Compare(token, ASE_LASTFRAME)) 
		{
			m_scene.lastFrame = GetInt();
		}
		else if (Compare(token, ASE_FRAMESPEED)) 
		{
			m_scene.frameSpeed = GetInt();
		}
		else if (Compare(token, ASE_TICKSPERFRAME)) 
		{
			m_scene.ticksPerFrame = GetInt();
		}
		else if (Compare(token, "{"))
			level++;
		else if (Compare(token, "}"))
			level--;

	} while (level > 0);
}

//////////////////////////////////////////////////////////////////////////
bool CASEParser::ParseString( CAnimObject *object,const char *buffer )
{
	m_animObject = object;
	m_buffer = buffer;

	char *token;
	while (token = GetToken())
	{
		if (Compare(token,ASE_SCENE))
		{
			ParseSceneParams();
		}
		else if (Compare(token,ASE_GEOMETRY))
		{
			ParseGeomNode();
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CASEParser::Parse( CAnimObject *object,const char *filename )
{
	m_animObject = object;

	FILE *file = fopen(filename,"rb");
	if (file)
	{
		fseek( file,0,SEEK_END );
		int fsize = ftell(file);
		fseek( file,0,SEEK_SET );

		char *buf = new char[fsize+1];
		fread( buf,fsize,1,file );
		buf[fsize] = 0;
		ParseString( m_animObject,buf );
		delete []buf;
		fclose(file);
	}

	return true;
}