#include "stdafx.h"
#include "Controller.h"
#include "ControllerCryBone.h"
#include "CryKeyInterpolation.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const bool g_bOptimizeAndLogKeys = false;

CControllerCryBone::CControllerCryBone():
	m_nControllerId(0),
	m_arrTimes ("CControllerCryBone.Keys"),
	m_arrKeys ("CControllerCryBone.Keys")
{
}

CControllerCryBone::~CControllerCryBone()
{
}

// Loads (initializes) controller from the given chunk. The chunk descriptor is followed by the 
// chunk data immediately, Returns true if successful.
bool CControllerCryBone::Load (const CONTROLLER_CHUNK_DESC_0826* pChunk, float fScale)
{
	//m_Chunk = *pChunk;
	m_nControllerId = pChunk->nControllerId;

	if(pChunk->chdr.ChunkType != ChunkType_Controller || pChunk->chdr.ChunkVersion != CONTROLLER_CHUNK_DESC_0826::VERSION)
  {
		m_nControllerId = 0;
    GetLog()->LogToFile("CControllerCryBone::Load: File version error");
    return false;
  }

	if (pChunk->type != CTRL_CRYBONE)
		return false;

	unsigned i, numKeys = pChunk->nKeys;
  m_arrKeys.reinit(numKeys);
	m_arrTimes.reinit (numKeys);
	{
		const CryBoneKey * pCryBoneKey = (const CryBoneKey*)(pChunk+1); // iterator through the original keys
		int *pTime = m_arrTimes.begin();
		PQLog* pKey = m_arrKeys.begin();
		CryQuat qLast; // the last orientation
		for (i = 0; i < numKeys; ++i)
		{
			m_arrTimes[i] = pCryBoneKey->time;
			m_arrKeys[i].vPos = pCryBoneKey->relpos * fScale;

			// make sure we don't make >PI rotations per  keyframe
			if ( (qLast|(pCryBoneKey->relquat)) >= 0)
				qLast = pCryBoneKey->relquat;
			else
				qLast = -pCryBoneKey->relquat;

			m_arrKeys[i].vRotLog = log(qLast).v;

			++pCryBoneKey, ++pTime, ++pKey;
		}
	}

	return true;
}

// Loads (initializes) controller from the given chunk. The chunk descriptor is followed by the 
// chunk data immediately. Returns true if successful
bool CControllerCryBone::Load (const CONTROLLER_CHUNK_DESC_0827* pChunk, unsigned nSize, float fScale)
{
	m_nControllerId = pChunk->nControllerId;
	unsigned numKeys = pChunk->numKeys;

	const CryKeyPQLog* pCryKey = (const CryKeyPQLog*)(pChunk+1);

	if ((const char*)(pCryKey + numKeys) > ((const char*)pChunk) + nSize)
	{
		GetLog()->LogError ("\002CControllerCryBone::Load: cannot load controller 0x%08X, chunk is truncated",
			m_nControllerId);
		return false;
	}

	m_arrKeys.reinit (numKeys);
	m_arrTimes.reinit (numKeys);
	int *pTime = m_arrTimes.begin();
	PQLog* pKey = m_arrKeys.begin();

	for (unsigned i = 0; i < numKeys; ++i)
	{
		m_arrTimes[i] = pCryKey->nTime;
		m_arrKeys[i].vPos = pCryKey->vPos * fScale;
		m_arrKeys[i].vRotLog = pCryKey->vRotLog;

		++pCryKey, ++pTime, ++pKey;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
// retrieves the position and orientation (in the logarithmic space,
// i.e. instead of quaternion, its logarithm is returned)
// may be optimal for motion interpolation
void CControllerCryBone::GetValue2 (float fTime, PQLog& pq)
{
  assert(numKeys());

  if(!(m_arrTimes[0] < fTime))
  {
		pq = m_arrKeys[0];
    return ;
  }

  if(m_arrTimes[numKeys()-1] <= fTime)
  {
    pq = m_arrKeys[numKeys()-1];
    return ;
  }

  assert(numKeys()>1);

  int nPos  = numKeys()>>1;
  int nStep = numKeys()>>2;

  // use binary search
  while(nStep)
  {
    if(fTime < m_arrTimes[nPos])
      nPos = nPos - nStep;
    else
    if(fTime > m_arrTimes[nPos])
      nPos = nPos + nStep;
    else 
      break;
  
    nStep = nStep>>1;
  }

  // finetuning needed since time is not linear
  while(fTime > m_arrTimes[nPos])
    nPos++;

  while(fTime < m_arrTimes[nPos-1])
    nPos--;

  assert(nPos > 0 && nPos < (int)numKeys());  
  assert(m_arrTimes[nPos] != m_arrTimes[nPos-1]);

  float t = (float(fTime-m_arrTimes[nPos-1]))/(m_arrTimes[nPos] - m_arrTimes[nPos-1]);
	PQLog pKeys[2] = {m_arrKeys[nPos-1], m_arrKeys[nPos]};
	AdjustLogRotations (pKeys[0].vRotLog, pKeys[1].vRotLog);
	pq.blendPQ (pKeys, t);
}

ILog* CControllerCryBone::GetLog() const
{
	return g_GetLog();
}


size_t CControllerCryBone::sizeofThis ()const
{
	return sizeof(*this) + m_arrKeys.size() * (sizeof(m_arrKeys[0])+sizeof(m_arrTimes[0]));
}