#ifndef _ARRAY2D_H_
#define _ARRAY2D_H_

// Dynamic replacement for static 2d array
template <class T> struct Array2d
{
  Array2d() 
  { 
    m_nSize = 0; 
    m_pData = 0;
  }

  void GetMemoryUsage(ICrySizer*pSizer)
  {
    pSizer->AddObject (m_pData, m_nSize*m_nSize*sizeof(T));
  }

  void Allocate(int nSize) 
  { 
    if(m_nSize == nSize)
      return;

    delete m_pData;

    m_nSize = nSize; 
    m_pData = new T [nSize*nSize];
    memset(m_pData, 0, nSize*nSize*sizeof(T));
  }

  ~Array2d() 
  { 
    delete [] m_pData;
  }

  T * m_pData;
  int m_nSize;

  T * operator [] (const int & nPos) const
  {
    assert(nPos>=0 && nPos<m_nSize);
    return &m_pData[nPos*m_nSize];
  }

  void operator = (const Array2d & other)
  {
    Allocate(other.m_nSize);
    memcpy(m_pData,other.m_pData,m_nSize*m_nSize*sizeof(T));
  }
};

#endif // _ARRAY2D_H_
