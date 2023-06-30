// Heuristic.cpp: implementation of the CHeuristic class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IAgent.h"
#include "Heuristic.h"
#include "Graph.h"
#include "AIObject.h"
#include "Cry_Math.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHeuristic::CHeuristic()
{
}

CHeuristic::~CHeuristic()
{

}

float CHeuristic::Estimate(GraphNode *pNode, CGraph* graph)
{
	// DEFAULT HEURISTIC LIKES EVERYTHING :)
	
//if (m_BaseValues.bWater != data.bWater) return 0;

//	float diff;
//	diff = m_BaseValues.fSlope - data.fSlope;
	
	return 1;
}


float CStandardHeuristic::Estimate(GraphNode *pNode, CGraph* graph)
{
float	estimation = 0.0f;
	GameNodeData data = pNode->data;
	// avoids water
//	if (data.bWater)
	//	return estimation;

	// avoids big slopes
	//estimation = 1 - data.fSlope;
	

	// calculate minimum distance from all tagged neighboors
	float mindist = 1000.f;	// an arbitrary large value
	GraphNode *pPrevious = 0;
	VectorOfLinks::iterator vli;
	for (vli=pNode->link.begin();vli!=pNode->link.end();vli++)
	{
		if ((*vli).pLink->tag)
		{
			float dist = ((*vli).pLink->data.m_pos - pNode->data.m_pos).GetLength();
			if (dist < mindist)
			{
				mindist = dist;
				pPrevious = (*vli).pLink;
			}
		}
	}
	if (pPrevious)
	{
		// paths that are very much longer than the straight path should be suppressed
		pNode->fDistance = pPrevious->fDistance + mindist;
		estimation += 1.f - (pNode->fDistance / graph->m_fDistance) * 0.5f;
	}
	return estimation;
}


float CVehicleHeuristic::Estimate(GraphNode *pNode, CGraph* graph)
{
float	estimation = 0.0f;
Vec3d candidateDir;
Vec3d curDir;
GraphNode *pPrev = 0;
float maxheur=pNode->fHeuristic;
//VectorOfLinks::iterator vi;
bool	firstStep = false;

//return 1;

	if( pNode->nBuildingID<0 )	// outdoors
	{
		CStandardHeuristic outdoorHeur;
		estimation = outdoorHeur.Estimate(pNode, graph);
		// just use it for now - somehove vehicle heuristic seems not to work, blin
		return estimation;
//		return 	outdoorHeur.Estimate(pNode, graph);
	}

//return 5 - estimation;

	size_t sz = graph->m_lstCurrentHistory.size();
	if(sz>2)
int tooBig=1;

	if(graph->m_lstCurrentHistory.size()==2)
	{
ListNodes::iterator prev = graph->m_lstCurrentHistory.begin();
ListNodes::iterator prevPrev = prev;
++prevPrev;
		curDir = (*graph->m_lstCurrentHistory.begin())->data.m_pos - (*(++graph->m_lstCurrentHistory.begin()))->data.m_pos;	
		candidateDir = pNode->data.m_pos - (*graph->m_lstCurrentHistory.begin())->data.m_pos;
	}
	else if(graph->m_lstCurrentHistory.size()==1)
	{
		curDir = (*graph->m_lstCurrentHistory.begin())->data.m_pos - graph->GetRequester()->GetPos();		
		candidateDir = pNode->data.m_pos - (*graph->m_lstCurrentHistory.begin())->data.m_pos;
	}
	else
	{
		Vec3d vAngles = graph->GetRequester()->GetAngles();
		curDir = Vec3d(0, -1, 0);
		Matrix44 mat;
		mat.SetIdentity();
		mat=Matrix44::CreateRotationZYX(-gf_DEGTORAD*vAngles)*mat; //NOTE: angles in radians and negated
		curDir = mat.TransformPointOLD(curDir);
		candidateDir = pNode->data.m_pos - graph->GetRequester()->GetPos();
		firstStep = true;
	}

	candidateDir.z = 0.0f;
	curDir.z = 0.0f;
	candidateDir.normalize();
	curDir.normalize();

	float	dotz = candidateDir.x * curDir.x + candidateDir.y * curDir.y;

//	if( fabs(dotz)<.05f )
//		return estimation;

	if(firstStep)	// first step - use requester direction)
	{
		if( dotz<0.0f )		// it's behind
			dotz = (dotz + 1.0f)*.2f;
		else
		{
//			if(dotz < .8f)
//				dotz = .8f + (dotz-.8f)*.2f;
			dotz = .2f + dotz*dotz*dotz*dotz*5.0f;
		}
	}
	else
	{
		if( dotz<0.0f )		// it's behind
			dotz = (dotz + 1.0f)*.2f;
		else
		{
//			if(dotz > .8f)
//				dotz = .8f + (dotz-.8f)*.2f;
			dotz = .2f + dotz*dotz*8.37f;
		}
	}

//	estimation = dotz*.7f + (1 - pNode->data.fSlope)*.3f;
//	estimation = dotz;//*.7f + (1 - data.fSlope)*.3f;
//	estimation += dotz*.3f;//*.7f + (1 - data.fSlope)*.3f;
	estimation += dotz*.3f;//*.7f + (1 - data.fSlope)*.3f;

	// avoids water
//	if (data.bWater)
//		return 0;

	// avoids big slopes

	return estimation;
}
