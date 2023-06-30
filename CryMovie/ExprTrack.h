#pragma once

#include "IMovieSystem.h"
#include "AnimTrack.h"

class CExprTrack : public TAnimTrack<IExprKey>
{
public:
	EAnimTrackType GetType() { return ATRACK_EXPRESSION; };
	EAnimValue GetValueType() { return AVALUE_EXPRESSION; };

	void GetKeyInfo( int key,const char* &description,float &duration );
	void SerializeKey( IExprKey &key,XmlNodeRef &keyNode,bool bLoading );
};
