#include "stdafx.h"
#include <ITimer.h>
#include <CrySizer.h>
#include "CrySizerImpl.h"

CrySizerImpl::CrySizerImpl()
{
	// to avoid reallocations during walk through the memory tree, reserve the space for the names
	clear();
}

CrySizerImpl::~CrySizerImpl()
{
}

void CrySizerImpl::Push (const char* szComponentName)
{
	m_stackNames.push_back (getNameIndex(getCurrentName(), szComponentName));
	// if the depth is too deep, something is wrong, perhaps an infinite loop
	assert (m_stackNames.size() < 128);
}

void CrySizerImpl::PushSubcomponent (const char* szSubcomponentName)
{
	Push (szSubcomponentName);
}


void CrySizerImpl::Pop ()
{
	if (!m_stackNames.empty())
		m_stackNames.pop_back();
	else
		assert (0);
}

// returns the index of the current name on the top of the name stack
size_t CrySizerImpl::getCurrentName()const
{
	assert(!m_stackNames.empty());
	return m_stackNames.empty()?0:m_stackNames.back();
}



// searches for the name in the name array; adds the name if it's not there and returns the index
size_t CrySizerImpl::getNameIndex (size_t nParent, const char* szComponentName)
{
	NameArray::const_iterator it = m_arrNames.begin(), itEnd = it + m_arrNames.size();
	for (; it != itEnd; ++it)
#if defined(LINUX)
		if (!strcasecmp(it->strName.c_str(), szComponentName) && it->nParent == nParent)
#else
		if (!strcmp(it->strName.c_str(), szComponentName) && it->nParent == nParent)
#endif
			return it-m_arrNames.begin();

	size_t nNewName = m_arrNames.size();
	m_arrNames.resize(nNewName+1);

	m_arrNames[nNewName].assign(szComponentName, nParent);

	m_arrNames[nParent].arrChildren.push_back(nParent);

	return nNewName;
}


// adds an object identified by the unique pointer (it needs not be
// the actual object position in the memory, though it would be nice,
// but it must be unique throughout the system and unchanging for this object)
// RETURNS: true if the object has actually been added (for the first time)
//          and calculated
bool CrySizerImpl::AddObject (const void* pIdentifier, size_t sizeBytes)
{
	if (!pIdentifier || !sizeBytes)
		return false; // we don't add the NULL objects

	//return true;
	
	Object NewObject(pIdentifier, sizeBytes, getCurrentName());

	// check if the last object was the same
	if (NewObject == m_LastObject)
	{
		assert (m_LastObject.nSize == sizeBytes);
		return false;
	}

	ObjectSet& rSet = m_setObjects[getHash(pIdentifier)];
	ObjectSet::iterator it = rSet.find (NewObject);
	if (it == rSet.end())
	{
		// there's no such object in the map, add it
		rSet.insert (NewObject);
		ComponentName& CompName = m_arrNames[getCurrentName()];
		++CompName.numObjects;
		CompName.sizeObjects += sizeBytes;
		return true;
	}
	else
	{
		// there's such object in the map, don't add it
		
		if (sizeBytes != it->nSize)
		{
			// if the following assert fails:
			assert (0);
			// .. it means we have one object that's added two times with different sizes; that's screws up the whole idea
			// we assume there are two different objects that are for some reason assigned the same id
			Object *pObj = const_cast<Object*>(&(*it));
			pObj->nSize += sizeBytes; // anyway it's an invalid situation
			ComponentName& CompName = m_arrNames[getCurrentName()];
			CompName.sizeObjects += sizeBytes;
			return true; // yes we added the object, though there were an error condition
		}
		return false;
	}
}



// finalizes data collection, should be called after all objects have been added
void CrySizerImpl::end()
{
	// clean up the totals of each name
	size_t i;
	for (i = 0; i < m_arrNames.size(); ++i)
	{
		assert (i == 0 || (m_arrNames[i].nParent < i && m_arrNames[i].nParent >=0));
		m_arrNames[i].sizeObjectsTotal = m_arrNames[i].sizeObjects;
	}

	// add the component's size to the total size of the parent.
	// for every component, all their children are put after them in the name array
	// we don't include the root because it doesn't belong to any other parent (nowhere further to add)
	for (i = m_arrNames.size() - 1; i > 0; --i)
	{
		// the parent's total size is increased by the _total_ size (already calculated) of this object
		m_arrNames[m_arrNames[i].nParent].sizeObjectsTotal += m_arrNames[i].sizeObjectsTotal;
	}
}


void CrySizerImpl::clear()
{
	for (unsigned i = 0 ; i < g_nHashSize; ++i)
		m_setObjects[i].clear();

	m_arrNames.clear();
	m_arrNames.push_back("TOTAL"); // the default name, with index 0
	m_stackNames.clear();
	m_stackNames.push_back(0);
	m_LastObject.pId = NULL;
}

// hash function for an address; returns value 0..1<<g_nHashSize
unsigned CrySizerImpl::getHash (const void* pId)
{
	//return (((unsigned)pId) >> 4) & (g_nHashSize-1);
	
	// pseudorandomizing transform
	ldiv_t _Qrem = ldiv(((UINT_PTR)pId >> 2), 127773);
	_Qrem.rem = 16807 * _Qrem.rem - 2836 * _Qrem.quot;
	if (_Qrem.rem < 0)
		_Qrem.rem += 2147483647; // 0x7FFFFFFF
	return ((unsigned)_Qrem.rem) & (g_nHashSize-1);
	
}
