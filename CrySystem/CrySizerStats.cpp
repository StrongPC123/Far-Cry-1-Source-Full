#include "stdafx.h"
#include "ILog.h"
#include "ITimer.h"
#include "ISystem.h"
#include "IConsole.h"
#include "IRenderer.h"
#include "Font.h"
#include "CrySizerImpl.h"
#include "CrySizerStats.h"


CrySizerStatsBuilder::CrySizerStatsBuilder (CrySizerImpl* pSizer, int nMinSubcomponentBytes):
	m_pSizer (pSizer),
	m_nMinSubcomponentBytes (nMinSubcomponentBytes < 0 || nMinSubcomponentBytes > 0x10000000 ? 0 : nMinSubcomponentBytes)
{

}


// creates the map of names from old (in the sizer Impl) to new (in the Stats)
void CrySizerStatsBuilder::processNames()
{
	size_t numCompNames = m_pSizer->m_arrNames.size();
	m_pStats->m_arrComponents.reserve (numCompNames);
	m_pStats->m_arrComponents.clear();

  m_mapNames.resize (numCompNames, -1);

	// add all root objects
	addNameSubtree(0,0);
}


//////////////////////////////////////////////////////////////////////////
// given the name in the old system, adds the subtree of names to the 
// name map and components. In case all the subtree is empty, returns false and 
// adds nothing
size_t CrySizerStatsBuilder::addNameSubtree (unsigned nDepth, size_t nName)
{
	assert (nName < m_pSizer->m_arrNames.size());

	CrySizerImpl::ComponentName& rCompName = m_pSizer->m_arrNames[nName];
	size_t sizeObjectsTotal = rCompName.sizeObjectsTotal;

	if (sizeObjectsTotal <= m_nMinSubcomponentBytes)
		return sizeObjectsTotal; // the subtree didn't pass

	// the index of the component in the stats object (sorted by the depth-first traverse order)
	size_t nNewName = m_pStats->m_arrComponents.size();
	m_pStats->m_arrComponents.resize (nNewName+1);

	Component& rNewComp = m_pStats->m_arrComponents[nNewName];
	rNewComp.strName = rCompName.strName;
	rNewComp.nDepth = nDepth;
	rNewComp.numObjects = rCompName.numObjects;
	rNewComp.sizeBytes = rCompName.sizeObjects;
	rNewComp.sizeBytesTotal = sizeObjectsTotal;
	m_mapNames[nName] = nNewName;

	// find the immediate children and sort them by their total size
	typedef std::map<size_t,size_t> UintUintMap;
	UintUintMap mapSizeName; // total size -> child index (name in old indexation)

	for (size_t i = nName + 1; i < m_pSizer->m_arrNames.size(); ++i)
	{
		CrySizerImpl::ComponentName& rChild = m_pSizer->m_arrNames[i];
		if (rChild.nParent == nName && rChild.sizeObjectsTotal > m_nMinSubcomponentBytes)
			mapSizeName.insert (UintUintMap::value_type(rChild.sizeObjectsTotal,i));
	}

	// add the sorted components
	/*
	for (unsigned i = nName + 1; i < m_pSizer->m_arrNames.size(); ++i)
		if (m_pSizer->m_arrNames[i].nParent == nName)
			addNameSubtree(nDepth+1,i);
	*/

	for (UintUintMap::reverse_iterator it = mapSizeName.rbegin(); it != mapSizeName.rend(); ++it)
	{
		addNameSubtree(nDepth + 1, it->second);
	}

	return sizeObjectsTotal;
}


//////////////////////////////////////////////////////////////////////////
// creates the statistics out of the given CrySizerImpl into the given CrySizerStats
// Maps the old to new names according to the depth-walk tree rule
void CrySizerStatsBuilder::build (CrySizerStats* pStats)
{
	m_pStats = pStats;

	m_mapNames.clear();

	processNames();

	m_pSizer->clear();
	pStats->refresh();
	pStats->m_nAgeFrames = 0;
}


//////////////////////////////////////////////////////////////////////////
// constructs the statistics based on the given cry sizer
CrySizerStats::CrySizerStats (CrySizerImpl* pCrySizer)
{
	CrySizerStatsBuilder builder (pCrySizer);
	builder.build(this);
}

CrySizerStats::CrySizerStats ()
{
}


// if there is already such name in the map, then just returns the index
// of the compoentn in the component array; otherwise adds an entry to themap
// and to the component array nad returns its index
CrySizerStatsBuilder::Component& CrySizerStatsBuilder::mapName (unsigned nName)
{
	assert (m_mapNames[nName] != -1);
	return m_pStats->m_arrComponents[m_mapNames[nName]];
	/*
	IdToIdMap::iterator it = m_mapNames.find (nName);
	if (it == m_mapNames.end())
	{
		unsigned nNewName = m_arrComponents.size();
		m_mapNames.insert (IdToIdMap::value_type(nName, nNewName));
		m_arrComponents.resize(nNewName + 1);
		m_arrComponents[nNewName].strName.swap(m_pSizer->m_arrNames[nName]);
		return m_arrComponents.back();
	}
	else
	{
		assert (it->second < m_arrComponents.size());
		return m_arrComponents[it->second];
	}
	*/
}

// refreshes the statistics built after the component array is built
void CrySizerStats::refresh()
{
	m_nMaxNameLength = 0; 
	for (size_t i = 0; i < m_arrComponents.size(); ++i)
	{
		size_t nLength = m_arrComponents[i].strName.length()+m_arrComponents[i].nDepth;
		if (nLength > m_nMaxNameLength)
			m_nMaxNameLength = nLength;
	}
}


bool CrySizerStats::Component::GenericOrder::operator () (const Component& left, const Component& right)const
{
	return left.strName < right.strName;
}


CrySizerStatsRenderer::CrySizerStatsRenderer (ISystem* pSystem, CrySizerStats* pStats, unsigned nMaxSubcomponentDepth, int nMinSubcomponentBytes):
	m_pStats(pStats),
	m_pRenderer(pSystem->GetIRenderer()),
	m_pLog (pSystem->GetILog()),
	m_pFont (pSystem->GetIConsole()->GetFont()),
	m_nMinSubcomponentBytes (nMinSubcomponentBytes < 0 || nMinSubcomponentBytes > 0x10000000 ? 0x8000 : nMinSubcomponentBytes),
	m_nMaxSubcomponentDepth (nMaxSubcomponentDepth)
{

}

void CrySizerStatsRenderer::render(bool bRefreshMark)
{
	if (!m_pStats->size())
		return;

	int x,y,dx,dy;
	m_pRenderer->GetViewport(&x,&y,&dx,&dy);

	// left coordinate of the text
	unsigned nNameWidth = (unsigned)(m_pStats->getMaxNameLength()+1);
	if (nNameWidth < 25)
		nNameWidth = 25;
	float fCharScaleX = 0.375f, fCharScaleY = 0.5f;
	float fCharSizeX = m_pFont->m_charsize*fCharScaleX, fCharSizeY = m_pFont->m_charsize*fCharScaleY;
	float fLeft = 0;
	float fTop  = 48;
	float fVStep = 10;

#ifdef _XBOX
	fTop = 20;
#endif

	m_pRenderer->WriteXY (m_pFont, (int)fLeft, (int)(fTop), fCharScaleX, fCharScaleY, 0.9f,0.85f,1,0.85f,
		"%-*s   TOTAL   partial  count",nNameWidth,bRefreshMark?"Memory usage (refresh*)":"Memory usage (refresh )");
	m_pRenderer->WriteXY (m_pFont, (int)fLeft, (int)(fTop + fVStep*0.25f), fCharScaleX, fCharScaleY, 0.85f,0.9f,1,0.85f,
		"%*s   _____   _______  _____",nNameWidth,"");

	unsigned nSubgroupDepth = 1;

	// different colors used to paint the statistics subgroups
	// a new statistic subgroup starts with a new subtree of depth <= specified
	float fGray = 0;//0.45f;
	float fLightGray = 0.5f;//0.8f;
	float fColors[] =
	{
		fLightGray,fLightGray,fGray, 1,
		1,1,1,1,
		fGray,1,1,1,
		1,fGray,1,1,
		1,1,fGray,1,
		fGray,fLightGray,1,1,
		fGray,1,fGray,1,
		1,fGray,fGray,1
	};
	float*pColor = fColors;

	for (unsigned i = 0; i < m_pStats->size(); ++i)
	{
		const Component& rComp = (*m_pStats)[i];

		if (rComp.nDepth <= nSubgroupDepth)
		{
			//switch the color
			pColor += 4;
			if (pColor >= fColors + sizeof(fColors)/sizeof(fColors[0]))
				pColor = fColors;

			fTop += fVStep*(0.333333f + (nSubgroupDepth - rComp.nDepth) * 0.15f);
		}

		if (rComp.sizeBytesTotal <= m_nMinSubcomponentBytes || rComp.nDepth > m_nMaxSubcomponentDepth)
			continue;

		fTop += fVStep;

		//m_pRenderer->WriteXY(m_pFont, (int)fLeft+fCharSizeX*(nNameWidth-rComp.strName.length()), (int)fTop, fCharScale, fCharScale, 1,1,1,1,
		//	"%s:%7.3f",rComp.strName.c_str(), rComp.getSizeMBytes());

		char szDepth[32] = " ..............................";
		if (rComp.nDepth < sizeof(szDepth))
			szDepth[rComp.nDepth] = '\0';

		char szSize[32];
		if (rComp.sizeBytes > 0)
		{
			if (rComp.sizeBytesTotal > rComp.sizeBytes)
				sprintf (szSize, "%7.3f  %7.3f", rComp.getTotalSizeMBytes(), rComp.getSizeMBytes());
			else
				sprintf (szSize, "         %7.3f", rComp.getSizeMBytes());
		}
		else
		{
			assert (rComp.sizeBytesTotal > 0);
			sprintf (szSize, "%7.3f         ", rComp.getTotalSizeMBytes());
		}
		char szCount[16];
#ifdef _DEBUG
		if (rComp.numObjects)
			sprintf (szCount, "%8u", rComp.numObjects);
		else
#endif
			szCount[0] = '\0';

		m_pRenderer->WriteXY(m_pFont, (int)fLeft, (int)(fTop), fCharScaleX, fCharScaleY, pColor[0],pColor[1],pColor[2],pColor[3],
			"%s%-*s:%s%s",szDepth, nNameWidth-rComp.nDepth,rComp.strName.c_str(), szSize, szCount);
	}

	fTop += 0.25f*fVStep;
	m_pRenderer->WriteXY(m_pFont, (int)fLeft, (int)(fTop), fCharScaleX, fCharScaleY, fLightGray,fLightGray,fLightGray,1,
			"%-*s %s",nNameWidth,"___________________________", "________________");
	fTop += fVStep;

	const char* szOverheadNames[CrySizerStats::g_numTimers] = 
	{
		".Collection",
		".Transformation",
		".Cleanup"
	};
	bool bOverheadsHeaderPrinted = false;
	for (unsigned i = 0; i < CrySizerStats::g_numTimers; ++i)
	{
		float fTime = m_pStats->getTime(i);
		if (fTime < 20)
			continue;
		// print the header
		if (!bOverheadsHeaderPrinted)
		{
			m_pRenderer->WriteXY(m_pFont, (int)fLeft, (int)(fTop), fCharScaleX, fCharScaleY, fLightGray,fLightGray,fLightGray,1,
				"%-*s",nNameWidth,"Overheads");
			fTop += fVStep;
			bOverheadsHeaderPrinted = true;
		}

		m_pRenderer->WriteXY(m_pFont, (int)fLeft, (int)(fTop), fCharScaleX, fCharScaleY, fLightGray,fLightGray,fLightGray,1,
				"%-*s:%7.1f ms",nNameWidth,szOverheadNames[i], fTime);
		fTop += fVStep;
	}
}

void CrySizerStatsRenderer::dump()
{
	if (!m_pStats->size())
		return;

	unsigned nNameWidth = (unsigned)(m_pStats->getMaxNameLength()+1);

	// left coordinate of the text
	m_pLog->LogToFile ("Memory Statistics:");
	m_pLog->LogToFile("%-*s   TOTAL   partial  count",nNameWidth,"");

	unsigned nSubgroupDepth = 1;

	// different colors used to paint the statistics subgroups
	// a new statistic subgroup starts with a new subtree of depth <= specified

	for (unsigned i = 0; i < m_pStats->size(); ++i)
	{
		const Component& rComp = (*m_pStats)[i];

		if (rComp.sizeBytesTotal <= m_nMinSubcomponentBytes || rComp.nDepth > m_nMaxSubcomponentDepth)
			continue;

		char szDepth[32] = " ..............................";
		if (rComp.nDepth < sizeof(szDepth))
			szDepth[rComp.nDepth] = '\0';

		char szSize[32];
		if (rComp.sizeBytes > 0)
		{
			if (rComp.sizeBytesTotal > rComp.sizeBytes)
				sprintf (szSize, "%7.3f  %7.3f", rComp.getTotalSizeMBytes(), rComp.getSizeMBytes());
			else
				sprintf (szSize, "         %7.3f", rComp.getSizeMBytes());
		}
		else
		{
			assert (rComp.sizeBytesTotal > 0);
			sprintf (szSize, "%7.3f         ", rComp.getTotalSizeMBytes());
		}
		char szCount[16];

		if (rComp.numObjects)
			sprintf (szCount, "%8u", rComp.numObjects);
		else
			szCount[0] = '\0';

		m_pLog->LogToFile ("%s%-*s:%s%s",szDepth, nNameWidth-rComp.nDepth,rComp.strName.c_str(), szSize, szCount);
	}
}


void CrySizerStats::startTimer(unsigned nTimer, ITimer* pTimer)
{
	assert (nTimer < g_numTimers);
	m_fTime[nTimer] = pTimer->GetAsyncCurTime();
}

void CrySizerStats::stopTimer(unsigned nTimer, ITimer* pTimer)
{
	assert (nTimer < g_numTimers);
	m_fTime[nTimer] = 1000*(pTimer->GetAsyncCurTime() - m_fTime[nTimer]);
}
