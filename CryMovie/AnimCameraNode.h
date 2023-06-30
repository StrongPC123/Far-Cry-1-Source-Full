////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   animcameranode.h
//  Version:     v1.00
//  Created:     16/8/2002 by Lennert.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __animcameranode_h__
#define __animcameranode_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "entitynode.h"

/** Camera node controls camera entity.
*/
class CAnimCameraNode : public CAnimEntityNode
{
private:
	IMovieSystem *m_pMovie;
	// Field of view in DEGREES! To Display it nicely for user.
	float m_fFOV;
public:
	CAnimCameraNode(IMovieSystem *sys );
	virtual ~CAnimCameraNode();
	virtual EAnimNodeType GetType() const { return ANODE_CAMERA; }
	virtual void Animate( SAnimContext &ec );
	virtual void CreateDefaultTracks();
	virtual void Reset();
	float GetFOV() { return m_fFOV; }

	//////////////////////////////////////////////////////////////////////////
	int GetParamCount() const;
	bool GetParamInfo( int nIndex, SParamInfo &info ) const;
	bool GetParamInfoFromId( int paramId, SParamInfo &info ) const;

	//////////////////////////////////////////////////////////////////////////
	bool SetParamValue( float time,AnimParamType param,float value );
	bool GetParamValue( float time,AnimParamType param,float &value );

	IAnimTrack* CreateTrack(AnimParamType nParamType);
	void Serialize( XmlNodeRef &xmlNode,bool bLoading );
};

#endif // __animcameranode_h__