#include "MathUtils.h"
extern Matrix44 debugGetMatrix (CCObject*);
extern void debugDrawBBox (const Matrix34&, const CryAABB& BBox, int nSubdiv,const float* pColor = NULL);
extern void debugDrawSphere (const Matrix34& m34, float fRadius, const float pColor[4] = NULL);
extern void debugDrawRootBone (const Matrix44& rBone, float fSize = 0.02f, const float* pColor = NULL);
extern void debugDrawBone (const Matrix44& rParent, const Matrix44& rBone, const float pColor[4] = NULL);
extern void debugDrawOctahedron (const Matrix44& matModel, float fSize, const float pColor[4] = NULL);
extern void debugDrawLine (const Vec3& a, const Vec3& b, const float pColor[4] = NULL);
extern void debugDrawLine (const Matrix44& matLCS, const Vec3& a, const Vec3& b, const float pColor[4] = NULL);

extern bool IsValidString(const char*szMorphTarget);