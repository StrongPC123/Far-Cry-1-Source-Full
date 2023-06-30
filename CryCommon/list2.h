////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   list2.h
//  Version:     v1.00
//  Created:     28/5/2002 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: simple container, supports serialization
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef LIST_3DENGINE_H
#define LIST_3DENGINE_H

#include 	"platform.h"
#if defined(LINUX)
	#include "ILog.h"
	#include "ICryPak.h"
#else
	#include <assert.h>
#endif

template <class T> class list2
{
  T * m_pElements;
  int m_nCount;
  int m_nAllocatedCount;

public:
	typedef T value_type;
   
  int GetAllocatedMB() { return (m_nAllocatedCount*sizeof(T))/(1024*1024); }
	int GetMemoryUsage() { return m_nAllocatedCount*sizeof(T);}
  
  list2() { m_nCount=0; m_pElements=0; Reset(); }

  ~list2() { Reset(); }

  void Free() { Reset(); }
  
  void Reset() 
  {    
    if(m_pElements)
      free (m_pElements); 
    m_pElements=0;
    m_nCount=0; 
    m_nAllocatedCount= 0; 
  }

  void Clear() { m_nCount=0; }

  int Find(const T & p)
  {
    for(int i=0; i<m_nCount; i++)
      if(p == (*this)[i])
        return i;

    return -1;;
  }

  inline void AddList(const list2<T> & lstAnother)
  {
    PreAllocate(m_nCount + lstAnother.Count());

    memcpy(&m_pElements[m_nCount],&lstAnother.m_pElements[0],sizeof(m_pElements[0])*lstAnother.Count());

    m_nCount += lstAnother.Count();
  }

  inline void AddList(T * pAnotherArray, int nAnotherCount)
  {
    PreAllocate(m_nCount + nAnotherCount);

    memcpy(&m_pElements[m_nCount],pAnotherArray,sizeof(m_pElements[0])*nAnotherCount);

    m_nCount += nAnotherCount;
  }

  inline void AddDebug(const char *p__FILE__, long n__LINE__, const T & p)
  {
#ifdef _DEBUG
    if( m_nCount >= m_nAllocatedCount )
    {
      assert(&p<m_pElements || &p>=(m_pElements+m_nAllocatedCount));
      m_nAllocatedCount = m_nCount*2 + 8;
			m_pElements = (T*)_realloc_dbg(m_pElements,m_nAllocatedCount*sizeof(T), _NORMAL_BLOCK, p__FILE__, n__LINE__);
      assert(m_pElements);
    }

    memcpy(&m_pElements[m_nCount],&p,sizeof(m_pElements[m_nCount]));
    m_nCount++;
#else
		Add(p);
#endif
  }

  inline void Add(const T & p)
  {
    if( m_nCount >= m_nAllocatedCount )
    {
      assert(&p<m_pElements || &p>=(m_pElements+m_nAllocatedCount));
      m_nAllocatedCount = m_nCount*2 + 8;
      m_pElements = (T*)realloc(m_pElements,m_nAllocatedCount*sizeof(T));
      assert(m_pElements);
    }

    memcpy(&m_pElements[m_nCount],&p,sizeof(m_pElements[m_nCount]));
    m_nCount++;
  }

  void InsertBefore(const T & p, const unsigned int nBefore)
  {
    assert( nBefore>=0 && nBefore<=(unsigned int)m_nCount );
		T tmp; Add(tmp); // add empty object to increase memory buffer
    memmove(&(m_pElements[nBefore+1]), &(m_pElements[nBefore]), sizeof(T)*(m_nCount-nBefore-1));
    m_pElements[nBefore] = p;
  }

  void PreAllocate(int elem_count, int nNewCount = 0)
  {
    if( elem_count > m_nAllocatedCount )
    {
      m_nAllocatedCount = elem_count;

      T * new_elements = (T*)malloc(m_nAllocatedCount*sizeof(T));
      assert(new_elements);
      memset(new_elements, 0, sizeof(T)*m_nAllocatedCount);
      memcpy(new_elements, m_pElements, sizeof(T)*m_nCount);
      if(m_pElements)
        free (m_pElements);
      m_pElements = new_elements;
    }
    
    if(nNewCount)
      m_nCount = nNewCount;
  }

  inline void Delete(const int nElemId, const int nElemCount = 1)
  {
    assert( nElemId >= 0 && nElemId+nElemCount <= m_nCount );
    memcpy(&(m_pElements[nElemId]), &(m_pElements[nElemId+nElemCount]), sizeof(T)*(m_nCount-nElemId-nElemCount));
    m_nCount-=nElemCount;
  }

  inline void DeleteFastUnsorted(const int nElemId, const int nElemCount = 1)
  {
		assert( nElemId >= 0 && nElemId+nElemCount <= m_nCount );
		memcpy(&(m_pElements[nElemId]), &(m_pElements[m_nCount-nElemCount]), sizeof(T)*nElemCount);
		m_nCount-=nElemCount;
  }

  inline bool Delete(const T & del)
  {
		bool bFound = false;    
		for( int i=0; i<Count(); i++ )
    if(m_pElements[i] == del)
    { 
      Delete(i); 
      i--; 
			bFound = true;
    }
		return bFound;
  }

  inline int Count() const { return m_nCount; }
  inline unsigned int Size() const { return m_nCount; }
  
  inline int IsEmpty() const { return m_nCount==0; }

  inline T & operator [] (int i) const { assert( i>=0 && i<m_nCount ); return m_pElements[i]; }
  inline T & GetAt (int i) const { assert( i>=0 && i<m_nCount ); return  m_pElements[i]; }
  inline T * Get   (int i) const { assert( i>=0 && i<m_nCount ); return &m_pElements[i]; }
  inline T * GetElements() { return m_pElements; }

  inline T & Last() const { assert(m_nCount); return m_pElements[m_nCount-1]; }
  inline void DeleteLast() { assert(m_nCount); m_nCount--; }

  inline list2<T>& operator = (list2<T> & source_list)
  {
    Reset();
    PreAllocate(source_list.Count()+2);

    for(int i=0; i<source_list.Count(); i++)
      Add(source_list[i]);
    
    return *this;
  }
  
  bool operator == (const list2<T> & l)
  {
    if(Count() != l.Count())
      return 0;

    if(memcmp(m_pElements, l.m_pElements, m_nCount*sizeof(T))!=0)
      return 0;

    return 1;
  }

  bool operator != (const list2<T> & l)
  {
    return !(*this == l);
  }

  // Save/Load
  void Save(const char * file_name)
  {
    FILE * f = fxopen(file_name, "wb");
    if(f)
    {
      int size = sizeof(T);
      fwrite(&size, 4, 1, f);

      if(m_nCount)
        fwrite(m_pElements, sizeof(T), m_nCount, f);
      fclose(f);
    }
  }

  void Load(const char * file_name, struct ICryPak *pPak)
  {
    Clear();
    FILE * f;

    f = pPak->FOpen(file_name, "rb");
    if(!f)
      return;

    int size = 0;
    pPak->FRead(&size, 4, 1, f);

    while(!pPak->FEof(f) && sizeof(T)==size)
    {
      T tmp;
      if(pPak->FRead(&tmp, 1, sizeof(T), f) == sizeof(T))
        Add(tmp);
    }

    pPak->FClose(f);
  }

  // Save/Load
  void SaveToBuffer(const unsigned char * pBuffer, int & nPos)
  {
    // copy size of element
    int nSize = sizeof(T);
    if(pBuffer)
      memcpy((void*)&pBuffer[nPos],&nSize,4);
    nPos+=4;

    // copy count
    if(pBuffer)
      memcpy((void*)&pBuffer[nPos],&m_nCount,4);
    nPos+=4;

    // copy data
    if(m_nCount)
    {
      if(pBuffer)
        memcpy((void*)&pBuffer[nPos], m_pElements, sizeof(T)*m_nCount);
      nPos += sizeof(T)*m_nCount;
    }
  }

  void LoadFromBuffer(const unsigned char * pBuffer, int & nPos)
  {
    Reset();

    // copy size of element
    int nElemSize = 0;
    memcpy(&nElemSize,(void*)&pBuffer[nPos],4);
    assert(nElemSize == sizeof(T));
    nPos+=4;

    // copy count
    int nNewCount=0;
    memcpy((void*)&nNewCount,&pBuffer[nPos],4);
    assert(nNewCount>=0 && nNewCount<1000000);
    nPos+=4;

    // copy data
    if(nNewCount)
    {
      PreAllocate(nNewCount,nNewCount);
      memcpy((void*)m_pElements, &pBuffer[nPos], sizeof(T)*nNewCount);
      nPos += sizeof(T)*nNewCount;
    }
  }

  // Sorting of objects
  static int __cdecl Cmp_Ptrs1(const void* v1, const void* v2)
  {
    T* p1 = (T*)v1;
    T* p2 = (T*)v2;

    if(p1->m_fDistance > p2->m_fDistance)
      return 1;
    else if(p1->m_fDistance < p2->m_fDistance)
      return -1;

    return 0;
  }

  static int __cdecl Cmp_Ptrs2(const void* v1, const void* v2)
  {
    T* p1 = (T*)v1;
    T* p2 = (T*)v2;

    if(p1->m_fDistance > p2->m_fDistance)
      return -1;
    else if(p1->m_fDistance < p2->m_fDistance)
      return 1;

    return 0;
  }

  void SortByDistanceMember(bool front_to_back = true, int nSortOffSet = 0)
  {
    if(m_nCount-nSortOffSet > 1)
    {
      if(front_to_back)
        qsort(&m_pElements[nSortOffSet], m_nCount-nSortOffSet, sizeof(T), Cmp_Ptrs1);
      else
        qsort(&m_pElements[nSortOffSet], m_nCount-nSortOffSet, sizeof(T), Cmp_Ptrs2);
    }
  }

  // Sorting of pointers to objects
  static int Cmp_Ptrs1_(const void* v1, const void* v2)
  {
    T p1 = *((T*)v1);
    T p2 = *((T*)v2);

    if(p1->m_fDistance > p2->m_fDistance)
      return 1;
    else if(p1->m_fDistance < p2->m_fDistance)
      return -1;

    return 0;
  }

  static int Cmp_Ptrs2_(const void* v1, const void* v2)
  {
    T p1 = *((T*)v1);
    T p2 = *((T*)v2);

    if(p1->m_fDistance > p2->m_fDistance)
      return -1;
    else if(p1->m_fDistance < p2->m_fDistance)
      return 1;

    return 0;
  }

  void SortByDistanceMember_(bool front_to_back = true)
  {
    if(front_to_back)
      qsort(&m_pElements[0], m_nCount, sizeof(T), Cmp_Ptrs1_);
    else
      qsort(&m_pElements[0], m_nCount, sizeof(T), Cmp_Ptrs2_);
  }

	// STL convention driver routines to facilitate conversion from this class to optimized
	// containers with standard, well-known and thus more convenient for some other (except
	// the author) people naming conventions.
	// All the routines mean the same as their counterparts in std::vector<T>.
  bool empty() const
	{
		return IsEmpty() != 0;
	}

	void reserve (unsigned numElements)
	{
		PreAllocate ((int)numElements);
	}

	// C++ Standard Library makes new[] call instead of old-fashioned *alloc()
	// this ensures the objects are constructed properly.
	void resize (size_t numElements)
	{
		int nOldCount = m_nCount;
		PreAllocate ((int)numElements, (int)numElements);
		assert (numElements == size());
		for (int nElement = nOldCount; nElement < m_nCount; ++nElement)
			(*this)[nElement] = T();
	}

	void push_back (const T& rElement)
	{
		Add (rElement);
	}

	unsigned size ()const
	{
		return (unsigned)m_nCount;
	}

	unsigned capacity() const
	{
		return (unsigned)m_nAllocatedCount;
	}

	T* begin() {return m_pElements;}
	T* end() {return m_pElements+m_nCount;}
	const T* begin()const {return m_pElements;}
	const T* end()const {return m_pElements+m_nCount;}
};

#endif // LIST_3DENGINE_H
