
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RandomExprLoadSink.h"

//////////////////////////////////////////////////////////////////////////
//! Reports a Game Warning to validator with WARNING severity.
inline void GameWarning( const char *format,... )
{
	if (!format)
		return;

	char buffer[MAX_WARNING_LENGTH];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);
	CryWarning( VALIDATOR_MODULE_GAME,VALIDATOR_WARNING,buffer );
}

CRandomExprLoadSink::CRandomExprLoadSink(bool bRaiseError, IScriptSystem *pScriptSystem, _SmartScriptObject *pObj, IAnimationSet *pAnimSet, TExprPatternVec *pvecExprPatterns, int nMode)
{
	m_bRaiseError=bRaiseError;
	m_pScriptSystem=pScriptSystem;
	m_pObj=pObj;
	m_pAnimSet=pAnimSet;
	m_pvecExprPatterns=pvecExprPatterns;
	m_nMode=nMode;
}

CRandomExprLoadSink::~CRandomExprLoadSink()
{
}

void CRandomExprLoadSink::OnElementFound(const char *sName, ScriptVarType type)
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
				case EXPRLOAD_MODE_BASE:
				{
					SExprPattern ExprPattern;
					ExprPattern.sName=sName;
					string sAnimName=string("#")+ExprPattern.sName;
					ExprPattern.nMorphTargetId=m_pAnimSet->FindMorphTarget(sAnimName.c_str());
					if (ExprPattern.nMorphTargetId==-1)
					{
						if (m_bRaiseError)
							GameWarning("Morph-Target '%s' (random expression) not found. Lip-syncing will only partially work !", sAnimName.c_str());
						break;
					}
					if (!pObj->GetValue("Offset", ExprPattern.fOffset))
						ExprPattern.fOffset=0.0f;
					if (!pObj->GetValue("Interval", ExprPattern.fInterval))
						ExprPattern.fInterval=5.0f;
					if (!pObj->GetValue("IntervalRandom", ExprPattern.fIntervalRandom))
						ExprPattern.fIntervalRandom=0.0f;
					if (!pObj->GetValue("Amp", ExprPattern.fAmp))
						ExprPattern.fAmp=1.0f;
					if (!pObj->GetValue("AmpRandom", ExprPattern.fAmpRandom))
						ExprPattern.fAmpRandom=0.0f;
					if (!pObj->GetValue("BlendIn", ExprPattern.fBlendIn))
						ExprPattern.fBlendIn=0.5f;
					if (!pObj->GetValue("Hold", ExprPattern.fHold))
						ExprPattern.fHold=0.0f;
					if (!pObj->GetValue("BlendOut", ExprPattern.fBlendOut))
						ExprPattern.fBlendOut=0.5f;
					m_pvecExprPatterns->push_back(ExprPattern);
					break;
				}
			}
			break;
		}
	}
}