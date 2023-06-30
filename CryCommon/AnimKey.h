////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   animkey.cpp
//  Version:     v1.00
//  Created:     22/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __animkey_h__
#define __animkey_h__

#if _MSC_VER > 1000
#pragma once
#endif

struct ISound;

enum EAnimKeyFlags
{
	//! This key is selected in track view.
	AKEY_SELECTED = 0x01,
};

//! Interface to animation key.
//! Not real interface though...
//! No virtuals for optimization reason.
struct IKey
{
	float time;
	int flags;

	// compare keys.
	bool operator<( const IKey &key ) const { return time < key.time; }
	bool operator==( const IKey &key ) const { return time == key.time; }
	bool operator>( const IKey &key ) const { return time > key.time; }
	bool operator<=( const IKey &key ) const { return time <= key.time; }
	bool operator>=( const IKey &key ) const { return time >= key.time; }
	bool operator!=( const IKey &key ) const { return time != key.time; }

protected:
	//! Protect from direct instantiation of this class.
	//! Only derived classes can be created,
	IKey() :time(0),flags(0) {};
};

/** ITcbKey used in all TCB tracks.
*/
struct ITcbKey : public IKey
{
	// Values.
	float fval[4];
	// Key controls.
	float tens;         //!< Key tension value.
  float cont;         //!< Key continuity value.
  float bias;         //!< Key bias value.
  float easeto;       //!< Key ease to value.
  float easefrom;     //!< Key ease from value.

	//! Protect from direct instantiation of this class.
	//! Only derived classes can be created,
	ITcbKey() {
		fval[0] = 0; fval[1] = 0; fval[2] = 0; fval[3] = 0;
		tens = 0, cont = 0, bias = 0, easeto = 0, easefrom = 0;
	};

	template <class T>
	void SetValue( const T& val ) { *((T*)fval) = val; };
	template <class T>
	void GetValue( T& val ) { val = *((T*)fval); };

	void SetFloat( float val ) { SetValue(val); };
	void SetVec3( const Vec3 &val ) { SetValue(val); };
	void SetQuat( const Quat &val ) { SetValue(val); };
	
	float GetFloat() const { return *((float*)fval); };
	const Vec3& GetVec3() const { return *((Vec3*)fval); };
	const Quat& GetQuat() const { return *((Quat*)fval); };
};

/** IEntityKey used in Entity track.
*/
struct IEventKey : public IKey
{
	char event[64];
	char animation[64];
	float duration;

	IEventKey()
	{
		event[0] = '\0'; // empty string.
		animation[0] = '\0'; // empty string.
		duration = 0;
	}
};

/** ISelectKey used in Camera selection track or Scene node.
*/
struct ISelectKey : public IKey
{
	char szSelection[128];	//!< Node name.
	float fDuration;
	
	ISelectKey()
	{
		fDuration = 0;
		szSelection[0] = '\0'; // empty string.
	}
};

/** ISoundKey used in sound track.
*/
struct ISoundKey : public IKey
{
	char pszFilename[128];
	unsigned char nVolume;
	unsigned char nPan;
	float inRadius;
	float outRadius;
	bool bStream;	//!< Stream sound from disk.
	bool b3DSound; //!< 3D or Stereo sound.
	bool bLoop; //!< Loop sound.
	float fDuration;
	char description[32];

	ISoundKey()
	{
		b3DSound = false;
		inRadius = 10;
		outRadius = 100;
		bStream = false;
		bLoop = false;
		pszFilename[0]=0;
		nVolume=255;
		nPan=127;
		fDuration=0.0f;
		description[0] = 0;
	}
};

/** ICharacterKey used in Character animation track.
*/
struct ICharacterKey : public IKey
{
	char animation[64];	//!< Name of character animation.
	float duration;		//!< Duration in seconds of this animation.
	float startTime;	//!< Start time of this animtion (Offset from begining of animation).
	float blendTime;	//!< Blending time for this animation.
	float speed;			//!< Speed multiplier for this key.
	bool bLoop;				//!< True if animation must be looped.
	bool bUnload;			//!< Unload after sequence is finished

	ICharacterKey()	{
		animation[0] = '\0'; duration = 0; blendTime = 0; startTime = 0; speed = 1;
		bLoop = false;
		bUnload = false;
	}
};

/** IExprKey used in expression animation track.
*/
struct IExprKey : public IKey
{
	IExprKey()
	{
		pszName[0]=0;
		fAmp=1.0f;
		fBlendIn=0.5f;
		fHold=1.0f;
		fBlendOut=0.5f;
	}
	char pszName[128];	//!< Name of morph-target
	float fAmp;
	float fBlendIn;
	float fHold;
	float fBlendOut;
};

/** IConsoleKey used in Console track, triggers console commands and variables.
*/
struct IConsoleKey : public IKey
{
	char command[64];

	IConsoleKey()
	{
		command[0] = '\0';
	}
};

enum EMusicKeyType
{
	eMusicKeyType_SetMood=0,
	eMusicKeyType_VolumeRamp
};

/** IMusicKey used in music track.
*/
struct IMusicKey : public IKey
{
	EMusicKeyType eType;
	char szMood[64];
	float fTime;
	float fDuration;
	char description[32];
	IMusicKey()
	{
		eType=eMusicKeyType_SetMood;
		szMood[0]=0;
		fTime=0.0f;
		fDuration=0.0f;
		description[0]=0;
	}
};

#endif // __animkey_h__
