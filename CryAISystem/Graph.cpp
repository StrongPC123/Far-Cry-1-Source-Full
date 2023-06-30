// Graph.cpp: implementation of the CGraph class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Graph.h"
#include "Heuristic.h"
#include "CAISystem.h"


#if !defined(LINUX)
#include <assert.h>
#endif


#include "Cry_Math.h"
#include <ISystem.h>
#include <Cry_Camera.h>

#include <IRenderer.h>
#include <ILog.h>
#include <I3DEngine.h>
#include <algorithm>
#include <IConsole.h>
#include "IPhysics.h"
#include "AIObject.h"
#include "VertexList.h"
#include <CryFile.h>

#if defined(WIN32) && defined(_DEBUG) 
#include <crtdbg.h> 
#define DEBUG_NEW_NORMAL_CLIENTBLOCK(file, line) new(_NORMAL_BLOCK, file, line) 
#define new DEBUG_NEW_NORMAL_CLIENTBLOCK( __FILE__, __LINE__) 
#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGraph::CGraph(CAISystem *pSystem)
{ 

	m_pFirst = CreateNewNode(true);//new GraphNode;
	m_pSafeFirst = m_pFirst;
	m_pCurrent = m_pFirst;
	m_pCurrent->data.Reset();
	m_pCurrent->link.clear();

	nNodes = 0;
	m_pHeuristic = 0;
	m_nTagged = 0;
	m_pPathBegin = 0;
	m_pPathfinderCurrent = 0;
	m_pWalkBackCurrent = 0;
	m_bBeautifying = true;

	m_pAISystem = pSystem;
	m_lstTagTracker.reserve(1000);
	m_lstMarkTracker.reserve(1000);
}

CGraph::~CGraph()
{
	m_vNodes.clear();
	DeleteGraph(m_pSafeFirst,0);
	char str[255];
	sprintf(str,"Released %d nodes\n",nNodes);
	OutputDebugString(str);
	m_mapEntrances.clear();
	if (m_pHeuristic)
	{
		delete m_pHeuristic;
		m_pHeuristic = 0;
	}

}


GraphNode *CGraph::GetCurrent()
{
	return m_pCurrent;
}


GraphNode *CGraph::GetEnclosing(const Vec3d &pos,GraphNode *pNode, bool bOutdoorOnly)
{

	if (bOutdoorOnly)
	{
		if (!pNode)
			DebugWalk(m_pFirst,pos);
		else
			DebugWalk(pNode,pos);

		return m_pCurrent;
	}
	
	IVisArea *pGoalArea; 
	int	nGoalBuilding;
	if (!m_pAISystem->CheckInside(pos,nGoalBuilding,pGoalArea))
	{
		nGoalBuilding = -1;
		pGoalArea = NULL;
	}
 
	if (nGoalBuilding<0)
	{
		if (!pNode)
			DebugWalk(m_pFirst,pos);
		else
			DebugWalk(pNode,pos);

		m_pFirst = m_pCurrent;
		return m_pCurrent;
	}
	else
	{
		GraphNode *pEntrance = GetEntrance(nGoalBuilding,pos);
		if (!pEntrance)
		{
			if (!pGoalArea)
				AIWarning("No entrance for navigation area nr %d. The position is (%.3f,%.3f,%.3f)",nGoalBuilding,pos.x,pos.y,pos.z);
			else
				AIWarning("No entrance into some indoors or some visareas not connected trough portals (NR:%d).(%3f,%3f,%3f)",nGoalBuilding,pos.x,pos.y,pos.z);
		}
		else
		{
			if (pGoalArea)
				IndoorDebugWalk(pEntrance,pos,pGoalArea);	
			else
				IndoorDebugWalk(pEntrance,pos);	
		}
	}
	
	return m_pCurrent;
	
}



void CGraph::Connect(GraphNode *one, GraphNode *two)
{

	if (one==two)
		return;

	if (!one || !two)
		return;

	VectorOfLinks::iterator vi;
	for (vi=one->link.begin();vi!=one->link.end();vi++)
	{
			if ( (*vi).pLink == two)
				return;
	}

	GraphLink NewLink;
	NewLink.pLink = two;
	if ((one==m_pSafeFirst) || (two==m_pSafeFirst))
		NewLink.fMaxRadius = -1.f; // do not EVER go to the safe first node while tracing
	else
		NewLink.fMaxRadius = 100.f;	// default big value
//	if (one->link.size()==3) 
//		int a=5;
	one->link.push_back(NewLink);
	two->AddRef();

	// connect two to one
	Connect(two,one);

	if (m_pCurrent == m_pSafeFirst)
	{
		// connect dummy first node to graph
		if (one->nBuildingID==-1)
		{
			Connect(m_pSafeFirst,one);
			m_pCurrent=one;
			m_pFirst = m_pCurrent;
		}
	}

}

void CGraph::WriteToFile(const char *pname)
{
#ifdef __MWERKS__
#warning Code not implemented under CodeWarrior
#else

	m_vLinks.clear();
	m_vLinksDesc.clear();
	m_vBuffer.clear();
	m_lstSaveStack.clear();
	
	m_vBuffer.reserve(100000);

	GraphNode *pNext;
	GraphNode *pCurrent = m_pSafeFirst;
	while (pNext = WriteLine(pCurrent)) pCurrent=pNext;
		
	CCryFile file;
	if( false != file.Open( pname, "wb" ) )
	{
		int iNumber = m_vBuffer.size();
		int nFileVersion = BAI_FILE_VERSION;
		int nRandomNr = 0;

		file.Write( &nFileVersion, sizeof( int ) );
		file.Write( &m_vBBoxMin.x, sizeof( float ) );
		file.Write( &m_vBBoxMin.y, sizeof( float ) );
		file.Write( &m_vBBoxMin.z, sizeof( float ) );
		file.Write( &m_vBBoxMax.x, sizeof( float ) );
		file.Write( &m_vBBoxMax.y, sizeof( float ) );
		file.Write( &m_vBBoxMax.z, sizeof( float ) );

		// write the triangle descriptors
		file.Write( &iNumber, sizeof( int ) );
		file.Write( &m_vBuffer[ 0 ], iNumber * sizeof( NodeDescriptor ) );
		//iNumber = m_vLinks.size();
		//WriteFile(hf,&iNumber,sizeof(int),&written,NULL);
		//WriteFile(hf,&m_vLinks[0],iNumber*sizeof(int),&written,NULL);

		m_pAISystem->m_VertexList.WriteToFile( file );

		iNumber = m_vLinksDesc.size();
		file.Write( &iNumber, sizeof( int ) );
		if (iNumber>0) 
			file.Write( &m_vLinksDesc[ 0 ], iNumber * sizeof( LinkDescriptor ) );

		file.Close();

		m_vLinks.clear();
		m_vLinksDesc.clear();
		m_vNodes.clear();
		m_vBuffer.clear();
		//m_mapEntrances.clear();
		ClearMarks();
		
	}

#endif
}

#ifdef __MWERKS__
#warning Code not implemented under CodeWarrior
#else


GraphNode *CGraph::WriteLine( GraphNode *pNode)
{
	// write the id of the node
    	NodeDescriptor desc;
		desc.id = (INT_PTR) pNode;					//AMD Port
		if (pNode == m_pSafeFirst) desc.id=1;
	
		///desc.pArea = pNode->pArea;
		desc.building = pNode->nBuildingID;

		desc.data = pNode->data;
		desc.bEntrance = false;
		desc.bExit = false;
		desc.bCreated = pNode->bCreated;
		// check if this node is an entrance;
		EntranceMap::iterator ei;
		ei = m_mapEntrances.find(desc.building);
		if (ei!=m_mapEntrances.end())
		{
			while ( (ei!=m_mapEntrances.end()) && (ei->first == desc.building))
			{
				if ( (ei->second) == pNode )
					desc.bEntrance = true;
				++ei;
			}
		}
		else
		{
			ei=m_mapExits.find(desc.building);
			if (ei!=m_mapExits.end())
			{
				while ( (ei!=m_mapExits.end()) && (ei->first == desc.building))
				{
					if ( (ei->second) == pNode )
						desc.bExit = true;
					++ei;
				}
			}
		}
		//desc.bEntrance = (m_mapEntrances.find(desc.building) != m_mapEntrances.end());

		MarkNode(pNode);

		if (!pNode->vertex.empty())
		{
			desc.nObstacles = pNode->vertex.size();
			if (desc.nObstacles>10)
				desc.nObstacles=10;

			int i=0;
			ObstacleIndexVector::iterator li;
			for (li=pNode->vertex.begin();li!=pNode->vertex.end() && i<10 ;li++,i++)
			{
				desc.obstacle[i] = (*li);
				//desc.obstacle[i].vPos = (*li).vPos;
				//desc.obstacle[i].vDir = (*li).vDir;
			}

			if (i==10)
				m_pAISystem->m_pSystem->GetILog()->Log("\003FOUND INDOOR WAYPOINT WITH MORE THAN 10 HIDEPOINTS LINKED TO IT. SURPLUS HIDEPOINTS IGNORED");
		}
		else
			desc.nObstacles = 0;
 
		m_vBuffer.push_back(desc);

		
		// now write the links
		// first size
/*		int size = pNode->link.size();
		
		m_vLinks.push_back(desc.id);
		m_vLinks.push_back(size);
		
		VectorOfLinks::iterator vi;
		for (vi=pNode->link.begin();vi!=pNode->link.end();vi++)
		{
			GraphNode *pToPush = (*vi).pLink;
			if (pToPush == m_pSafeFirst)
				m_vLinks.push_back(1);
			else
				m_vLinks.push_back((int) pToPush);

			m_lstSaveStack.remove(pToPush);

			if (pToPush->mark)
				continue;


			m_lstSaveStack.push_back(pToPush);
		}*/

	
	

		VectorOfLinks::iterator vi;
		for (vi=pNode->link.begin();vi!=pNode->link.end();vi++)
		{
			GraphNode *pToPush = (*vi).pLink;
			
			LinkDescriptor ld;
			ld.nSourceNode = desc.id;
			if (pToPush == m_pSafeFirst)
				ld.nTargetNode = 1;
			else
				ld.nTargetNode = (INT_PTR) pToPush;
			ld.fMaxPassRadius = (*vi).fMaxRadius;
			ld.nStartIndex = (*vi).nStartIndex;
			ld.nEndIndex = (*vi).nEndIndex;
			ld.vEdgeCenter = (*vi).vEdgeCenter;
			ld.vWayOut = (*vi).vWayOut;

			m_vLinksDesc.push_back(ld);

			if (pToPush->mark)
			{
				m_lstSaveStack.remove(pToPush);
				continue;
			}

			if (std::find(m_lstSaveStack.begin(),m_lstSaveStack.end(),pToPush) == m_lstSaveStack.end())
				m_lstSaveStack.push_back(pToPush);
		}

//	Disconnect(pNode);
//	if (pNode == m_pFirst)
//		delete pNode;

	if (m_lstSaveStack.empty())
		return NULL;
	else
	{
		GraphNode *pNext;
		while (!m_lstSaveStack.empty())
		{
			pNext = m_lstSaveStack.front();
			m_lstSaveStack.pop_front();

			if (!pNext->mark)
				break;
		}

		if (pNext->mark)
			return NULL;
		else
			return pNext;
	}

}
#endif


void CGraph::DebugWalk(GraphNode *pNode, const Vec3d &pos)
{
	// get out of indoor
	if (pNode->nBuildingID>=0)
	{
		pNode=GetEntrance(pNode->nBuildingID,pos);
		VectorOfLinks::iterator vli,iend = pNode->link.end();
		for (vli=pNode->link.begin();vli!=iend;++vli)
			if ((*vli).pLink->nBuildingID<0)
			{
				pNode = (*vli).pLink;
				break;
			}
	}

	ClearMarks();
	GraphNode *pNextNode = pNode;
	GraphNode *pPrevNode = 0;
	int iterations=0;
	while (pPrevNode != pNextNode)
	{
		iterations++;
		pPrevNode = pNextNode;
		pNextNode = GREEDYStep(pPrevNode,pos);
	}

	m_pAISystem->f7+=iterations;
	m_pAISystem->f7/=2.f;
	
	m_pCurrent = pNextNode; // or pPrevNode, they are the same
	ClearMarks();
	m_mapGreedyWalkCandidates.clear();

	if (!m_pCurrent)
		CryError("[AIERROR] located in NULL graph node... Try regenerating triangulation, or submit a bug report.");
 
}



void CGraph::DrawPath(IRenderer *pRenderer)
{

	

	//if (m_lstVisited.empty()) return;

	ListNodes::iterator i;
	CandidateMap::iterator  ci;
	pRenderer->SetMaterialColor(1,0,0,1);
	//ci=m_lstVisited.begin();
	GraphNode *pStart = (ci->second);
//	for (ci++;ci!=m_lstVisited.end();ci++)
	{
		GraphNode *pEnd = (ci->second);

		Vec3d stpos = pStart->data.m_pos;
		Vec3d endpos = pEnd->data.m_pos;
		stpos.z+=1;
		endpos.z+=1;

		pRenderer->DrawLine(stpos, endpos );

		pStart = pEnd;

	}

	if (m_lstPath.empty()) return;
	

	Vec3d vStartPos;
	ListPositions::iterator pi;
	pRenderer->ResetToDefault();
	pi=m_lstPath.begin();
	vStartPos = (*pi);
	for (pi++;pi!=m_lstPath.end();pi++)
	{
		//GraphNode *pEnd = (*i);
		Vec3d vEndPos = (*pi);

//		Vec3d stpos = pStart->data.m_pos;
//		Vec3d endpos = pEnd->data.m_pos;
		vStartPos.z+=1;
		vEndPos.z+=1;

		pRenderer->DrawLine(vStartPos, vEndPos );

		vStartPos = vEndPos;
	}

	
}

void CGraph::GetFieldCenter(Vec3d &pos)
{
		pos = m_pCurrent->data.m_pos;
}


void CGraph::DEBUG_DrawCenters(GraphNode *pNode, IRenderer *pRenderer, int dist)
{
	return;

	if (dist > 10 ) return;
	//if (pNode->bDebug) return;

	//pNode->bDebug = true;
	pRenderer->ResetToDefault();
	if (pNode->data.bWater)
		pRenderer->SetMaterialColor(0,0,1.0,1.0);
	else
		pRenderer->SetMaterialColor(pNode->data.fSlope,(float)(1.0-pNode->data.fSlope),(float) (1.0-pNode->data.fSlope),1.0);
	
	pRenderer->DrawBall(pNode->data.m_pos,1.5f);

	VectorOfLinks::iterator vi;
	for (vi=pNode->link.begin();vi!=pNode->link.end();vi++)
	{
		Vec3d start, end;
		start = pNode->data.m_pos;
		start.x+=0.5f;
		start.y+=0.5f;
		end = (*vi).pLink->data.m_pos;
		end.y+=0.5f;
		end.x+=0.5f;
		pRenderer->DrawLine(start, end);
		DEBUG_DrawCenters((*vi).pLink,pRenderer,dist+1);
	}
}



int CGraph::WalkAStar(GraphNode *pBegin, GraphNode *pEnd, int &nIterations)
{
		m_lstCurrentHistory.clear();
		ClearPath();	// clear the previously generated path
	//	m_lstVisited.clear();
		if (!ClearTags())	// clear all the tags in the diagram
			return PATHFINDER_CLEANING_GRAPH;
			

		if ((!pBegin) || (!pEnd)) return PATHFINDER_NOPATH;

		// lets check if last generated path was similar to this one
		if (!m_lstLastPath.empty())
		{
			if (m_lstLastPath.back() == pEnd)
			{
				if (CanReuseLastPath(pBegin))
					return PATHFINDER_BEAUTIFYINGPATH;
			}
		}

	
		m_pPathfinderCurrent = pBegin;
		m_pPathBegin = pBegin;

		m_nAStarDistance = 0;
		m_pPathfinderCurrent->fDistance = 0;

		//m_fDistance = (pBegin->data.m_pos - pEnd->data.m_pos).GetLength();
		m_mapCandidates.clear();
		m_pPathfinderCurrent = ASTARStep(pBegin, pEnd);
		while (m_pPathfinderCurrent && !m_mapCandidates.empty() && (m_pPathfinderCurrent != pEnd) && (nIterations--))
			m_pPathfinderCurrent = ASTARStep(m_pPathfinderCurrent, pEnd);

		if (!m_pPathfinderCurrent)
			return PATHFINDER_NOPATH;
		if (m_pPathfinderCurrent == pEnd)
			return PATHFINDER_WALKINGBACK;

		return PATHFINDER_STILLTRACING;

}

GraphNode * CGraph::ASTARStep(GraphNode *pBegin, GraphNode *pEnd)
{
	TagNode(pBegin);
	if (pEnd == pBegin) 
	{
		pEnd->fHeuristic = 10000 - (float)m_nAStarDistance;
		return pEnd; // reached the end
	}

		// this piece evaluates simple distance heuristic for backtracking----------
	///float thisdist = (pBegin->data.m_pos - m_pPathBegin->data.m_pos).GetLength();
	//pBegin->fHeuristic =  1.f - (thisdist / m_fDistance);
	//--------------------------------------------------------------------------
	pBegin->fHeuristic = 10000 - (float)m_nAStarDistance;
	m_nAStarDistance++;

	VectorOfLinks::iterator vi;
	for (vi=pBegin->link.begin();vi!=pBegin->link.end();vi++)
	{
//		if ((*vi).fMaxRadius >= 1.f)
		if ((*vi).fMaxRadius >= m_pRequester->m_fPassRadius)
		{
				EvaluateNode( (*vi).pLink, pEnd, pBegin);
		}
	}



	 
	if (!m_mapCandidates.empty())
	{
		CandidateHistoryMap::reverse_iterator ci = m_mapCandidates.rbegin();

		GraphNode *pNextNode = 0;
		if (ci!=m_mapCandidates.rend())
		{
			pNextNode = (ci->second).pNode;
			
			while ((pNextNode == pBegin)||(pNextNode->tag))
			{
				m_mapCandidates.erase((++ci).base());
				ci = m_mapCandidates.rbegin();
				if (ci==m_mapCandidates.rend())
					break;
				pNextNode = (ci->second).pNode;
			}
		}

		if (ci==m_mapCandidates.rend())
			return 0;

		m_lstCurrentHistory = (ci->second).lstParents;
		float f = ci->first;
		m_mapCandidates.erase((++ci).base());
		if (GetAISystem()->m_cvDrawPath->GetIVal()==2) 
		 m_lstVisited.insert(CandidateMap::iterator::value_type(f,pNextNode));
		return pNextNode;
	}
	else
		return 0;

}

void CGraph::TagNode(GraphNode *pNode)
{
	pNode->tag = true;
	m_lstTagTracker.push_back(pNode);
}

bool CGraph::ClearTags()
{

	int nIterations = PATHFINDER_ITERATIONS;

	while (!m_lstTagTracker.empty() && (nIterations--))
	{
		GraphNode *pLastNode = m_lstTagTracker.back();
		pLastNode->tag = false;
		pLastNode->fHeuristic = -9999.f;
		m_lstTagTracker.pop_back();
	}
		
	if (!m_lstTagTracker.empty()) 
		return false; // we are still cleaning up

	return true;
}

void CGraph::EvaluateNode(GraphNode *pNode,GraphNode *pEnd, GraphNode *pParent)
{
	if (!pNode) return;
	if (pNode->tag) return;
	float desirability=0;
	float thisdist = (pNode->data.m_pos - m_vRealPathfinderEnd).GetLength();

	desirability = 1.f - (thisdist / m_fDistance) * 0.5f;
	desirability += m_pHeuristic->Estimate(pNode, this) * 0.5f;

	NodeWithHistory nwh;
	nwh.pNode = pNode;
	if (m_lstCurrentHistory.size() > 1	)
		m_lstCurrentHistory.pop_back();
	m_lstCurrentHistory.push_front(pParent);
	nwh.lstParents = m_lstCurrentHistory;
	m_mapCandidates.insert(CandidateHistoryMap::iterator::value_type(desirability,nwh));
}

int CGraph::ContinueAStar(GraphNode *pEnd, int &nIterations)
{
		if (!pEnd) return PATHFINDER_NOPATH;
		if (!m_pHeuristic) return PATHFINDER_NOPATH;

		//int nIterations = PATHFINDER_ITERATIONS;

		m_pPathfinderCurrent = ASTARStep(m_pPathfinderCurrent, pEnd);
		while (!m_mapCandidates.empty() && (m_pPathfinderCurrent != pEnd) && (nIterations--))
			m_pPathfinderCurrent = ASTARStep(m_pPathfinderCurrent, pEnd);

		if (!m_pPathfinderCurrent)
			return PATHFINDER_NOPATH;
		if (m_pPathfinderCurrent == pEnd)
			return PATHFINDER_WALKINGBACK;

	  return PATHFINDER_STILLTRACING;
}

int CGraph::WalkBack(GraphNode *pBegin, GraphNode *pEnd, int &nIterations)
{
	//int nIterations = PATHFINDER_ITERATIONS;
	//GraphNode *pCurrent;
	if (!m_pWalkBackCurrent)
	{
		m_lstNodeStack.clear();
		m_lstPath.clear();
		m_pWalkBackCurrent = pBegin;
	}
	//else
		//m_pWalkBackCurrent = m_pWalkBackCurrent;
	
	float maxHeur = m_pWalkBackCurrent->fHeuristic;
	while (m_pWalkBackCurrent!=pEnd && --nIterations)
	{
		m_lstPath.push_front(m_pWalkBackCurrent->data.m_pos);		// push in path
		m_lstNodeStack.push_front(m_pWalkBackCurrent);						// push in nodestack
		
		GraphNode *pNext = 0;
		float maxheur = m_pWalkBackCurrent->fHeuristic;
		VectorOfLinks::iterator vi;
		for (vi=m_pWalkBackCurrent->link.begin(); vi!=m_pWalkBackCurrent->link.end(); vi++)
		{
			GraphNode *pLink = (*vi).pLink;
			if (pLink->fHeuristic > maxheur && (*vi).fMaxRadius>=1.f)
			{
				maxheur = pLink->fHeuristic;
				pNext = pLink;
			}
		}

		m_pWalkBackCurrent->fHeuristic = -9999.f;
		
		if (pNext)
			m_pWalkBackCurrent = pNext;
		else
		{
			// dead end hit... retrace
			// try to continue moving with a revised heuristic
			for (vi=m_pWalkBackCurrent->link.begin(); vi!=m_pWalkBackCurrent->link.end(); vi++)
			{
				GraphNode *pLink = (*vi).pLink;
				if (pLink->fHeuristic > m_pWalkBackCurrent->fHeuristic)
				{
					maxheur = pLink->fHeuristic;
					pNext = pLink;
				}
			}

			if (pNext)
				m_pWalkBackCurrent = pNext;
			else
			{
				if (!m_lstPath.empty())
					m_lstPath.pop_front();
				if (!m_lstNodeStack.empty())
					m_lstNodeStack.pop_front();
				if (m_lstNodeStack.empty())
					return PATHFINDER_NOPATH;;
				m_pWalkBackCurrent = m_lstNodeStack.front();
				if (!m_lstNodeStack.empty())
					m_lstNodeStack.pop_front();
				if (!m_lstPath.empty())
					m_lstPath.pop_front();
			}
			
		}

//		if (std::find(m_lstNodeStack.begin(),m_lstNodeStack.end(),pCurrent) != m_lstNodeStack.end())
	//			DEBUG_BREAK;
	}

	if (m_pWalkBackCurrent == pEnd)
	{
		m_pWalkBackCurrent = 0;
		m_mapGreedyWalkCandidates.clear();
		if (std::find(m_lstNodeStack.begin(),m_lstNodeStack.end(),pEnd)==m_lstNodeStack.end())
		{
			m_lstNodeStack.push_front(pEnd);
			m_lstPath.push_front(pEnd->data.m_pos);
		}
		m_lstLastPath.clear();
		m_lstLastPath.insert(m_lstLastPath.begin(),m_lstNodeStack.begin(),m_lstNodeStack.end());
		return PATHFINDER_BEAUTIFYINGPATH;
	}
	else
		return PATHFINDER_WALKINGBACK;
}



void CGraph::ClearPath()
{
	m_lstPath.clear();
}

void CGraph::DeleteGraph(GraphNode *pNode, int depth)
{
	  
	VectorOfLinks::iterator vi;

	m_lstDeleteStack.push_front(pNode);

	while (!m_lstDeleteStack.empty())
	{
		GraphNode *pCurrentNode = m_lstDeleteStack.front();
		m_lstDeleteStack.pop_front();
		
		// put all links into the node stack
		for (vi=pCurrentNode->link.begin();vi!=pCurrentNode->link.end();vi++)
		{
			GraphNode *pLink =(*vi).pLink;
			if (std::find(m_lstDeleteStack.begin(),m_lstDeleteStack.end(),pLink) == m_lstDeleteStack.end())
				m_lstDeleteStack.push_front(pLink);
		}

		Disconnect(pCurrentNode);
		/*if (!pCurrentNode->link.empty())
		{
				// delink this node from all his adjacent nodes
				for (vi=pCurrentNode->link.begin();vi!=pCurrentNode->link.end();vi++)
				{
					GraphNode *pLink = (*vi);
					if ((found = std::find(pLink->link.begin(),pLink->link.end(),pCurrentNode)) != pLink->link.end())
					{
						(*found)->Release();
						pLink->link.erase(found);
					}
				}

				
				GraphNode *pNext= pCurrentNode->link.back();
				m_lstNodeStack.push_front(pNext);
				pCurrentNode->link.pop_back();
		}
		else
		{
			if (pCurrentNode != m_pFirst)
				DeleteNode(pCurrentNode);
			else
				delete m_pFirst;
			//if (pCurrentNode->Release())
			//{
			//	delete pCurrentNode;
			//	nNodes++;
			//}
			m_lstNodeStack.pop_front();
		}
		*/
		if (pCurrentNode == m_pSafeFirst)
			m_pSafeFirst = 0;
	}

	
}

void CGraph::ClearDebugFlag(GraphNode *pNode)
{
	//if (!pNode->bDebug) return;

//	pNode->bDebug = false;

	//VectorOfLinks::iterator vi;
	//for (vi=pNode->link.begin();vi!=pNode->link.end();vi++)
	//	ClearDebugFlag((*vi).pLink);
}

GraphNode * CGraph::CheckClosest(GraphNode *pCurrent, const Vec3d &pos)
{

	if (!pCurrent)
	{
		DebugWalk(m_pCurrent,pos);
		return m_pCurrent;
	}
	else
	{
		float dist = (pCurrent->data.m_pos - pos).GetLength();
		GraphNode *pReturnNode = pCurrent;

		VectorOfLinks::iterator si;
		for (si=pCurrent->link.begin();si!=pCurrent->link.end();si++)
		{
			GraphNode *pNode = (*si).pLink;
			float cmpdist = (pNode->data.m_pos - pos).GetLength();
			if (cmpdist < dist)
				pReturnNode = pNode;
		}

		if (pReturnNode != pCurrent)
			return CheckClosest(pReturnNode, pos);
		else
			return pReturnNode;
	}


}

void CGraph::Disconnect(GraphNode * pDisconnected, bool bDelete)
{
	// if the node we are disconnecting is the current node, move the current 
	// to one of his links, or the root if it has no links

	if (m_pSafeFirst)
	{
		if (m_pSafeFirst->link.empty())
			return;
	}


	if (pDisconnected == m_pCurrent)
	{
		if (!pDisconnected->link.empty())
		{
			m_pCurrent = pDisconnected->link.front().pLink;
		}
		else
			m_pCurrent = m_pSafeFirst;
	}



	// if its the root that is being disconnected, move it
	if (m_pFirst == pDisconnected)
	{
		if (!pDisconnected->link.empty())
		{
			m_pFirst = pDisconnected->link.front().pLink;
		}
		else
		{
			if (m_pFirst!=m_pSafeFirst)
				m_pFirst = m_pSafeFirst;
			else
				m_pFirst = 0;
		}

	}



	// now disconnect this node from its links
	if (!pDisconnected->link.empty())
	{
		VectorOfLinks::iterator vi;
		for (vi=pDisconnected->link.begin();vi!=pDisconnected->link.end();vi++)
		{
			GraphNode *pLink = (*vi).pLink;
			if (!pLink->link.empty())
			{
				VectorOfLinks::iterator li;
				for (li=pLink->link.begin();li!=pLink->link.end();li++)
				{
					GraphNode *pBackLink = (*li).pLink;
					if (pBackLink == pDisconnected)
					{
						pLink->link.erase(li);
						pDisconnected->Release();
						pLink->Release();
						break;
					}
				} // li
			}
		} // vi
	}


	pDisconnected->link.clear();

	if (bDelete)
		DeleteNode(pDisconnected);

	if (!m_pSafeFirst)
		return;

	if (pDisconnected != m_pSafeFirst && m_pSafeFirst->link.empty())
	{
		GraphNode *pFirst = m_pFirst;
		// we have disconnected the link to the dummy safe node - relink it to any outdoor node of the graph
		if (pFirst == m_pSafeFirst)
		{
			if (m_pCurrent == m_pSafeFirst)
			{
				// try any entrance
				if (!m_mapEntrances.empty())
					pFirst= (m_mapEntrances.begin()->second);
				else
				{
					AIError("!Could not recover from deletion of Safe graph node. Try deleting .bai files and regenerating.");
					return;
				}

			}
			else
				pFirst = m_pCurrent;
		}

		if (pFirst->nBuildingID==-1)	
			Connect(m_pSafeFirst,pFirst);
		else
		{
			GraphNode *pEntrance = GetEntrance(pFirst->nBuildingID,Vec3d(0,0,0));
			if (pEntrance)
			{
				VectorOfLinks::iterator vli,iend = pEntrance->link.end();
				for (vli = pEntrance->link.begin();vli!=iend;++vli)
					if ( (*vli).pLink->nBuildingID == -1 )
					{
						Connect(m_pSafeFirst,(*vli).pLink);
						break;
					}
			}
		}
	}

}

// walk that will always produce a result, for indoors
void CGraph::IndoorDebugWalk(GraphNode * pNode, const Vec3d & pos, IVisArea *pTargetArea)
{

	if (pNode->pArea)
	{
		float this_dist = (pos - pNode->data.m_pos).GetLength();
		m_mapGreedyWalkCandidates.insert(CandidateMap::iterator::value_type(this_dist,pNode));
	}

	FillGreedyMap(pNode,pos,pTargetArea,false);


	if (m_mapGreedyWalkCandidates.empty())
	{
		AIWarning("No nodes found for this indoor or navigation modifier area apart from entrance!!");
		m_pCurrent = pNode;
	}
	else
	{
		m_pCurrent = (m_mapGreedyWalkCandidates.begin())->second;
		ClearMarks();
		m_mapGreedyWalkCandidates.clear();

	}

	if (!m_pCurrent)
		CryError("[AIERROR] located in NULL graph node... Try regenerating triangulation, or submit a bug report.");
}

// Clears the tags of the graph without time-slicing the operation
void CGraph::ClearTagsNow(void)
{
	while (!ClearTags());
}


// Check whether a position is within a node's triangle
bool CGraph::PointInTriangle(const Vec3d & pos, GraphNode * pNode)
{
	if (pNode->vertex.empty())
		return false;

	bool bSide=false;
	// check first and last 
	//Vec3d edge = (pNode->vertex.back()).vPos - (pNode->vertex.front()).vPos;
	Vec3d vFront = m_pAISystem->m_VertexList.GetVertex(pNode->vertex.front()).vPos;
	Vec3d edge = m_pAISystem->m_VertexList.GetVertex(pNode->vertex.back()).vPos - vFront;
	Vec3d test = pos - vFront;

	float cross = edge.x * test.y - edge.y * test.x;


	if (cross>0)
		bSide = true;
	
	for (unsigned int i=1;i<pNode->vertex.size();i++)
	{
		Vec3d vI = m_pAISystem->m_VertexList.GetVertex(pNode->vertex[i]).vPos;
		edge =  m_pAISystem->m_VertexList.GetVertex(pNode->vertex[i-1]).vPos - vI;
		test = pos - vI;
		cross = edge.x * test.y - edge.y * test.x;

		if ((cross<0) && bSide)	
			return false;
		if ((cross>0) && !bSide)	
			return false;

	}
	
	return true;
}

// uses mark for internal graph operation without disturbing the pathfinder
void CGraph::MarkNode(GraphNode * pNode)
{
	pNode->mark = true;
	m_lstMarkTracker.push_back(pNode);
}



// clears the marked nodes
void CGraph::ClearMarks(bool bJustClear)
{
	if (bJustClear)
	{
		m_lstMarkTracker.resize(0);
		return;
	}

	while (!m_lstMarkTracker.empty())
	{
		m_lstMarkTracker.back()->mark = false;
		m_lstMarkTracker.pop_back();
	}
}

// iterative function to quickly converge on the target position in the graph
GraphNode * CGraph::GREEDYStep(GraphNode * pBegin, const Vec3d & pos, bool bIndoor)
{
 
	MarkNode(pBegin);

	if (bIndoor)
	{
		//indoor

		float d_dist = (pos - pBegin->data.m_pos).GetLength();
		if (d_dist < 1.f)
			return pBegin;

		VectorOfLinks::iterator vi;
		if (pBegin->link.empty())
			return pBegin;
		for (vi=pBegin->link.begin();vi!=pBegin->link.end();vi++)
		{
			GraphNode *pLink = (*vi).pLink;
			if ((pLink->mark) || (pLink->nBuildingID==-1)) // don't go outside
					continue;
			float this_dist = (pos - pLink->data.m_pos).GetLength();
			m_mapGreedyWalkCandidates.insert(CandidateMap::iterator::value_type(this_dist,pLink));
		}

		if (!m_mapGreedyWalkCandidates.empty())
		{
			CandidateMap::iterator ci = m_mapGreedyWalkCandidates.end();
			ci--;
			GraphNode *pNextNode = (ci->second);
			if (pNextNode == pBegin)
			{
				ci--;
				pNextNode = (ci->second);
			}
			m_mapGreedyWalkCandidates.erase(ci);
			return pNextNode;
		}
		else
		{
				return pBegin;
		}

	}
	else
	{
		// outdoor
		if (PointInTriangle(pos,pBegin))
		 	return pBegin;		// we have arrived
	
		if (pBegin == m_pSafeFirst)
		{
			if (pBegin->link.empty())
				return pBegin;
			m_mapGreedyWalkCandidates.insert(CandidateMap::iterator::value_type(0,pBegin->link.front().pLink));
		}
		else
		{
			VectorOfLinks::iterator vi;
			if (pBegin!=m_pSafeFirst || !pBegin->mark)
			{
				for (vi=pBegin->link.begin();vi!=pBegin->link.end();vi++)
				{
					GraphNode *pLink = (*vi).pLink;
					if (pLink->mark || (pLink->nBuildingID!=-1)) // dont go in marked or indoors
						continue;
					//	float this_dist = (pos - pLink->data.m_pos).GetLength();
					if (pBegin->vertex.empty())
						continue;

					Vec3d midpoint = (m_pAISystem->m_VertexList.GetVertex(pBegin->vertex[(*vi).nStartIndex]).vPos + m_pAISystem->m_VertexList.GetVertex(pBegin->vertex[(*vi).nEndIndex]).vPos)/2.f;
					midpoint.z = pos.z;
					Vec3d dir1 = pos-midpoint;
					Vec3d vWayOut = (*vi).vWayOut;
					vWayOut.z = 0.f;//pos.z;
					dir1.Normalize();

					float this_dist = (1.f-dir1.Dot(vWayOut));
					this_dist*= (pLink->data.m_pos - pos).GetLength()/30.f;

					m_mapGreedyWalkCandidates.insert(CandidateMap::iterator::value_type(this_dist,pLink));
				}
			}

		}

		if (!m_mapGreedyWalkCandidates.empty())
		{

			GraphNode *pNextNode;
			do {
			if (m_mapGreedyWalkCandidates.empty())
			{
				AIWarning("Could not locate position (%.3f,%.3f,%.3f) in triangulation. Call Petar!",pos.x,pos.y,pos.z);
				return pBegin;
			}
			pNextNode= (m_mapGreedyWalkCandidates.begin()->second);
			m_mapGreedyWalkCandidates.erase(m_mapGreedyWalkCandidates.begin());

			} while (pNextNode == pBegin);

			return pNextNode;
			
		}
		else
			return pBegin;
	}
	

		

}

// adds an entrance for easy traversing later
void CGraph::AddIndoorEntrance(int nBuildingID, GraphNode* pNode, bool bExitOnly)
{

	if (m_pSafeFirst)
	{
		if (m_pSafeFirst->link.empty())
			return;
	}

	if (bExitOnly)
		m_mapExits.insert(EntranceMap::iterator::value_type(nBuildingID,pNode));
	else
		m_mapEntrances.insert(EntranceMap::iterator::value_type(nBuildingID,pNode));
	
	if (pNode->link.empty())
	{
		// it has to be connected to the outside
		GraphNode *pOutsideNode = GetEnclosing(pNode->data.m_pos,0,true);
		Connect(pNode,pOutsideNode);
	}
	else
	{
		VectorOfLinks::iterator gi;
		bool bOutsideLink = false;
		for (gi=pNode->link.begin();gi!=pNode->link.end();gi++)
		{
			if ( (*gi).pLink->nBuildingID == -1 )
				bOutsideLink = true;
		}

		if (!bOutsideLink)
		{
				// it has to be connected to the outside
				GraphNode *pOutsideNode = GetEnclosing(pNode->data.m_pos,0,true);
				Connect(pNode,pOutsideNode);
		}

	}

	if (bExitOnly)
	{
		VectorOfLinks::iterator gi;
		for (gi=pNode->link.begin();gi!=pNode->link.end();gi++)
		{
			if ( (*gi).pLink->nBuildingID == -1 )
			{
				GraphNode *pOutNode = (*gi).pLink;
				VectorOfLinks::iterator ii;
				for (ii=pOutNode->link.begin();ii!=pOutNode->link.end();ii++)
				{
					if ( (*ii).pLink == pNode)
						(*ii).fMaxRadius = 0;
				}
				
			}
		}

	}
}

// Reads the AI graph from a specified file
bool CGraph::ReadFromFile(const char * szName)
{
#ifdef __MWERKS__
#warning Code not implemented under CodeWarrior
#else

	CCryFile file;;
	if (file.Open( szName,"rb"))
	{
		ReadNodes( file );
		return true;
	}
	//[Timur]
	/*

	HANDLE hf;
	hf = CreateFile(szName,GENERIC_READ,0,NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
	
	if (hf != INVALID_HANDLE_VALUE)
	{
		ReadNodes(hf);
		CloseHandle(hf);
		return true;
	}
	*/
	
	return false;
		
#endif  
}

#ifdef __MWERKS__
#warning Code not implemented under CodeWarrior
#else
// reads all the nodes in a map
bool CGraph::ReadNodes( CCryFile &file )
{
	//assert(_heapchk()==_HEAPOK);
	
	//DWORD read;
	int iNumber;
	Vec3d mins,maxs;

	m_pAISystem->m_pSystem->GetILog()->UpdateLoadingScreen("\003[AISYSTEM] Verifying BAI file version");
	file.Read(&iNumber, sizeof(int) );
	if (iNumber != BAI_FILE_VERSION)
	{
		m_pAISystem->m_pSystem->GetILog()->UpdateLoadingScreen("\002$3[AISYSTEM WARNING] Wrong BAI file version!! Delete them and regenerate them in the editor.");
		return false;
	}

	m_pAISystem->m_pSystem->GetILog()->UpdateLoadingScreen("\003[AISYSTEM] Reading BBOX");
	file.Read( &mins.x, sizeof(float) );
	file.Read( &mins.y, sizeof(float) );
	file.Read( &mins.z, sizeof(float) );

	file.Read( &maxs.x, sizeof(float) );
	file.Read( &maxs.y, sizeof(float) );
	file.Read( &maxs.z, sizeof(float) );

	SetBBox(mins,maxs);

	m_pAISystem->m_pSystem->GetILog()->UpdateLoadingScreen("\003[AISYSTEM] Reading node descriptors");

	file.Read( &iNumber, sizeof(int) );

	if (iNumber>0) 
	{
		m_vBuffer.resize(iNumber);
		file.Read( &m_vBuffer[0], iNumber*sizeof(NodeDescriptor) );
	} 

	m_vNodes.clear();
//	m_vNodes.resize(iNumber);

	m_pAISystem->m_pSystem->GetILog()->UpdateLoadingScreen("\003[AISYSTEM] Creating graph nodes");
	
	I3DEngine *pEngine = m_pAISystem->m_pSystem->GetI3DEngine();
	NodeBuffer::iterator ni;
	int index=0;
	for (ni=m_vBuffer.begin();ni!=m_vBuffer.end();ni++,index++)
	{
		NodeDescriptor buffer = (*ni);
		GraphNode *pNode = CreateNewNode(!buffer.bCreated);//new GraphNode;//&m_vNodes[index];
		
		if (buffer.bCreated)
			m_pAISystem->CheckInside(buffer.data.m_pos,pNode->nBuildingID,pNode->pArea);
		else
		{
			pNode->nBuildingID=-1;
			pNode->pArea = 0;
		}

		pNode->data = buffer.data;
		pNode->vertex.clear();
		if (buffer.nObstacles)
		{
			pNode->vertex.reserve(buffer.nObstacles);
			for (int i=0;i<buffer.nObstacles;i++)
			{
				int nVertexIndex = buffer.obstacle[i];
				//ObstacleData od = buffer.obstacle[i];
				pNode->vertex.push_back(nVertexIndex);
				//pNode->vertex.push_back(buffer.obstacle[i]);
	
			}
		}
		m_mapReadNodes.insert(EntranceMap::iterator::value_type((EntranceMap::key_type)buffer.id,pNode));

		if (buffer.bEntrance)
			m_mapEntrances.insert(EntranceMap::iterator::value_type(pNode->nBuildingID,pNode));

		if (buffer.bExit)
			m_mapExits.insert(EntranceMap::iterator::value_type(pNode->nBuildingID,pNode));

	//	if ((pNode->nBuildingID < 0) && (pNode->vertex.empty()) && buffer.id!=1)
	//	AIWarning("![AIERROR] A node was found that had buildingid:%d during export, but now it has buildingid:-1 [pos:(x:%.3f,y:%.3f,z:%.3f)]. Please make sure that all your navigation modifiers are correctly exported.",
		//			buffer.building,buffer.data.m_pos.x,buffer.data.m_pos.y,buffer.data.m_pos.z);

		
	}

	m_pAISystem->m_pSystem->GetILog()->UpdateLoadingScreen("\003[AISYSTEM] Reading vertex list");
	
	m_pAISystem->m_VertexList.ReadFromFile(file);

	
	m_pAISystem->m_pSystem->GetILog()->UpdateLoadingScreen("\003[AISYSTEM] Reading links");
	
/*	file.Read( &iNumber, sizeof(int) );
	m_vLinks.resize(iNumber);
	file.Read( &m_vLinks[0], iNumber * sizeof(int) );
	*/

	file.Read( &iNumber, sizeof(int) );
	if (iNumber>0) 
	{
		m_vLinksDesc.resize(iNumber);
		file.Read( &m_vLinksDesc[0], iNumber * sizeof(LinkDescriptor) );
	}

	EntranceMap::iterator ei,link;

	ei = m_mapReadNodes.find(1);
	if (ei==m_mapReadNodes.end())
	{
		m_pAISystem->m_pSystem->GetILog()->UpdateLoadingScreen("\001[AIERROR]   FIRST NODE NOT FOUND !!!!");
		return false;
	}
	else
	{
		Disconnect(m_pSafeFirst);
	 //delete m_pFirst;
	 m_pSafeFirst = (ei->second);
	 m_pFirst = m_pSafeFirst;
	 m_pCurrent = m_pSafeFirst;
	}

	m_pAISystem->m_pSystem->GetILog()->UpdateLoadingScreen("\003[AISYSTEM] Reconnecting links (might take some time:)");

	if (!m_vLinksDesc.empty())
	{
		LinkDescBuffer::iterator iend = m_vLinksDesc.end();
		for (LinkDescBuffer::iterator ldbi=m_vLinksDesc.begin();ldbi!=iend;++ldbi)
		{
			LinkDescriptor &ldb = (*ldbi);
			ei=m_mapReadNodes.find((EntranceMap::key_type)ldb.nSourceNode);
			GraphNode *pNode = ei->second;
			link = m_mapReadNodes.find((EntranceMap::key_type)ldb.nTargetNode);
			if (link==m_mapReadNodes.end() || ei==m_mapReadNodes.end())
				AIError("!Read a link to a node which could not be found!");
			else
			{
				Connect(pNode,link->second);

				for (VectorOfLinks::iterator vli=pNode->link.begin();vli!=pNode->link.end();vli++)
					if ( (*vli).pLink == link->second )
					{
						(*vli).fMaxRadius = ldb.fMaxPassRadius;
						(*vli).nStartIndex = ldb.nStartIndex;
						(*vli).nEndIndex = ldb.nEndIndex;
						(*vli).vWayOut = ldb.vWayOut;
						(*vli).vEdgeCenter = ldb.vEdgeCenter;
						break;
					}
			}
		}
	}
		
	/*int count = 0;
	while (count < iNumber)
	{
		// get id of node
		ei = m_mapReadNodes.find(m_vLinks[count++]);
		GraphNode *pNode = (ei->second);
    int nrlinks = m_vLinks[count++];
		pNode->link.reserve(nrlinks);
		while(nrlinks--)
		{
			link = m_mapReadNodes.find(m_vLinks[count++]);
			Connect(pNode,(link->second));
			ResolveLinkData(pNode,(link->second));
		}
	}
	*/

	m_vBuffer.clear();
	m_mapReadNodes.clear();
	m_vLinksDesc.clear();
	//assert(_heapchk()==_HEAPOK);
	return true;
}
#endif  

// defines bounding rectangle of this graph
void CGraph::SetBBox(const Vec3d & min, const Vec3d & max)
{
	m_vBBoxMin = min;
	m_vBBoxMax = max;
}

// how is that for descriptive naming of functions ??
bool CGraph::OutsideOfBBox(const Vec3d & pos)
{
	if (pos.x < m_vBBoxMin.x) 
		return true;
	if (pos.x > m_vBBoxMax.x)
		return true;

	if (pos.y < m_vBBoxMin.y)
		return true;
	if (pos.y > m_vBBoxMax.y)
		return true;

	return false;
}

void CGraph::FillGreedyMap(GraphNode * pNode, const Vec3d &pos, IVisArea *pTargetArea, bool bStayInArea)
{
	MarkNode(pNode);

	if (pNode->link.empty())
		return;

	GraphNode *pNext = 0;
	VectorOfLinks::iterator pi,piend = pNode->link.end();
	for (pi=pNode->link.begin();pi!=piend;++pi)
	{ 
		GraphNode *pNow = (*pi).pLink;
		if ( (pNow->mark) || (pNow->nBuildingID!=pNode->nBuildingID)) 
			continue;
		if (bStayInArea && (pNow->pArea != pTargetArea))
			continue;

		float thisdist = (pos - pNow->data.m_pos).GetLength();
		m_mapGreedyWalkCandidates.insert(CandidateMap::iterator::value_type(thisdist,pNow));

		// this snippet will make sure we only check all points inside the target area - not the whole building
		if (pTargetArea && pNow->pArea == pTargetArea)
			FillGreedyMap(pNow,pos,pTargetArea,true);
		else
			FillGreedyMap(pNow,pos,pTargetArea,false);
	}

}

bool CGraph::RemoveEntrance(int nBuildingID, GraphNode * pNode)
{
	EntranceMap::iterator ei= m_mapEntrances.find(nBuildingID);
	if (ei!=m_mapEntrances.end())
	{
		while ((ei->first == nBuildingID) && ei!=m_mapEntrances.end())
		{
			if (ei->second == pNode) 
			{
				m_mapEntrances.erase(ei);
				return true;
			}
			ei++;
		}
	}

	ei = m_mapExits.find(nBuildingID);
	if (ei!=m_mapExits.end())
	{
		while ((ei->first == nBuildingID) && ei!=m_mapExits.end())
		{
			if (ei->second == pNode) 
			{
				m_mapExits.erase(ei);
				return true;
			}
			ei++;
		}
	}
	return false;
} 

void CGraph::RemoveIndoorNodes(void)
{
	if (m_mapEntrances.empty())
		return;

	VectorOfLinks::iterator vi;

	m_lstDeleteStack.push_front((m_mapEntrances.begin())->second);

	while (!m_lstDeleteStack.empty())
	{
		GraphNode *pCurrentNode = m_lstDeleteStack.front();
		m_lstDeleteStack.pop_front();
		
		// put all links into the node stack only if they are indoor nodes
		for (vi=pCurrentNode->link.begin();vi!=pCurrentNode->link.end();vi++)
		{
			GraphNode *pLink =(*vi).pLink;
			if (pLink->nBuildingID>=0) 
			{
				if (std::find(m_lstDeleteStack.begin(),m_lstDeleteStack.end(),pLink) == m_lstDeleteStack.end())
					m_lstDeleteStack.push_front(pLink);
			}
		}

		Disconnect(pCurrentNode);
	}

	m_mapEntrances.clear();
	
}

void CGraph::REC_RemoveNodes(GraphNode * pNode)
{
	if (pNode->link.empty())
		return;

	if (pNode->nBuildingID == -1)
		return;
	
	MarkNode(pNode);

	VectorOfLinks vecLinks;
	vecLinks.insert(vecLinks.begin(),pNode->link.begin(),pNode->link.end());
	VectorOfLinks::iterator i;
	for (i=vecLinks.begin();i!=vecLinks.end();i++)
	{
		GraphNode *pLink = (*i).pLink;
		if ((pLink->nBuildingID == pNode->nBuildingID) && (!pLink->mark))
			REC_RemoveNodes(pLink);
	}

	Disconnect(pNode);
}

GraphNode * CGraph::CreateNewNode(bool bFromTriangulation)
{
	nNodes++;
	GraphNode *pRet = new GraphNode;
	if (bFromTriangulation)
		pRet->bCreated = false;
	pRet->AddRef();
	return pRet;
}

void CGraph::DeleteNode(GraphNode * pNode)
{
	if (pNode->Release())
	{
		delete pNode;
		nNodes--;
	}
}

int CGraph::SelectNodesInSphere(const Vec3d & vCenter, float fRadius, GraphNode *pStart)
{
	GraphNode *pNode = GetEnclosing(vCenter);
	ClearMarks();
	m_lstSelected.clear();

	if (pNode->nBuildingID == -1)
		SelectNodeRecursive(pNode,vCenter,fRadius);
	else
	{ 
		SelectNodesRecursiveIndoors(pNode,vCenter,fRadius,0);
	}

	ClearMarks();

	return m_lstSelected.size();
}

void CGraph::SelectNodeRecursive(GraphNode* pNode, const Vec3d & vCenter, float fRadius)
{
	if (pNode->vertex.empty())
		return;

	if (pNode->mark)
		return;

	MarkNode(pNode);

	
	bool stillin = false;
	ObstacleIndexVector::iterator oi;
	for (oi=pNode->vertex.begin();oi!=pNode->vertex.end();oi++)
	{
		ObstacleData od = m_pAISystem->m_VertexList.GetVertex((*oi));
		float flength = GetLengthSquared((od.vPos - vCenter));
		if (flength < (fRadius*fRadius))
		{
			if ( std::find(m_lstSelected.begin(),m_lstSelected.end(),od) == m_lstSelected.end())
					m_lstSelected.push_back(od);
			stillin = true;
		}
	}

	if (!stillin)
		return;

	// go trough all the links of this one
	VectorOfLinks::iterator gi;
	for (gi=pNode->link.begin();gi!=pNode->link.end();gi++)
	{
		if ((*gi).pLink->nBuildingID<0)
			SelectNodeRecursive((*gi).pLink,vCenter,fRadius);
	}

}

void CGraph::SelectNodesRecursiveIndoors(GraphNode * pNode, const Vec3d & vCenter, float fRadius, float fDistance)
{
	if (pNode->nBuildingID == -1)
			return;	// DO NOT go outdoors

	if (fDistance>fRadius)
		return;

	if (pNode->mark)
		return;

	MarkNode(pNode);

	
	bool stillin = false;

	if ( GetLengthSquared((pNode->data.m_pos - vCenter)) < 2*(fRadius*fRadius))
		stillin = true;
 
	ObstacleIndexVector::iterator oi;
	for (oi=pNode->vertex.begin();oi!=pNode->vertex.end();oi++)
	{
		ObstacleData od = m_pAISystem->m_VertexList.GetVertex((*oi));
		float flength = GetLengthSquared((od.vPos - vCenter));
//		float fPathLength = fDistance + (pNode->data.m_pos - od.vPos).GetLength();
//		if (fPathLength*fPathLength > flength*2.f)
	//		continue;
		if (flength < (fRadius*fRadius))
		{
			if ( std::find(m_lstSelected.begin(),m_lstSelected.end(),od) == m_lstSelected.end())
				m_lstSelected.push_back(od);
			stillin = true;
		}
	}

	if (!stillin)
		return;

	// go trough all the links of this one
	VectorOfLinks::iterator gi;
	for (gi=pNode->link.begin();gi!=pNode->link.end();gi++)
	{
		GraphNode *pNextNode = (*gi).pLink;
		SelectNodesRecursiveIndoors(pNextNode,vCenter,fRadius,fDistance+(pNode->data.m_pos - pNextNode->data.m_pos).GetLength());
	}
}

void CGraph::AddHidePoint(GraphNode* pOwner, const Vec3d & pos, const Vec3d & dir)
{
		if (!pOwner)
			return;

		
		ObstacleData od;
		od.vPos = pos;
		od.vDir = dir;

		index_t i = m_pAISystem->m_VertexList.FindVertex(od);
		
		

		if (i<0)
		{
			//pOwner->vertex.push_back(od);
			pOwner->vertex.push_back(m_pAISystem->m_VertexList.AddVertex(od));
		}
		else
		{
			ObstacleIndexVector::iterator oi = std::find(pOwner->vertex.begin(),pOwner->vertex.end(),i);
			if (oi!=pOwner->vertex.end())
			{
				m_pAISystem->m_VertexList.ModifyVertex(i).vPos = pos;
				m_pAISystem->m_VertexList.ModifyVertex(i).vDir = dir;
			}
			else
				pOwner->vertex.push_back(i);
		}
			

}

void CGraph::RemoveHidePoint(GraphNode * pOwner, const Vec3d & pos, const Vec3d & dir)
{
	if (!pOwner)
		return;

	ObstacleIndexVector::iterator oi;
	for ( oi = pOwner->vertex.begin();oi != pOwner->vertex.end(); oi++ )
	{
		ObstacleData od = m_pAISystem->m_VertexList.GetVertex((*oi));
		if ( ( (od.vPos - pos).GetLength() < 0.0001 ) ) 
		{
			pOwner->vertex.erase(oi);
			break;
		}
	}
}

void CGraph::DisconnectUnreachable(void)
{
	GraphNode *pCurrent = m_pFirst;

	m_lstNodeStack.clear();
	ClearMarks();

	
	m_lstNodeStack.push_back(pCurrent);
	ray_hit rh; 

	while (!m_lstNodeStack.empty())
	{
		MarkNode(pCurrent);
		vectorf start = pCurrent->data.m_pos;

		VectorOfLinks linkVector;
		linkVector = pCurrent->link;
		VectorOfLinks::iterator vi;
		for (vi=linkVector.begin();vi!=linkVector.end();vi++)
		{

			if (!(*vi).pLink->mark)
			{
				if (std::find(m_lstNodeStack.begin(),m_lstNodeStack.end(),(*vi).pLink) == m_lstNodeStack.end())
					m_lstNodeStack.push_front((*vi).pLink);
			}

			// check connectivity

			vectorf end = (*vi).pLink->data.m_pos;
			int cnt = m_pAISystem->GetPhysicalWorld()->RayWorldIntersection(start,end-start,ent_static, 0,&rh,1);

			if (cnt)
				DisconnectLink(pCurrent,(*vi).pLink);

		}
		
		pCurrent = m_lstNodeStack.front();
		m_lstNodeStack.pop_front();

	}

	ClearMarks();
}


void CGraph::DisconnectLink(GraphNode * one, GraphNode * two, bool bOneWay)
{
	if (!one || !two) 
		return;

	VectorOfLinks::iterator vi;
	for (vi=one->link.begin();vi!=one->link.end();vi++)
	{
			if ( (*vi).pLink == two)
			{
				one->link.erase(vi);
				DeleteNode(two);		// this will lower the ref count... will delete if appropriate
				break;
			}
	}

	if (bOneWay)
		return; 
	
	for (vi=two->link.begin();vi!=two->link.end();vi++)
	{
			if ( (*vi).pLink == one)
			{
				two->link.erase(vi);
				DeleteNode(one);  // this will lower the ref count... will delete if appropriate
				break;
			}
	}

}

int CGraph::BeautifyPath(int &nIterations, const Vec3d &start, const Vec3d &end)
{
	//return PATHFINDER_PATHFOUND;
	if (!m_pAISystem->m_cvBeautifyPath->GetIVal())
	{
		return PATHFINDER_PATHFOUND;
	}

	// try to skip unnecessary wiggling around
	if (m_bBeautifying)
	{
		m_lstPath.clear();
		if (m_lstNodeStack.size()==1)
		{
			m_lstPath.push_back(end);
			return PATHFINDER_PATHFOUND;
		}

		m_iFirst = m_lstNodeStack.begin();
		m_iThird = m_lstNodeStack.end();
		m_bBeautifying = false;
		m_vBeautifierStart = start;

		//CryLogAlways("Start BEAUTIFY PATH for: %s ",m_pRequester->GetName());
		//CryLogAlways("From position (%.3f,%.3f,%.3f)",start.x,start.y,start.z);
		//CryLogAlways("To position (%.3f,%.3f,%.3f)",end.x,end.y,end.z);
	}
	//else
	//{
	//	CryLogAlways("continuing beautification for: %s ",m_pRequester->GetName());
	//}

	
	while ((nIterations--) && (m_iFirst!=m_lstNodeStack.end()))
	{
		ListNodes::iterator iNextFirst = m_lstNodeStack.end();
		m_iSecond = m_iFirst;
		m_iSecond++;
		
		Vec3d vRealStart,vRealEnd;
		
		bool bSkipEval = false;

		if ((*m_iFirst)->nBuildingID < 0)
		{
			while (m_iSecond != m_lstNodeStack.end())
			{
				GraphNode *pFirst = (*m_iFirst);
				GraphNode *pSecond = (*m_iSecond);

				if (pSecond->nBuildingID >= 0)
				{
					if (!bSkipEval)
					{
						bSkipEval = true;
						iNextFirst = m_lstNodeStack.end();
					}
				//	if (m_iThird!=m_lstNodeStack.end())
				//	{
				//		m_lstPath.push_back( (*m_iThird)->data.m_pos);
				//		m_iFirst = m_iThird;
				//	}
					break;
				}

				
				
				// traverse list from iFirst to iSecond testing edges as you go
				bool bPassable = true; // optimistic expectation
				ListNodes::iterator iPairStart=m_iFirst;
				for (;iPairStart!=m_iSecond;iPairStart++)
				{
					ListNodes::iterator iPairEnd = iPairStart;

					iPairEnd++;

					Vec3d EdgStart;
					Vec3d EdgEnd;
					Vec3d EdgDir;
					Vec3d EdgCenter;


					VectorOfLinks::iterator vli;
					for (vli=(*iPairStart)->link.begin();vli!=(*iPairStart)->link.end();vli++)
					{


						if ((*vli).pLink == (*iPairEnd))
						{
							
							if ((m_iFirst == m_lstNodeStack.begin()) || (m_lstPath.empty()))
								vRealStart = start;
							else
								vRealStart = m_lstPath.back();

							
							if ((*m_iSecond) == (*m_lstNodeStack.rbegin()))
								vRealEnd = end;
							else
								vRealEnd = (*m_iSecond)->data.m_pos;



								EdgStart = m_pAISystem->m_VertexList.GetVertex((*iPairStart)->vertex[(*vli).nStartIndex]).vPos;

								EdgEnd = m_pAISystem->m_VertexList.GetVertex((*iPairStart)->vertex[(*vli).nEndIndex]).vPos;
								EdgDir = EdgEnd - EdgStart;
								EdgCenter =(*vli).vEdgeCenter;
								EdgCenter.z = 0;
								vRealStart.z = 0;
								vRealEnd.z = 0;
								float s=-1,t=-1;
								

								
								if (m_pAISystem->SegmentsIntersect(vRealStart, vRealEnd-vRealStart,EdgStart,EdgDir,s,t))
								{
									if ( s>0.f && s<1.f && t>0.f && t<1.f)
									{
										
										m_vLastIntersection = EdgStart+t*EdgDir;	
										float fCenterDist = (m_vLastIntersection - EdgCenter).GetLength();
										//if (fCenterDist > ((*vli).fMaxRadius-1.f))
										if (fCenterDist > ((*vli).fMaxRadius-m_pRequester->m_fPassRadius))
										{
											bPassable = false;
										}

									}
									else
										bPassable = false;
								}
								else
								{
									bPassable = false;
								}
								break;
							}
					}
 
					if (!bPassable)
					{
						float fEdgeLength =EdgDir.Length();
						//float dist_from_obstacle = 0.6f;
						float dist_from_obstacle = m_pRequester->m_fPassRadius;

						// if we have more space, do not get too close to the obstacle
						if ((m_pRequester->m_fPassRadius < 1.f) && ((*vli).fMaxRadius > 1.f))
							dist_from_obstacle = 1.f;
						if ((EdgStart-m_vLastIntersection).Length() < (EdgEnd-m_vLastIntersection).Length() )
						{
							// closer to start edge
							float fFullDistToCenter = (EdgStart - EdgCenter).Length();
							float minT = fFullDistToCenter - (*vli).fMaxRadius + dist_from_obstacle;
							minT/=fEdgeLength;
							m_vBeautifierStart = EdgStart+minT*EdgDir;
						}
						else
						{
							// closer to end edge
							float fFullDistToCenter = (EdgEnd - EdgCenter).Length();
							float minT = fFullDistToCenter - (*vli).fMaxRadius + dist_from_obstacle;
							minT/=fEdgeLength;
							m_vBeautifierStart = EdgEnd-minT*EdgDir;
						}

/*						float minT =  ((fEdgeLength/2.f)-((*vli).fMaxRadius-dist_from_obstacle)) / fEdgeLength;
						// which edge is closer to the intersection point
						if ((EdgStart-m_vLastIntersection).Length() < (EdgEnd-m_vLastIntersection).Length() )
							m_vBeautifierStart = EdgStart+minT*EdgDir;
						else
							m_vBeautifierStart = EdgEnd-minT*EdgDir;*/
						break;

					}

				}

				if (bPassable)
				{
					m_iThird = m_iSecond;
					if (bSkipEval)
					{
						iNextFirst = m_lstNodeStack.end();
						bSkipEval = false;
					}
				}
				else
				{
					if (!bSkipEval)
					{
						iNextFirst = iPairStart;
						bSkipEval = true;
					}
				}

				m_iSecond++;
			}

			if (!bSkipEval)
			{
				// if the third is not valid, then add midpoint od first and second, and then second
				if (m_iThird == m_lstNodeStack.end())
				{
					m_iSecond = m_iFirst;
					m_iSecond++;
					if (m_iSecond!=m_lstNodeStack.end() && ((*m_iSecond)->nBuildingID <0))
					{
						///m_lstPath.push_back(vBestPass);
						VectorOfLinks::iterator vli;
						for (vli=(*m_iSecond)->link.begin();vli!=(*m_iSecond)->link.end();vli++)
						{
							if ((*vli).pLink == (*m_iFirst))
								m_lstPath.push_back((*vli).vEdgeCenter);
						}
					}
					
				}
				else
				{
					m_iSecond = m_iFirst;
					m_iSecond++;
					if ((m_iSecond!=m_iThird) && (m_iSecond != m_lstNodeStack.end()))
					{
						// there are some nodes to delete, so delete them
						for (ListNodes::iterator li=m_iSecond; (li!=m_iThird) && (li!=m_lstNodeStack.end());li=m_lstNodeStack.erase(li));
					}

					//if (m_iThird == --m_lstNodeStack.end())
					if ((*m_iThird) == (*m_lstNodeStack.rbegin()))
					{
						m_lstPath.push_back(end);
					}
					else
					{
						m_lstPath.push_back( (*m_iThird)->data.m_pos);
						//vBestPass.z = m_pAISystem->m_pSystem->GetI3DEngine()->GetTerrainElevation(vBestPass.x, vBestPass.y);
						//m_lstPath.push_back( vBestPass);
					}
				}
			}
			else
			{
				if (iNextFirst != m_lstNodeStack.end())
				{
					I3DEngine *pEngine = m_pAISystem->m_pSystem->GetI3DEngine();
					if (pEngine)
						m_vBeautifierStart.z = pEngine->GetTerrainElevation(m_vBeautifierStart.x,m_vBeautifierStart.y);
					m_lstPath.push_back(m_vBeautifierStart);
					m_iFirst = iNextFirst;
				}

			}
		}
		else
		{
			m_lstPath.push_back( (*m_iFirst)->data.m_pos);
		}
	
		m_iThird = m_lstNodeStack.end();
		m_iFirst++;
	}

	if (m_iFirst==m_lstNodeStack.end())
		m_lstPath.push_back(end);


	if (m_iFirst == m_lstNodeStack.end())
	{
		m_bBeautifying = true;
		m_lstNodeStack.clear();
		if (m_lstPath.size() > 2)
		{
			int todel = 1;
			ListPositions::iterator li = m_lstPath.begin();
			Vec3d firstpos = (*li++)-start;
			Vec3d secondpos= (*li)-start;
			while ((*li) == m_lstPath.front())
			{
				secondpos= (*++li)-start;;
				todel++;
			}

			if (firstpos.Dot(secondpos) < 0)
			{
				while (todel--)
				{
					if (!m_lstPath.empty())
						m_lstPath.pop_front();
				}
			}
		}
		return PATHFINDER_PATHFOUND;
	}

	//CryLogAlways("----Ran out of iterations(%.3f,%.3f,%.3f)",(*m_iFirst)->data.m_pos.x,(*m_iFirst)->data.m_pos.y,(*m_iFirst)->data.m_pos.z);
	return PATHFINDER_BEAUTIFYINGPATH;
}



void CGraph::ResolveLinkData(GraphNode* pOne, GraphNode* pTwo)
{
	if (pOne->nBuildingID>=0 || pTwo->nBuildingID>=0) 
		return;

	if (pOne->vertex.empty() || pTwo->vertex.empty())
		return;

	int iOneEdge1=-1,iOneEdge2=-1;
	int iTwoEdge1=-1,iTwoEdge2=-1;

	for (unsigned int i=0;i<pOne->vertex.size();i++)
		for (unsigned int j=0;j<pTwo->vertex.size();j++)
		{
		/*		if ( (fabs(pOne->vertex[i].vPos.x - pTwo->vertex[j].vPos.x) < 0.0001f) &&
					 (fabs(pOne->vertex[i].vPos.y - pTwo->vertex[j].vPos.y) < 0.0001f) &&
					 (fabs(pOne->vertex[i].vPos.z - pTwo->vertex[j].vPos.z) < 0.0001f) )
					 */
			  //  if (IsEquivalent(pOne->vertex[i].vPos,pTwo->vertex[j].vPos,0.001f))
				//if (pOne->vertex[i].vPos==pTwo->vertex[j].vPos )
				if (pOne->vertex[i]==pTwo->vertex[j])
				{
					if (iOneEdge1<0)
						iOneEdge1 = i;
					else
						iOneEdge2 = i;

					if (iTwoEdge1<0)
						iTwoEdge1 = j;
					else
						iTwoEdge2 = j;
				}
		}

	// find triangle normal for one
	
	Vec3d OneNormal = (m_pAISystem->m_VertexList.GetVertex(pOne->vertex[0]).vPos - m_pAISystem->m_VertexList.GetVertex(pOne->vertex[1]).vPos).Cross(m_pAISystem->m_VertexList.GetVertex(pOne->vertex[2]).vPos - m_pAISystem->m_VertexList.GetVertex(pOne->vertex[1]).vPos);
	Vec3d TwoNormal = (m_pAISystem->m_VertexList.GetVertex(pTwo->vertex[0]).vPos - m_pAISystem->m_VertexList.GetVertex(pTwo->vertex[1]).vPos).Cross(m_pAISystem->m_VertexList.GetVertex(pTwo->vertex[2]).vPos - m_pAISystem->m_VertexList.GetVertex(pTwo->vertex[1]).vPos);

	if ( iOneEdge1 * iOneEdge2 < 0)
		int a=5;

	if ( iTwoEdge1 * iTwoEdge2 < 0)
		int a=5;

	int iNoOne = 3 - (iOneEdge1 + iOneEdge2);
	int iNoTwo = 3 - (iTwoEdge1 + iTwoEdge2);

	// find which link from one goes to two
	VectorOfLinks::iterator vi;
	for (vi=pOne->link.begin();vi!=pOne->link.end();vi++)
	{
		if ((*vi).pLink == pTwo)
		{
			(*vi).nStartIndex = iOneEdge1;
			(*vi).nEndIndex = iOneEdge2;

			Vec3d edge = m_pAISystem->m_VertexList.GetVertex(pOne->vertex[iOneEdge1]).vPos - m_pAISystem->m_VertexList.GetVertex(pOne->vertex[iOneEdge2]).vPos;

		///	if (pOne->vertex[iOneEdge1].bOccupied && pOne->vertex[iOneEdge2].bOccupied)
		//		(*vi).fMaxRadius = -1;

			Vec3d vCross(edge.Cross(TwoNormal));
			float fLen = vCross.GetLength();
			Vec3d normal = fLen>0 ? vCross/fLen : Vec3d(0,0,0);
			float fdot = normal.Dot(m_pAISystem->m_VertexList.GetVertex(pTwo->vertex[iNoTwo]).vPos - (m_pAISystem->m_VertexList.GetVertex(pTwo->vertex[iTwoEdge1]).vPos + edge/2.f));
		
			(*vi).vWayOut = normal; 
			if (fdot > 0 )
				(*vi).vWayOut *=-1.f; 
		}
	}

	for (vi=pTwo->link.begin();vi!=pTwo->link.end();vi++)
	{
		if ((*vi).pLink == pOne)
		{
			(*vi).nStartIndex = iTwoEdge1;
			(*vi).nEndIndex = iTwoEdge2;

			Vec3d edge = m_pAISystem->m_VertexList.GetVertex(pTwo->vertex[iTwoEdge1]).vPos - m_pAISystem->m_VertexList.GetVertex(pTwo->vertex[iTwoEdge2]).vPos;

			//if (pTwo->vertex[iTwoEdge1].bOccupied && pTwo->vertex[iTwoEdge2].bOccupied)
			//	(*vi).fMaxRadius = -1;

			Vec3d vCross(edge.Cross(TwoNormal));
			float fLen = vCross.GetLength();
			Vec3d normal = fLen>0 ? vCross/fLen : Vec3d(0,0,0);
			float fdot = normal.Dot(m_pAISystem->m_VertexList.GetVertex(pTwo->vertex[iNoTwo]).vPos - (m_pAISystem->m_VertexList.GetVertex(pTwo->vertex[iTwoEdge1]).vPos + edge/2.f));

			(*vi).vWayOut = normal;
			if (fdot > 0 )
				(*vi).vWayOut *=-1.f; 
		}
	}

}

void CGraph::ConnectNodes(ListNodes & lstNodes)
{

	// clear degenerate triangles
	ListNodes::iterator it;
	for (it=lstNodes.begin();it!=lstNodes.end();)
	{	
		GraphNode *pCurrent = (*it);
		ObstacleIndexVector::iterator obi,obi2;
		bool bRemoved = false;

		for (obi=pCurrent->vertex.begin();obi!=pCurrent->vertex.end();obi++)
		{
			for (obi2=pCurrent->vertex.begin();obi2!=pCurrent->vertex.end();obi2++)
			{
				if (obi==obi2)
					continue;
				if ( (*obi) == (*obi2) )
				{
					it=lstNodes.erase(it);
					bRemoved = true;
					break;
				}
			}
			if (bRemoved)
				break;
		}
		if (!bRemoved)
			it++;
	}
	

	// reconnect triangles in nodes list
	ListNodes::iterator it1,it2;
	for (it1=lstNodes.begin();it1!=lstNodes.end();it1++)
	{
        GraphNode *pCurrent = (*it1);
		for (it2=lstNodes.begin();it2!=lstNodes.end();it2++)
		{
			GraphNode *pCandidate = (*it2);
			ObstacleIndexVector::iterator obCur,obNext;
			for (obCur=pCurrent->vertex.begin();obCur!=pCurrent->vertex.end();obCur++)
			{
				obNext = obCur;
				obNext++;
				if (obNext==pCurrent->vertex.end())
					obNext = pCurrent->vertex.begin();

				ObstacleIndexVector::iterator obCan,obNextCan;
				for (obCan=pCandidate->vertex.begin();obCan!=pCandidate->vertex.end();obCan++)
				{
					obNextCan = obCan;
					obNextCan++;
					if (obNextCan == pCandidate->vertex.end())
						obNextCan = pCandidate->vertex.begin();

					/*
					if ( (fabs((*obCur).vPos.x - (*obCan).vPos.x) < 0.0001f) &&
						 (fabs((*obCur).vPos.y - (*obCan).vPos.y) < 0.0001f) &&
						 (fabs((*obCur).vPos.z - (*obCan).vPos.z) < 0.0001f) &&
						 (fabs((*obNext).vPos.x - (*obNextCan).vPos.x) < 0.0001f) &&
						 (fabs((*obNext).vPos.y - (*obNextCan).vPos.y) < 0.0001f) &&
						 (fabs((*obNext).vPos.z - (*obNextCan).vPos.z) < 0.0001f) )
						 */
					if (  ((*obCur)==(*obCan)) && ((*obNext)==(*obNextCan))  )
					{
						if (pCandidate!=pCurrent)
						{
							Connect(pCurrent,pCandidate);
							ResolveLinkData(pCurrent,pCandidate);
						}
					}

					/*
					if ( (fabs((*obCur).vPos.x - (*obNextCan).vPos.x) < 0.0001f) &&
						 (fabs((*obCur).vPos.y - (*obNextCan).vPos.y) < 0.0001f) &&
						 (fabs((*obCur).vPos.z - (*obNextCan).vPos.z) < 0.0001f) &&
						 (fabs((*obNext).vPos.x - (*obCan).vPos.x) < 0.0001f) &&
						 (fabs((*obNext).vPos.y - (*obCan).vPos.y) < 0.0001f) &&
						 (fabs((*obNext).vPos.z - (*obCan).vPos.z) < 0.0001f) )
						 */
					if ( ((*obCur)==(*obNextCan)) && ((*obNext)==(*obCan)) )
					{
						if (pCandidate!=pCurrent)
						{
							Connect(pCurrent,pCandidate);
							ResolveLinkData(pCurrent,pCandidate);
						}
					}
				}
			}
		}
	}

	for (it1=lstNodes.begin();it1!=lstNodes.end();it1++)
	{
		GraphNode *pCurrent = (*it1);
		VectorOfLinks::iterator vi;
		for (vi=pCurrent->link.begin();vi!=pCurrent->link.end();vi++)
			ResolveLinkData(pCurrent,(*vi).pLink);
	}

}

void CGraph::FillGraphNodeData(GraphNode* pNode)
{
	I3DEngine *pEngine = m_pAISystem->m_pSystem->GetI3DEngine();

	//CheckClockness(pNode);

	ObstacleIndexVector::iterator oi;
	for (oi=pNode->vertex.begin();oi!=pNode->vertex.end();oi++)
	{
		pNode->data.m_pos.x += m_pAISystem->m_VertexList.GetVertex((*oi)).vPos.x;
		pNode->data.m_pos.y += m_pAISystem->m_VertexList.GetVertex((*oi)).vPos.y;
		pNode->data.m_pos.z += m_pAISystem->m_VertexList.GetVertex((*oi)).vPos.z;
	}

	pNode->data.m_pos.x/=3.f;
	pNode->data.m_pos.y/=3.f;
	pNode->data.m_pos.z/=3.f;

	//	pNode->data.m_pos.x = (v1->x + v2->x + v3->x) / 3.f;
	//	pNode->data.m_pos.y =	(v1->y + v2->y + v3->y) / 3.f;
	float x = pNode->data.m_pos.x ;
	float y = pNode->data.m_pos.y ;
	// put it on the terrain
	float z = pEngine->GetTerrainElevation(x,y);
	pNode->data.m_pos.z = z+0.2f;		// elevate it a little bit

	// calculate slope index		
	float he,ve;
	he = ( pEngine->GetTerrainElevation(x+1,y) - pEngine->GetTerrainElevation(x-1,y) ) / 2.f; // tg(alpha)
	he= (float) fabs(he);
	if ( he>1.f ) he = 1.0f; //for now, no slopes beyond 45 degrees
	ve = ( pEngine->GetTerrainElevation(x,y+1) - pEngine->GetTerrainElevation(x,y-1) ) / 2.f; // tg(alpha)
	ve= (float) fabs(ve);
	if ( ve>1.f ) ve = 1.0f; //for now, no slopes beyond 45 degrees
	if (he>ve) pNode->data.fSlope = he;
	else pNode->data.fSlope = ve;

	//check if point in water
	if (pEngine->IsPointInWater(Vec3d(pNode->data.m_pos)))
		pNode->data.bWater = true;

	// check if point in shadow 
//	if (pEngine->IsPointInShadow(Vec3d(pNode->data.m_pos)))
//		pNode->data.bShadow = true;

}

void CGraph::SetCurrentHeuristic(unsigned int heuristic_type)
{
	if (m_pHeuristic) 
	{
		delete m_pHeuristic;
		m_pHeuristic = 0;
	}

	switch (heuristic_type)
	{
		case AIHEURISTIC_DEFAULT:
			m_pHeuristic = new CHeuristic;
		break;
		case AIHEURISTIC_STANDARD:
			m_pHeuristic = new CStandardHeuristic;
		break;
		case AIHEURISTIC_VEHICLE:
			m_pHeuristic = new CVehicleHeuristic;
		break;

	}
}

void CGraph::ResolveTotalLinkData(void)
{
	m_lstNodeStack.clear();
	m_lstNodeStack.push_back(m_pSafeFirst);

	while (!m_lstNodeStack.empty())
	{
		GraphNode *pCurrent = m_lstNodeStack.front();
		m_lstNodeStack.pop_front();

		MarkNode(pCurrent);

		for (VectorOfLinks::iterator vi=pCurrent->link.begin();vi!=pCurrent->link.end();vi++)
		{
			ResolveLinkData(pCurrent,(*vi).pLink);

			if (!vi->pLink->mark)
			{
				if (std::find(m_lstNodeStack.begin(),m_lstNodeStack.end(),vi->pLink) == m_lstNodeStack.end())
					m_lstNodeStack.push_back(vi->pLink);
			}
		}
	}

	ClearMarks();
}

bool CGraph::CanReuseLastPath(GraphNode * pBegin)
{
	ListNodes::iterator i = m_lstLastPath.begin();

	while (i!=m_lstLastPath.end())
	{
		if ( (*i) == pBegin )
		{
			m_lstNodeStack.clear();
			m_lstNodeStack.insert(m_lstNodeStack.begin(),i,m_lstLastPath.end());
			m_lstPath.clear();
			for (ListNodes::iterator fi = i;fi!=m_lstLastPath.end();fi++)
				m_lstPath.push_back((*fi)->data.m_pos);
			return true;
		}
		++i;
	}

	return false;
}

GraphNode * CGraph::GetThirdNode(const Vec3d & vFirst, const Vec3d & vSecond, const Vec3d &vThird)
{
	GraphNode *pFirst = GetEnclosing(vFirst);
	GraphNode *pSecond = GetEnclosing(vSecond);
	GraphNode *pThird = GetEnclosing(vThird);
	
	VectorOfLinks::iterator vli;
	if (pFirst->link.size()>2)
	{
		for (vli=pFirst->link.begin();vli!=pFirst->link.end();vli++)
			if (((*vli).pLink != pSecond) && ((*vli).pLink!=pThird))
				return (*vli).pLink;
	}
	else
	{
		for (vli=pFirst->link.begin();vli!=pFirst->link.end();vli++)
			if ( (*vli).pLink != pSecond )
				return (*vli).pLink;
	}

	return NULL;
}

void CGraph::Reset(void)
{
	m_pWalkBackCurrent = 0;
	m_pPathfinderCurrent = 0;
	m_lstCurrentHistory.clear();
	m_lstLastPath.clear();
	m_lstNodeStack.clear();
	m_bBeautifying = true;
}

GraphNode * CGraph::GetEntrance(int nBuildingID,const Vec3d &pos)
{
	GraphNode *pEntrance=0;
	float mindist = 1000000;
	EntranceMap::iterator ei = m_mapEntrances.find(nBuildingID);
	if (ei != m_mapEntrances.end())
	{
		pEntrance=ei->second;
		if (m_mapEntrances.count(nBuildingID) > 1)
		{
			mindist = GetLengthSquared((pEntrance->data.m_pos-pos));
			for (;ei!=m_mapEntrances.end();ei++)
			{
				if (ei->first != nBuildingID)
					break;
				float curr_dist = GetLengthSquared(((ei->second)->data.m_pos-pos));
				if (curr_dist<=mindist)
				{
					pEntrance = ei->second;
					mindist = curr_dist;
				}
			}
		}
	}

	ei = m_mapExits.find(nBuildingID);
	if (ei != m_mapExits.end())
	{
		pEntrance=ei->second;
		if (m_mapExits.count(nBuildingID) > 1)
		{
			mindist = GetLengthSquared((pEntrance->data.m_pos-pos));
			for (;ei!=m_mapExits.end();ei++)
			{
				if (ei->first != nBuildingID)
					break;
				float curr_dist = GetLengthSquared(((ei->second)->data.m_pos-pos));
				if (curr_dist<=mindist)
				{
					pEntrance = ei->second;
					mindist = curr_dist;
				}
			}
		}
	}


	return pEntrance;
}

void CGraph::RemoveDegenerateTriangle(GraphNode * pDegenerate, bool bRecurse)
{
	VectorOfLinks::iterator li, liend = pDegenerate->link.end();
	for (li=pDegenerate->link.begin();li!=liend;++li)
	{
		Vec3d vStart = m_pAISystem->m_VertexList.GetVertex(pDegenerate->vertex[(*li).nStartIndex]).vPos;
		Vec3d vEnd = m_pAISystem->m_VertexList.GetVertex(pDegenerate->vertex[(*li).nEndIndex]).vPos;

		if (IsEquivalent(vStart,vEnd,0.1f))
			break;
	}

	if (li==liend)
		return;

	int noStartIndex = (*li).nStartIndex;
	int noEndIndex	 = (*li).nEndIndex;

	int noVertex = 3 - (noStartIndex+noEndIndex);
	GraphNode *pLink1=0,*pLink2=0;
	VectorOfLinks::iterator vol,ivolend = pDegenerate->link.end();
	for (vol=pDegenerate->link.begin();vol!=ivolend;++vol)
	{
		if ( ( (*vol).nStartIndex == noVertex) ||
			( (*vol).nEndIndex == noVertex ))
		{
			// we have one of the other two links - supposedly correct
			if ( (*vol).nStartIndex == noStartIndex  || (*vol).nEndIndex == noStartIndex)
				pLink1 = (*vol).pLink;
			else
				pLink2 = (*vol).pLink;
		}
	}

	if (!pLink1 || !pLink2)
		AIError("!Bad triangle found while trying to eliminate a degenerate triangle.");

	if (bRecurse)
		RemoveDegenerateTriangle((*li).pLink,false);

	Disconnect(pDegenerate);
	Connect(pLink1,pLink2);

	if (pLink1->mark)
		pLink1->mark = false;

	if (pLink2->mark)
		pLink2->mark = false;
}


void CGraph::FindTrapNodes(GraphNode *pNode, int recCount)
{
	if (recCount<0)
		return;
	MarkNode(pNode);

	if (pNode->nBuildingID>=0) // no inside
		return;

	if (!pNode->link.empty())
	{
		bool bTrapped = true;
		bool bCanHoldPlayer = false;
		VectorOfLinks::iterator vli, vliend = pNode->link.end();
		for (vli=pNode->link.begin();vli!=vliend;++vli)
		{
			GraphLink glink = (*vli);
			if (glink.fMaxRadius>0.6f)
				bTrapped = false;
			Vec3d start = m_pAISystem->m_VertexList.GetVertex((*vli).nStartIndex).vPos;
			Vec3d end = m_pAISystem->m_VertexList.GetVertex((*vli).nEndIndex).vPos;
			if ( GetLength(start-end) > 0.6)
				bCanHoldPlayer = true;

		}

		if (bTrapped && bCanHoldPlayer)
		{
			m_lstTrapNodes.push_back(pNode);
		}

		for (vli=pNode->link.begin();vli!=vliend;++vli)
		{
			GraphLink glink = (*vli);
			FindTrapNodes(glink.pLink,recCount-1);
		}
	}
}

void CGraph::FixDegenerateTriangles(void)
{
	m_lstNodeStack.clear();
	m_lstNodeStack.push_back(m_pSafeFirst);

	// traverse all the graph
	while (!m_lstNodeStack.empty())
	{
		GraphNode *pCurrent = m_lstNodeStack.front();
		m_lstNodeStack.pop_front();

		MarkNode(pCurrent);



		// add neighbors to list
		for (VectorOfLinks::iterator vi=pCurrent->link.begin();vi!=pCurrent->link.end();vi++)
		{
			if (!vi->pLink->mark)
			{
				if (std::find(m_lstNodeStack.begin(),m_lstNodeStack.end(),vi->pLink) == m_lstNodeStack.end())
					m_lstNodeStack.push_back(vi->pLink);
			}
		}

		if (pCurrent == m_pSafeFirst)
			continue;

		// check for degeneracy
		float fDegen = pCurrent->GetDegeneracyValue();
		if (fDegen<0.01f) 
		{
			GraphNode *pPair = 0;
			float fTSize = (float) GetAISystem()->m_pSystem->GetI3DEngine()->GetTerrainSize();
			VectorOfLinks::iterator vli = pCurrent->link.begin(),vliend = pCurrent->link.end();
			for (;vli!=vliend;++vli)
			{
				if ( (*vli).fMaxRadius>0 )	// make sure you do not break forbidden areas
				{ 
					GraphNode *pCandidate = (*vli).pLink;
					if (pCandidate==m_pSafeFirst)
						continue;
					// find the link in the candidate that leads to the current node
					VectorOfLinks::iterator vback = pCandidate->link.begin(),vbackend = pCandidate->link.end();
					for (;vback!=vbackend;++vback)
					{
						if ( (*vback).pLink == pCurrent )
							break;
					}

					// create the two new nodes
					GraphNode *p1 = CreateNewNode();
					GraphNode *p2 = CreateNewNode();
				
					p1->vertex.push_back( pCurrent->vertex[(*vli).nStartIndex] );
					p1->vertex.push_back( pCurrent->vertex[3 - ((*vli).nEndIndex+ (*vli).nStartIndex)] );
					p1->vertex.push_back( pCandidate->vertex[3 - ((*vback).nEndIndex+ (*vback).nStartIndex)] );
					FillGraphNodeData(p1);

					p2->vertex.push_back( pCurrent->vertex[(*vli).nEndIndex] );
					p2->vertex.push_back( pCurrent->vertex[3 - ((*vli).nEndIndex+ (*vli).nStartIndex)] );
					p2->vertex.push_back( pCandidate->vertex[3 - ((*vback).nEndIndex+ (*vback).nStartIndex)] );
					FillGraphNodeData(p2);

					float p1Deg = p1->GetDegeneracyValue();
					float p2Deg = p2->GetDegeneracyValue();
					if ( (p1Deg>fDegen)  && (p2Deg>fDegen) )
					{
						fDegen = (p1Deg<p2Deg)? p2Deg : p1Deg;
						pPair=pCandidate;
					}

					Disconnect(p1);
					Disconnect(p2);
				}
			}

			if (pPair)
			{
					
					// find the link in the candidate that leads to the current node
					vli = pCurrent->link.begin(),vliend = pCurrent->link.end();
					for (;vli!=vliend;++vli)
					{
						if ( (*vli).pLink == pPair )
							break;
					}

					VectorOfLinks::iterator vback = pPair->link.begin(),vbackend = pPair->link.end();
					for (;vback!=vbackend;++vback)
					{
						if ( (*vback).pLink == pCurrent )
							break;
					}

					// create the two new nodes
					GraphNode *p1 = CreateNewNode();
					GraphNode *p2 = CreateNewNode();

					ListNodes lstNewNodes;
				
					p1->vertex.push_back( pCurrent->vertex[(*vli).nStartIndex] );
					p1->vertex.push_back( pCurrent->vertex[3 - ((*vli).nEndIndex+ (*vli).nStartIndex)] );
					p1->vertex.push_back( pPair->vertex[3 - ((*vback).nEndIndex+ (*vback).nStartIndex)] );
					FillGraphNodeData(p1);

					p2->vertex.push_back( pCurrent->vertex[(*vli).nEndIndex] );
					p2->vertex.push_back( pCurrent->vertex[3 - ((*vli).nEndIndex+ (*vli).nStartIndex)] );
					p2->vertex.push_back( pPair->vertex[3 - ((*vback).nEndIndex+ (*vback).nStartIndex)] );
					FillGraphNodeData(p2);

					lstNewNodes.push_back(p1);
					lstNewNodes.push_back(p2);
					// push all neighbors of existing 2 triangles
					vback = pPair->link.begin(),vbackend = pPair->link.end();
					for (;vback!=vbackend;++vback)
					{
						if ( std::find(lstNewNodes.begin(),lstNewNodes.end(),(*vback).pLink)==lstNewNodes.end() )
						{
							if (( (*vback).pLink!=m_pSafeFirst) && ((*vback).pLink!=pCurrent))
								lstNewNodes.push_back((*vback).pLink);
						}
					}
					vback = pCurrent->link.begin(),vbackend = pCurrent->link.end();
					for (;vback!=vbackend;++vback)
					{
						if ( std::find(lstNewNodes.begin(),lstNewNodes.end(),(*vback).pLink)==lstNewNodes.end())
						{
							if  (( (*vback).pLink!=m_pSafeFirst) && ((*vback).pLink!=pPair) )
								lstNewNodes.push_back((*vback).pLink);
						}
					}


					Disconnect(pPair);
					Disconnect(pCurrent);

					ListNodes::iterator lif = std::find(m_lstNodeStack.begin(),m_lstNodeStack.end(),pPair);
					while (lif != m_lstNodeStack.end())
					{
						m_lstNodeStack.erase(lif);
						lif = std::find(m_lstNodeStack.begin(),m_lstNodeStack.end(),pPair);
					}
					lif = std::find(m_lstNodeStack.begin(),m_lstNodeStack.end(),pCurrent);
					while (lif != m_lstNodeStack.end())
					{
						m_lstNodeStack.erase(lif);
						lif = std::find(m_lstNodeStack.begin(),m_lstNodeStack.end(),pCurrent);
					}
					
					ConnectNodes(lstNewNodes);
			}
		
		}
		// find optimal rearrangement
		// rearrange
	}

	ClearMarks();
}

void CGraph::DisableInSphere(const Vec3 &pos,float fRadius)
{
	GetNodesInSphere(pos,fRadius);
	ListNodes::iterator li = m_lstNodesInsideSphere.begin(),liend = m_lstNodesInsideSphere.end();
	for (;li!=liend;++li)
	{
		GraphNode *pNode = (*li);
		if (pNode->nBuildingID<0) 
			continue;

		VectorOfLinks::iterator vli=pNode->link.begin(),vliend=pNode->link.end();
		for (;vli!=vliend;++vli)
		{
			(*vli).fMaxRadius = -1;	// forbid this link

			GraphNode *pLink = (*vli).pLink;
			VectorOfLinks::iterator bi=pLink->link.begin(),biend=pLink->link.end();
			for (;bi!=biend;++bi)
			{
				if ((*bi).pLink == pNode)
				{
					(*bi).fMaxRadius = -1;
					break;
				}
			}

		}
	}

}

void CGraph::EnableInSphere(const Vec3 &pos,float fRadius)
{
	GetNodesInSphere(pos,fRadius);
	ListNodes::iterator li = m_lstNodesInsideSphere.begin(),liend = m_lstNodesInsideSphere.end();
	for (;li!=liend;++li)
	{
		GraphNode *pNode = (*li);
		if (pNode->nBuildingID<0) 
			continue;

		VectorOfLinks::iterator vli=pNode->link.begin(),vliend=pNode->link.end();
		for (;vli!=vliend;++vli)
		{
			(*vli).fMaxRadius = 100;	// allow this link

			GraphNode *pLink = (*vli).pLink;
			VectorOfLinks::iterator bi=pLink->link.begin(),biend=pLink->link.end();
			for (;bi!=biend;++bi)
			{
				if ((*bi).pLink == pNode)
				{
					(*bi).fMaxRadius = 100;
					break;
				}
			}
		}
	}

}

int CGraph::GetNodesInSphere(const Vec3 &pos, float fRadius)
{
	ClearTagsNow();
	m_lstNodesInsideSphere.clear();
	GraphNode *pNode = GetEnclosing(pos);

	if (pNode->nBuildingID<0)
		return 0;

	
	m_lstNodesInsideSphere.push_front(pNode);
	while(m_lstNodesInsideSphere.front()->tag == false)
	{

		ListNodes::iterator li = m_lstNodesInsideSphere.begin(),liend = m_lstNodesInsideSphere.end();
		for (;li!=liend;)
		{
			ListNodes::iterator linext = li;
			++linext;
			if (linext!=liend)
			{
				if ((*linext)->tag)
					break;
			}
			++li;
		}

		if (li!=liend)
			pNode = (*li);
		TagNode(pNode);

		VectorOfLinks::iterator vli=pNode->link.begin(),vliend=pNode->link.end();
		for (;vli!=vliend;++vli)
		{
			GraphNode *pLink = (*vli).pLink;
			if (pLink->tag)
				continue;
			if ( GetLength(pLink->data.m_pos-pos) < fRadius )
			{
				m_lstNodesInsideSphere.push_front(pLink);
			}
		}
	}

	ClearTagsNow();
	return m_lstNodesInsideSphere.size();

}
