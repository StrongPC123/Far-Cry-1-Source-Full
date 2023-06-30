
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bitstream_compressed.h"

static const DWORD g_dwBits=16;				// higher quality angles

//////////////////////////////////////////////////////////////////////////
CBitStream_Compressed::CBitStream_Compressed()
{
}

//////////////////////////////////////////////////////////////////////////
CBitStream_Compressed::~CBitStream_Compressed()
{
}

//////////////////////////////////////////////////////////////////////////
bool CBitStream_Compressed::WriteBitStream( CStream &stm, const uint32 Value, const eBitStreamHint eHint )
{
	switch(eHint)
	{
		case eEntityId:
			{
				WORD word=(WORD)Value;
				if(!stm.WritePkd((int16)word))			// compressed as signed number because negative EntityId are dynamic
					return false;
			}
			return true;

		case eEntityClassId:
			return stm.WritePkd((uint16)Value);
	}

	return stm.Write((unsigned int)Value);
}

bool CBitStream_Compressed::ReadBitStream( CStream &stm, uint32 &Value, const eBitStreamHint eHint )
{
	switch(eHint)
	{
		case eEntityId:
			{
				int16 Val;
				if(!stm.ReadPkd(Val))								// compressed as signed number because negative EntityId are dynamic
					return false;

				uint16 wVal=(uint16)Val;

				Value=(uint32)wVal;
			}
			return true;

		case eEntityClassId:
			{
				uint16 Val;
				if(!stm.ReadPkd(Val))
					return false;
	
				Value=(int32)Val;
			}
			return true;
	}

	return stm.Read((unsigned int&)Value);
}

bool CBitStream_Compressed::WriteBitStream( CStream &stm, const int32 Value, const eBitStreamHint eHint )
{
	return WriteBitStream(stm,(uint32)Value,eHint);
}

bool CBitStream_Compressed::ReadBitStream( CStream &stm, int32 &Value, const eBitStreamHint eHint )
{
	return ReadBitStream(stm,(uint32 &)Value,eHint);
}

bool CBitStream_Compressed::WriteBitStream( CStream &stm, const float Value, const eBitStreamHint eHint )
{
	switch(eHint)
	{
		case eSignedUnitValueLQ:
			{
				assert(Value>=-1.0 && Value<=1.0);				// check valid range
				if(!stm.Write(Value!=0))
					return false;
				if(Value!=0)															// can be done better
				{
					if(!stm.Write(Value>0)) //sign
						return false;
					if(!stm.Write(((BYTE)(fabs(Value)/(1.0f/255)))))
						return false;
				}
			}
			return true;

		default:
			return stm.Write(Value);
	}
}

bool CBitStream_Compressed::ReadBitStream( CStream &stm, float &Value, const eBitStreamHint eHint )
{
	switch(eHint)
	{
		case eSignedUnitValueLQ:
			{
				bool bNotZero;

				if(!stm.Read(bNotZero))
					return false;

				if(bNotZero)
				{
					bool sign;
					BYTE lean;

					if(!stm.Read(sign))
						return false;
					if(!stm.Read(lean))
						return false;
					Value=sign?(lean*(1.0f/255)):-(lean*(1.0f/255));
				}
				else Value=0.0f;
			}
			return true;

		default:
			return stm.Read(Value);
	}
}

bool CBitStream_Compressed::WriteBitStream( CStream &stm, const Vec3 &Value, const eBitStreamHint eHint )
{
	switch(eHint)
	{
		case eEulerAnglesHQ:
			{
				const float fScale=(1<<g_dwBits)/360.0f;

				DWORD X=(DWORD)(Value.x*fScale+0.5f);		// convert
				DWORD Y=(DWORD)(Value.y*fScale+0.5f);
				DWORD Z=(DWORD)(Value.z*fScale+0.5f);

				DWORD dwX=(X)&((1<<g_dwBits)-1);		// mask
				DWORD dwY=(Y)&((1<<g_dwBits)-1);
				DWORD dwZ=(Z)&((1<<g_dwBits)-1);

				if(!stm.WriteNumberInBits((unsigned int) dwX,g_dwBits))
					return false;

				if(dwY==0)
				{
					if(!stm.Write(false))	// bNotZero
						return false;
				}
				else
        {
					if(!stm.Write(true))	// bNotZero
						return false;

					if(!stm.WriteNumberInBits((unsigned int) dwY,g_dwBits))
						return false;
				}

				if(!stm.WriteNumberInBits((unsigned int) dwZ,g_dwBits))
					return false;
			}
			return true;

		default:
			return stm.Write(Value);
	}
}

bool CBitStream_Compressed::ReadBitStream( CStream &stm, Vec3 &Value, const eBitStreamHint eHint )
{
	switch(eHint)
	{
		case eEulerAnglesHQ:
			{
				const float fScale=360.0f/(1<<g_dwBits);

				DWORD dwX,dwY,dwZ;
			
				if(!stm.ReadNumberInBits((unsigned int&) dwX,g_dwBits))
					return false;

				bool bNotZero;
				
				if(!stm.Read(bNotZero))
					return false;

				if(bNotZero)
				{
					if(!stm.ReadNumberInBits((unsigned int&) dwY,g_dwBits))
						return false;
				}
				else dwY=0;

				if(!stm.ReadNumberInBits((unsigned int&) dwZ,g_dwBits))
					return false;

				Value = Vec3(((float)dwX-0.5f)*fScale,((float)dwY-0.5f)*fScale,((float)dwZ-0.5f)*fScale);		// convert
				return true;
			}
			break;

		default:
			return stm.Read(Value);
	}
}

bool CBitStream_Compressed::WriteBitStream( CStream &stm, const uint16 Value, const eBitStreamHint eHint )
{
	switch(eHint)
	{
		case eEntityId:
			{
				WORD word=(WORD)Value;
				if(!stm.WritePkd((int16)word))			// compressed as signed number because negative EntityId are dynamic
					return false;
			}
			return true;

		case eEntityClassId:
			return stm.WritePkd((uint16)Value);
	}

	return stm.Write(Value);
}

bool CBitStream_Compressed::ReadBitStream( CStream &stm, uint16 &Value, const eBitStreamHint eHint )
{
	switch(eHint)
	{
		case eEntityId:
			{
				int16 Val;
				if(!stm.ReadPkd(Val))								// compressed as signed number because negative EntityId are dynamic
					return false;

				Value=(uint16)Val;
			}
			return true;

		case eEntityClassId:
			{
				uint16 Val;
				if(!stm.ReadPkd(Val))
					return false;
	
				Value=(int16)Val;
			}
			return true;
	}

	return stm.Read(Value);
}
