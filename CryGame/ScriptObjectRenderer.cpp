
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	File: ScriptObjectRender.cpp
//
//  Description: 
//		ScriptObjectRender.cpp: implementation of the CScriptObjectRender class.
//
//	History: 
//		- created by Marco C.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "scriptobjectrenderer.h"
#include "IRenderer.h"

#define REG_FUNC(_class,_func) _class::RegisterFunction(pSS,#_func,&_class::_func);
_DECLARE_SCRIPTABLEEX(CScriptObjectRenderer)

CScriptObjectRenderer::CScriptObjectRenderer(void)
{
}

CScriptObjectRenderer::~CScriptObjectRenderer(void)
{
}

IScriptObject *CScriptObjectRenderer::Create(IScriptSystem *pSS,IRenderer *pRen)
{
	Init(pSS,this);
	//m_pScriptThis->RegisterParent(this);
	IScriptObject *pHolder=pSS->CreateObject();
	//pHolder->SetValue("0",pSS->CreateUserData((int)this,USER_DATA_SCRIPTOBJRENDERER));
	//pHolder->Delegate(m_pScriptThis);
	
	m_pRenderer=pRen;
	return m_pScriptThis;
}

void CScriptObjectRenderer::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectRenderer>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectRenderer,Reset);
	REG_FUNC(CScriptObjectRenderer,PushQuad);
	REG_FUNC(CScriptObjectRenderer,Draw);
}

int CScriptObjectRenderer::Reset(IFunctionHandler *pH)
{
	m_vBuffer.resize(0);
	m_vIdxBuf.resize(0);
	return pH->EndFunction();
}

int CScriptObjectRenderer::PushQuad(IFunctionHandler *pH)
{
	int params=pH->GetParamCount();
	if(params<5)
	{
		m_pScriptSystem->RaiseError("CScriptObjectRenderer::PushQuad wrong number of params");
		return pH->EndFunction();
	}
	float x,y,w,h,r=1,g=1,b=1,a=1,u0,v0,u1,v1;
	_SmartScriptObject pTI(m_pScriptSystem,true);
	pH->GetParam(1,x);
	pH->GetParam(2,y);
	pH->GetParam(3,w);
	pH->GetParam(4,h);
	if(!pH->GetParam(5,pTI))
	{
		m_pScriptSystem->RaiseError("CScriptObjectRenderer::PushQuad Invalid texinfo");
		return pH->EndFunction();
	}
	pTI->GetAt(1,u0);
	pTI->GetAt(2,v0);
	pTI->GetAt(3,u1);
	pTI->GetAt(4,v1);

	if(params>5)
	{
		if(m_pRenderer->GetFeatures() & RFT_RGBA)
		{
			pH->GetParam(6,r);
			pH->GetParam(7,g);
			pH->GetParam(8,b);
			pH->GetParam(9,a);
		}
		else
		{
			pH->GetParam(6,b);
			pH->GetParam(7,g);
			pH->GetParam(8,r);
			pH->GetParam(9,a);
		}
	}
	_Vtx vtx[4];
	unsigned short base=m_vBuffer.size();

	vtx[0].x=x;
	vtx[0].y=y;
  vtx[0].z=1.0f;
	vtx[0].u=u0;
	vtx[0].v=v0;

	vtx[1].x=x+w;
	vtx[1].y=y;
  vtx[1].z=1.0f;
	vtx[1].u=u1;
	vtx[1].v=v0;

	vtx[2].x=x+w;
	vtx[2].y=y+h;
  vtx[2].z=1.0f;
	vtx[2].u=u1;
	vtx[2].v=v1;

	vtx[3].x=x;
	vtx[3].y=y+h;
  vtx[3].z=1.0f;
	vtx[3].u=u0;
	vtx[3].v=v1;

  assert (r<=1 && g<=1 && b<=1 && a<=1);

	for(int i=0;i<4;i++)
	{
		vtx[i].cc[0]=(unsigned char)(r*255.0f);
		vtx[i].cc[1]=(unsigned char)(g*255.0f);
		vtx[i].cc[2]=(unsigned char)(b*255.0f);
		vtx[i].cc[3]=(unsigned char)(a*255.0f);
		m_vBuffer.push_back(vtx[i]);
	}

	m_vIdxBuf.push_back(base);
	m_vIdxBuf.push_back(base+1);
	m_vIdxBuf.push_back(base+3);
	m_vIdxBuf.push_back(base+1);
	m_vIdxBuf.push_back(base+2);
	m_vIdxBuf.push_back(base+3);

	return pH->EndFunction();
}

int CScriptObjectRenderer::Draw(IFunctionHandler *pH)
{
	if (m_vBuffer.size() > 0 && m_vIdxBuf.size() > 0)
	{
		USER_DATA tid;
		int cookie;
		if(pH->GetParamCount()<1)
		{
			m_pScriptSystem->RaiseError("CScriptObjectRenderer::Draw wrong number of params");
			return pH->EndFunction();
		}
		if((!pH->GetParamUDVal(1,tid,cookie)) || (!(cookie==USER_DATA_TEXTURE)))
		{
			m_pScriptSystem->RaiseError("CScriptObjectRenderer::Draw invalid texture");
			return pH->EndFunction();
		}

		//m_pRenderer->ResetToDefault();
		m_pRenderer->Set2DMode(true,800,600);
		m_pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
		m_pRenderer->SetTexture(tid);
		m_pRenderer->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
		m_pRenderer->SetCullMode(R_CULL_DISABLE);

		m_pRenderer->DrawDynVB((struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)&m_vBuffer[0], &m_vIdxBuf[0], m_vBuffer.size(), m_vIdxBuf.size(), R_PRIMV_TRIANGLES);

		m_pRenderer->Set2DMode(false,0,0);
	}
	
	m_vBuffer.resize(0);
	m_vIdxBuf.resize(0);
	
	return pH->EndFunction();
}

