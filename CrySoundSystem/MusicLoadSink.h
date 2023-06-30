#include "IScriptSystem.h"
#include <StlUtils.h>

#define MUSICLOAD_MODE_BASE					0
#define MUSICLOAD_MODE_PATTERNS			1
#define MUSICLOAD_MODE_THEMES				2
#define MUSICLOAD_MODE_MOODS				3
#define MUSICLOAD_MODE_MOODLAYERS		4
#define MUSICLOAD_MODE_BRIDGES			5
#define MUSICLOAD_MODE_FADEPOS			6
#define MUSICLOAD_MODE_DEFAULTMOOD	7
#define MUSICLOAD_MODE_PATTERNSET		8

class CMusicLoadSink : public IScriptObjectDumpSink
{
private:
	SMusicData *m_pMusicData;
	IScriptSystem *m_pScriptSystem;
	_SmartScriptObject *m_pObj;
	int m_nMode;
	SMusicTheme *m_pTheme;
	SMusicMood *m_pMood;
	SMusicPatternSet *m_pPatternSet;
	SPatternDef *m_pPattern;
	int m_nLayer;

	typedef std::map<string,SPatternDef*,stl::less_stricmp<string> > TPatternsMap;
	TPatternsMap m_patternsMap;
public:
	CMusicLoadSink(SMusicData *pMusicData, IScriptSystem *pScriptSystem, _SmartScriptObject *pObj, int nMode=MUSICLOAD_MODE_BASE, SPatternDef *pPattern=NULL, SMusicTheme *pTheme=NULL, SMusicMood *pMood=NULL, SMusicPatternSet *pPatternSet=NULL, int nLayer=MUSICLAYER_MAIN)
	{
		m_pMusicData=pMusicData;
		m_pScriptSystem=pScriptSystem;
		m_pObj=pObj;
		m_nMode=nMode;
		m_pTheme=pTheme;
		m_pMood=pMood;
		m_pPatternSet=pPatternSet;
		m_pPattern=pPattern;
		m_nLayer=nLayer;

		if (nMode == MUSICLOAD_MODE_MOODLAYERS)
		{
			// Make map of patterns.
			for (int i = 0; i < (int)m_pMusicData->vecPatternDef.size(); i++)
			{
				SPatternDef *ptrn = m_pMusicData->vecPatternDef[i];
				m_patternsMap[ptrn->sName.c_str()] = ptrn;
			}
		}
	}
private:
	void OnElementFound(const char *sName, ScriptVarType type)
	{
		switch (type)
		{
		case svtObject:
			{
				_SmartScriptObject pObj(m_pScriptSystem, true);
				if (!(*m_pObj)->GetValue(sName, pObj))
					break;
				switch (m_nMode)
				{
				case MUSICLOAD_MODE_BASE:
					{
						if (strcmp("Patterns", sName)==0)
						{
							CMusicLoadSink Sink(m_pMusicData, m_pScriptSystem, &pObj, MUSICLOAD_MODE_PATTERNS);
							pObj->Dump(&Sink);
						}else
							if (strcmp("Themes", sName)==0)
							{
								CMusicLoadSink Sink(m_pMusicData, m_pScriptSystem, &pObj, MUSICLOAD_MODE_THEMES);
								pObj->Dump(&Sink);
							}
							break;
					}
				case MUSICLOAD_MODE_PATTERNS:
					{
						SPatternDef *pPatternDef=new SPatternDef();
						const char *pszFilename;
						if (!pObj->GetValue("File", pszFilename))
							break;
						_SmartScriptObject pSubObj(m_pScriptSystem, true);
						if (!pObj->GetValue("FadePos", pSubObj))
							break;
						if (!pObj->GetValue("LayeringVolume", pPatternDef->nLayeringVolume))
							pPatternDef->nLayeringVolume=255;
						pPatternDef->sName=sName;
						pPatternDef->sFilename=pszFilename;
						CMusicLoadSink FadePosSink(m_pMusicData, m_pScriptSystem, &pSubObj, MUSICLOAD_MODE_FADEPOS, pPatternDef);
						pSubObj->Dump(&FadePosSink);
						m_pMusicData->vecPatternDef.push_back(pPatternDef);
						break;
					}
				case MUSICLOAD_MODE_THEMES:
					{
						_SmartScriptObject pSubObj(m_pScriptSystem, true);
						SMusicTheme *pTheme=new SMusicTheme();
						pTheme->sName=sName;
						if (pObj->GetValue("DefaultMood", pSubObj))
						{
							const char *pszDefaultMood;
							if (pSubObj->GetValue("Mood", pszDefaultMood))
								pTheme->sDefaultMood=pszDefaultMood;
							else
								pTheme->sDefaultMood="";
							pSubObj->GetValue("Timeout", pTheme->fDefaultMoodTimeout);
						}
						if (pObj->GetValue("Moods", pSubObj))
						{
							CMusicLoadSink MoodsSink(m_pMusicData, m_pScriptSystem, &pSubObj, MUSICLOAD_MODE_MOODS, NULL, pTheme);
							pSubObj->Dump(&MoodsSink);
						}
						if (pObj->GetValue("Bridges", pSubObj))
						{
							CMusicLoadSink BridgesSink(m_pMusicData, m_pScriptSystem, &pSubObj, MUSICLOAD_MODE_BRIDGES, NULL, pTheme);
							pSubObj->Dump(&BridgesSink);
						}
						m_pMusicData->mapThemes.insert(TThemeMapIt::value_type(sName, pTheme));
						break;
					}
				case MUSICLOAD_MODE_MOODS:
					{
						if (!m_pTheme)
							break;
						SMusicMood *pMood=new SMusicMood();
						pMood->sName=sName;
						if (!pObj->GetValue("Priority", pMood->nPriority))
							pMood->nPriority=0;	// default priority
						if (!pObj->GetValue("FadeOutTime", pMood->fFadeOutTime))
							pMood->fFadeOutTime=DEFAULT_CROSSFADE_TIME;
						if (!pObj->GetValue("PlaySingle", pMood->bPlaySingle))
							pMood->bPlaySingle=false;
						_SmartScriptObject pSubObj(m_pScriptSystem, true);
						if (pObj->GetValue("PatternSet", pSubObj))
						{
							CMusicLoadSink Sink(m_pMusicData, m_pScriptSystem, &pSubObj, MUSICLOAD_MODE_PATTERNSET, NULL, m_pTheme, pMood);
							pSubObj->Dump(&Sink);
						}
						m_pTheme->mapMoods.insert(TMoodMapIt::value_type(sName, pMood));
						break;
					}
				}
				break;
			}
		case svtString:
		case svtNumber:
			{
				switch (m_nMode)
				{
				case MUSICLOAD_MODE_BRIDGES:
					{
						if (!m_pTheme)
							break;
						const char *pszPattern;
						if (!(*m_pObj)->GetValue(sName, pszPattern))
							break;
						m_pTheme->mapBridges.insert(TThemeBridgeMapIt::value_type(sName, pszPattern));
						break;
					}
				}
				break;
			}
		}
	}
	void OnElementFound(int nIdx, ScriptVarType type)
	{
		switch (type)
		{
		case svtObject:
			{
				switch (m_nMode)
				{
				case MUSICLOAD_MODE_PATTERNSET:
					{
						if (!m_pMood)
							break;
						SMusicPatternSet *pPatternSet=new SMusicPatternSet();
						_SmartScriptObject pObj(m_pScriptSystem, true);
						if (!(*m_pObj)->GetAt(nIdx, pObj))
							break;
						if (!pObj->GetValue("MinTimeout", pPatternSet->fMinTimeout))
							pPatternSet->fMinTimeout=60.0f;
						if (!pObj->GetValue("MaxTimeout", pPatternSet->fMaxTimeout))
							pPatternSet->fMaxTimeout=120.0f;
						_SmartScriptObject pSubObj(m_pScriptSystem, true);
						pPatternSet->fTotalMainPatternProbability=0.0f;
						if (pObj->GetValue("MainLayer", pSubObj))
						{
							_SmartScriptObject pPatternsSubObj(m_pScriptSystem, true);
							if (pSubObj->GetValue("Patterns", pPatternsSubObj))
							{
								CMusicLoadSink Sink(m_pMusicData, m_pScriptSystem, &pPatternsSubObj, MUSICLOAD_MODE_MOODLAYERS, NULL, m_pTheme, m_pMood, pPatternSet, MUSICLAYER_MAIN);
								pPatternsSubObj->Dump(&Sink);
							}
						}
						pPatternSet->fTotalRhythmicPatternProbability=0.0f;
						if (pObj->GetValue("RhythmicLayer", pSubObj))
						{
							if (!pSubObj->GetValue("Probability", pPatternSet->fRhythmicLayerProbability))
								pPatternSet->fRhythmicLayerProbability=100.0f;
							if (!pSubObj->GetValue("MaxSimultaneousPatterns", pPatternSet->nMaxSimultaneousRhythmicPatterns))
								pPatternSet->nMaxSimultaneousRhythmicPatterns=1;
							_SmartScriptObject pPatternsSubObj(m_pScriptSystem, true);
							if (pSubObj->GetValue("Patterns", pPatternsSubObj))
							{
								CMusicLoadSink Sink(m_pMusicData, m_pScriptSystem, &pPatternsSubObj, MUSICLOAD_MODE_MOODLAYERS, NULL, m_pTheme, m_pMood, pPatternSet, MUSICLAYER_RHYTHMIC);
								pPatternsSubObj->Dump(&Sink);
							}
						}
						pPatternSet->fTotalIncidentalPatternProbability=0.0f;
						if (pObj->GetValue("IncidentalLayer", pSubObj))
						{
							if (!pSubObj->GetValue("Probability", pPatternSet->fIncidentalLayerProbability))
								pPatternSet->fIncidentalLayerProbability=100.0f;
							if (!pSubObj->GetValue("MaxSimultaneousPatterns", pPatternSet->nMaxSimultaneousIncidentalPatterns))
								pPatternSet->nMaxSimultaneousIncidentalPatterns=1;
							_SmartScriptObject pPatternsSubObj(m_pScriptSystem, true);
							if (pSubObj->GetValue("Patterns", pPatternsSubObj))
							{
								CMusicLoadSink Sink(m_pMusicData, m_pScriptSystem, &pPatternsSubObj, MUSICLOAD_MODE_MOODLAYERS, NULL, m_pTheme, m_pMood, pPatternSet, MUSICLAYER_INCIDENTAL);
								pPatternsSubObj->Dump(&Sink);
							}
						}
						m_pMood->vecPatternSets.push_back(pPatternSet);
						break;
					}
				case MUSICLOAD_MODE_MOODLAYERS:
					{
						if (!m_pPatternSet)
							break;
						_SmartScriptObject pSubObj(m_pScriptSystem, true);
						if (!(*m_pObj)->GetAt(nIdx, pSubObj))
							break;
						// read attributes
						string sPatternName;
						float fProbability = 0;
						const char *pszPatternName;
						if (!pSubObj->GetValue("Pattern", pszPatternName))
							break;
						sPatternName = pszPatternName;
						pSubObj->GetValue("Probability", fProbability);

						SPatternDef *pPattern = stl::find_in_map( m_patternsMap,sPatternName, (SPatternDef *)NULL );
						if (pPattern)
						{
							pPattern->fProbability = fProbability;

							switch (m_nLayer)
							{
							case MUSICLAYER_MAIN:
								m_pPatternSet->vecMainPatterns.push_back(pPattern);
								m_pPatternSet->fTotalMainPatternProbability+=fProbability;
								break;
							case MUSICLAYER_RHYTHMIC:
								m_pPatternSet->vecRhythmicPatterns.push_back(pPattern);
								m_pPatternSet->fTotalRhythmicPatternProbability+=fProbability;
								break;
							case MUSICLAYER_INCIDENTAL:
								m_pPatternSet->vecIncidentalPatterns.push_back(pPattern);
								m_pPatternSet->fTotalIncidentalPatternProbability+=fProbability;
								break;
							}
						}
						break;
					}
				}
				break;
			}
		case svtString:
		case svtNumber:
			{
				switch (m_nMode)
				{
				case MUSICLOAD_MODE_FADEPOS:
					{
						if (!m_pPattern)
							break;
						int nFadePos;
						if (!(*m_pObj)->GetAt(nIdx, nFadePos))
							break;
						m_pPattern->vecFadePoints.push_back(nFadePos);
						break;
					}
				}
				break;
			}
		}
	}
};
