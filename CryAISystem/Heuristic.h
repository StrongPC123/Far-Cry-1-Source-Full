// Heuristic.h: interface for the CHeuristic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HEURISTIC_H__84C857C2_E03E_46B5_B45F_1F0E470A7352__INCLUDED_)
#define AFX_HEURISTIC_H__84C857C2_E03E_46B5_B45F_1F0E470A7352__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CGraph;

class CHeuristic  
{
	//GameNodeData	m_BaseValues;
public:
	CHeuristic(/*const GameNodeData &basevalues*/);
	virtual ~CHeuristic();

	virtual float Estimate(GraphNode *pNode, CGraph* graph );
};

class CStandardHeuristic : public CHeuristic
{
public:
	float Estimate(GraphNode *pNode, CGraph* graph);
};

class CVehicleHeuristic : public CHeuristic
{
public:
	float Estimate(GraphNode *pNode, CGraph* graph);
};


#endif // !defined(AFX_HEURISTIC_H__84C857C2_E03E_46B5_B45F_1F0E470A7352__INCLUDED_)
