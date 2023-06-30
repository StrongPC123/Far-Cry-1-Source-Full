
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef BITSTREAM_COMPRESSED_H
#define BITSTREAM_COMPRESSED_H

#include "bitstream_base.h"				// CBitStream_Base

class CBitStream_Compressed :public CBitStream_Base
{
public:
	//! constructor
	CBitStream_Compressed();
	//! destructor
	virtual ~CBitStream_Compressed();

	// interface IBitSubStream -----------------------------------------------

  virtual bool WriteBitStream( CStream &stm, const uint32 Value, const eBitStreamHint eHint );
	virtual bool ReadBitStream( CStream &stm, uint32 &Value, const eBitStreamHint eHint );

	virtual bool WriteBitStream( CStream &stm, const int32 Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, int32 &Value, const eBitStreamHint eHint );

  virtual bool WriteBitStream( CStream &stm, const float Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, float &Value, const eBitStreamHint eHint );

	virtual bool WriteBitStream( CStream &stm, const Vec3 &Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, Vec3 &Value, const eBitStreamHint eHint );

	virtual bool WriteBitStream( CStream &stm, const uint16 Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, uint16 &Value, const eBitStreamHint eHint );
};


#endif // BITSTREAM_COMPRESSED_H