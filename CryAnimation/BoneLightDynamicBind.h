#ifndef _BONE_LIGHT_DYNAMIC_BIND_HDR_
#define _BONE_LIGHT_DYNAMIC_BIND_HDR_

//////////////////////////////////////////////////////////////////////////
// this is used to bind the light sources dynamically.
// it can update the light source's position and rotation
// based on the relative position and rotation and the bone matrix
class CBoneLightDynamicBind
{
public:

	// returns the index of the bone to which the light source is bound
	unsigned getBone() const {return m_nBone;}

	// initializes this object out of the given DLight structure and bone index
	// this is one-time call that is only required for construction out of the DLight
	// to initialize the constant parameters
	void init (ICryCharInstance* pParent, CDLight* pDLight, unsigned nBone, const Matrix44& matBone, bool bCopyLight);
	// deletes the light object, if needed
	void done();

	// per-frame update of the DLight structure. Updates the light position and radius
	void updateDLight (const Matrix44& matBone, const Matrix44& matModel, float fRadiusMultiplier);

	void debugDraw (const Matrix44& matBone, const Matrix44& matModel);

	// returns true if this light source is local (affects only the character)
	bool isLocal()const;

	bool isHeatSource()const;
	bool isLightSource()const;

	CDLight* getDLight() {return m_pDLight;}

protected:
	CDLight* m_pDLight;
	// the bone to which this light is attached
	unsigned m_nBone;
	// the original DLight radius
	float m_fRadius;
	unsigned m_nDLightFlags;
	// the matrix in bone's coordinates
	Matrix44 m_matLight; // transform matrix of the light in the bone CS
	// should we delete the light upon final destruction of this object?
	bool m_bDeleteLight;
};


#endif