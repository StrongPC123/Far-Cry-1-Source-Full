////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   bitarray.h
//  Version:     v1.00
//  Created:     29/11/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Array of bits.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __bitarray_h__
#define __bitarray_h__

#if _MSC_VER > 1000
#pragma once
#endif


/*!
 *		
 */
class	CBitArray
{
public:
	CBitArray()	{ m_base = NULL; bits = NULL; m_size = 0; };
	CBitArray( int c )	{ reserve( c ); };
	~CBitArray()	{ if (m_base) free(m_base); };

	void	reserve( int c )	{
		int newSize = ((c+63)&(~63)) >> 5;
		if (newSize > m_size)
			alloc( newSize );
	}
	void	reserve( const CBitArray &b )	{
		alloc( b.m_size );
	}

	void	copy( const CBitArray &b )	{
		if (m_size != b.m_size)	{
			alloc( b.m_size );
		}
		memcpy( bits,b.bits,m_size*sizeof(uint) );
	};
	void	set()	{
		memset( bits,0xFFFFFFFF,m_size*sizeof(uint) );	// Set all bits.
	}
	void	set_num( int numBits )	{
		int num = (numBits >> 3) + 1;
		if (num > (m_size*sizeof(uint))) num = m_size*sizeof(uint);
		memset( bits,0xFFFFFFFF,num );	// Reset num bits.
	}
	void	clear()	{
		memset( bits,0,m_size*sizeof(uint) );	// Reset all bits.
	}
	void	clear_num( int numBits )	{
		int num = (numBits >> 3) + 1;
		if (num > (m_size*sizeof(uint))) num = m_size*sizeof(uint);
		memset( bits,0,num );	// Reset num bits.
	}
	void	set( int pos )	{
		bits[index(pos)] |= shift(pos);
	}
	void	clear( int pos )	{
		bits[index(pos)] &= ~shift(pos);
	}
	void	flip( int pos )	{
		//bits[index(pos)] ^= ~shift(pos);
	}
	int	size() const { return m_size; };

	bool	valid()	{ return m_size > 0; };

	int		count()	const {
		int c = 0;
		for (int i = 0; i < m_size; i++)	{
			uint v = bits[i];
			for (int j = 0; j < 32; j++)	{
				if (v & (1 << (j & 0x1F))) c++;	// if bit set increase bit count.
			}
		}
		return c;
	}
	int	operator[]( int pos )	{
		return bits[index(pos)] & shift(pos);
	}
	bool	checkByte( int pos )	{	return reinterpret_cast<char*>(bits)[pos] != 0; };

	CBitArray&	operator =( const CBitArray &b )	{
		copy( b );
		return *this;
	}

	void	compress( CBitArray &b )	{
		int i,countz,compsize,bsize;
		char *out;
		char *in;

		bsize = m_size*4;
		compsize = 0;
		in = (char*)bits;
		for (i = 0; i < bsize; i++)	{
			compsize++;
			if (in[i] == 0)	{
				countz = 1;
				while (++i < bsize)	{
					if (in[i] == 0 && countz != 255) countz++; else break;
				}
				i--;
				compsize++;
			}
		}
		b.reserve( (compsize+1)<<3 );
		out = (char*)b.bits;
		in = (char*)bits;
		*out++ = bsize;
		for (i = 0; i < bsize; i++)	{
			*out++ = in[i];
			if (in[i] == 0)	{
				countz = 1;
				while (++i < bsize)	{
					if (in[i] == 0 && countz != 255) countz++; else break;
				}
				i--;
				*out++ = countz;
			}
		}
	}
	
	void	decompress( CBitArray &b )	{
		int raw,decompressed,c;
		char *out,*in;

		in = (char*)bits;
		out = (char*)b.bits;
		decompressed = 0;
		raw = *in++;
		while (decompressed < raw)	{
			if (*in != 0)	
			{
				*out++ = *in++;
				decompressed++;
			}	else	{
				in++;
				c = *in++;
				decompressed += c;
				while (c)	{	*out++ = 0; c--; };
			}
		}
	}

	void	copyFromMem( const char *src,int size ) {
		alloc( size );
		memcpy( bits,src,size );
	}
	int		copyToMem( char *trg ) {
		memcpy( trg,bits,m_size );
		return m_size;
	}

private:
	void	*m_base;
	uint	*bits;
	int		m_size;

	void	alloc( int s )	{
		if (m_base) free(m_base);
		m_size = s;
		m_base = (char*)malloc(m_size*sizeof(uint)+32);
		bits = (uint*)( ((UINT_PTR)m_base + 31) & (~31) );	// align by 32.
	}
	unsigned	int	shift( int pos )	{
		return (1 << (pos&0x1F));
	}
	unsigned	int	index( int pos )	{
		return pos >> 5;
	}

	friend	int	concatBitarray( CBitArray &b1,CBitArray &b2,CBitArray &test,CBitArray &res );
};

inline	int	concatBitarray( CBitArray &b1,CBitArray &b2,CBitArray &test,CBitArray &res )
{
	unsigned int b,any;
	any = 0;
	for (int i = 0; i < b1.size(); i++)	{
		b = b1.bits[i] & b2.bits[i];
		any |= (b & (~test.bits[i]));	// test if any different from test(i) bit set.
		res.bits[i] = b;
	}
	return any;
}

#endif // __bitarray_h__