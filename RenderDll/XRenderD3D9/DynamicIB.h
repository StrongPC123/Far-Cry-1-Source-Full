#ifndef _DynamicIB_H_
#define _DynamicIB_H_

template <class Type> class DynamicIB 
{
  private :

    LPDIRECT3DINDEXBUFFER9 m_pIB;
    uint m_Count;
    int m_nOffs;
    bool m_bLocked;
    Type* m_pLockedData;

  public :

    uint GetCount() const 
    { 
      return m_Count; 
    }
    int GetOffset() const 
    { 
      return m_nOffs; 
    }
    void Reset()
    {
      m_nOffs = 0;
    }

    DynamicIB(const LPDIRECT3DDEVICE9 pD3D, const unsigned int& theElementsCount)
    {
      m_pIB = 0;

      m_bLocked = false;
      m_pLockedData = NULL;
      m_nOffs = 0;

      m_Count = theElementsCount;
    
      HRESULT hr = pD3D->CreateIndexBuffer(theElementsCount*sizeof(Type), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, sizeof(Type)==2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &m_pIB, NULL);
      assert((hr == D3D_OK) && (m_pIB));
    }

    LPDIRECT3DVERTEXBUFFER9 GetInterface() const { return m_pIB; }

    Type* Lock(const unsigned int& theLockCount, int &nOffs)
    {
      if (theLockCount > m_Count)
      {
        assert(false);
        return NULL;
      }

      if (m_bLocked)
        return m_pLockedData;

      HRESULT hr;
      if ( m_pIB )
      {
        if (theLockCount+m_nOffs > m_Count)
        {
          hr = m_pIB->Lock(0, theLockCount*sizeof(Type), (void **) &m_pLockedData, D3DLOCK_DISCARD);
          nOffs = 0;
          m_nOffs = theLockCount;
        }
        else
        {
          hr = m_pIB->Lock(m_nOffs*sizeof(Type), theLockCount*sizeof(Type), (void **) &m_pLockedData, D3DLOCK_NOOVERWRITE);
          nOffs = m_nOffs;
          m_nOffs += theLockCount;
        }
        assert(m_pLockedData != NULL);
        m_bLocked = true;
      }

      return m_pLockedData;
    }

    void Unlock()
    {
      if ((m_bLocked) && (m_pIB))
      {
        HRESULT hr = m_pIB->Unlock();        
        assert( hr == D3D_OK );
        m_bLocked = false;
      }
    }

    HRESULT Bind(const LPDIRECT3DDEVICE9 pD3D)
    {
      return pD3D->SetIndices(m_pIB);
    }

    ~DynamicIB()
    {
      Unlock();
      SAFE_RELEASE(m_pIB);
    }
  
};

#endif  _DynamicIB_H_
