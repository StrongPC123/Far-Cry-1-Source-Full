#ifndef __TARRAY_H__
#define __TARRAY_H__

#include 	"platform.h"
#if defined(LINUX)
	#include "ILog.h"
#else
	#include <assert.h>
#endif

#ifndef CLAMP
#define CLAMP(X, mn, mx) (X<mn ? mn : X<mx ? X : mx)
#endif

#ifndef LERP
#define LERP(A, B, Alpha) (A + Alpha * (B-A))
#endif

// Safe memory freeing
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);		(p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);		(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release();	(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE_FORCE
#define SAFE_RELEASE_FORCE(p)			{ if(p) { (p)->Release(1);	(p)=NULL; } }
#endif

// this is an allocator that's allocates 16-byte-aligned blocks,
// using the normal mem manager. The overhead of allocation is always 16 byte more,
// to keep the original address in the DWORD before the aligned address
// NOTE: since this is like a garbage collector (the items are not guaranteed to be
// freed immediately), the destructors won't be called
// for simplicity, constructors won't be called either
//
// DOES NOT construct / destruct objects, so use with care!
template <typename T>
class TAllocator16
{
public:
	typedef size_t size_type;
	typedef T *pointer;

	TAllocator16 ()
#if defined(_DEBUG) && defined(_INC_CRTDBG) && !defined(WIN64)
		: m_szParentObject("TAllocator16"),
		m_nParentIndex (1)
#endif
	{
	}
	TAllocator16 (const char* szParentObject, int nParentIndex)
#if defined(_DEBUG) && defined(_INC_CRTDBG) && !defined(WIN64)
		: m_szParentObject(szParentObject),
		m_nParentIndex (nParentIndex)
#endif
	{
	}

	// allocates the aligned memory for the given number of objects;
	// puts the pointer to the actually allocated block before the aligned memory block
	pointer allocate (size_type _Count)
	{
		pointer p;
#if defined(_DEBUG) && defined(_INC_CRTDBG) && !defined(WIN64)
		p = (pointer)_malloc_dbg (0x10+_Count * sizeof(T), _NORMAL_BLOCK, m_szParentObject, m_nParentIndex);
#else
		p = (pointer)malloc (0x10+_Count*sizeof(T));
#endif
		pointer pResult = (pointer)(((UINT_PTR)p+0x10)&~0xF);
		// save the offset to the actual allocated address behind the useable aligned address
		reinterpret_cast<int*>(pResult)[-1] = (char*)p - (char*)pResult;
		return pResult;
	}

	pointer allocate_construct (size_type _Count)
	{
		pointer p = allocate(_Count);
		return p;
	}

	void deallocate_destroy(pointer _Ptr)
	{
		if (!_Ptr)
			return;
		int nOffset = ((int*)_Ptr)[-1];
		assert (nOffset >= -16 && nOffset <= -4);
#if defined(_DEBUG) && defined(_INC_CRTDBG) && !defined(WIN64)
		_free_dbg (((char*)_Ptr)+nOffset, _NORMAL_BLOCK);
#else
		free (((char*)_Ptr)+nOffset);
#endif
	}

protected:
#if defined(_DEBUG) && defined(_INC_CRTDBG) && !defined(WIN64)
	// the parent object (on behalf of which to call the new operator)
	const char* m_szParentObject; // the file name
	int m_nParentIndex; // the file line number
#endif
};


/*----------------------------------------------------------------------------
  Sorting template.
----------------------------------------------------------------------------*/

//
// Sort elements. The sort is unstable, meaning that the ordering of equal 
// items is not necessarily preserved.
//
template<class T> struct TSortStack
{
  T* Min;
  T* Max;
};

template<class T> void Sort( T* First, int Num )
{
  if( Num<2 )
    return;
  TSortStack<T> RecursionStack[32]={{First,First+Num-1}}, Current, Inner;
  for( TSortStack<T>* StackTop=RecursionStack; StackTop>=RecursionStack; --StackTop )
  {
    Current = *StackTop;
  Loop:
    INT Count = Current.Max - Current.Min + 1;
    if( Count <= 8 )
    {
      // Use simple bubble-sort.
      while( Current.Max > Current.Min )
      {
        T *Max, *Item;
        for( Max=Current.Min, Item=Current.Min+1; Item<=Current.Max; Item++ )
          if( Compare(*Item, *Max) > 0 )
            Max = Item;
        Exchange( *Max, *Current.Max-- );
      }
    }
    else
    {
      // Grab middle element so sort doesn't exhibit worst-cast behaviour with presorted lists.
      Exchange( Current.Min[Count/2], Current.Min[0] );

      // Divide list into two halves, one with items <=Current.Min, the other with items >Current.Max.
      Inner.Min = Current.Min;
      Inner.Max = Current.Max+1;
      for( ; ; )
      {
        while( ++Inner.Min<=Current.Max && Compare(*Inner.Min, *Current.Min) <= 0 );
        while( --Inner.Max> Current.Min && Compare(*Inner.Max, *Current.Min) >= 0 );
        if( Inner.Min>Inner.Max )
          break;
        Exchange( *Inner.Min, *Inner.Max );
      }
      Exchange( *Current.Min, *Inner.Max );

      // Save big half and recurse with small half.
      if( Inner.Max-1-Current.Min >= Current.Max-Inner.Min )
      {
        if( Current.Min+1 < Inner.Max )
        {
          StackTop->Min = Current.Min;
          StackTop->Max = Inner.Max - 1;
          StackTop++;
        }
        if( Current.Max>Inner.Min )
        {
          Current.Min = Inner.Min;
          goto Loop;
        }
      }
      else
      {
        if( Current.Max>Inner.Min )
        {
          StackTop->Min = Inner  .Min;
          StackTop->Max = Current.Max;
          StackTop++;
        }
        if( Current.Min+1<Inner.Max )
        {
          Current.Max = Inner.Max - 1;
          goto Loop;
        }
      }
    }
  }
}

    
template<class T> class TArray
{
protected:
  T*    m_pElements;
  int   m_nCount;
  int   m_nAllocatedCount;

public:
	typedef T value_type;

  TArray(void)       { m_pElements = NULL, m_nCount = m_nAllocatedCount = 0; }
  TArray(int Count)  { m_pElements = NULL; m_nCount = Count; m_nAllocatedCount = Count; Realloc(); }
  TArray(int Use, int Max)  { m_nCount = Use; m_nAllocatedCount = Max; Realloc(); }
  ~TArray(void)
  {
    Free();
  }

  void Reset(){Free();}

  void Free()
  {
    if (m_pElements)
    {
      free(m_pElements);
      m_pElements = NULL;
    }
    m_nCount = m_nAllocatedCount = 0;
  }

  void Create (int Count) { m_pElements = NULL; m_nCount = Count; m_nAllocatedCount = Count; Realloc(); Clear(); }
  void Copy (const TArray<T>& src)
  {
    m_pElements = NULL;
    m_nCount = m_nAllocatedCount = src.Num();
    Realloc();
    memcpy(m_pElements, src.m_pElements, src.Num()*sizeof(T));
  }
  void Copy (const T *src, int numElems)
  {
    for (int i=0; i<numElems; i++)
    {
      AddElem(src[i]);
    }
  }
#ifdef _XBOX
#ifndef _DEBUG
#ifndef realloc
#error !!!!!
#endif
#endif //_DEBUG
#endif //_XBOX
  void Realloc()
  {
    m_pElements = (T *)realloc(m_pElements, m_nAllocatedCount*sizeof(T));
    if (m_nAllocatedCount*sizeof(T))
      assert (m_pElements);
  }

  void Remove(int Index,int Count=1)
  {
    if ( Count )
    {
      memcpy(m_pElements+Index, m_pElements+(Index+Count), sizeof(T)*(m_nCount-Index-Count));
      m_nCount -= Count;
    }
  }

  void Shrink()
  {
    if (m_nCount <= 0)
      return;
    assert(m_nAllocatedCount>=m_nCount);
    if( m_nAllocatedCount != m_nCount )
    {
      m_nAllocatedCount = m_nCount;
      Realloc();
    }
  }

  void _Remove(int Index,int Count)
  {
    assert ( Index>=0 );
    assert ( Index<=m_nCount );
    assert ( (Index+Count)<=m_nCount );

    Remove(Index, Count);
  }

  int Num(void) const  { return m_nCount; }
  int GetSize(void) const { return m_nAllocatedCount; }
  void SetNum(int n) { m_nCount = m_nAllocatedCount = n; }
  void SetUse(int n) { m_nCount = n; }
  void Alloc(int n) { m_nAllocatedCount = n; Realloc(); }
  void Reserve(int n) { SetNum(n); Realloc(); Clear(); }
  void Expand(int n)
  {
    if (n > m_nAllocatedCount)
      ReserveNew(n);
  }
  void ReserveNew(int n)
  {
    int num = m_nCount;
    SetNum(n);
    Realloc();
    memset(&m_pElements[num], 0, sizeof(T)*(m_nCount-num));
  }
  void Grow(int n)
  {
    n += m_nCount;
    SetNum(n);
    Realloc();
  }
  void GrowReset(int n)
  {
    int num = m_nAllocatedCount;
    AddIndex(n);
    if (num != m_nAllocatedCount)
      memset(&m_pElements[num], 0, sizeof(T)*(m_nAllocatedCount-num));
  }

  int *GetNumAddr(void) { return &m_nCount; }
  T** GetDataAddr(void) { return &m_pElements; }

  T* Data(void) const { return m_pElements; }
  T& Get(int id) const { return m_pElements[id]; }

  TArray& operator=(TArray& fa)
  {
    m_pElements = fa.m_pElements;
    m_nCount = fa.m_nCount;
    m_nAllocatedCount = fa.m_nAllocatedCount;
    return *this;
  }

  /*const TArray operator=(TArray fa) const
  {
    TArray<T> t = TArray(fa.m_nCount,fa.m_nAllocatedCount);
    for ( int i=0; i<fa.Num(); i++ )
    {
      t.AddElem(fa[i]);
    }
    return t;
  }*/

  const T& operator[](int i) const { return m_pElements[i]; }
        T& operator[](int i)       { return m_pElements[i]; }
  T& operator * ()                 { return *m_pElements;   }

  TArray(const TArray<T>& cTA)
  {
    m_pElements = NULL;
    m_nCount = m_nAllocatedCount = cTA.Num();
    Realloc();
    m_nCount = 0;
    for ( int i=0; i<cTA.Num(); i++ )
    {
      AddElem(cTA[i]);
    }
  }

  void Clear(void)
  {
    memset(m_pElements, 0, m_nCount*sizeof(T));
  }

  void Set(int m)
  {
    memset(m_pElements, m, m_nAllocatedCount*sizeof(T));
  }

  void AddIndex(int inc)
  {
    m_nCount += inc;
    if ( m_nCount > m_nAllocatedCount )
    {
#ifdef PS2    
      m_nAllocatedCount = m_nCount + (m_nCount>>1) + 128;
#else
      m_nAllocatedCount = m_nCount + (m_nCount>>1) + 32;
#endif
      Realloc();
    }
  }

  void AddIndexNoCache(int inc)
  {
    m_nCount += inc;
    if ( m_nCount > m_nAllocatedCount )
    {
      m_nAllocatedCount = m_nCount;
      Realloc();
    }
  }

  void Add(const T & elem){AddElem(elem);}
  void AddElem(const T elem)
  {
    int m = m_nCount;
    AddIndex(1);
    m_pElements[m] = elem;
  }
  void AddElemNoCache(const T elem)
  {
    int m = m_nCount;
    AddIndexNoCache(1);
    m_pElements[m] = elem;
  }

  int Find(const T & p)
  {
    for(int i=0; i<m_nCount; i++)
    {
      if(p == (*this)[i])
        return i;
    }
    return -1;
  }

  void Delete(int n){DelElem(n);}
  void DelElem(int n)
  {
//    memset(&m_pElements[n],0,sizeof(T));
    _Remove(n, 1);
  }

  void Load(const char * file_name)
  {
    Clear();
    FILE * f = fxopen(file_name, "rb");
    if(!f)
      return;
    
    int size = 0;
    fread(&size, 4, 1, f);
    
    while(!feof(f) && sizeof(T)==size)
    {
      T tmp;
      if(fread(&tmp, 1, sizeof(T), f) == sizeof(T))
        AddElem(tmp);
    }
    
    fclose(f);
  }


  /* LINUX - port [MG], this is not needed and does not compile with gcc under linux
  // Sorting
  static int Cmp_Ptrs1(const void* v1, const void* v2)
  {
    T* p1 = (T*)v1;
    T* p2 = (T*)v2;
    
    if(p1->distance > p2->distance)
      return 1;
    else if(p1->distance < p2->distance)
      return -1;
    
    return 0;
  }
  
  static int Cmp_Ptrs2(const void* v1, const void* v2)
  {
    T* p1 = (T*)v1;
    T* p2 = (T*)v2;
    
    if(p1->distance > p2->distance)
      return -1;
    else if(p1->distance < p2->distance)
      return 1;
    
    return 0;
  }
  
  void SortByDistanceMember(bool front_to_back = true)
  {
    if(front_to_back)
      qsort(&m_pElements[0], m_nCount, sizeof(T), Cmp_Ptrs1);
    else
      qsort(&m_pElements[0], m_nCount, sizeof(T), Cmp_Ptrs2);
  }
*/

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
			Reserve(nNewCount);
      memcpy((void*)m_pElements, &pBuffer[nPos], sizeof(T)*nNewCount);
      nPos += sizeof(T)*nNewCount;
    }
  }

	// Standard compliance interface
	//
	// This is for those who don't want to learn the non standard and 
	// thus not very convenient interface of TArray, but are unlucky
	// enough not to be able to avoid using it.
	void clear(){Free();}
	unsigned size()const {return m_nCount;}
	unsigned capacity() const {return m_nAllocatedCount;}
	bool empty()const {return size() == 0;}
	void push_back (const T& rSample) {Add(rSample);}
	T* begin() {return m_pElements;}
	T* end() {return m_pElements + m_nCount;}
	const T* begin()const {return m_pElements;}
	const T* end()const {return m_pElements + m_nCount;}

	int GetMemoryUsage() { return (int)(m_nAllocatedCount*sizeof(T)); }
};

template <class T> inline void Exchange(T& X, T& Y)
{
  const T Tmp = X;
  X = Y;
  Y = Tmp;
}

#endif // __TARRAY_H__
