#ifndef _DynamicVB_H_
#define _DynamicVB_H_

template <class VertexType> class DynamicVB 
{
  private :

    LPDIRECT3DVERTEXBUFFER9 m_pVB;
    uint m_BytesCount;
    int m_nBytesOffs;
    bool m_bLocked;
    VertexType* m_pLockedData;

  public :

    uint GetBytesCount() const 
    { 
      return m_BytesCount; 
    }
    int GetBytesOffset() const 
    { 
      return m_nBytesOffs; 
    }
    void Reset()
    {
      m_nBytesOffs = m_BytesCount;
    }

    DynamicVB(const LPDIRECT3DDEVICE9 pD3D, const DWORD& theFVF, const unsigned int& theVertsCount )
    {
      m_pVB = 0;

      m_bLocked = false;
      m_pLockedData = NULL;
      m_nBytesOffs = 0;

      m_BytesCount = theVertsCount * sizeof(VertexType);
    
      HRESULT hr = pD3D->CreateVertexBuffer(m_BytesCount, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, theFVF, D3DPOOL_DEFAULT, &m_pVB, NULL);
      assert((hr == D3D_OK) && (m_pVB));
    }

    LPDIRECT3DVERTEXBUFFER9 GetInterface() const { return m_pVB; }

    VertexType* Lock(const unsigned int& theLockBytesCount, int &nOffs)
    {
      if (theLockBytesCount > m_BytesCount)
      {
        assert(false);
        return NULL;
      }

      if (m_bLocked)
        return m_pLockedData;

      HRESULT hr;
      if ( m_pVB )
      {
        if (theLockBytesCount+m_nBytesOffs > m_BytesCount)
        {
          hr = m_pVB->Lock(0, theLockBytesCount, (void **) &m_pLockedData, D3DLOCK_DISCARD);
          nOffs = 0;
          m_nBytesOffs = theLockBytesCount;
        }
        else
        {
          hr = m_pVB->Lock(m_nBytesOffs, theLockBytesCount, (void **) &m_pLockedData, D3DLOCK_NOOVERWRITE);
          nOffs = m_nBytesOffs;
          m_nBytesOffs += theLockBytesCount;
        }
        assert(m_pLockedData != NULL);
        m_bLocked = true;
      }

      return m_pLockedData;
    }

    void Unlock()
    {
      if ((m_bLocked) && (m_pVB))
      {
        HRESULT hr = m_pVB->Unlock();        
        assert( hr == D3D_OK );
        m_bLocked = false;
      }
    }

    HRESULT Bind(const LPDIRECT3DDEVICE9 pD3D, uint StreamNumber, int nBytesOffset, int Stride)
    {
      return pD3D->SetStreamSource(StreamNumber, m_pVB, nBytesOffset, Stride);
    }

    ~DynamicVB()
    {
      Unlock();
      SAFE_RELEASE(m_pVB);
    }
  
};

#endif  _DynamicVB_H_
