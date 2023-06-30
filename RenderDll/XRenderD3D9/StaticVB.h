#ifndef _StaticVB_H_
#define _StaticVB_H_

template <class VertexType> class StaticVB 
{
  private :

    LPDIRECT3DVERTEXBUFFER9 mpVB;
    uint mVertexCount;
    bool    mbLocked;
    VertexType* m_pLockedData;

  public :

    uint GetVertexCount() const 
    { 
      return mVertexCount; 
    }

    StaticVB( const LPDIRECT3DDEVICE9 pD3D, const DWORD& theFVF, const unsigned int& theVertexCount )
    {
      mpVB = 0;

      mbLocked = false;
      m_pLockedData = NULL;

      mVertexCount = theVertexCount;
    
      HRESULT hr = pD3D->CreateVertexBuffer( mVertexCount * sizeof( VertexType ), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, theFVF, D3DPOOL_DEFAULT, &mpVB, NULL);
      ASSERT( ( hr == D3D_OK ) && ( mpVB ) );
    }

    LPDIRECT3DVERTEXBUFFER9 GetInterface() const { return mpVB; }

    VertexType* Lock( const unsigned int& theLockCount, unsigned int& theStartVertex )
    {
      theStartVertex = 0;

      // Ensure there is enough space in the VB for this data
      ASSERT ( theLockCount <= mVertexCount );

      if (mbLocked)
        return m_pLockedData;

      if ( mpVB )
      {
        DWORD dwFlags = D3DLOCK_DISCARD;
        DWORD dwSize = 0;

        HRESULT hr = mpVB->Lock( 0, 0, reinterpret_cast<void**>( &m_pLockedData ), dwFlags );

        ASSERT( hr == D3D_OK );
        ASSERT( m_pLockedData != 0 );
        mbLocked = true;
      }

      return m_pLockedData;
    }

    void Unlock()
    {
      if ( ( mbLocked ) && ( mpVB ) )
      {
        HRESULT hr = mpVB->Unlock();        
        ASSERT( hr == D3D_OK );
        mbLocked = false;
      }
    }

    ~StaticVB()
    {
      Unlock();
      if ( mpVB )
      {
        mpVB->Release();
      }
    }
  
};

#endif  _StaticVB_H_
