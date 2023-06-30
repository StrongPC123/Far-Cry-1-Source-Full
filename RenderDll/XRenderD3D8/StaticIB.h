#ifndef _STATICIB_H_
#define _STATICIB_H_

/////////////////////////////
// D. Sim Dietrich Jr.
// sim.dietrich@nvidia.com
//////////////////////

template < class IndexType > class StaticIB
{
  private :

    LPDIRECT3DINDEXBUFFER8 mpIB;

    uint mIndexCount;
    bool    mbLocked;

  public :

    unsigned int GetIndexCount() const 
    { 
      return mIndexCount; 
    }

    StaticIB( const LPDIRECT3DDEVICE8 pD3D, const unsigned int& theIndexCount )
    {
      mpIB = 0;
      mbLocked = false;

      mIndexCount = theIndexCount;
    
      HRESULT hr = pD3D->CreateIndexBuffer( mIndexCount * sizeof( IndexType ), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mpIB);

      ASSERT( ( hr == D3D_OK ) && ( mpIB ) );
    }

    LPDIRECT3DINDEXBUFFER8 GetInterface() const { return mpIB; }

    IndexType* Lock( const unsigned int& theLockCount, unsigned int& theStartIndex )
    {
      IndexType* pLockedData = 0;

      // Ensure there is enough space in the IB for this data
      ASSERT ( theLockCount <= mIndexCount )
      if (mbLocked)
        Unlock();

      if ( mpIB )
      {
        DWORD dwFlags = 0;
        DWORD dwSize = 0;

        HRESULT hr = mpIB->Lock( 0, 0, reinterpret_cast< BYTE** >( &pLockedData ), dwFlags );

        ASSERT( hr == D3D_OK );
        ASSERT( pLockedData != 0 );
        mbLocked = true;
        theStartIndex = 0;
      }

      return pLockedData;
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
