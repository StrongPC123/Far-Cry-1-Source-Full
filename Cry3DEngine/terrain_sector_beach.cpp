////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrain_sector_beach.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: shore siluet calculations
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "terrain_sector.h"

//min size of water area with beaches
#define MIN_UNITS_IN_WATER_AREA 8
	 /*
bool CBeachGenerator::ProcessPoint(short x, short y, int nAreaID)
{
  if(x<0 || y<0 || x>=CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize() || y>=CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize())
    return false;

  if(m_arrAlreadyProcessed[x][y])// || m_WaterAreaMap[x][y]==nAreaID)
    return false;

  m_arrAlreadyProcessed[x][y] = true;

  bool in_water = m_pTerrain->GetZSafe(x*CTerrain::GetHeightMapUnitSize(),y*CTerrain::GetHeightMapUnitSize()) < m_pTerrain->GetWaterLevel();

  if(!in_water)
    return false;

  m_WaterAreaMap[x][y] = nAreaID;

  return true;
}*/

struct TerPoint 
{ 
  TerPoint (short _x, short _y)
  {
    x=_x;
    y=_y;
  }
  short x,y; 
};


void CBeachGenerator::RenameWaterArea( int nOld, int nNew )
{
/*	int nSize = CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize();
  for(int x=0; x<nSize; x++)
  for(int y=0; y<nSize; y++)
		if(m_WaterAreaMap[x][y] == nOld)
			m_WaterAreaMap[x][y] = nNew;*/
}

struct RenameInfo
{
	int nTop,nLeft;
};

int CBeachGenerator::MarkWaterAreas()
{
 	ushort nMaxAreaId = 0;
	int nSize = CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize();

  for(int x=0; x<nSize; x++)
	{
		for(int y=0; y<nSize; y++)
		{
			bool in_water = m_pTerrain->GetZSafe(x*CTerrain::GetHeightMapUnitSize(),y*CTerrain::GetHeightMapUnitSize()) < m_pTerrain->GetWaterLevel();
			m_WaterAreaMap[x][y] = in_water ? 1 : 0;
		}
	}

	m_lstWaterAreaSizeTable.Reset();
  for(int x=0; x<nSize; x++)
  for(int y=0; y<nSize; y++)
	{
		while(m_lstWaterAreaSizeTable.Count()<=m_WaterAreaMap[x][y])
			m_lstWaterAreaSizeTable.Add(int(0));

		m_lstWaterAreaSizeTable[m_WaterAreaMap[x][y]] ++;
	}

  return 1;
}
/*
int CBeachGenerator::MarkWaterAreas()
{
 	ushort nMaxAreaId = 0;
	int nSize = CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize();

	list2<int*> lstAreaPtrTable;
	lstAreaPtrTable.Add(0);
	lstAreaPtrTable[0] = new int;
	*lstAreaPtrTable[0] = ++nMaxAreaId;

	list2<RenameInfo> lstRenameTable;

  for(int x=0; x<nSize; x++)
	{
		for(int y=0; y<nSize; y++)
		{
			bool in_water = m_pTerrain->GetZSafe(x*CTerrain::GetHeightMapUnitSize(),y*CTerrain::GetHeightMapUnitSize()) < m_pTerrain->GetWaterLevel();
			if(!in_water)
			{
				m_WaterAreaMap[x][y] = 0;
				continue;
			}

			int * nLeftAreaId = y>0 ? m_WaterAreaMap[x][y-1] : lstAreaPtrTable[0];
			int * nTopAreaId = x>0 ? m_WaterAreaMap[x-1][y] : lstAreaPtrTable[0];

			if(nLeftAreaId && !nTopAreaId)
			{ // only left valid
				m_WaterAreaMap[x][y] = nLeftAreaId;
			}
			else if(nTopAreaId && !nLeftAreaId)
			{	// only top valid
				m_WaterAreaMap[x][y] = nTopAreaId;
			}
			else if(nTopAreaId && nTopAreaId == nLeftAreaId)
			{ // both are the same and valid
				m_WaterAreaMap[x][y] = nLeftAreaId;
			}
			else if(nTopAreaId && nLeftAreaId && (nTopAreaId != nLeftAreaId))
			{ // both valid and different, // todo: rename top units
				RenameInfo ri;
				ri.nTop = *nTopAreaId;
				ri.nLeft = *nLeftAreaId;
				lstRenameTable.Add(ri);

//				*nTopAreaId = *nLeftAreaId;
				m_WaterAreaMap[x][y] = nTopAreaId;
			}
			else if(!nTopAreaId && !nLeftAreaId)
			{ // both zero - new area
				m_WaterAreaMap[x][y] = new int;
				*m_WaterAreaMap[x][y] = ++nMaxAreaId;
				lstAreaPtrTable.Add(m_WaterAreaMap[x][y]);
			}
			else
			{
				assert(0);
			}

		}
	}

	// rename units
	for(int i=lstRenameTable.Count()-1; i>0; i--)
	{
		RenameInfo * pRi = &lstRenameTable[i];

		if(pRi->nLeft<pRi->nTop)
		{
			for(int n=0; n<lstAreaPtrTable.Count(); n++)
				if(*lstAreaPtrTable[n] == pRi->nTop)
					*lstAreaPtrTable[n] = pRi->nLeft;
		}
		else
		{
			for(int n=0; n<lstAreaPtrTable.Count(); n++)
				if(*lstAreaPtrTable[n] == pRi->nLeft)
					*lstAreaPtrTable[n] = pRi->nTop;
		}
	}


	m_lstWaterAreaSizeTable.Reset();
  for(int x=0; x<nSize; x++)
  for(int y=0; y<nSize; y++)
		if(m_WaterAreaMap[x][y])
	{
		while(m_lstWaterAreaSizeTable.Count()<=*m_WaterAreaMap[x][y])
			m_lstWaterAreaSizeTable.Add(int(0));

		m_lstWaterAreaSizeTable[*m_WaterAreaMap[x][y]] ++;
	}

	int nAreasCount=0;
	for(int i=1; i<m_lstWaterAreaSizeTable.Count(); i++)
	if(m_lstWaterAreaSizeTable[i])
		nAreasCount++;
	
  return nAreasCount;
}*/

/*
int CBeachGenerator::MarkOutWater()
{
  list2<TerPoint> Stack;
  Stack.PreAllocate(8000000/sizeof(TerPoint));

  m_lstWaterAreaSizeTable.Reset();

  ushort nAreaID = 1, nStep = MIN_UNITS_IN_WATER_AREA*4;
  for(int X=nStep; X<CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize()-nStep; X+=nStep)
  for(int Y=nStep; Y<CTerrain::GetTerrainSize()/CTerrain::GetHeightMapUnitSize()-nStep; Y+=nStep)
  {
    TerPoint CurPoint(X,Y);

    int nCounter = 0;

    while(1)
    {
      // recursively mark neighbor units of under water area
      if(ProcessPoint(CurPoint.x,CurPoint.y,nAreaID))
      {
        nCounter++;
			  if(CurPoint.x<m_arrAlreadyProcessed.m_nSize && CurPoint.y<m_arrAlreadyProcessed.m_nSize && 
					CurPoint.x>0 && CurPoint.y>0 && 
					!m_arrAlreadyProcessed[CurPoint.x][CurPoint.y+1])
					Stack.Add(TerPoint(CurPoint.x  ,CurPoint.y+1));
			  
				if(CurPoint.x<m_arrAlreadyProcessed.m_nSize && CurPoint.y<m_arrAlreadyProcessed.m_nSize && 
					CurPoint.x>0 && CurPoint.y>0 && 
					!m_arrAlreadyProcessed[CurPoint.x][CurPoint.y-1])
					Stack.Add(TerPoint(CurPoint.x  ,CurPoint.y-1));
			  
				if(CurPoint.x<m_arrAlreadyProcessed.m_nSize && CurPoint.y<m_arrAlreadyProcessed.m_nSize && 
					CurPoint.x>0 && CurPoint.y>0 && 
					!m_arrAlreadyProcessed[CurPoint.x-1][CurPoint.y])
					Stack.Add(TerPoint(CurPoint.x-1,CurPoint.y  ));
			  
				if(CurPoint.x<m_arrAlreadyProcessed.m_nSize && CurPoint.y<m_arrAlreadyProcessed.m_nSize && 
					CurPoint.x>0 && CurPoint.y>0 && 
					!m_arrAlreadyProcessed[CurPoint.x+1][CurPoint.y])
					Stack.Add(TerPoint(CurPoint.x+1,CurPoint.y  ));
      }
      else if(Stack.Count())
      {
        CurPoint = Stack.Last();
        Stack.DeleteLast();
      }
      else
        break;
    }

    while(m_lstWaterAreaSizeTable.Count()<=nAreaID)
      m_lstWaterAreaSizeTable.Add(int(0));

    m_lstWaterAreaSizeTable[nAreaID] = nCounter;

    if(nCounter>m_nMaxAreaSize)
      m_nMaxAreaSize = nCounter;

    if(nCounter)
      nAreaID++;

    break; // only out water
  }

  return 0;
}*/

void CSectorInfo::MakeBeachStage1()
{
	if(!m_pTerrain->m_pBeachGenerator->m_lstWaterAreaSizeTable.Count())
		return;
  
  m_bBeachPresent = false;
  
  for(int x=m_nOriginX; x<=m_nOriginX+CTerrain::GetSectorSize(); x+=CTerrain::GetHeightMapUnitSize())
  for(int y=m_nOriginY; y<=m_nOriginY+CTerrain::GetSectorSize(); y+=CTerrain::GetHeightMapUnitSize())
  {
    bool in_water = m_pTerrain->GetZSafe(x,y) < m_pTerrain->GetWaterLevel();
    bool beach = 0;
    for(int _x=x-CTerrain::GetHeightMapUnitSize(); _x<=x+CTerrain::GetHeightMapUnitSize(); _x+=CTerrain::GetHeightMapUnitSize())
    for(int _y=y-CTerrain::GetHeightMapUnitSize(); _y<=y+CTerrain::GetHeightMapUnitSize(); _y+=CTerrain::GetHeightMapUnitSize())
    {
      bool _in_water = (m_pTerrain->GetZSafe(_x,_y) < m_pTerrain->GetWaterLevel()) 
        && (_x>0 && _y>0 && _x<CTerrain::GetTerrainSize() && _y<CTerrain::GetTerrainSize()) 
        && (m_pTerrain->m_pBeachGenerator->m_lstWaterAreaSizeTable
				[
					(m_pTerrain->m_pBeachGenerator->m_WaterAreaMap[_x/CTerrain::GetHeightMapUnitSize()][_y/CTerrain::GetHeightMapUnitSize()])
				]
				>MIN_UNITS_IN_WATER_AREA);

      if(in_water != _in_water && (x==_x || y==_y))
      {
        beach = true;
        m_bBeachPresent = true;
        break;
      }
    }

    m_pTerrain->m_pBeachGenerator->m_arrBeachMap[x/CTerrain::GetHeightMapUnitSize()][y/CTerrain::GetHeightMapUnitSize()].beach = beach;
    m_pTerrain->m_pBeachGenerator->m_arrBeachMap[x/CTerrain::GetHeightMapUnitSize()][y/CTerrain::GetHeightMapUnitSize()].in_water = in_water;
  }
}

void CSectorInfo::MakeBeachStage2(FILE * hFileToSave)
{
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * m_pVertBufferBeach=0;
  int m_nVertNumBeach=0;

  if(!m_bBeachPresent)
  { 
    if(hFileToSave)
      GetSystem()->GetIPak()->FPrintf(hFileToSave, "BeachInfo(%8d)",GetSecIndex());
    // m_nVertNumBeach is zero = no beach here
    if(hFileToSave)
      GetSystem()->GetIPak()->FWrite(&m_nVertNumBeach,1,sizeof(m_nVertNumBeach),hFileToSave);
    return;
  }

  int group;
  for(group=0; group<MAX_BEACH_GROUPS; group++)
    m_arrlistBeachVerts[group].Clear();

  for(int x=m_nOriginX; x<=m_nOriginX+CTerrain::GetSectorSize(); x+=CTerrain::GetHeightMapUnitSize())
  for(int y=m_nOriginY; y<=m_nOriginY+CTerrain::GetSectorSize(); y+=CTerrain::GetHeightMapUnitSize())
  {
    if(m_pTerrain->m_pBeachGenerator->m_arrBeachMap[x/CTerrain::GetHeightMapUnitSize()][y/CTerrain::GetHeightMapUnitSize()].beach && 
      !m_pTerrain->m_pBeachGenerator->m_arrBeachMap[x/CTerrain::GetHeightMapUnitSize()][y/CTerrain::GetHeightMapUnitSize()].in_water)
    {
      Vec3d water_dir(0,0,0);
      for(int _x=x-CTerrain::GetHeightMapUnitSize(); _x<=x+CTerrain::GetHeightMapUnitSize(); _x+=CTerrain::GetHeightMapUnitSize())
      for(int _y=y-CTerrain::GetHeightMapUnitSize(); _y<=y+CTerrain::GetHeightMapUnitSize(); _y+=CTerrain::GetHeightMapUnitSize())
			if(_x>=0 && _y>=0 && _x<CTerrain::GetTerrainSize() && _y<CTerrain::GetTerrainSize())
      if(m_pTerrain->m_pBeachGenerator->m_arrBeachMap[_x/CTerrain::GetHeightMapUnitSize()][_y/CTerrain::GetHeightMapUnitSize()].in_water)
        water_dir += Vec3d(float(_x-x),float(_y-y),0);

      water_dir.Normalize();
      Vec3d border_pos((float)x,(float)y,m_pTerrain->GetWaterLevel());
      
      BeachPairStruct pair;
      pair.water_dir = water_dir;

      water_dir/=100;      
      int t=0;
      while(m_pTerrain->GetZApr(border_pos.x,border_pos.y)>m_pTerrain->GetWaterLevel() && t<100)
      { 
        border_pos += water_dir; 
        t++; 
      }

    //  assert(t<100);

/*      if( border_pos.x>=m_nOriginX && 
          border_pos.y>=m_nOriginY && 
          border_pos.x<(m_nOriginX+CTerrain::GetSectorSize()) && 
          border_pos.y<(m_nOriginY+CTerrain::GetSectorSize()) )*/
      {
        pair.pos = border_pos ;//+ Vec3d(0,0,0.01f);
        m_lstUnsortedBeachVerts.Add(pair);
      }
/*      else
      {
        if(GetSectorFromPoint(border_pos.x,border_pos.y) == this
      }*/
    }
  }

  if(!m_lstUnsortedBeachVerts.Count())
  { 
    if(hFileToSave)
      GetSystem()->GetIPak()->FPrintf(hFileToSave, "BeachInfo(%8d)",GetSecIndex());
    // m_nVertNumBeach is zero = no beach here
    if(hFileToSave)
      GetSystem()->GetIPak()->FWrite(&m_nVertNumBeach,1,sizeof(m_nVertNumBeach),hFileToSave);
    return;
  }

  for(group=0; group<MAX_BEACH_GROUPS; group++)
  {
    int prev_closest_id = -1;
    m_arrlistBeachVerts[group].Clear();

    // search for free starting point
    int first;
    for(first=0; first<m_lstUnsortedBeachVerts.Count(); first++)
    if(m_lstUnsortedBeachVerts[first].busy < 2)
      break;

    if(first>=m_lstUnsortedBeachVerts.Count())
      break; // no more free points
    
    m_arrlistBeachVerts[group].Add(m_lstUnsortedBeachVerts[first]);
    m_lstUnsortedBeachVerts[first].busy ++;

    for(int pass=0; pass<m_lstUnsortedBeachVerts.Count(); pass++)
    {
      // find totaly free point closest to last result point
      int closest_id = -1;
      {
        float closest_dist = 100000;
        for(int j=0; j<m_lstUnsortedBeachVerts.Count(); j++)
        if(m_lstUnsortedBeachVerts[j].busy==0)
        {
          float dist = GetDistance( m_arrlistBeachVerts[group].Last().pos, m_lstUnsortedBeachVerts[j].pos);
          if(dist<closest_dist && dist<4 && dist!=0)
          {
            closest_dist = dist;
            closest_id = j;
          }
        }
      }

      if(closest_id>=0)
      { // if found add into current group
        if(prev_closest_id>=0)
          m_lstUnsortedBeachVerts[prev_closest_id].busy++;
        prev_closest_id = closest_id;

        m_arrlistBeachVerts[group].Add(m_lstUnsortedBeachVerts[closest_id]);
        m_lstUnsortedBeachVerts[closest_id].busy ++;
      }
      else
      {
        if(prev_closest_id>=0)
          m_lstUnsortedBeachVerts[prev_closest_id].busy++;
        prev_closest_id = closest_id;

        closest_id = -1;
        { // try to find semi free point as last
          float closest_dist = 100000;
          for(int j=0; j<m_lstUnsortedBeachVerts.Count(); j++)
          if(m_lstUnsortedBeachVerts[j].busy==1)
          {
            // skip point if it is already used in this group
            int k;
            for(k=0; k<m_arrlistBeachVerts[group].Count(); k++)
            if(GetDistance(m_arrlistBeachVerts[group][k].pos,m_lstUnsortedBeachVerts[j].pos) == 0)
              break;

            if(k<m_arrlistBeachVerts[group].Count())
              continue;

            float dist = GetDistance(m_arrlistBeachVerts[group].Last().pos,m_lstUnsortedBeachVerts[j].pos);
            if(dist<closest_dist && dist<4 && dist!=0)
            {
              closest_dist = dist;
              closest_id = j;
            }
          }
        }
        
        if(closest_id>=0)
        { // if found add into current group
          m_arrlistBeachVerts[group].Add(m_lstUnsortedBeachVerts[closest_id]);
          m_lstUnsortedBeachVerts[closest_id].busy ++;
        }

        if(m_arrlistBeachVerts[group].Count()==1)
          m_arrlistBeachVerts[group].Clear();
  
        break;
      }
    }
  }

//  for(int u=0; u<UnsortedBeachVerts.Count(); u++)
  //  CSystem::GetRenderer()->DrawLabel(UnsortedBeachVerts[u].pos,1,"%d",UnsortedBeachVerts[u].busy);

  // smooth
  for(group=0; group<MAX_BEACH_GROUPS; group++)
  {
    for(int i=1; i<m_arrlistBeachVerts[group].Count()-1; i++)
    {
      Vec3d dir1 = m_arrlistBeachVerts[group][i+1].pos - m_arrlistBeachVerts[group][i].pos;
      dir1.Normalize();
      Vec3d dir2 = m_arrlistBeachVerts[group][i-1].pos - m_arrlistBeachVerts[group][i].pos;
      dir2.Normalize();

      Vec3d dir;

      if(dir1.Dot(dir2)>-0.99f)
        dir = dir1 + dir2;
      else
        dir = dir1.Cross(Vec3d(0,0,1));

      dir.Normalize();
      if(dir.Dot(m_arrlistBeachVerts[group][i  ].water_dir)<0)
        dir = -dir;
      m_arrlistBeachVerts[group][i  ].water_dir = dir;
    }
  }

  for( group=0; group<MAX_BEACH_GROUPS; group++)
  {
    for( int i=0; i<m_arrlistBeachVerts[group].Count()-1; i++)
    {
      BeachPairStruct * p1 = &m_arrlistBeachVerts[group][i  ];
      BeachPairStruct * p2 = &m_arrlistBeachVerts[group][i+1];

      int _x = int((p1->pos + p1->water_dir*2).x);
      int _y = int((p1->pos + p1->water_dir*2).y);

//      int nAreaID = m_pTerrain->m_pBeachGenerator->m_WaterAreaMap[_x/CTerrain::GetHeightMapUnitSize()][_y/CTerrain::GetHeightMapUnitSize()];
      float fBeachSize = (float)m_pTerrain->m_fShoreSize;//1.f+8.f*((float)m_lstWaterAreaSizeTable[nAreaID]/(float)m_nMaxAreaSize);

      // left
      p1->pos1  = p1->pos;
    
      int l=0;
      while(p1->pos1.z<m_pTerrain->GetWaterLevel()+0.1f*fBeachSize && l<70)
      {
        p1->pos1 = p1->pos1 - m_arrlistBeachVerts[group][i].water_dir*0.05f;
        p1->pos1.z = m_pTerrain->GetZApr(p1->pos1.x,p1->pos1.y);
        l++;
      }

      p1->pos2  = p1->pos + p1->water_dir*fBeachSize;

      p1->posm = p1->pos + Vec3d(0,0,0.2f*fBeachSize/4);

    
      // right
      p2->pos1 = p2->pos;

      l=0;
      while(p2->pos1.z<m_pTerrain->GetWaterLevel()+0.1f*fBeachSize && l<70)
      {
        p2->pos1 = p2->pos1 - p2->water_dir*0.05f;
        p2->pos1.z = m_pTerrain->GetZApr(p2->pos1.x,p2->pos1.y);
        l++;
      }
    
      p2->pos2 = p2->pos + p2->water_dir*fBeachSize;    

      p2->posm = p2->pos + Vec3d(0,0,0.2f*fBeachSize/4);
    }
  }

  m_lstUnsortedBeachVerts.Reset();

/////////////////////////////////////////////////////////////////////////////////////////////
// stage 3
/////////////////////////////////////////////////////////////////////////////////////////////

  if(!m_bBeachPresent)
  { 
    if(hFileToSave)
      GetSystem()->GetIPak()->FPrintf(hFileToSave, "BeachInfo(%8d)",GetSecIndex());
    // m_nVertNumBeach is zero = no beach here
    if(hFileToSave)
      GetSystem()->GetIPak()->FWrite(&m_nVertNumBeach,1,sizeof(m_nVertNumBeach),hFileToSave);
    return;
  }

  // count elements
  int nElements = 0;
  for( group=0; group<MAX_BEACH_GROUPS; group++)
  for( int i=0; i<m_arrlistBeachVerts[group].Count()-1; i++)
    nElements ++;

  if(!nElements)
  {
    m_bBeachPresent = false;
    { 
      if(hFileToSave)
        GetSystem()->GetIPak()->FPrintf(hFileToSave, "BeachInfo(%8d)",GetSecIndex());
      // m_nVertNumBeach is zero = no beach here
      if(hFileToSave)
        GetSystem()->GetIPak()->FWrite(&m_nVertNumBeach,1,sizeof(m_nVertNumBeach),hFileToSave);
      return;
    }
  }

  // make buffer
  m_nVertNumBeach = 6*nElements;
  m_pVertBufferBeach = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[m_nVertNumBeach];

  list2<ushort> lstBeachIndices;

  // tmp buff
  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F verts[6];

  // color
  Vec3d vTerrainColor = GetSystem()->GetI3DEngine()->GetWorldColor();//*GetSystem()->GetI3DEngine()->GetWorldBrightnes();

  for(int i=0; i<6; i++)
  {
    verts[i].color.bcolor[0]=(byte)min(255,vTerrainColor[0]*512.0f);
    verts[i].color.bcolor[1]=(byte)min(255,vTerrainColor[1]*512.0f);
    verts[i].color.bcolor[2]=(byte)min(255,vTerrainColor[2]*512.0f);
    verts[i].color.bcolor[3]=0;                       
  }

  verts[2].color.bcolor[3]=255;                     
  verts[3].color.bcolor[3]=255;                     
  
  // tex coord
  verts[0].st[0]=0; verts[0].st[1]=1.0f;
  verts[1].st[0]=1; verts[1].st[1]=1.0f;
  verts[2].st[0]=0; verts[2].st[1]=0.5f;
  verts[3].st[0]=1; verts[3].st[1]=0.5f;
  verts[4].st[0]=0; verts[4].st[1]=0.0f;
  verts[5].st[0]=1; verts[5].st[1]=0.0f;

  // make final buffer
  int nPos=0;
  int nIndex=0;
  for( group=0; group<MAX_BEACH_GROUPS; group++)
  for( int i=0; i<m_arrlistBeachVerts[group].Count()-1; i++)
  {
    BeachPairStruct * p1 = &m_arrlistBeachVerts[group][i  ];
    BeachPairStruct * p2 = &m_arrlistBeachVerts[group][i+1];

		// limit to avoid shore going up to the hill
		float fMaxShoreHeight = 0.25f;
		p1->pos1.z = min(p1->pos1.z, m_pTerrain->GetWaterLevel()+fMaxShoreHeight);
		p1->pos2.z = min(p1->pos2.z, m_pTerrain->GetWaterLevel()+fMaxShoreHeight);
		p1->posm.z = min(p1->posm.z, m_pTerrain->GetWaterLevel()+fMaxShoreHeight);
		p2->pos1.z = min(p2->pos1.z, m_pTerrain->GetWaterLevel()+fMaxShoreHeight);
		p2->pos2.z = min(p2->pos2.z, m_pTerrain->GetWaterLevel()+fMaxShoreHeight);
		p2->posm.z = min(p2->posm.z, m_pTerrain->GetWaterLevel()+fMaxShoreHeight);

		float fShoreHeightOffset = 0.05f;

		Vec3d vTerrainColor = GetSystem()->GetI3DEngine()->GetWorldColor();
		Vec3d vAmbColor = GetSystem()->GetI3DEngine()->GetOutdoorAmbientColor();
		vAmbColor.x *= (vTerrainColor.x*0.7f);
		vAmbColor.y *= (vTerrainColor.y*0.7f);
		vAmbColor.z *= (vTerrainColor.z*0.7f);

		float fBrR = vAmbColor.x + (1.f-vAmbColor.x)*m_pTerrain->IsOnTheLight((int)p1->posm.x,(int)p1->posm.y);
		float fBrG = vAmbColor.y + (1.f-vAmbColor.y)*m_pTerrain->IsOnTheLight((int)p1->posm.x,(int)p1->posm.y);
		float fBrB = vAmbColor.z + (1.f-vAmbColor.z)*m_pTerrain->IsOnTheLight((int)p1->posm.x,(int)p1->posm.y);

		uchar r1 = (byte)(min(255,vTerrainColor[0]*512.0f)*fBrR);
		uchar g1 = (byte)(min(255,vTerrainColor[1]*512.0f)*fBrG);
		uchar b1 = (byte)(min(255,vTerrainColor[2]*512.0f)*fBrB);

		fBrR = vAmbColor.x + (1.f-vAmbColor.x)*m_pTerrain->IsOnTheLight((int)p2->posm.x,(int)p2->posm.y);
		fBrG = vAmbColor.y + (1.f-vAmbColor.y)*m_pTerrain->IsOnTheLight((int)p2->posm.x,(int)p2->posm.y);
		fBrB = vAmbColor.z + (1.f-vAmbColor.z)*m_pTerrain->IsOnTheLight((int)p2->posm.x,(int)p2->posm.y);

		uchar r2 = (byte)(min(255,vTerrainColor[0]*512.0f)*fBrR);
		uchar g2 = (byte)(min(255,vTerrainColor[1]*512.0f)*fBrG);
		uchar b2 = (byte)(min(255,vTerrainColor[2]*512.0f)*fBrB);

    // pos
    verts[0].xyz.x = p1->pos1.x;
    verts[0].xyz.y = p1->pos1.y;
    verts[0].xyz.z = p1->pos1.z+fShoreHeightOffset;
		verts[0].color.bcolor[0] = r1;
		verts[0].color.bcolor[1] = g1;
		verts[0].color.bcolor[2] = b1;

    verts[1].xyz.x = p2->pos1.x;    
    verts[1].xyz.y = p2->pos1.y;    
    verts[1].xyz.z = p2->pos1.z+fShoreHeightOffset;    
		verts[1].color.bcolor[0] = r2;
		verts[1].color.bcolor[1] = g2;
		verts[1].color.bcolor[2] = b2;

    verts[2].xyz.x = p1->posm.x;
    verts[2].xyz.y = p1->posm.y;
    verts[2].xyz.z = p1->posm.z+fShoreHeightOffset;
		verts[2].color.bcolor[0] = r1;
		verts[2].color.bcolor[1] = g1;
		verts[2].color.bcolor[2] = b1;

    verts[3].xyz.x = p2->posm.x;
    verts[3].xyz.y = p2->posm.y;
    verts[3].xyz.z = p2->posm.z+fShoreHeightOffset;
		verts[3].color.bcolor[0] = r2;
		verts[3].color.bcolor[1] = g2;
		verts[3].color.bcolor[2] = b2;

    verts[4].xyz.x = p1->pos2.x;
    verts[4].xyz.y = p1->pos2.y;
    verts[4].xyz.z = p1->pos2.z+fShoreHeightOffset;
		verts[4].color.bcolor[0] = r1;
		verts[4].color.bcolor[1] = g1;
		verts[4].color.bcolor[2] = b1;

    verts[5].xyz.x = p2->pos2.x;
    verts[5].xyz.y = p2->pos2.y;
    verts[5].xyz.z = p2->pos2.z+fShoreHeightOffset;
		verts[5].color.bcolor[0] = r2;
		verts[5].color.bcolor[1] = g2;
		verts[5].color.bcolor[2] = b2;

    // draw
    memcpy(&m_pVertBufferBeach[nPos],verts,sizeof(verts));
    nPos+=6;

    lstBeachIndices.Add(nIndex+0);
    lstBeachIndices.Add(nIndex+1);
    lstBeachIndices.Add(nIndex+2);

    lstBeachIndices.Add(nIndex+3);
    lstBeachIndices.Add(nIndex+2);
    lstBeachIndices.Add(nIndex+1);

    lstBeachIndices.Add(nIndex+2);
    lstBeachIndices.Add(nIndex+3);
    lstBeachIndices.Add(nIndex+4);

    lstBeachIndices.Add(nIndex+5);
    lstBeachIndices.Add(nIndex+4);
    lstBeachIndices.Add(nIndex+3);

    nIndex+=6;
  }

  // save calculated data into file
  if(hFileToSave)
  {
    // header
    GetSystem()->GetIPak()->FPrintf(hFileToSave, "BeachInfo(%8d)",GetSecIndex());

    // verts
    GetSystem()->GetIPak()->FWrite(&m_nVertNumBeach,1,sizeof(m_nVertNumBeach),hFileToSave);
    GetSystem()->GetIPak()->FWrite(m_pVertBufferBeach,m_nVertNumBeach,sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F),hFileToSave);

    // indices
    int nIndCount = lstBeachIndices.Count();
    GetSystem()->GetIPak()->FWrite(&nIndCount,1,sizeof(nIndCount),hFileToSave);
    GetSystem()->GetIPak()->FWrite(&lstBeachIndices[0],nIndCount,sizeof(ushort),hFileToSave);
  }

  // make vertex buffer
  if(m_pLeafBufferBeach)
    GetRenderer()->DeleteLeafBuffer(m_pLeafBufferBeach);
  m_pLeafBufferBeach = GetRenderer()->CreateLeafBufferInitialized(
    m_pVertBufferBeach, m_nVertNumBeach, VERTEX_FORMAT_P3F_COL4UB_TEX2F,
    lstBeachIndices.GetElements(), lstBeachIndices.Count(), R_PRIMV_TRIANGLES, "WaterBeach");

  m_pLeafBufferBeach->SetChunk( m_pTerrain->m_pSHShore, 
                                0, m_nVertNumBeach, 
                                0, lstBeachIndices.Count());

  assert(m_pLeafBufferBeach);
  delete [] m_pVertBufferBeach; m_pVertBufferBeach=0;
}

// load precalculated data from file
void CSectorInfo::LoadBeach(FILE * hFileToLoad)
{ 
  if(!hFileToLoad)
    return;

  // header
  int nReadedIndex=-1;
  char szHeader[32]="";
  GetSystem()->GetIPak()->FRead(szHeader, 1, 19, hFileToLoad);
  szHeader[19]=0;
  sscanf(szHeader, "BeachInfo(%8d)", &nReadedIndex);

  if(nReadedIndex!=GetSecIndex())
  {
    GetConsole()->Exit("CSectorInfo::LoadBeach: File beach.tmp is corrupted, try to delete this file (or all *.tmp files) and run again");
    assert(0); // Exit should do exit 
  }

  m_bBeachPresent = true;

  struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * m_pVertBufferBeach=0;
  int m_nVertNumBeach=0;

  // verts
  GetSystem()->GetIPak()->FRead(&m_nVertNumBeach,1,sizeof(m_nVertNumBeach),hFileToLoad);
  if(!m_nVertNumBeach)
    return; // no beach here

  assert(m_nVertNumBeach<=2048);

  if(m_nVertNumBeach>2048)
    return; // no beach here

  m_pVertBufferBeach = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[m_nVertNumBeach];
  GetSystem()->GetIPak()->FRead(m_pVertBufferBeach,m_nVertNumBeach,sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F),hFileToLoad);

	// flip colors for DX
	if(!(GetRenderer()->GetFeatures() & RFT_RGBA))
	{
		struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pVert = m_pVertBufferBeach;
		for(int i=0; i<m_nVertNumBeach; i++)
		{
			Exchange(pVert->color.bcolor[0], pVert->color.bcolor[2]);
			pVert++;
		}
	}

  // indices
  int nIndCount = -1;
  GetSystem()->GetIPak()->FRead(&nIndCount,1,sizeof(nIndCount),hFileToLoad);
  list2<ushort> lstBeachIndices;
  lstBeachIndices.PreAllocate(nIndCount,nIndCount);
  GetSystem()->GetIPak()->FRead(&lstBeachIndices[0],nIndCount,sizeof(ushort),hFileToLoad);

  // make vertex buffer
  if(m_pLeafBufferBeach)
    GetRenderer()->DeleteLeafBuffer(m_pLeafBufferBeach);
  m_pLeafBufferBeach = GetRenderer()->CreateLeafBufferInitialized(
    m_pVertBufferBeach, m_nVertNumBeach, VERTEX_FORMAT_P3F_COL4UB_TEX2F,
    lstBeachIndices.GetElements(), lstBeachIndices.Count(), R_PRIMV_TRIANGLES, "WaterBeach");

  if(m_pTerrain->m_pSHShore)
    m_pLeafBufferBeach->SetChunk( 
                                m_pTerrain->m_pSHShore, 
                                0, m_nVertNumBeach, 
                                0, lstBeachIndices.Count());

  assert(m_pLeafBufferBeach);
  delete [] m_pVertBufferBeach; m_pVertBufferBeach=0;
}

void CSectorInfo::RenderBeach(IShader * pShader, float fZoomFactor, float fCamZ)
{
  if(m_pLeafBufferBeach && m_bBeachPresent)
    if(m_fDistance*fZoomFactor < 128 + (fCamZ - m_fMinZ)*0.5f)
  {
    CCObject * pObject = GetRenderer()->EF_GetObject(true);
 	  m_pLeafBufferBeach->SetShader(pShader);
    pObject->m_Matrix.SetIdentity();

	  m_pLeafBufferBeach->AddRE( pObject, 0, (fCamZ>m_pTerrain->GetWaterLevel()) ? eS_Banner : eS_WaterBeach );
    
    pObject->m_SortId = -1; // render water beaches after water
    // let distance sorter think that object is in camera position
    if(m_pLeafBufferBeach->m_pMats->Count() && m_pLeafBufferBeach->m_pMats->Get(0)->pRE)
      m_pLeafBufferBeach->m_vBoxMin = m_pLeafBufferBeach->m_vBoxMax = GetViewCamera().GetPos(); 
  }
}

const void * CSectorInfo::GetShoreGeometry(int & nPosStride, int & nVertCount)
{
	if(!m_pLeafBufferBeach || !m_pLeafBufferBeach->m_SecVertCount)
		return 0;

	nVertCount = m_pLeafBufferBeach->m_SecVertCount;

	return m_pLeafBufferBeach->GetPosPtr(nPosStride,0,true);
}