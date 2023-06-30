//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:IStatObj.h
//  Interface for CStatObj class
//
//	History:
//	-:Created by Vladimir Kajalin
//
//////////////////////////////////////////////////////////////////////

#ifndef _IStatObj_H_
#define _IStatObj_H_

// forward declarations
//////////////////////////////////////////////////////////////////////
class			CIndexedMesh;
struct		CLeafBuffer;
template	<class T> class list2;
struct		ShadowMapLightSourceInstance;
struct		ShadowMapLightSource;
class			CCObject;
struct		ItShadowVolume;
struct		IShader;
class			CDLight;
class     CIndoorArea;

//! Interface to non animated object
struct phys_geometry;

#include "IBindable.h"

// Summary:
//     Interface to hold static object data
struct IStatObj: public IBindable
{
	// Description:
	//     Provide access to the faces, vertices, texture coordinates, normals and 
	//     colors of the object.
	// Return Value:
	//     
	// Summary:
	//     Get the object geometry
	virtual CIndexedMesh * GetTriData()=0;

	//! Access to rendering geometry for indoor engine ( optimized vert arrays, lists of shader pointers )
	virtual CLeafBuffer * GetLeafBuffer()=0;
	//! Assign leaf buffer to static object.
	virtual void SetLeafBuffer( CLeafBuffer *buf ) = 0;
	//! Prepare leaf buffer for lightmaps, return true if success
	virtual bool EnableLightamapSupport() = 0;

	// Description:
	//     Returns the physical representation of the object.
	// Arguments:
	//     nType - Pass 0 to get the physic geometry or pass 1 to get the obstruct geometry
	// Return Value:
	//     A pointer to a phys_geometry class. 
	// Summary:
	//     Get the physic representation
	virtual phys_geometry * GetPhysGeom(int nType = 0)=0;
  
	//! Returns script material name
	virtual const char * GetScriptMaterialName(int Id=-1)=0;

	// Return Value:
	//     A Vec3 object countaining the bounding box.
	// Summary:
	//     Get the minimal bounding box component
	virtual Vec3 GetBoxMin()=0;

	// Return Value:
	//     A Vec3 object countaining the bounding box.
	// Summary:
	//     Get the minimal bounding box component
	virtual Vec3 GetBoxMax()=0;

	// Arguments:
	//     Minimum bounding box component
	// Summary:
	//     Set the minimum bounding box component
	virtual void	SetBBoxMin(const Vec3 &vBBoxMin)=0;

	// Arguments:
	//     Minimum bounding box component
	// Summary:
	//     Set the minimum bounding box component
	virtual void	SetBBoxMax(const Vec3 &vBBoxMax)=0;

	// Summary:
	//     Get the object radius
	// Return Value:
	//     A float containing the radius
	virtual float GetRadius()=0;

  //! Sets shader template for rendering
//  virtual bool SetShaderTemplate(int nTemplate, const char *TemplName, const char *ShaderName, bool bOnlyRegister=false)=0;

	//! Sets shader float parameter
	virtual void SetShaderFloat(const char *Name, float Val)=0;

//DOC-IGNORE-BEGIN
	// not implemented
	//! Sets color parameter
	virtual void SetColor(const char *Name, float fR, float fG, float fB, float fA)=0;
//DOC-IGNORE-END

	// Description:
	//     Reloads one or more component of the object. The possible flags 
	//     are FRO_SHADERS, FRO_TEXTURES and FRO_GEOMETRY.
	// Arguments:
	//     nFlags - One or more flag which indicate which element of the object
	//     to reload
	// Summary:
	//     Reloads the object
	virtual void Refresh(int nFlags)=0;

	// Description:
	//     Registers the object elements into the renderer.
	// Arguments:
	//     rParams   - Render parameters
	//     nLogLevel - Level of the LOD
	// Summary:
	//     Renders the object
	virtual void Render(const struct SRendParams & rParams, const Vec3& t, int nLodLevel=0)=0;

	// Description:
	//     Returns the LOD object, if present.
	// Arguments:
	//     nLodLevel - Level of the LOD
	// Return Value:
	//     A static object with the desired LOD. The value NULL will be return if
	//     there isn't any LOD object for the level requested.
	// Summary:
	//     Get the LOD object
	virtual IStatObj * GetLodObject(int nLodLevel)=0;

	// Description:
	//     Draw the shadow volumes for the object into the stencil buffer.
	// Arguments:
	//     rParams   - Rendering Parameters
	//     nLimitLOD - The maximum LOD to be used, this functionality may or 
	//                 may not supported in the implementation
	// Summary:
	//     Draw the shadow volumes
	virtual void RenderShadowVolumes(const struct SRendParams *pParams, int nLimitLod = -1)=0;

	// Description:
	//     Returns a light source specified by the id.
	// Arguments:
	//     nId - Id of the light source
	// Return Value:
	//     A pointer to a CDLight object will be returned. In case the id 
	//     specified as parameter was out of range, the value NULL will be 
	//     returned.
	// Summary:
	//     Get the light sources
	virtual const CDLight * GetLightSources(int nId) = 0;

	// Summary:
	//     Returns the folder name of the object
	// Return Value:
	//     A null terminated string which contain the folder name of the object.
	virtual const char *GetFolderName()=0;

	// Summary:
	//     Returns the filename of the object
	// Return Value:
	//     A null terminated string which contain the filename of the object.
	virtual	const char *GetFileName()=0;		

	// Summary:
	//     Returns the name of the geometry
	// Return Value:
	//     A null terminated string which contains the name of the geometry
	virtual	const char *GetGeoName()=0;		

	// Summary:
	//     Compares if another object is the same
	// Arguments:
	//     szFileName - Filename of the object to compare
	//     szGeomName - Geometry name of the object to compare (optional)
	// Return Value:
	//     A boolean which equals to true in case both object are the same, or false in the opposite case.
	virtual bool IsSameObject(const char * szFileName, const char * szGeomName)=0;

	// Description:
	//     Will return the position of the helper named in the argument. The 
	//     helper should have been specified during the exporting process of 
	//     the cgf file.
	// Arguments:
	//     szHelperName - A null terminated string holding the name of the helper
	// Return Value:
	//     A Vec3 object which contains the position.
	// Summary:
	//     Gets the position of a specified helper
	virtual Vec3 GetHelperPos(const char * szHelperName)=0;

	//! Returns name, position and rotation for specified helper id, returns false if id is out of range
	virtual const char *GetHelperById(int nId, Vec3 & vPos, Matrix44 * pMat = NULL, int * pnType = NULL)=0;

	// Description:
	//     Will return the matrix of the helper named in the argument. The 
	//     helper should have been specified during the exporting process of 
	//     the cgf file.
	// Arguments:
	//     szHelperName - A null terminated string holding the name of the helper
	// Return Value:
	//     A Matrix44 of the object
	// Summary:
	//     Gets the matrix of a specified helper
	virtual const Matrix44 * GetHelperMatrixByName(const char * szHelperName) = 0;

	// Description:
	//     Increase the reference count of the object.
	// Summary:
	//     Notifies that the object is being used
	virtual void RegisterUser() = 0;

	// Description:
	//     Decrease the reference count of the object. If the reference count 
	//     reaches zero, the object will be deleted from memory.
	// Summary:
	//     Notifies that the object is no longer needed
	virtual void UnregisterUser() = 0;

//DOC-IGNORE-BEGIN
	//! Tell us if the object is not found 
	virtual bool IsDefaultObject()=0;
//DOC-IGNORE-END

	// Summary: Unsupported. Should not be used.
	virtual bool MakeObjectPicture(unsigned char * pRGBAData, int nWidth) = 0;

	// Summary:
	//     Get the shadow volume object
	// Return Value:
	//     An ItShadowVolume object.
	virtual ItShadowVolume *GetShadowVolume()=0;

	// Summary:
	//     Set the shadow volume object
	// Arguments:
	//     pSvObj - A new shadow volume
	virtual void SetShadowVolume(ItShadowVolume *pSvObj)=0;

	//! returns occlusion volume in object space
	virtual bool GetOcclusionVolume(list2<Vec3> * & plstOcclVolVerts, list2<int> * & plstOcclVolInds) = 0;

	// Summary:
	//     Free the geometry data
	virtual void FreeTriData() = 0;

//DOC-IGNORE-BEGIN
	// Pushes the underlying tree of objects into the given Sizer object for statistics gathering
	virtual void GetMemoryUsage(class ICrySizer* pSizer) {} // TODO: implement
//DOC-IGNORE-END

	// Description:
	//     Will looks if the object has all the needed shaders to be used as vegetation.
	// Return Value:
	//     true will be returned if the object can be used as vegetation, else false will be returned.
	// Summary:
	//     Determines if the object can be used as vegetation
	virtual bool CheckValidVegetation() = 0;

//DOC-IGNORE-BEGIN
	//! used for sprites
	virtual float & GetRadiusVert() = 0;

	//! used for sprites
	virtual float & GetRadiusHors() = 0;
//DOC-IGNORE-END

	// Summary:
	//     Determines if the object has physics capabilities
	virtual bool IsPhysicsExist() = 0;

	// Summary:
	//     Starts preloading textures, shaders and sprites
	virtual void PreloadResources(float fDist, float fTime, int dwFlags) = 0;

	// Summary: 
	//     Returns a pointer to the object
	// Return Value:
	//     A pointer to the current object, which is simply done like this "return this;"
	virtual struct IStatObj* GetIStatObj() {return this;}
};

#endif // _IStatObj_H_
