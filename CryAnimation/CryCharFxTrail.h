#ifndef _CRY_ANIMATION_CRY_CHAR_FX_TRAIL_HDR_
#define _CRY_ANIMATION_CRY_CHAR_FX_TRAIL_HDR_

#include <CryCharFxTrailParams.h>
#include "CryCharRenderElement.h"

// SFX : sword trail
// The trail is composed of quads, which are formed with an index buffer referring to
// "extrusion lines" in the vertex buffer. Extrusion line is (e.g. 2) vertices describing
// the sword line in one of the previous frames. All this forms a circular queue of entries
// Each entry has:
//   2(currently) vertices in the vertex buffer
//   1 float in the time history
//
//  The circular queue moves BACKward (the head is circularly decremented)
class CryCharFxTrail: public ICryCharFxTrail
{
public:
	CryCharFxTrail(class CryModelState* pState, const CryCharFxTrailParams& params);
	~CryCharFxTrail();

	//! Renderer calls this function to allow update the video vertex buffers right before the rendering
	void ProcessSkinning (const Vec3& t, const Matrix44& mtxModel, int nTemplate, int nLod=-1, bool bForceUpdate=false);

	// returns true if the given submesh is visible
	bool IsVisible() {return m_bVisible;}

	// depending on bVisible, either makes the submesh visible or invisible
	void SetVisible(bool bVisible = true) {m_bVisible = bVisible;}

	// adds the render data to the renderer, so that the current decals can be rendered
	void AddRenderData (CCObject *pObj, const SRendParams & rRendParams);

	void Render (const struct SRendParams & RendParams, Matrix44& mtxObjMatrix, struct CryCharInstanceRenderParams& rCharParams);

	// returns the memory usage by this object into the sizer
	// TODO: use
	void GetMemoryUsage (ICrySizer* pSizer);

	// gets called upon Deform() of the model state
	void Deform (const Matrix44* pBones);
protected:

	// updates the system vertex buffer and the queue of entries/times
	void UpdateEntries(const Matrix44* pBones);
	void UpdateIndices();

	// returns the tail entry, return value is invalid if m_numEntries  == 0
	unsigned getTailEntry() const {return (m_nHeadEntry + m_numEntries - 1) % numMaxEntries();}

	// max number of history entries
	unsigned numMaxEntries() const {return m_Params.numMaxQuads+1;}

	unsigned numVertices()const {return m_Params.numVerts * numMaxEntries();}

protected:
	CryCharFxTrailParams m_Params;
	bool m_bVisible;
	CryCharRenderElement m_RE;

	// the length of the trail history in the system vertex buffer
	// (number of vertices per extrusion line) * (length of history) = (total number of vertices in sysbuffer)
	// currently , only 2 verts per extrusion line are supported
	unsigned m_numEntries;
	
	// the starting entry in the history queue
	unsigned m_nHeadEntry;

	// last frame when the deform was called
	int m_nLastFrameDeform;

	// the shader of the trail
	IShader *m_pShader;

	// array [maxEntries] of times of each entry forming
	TElementaryArray<float> m_pTimes;

	CryModelState* m_pParent;
};


TYPEDEF_AUTOPTR(CryCharFxTrail);

#endif