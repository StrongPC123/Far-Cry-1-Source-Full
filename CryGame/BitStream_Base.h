
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef BITSTREAM_BASE_H
#define BITSTREAM_BASE_H

#include "ibitstream.h"

class CBitStream_Base :public IBitStream
{
public:
	CBitStream_Base();
	virtual ~CBitStream_Base();

	// interface IBitSubStream -----------------------------------------------

	virtual bool WriteBitStream( CStream &stm, const int8 Value, const eBitStreamHint eHint );
  virtual bool WriteBitStream( CStream &stm, const int16 Value, const eBitStreamHint eHint );
  virtual bool WriteBitStream( CStream &stm, const int32 Value, const eBitStreamHint eHint );
	virtual bool WriteBitStream( CStream &stm, const uint8 Value, const eBitStreamHint eHint );
  virtual bool WriteBitStream( CStream &stm, const uint16 Value, const eBitStreamHint eHint );
  virtual bool WriteBitStream( CStream &stm, const uint32 Value, const eBitStreamHint eHint );
  virtual bool WriteBitStream( CStream &stm, const float Value, const eBitStreamHint eHint );
  virtual bool WriteBitStream( CStream &stm, const Vec3 &Value, const eBitStreamHint eHint );
  virtual bool WriteBitStream( CStream &stm, const char *Value, const DWORD nBufferSize, const eBitStreamHint eHint );

	virtual bool ReadBitStream( CStream &stm, int8 &Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, int16 &Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, int32 &Value, const eBitStreamHint eHint );
	virtual bool ReadBitStream( CStream &stm, uint8 &Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, uint16 &Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, uint32 &Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, float &Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, Vec3 &Value, const eBitStreamHint eHint );
  virtual bool ReadBitStream( CStream &stm, char *Value, const DWORD nBufferSize, const eBitStreamHint eHint );

	virtual void SimulateWriteRead( int8 &Value, const eBitStreamHint eHint );
  virtual void SimulateWriteRead( int16 &Value, const eBitStreamHint eHint );
  virtual void SimulateWriteRead( int32 &Value, const eBitStreamHint eHint );
	virtual void SimulateWriteRead( uint8 &Value, const eBitStreamHint eHint );
  virtual void SimulateWriteRead( uint16 &Value, const eBitStreamHint eHint );
  virtual void SimulateWriteRead( uint32 &Value, const eBitStreamHint eHint );
  virtual void SimulateWriteRead( float &Value, const eBitStreamHint eHint );
	virtual void SimulateWriteRead( Vec3 &Value, const eBitStreamHint eHint );
  virtual void SimulateWriteRead( char *Value, const DWORD nBufferSize, const eBitStreamHint eHint );
};


#endif // BITSTREAM_BASE_H