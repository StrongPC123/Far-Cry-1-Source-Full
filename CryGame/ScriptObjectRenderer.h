
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef _SCRIPT_OBJECT_RENDERER_H_
#define _SCRIPT_OBJECT_RENDERER_H_

#include <vector>
#include <IRenderer.h>
#include <IScriptSystem.h>
#include <_ScriptableEx.h>

struct IRenderer;
class CVertexBuffer;
#define USER_DATA_SCRIPTOBJRENDERER 0x100

typedef union tag_Clr
{
	unsigned int c;
	unsigned char cc[4];
}_Clr;

typedef struct tag_Vtx
{
	float x,y,z;
	unsigned char cc[4];
	float u,v;
}_Vtx;

typedef std::vector<_Vtx> _VtxVec;
typedef std::vector<unsigned short> _IdxBuf;

class CScriptObjectRenderer :
	public _ScriptableEx<CScriptObjectRenderer>
{
public:
	CScriptObjectRenderer(void);
	virtual ~CScriptObjectRenderer(void);
	static void InitializeTemplate(IScriptSystem *pSS);
	IScriptObject *Create(IScriptSystem *pSS,IRenderer *pRen);
	int Reset(IFunctionHandler *pH);
	int PushQuad(IFunctionHandler *pH);
	int Draw(IFunctionHandler *pH);
private:
	_VtxVec m_vBuffer;
	_IdxBuf m_vIdxBuf;
	IRenderer *m_pRenderer;
};

#endif
