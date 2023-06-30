
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bitstream_base.h"

CBitStream_Base::CBitStream_Base()
{
}

CBitStream_Base::~CBitStream_Base()
{
}


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------


bool CBitStream_Base::WriteBitStream( CStream &stm, const int8 Value, const eBitStreamHint eHint )
{
	return stm.Write(Value);
}

bool CBitStream_Base::ReadBitStream( CStream &stm, int8 &Value, const eBitStreamHint eHint )
{
	return stm.Read((char&)Value);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CBitStream_Base::WriteBitStream( CStream &stm, const int16 Value, const eBitStreamHint eHint )
{
	return stm.Write(Value);
}

bool CBitStream_Base::ReadBitStream( CStream &stm, int16 &Value, const eBitStreamHint eHint )
{
	return stm.Read(Value);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CBitStream_Base::WriteBitStream( CStream &stm, const int32 Value, const eBitStreamHint eHint )
{
	return stm.Write((unsigned int)Value);
}

bool CBitStream_Base::ReadBitStream( CStream &stm, int32 &Value, const eBitStreamHint eHint )
{
	return stm.Read((int&)Value);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CBitStream_Base::WriteBitStream( CStream &stm, const uint8 Value, const eBitStreamHint eHint )
{
	return stm.Write(Value);
}

bool CBitStream_Base::ReadBitStream( CStream &stm, uint8 &Value, const eBitStreamHint eHint )
{
	return stm.Read(Value);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CBitStream_Base::WriteBitStream( CStream &stm, const uint16 Value, const eBitStreamHint eHint )
{
	return stm.Write(Value);
}

bool CBitStream_Base::ReadBitStream( CStream &stm, uint16 &Value, const eBitStreamHint eHint )
{
	return stm.Read(Value);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CBitStream_Base::WriteBitStream( CStream &stm, const uint32 Value, const eBitStreamHint eHint )
{
	return stm.Write((unsigned int)Value);
}

bool CBitStream_Base::ReadBitStream( CStream &stm, uint32 &Value, const eBitStreamHint eHint )
{
	return stm.Read((unsigned int&)Value);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CBitStream_Base::WriteBitStream( CStream &stm, const float Value, const eBitStreamHint eHint )
{
	return stm.Write(Value);
}

bool CBitStream_Base::ReadBitStream( CStream &stm, float &Value, const eBitStreamHint eHint )
{
	return stm.Read(Value);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CBitStream_Base::WriteBitStream( CStream &stm, const Vec3 &Value, const eBitStreamHint eHint )
{
	return stm.Write(Value);
}

bool CBitStream_Base::ReadBitStream( CStream &stm, Vec3 &Value, const eBitStreamHint eHint )
{
	return stm.Read(Value);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

bool CBitStream_Base::WriteBitStream( CStream &stm, const char *Value, const DWORD nBufferSize, const eBitStreamHint eHint )
{
	assert(strlen(Value)<nBufferSize);
	return stm.Write(Value);
}

bool CBitStream_Base::ReadBitStream( CStream &stm, char *Value, const DWORD nBufferSize, const eBitStreamHint eHint )
{
	return stm.Read(Value,nBufferSize);
}


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

void CBitStream_Base::SimulateWriteRead( int8 &Value, const eBitStreamHint eHint )
{
	CStream stm; 

	WriteBitStream(stm,Value,eHint);
	ReadBitStream(stm,Value,eHint);
}

void CBitStream_Base::SimulateWriteRead( int16 &Value, const eBitStreamHint eHint )
{
	CStream stm; 

	WriteBitStream(stm,Value,eHint);
	ReadBitStream(stm,Value,eHint);
}

void CBitStream_Base::SimulateWriteRead( int32 &Value, const eBitStreamHint eHint )
{
	CStream stm; 

	WriteBitStream(stm,Value,eHint);
	ReadBitStream(stm,Value,eHint);
}

void CBitStream_Base::SimulateWriteRead( uint8 &Value, const eBitStreamHint eHint )
{
	CStream stm; 

	WriteBitStream(stm,Value,eHint);
	ReadBitStream(stm,Value,eHint);
}

void CBitStream_Base::SimulateWriteRead( uint16 &Value, const eBitStreamHint eHint )
{
	CStream stm; 

	WriteBitStream(stm,Value,eHint);
	ReadBitStream(stm,Value,eHint);
}

void CBitStream_Base::SimulateWriteRead( uint32 &Value, const eBitStreamHint eHint )
{
	CStream stm; 

	WriteBitStream(stm,Value,eHint);
	ReadBitStream(stm,Value,eHint);
}

void CBitStream_Base::SimulateWriteRead( float &Value, const eBitStreamHint eHint )
{
	CStream stm; 

	WriteBitStream(stm,Value,eHint);
	ReadBitStream(stm,Value,eHint);
}

void CBitStream_Base::SimulateWriteRead( Vec3 &Value, const eBitStreamHint eHint )
{
	CStream stm; 

	WriteBitStream(stm,Value,eHint);
	ReadBitStream(stm,Value,eHint);
}

void CBitStream_Base::SimulateWriteRead( char *Value, const DWORD nBufferSize, const eBitStreamHint eHint )
{
	CStream stm; 

	WriteBitStream(stm,Value,nBufferSize,eHint);
	ReadBitStream(stm,Value,nBufferSize,eHint);
}
