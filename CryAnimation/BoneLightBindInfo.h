#ifndef _BONELIGHTBIND_HDR_
#define _BONELIGHTBIND_HDR_

// the storable in file structure for permanent data in CBoneLightBindInfo
struct CBoneLightBindDesc
{
	unsigned m_nDLightFlags; // additional flags that DLight receives (maybe light or heatsource)

	unsigned m_nBone; // the bone to which the binding is made
	Vec3 m_vPos;     // position of the light in the bone CS
	CryQuat m_qRot;   // orientation of the light within the bone CS

	LightTypes m_nType;    // one of the LightTypes values
	CFColor m_rgbColor; // RGB color
	float m_fIntensity; // multiplier value

	float m_fHotSpotSize;		 // for spot and direct lights hotspot value
	float m_fFalloffSize;		 // for spot and direct lights falloff value
	float m_fNearAttenStart; // near attenuation ranges
	float m_fNearAttenEnd;
	float m_fFarAttenStart;  // far attenuation ranges
	float m_fFarAttenEnd;

	Vec3 m_vDirection;          //spot light direction

	// for better alignment, these flags are at the end of the structure
	bool m_bOn;             // light is on
	bool m_bUseNearAtten;   // near atten. on
	bool m_bUseFarAtten;    // far atteniation
	bool m_bShadow;         // shadow is on
};


//////////////////////////////////////////////////////////////////////////
// the structure describing binding of a light to a bone.
// refers to the bone by the id, and contains the whole info about the ligtht
class CBoneLightBindInfo: public CBoneLightBindDesc
{
public:
	CBoneLightBindInfo()
	{m_nBone = 0;}

	// initializes this from the structures found in the cgf file
	void load (const SBoneLightBind&, const LIGHT_CHUNK_DESC&, const char* szLightName, float fScale);

	void scale (float fScale);

	// returns the index of the bone to which the light source is bound
	unsigned getBone() const {return m_nBone;}

	// initializes the given DLight structure out of the given bone instance
	// this is one-time call that is only required after construction of the DLight to initialize its constant parameters
	void initDLight (CDLight& DLight);

	// per-frame update of the DLight structure. Updates the light position and radius
	void updateDLight (const Matrix44& matParentBone, float fRadiusMultiplier, CDLight& DLight);

	// returns true if this light source is local (affects only the character)
	bool isLocal()const;

	bool isHeatSource()const;
	bool isLightSource()const;

	// returns the priority to sort
	int getPriority()const;

	bool operator < (const CBoneLightBindInfo& rRight)const
	{
		return getPriority() < rRight.getPriority();
	}

	// Serialization to/from memory buffer.
	// if the buffer is NULL, and bSave is true, returns the required buffer size
	// otherwise, tries to save/load from the buffer, returns the number of bytes written/read
	// or 0 if the buffer is too small or some error occured
	unsigned Serialize (bool bSave, void* pBuffer, unsigned nSize);

	const char* getLightImageCStr()const {return m_strLightImage.c_str();}
protected:
	string m_strLightImage; //spot light texture

	// the following members are not serialized:
	Matrix44 m_matLight; // transform matrix of the light in the bone CS

	void constructMatLight();
};


#endif