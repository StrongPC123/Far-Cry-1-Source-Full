#pragma once

#include "IMovieSystem.h"
#include "AnimTrack.h"

struct SSoundInfo
{
	int nLastKey;
	string sLastFilename;
	_smart_ptr<ISound> pSound;
	int nLength;
};

class CSoundTrack : public TAnimTrack<ISoundKey>
{
public:
	EAnimTrackType GetType() { return ATRACK_SOUND; };
	EAnimValue GetValueType() { return AVALUE_SOUND; };

	void GetKeyInfo( int key,const char* &description,float &duration );
	void SerializeKey( ISoundKey &key,XmlNodeRef &keyNode,bool bLoading );
};
