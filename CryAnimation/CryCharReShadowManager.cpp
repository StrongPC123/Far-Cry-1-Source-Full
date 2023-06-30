#include "stdafx.h"
//#include "CryAnimation.h"
#include "CVars.h"
#include "CryCharReShadowVolume.h"
#include "CryCharReShadowManager.h"


CryCharReShadowManager::CryCharReShadowManager ()
{
	
}

CryCharReShadowManager::~CryCharReShadowManager ()
{
	for (unsigned i = 0; i < m_arrPool.size(); ++i)
		delete m_arrPool[i];
	m_arrPool.clear();
}

// creates a new shadow volume object or retrieves an old one from the pool
// May return NULL, if insufficient resources; in this case, no further action on
// creating shadow volumes must be attempted
CryCharReShadowVolume* CryCharReShadowManager::newShadow ()
{
	// first try to find already existing resource:
	// the one with age > 1. If the age is:
	// 0: this is a different shadow that was rendered in this very frame 
	//    this means multiple shadows from different light sources, and we need
	//    different resources for them
	// 1: this is a shadow that was rendered in the previous frame and may still
	//    be being rendered, which means we'd have to wait the fences.
	for (unsigned i = 0; i < m_arrPool.size(); ++i)
		if (m_arrPool[i]->getAgeFrames() > (unsigned)(g_GetCVars()->ca_ShadowDoubleBuffer()&1))
		{
			return m_arrPool[i];
		}

	// we didn't find any in the pool, so create a new one, if we didn't yet exceed the limit
	if (m_arrPool.size() > (unsigned) g_GetCVars()->ca_ShadowBufferLimit())
		return NULL; // we exceeded the limit

	CryCharReShadowVolume* pNewShadow = new CryCharReShadowVolume();
	m_arrPool.push_back(pNewShadow );
	return pNewShadow;
}


// cleans up the shadow volume resources that weren't used lately
// (collect the garbage)
void CryCharReShadowManager::shrink()
{
	// we leave at least one shadow resource always;
	// we delete excess shadow resources when they're not requested during
	// 50 frames AND 5 seconds

	// 1 in case of single-buffer
	// 2 in case of double-buffer
	unsigned nMinPoolSize = 1 + (g_GetCVars()->ca_ShadowDoubleBuffer()&1);

	if (nMinPoolSize > (unsigned) g_GetCVars()->ca_ShadowBufferLimit())
		nMinPoolSize = g_GetCVars()->ca_ShadowBufferLimit();

	// delete the extra unused buffers.
	std::vector<CryCharReShadowVolume*>::iterator it;
	for (it = m_arrPool.begin(); it != m_arrPool.end() && m_arrPool.size() > nMinPoolSize; )
	{
		CryCharReShadowVolume* pShadow = *it;
		if (pShadow->getAgeFrames() > 50
			&& pShadow->getAgeSeconds() > 5)
		{
			delete pShadow;
			it = m_arrPool.erase(it);
		}
		else
			++it;
	}
}



void CryCharReShadowManager::GetMemoryUsage (ICrySizer* pSizer)
{
	pSizer->Add (*this);
	for (unsigned i = 0; i < m_arrPool.size(); ++i)
		m_arrPool[i]->GetMemoryUsage(pSizer);
}