#ifndef _STATICIB_H_
#define _STATICIB_H_

/////////////////////////////
// D. Sim Dietrich Jr.
// sim.dietrich@nvidia.com
//////////////////////

template < class IndexType > class StaticIB
{
  private :

    LPDIRECT3DINDEXBUFFER9 mpIB;

    uint mIndexCount;
    bool    mbLocked;
    IndexType* m_pLockedData;

  public :

    unsigned int GetIndexCount() const 
    { 
      return mIndexCount; 
    }

    StaticIB( const LPDIRECT3DDEVICE9 pD3D, const unsigned int& theIndexCount )
    {
      mpIB = 0;
      mbLocked = false;

      mIndexCount = theIndexCount;
    
      HRESULT hr = pD3D->CreateIndexBuffer( mIndexCount * sizeof( IndexType ), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &mpIB, NULL);

      ASSERT( ( hr == D3D_OK ) && ( mpIB ) );
    }

    LPDIRECT3DINDEXBUFFER9 GetInterface() const { return mpIB; }

    IndexType* Lock( const unsigned int& theLockCount, unsigned int& theStartIndex )
    {
      // Ensure there is enough space in the IB for this data
      ASSERT ( theLockCount <= mIndexCount );

      if  (mbLocked)
        return m_pLockedData;

      if ( mpIB )
      {
        DWORD dwFlags = D3DLOCK_DISCARD;
        DWORD dwSize = 0;

        HRESULT hr = mpIB->Lock( 0, 0, reinterpret_cast<void**>( &m_pLockedData ), dwFlags );

        ASSERT( hr == D3D_OK );
        ASSERT( m_pLockedData != 0 );
        mbLocked = true;
        theStartIndex = 0;
      }

      return m_pLockedData;
    }

    void Unlock()
    {
      if ( ( mbLocked ) && ( mpIB ) )
      {
        HRESULT hr = mpIB->Unlock();        
        ASSERT( hr == D3D_OK );
        mbLocked = false;
      }
    }

    ~StaticIB()
    {
      Unlock();
      if ( mpIB )
      {
        mpIB->Release();
      }
    }
  
};

typedef StaticIB< unsigned short > StaticIB16;
typedef StaticIB< unsigned int > StaticIB32;

#endif  _STATICIB_H_
