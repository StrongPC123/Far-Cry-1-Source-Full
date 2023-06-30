#include "StdAfx.h"
#include "AnimObjectLoader.h"

#include "ChunkFileReader.h"
#include "ControllerTCB.h"
#include "StringUtils.h"

#include <ICryPak.h>

#include "makepath.h"
#include "splitpath.h"

#define ANIMATION_EXT "anm"

using namespace CryStringUtils;

//////////////////////////////////////////////////////////////////////////
// Loads animation object.
//////////////////////////////////////////////////////////////////////////
bool CAnimObjectLoader::Load( CAnimObject* animObject,const char *geomName,const char *animFile )
{
	m_animObject = animObject;

	// Load chunk file.
	// try to read the file
	CChunkFileReader_AutoPtr pReader = new CChunkFileReader ();
	if (!pReader->open (geomName))
	{
		// NOTE: see the SIDE EFFECT NOTES
		//GetLog()->LogError ("Error: CControllerManager::LoadAnimation: file loading %s", strFileName.c_str());
		return false;
	}

	// check the file header for validity
	const FILE_HEADER& fh = pReader->getFileHeader();

	if(fh.Version != GeomFileVersion || fh.FileType != FileType_Geom)
	{
		g_GetLog()->LogError ("CAnimObjectLoader::Load: file version error or not an geometry file: %s", geomName );
		return false;
	}

	m_animStart = 0;
	m_animEnd = 0;
	m_secsPerTick = 1;
	m_ticksPerFrame = 1;

	m_currAnimation = new CAnimObject::Animation;
	m_currAnimation->name = "Default";
	animObject->AddAnimation(m_currAnimation);

	LoadChunks( pReader,true );

	LoadAnimations( geomName );

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimObjectLoader::LoadAnimation( const char *animFile )
{
	// Load chunk file.
	// try to read the file
	CChunkFileReader_AutoPtr pReader = new CChunkFileReader ();
	if (!pReader->open (animFile))
	{
		g_GetLog()->LogError ("CAnimObjectLoader::LoadAnimation: file loading %s", animFile );
		return false;
	}

	// check the file header for validity
	const FILE_HEADER& fh = pReader->getFileHeader();

	//if(fh.Version != AnimFileVersion || fh.FileType != FileType_Anim) 
	if(fh.Version != GeomFileVersion || fh.FileType != FileType_Geom)
	{
		g_GetLog()->LogError ("CAnimObjectLoader::LoadAnimation: file version error or not an animation file: %s", animFile );
		return false;
	}

	m_nodeMap.clear();

	m_animStart = 0;
	m_animEnd = 0;
	m_secsPerTick = 1;
	m_ticksPerFrame = 1;

	// Get file name, this is a name of application.
	char fname[_MAX_PATH];
	strcpy( fname,animFile );
	StripFileExtension(fname);
	const char *sAnimName = FindFileNameInPath(fname);

	const char *sName = strchr(sAnimName,'_');
	if (sName)
	{
		sName += 1;
	}
	else
		sName = sAnimName;

	//////////////////////////////////////////////////////////////////////////
	m_currAnimation = new CAnimObject::Animation;
	m_currAnimation->name = sName;
	m_animObject->AddAnimation(m_currAnimation);

	LoadChunks( pReader,false );
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CAnimObjectLoader::LoadChunks( CChunkFileReader* pReader,bool bMakeNodes )
{
	m_numChunks = pReader->numChunks();
	m_controllers = new IController*[m_numChunks];
	memset( m_controllers,0,sizeof(IController*)*m_numChunks );

	// scan the chunks and load all controllers and time data into the animation structure
	for (int nChunk = 0; nChunk < pReader->numChunks (); ++nChunk)
	{
		// this is the chunk header in the chunk table at the end of the file
		const CHUNK_HEADER& chunkHeader = pReader->getChunkHeader(nChunk);
		// this is the chunk raw data, starts with the chunk header/descriptor structure
		const void* pChunk = pReader->getChunkData (nChunk);
		unsigned nChunkSize = pReader->getChunkSize(nChunk);

		switch (chunkHeader.ChunkType)
		{
		case ChunkType_Node:
			LoadNodeChunk( chunkHeader.ChunkID,pChunk,bMakeNodes );
			break;

		case ChunkType_Controller:
			if (chunkHeader.ChunkVersion == CONTROLLER_CHUNK_DESC_0826::VERSION)
			{
				// load and add a controller constructed from the controller chunk
				LoadControllerChunk( chunkHeader.ChunkID,pChunk );
			}
			break;
			/*
			default:
			GetLog()->LogError ("\003Unsupported controller chunk 0x%08X version 0x%08X in file %s. Please re-export the file.", chunkHeader.ChunkID, chunkHeader.ChunkVersion, strFileName.c_str());
			return -1;
			break;
			}
			*/

		case ChunkType_Timing:
			{
				// memorize the timing info
				const TIMING_CHUNK_DESC* pTimingChunk = static_cast<const TIMING_CHUNK_DESC*> (pChunk);
				m_ticksPerFrame = pTimingChunk->TicksPerFrame;
				m_secsPerTick = pTimingChunk->SecsPerTick;
				m_animStart = m_secsPerTick * m_ticksPerFrame * pTimingChunk->global_range.start;
				m_animEnd = m_secsPerTick * m_ticksPerFrame * pTimingChunk->global_range.end;
				
				m_currAnimation->secsPerFrame = m_ticksPerFrame*m_secsPerTick;
				m_currAnimation->startTime = m_animStart;
				m_currAnimation->endTime = m_animEnd;
			}
			break;
		}
	}

	InitNodes();

	delete []m_controllers;
}


//////////////////////////////////////////////////////////////////////////
void CAnimObjectLoader::LoadNodeChunk( int chunkID,const void* pChunk,bool bMakeNode )
{
	const NODE_CHUNK_DESC *pNodeChunk = static_cast<const NODE_CHUNK_DESC*>(pChunk);

	NodeDesc nd;
	nd.parentID = pNodeChunk->ParentID;
	nd.pos_cont_id = pNodeChunk->pos_cont_id;
	nd.rot_cont_id = pNodeChunk->rot_cont_id;
	nd.scl_cont_id = pNodeChunk->scl_cont_id;

	nd.pos = pNodeChunk->pos * (1.0f/100.0f);
	nd.rotate = pNodeChunk->rot;
	nd.scale = pNodeChunk->scl;
	
	CAnimObject::Node *node = 0;
	if (bMakeNode)
	{
		node = m_animObject->CreateNode( pNodeChunk->name );
		if (!node)
			return;

		// Fill node params.
		// Position must be scaled down by 100.
		node->m_tm = pNodeChunk->tm;
		node->m_tm.SetTranslationOLD( node->m_tm.GetTranslationOLD()*(1.0f/100.0f) );
		node->m_pos = nd.pos;
		node->m_rotate = nd.rotate;
		node->m_scale = nd.scale;
		m_nodeNameMap[node->m_name] = node;
	}
	else
	{
		// Load node from map.
		NodeNamesMap::iterator it = m_nodeNameMap.find(pNodeChunk->name);
		if (it == m_nodeNameMap.end())
		{
			// No such node.
			return;
		}
		node  = it->second;
	}

	// Add this node description.
	nd.node = node;
	m_nodeMap[chunkID] = nd;
}

//////////////////////////////////////////////////////////////////////////
void CAnimObjectLoader::LoadControllerChunk( int chunkID,const void* pChunk )
{
	assert( chunkID >= 0 && chunkID < m_numChunks );
	const CONTROLLER_CHUNK_DESC_0826* pCtrlChunk = static_cast<const CONTROLLER_CHUNK_DESC_0826*>(pChunk);
	IController *ctrl = 0;
	switch (pCtrlChunk->type)
	{
	case CTRL_TCB3:
		{
			CControllerTCBVec3 *ctrl = new CControllerTCBVec3;
			ctrl->Load( pCtrlChunk,m_secsPerTick );
			m_controllers[chunkID] = ctrl;
		}
		break;
	case CTRL_TCBQ:
		{
			CControllerTCBQuat *ctrl = new CControllerTCBQuat;
			ctrl->Load( pCtrlChunk,m_secsPerTick );
			m_controllers[chunkID] = ctrl;
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimObjectLoader::InitNodes()
{
	m_currAnimation->nodeAnims.resize( m_nodeMap.size() );

	NodeDescMap::iterator nit;
	// Iterate over all loaded nodes and initialize parent links, and assign controllers.
	for (nit = m_nodeMap.begin(); nit != m_nodeMap.end(); ++nit)
	{
		const NodeDesc &nd = nit->second;

		if (nd.node->m_id < 0 || nd.node->m_id >= (int)m_currAnimation->nodeAnims.size())
			continue;

		CAnimObject::NodeAnim *nodeAnim = &m_currAnimation->nodeAnims[nd.node->m_id];

		// find parent node.
		if (nd.parentID != 0)
		{
			NodeDescMap::iterator it = m_nodeMap.find(nd.parentID);
			if (it != m_nodeMap.end())
				nd.node->m_parent = it->second.node;
		}

		nodeAnim->m_pos = nd.pos;
		nodeAnim->m_rotate = nd.rotate;
		nodeAnim->m_scale = nd.scale;
		
		// find controllers.
		if (nd.pos_cont_id >= 0)
		{
			assert( nd.pos_cont_id >= 0 && nd.pos_cont_id < m_numChunks );
			nodeAnim->m_posTrack = m_controllers[nd.pos_cont_id];
			if (nodeAnim->m_posTrack)
			{
				if (nodeAnim->m_posTrack->IsLooping())
					m_currAnimation->haveLoopingController = true;
			}
		}
		if (nd.rot_cont_id >= 0)
		{
			assert( nd.rot_cont_id >= 0 && nd.rot_cont_id < m_numChunks );
			nodeAnim->m_rotTrack = m_controllers[nd.rot_cont_id];
			if (nodeAnim->m_rotTrack)
			{
				if (nodeAnim->m_rotTrack->IsLooping())
					m_currAnimation->haveLoopingController = true;
			}
		}
		if (nd.scl_cont_id >= 0)
		{
			assert( nd.scl_cont_id >= 0 && nd.scl_cont_id < m_numChunks );
			nodeAnim->m_scaleTrack = m_controllers[nd.scl_cont_id];
			if (nodeAnim->m_scaleTrack)
			{
				if (nodeAnim->m_scaleTrack->IsLooping())
					m_currAnimation->haveLoopingController = true;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimObjectLoader::LoadAnimations( const char *cgaFile )
{
	// Load all filename_***.anm files.
	char filter[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	portable_splitpath( cgaFile,drive,dir,fname,ext );
	strcat( fname,"_*");
	portable_makepath( filter, drive,dir,fname,"anm" );

	char fullpath[_MAX_PATH];
	char filename[_MAX_PATH];
	portable_makepath( fullpath, drive,dir,NULL,NULL );

	ICryPak *pack = g_GetISystem()->GetIPak();

	// Search files that match filter specification.
	_finddata_t fd;
	int res;
	intptr_t handle;
	if ((handle = pack->FindFirst( filter,&fd )) != -1)
	if (handle != -1)
	{
		do
		{
			// Animation file found, load it.
			strcpy( filename,fullpath );
			strcat( filename,fd.name );
			LoadAnimation( filename );

			res = pack->FindNext( handle,&fd );
		} while (res >= 0);
		pack->FindClose(handle);
	}
}