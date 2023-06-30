//////////////////////////////////////////////////////////////////////
//
//	Crytek Indoor Engine DLL source code
//	
//	File: CryIndoorEngine.h
//
//	History:
//	-September 03,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////


#ifndef INDOORINTERFACE_H
#define INDOORINTERFACE_H

#if _MSC_VER > 1000
# pragma once
#endif

//////////////////////////////////////////////////////////////////////
#define INDOORSCALE 0

//forward declarations
//////////////////////////////////////////////////////////////////////
struct	IStatObj;
struct	ILog;
struct	IRenderer;
struct	IConsole;
struct	I3DEngine;
struct	ICVar;
struct	IEntityRender;
struct	ISystem;
class		CShadowVolObject;
class CIndoorArea;
struct	tShadowVolume;
struct	RenderParams; 
struct	SRendParams;
//class   Quaternion_tpl;


//////////////////////////////////////////////////////////////////////
struct IndoorBaseInterface
{
	ILog			*m_pLog;
	IRenderer	*m_pRenderer;	
	I3DEngine	*m_p3dEngine;
	IConsole	*m_pConsole;
	ISystem		*m_pSystem;

	ICVar			*m_pCVDrawSoftEdges;
	ICVar			*m_pCVRefreshCubeMap;
	ICVar			*m_pCVRefreshLights;
	ICVar			*m_pCVShadowsEnable;	
	ICVar			*m_pCVDrawBorderEdges;
	ICVar			*m_pCVDrawLight;	
	ICVar			*m_pCVTransparentShadows;
	ICVar			*m_pCVCLipSimple;
	ICVar			*m_pCVStaticLights;	
	ICVar			*m_pCVDrawModels;
	ICVar			*m_pCVCalcLight;
	ICVar			*m_pCVSetFence;
	ICVar			*m_pCVAmbientOnly;
	ICVar			*m_pCVDetectShader;
	ICVar			*m_pCVUpdateBuffers;
	ICVar			*m_pCVAnimatedShadows;
	//! ambient color for the building
	ICVar			*m_pcvRAmbientColor;
	ICVar			*m_pcvGAmbientColor;
	ICVar			*m_pcvBAmbientColor;
	ICVar			*m_pcvAmbientColor;
	//! M.M. for debugging stencil shadows (0 is default, use !=0 to force reacalculation of indoor stencil shadow volumes)
	ICVar			*m_pcvRecalcStencilVolumes;
};

/*! Interface to the Indoor Engine.

	The Indoor Engine interface is very simple and consists of a few functions
	to allow to load and perform some basic operations on an indoor structure,
	a so called building.

	December 04 changes:
	- The building now is more like a building manager that includes multiple
	buildings, the loading of buildings is done in the DLL using the XML parser
	August 04 changes:
	- The buildings are loaded dynamically from the game 

	IMPLEMENTATIONS NOTES:
	During the loading of the building, a preprocess step is performed 
	(IndoorBuildingPreprocess.cpp) to create a list of potentially silhouette edges casting 
	shadows.
	As the light source position is provided to the module (specified in the .CGF file), 
	a list of edges casting shadows is
	detected and a list of shadow volume polygons is created; those shadow volumes polygons
	are subsequently clipped with the convex hull of the corresponding area.
	The shadow volumes are also capped (at the beginning and at the end of the extruded edges) 
	and clipped, as mentioned above, to solve the "camera-in-shadow-volume" and near clip plane 
	issues. (IndoorShadowVolumePreprocess.cpp).
	The building basically is composed by a set of areas connected by portals; the areas to be
	rendered are determined recursively based on a camera position provided by the 3d engine.
	The indoor engine is also responsable for detecting if there is any outdoor area visible.
	All the classes are derived from CVolume;further details are provided in the .cpp and .h files
	of the DLL implementation.		
*/ 


//! contains material information returned by ray intersection
//////////////////////////////////////////////////////////////////////
struct IndoorRayIntInfo
{
	IStatObj	*pObj;
	int				nFaceIndex;		
	Vec3_tpl<float>			vPoint; //point of intersection
	float			fDist2; //squared distance from the ray's starting point
	Vec3_tpl<float>			vNormal; //normal of the polygon intersecting the ray
	int				nBuildId;
};

/*
//! holds shadow volume informations for static objects
//////////////////////////////////////////////////////////////////////
struct ItShadowVolume
{
	virtual void	Release()=0;	
	virtual Vec3d	GetPos()=0;
	virtual void	SetPos(const Vec3d &vPos)=0;
	virtual	CShadowVolObject	*GetShadowVolume()=0;
	virtual	void	SetShadowVolume(CShadowVolObject *psvObj)=0;

	//! /param lSource lightsource, worldspace and objectspace position is used
	//! /param inArea pointer to the area whre the object is in (could be 0 - but shadow extrusion is set to maximum)
	virtual	void	RebuildDynamicShadowVolumeBuffer( const CDLight &lSource, CIndoorArea *inArea )=0;

	//! return memory usage
	virtual	int GetMemoryUsage(){ return 0; }; //todo: implement
};
	*/
//! contains sector/building infos
//////////////////////////////////////////////////////////////////////
struct tIndoorSector
{
	int nSectorId;
	int nBuildingId;	
};

//////////////////////////////////////////////////////////////////////
struct IIndoorBase
{		
	/*! Render the building
		@param Camera position
	*/	
	virtual void			Render(const CCamera &cam)=0;

	/*! Check if a point is inside the building
		@param vPos point position, in world space
		@param nBuilding building number, default -1 will check in all buildings
		@return true if the point is inside the building, false otherwise		
	*/	
	virtual	bool			CheckInside(const Vec3d &vPos,int nBuilding=-1)=0;	

	/*! Check if a point is inside the building
		@param vPos point position, in world space
		@param nBuilding return building number
		@param nSectorId return sectorid
		@return true if the point is inside the building, false otherwise		
	*/	
	virtual	bool			CheckInside(const Vec3d &vPos,int &nBuilding,int &nSectorId)=0;	

	/*! Check if a point is inside the building
		@param vPos point position, in world space
		@param tSecInfo returns sectors info
		@return true if the point is inside the building, false otherwise		
	*/	
	virtual	bool			CheckInside(const Vec3d &vPos,tIndoorSector &tInfo)=0;	

	//! delete the building	manager and all buildings
	virtual	void			Release()=0;
	
	/*! Register an entity for rendering into the building
		@param pModel interface to the entity to be added
		@param vPos object position, in world space
		@return true if the model is inside any building, false otherwise		
	*/		
	virtual bool	RegisterEntity(IEntityRender *pModel,const Vec3d &pos)=0;

	/*! UnRegister an entity from the building
		@param pModel interface to the entity to be removed
	*/		
	virtual void	UnRegisterEntity(IEntityRender *pModel)=0;

	/*! Activate or deactivate a light source
		@param szName name of the light source, as exported into the .cgf file
		@param specify if the light should be either activated or deactivated
	*/		
	virtual void	ActivateLight(const char *szName,bool bActivate)=0;

	/*! For debug purposes, should not be used
	*/		
	virtual	void	RenderPostProcess()=0;

	/*! For debug purposes, should not be used
	*/		
	virtual void	RenderDebug(const CCamera &cam)=0;

	/*! return a specific object for collision detection registration
		@param	nPartId		an integer number speciyfing which object to return 
		@param  nBuildId	specify which building to use, -1 will start from building 0
		@return the requested object
		REMARKS:
		if nPartId is invalid,NULL will be returned
	*/				
	virtual	IStatObj	*GetStatObj(int nPartId,int nBuildId=-1)=0;

	/*! return the position of the requested building in world space
		@param	nBuildId specify which building to get the position from
		@return building position 
		REMARKS:
		return (0,0,0) if buildid is invalid
	*/
	virtual Vec3d			GetPosition(int nBuildId)=0;

  /*! Initialize system interface
   //@param	pIndInterface indoorbaseinterface
   */
  virtual void  SetInterface(const IndoorBaseInterface &pIndInterface)=0;
  
	/*
  //! Load the buildings of the level from the xml file
	//@param	IndInterface interface to the system
	//@param	szFileName xml filename  
	//@param	szMissionName mission name
	//@return true if loading succeed
	//@return false if file not found, cannot load buildings or empty building list
	//
	virtual bool	LoadBuildings(const IndoorBaseInterface &IndInterface,const char *szFileName,const char *szMissionName=NULL)=0;
	*/

	/*! Add a dynamic light source to the building
		@param	light source structure
		@param	bMainLight assign the mainlight of the area to the specified position
		@return true if the light is inside the building
	*/
	virtual bool	AddLightSource(const class CDLight &lSource)=0;

	//! set ambient color for indoor (default ambient is black)
	virtual void	SetIndoorAmbientColor(const Vec3d &Vcolor)=0;

	/*! render and if necessary create shadow volume resources
		@param pObj the source object
		@param pParams render params
		@param vLightPos light position
	*/
	virtual	void	RenderShadowVolumes(IStatObj *pObj,const SRendParams *pParams)=0;

//Editor's functions

	/*! Load a Building and add to the building manager
		@param pIndInterface IndoorBaseInterface filled with pointers to system interface
		@return building id if loaded successfully, otherwise returns -1
	*/
	virtual int		CreateBuilding(const IndoorBaseInterface &pIndInterface)=0;

	/*! Set the building position
	@param vPos building position
	@param nBuildId the building's IdNumber returned by LoadBuilding
	*/
	virtual void	SetBuildingPos(const Vec3d &vPos,int nBuildId)=0;

	/*! Set the building bbox
	@param vMins building mins bbox
	@param vMaxs building maxs bbox
	@param nBuildId the building's IdNumber returned by LoadBuilding
	*/
	virtual void	SetBuildingBBox(const Vec3d &vMins,const Vec3d &vMaxs,int nBuildId)=0;

	/*! Remove a building	
	@param nBuildId the building's IdNumber returned by LoadBuilding
	*/
	virtual void	DeleteBuilding(int nBuildId)=0;
	
	/*! Remove all buildings	 	
	 */
	virtual void	DeleteAllBuildings()=0;

	//-1 if not found
	virtual	int		GetBuildingIdFromObj(IStatObj *pObj)=0;

	/*! Add a building to the building manager
	@param	szFilename cgf room filename to add to the building
	@param	nBuildId the building's IdNumber returned by LoadBuilding
	@return true if the operation succeed
	*/
	virtual int	AddBuilding(const char *szFilename,int nBuildId)=0;
	
	/*! Draw a building	
	@param	vPos building position
	@param	nBuildId the building's IdNumber returned by LoadBuilding	
	*/
	virtual void	DrawBuilding(const CCamera &cam,int nBuildId)=0;

	/*! Get building's bbox	
	@param	vBBMin,vBBMax bbox information (filled by the function)
	@param	nBuildId the building's IdNumber returned by LoadBuilding	
	*/
	virtual void	GetBBox(Vec3d &vBBMin,Vec3d &vBBMax,int nBuilID)=0;

	/*! Get helper infos, to allow editor to snap to helpers
	@param	nHelperId helper Id
	@param	nBuildId	the building's IdNumber returned by LoadBuilding	
	@param	vPos					return helper's position	
	@param	pMat					return helper's position & orientation
	@param	nHelperType		return helper's type
	@return helper name, NULL if nHelperId is out of range
	*/
	virtual const	char *GetHelper(int nHelperId, int nBuildId, Vec3d &vPos, Matrix44 * pMat = NULL, int * pnHelperType = NULL)=0;

	/*! Hide a building
	@param bHide hide/unhide the building
	@param nBuildId	the building's IdNumber returned by LoadBuilding	
	*/
	virtual void	HideBuilding(bool bHide,int nBuildId)=0;

	/*! Activate or deactivate a main light source
		@param vPos		 light refernce position to detect the area with the main light	
		@param specify if the light should be either activated or deactivated
	*/		
	virtual void	ActivateMainLight(const Vec3d &vPos,bool bActivate)=0;

	/*! Hide a sector
	@param bHide hide/unhide the sector
	@param nBuildId		the building's IdNumber returned by LoadBuilding	
	@param nSectorId	sector's idnumber
	*/
	virtual void	HideSector(bool bHide,int nBuildId,int nSectorId)=0;		

	/*! Enable/Disable visibility processing 
	@param bEnable
	*/
	virtual void	EnableVisibility(bool bEnable)=0;

	/*! Enable/Disable wireframe mode for building only
	@param bEnable
	*/
	virtual void	EnableWireframe(bool bEnable)=0;	

	/*!tell the number of buildings loaded
	@return	number of buildings loaded 	
	*/
	virtual	int		GetBuildingsNum()=0;
  
	/*! Check visibility between sector containing vPos and camera sector
		@param	vPos	reference point		
		@param	tInfo if provided will check for the specified sector and building
		@return	false if sector is not visible from the current camera, true otherwise		
	*/
  virtual	bool IsPointPotentiallyVisible(const Vec3d &vPos,tIndoorSector *tInfo=NULL)=0;

	/*! Check visibility between sector containing the bbox and camera sector
		@param	vMins	bbox's mins		
		@param	vMaxs	bbox's maxs		
		@param	tInfo if provided will check for the specified sector and building
		@return	false if sector is not visible from the current camera, true otherwise		
	*/
  virtual	bool IsBBoxPotentiallyVisible(const Vec3d &vMins,const Vec3d &vMaxs,tIndoorSector *tInfo=NULL)=0;

	/*! Return dynamic light mask of sector containing vPos
		@param	vPos	reference point		
		@param	tInfo if provided will check for the specified sector and building
		@return	dynamic light mask 
	*/
  virtual	int GetLightMaskForPoint(const Vec3d &vPos,tIndoorSector *tInfo=NULL)=0;

	/*! Return the sorrounding ambient color for this point
		@param	vPos	reference point
		@param	tInfo if provided will check for the specified sector and building
		@return	color
	 */
	virtual Vec3d GetAmbientColor(const Vec3d &vPos,tIndoorSector *tInfo=NULL)=0;

	/*! closes/opens the portal (for example a door can totally block visibility)
	 @param	vPos portal position (the closest portal will be detected)
	 @param	bActivate		close/open the portal
	 @param	pEntity			the entity that closes or opens the portal
	 */
	virtual void	ActivatePortal(const Vec3d &vPos,bool bActivate,IEntityRender *pEntity)=0;

	/*! decides if a sound is potentially hearable (different sectors, a door block the sounds)
		@param nBuldingId (the buildingId)
		@param nSecId1	  (the sectorid of one of the source)
		@param nSecId2	  (the sectorid of one of the source)
		@return	true if sound is hearable, false otherwise
	*/
	virtual bool	IsSoundPotentiallyHearable(int nBuildingId,int nSecId1,int nSecId2)=0;

	/*! adds/removes a cgf to the outside part of the building (no portals / areas /occlusion)
	 @param nBuldingId	The buildingId
	 @param pObj				The cgf to add	 
	 @param bRemove			Tells to the engine if the pObj must be added or removed.
											If the obj is already present will not be added again
	 */
	virtual bool	SetOutsideStatObj(int nBuildingId,IStatObj *pObj,bool bRemove=false)=0;

};

#ifndef _XBOX
#ifdef CRYINDOORENGINE_EXPORTS
#define CRYINDOORENGINE_API __declspec(dllexport)
#else
#define CRYINDOORENGINE_API __declspec(dllimport)
#endif
#else
#define CRYINDOORENGINE_API
#endif

//////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
	
typedef IIndoorBase	* (*PFNCREATEBUILDINGMANAGER)(IndoorBaseInterface *pIndInterface);
CRYINDOORENGINE_API IIndoorBase* CreateBuildingManager(IndoorBaseInterface *pIndInterface);

#ifdef __cplusplus
}
#endif


#endif