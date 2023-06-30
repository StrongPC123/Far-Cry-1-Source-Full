////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   MusicThemeLibItem.cpp
//  Version:     v1.00
//  Created:     3/2/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MusicThemeLibItem.h"
#include "MusicManager.h"
#include "BaseLibrary.h"
#include "ErrorReport.h"

#include <ISound.h>

//////////////////////////////////////////////////////////////////////////
CMusicThemeLibItem::CMusicThemeLibItem()
{
	m_pTheme = new SMusicTheme;
}

//////////////////////////////////////////////////////////////////////////
CMusicThemeLibItem::~CMusicThemeLibItem()
{
}

//////////////////////////////////////////////////////////////////////////
void CMusicThemeLibItem::SerializePattern( SerializeContext &ctx,SPatternDef *pPattern )
{
	XmlNodeRef node = ctx.node;
	if (ctx.bLoading)
	{
		// Loading.
		if (!ctx.bCopyPaste)
		{
			pPattern->sName = node->getAttr( "Name" );
		}
		node->getAttr( "Probability",pPattern->fProbability );
		node->getAttr( "LayeringVolume",pPattern->nLayeringVolume );
		pPattern->sFilename = node->getAttr( "Filename" );

		CString sFadePoints;
		if (node->getAttr( "FadePoints",sFadePoints ))
		{
			int iStart = 0;
			pPattern->vecFadePoints.clear();
			CString token = TokenizeString( sFadePoints,",",iStart );
			while (!token.IsEmpty())
			{
				pPattern->vecFadePoints.push_back( atoi(token) );
				token = TokenizeString( sFadePoints,",",iStart );
			}
		}

		if (!ctx.bCopyPaste)
		{
			// Add this pattern to music data.
			CMusicManager *pManager = GetIEditor()->GetMusicManager();
			SMusicData *pMusicData = pManager->GetMusicData();
			if (pMusicData)
			{
				pMusicData->vecPatternDef.push_back(pPattern);
			}
		}
	}
	else
	{
		// Saving.
		node->setAttr( "Name",pPattern->sName.c_str() );
		node->setAttr( "Probability",pPattern->fProbability );
		node->setAttr( "LayeringVolume",pPattern->nLayeringVolume );
		node->setAttr( "Filename",pPattern->sFilename.c_str() );

		if (!pPattern->vecFadePoints.empty())
		{
			CString str;
			for (int i = 0; i < pPattern->vecFadePoints.size(); i++)
			{
				CString temp;
				if (i > 0)
					temp.Format( ",%d",(int)pPattern->vecFadePoints[i] );
				else
					temp.Format( "%d",(int)pPattern->vecFadePoints[i] );
				str += temp;
			}
			node->setAttr( "FadePoints",str );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicThemeLibItem::SerializePatternSet( SerializeContext &ctx,SMusicPatternSet *pPatternSet )
{
	XmlNodeRef nodePtrnSet = ctx.node;
	if (ctx.bLoading)
	{
		// Loading.

		nodePtrnSet->getAttr( "MaxTimeout",pPatternSet->fMaxTimeout );
		nodePtrnSet->getAttr( "MinTimeout",pPatternSet->fMinTimeout );
		nodePtrnSet->getAttr( "IncidentalLayerProbability",pPatternSet->fIncidentalLayerProbability );
		nodePtrnSet->getAttr( "RhythmicLayerProbability",pPatternSet->fRhythmicLayerProbability );
		nodePtrnSet->getAttr( "MaxSimultaneousIncidentalPatterns",pPatternSet->nMaxSimultaneousIncidentalPatterns );
		nodePtrnSet->getAttr( "MaxSimultaneousRhythmicPatterns",pPatternSet->nMaxSimultaneousRhythmicPatterns );

		// load patterns.
		XmlNodeRef nodeMainLayer = nodePtrnSet->findChild( "MainLayer" );
		XmlNodeRef nodeRhythmicLayer = nodePtrnSet->findChild( "RhythmicLayer" );
		XmlNodeRef nodeIncidentalLayer = nodePtrnSet->findChild( "IncidentalLayer" );

		if (nodeMainLayer)
		{
			SerializeContext patternCtx(ctx);
			for (int j = 0; j < nodeMainLayer->getChildCount(); j++)
			{
				patternCtx.node = nodeMainLayer->getChild(j);
				SPatternDef *pPattern = new SPatternDef;
				pPatternSet->vecMainPatterns.push_back(pPattern);
				SerializePattern( patternCtx,pPattern );
			}
		}
		if (nodeRhythmicLayer)
		{
			SerializeContext patternCtx(ctx);
			for (int j = 0; j < nodeRhythmicLayer->getChildCount(); j++)
			{
				patternCtx.node = nodeRhythmicLayer->getChild(j);
				SPatternDef *pPattern = new SPatternDef;
				pPatternSet->vecRhythmicPatterns.push_back(pPattern);
				SerializePattern( patternCtx,pPattern );
			}
		}
		if (nodeIncidentalLayer)
		{
			SerializeContext patternCtx(ctx);
			for (int j = 0; j < nodeIncidentalLayer->getChildCount(); j++)
			{
				patternCtx.node = nodeIncidentalLayer->getChild(j);
				SPatternDef *pPattern = new SPatternDef;
				pPatternSet->vecIncidentalPatterns.push_back(pPattern);
				SerializePattern( patternCtx,pPattern );
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Calc pattern set probability.
		pPatternSet->fTotalMainPatternProbability = 0.0f;
		pPatternSet->fTotalRhythmicPatternProbability = 0.0f;
		pPatternSet->fTotalIncidentalPatternProbability = 0.0f;
		for (int j = 0; j < pPatternSet->vecMainPatterns.size(); j++)
		{
			pPatternSet->fTotalMainPatternProbability += pPatternSet->vecMainPatterns[j]->fProbability;
		}
		for (int j = 0; j < pPatternSet->vecRhythmicPatterns.size(); j++)
		{
			pPatternSet->fTotalRhythmicPatternProbability += pPatternSet->vecRhythmicPatterns[j]->fProbability;
		}
		for (int j = 0; j < pPatternSet->vecIncidentalPatterns.size(); j++)
		{
			pPatternSet->fTotalIncidentalPatternProbability += pPatternSet->vecIncidentalPatterns[j]->fProbability;
		}
	}
	else
	{
		// Saving.
		nodePtrnSet->setAttr( "MaxTimeout",pPatternSet->fMaxTimeout );
		nodePtrnSet->setAttr( "MinTimeout",pPatternSet->fMinTimeout );
		nodePtrnSet->setAttr( "IncidentalLayerProbability",pPatternSet->fIncidentalLayerProbability );
		nodePtrnSet->setAttr( "RhythmicLayerProbability",pPatternSet->fRhythmicLayerProbability );
		nodePtrnSet->setAttr( "MaxSimultaneousIncidentalPatterns",pPatternSet->nMaxSimultaneousIncidentalPatterns );
		nodePtrnSet->setAttr( "MaxSimultaneousRhythmicPatterns",pPatternSet->nMaxSimultaneousRhythmicPatterns );

		// Save patterns.
		XmlNodeRef nodeMainLayer = nodePtrnSet->newChild( "MainLayer" );
		XmlNodeRef nodeRhythmicLayer = nodePtrnSet->newChild( "RhythmicLayer" );
		XmlNodeRef nodeIncidentalLayer = nodePtrnSet->newChild( "IncidentalLayer" );

		{
			SerializeContext patternCtx(ctx);
			for (int j = 0; j < pPatternSet->vecMainPatterns.size(); j++)
			{
				patternCtx.node = nodeMainLayer->newChild("Pattern");
				SerializePattern( patternCtx,pPatternSet->vecMainPatterns[j] );
			}
		}
		{
			SerializeContext patternCtx(ctx);
			for (int j = 0; j < pPatternSet->vecRhythmicPatterns.size(); j++)
			{
				patternCtx.node = nodeRhythmicLayer->newChild("Pattern");
				SerializePattern( patternCtx,pPatternSet->vecRhythmicPatterns[j] );
			}
		}
		{
			SerializeContext patternCtx(ctx);
			for (int j = 0; j < pPatternSet->vecIncidentalPatterns.size(); j++)
			{
				patternCtx.node = nodeIncidentalLayer->newChild("Pattern");
				SerializePattern( patternCtx,pPatternSet->vecIncidentalPatterns[j] );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicThemeLibItem::SerializeMood( SerializeContext &ctx,SMusicMood *pMood )
{
	XmlNodeRef node = ctx.node;
	if (ctx.bLoading)
	{
		// Loading.
		pMood->sName = node->getAttr( "Name" );
		node->getAttr( "PlaySingle",pMood->bPlaySingle );
		node->getAttr( "Priority",pMood->nPriority );
		node->getAttr( "FadeOutTime",pMood->fFadeOutTime );
		if (node->getChildCount() > 0)
		{
			SerializeContext patternSetCtx(ctx);
			// load pattern sets.
			for (int i = 0; i < node->getChildCount(); i++)
			{
				patternSetCtx.node = node->getChild(i);

				SMusicPatternSet *pPatternSet = new SMusicPatternSet;
				pMood->vecPatternSets.push_back(pPatternSet);

				SerializePatternSet( patternSetCtx,pPatternSet );
			}
		}
	}
	else
	{
		// Saving.
		node->setAttr( "Name",pMood->sName.c_str() );
		node->setAttr( "PlaySingle",pMood->bPlaySingle );
		node->setAttr( "Priority",pMood->nPriority );
		node->setAttr( "FadeOutTime",pMood->fFadeOutTime );
		if (!pMood->vecPatternSets.empty())
		{
			// Save pattern sets.
			SerializeContext patternSetCtx(ctx);
			for (int i = 0; i < pMood->vecPatternSets.size(); i++)
			{
				SMusicPatternSet *pPatternSet = pMood->vecPatternSets[i];
				patternSetCtx.node = node->newChild("PatternSet");

				SerializePatternSet( patternSetCtx,pPatternSet );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicThemeLibItem::Serialize( SerializeContext &ctx )
{
	assert(m_pTheme);

	CBaseLibraryItem::Serialize( ctx );
	XmlNodeRef node = ctx.node;
	if (ctx.bLoading)
	{
		// Loading.
		m_pTheme->sName = (const char*)GetName();
		m_pTheme->sDefaultMood = node->getAttr( "DefaultMood" );
		node->getAttr( "DefaultMoodTimeout",m_pTheme->fDefaultMoodTimeout );

		XmlNodeRef nodeMoods = node->findChild("Moods");
		if (nodeMoods)
		{
			SerializeContext moodCtx(ctx);
			for (int i = 0; i < nodeMoods->getChildCount(); i++)
			{
				moodCtx.node = nodeMoods->getChild(i);
				SMusicMood *pMood = new SMusicMood;
				SerializeMood( moodCtx,pMood );
				m_pTheme->mapMoods[pMood->sName.c_str()] = pMood;
			}
		}
		
		XmlNodeRef nodeBridges = node->findChild( "Bridges" );
		if (nodeBridges)
		{
			for (int i = 0; i < nodeBridges->getChildCount(); i++)
			{
				XmlNodeRef nodeBridge = nodeBridges->getChild(i);
				CString sPattern;
				if (nodeBridge->getAttr( "Pattern",sPattern))
				{
					m_pTheme->mapBridges[nodeBridges->getTag()] = (const char*)sPattern;
				}
			}
		}

		// Add this pattern to music data.
		CMusicManager *pManager =(CMusicManager*)GetLibrary()->GetManager();
		SMusicData *pMusicData = pManager->GetMusicData();
		if (pMusicData)
		{
			pMusicData->mapThemes[m_pTheme->sName.c_str()] = m_pTheme;
		}
	}
	else
	{
		// Saving.
		node->setAttr( "DefaultMood",m_pTheme->sDefaultMood.c_str() );
		node->setAttr( "DefaultMoodTimeout",m_pTheme->fDefaultMoodTimeout );

		if (!m_pTheme->mapMoods.empty())
		{
			XmlNodeRef nodeMoods = node->newChild("Moods");
			SerializeContext moodCtx(ctx);
			for (TMoodMap::iterator it = m_pTheme->mapMoods.begin(); it != m_pTheme->mapMoods.end(); ++it)
			{
				moodCtx.node = nodeMoods->newChild("Mood");
				SerializeMood( moodCtx,it->second );
			}
		}
		if (!m_pTheme->mapBridges.empty())
		{
			XmlNodeRef nodeBridges = node->newChild( "Bridges" );
			for (TThemeBridgeMap::iterator it = m_pTheme->mapBridges.begin(); it != m_pTheme->mapBridges.end(); ++it)
			{
				const char *sTheme = it->first.c_str();
				const char *sPattern = it->second.c_str();
				XmlNodeRef nodeBridge = nodeBridges->newChild(sTheme);
				nodeBridge->setAttr( "Pattern",sPattern );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicThemeLibItem::GatherUsedResources( CUsedResources &resources )
{
	// Add all music files.
	if (!m_pTheme)
		return;

	for (TMoodMap::iterator mit = m_pTheme->mapMoods.begin(); mit != m_pTheme->mapMoods.end(); ++mit)
	{
		SMusicMood *pMood = mit->second;
		for (int p = 0; p < pMood->vecPatternSets.size(); p++)
		{
			SMusicPatternSet *pPatternSet = pMood->vecPatternSets[p];
			{
				for (int j = 0; j < pPatternSet->vecMainPatterns.size(); j++)
				{
					AddMusicResourceFile( pPatternSet->vecMainPatterns[j]->sFilename.c_str(),resources );
				}
			}
			{
				for (int j = 0; j < pPatternSet->vecRhythmicPatterns.size(); j++)
				{
					AddMusicResourceFile( pPatternSet->vecRhythmicPatterns[j]->sFilename.c_str(),resources );
				}
			}
			{
				for (int j = 0; j < pPatternSet->vecIncidentalPatterns.size(); j++)
				{
					AddMusicResourceFile( pPatternSet->vecIncidentalPatterns[j]->sFilename.c_str(),resources );
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicThemeLibItem::AddMusicResourceFile( const char *szFilename,CUsedResources &resources )
{
	// Try adding both .ogg and .wav versions.
	CString ext = Path::GetExt(szFilename);
	if (stricmp(ext,"wav") || stricmp(ext,"ogg"))
	{
		resources.Add( Path::ReplaceExtension(szFilename,"wav") );
		resources.Add( Path::ReplaceExtension(szFilename,"ogg") );
	}
	else
	{
		resources.Add( szFilename );
	}
}