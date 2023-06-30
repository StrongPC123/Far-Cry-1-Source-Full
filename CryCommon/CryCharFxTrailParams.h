#ifndef _CRY_COMMON_CRY_CHAR_FX_TRAIL_PARAMS_HDR_
#define _CRY_COMMON_CRY_CHAR_FX_TRAIL_PARAMS_HDR_

struct CryCharFxTrailParams
{
	CryCharFxTrailParams(){}

	CryCharFxTrailParams (int nBone, int nTextureId = -1, float fLength = 0.25f, const Vec3d& vStart = Vec3d(0,0,0), const Vec3d& vEnd = Vec3d(0,0,1), unsigned numMaxQuads = 24)
	{
		this->nBone      = nBone;
		this->nTextureId = nTextureId;
		this->vLine[0]   = vStart;
		this->vLine[1]   = vEnd;
		this->fLength    = fLength;
		this->numMaxQuads = numMaxQuads;
	}

	int nBone;			        // bone to which to attach the trail
	int nTextureId;         // tid of the trail
	enum {numVerts = 2};    // number of vertices in the line strip
	Vec3d vLine[numVerts];  // the line strip to extrude, in the coordinate frame of the bone
	float fLength;          // length of the trail, in SECONDS
	unsigned numMaxQuads;   // max number of quads constituting the strape
};

#endif