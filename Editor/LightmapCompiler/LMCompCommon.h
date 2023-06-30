// ---------------------------------------------------------------------------------------------
//	Crytek CryENGINE source code
//	History:
//  - Created by Tim Schroeder
// ---------------------------------------------------------------------------------------------

#ifndef __LM_COMP_COMMONH_H__
#define __LM_COMP_COMMONH_H__

#pragma once

struct IndoorBaseInterface
{
	ILog			*m_pLog;
	IRenderer	*m_pRenderer;	
	I3DEngine	*m_p3dEngine;
	IConsole	*m_pConsole;
	ISystem		*m_pSystem;
};

#include "LMCompStructures.h"

extern ICompilerProgress *g_pIProgress;

_inline void __cdecl _TRACE(std::vector<CString>& rLogInfo, const bool cbDoOutput, const char *sFormat, ... )
{
	va_list vl;
	static char sTraceString[1024];
	
	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);

	rLogInfo.push_back(CString(sTraceString));

	if(cbDoOutput == false)
		return;

	if (g_pIProgress == NULL)
		return;
	g_pIProgress->Output(sTraceString);
}

_inline void __cdecl _TRACE(const char *sFormat, ... )
{
	va_list vl;
	static char sTraceString[1024];
	
	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);

	if (g_pIProgress == NULL)
		return;
	g_pIProgress->Output(sTraceString);
}

/*
__inline bool IsLeafBufferEmpty(CLeafBuffer *pLB)
{
	return (pLB->m_pSecVertBuffer == NULL && pLB->m_pSecVertBuffer->m_NumVerts == 0 && pLB->m_NumIndices == 0);
}
*/

#endif