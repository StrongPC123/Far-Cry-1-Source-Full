////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Image.h
//  Version:     v1.00
//  Created:     14/11/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: General Image
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Image_h__
#define __Image_h__

#if _MSC_VER > 1000
#pragma once
#endif

class CXmlArchive;

/*!
 *  Templated image class.
 */

template <class T>
class TImage
{
public:
	TImage()
	{
		m_data = 0;
		m_width = 0;
		m_height = 0;
	}
	virtual ~TImage() {}

	T&	ValueAt( int x,int y ) { return m_data[x+y*m_width]; };
	const T& ValueAt( int x,int y ) const { return m_data[x+y*m_width]; };

	T*	GetData() const { return m_data; };
	int GetWidth() const { return m_width; };
	int GetHeight() const { return m_height; };
	int GetSize() const { return m_width*m_height*sizeof(T); };

	bool IsValid() const { return m_data != 0; };

	void Attach( T* data,int width,int height )
	{
		assert( data );
		m_memory = new CMemoryBlock;
		m_memory->Attach( data,width*height*sizeof(T) );
		m_data = data;
		m_width = width;
		m_height = height;
	}
	void Attach( const TImage<T> &img )
	{
		assert( img.IsValid() );
		m_memory = img.m_memory;
		m_data = (T*)m_memory->GetBuffer();
		m_width = img.m_width;
		m_height = img.m_height;
	}
	void Detach()
	{
		m_memory = 0;
		m_data = 0;
		m_width = 0;
		m_height = 0;
	}

	bool Allocate( int width,int height )
	{
		// New memory block.
		m_memory = new CMemoryBlock;
		m_memory->Allocate( width*height*sizeof(T) ); // +width for crash safety.
		m_data = (T*)m_memory->GetBuffer();
		m_width = width;
		m_height = height;
		if (!m_data)
			return false;
		return true;
	}

	void Release()
	{
		m_memory = 0;
		m_data = 0;
		m_width = 0;
		m_height = 0;
	}

	// Copy operator.
	void Copy( const TImage<T> &img )
	{
		if (!img.IsValid())
			return;
		if (m_width != img.GetWidth() || m_height != img.GetHeight())
			Allocate( img.GetWidth(),img.GetHeight() );
		*m_memory = *img.m_memory;
		m_data = (T*)m_memory->GetBuffer();
	}

	//////////////////////////////////////////////////////////////////////////
	void Clear()
	{
		Fill(0);
	}

	//////////////////////////////////////////////////////////////////////////
	void Fill( unsigned char c )
	{
		if (IsValid())
			memset( GetData(),c,GetSize() );
	}

	//////////////////////////////////////////////////////////////////////////
	void GetSubImage( int x1,int y1,int width,int height,TImage<T> &img )
	{
		int size = width*height;
		img.Allocate( width,height );
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				img.ValueAt(x,y) = ValueAt(x1+x,y1+y);
			}
		}
	}
	void SetSubImage( int x1,int y1,const TImage<T> &subImage )
	{
		int width = subImage.GetWidth();
		int height = subImage.GetHeight();
		if (x1 < 0)
		{
			width = width + x1;
			x1 = 0;
		}
		if (y1 < 0)
		{
			height = height + y1;
			y1 = 0;
		}
		if (x1+width > m_width)
			width = m_width - x1;
		if (y1+height > m_height)
			height = m_height - y1;
		if (width <= 0 || height <= 0)
			return;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				ValueAt(x1+x,y1+y) = subImage.ValueAt(x,y);
			}
		}
	}

	//! Compress image to memory block.
	void Compress( CMemoryBlock &mem ) const
	{
		assert( IsValid() );
		m_memory->Compress(mem);
	}

	//! Uncompress image from memory block.
	bool Uncompress( const CMemoryBlock &mem )
	{
		assert( IsValid() );
		// New memory block.
		TSmartPtr<CMemoryBlock> temp = new CMemoryBlock;
		mem.Uncompress( *temp );
		bool bValid = (GetSize() == m_memory->GetSize())
						|| ((GetSize()+m_width*sizeof(T)) == m_memory->GetSize());
		if (bValid)
		{
			m_memory = temp;
			m_data = (T*)m_memory->GetBuffer();
		}
		return bValid;
		//assert( GetSize() == m_memory.GetSize() );
	}

	void Serialize( CXmlArchive &ar );

private:
	// Restrict use of copy constructor.
	TImage( const TImage<T> &img ) {};
	TImage<T>& operator=( const TImage<T> &img ) {};


	//! Memory holding image data.
	TSmartPtr<CMemoryBlock> m_memory;

	T*	m_data;
	int m_width;
	int m_height;
};

template <class T>
void	TImage<T>::Serialize( CXmlArchive &ar )
{
	if (ar.bLoading)
	{
		// Loading
		ar.root->getAttr( "ImageWidth",m_width );
		ar.root->getAttr( "ImageHeight",m_height );
		m_data = Allocate( m_width,m_height );
		void *pData = 0;
		int nDataSize = 0
		bool bHaveBlock = ar.pNamedData->GetDataBlock( ar.root->getTag(),pData,nDataSize );
		if (bHaveBlock && nDataSize == GetSize())
		{
			m_data = (T*)pData;
		}
	}
	else
	{
		// Saving.
		ar.root->setAttr( "ImageWidth",m_width );
		ar.root->setAttr( "ImageHeight",m_height );

		ar.pNamedData->AddDataBlock( ar.root->getTag(),(void*)m_data,GetSize() );
	}
};

//////////////////////////////////////////////////////////////////////////
class CImage : public TImage<unsigned int>
{
public:
	bool LoadGrayscale16Tiff( const CString &file );
	bool SaveGrayscale16Tiff( const CString &file );

	void SwapRedAndBlue();
};

//////////////////////////////////////////////////////////////////////////
// Define types of most commonly used images.
//////////////////////////////////////////////////////////////////////////
typedef TImage<bool> CBoolImage;
typedef TImage<float> CFloatImage;
typedef TImage<unsigned char> CByteImage;
typedef TImage<unsigned short> CWordImage;
typedef TImage<unsigned int> CIntImage;

#endif // __Image_h__
