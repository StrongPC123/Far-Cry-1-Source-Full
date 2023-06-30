#ifndef _ANIMATION_LAYER_INFO_HDR_
#define _ANIMATION_LAYER_INFO_HDR_

// This is used to pass the information about animation that needs to be blended between
// several animations to the bone that calculates the actual position / rotation out of it
struct CAnimationLayerInfo
{
	int nAnimId;
	float fTime; // this is time suitable for passing to controllers, i.e. ticks 
	float fBlending;
	CAnimationLayerInfo (int animid, float time, float blending): fBlending(blending), fTime(time), nAnimId(animid) {}
	CAnimationLayerInfo(){}
};

typedef std::vector<CAnimationLayerInfo> CAnimationLayerInfoArray;
/*
// This is used to pass the information about morphs that need to be blended
struct CMorphLayerInfo
{

};
*/
#endif