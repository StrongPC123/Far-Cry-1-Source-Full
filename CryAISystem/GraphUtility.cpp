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


//
//--------------------------------------------------------------------------------------------------
int CGraph::Rearrange( ListNodes& nodesList, const Vec3d& cutStart, const Vec3d& cutEnd )
{
int counter=0;
GraphNode	*curNode;
ListNodes::iterator it;
int				ndNumber=0;

	for(it=nodesList.begin(); it!=nodesList.end(); it++, ndNumber++)
	{
	int counter=0;
		curNode = (*it);
//		VectorOfLinks::iterator vi;
		if( curNode->vertex.size()!=3 || curNode == m_pSafeFirst)
		{
			nodesList.erase( it );
			it=nodesList.begin();
		}
	}


	counter = ProcessMegaMerge( nodesList, cutStart, cutEnd );
	return counter;
}

//
//--------------------------------------------------------------------------------------------------
int CGraph::ProcessMegaMerge( ListNodes& nodesList, const Vec3d& vCutStart, const Vec3d& vCutEnd )
{
ListNodes	leftSideNodes;
ListNodes	rightSideNodes;
int				Counter=0;
Vec3d			cutDirection = vCutEnd-vCutStart;
//Obstacles	leftOutline;
//Obstacles	rightOutline;
ObstacleIndexList	leftOutline;
ObstacleIndexList	rightOutline;
//ListPositions	leftOutline;
//ListPositions	rightOutline;
GraphNode	*curNode;
VectorOfLinks::iterator vi;

ObstacleData	tmpObst;
tmpObst.vPos = vCutEnd;
int	cutEnd = m_pAISystem->m_VertexList.FindVertex( tmpObst );
tmpObst.vPos = vCutStart;
int	cutStart = m_pAISystem->m_VertexList.FindVertex( tmpObst );


	if( cutStart<0 || cutEnd<0 )
	{
		m_pAISystem->m_pSystem->GetILog()->Log("\001 CGraph::ProcessMegaMerge  cut is not in m_VertexList %d %d", cutStart, cutEnd);
		AIWarning( "CGraph::ProcessMegaMerge  cut is not in m_VertexList %d %d", cutStart, cutEnd);

		assert(0);
		return 0;
	}

	if( nodesList.empty() )
		return 0;

	ListNodes::iterator it;

DbgCheckList( nodesList );

	Vec3d	cutN = cutDirection.normalized();

	for(it=nodesList.begin(); it!=nodesList.end(); it++)
	{
		curNode = (*it);

		if (std::find(rightSideNodes.begin(),rightSideNodes.end(),curNode) != rightSideNodes.end()) // already in list - skip
			continue;
		if (std::find(leftSideNodes.begin(),leftSideNodes.end(),curNode) != leftSideNodes.end()) // already in list - skip
			continue;

		GraphLink* pNewLink = curNode->FindNewLink();
		if( pNewLink )
		{
			GraphNode	*nbrNode = pNewLink->pLink;
			VectorOfLinks::iterator nbrVi;
			float curCross = curNode->GetCross( vCutStart, cutN, *pNewLink );

			for (nbrVi=nbrNode->link.begin();nbrVi!=nbrNode->link.end();nbrVi++)
				if((*nbrVi).pLink == curNode )
					break;
			// mark neighbor - it's a new link in edge
			(*nbrVi).fMaxRadius = -1.f;
			float nbrCross = nbrNode->GetCross( vCutStart, cutN, (*nbrVi) );

			if( fabs(nbrCross) > fabs(curCross) )
				curCross = -nbrCross;
			if( curCross<0 )
			{
				leftSideNodes.push_back( curNode );
				rightSideNodes.push_back( nbrNode );
			}
			else
			{
				leftSideNodes.push_back( nbrNode );
				rightSideNodes.push_back( curNode );
			}
		}

	}

	if(leftSideNodes.size()!=rightSideNodes.size())
	{
		assert(0);	// triangulation optimisation error -- newly-marked nodes are not simmetrical
		return 0;
	}

DbgCheckList( nodesList );
	for(it=nodesList.begin(); it!=nodesList.end(); it++)
	{
		curNode = (*it);
		if (std::find(leftSideNodes.begin(),leftSideNodes.end(),curNode) != leftSideNodes.end()) // already in list - skip
			continue;
		if (std::find(rightSideNodes.begin(),rightSideNodes.end(),curNode) != rightSideNodes.end()) // already in list - skip
			continue;
		if(		( curNode->vertex[0] == cutStart ) ||
				( curNode->vertex[1] == cutStart ) ||
				( curNode->vertex[2] == cutStart ) ||
				( curNode->vertex[0] == cutEnd ) ||
				( curNode->vertex[1] == cutEnd ) ||
				( curNode->vertex[2] == cutEnd ) )	
			continue;				// don't touch begin/end of cut

		bool bHasPointOnEdge=false;
		int vIdx( 0 );
		for( ; vIdx<3 && !bHasPointOnEdge; vIdx++ )
		{
			for(ListNodes::iterator li=rightSideNodes.begin();li!=rightSideNodes.end() && !bHasPointOnEdge;li++)
			{
			GraphNode	*refNode = (*li);
			GraphLink	*pNewLink = refNode->FindNewLink();
				if( !pNewLink )
					continue;
				if( refNode->vertex[pNewLink->nStartIndex] == curNode->vertex[vIdx] ||
					refNode->vertex[pNewLink->nEndIndex] == curNode->vertex[vIdx] )
				// found it - it's on cut - add the hode to outline
					bHasPointOnEdge = true; 
			}	
		}
		if( !bHasPointOnEdge )
			continue;
		vIdx--;
		// so just check now if it's left or right side and add to appropriate list
		int otherVertexIdx1 = 2 - vIdx;
		int otherVertexIdx2 = 3 - (otherVertexIdx1 + vIdx);

		Vec3d theOtherVertex = m_pAISystem->m_VertexList.GetVertex(curNode->vertex[otherVertexIdx1]).vPos
														- vCutStart;
		theOtherVertex.normalize();
		float curCross = cutN.x * theOtherVertex.y - cutN.y * theOtherVertex.x;
		theOtherVertex = m_pAISystem->m_VertexList.GetVertex(curNode->vertex[otherVertexIdx2]).vPos
														- vCutStart;
		theOtherVertex.normalize();
		float cross2 = cutN.x * theOtherVertex.y - cutN.y * theOtherVertex.x;

		if( fabs(curCross) < fabs(cross2) )
			curCross = cross2;
		if( curCross<0 )
			leftSideNodes.push_back( curNode );
		else
			rightSideNodes.push_back( curNode );

/*
			Vec3d	theOtherVertex = m_pAISystem->m_VertexList.GetVertex(
								curNode->vertex[3-((*vi).nStartIndex + (*vi).nEndIndex)]).vPos
														- vCutStart;
			float cross = cutDirection.x * theOtherVertex.y - cutDirection.y * theOtherVertex.x;

			if( cross<0 )
			{
				if (std::find(leftSideNodes.begin(),leftSideNodes.end(),(*vi).pLink) != leftSideNodes.end())
				{
						bool hasPointOnEdge = false;
						for(ListNodes::iterator li=leftSideNodes.begin();li!=leftSideNodes.end() && !hasPointOnEdge;li++)
						{
						GraphNode	*refNode = (*li);
						VectorOfLinks::iterator refLinks;
							for(refLinks=refNode->link.begin();refLinks!=refNode->link.end();refLinks++)
								if((*refLinks).IsNewLink())
								{
									if( ( refNode->vertex[(*refLinks).nStartIndex] == curNode->vertex[(*vi).nStartIndex] ) ||
											( refNode->vertex[(*refLinks).nStartIndex] == curNode->vertex[(*vi).nEndIndex] ) ||
											( refNode->vertex[(*refLinks).nEndIndex] == curNode->vertex[(*vi).nStartIndex] ) ||
											( refNode->vertex[(*refLinks).nEndIndex] == curNode->vertex[(*vi).nEndIndex] ) 
											)
									{
										hasPointOnEdge = true;
									}
									break;
								}
						}
						if( hasPointOnEdge )
						{
							leftSideNodes.push_back( curNode );
							startFromBeginning = true;
							break;
						}
	//				break;
				}
			}
			else
			if (std::find(rightSideNodes.begin(),rightSideNodes.end(),(*vi).pLink) != rightSideNodes.end())
			{
					bool hasPointOnEdge = false;
					for(ListNodes::iterator li=rightSideNodes.begin();li!=rightSideNodes.end() && !hasPointOnEdge;li++)
					{
					GraphNode	*refNode = (*li);
					VectorOfLinks::iterator refLinks;
						for(refLinks=refNode->link.begin();refLinks!=refNode->link.end();refLinks++)
							if((*refLinks).IsNewLink())
							{
								if( (refNode->vertex[(*refLinks).nStartIndex] == curNode->vertex[(*vi).nStartIndex] ) ||
										(refNode->vertex[(*refLinks).nStartIndex] == curNode->vertex[(*vi).nEndIndex] ) ||
										(refNode->vertex[(*refLinks).nEndIndex] == curNode->vertex[(*vi).nStartIndex] ) ||
										(refNode->vertex[(*refLinks).nEndIndex] == curNode->vertex[(*vi).nEndIndex] ) 
										)
								{
									hasPointOnEdge = true;
								}
								break;
							}
					}
					if( hasPointOnEdge )
					{
//curNode->data.fSlope = 1111;
						rightSideNodes.push_back( curNode );
						startFromBeginning = true;
						break;
					}
//				break;
			}
*/
//		}
//		if( startFromBeginning )
//			it=nodesList.begin();
//		else
//			it++;
	}


//return 0;

	if( leftSideNodes.size() > 1 )
	{
		// create left outline
		leftOutline.push_front( cutEnd );
		leftOutline.push_front( cutStart );
		if(!CreateOutline( leftSideNodes, nodesList, leftOutline ))
			return 0;		// some strange problems (see CreateOutline)  - don't optimize this segment, return
	}
	else
		leftSideNodes.clear();

DbgCheckList( nodesList );
	if( rightSideNodes.size() > 1 )
	{
		// create right outline
		rightOutline.push_front( cutEnd );
		rightOutline.push_front( cutStart );
		if(!CreateOutline( rightSideNodes, nodesList, rightOutline ))
			return 0;		// some strange problems (see CreateOutline)  - don't optimize this segment, return
	}
	else
		rightSideNodes.clear();

	m_DEBUG_outlineListL.clear();
//	m_DEBUG_outlineListL.insert(m_DEBUG_outlineListL.begin(),leftOutline.begin(), leftOutline.end() );

	m_DEBUG_outlineListR.clear();
//	m_DEBUG_outlineListR.insert(m_DEBUG_outlineListR.begin(),rightOutline.begin(), rightOutline.end() );


DbgCheckList( nodesList );
	for( it=leftSideNodes.begin(); !leftSideNodes.empty() && it!=leftSideNodes.end(); it++)
	{
		curNode = (*it);

curNode->data.fSlope = 1111;

		Disconnect( curNode );
		ListNodes::iterator itList = std::find(nodesList.begin(),nodesList.end(),curNode);
		nodesList.erase( itList );
	}

DbgCheckList( nodesList );
	for( it=rightSideNodes.begin(); !rightSideNodes.empty() && it!=rightSideNodes.end(); it++)	
	{
		curNode = (*it);
		Disconnect( curNode );
		ListNodes::iterator itList = std::find(nodesList.begin(),nodesList.end(),curNode);
		nodesList.erase( itList );
	}
//*/
//return 0;

//DbgCheckList( nodesList );
//	ProcessRearrange( nodesList, cutStart, cutEnd );

DbgCheckList( nodesList );
	TriangulateOutline( nodesList, leftOutline, false );

//return 0;

DbgCheckList( nodesList );
	TriangulateOutline( nodesList, rightOutline, true );
DbgCheckList( nodesList );

	ConnectNodes( nodesList );
DbgCheckList( nodesList );


	for( it=nodesList.begin(); it!=nodesList.end(); it++)	
	{
		curNode = (*it);
		for (vi=curNode->link.begin();vi!=curNode->link.end();vi++)
			if( (*vi).fMaxRadius<0.0f ||
//				(curNode->vertex[(*vi).nStartIndex].vPos.x==cutStart.x &&
//				 curNode->vertex[(*vi).nStartIndex].vPos.y==cutStart.y &&
///				 curNode->vertex[(*vi).nEndIndex].vPos.x==cutEnd.x &&
//				 curNode->vertex[(*vi).nEndIndex].vPos.y==cutEnd.y ) ||
//				(curNode->vertex[(*vi).nEndIndex].vPos.x==cutStart.x &&
//				 curNode->vertex[(*vi).nEndIndex].vPos.y==cutStart.y &&
//				 curNode->vertex[(*vi).nStartIndex].vPos.x==cutEnd.x &&
//				 curNode->vertex[(*vi).nStartIndex].vPos.y==cutEnd.y )
//				)
				(curNode->vertex[(*vi).nStartIndex]==cutStart &&
				 curNode->vertex[(*vi).nEndIndex]==cutEnd) ||
				(curNode->vertex[(*vi).nEndIndex]==cutStart&&
				 curNode->vertex[(*vi).nStartIndex]==cutEnd)
				)


				(*vi).fMaxRadius = -10.0f;
	}


DbgCheckList( nodesList );
	return Counter;
}


//
//--------------------------------------------------------------------------------------------------
// creates outline of insideNodes, removes all triangles in outline from nodesList
bool CGraph::CreateOutline( ListNodes& insideNodes, ListNodes& nodesList, ObstacleIndexList&	outline)
{
ListNodes::iterator it;
VectorOfLinks::iterator vi;
GraphNode	*curNode;
//GraphNode	*prevNode;
Vec3d		curOtherVertex;
bool	doneHere = false;
ListNodes	insideNodesCurrent = insideNodes;

	for(it=insideNodesCurrent.begin(); !doneHere; it++)
	//for(it=insideNodesCurrent.begin(); it!=insideNodesCurrent.end(); it++)
	{
		if( it==insideNodesCurrent.end() )
		{
			m_pAISystem->m_pSystem->GetILog()->Log("\001 CGraph::CreateOutline can't find OutlineBegin  ");
			AIWarning( "CGraph::CreateOutline can't find OutlineBegin  ");
			assert( 0 );
			return false;
		}

		curNode = (*it);
		for (vi=curNode->link.begin();vi!=curNode->link.end();vi++)
		{
			if( (*vi).IsNewLink() ) // it's on the edge - it's a new node
				if( (curNode->vertex[(*vi).nStartIndex] == (*outline.begin()) ) ||
						(curNode->vertex[(*vi).nEndIndex] == (*outline.begin()) ) )
				{
					int		theOtherVertexIdx = curNode->vertex[3-((*vi).nStartIndex + (*vi).nEndIndex)];
//					Vec3d	theOtherVertex = m_pAISystem->m_VertexList.GetVertex(theOtherVertexIdx).vPos;

					if( std::find( outline.begin(), outline.end(),theOtherVertexIdx ) == outline.end())
						outline.push_front( theOtherVertexIdx );
//					else
//						assert(0);
					it = insideNodesCurrent.erase( it );
					doneHere=true;
					break;				
				}
		}
		if (doneHere)
			break;
	}

//	assert(curNode);	//
	if( !curNode )
	{
		m_pAISystem->m_pSystem->GetILog()->Log("\001 CGraph::CreateOutline curNode is NULL  ");
		AIWarning( "CGraph::CreateOutline curNode is NULL  ");
		assert( 0 );
		return false;
	}



	while( !insideNodesCurrent.empty() )
	{
	GraphNode	*nextCandidate = NULL;
//	Vec3d	theOtherVertex;
//	Vec3d	nextOtherVertex;
	int		theOtherVertexIdx;
	int		nextOtherVertexIdx=0;
	bool	bAddVertex = true;

		for (vi=curNode->link.begin(); vi!=curNode->link.end()&&bAddVertex; vi++)
			if((*vi).IsNewLink())
				bAddVertex = false;

		//
		// if has links on base
		for (vi=curNode->link.begin();vi!=curNode->link.end();vi++)
		{
			GraphNode *nextNode = (*vi).pLink;
			if ((it=std::find(insideNodesCurrent.begin(),insideNodesCurrent.end(),nextNode)) != insideNodesCurrent.end())
			{
			VectorOfLinks::iterator viNext;
				for (viNext=nextNode->link.begin();viNext!=nextNode->link.end();viNext++)
					if( (*viNext).IsNewLink() )
					{
						nextCandidate = nextNode;
						insideNodesCurrent.erase( it );
						break;
					}
			}
		}
		//
		// if has noBase links
		for (vi=curNode->link.begin();vi!=curNode->link.end();vi++)
		{
			GraphNode *nextNode = (*vi).pLink;
			if ((it=std::find(insideNodesCurrent.begin(),insideNodesCurrent.end(),nextNode)) != insideNodesCurrent.end())
			{
			VectorOfLinks::iterator viNext;
				for (viNext=nextNode->link.begin();viNext!=nextNode->link.end();viNext++)
					if( (*viNext).pLink == curNode )
					{
						if( !nextCandidate )	// no base link - use this one
						{
							nextCandidate = nextNode;
							insideNodesCurrent.erase( it );
							nextOtherVertexIdx = nextNode->vertex[3-((*viNext).nStartIndex + (*viNext).nEndIndex)];
						}
						else
							bAddVertex = false;
						
						break;
					}
			}
		}
		if(bAddVertex)
		{
			if( std::find( outline.begin(), outline.end(),theOtherVertexIdx ) == outline.end())
				outline.push_front( theOtherVertexIdx );
		}
		theOtherVertexIdx = nextOtherVertexIdx;

//		assert(nextCandidate);	//
		if( !nextCandidate )
		{
			m_pAISystem->m_pSystem->GetILog()->Log("\001 CGraph::CreateOutline nextCandidate is NULL  ");
			AIWarning( "CGraph::CreateOutline nextCandidate is NULL  ");
			assert( 0 );
			return false;
		}
		curNode = nextCandidate;
	}
	return true;
}


//
//--------------------------------------------------------------------------------------------------
void CGraph::TriangulateOutline( ListNodes& nodesList, ObstacleIndexList&	outline, bool orientation )
{

	if(outline.empty())
		return;

//ListPositions::reverse_iterator crItr=outline.rbegin();
//Vec3d	vCut1 = *crItr++;
//Vec3d	vCut2 = *crItr;

GraphNode	*candidateNode;
ObstacleData	obst;
GraphNode	tmpNode;
ObstacleIndexList	originalOutline;

	originalOutline.insert(originalOutline.begin(), outline.begin(), outline.end());

//m_DEBUG_outlineListR.clear();


	while(outline.size()>3)
	{
//	GraphNode	tmpNode;
//	ListPositions::iterator	candidateRemove=outline.end();	

	ObstacleIndexList::iterator	candidateRemove=outline.end();	
	bool notGood=false;
	float	candidateDegeneracy=-1.0f;

		candidateNode = CreateNewNode();
		
		int cntr=1;
		for(ObstacleIndexList::iterator curItr=outline.begin(); curItr!=outline.end(); curItr++,cntr++)
		{
		ObstacleIndexList::iterator	vItr1=curItr;
		ObstacleIndexList::iterator	vItr2=curItr;
			if( cntr==outline.size() )
			{
				vItr1 = outline.begin();
				vItr2 = vItr1;
				++vItr2;
			}
			else if( cntr==outline.size()-1 )
			{
				vItr2 = outline.begin();
				++vItr1;
			
			}
			else
			{
				++vItr1;			
				++vItr2;
				++vItr2;
			}

			tmpNode.vertex.clear();
//			obst.vPos = *curItr;
			tmpNode.vertex.push_back( *curItr );
//			obst.vPos = *vItr1;
			tmpNode.vertex.push_back( *vItr1 );
//			obst.vPos = *vItr2;
			tmpNode.vertex.push_back( *vItr2 );

			notGood = false;
			if(tmpNode.IsAntiClockwise()!=orientation)
				notGood = true;
			tmpNode.MakeAntiClockwise();

			// check if enithing inside the curremt triangle
			for(ObstacleIndexList::iterator	tmpItr=originalOutline.begin(); tmpItr!=originalOutline.end() && !notGood; tmpItr++)
				if ( 
//					IsEquivalent()
					*tmpItr == *curItr || *tmpItr == *vItr1 || *tmpItr == *vItr2 )
					continue;
//				else if(PointInTriangle(*tmpItr, &tmpNode))
				else if(PointInTriangle(m_pAISystem->m_VertexList.GetVertex(*tmpItr).vPos, &tmpNode))
					notGood = true;

			if( notGood )	// this triangle can't be used
				continue;

			float	dVal = tmpNode.GetDegeneracyValue();

/*
			if( (vCut1==*curItr && vCut2==*vItr1) ||
				(vCut2==*curItr && vCut1==*vItr1) ||
				(vCut1==*curItr && vCut2==*vItr2) ||
				(vCut2==*curItr && vCut1==*vItr2) ||
				(vCut1==*vItr1 && vCut2==*vItr2) ||
				(vCut2==*vItr1 && vCut1==*vItr2) )
				dVal += 100;
*/
			if( dVal>candidateDegeneracy )	// better candidate
			{
				candidateDegeneracy = dVal;
				candidateNode->vertex = tmpNode.vertex;
				candidateRemove = vItr1;
			}
		}

		//
		//add the best candidate - remove vertex from outline
		candidateNode->MakeAntiClockwise();
		FillGraphNodeData( candidateNode );
		nodesList.push_back( candidateNode );
		assert(candidateRemove != outline.end());
		outline.erase( candidateRemove );
	}


	//
	// add the last triangle
	candidateNode = CreateNewNode();
//	obst.vPos = outline.front();
//	outline.pop_front();
	candidateNode->vertex.push_back( outline.front() );
	outline.pop_front();
//	obst.vPos = outline.back();
	candidateNode->vertex.push_back( outline.back() );
//	obst.vPos = outline.front();
	candidateNode->vertex.push_back( outline.front() );
	candidateNode->MakeAntiClockwise();
	FillGraphNodeData( candidateNode );
	nodesList.push_back( candidateNode );

}


/*
 *	


//
//--------------------------------------------------------------------------------------------------
void CGraph::TriangulateOutline( ListNodes& nodesList, ListPositions&	outline, bool orientation )
{

	if(outline.empty())
		return;

GraphNode	*candidateNode;
ObstacleData	obst;
GraphNode	tmpNode;

	while(outline.size()>3)
	{
//	GraphNode	tmpNode;
	ListPositions::iterator	candidateRemove=outline.end();	
	ListPositions::iterator	tmpVertex;	
	ListPositions::reverse_iterator tmpLastVertex;	
	bool notGood=false;
	float	candidateDegeneracy=-1.0f;

		candidateNode = CreateNewNode();
		//
		// first triangle - begin-next-next
		tmpNode.vertex.clear();
		tmpVertex = outline.begin();
		obst.vPos = *tmpVertex;
		tmpNode.vertex.push_back( obst );
		tmpVertex++;
		obst.vPos = *tmpVertex;
		tmpNode.vertex.push_back( obst );
		tmpVertex++;
		obst.vPos = *tmpVertex;
		tmpNode.vertex.push_back( obst );
		notGood = false;
		if(tmpNode.IsAntiClockwise()!=orientation)
			notGood = true;
		tmpNode.MakeAntiClockwise();

		// check if enithing inside the curremt triangle
		for(tmpVertex++; tmpVertex!=outline.end()&&!notGood;tmpVertex++)
			if(PointInTriangle(*tmpVertex, &tmpNode))
				notGood = true;
		if(!notGood)
		{
		float	dVal = tmpNode.GetDegeneracyValue();
			if( dVal>candidateDegeneracy )	// better candidate
			{
				candidateDegeneracy = dVal;
				candidateNode->vertex = tmpNode.vertex;
				candidateRemove = ++outline.begin();
			}
		}

		//
		// second triangle - begin-next-end
		tmpNode.vertex.clear();
		tmpVertex = outline.begin();
		obst.vPos = *tmpVertex;
		tmpNode.vertex.push_back( obst );
		tmpVertex++;
		obst.vPos = *tmpVertex;
		tmpNode.vertex.push_back( obst );
		obst.vPos = *(outline.rbegin());
		tmpNode.vertex.push_back( obst );
		notGood = false;
		if(tmpNode.IsAntiClockwise()!=orientation)
			notGood = true;
		tmpNode.MakeAntiClockwise();

		// check if enithing inside the curremt triangle
		tmpLastVertex = outline.rbegin();
		tmpLastVertex++;
		for(tmpVertex++; 
			//*tmpVertex!= *tmpLastVertex; 
			!IsEquivalent(*tmpVertex,*tmpLastVertex,VEC_EPSILON) 
			&& !notGood ; 
			tmpVertex++
			)
			if(PointInTriangle(*tmpVertex, &tmpNode))
				notGood = true;
		if(!notGood)
		{
		float	dVal = tmpNode.GetDegeneracyValue();
			if( dVal>candidateDegeneracy )	// better candidate
			{
				candidateDegeneracy = dVal;
				candidateNode->vertex = tmpNode.vertex;
				candidateRemove = outline.begin();
			}
		}
		//
		// third triangle - begin-end-preend
		tmpNode.vertex.clear();
		obst.vPos = *outline.begin();
		tmpNode.vertex.push_back( obst );
		tmpLastVertex = outline.rbegin();
		tmpLastVertex++;					// get preend
		obst.vPos = *tmpLastVertex;		
		tmpNode.vertex.push_back( obst );
		tmpLastVertex = outline.rbegin();	// get end
		obst.vPos = *tmpLastVertex;
		tmpNode.vertex.push_back( obst );
		notGood = false;
		if(tmpNode.IsAntiClockwise()!=orientation)
			notGood = true;
		tmpNode.MakeAntiClockwise();
		tmpLastVertex = outline.rbegin();
		tmpLastVertex++;							// set to preend
		// check if enithing inside the curremt triangle
		for(tmpLastVertex++; 
			//*tmpLastVertex!=*outline.begin()
			!IsEquivalent(*tmpLastVertex,*outline.begin(),VEC_EPSILON)
			&&!notGood;
			tmpLastVertex++
			)
			if(PointInTriangle(*tmpLastVertex, &tmpNode))
				notGood = true;
		if(!notGood)
		{
		float	dVal = tmpNode.GetDegeneracyValue();
			if( dVal>candidateDegeneracy )	// better candidate
			{
				candidateDegeneracy = dVal;
				candidateNode->vertex = tmpNode.vertex;
				candidateRemove = (++outline.rbegin()).base();
			}
		}
		//
		// last triangle - end-preend-prepreend
		tmpNode.vertex.clear();
		tmpLastVertex = outline.rbegin();
		tmpLastVertex++;
		tmpLastVertex++;		// pre-preend
		obst.vPos = *tmpLastVertex;
		tmpNode.vertex.push_back( obst );
		tmpLastVertex--;				// pre-end
		obst.vPos = *tmpLastVertex;
		tmpNode.vertex.push_back( obst );
		tmpLastVertex--;		// end
		obst.vPos = *tmpLastVertex;
		tmpNode.vertex.push_back( obst );
		notGood = false;
		if(tmpNode.IsAntiClockwise()!=orientation)
			notGood = true;
		tmpNode.MakeAntiClockwise();
		tmpLastVertex++;
		tmpLastVertex++;		// set to pre-preend

		// check if enithing inside the curremt triangle
		for(tmpLastVertex++; tmpLastVertex!=(outline.rend())&&!notGood;tmpLastVertex++)
			if(PointInTriangle(*tmpLastVertex, &tmpNode))
				notGood = true;
		if(!notGood)
		{
		float	dVal = tmpNode.GetDegeneracyValue();
			if( dVal>candidateDegeneracy )	// better candidate
			{
				candidateDegeneracy = dVal;
				candidateNode->vertex = tmpNode.vertex;
				candidateRemove = (++(++outline.rbegin())).base();
//				candidateRemove--;
			}
		}

		//
		//add the best candidate - remove vertex from outline
		candidateNode->MakeAntiClockwise();
		FillGraphNodeData( candidateNode );


		nodesList.push_back( candidateNode );
		assert(candidateRemove != outline.end());

ListPositions::iterator dbg=std::find(m_DEBUG_outlineListR.begin(), m_DEBUG_outlineListR.end(), *candidateRemove);
if(dbg!=m_DEBUG_outlineListR.end())
(*dbg).z = -500;



		outline.erase( candidateRemove );


	}


	//
	// add the last triangle
	candidateNode = CreateNewNode();
	obst.vPos = outline.front();
//	obst.bOccupied = false;
	outline.pop_front();
	candidateNode->vertex.push_back( obst );
	obst.vPos = outline.back();
//	obst.bOccupied = false;
	candidateNode->vertex.push_back( obst );
	obst.vPos = outline.front();
	//obst.bOccupied = false;
	candidateNode->vertex.push_back( obst );
	candidateNode->MakeAntiClockwise();
	FillGraphNodeData( candidateNode );
	nodesList.push_back( candidateNode );

}


 */

//
//--------------------------------------------------------------------------------------------------
int CGraph::ProcessRearrange( ListNodes& nodesList, const Vec3d& cutStart, const Vec3d& cutEnd )
{
return 0;
/*
int counter = 0;
ListNodes::iterator	it;	
VectorOfLinks::iterator	vi;
Obstacles::iterator	oi;

int dbgcounter=0;

	for(it=nodesList.begin();it!=nodesList.end();it++)
	{
dbgcounter++;
		GraphNode	*curNode = (*it);
//		if(curNode->link.empty())
		if(curNode->link.size()!=3)
			continue;
		bool onCut = false;
		for (oi=curNode->vertex.begin();oi!=curNode->vertex.end(); oi++)

			if(  IsEquivalent((*oi).vPos,cutStart,VEC_EPSILON) || IsEquivalent((*oi).vPos,cutEnd,VEC_EPSILON) ) 
			{
				onCut = true;
				break;
			}
		if(!onCut)
			continue;
		for (vi=curNode->link.begin();vi!=curNode->link.end();vi++)
		{
			if ( (*vi).fMaxRadius<0.0f )	// it's cut edge
				continue;
			if ((*vi).pLink->link.size()!=3)
				continue;
			if (std::find(nodesList.begin(),nodesList.end(),(*vi).pLink) != nodesList.end()) // link is in list as well
			{
DbgCheckList( nodesList );
				if(ProcessRearrange( curNode, (*vi).pLink, nodesList ))
				{
					it=nodesList.begin();
					counter++;
DbgCheckList( nodesList );
					break;
				}
DbgCheckList( nodesList );
//				break;
			}
		}
	}
	return counter;
*/
}

//
//--------------------------------------------------------------------------------------------------
bool CGraph::ProcessRearrange( GraphNode	*node1, GraphNode	*node2, ListNodes& nodesList )
{
return false;
/*
float			dValue; 
float			dValue1 = node1->GetDegeneracyValue();
float			dValue2 = node2->GetDegeneracyValue();
GraphNode	tmpNode;
ObstacleData	obst[4];
unsigned int	other1VertexIdx;
unsigned int	other2VertexIdx;

	if(dValue1>dValue2)
		dValue1 = dValue2;

	if( dValue1>.02f )
		return false;

VectorOfLinks::iterator vi;	
	for (vi=node2->link.begin();vi!=node2->link.end();vi++)
		if( (*vi).pLink == node1 )
		{
			other2VertexIdx = 3 - ((*vi).nStartIndex + (*vi).nEndIndex);
			break;
		}
	for (vi=node1->link.begin();vi!=node1->link.end();vi++)
		if( (*vi).pLink == node2 )
		{
			other1VertexIdx = 3 - ((*vi).nStartIndex + (*vi).nEndIndex);
			break;
		}

	//check if it is possible configuration for rearranement
	Vec3d	newEdge = node1->vertex[other1VertexIdx].vPos - node2->vertex[other2VertexIdx].vPos;
	Vec3d	v1 = node1->vertex[(*vi).nStartIndex].vPos - node2->vertex[other2VertexIdx].vPos;
	Vec3d	v2 = node1->vertex[(*vi).nEndIndex].vPos - node2->vertex[other2VertexIdx].vPos;
	float cross1 = v1.x * newEdge.y - v1.y * newEdge.x;				
	float cross2 = v2.x * newEdge.y - v2.y * newEdge.x;				
	if( cross1<0 && cross2<0 || cross1>0 && cross2>0  )
		return false;				// can't rearrange this two triangles

	obst[0] = node1->vertex[other1VertexIdx];
	obst[1] = node1->vertex[(*vi).nStartIndex];
	obst[2] = node2->vertex[other2VertexIdx];
	obst[3] = node1->vertex[(*vi).nEndIndex];

//	obst[0].bOccupied = false;
//	obst[1].bOccupied = false;
//	obst[2].bOccupied = false;
//	obst[3].bOccupied = false;

	tmpNode.vertex.push_back( obst[0] );
	tmpNode.vertex.push_back( obst[1] );
	tmpNode.vertex.push_back( obst[2] );

	dValue = tmpNode.GetDegeneracyValue();
	if( dValue<=dValue1 )
		return false;

	tmpNode.vertex.clear();
	tmpNode.vertex.push_back( obst[0] );
	tmpNode.vertex.push_back( obst[2] );
	tmpNode.vertex.push_back( obst[3] );

	dValue = tmpNode.GetDegeneracyValue();
	if( dValue<=dValue1 )
		return false;

DbgCheckList( nodesList );
	Disconnect( node1, true );
//DbgCheckList( nodesList );
	Disconnect( node2, true );
//( nodesList );

	ListNodes::iterator nIt = std::find(nodesList.begin(),nodesList.end(),node1);
if(nIt == nodesList.end())
DbgCheckList( nodesList );

//	if( nIt!=nodesList.end() )
	nodesList.erase( nIt );
	nIt = std::find(nodesList.begin(),nodesList.end(),node2);
if(nIt == nodesList.end())
DbgCheckList( nodesList );
	nodesList.erase( nIt );
DbgCheckList( nodesList );

	node1 = CreateNewNode();
	node1->vertex.push_back( obst[0] );
	node1->vertex.push_back( obst[1] );
	node1->vertex.push_back( obst[2] );
	node1->MakeAntiClockwise();
	FillGraphNodeData( node1 );

	node2 = CreateNewNode();
	node2->vertex.push_back( obst[0] );
	node2->vertex.push_back( obst[2] );
	node2->vertex.push_back( obst[3] );
	node2->MakeAntiClockwise();
	FillGraphNodeData( node2 );

	nodesList.push_front( node1 );
	nodesList.push_front( node2 );

	return true;
*/
}


//
//--------------------------------------------------------------------------------------------------


//
//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
float	GraphNode::GetDegeneracyValue()
{

CVertexList *pVList = &GetAISystem()->m_VertexList;


Vec3d	firstE = pVList->GetVertex(vertex[1]).vPos - pVList->GetVertex(vertex[0]).vPos;
Vec3d	secondE = pVList->GetVertex(vertex[2]).vPos - pVList->GetVertex(vertex[0]).vPos;

	firstE.Normalize();
	secondE.Normalize();
float	cosAngle0 = 1.0f - firstE.Dot( secondE );

	firstE = pVList->GetVertex(vertex[0]).vPos - pVList->GetVertex(vertex[1]).vPos;
	secondE = pVList->GetVertex(vertex[2]).vPos - pVList->GetVertex(vertex[1]).vPos;
	firstE.Normalize();
	secondE.Normalize();
float	cosAngle1 = 1.0f - firstE.Dot( secondE );

	firstE = pVList->GetVertex(vertex[0]).vPos - pVList->GetVertex(vertex[2]).vPos;
	secondE = pVList->GetVertex(vertex[1]).vPos - pVList->GetVertex(vertex[2]).vPos;
	firstE.Normalize();
	secondE.Normalize();
float	cosAngle2 = 1.0f - firstE.Dot( secondE );

	if( cosAngle0<=cosAngle1 && cosAngle0<=cosAngle2 )
		return cosAngle0;
	if( cosAngle1<=cosAngle2 && cosAngle1<=cosAngle0 )
		return cosAngle1;
	return cosAngle2;
}
//
//--------------------------------------------------------------------------------------------------
void GraphNode::MakeAntiClockwise()
{
	if (vertex.size() < 3) 
		return;

	CVertexList *pVList = &GetAISystem()->m_VertexList;

	int od1 = vertex[0];
	int od2 = vertex[1];
	int od3 = vertex[2];

	Vec3d one = pVList->GetVertex(od2).vPos - pVList->GetVertex(od1).vPos;
	Vec3d two = pVList->GetVertex(od3).vPos - pVList->GetVertex(od2).vPos;

	float fcrossz = one.x * two.y - two.x * one.y;

	if (fcrossz<0)
	{
		// rearrange the first and second
		vertex.clear();

		vertex.push_back(od2);
		vertex.push_back(od1);
		vertex.push_back(od3);

//		MakeAntiClockwise();
	}
}


//
//--------------------------------------------------------------------------------------------------
bool GraphNode::IsAntiClockwise()
{
	if (vertex.size() < 3) 
		return false;

	CVertexList *pVList = &GetAISystem()->m_VertexList;

	int od1 = vertex[0];
	int od2 = vertex[1];
	int od3 = vertex[2];

	Vec3d one = pVList->GetVertex(od2).vPos - pVList->GetVertex(od1).vPos;
	Vec3d two = pVList->GetVertex(od3).vPos - pVList->GetVertex(od2).vPos;

	float fcrossz = one.x * two.y - two.x * one.y;

	if (fcrossz<0)
		return false;
	return true;
}


//
//--------------------------------------------------------------------------------------------------
bool CGraph::DbgCheckList( ListNodes& nodesList )	const
{
bool isGood = true;
GraphNode	*curNode;
ListNodes::iterator it;
int				ndNumber=0;

	for(it=nodesList.begin(); it!=nodesList.end(); it++, ndNumber++)
	{
	int counter=0;
		curNode = (*it);
		VectorOfLinks::iterator vi;
		for (vi=curNode->link.begin();vi!=curNode->link.end();vi++)
			if( (*vi).IsNewLink() ) // it's on the edge - it's a new node
				counter++;
		if(counter > 1)
			isGood = false;
		if( curNode->vertex.size()<3 )
			isGood = false;

		if (curNode->nRefCount>1 && curNode->nRefCount<4)
			isGood = false;
		ListNodes::iterator li;
		ListNodes::iterator itnext = it;
		if (++itnext != nodesList.end())
		{
			li = std::find(itnext,nodesList.end(),(*it));
			if (li!=nodesList.end())
				isGood = false;

		}
	}

	return isGood;
}

//
//--------------------------------------------------------------------------------------------------
float GraphNode::GetCross( const Vec3d &vCutStart, const Vec3d &vDir, const GraphLink &theLink )
{
Vec3d	theOtherVertex = GetAISystem()->m_VertexList.GetVertex(vertex[3-(theLink.nStartIndex + theLink.nEndIndex)]).vPos 
										 - vCutStart;
	theOtherVertex.normalize();
	return vDir.x * theOtherVertex.y - vDir.y * theOtherVertex.x;
}

//
//--------------------------------------------------------------------------------------------------
GraphLink*	GraphNode::FindNewLink()
{
VectorOfLinks::iterator	vi;
	for (vi=link.begin();vi!=link.end();vi++)
		if( (*vi).IsNewLink() ) // it's on the edge - it's a new node
			return &(*vi);
	return NULL;
}
//
//--------------------------------------------------------------------------------------------------
