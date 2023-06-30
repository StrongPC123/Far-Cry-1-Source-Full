// ScriptObjectParticle.h: interface for the CScriptObjectParticle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTPARTICLE_H__893DBB83_74EB_421D_A995_066F02F85156__INCLUDED_)
#define AFX_SCRIPTOBJECTPARTICLE_H__893DBB83_74EB_421D_A995_066F02F85156__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>

struct ISystem;
struct I3DEngine;
struct ParticleParams;
/*! This class implements script-functions for particles and decals.

	REMARKS:
	After initialization of the script-object it will be globally accessable through scripts using the namespace "Particle".
	
	Example:
		Particle.CreateDecal(pos, normal, scale, lifetime, decal.texture, decal.object, rotation);

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/

class CScriptObjectParticle :
public _ScriptableEx<CScriptObjectParticle>
{
public:
	static void InitializeTemplate(IScriptSystem *pSS);
	int CreateParticleLine(IFunctionHandler *pH);
	CScriptObjectParticle();
	virtual ~CScriptObjectParticle();
	void Init(IScriptSystem *pScriptSystem, ISystem *pSystem);
	int CreateParticle(IFunctionHandler *pH); //vector vector obj(return void)
	int CreateDecal(IFunctionHandler *pH); //Vector,Vector,float,float,int(return void)
	int SpawnEffect(IFunctionHandler *pH); //pos,dir,sEffectName (return void)
  bool ReadParticleTable(IScriptObject *pITable, ParticleParams &sParamOut);	

private:

	ISystem *m_pSystem;
	I3DEngine *m_p3DEngine;
public:
	int Attach(IFunctionHandler * pH);
	int Detach(IFunctionHandler * pH);
};

#endif // !defined(AFX_SCRIPTOBJECTPARTICLE_H__893DBB83_74EB_421D_A995_066F02F85156__INCLUDED_)
