
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// XSurfaceMgr.h: interface for the CXSurfaceMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XSURFACEMGR_H__FA141F16_72C8_44B1_98D3_CEB5E61093DA__INCLUDED_)
#define AFX_XSURFACEMGR_H__FA141F16_72C8_44B1_98D3_CEB5E61093DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <I3DEngine.h>

struct SMatProps
{
	SMatProps()
	{
		bNoCollide=false;
	}
	bool bNoCollide;
};

#define TERRAIN_MATERIAL_SURFACE_ID_BASE 0
#define ENTITY_MATERIAL_SURFACE_ID_BASE 10
/*!manage the loading and mapping of physical surface ids anf materials script
BEHAVIOUR:
	-at creation time when LoadMaterials is invocked the class traverse the "scripts/materials" directory and store all existing materials paths
	-later the game will load the terrain materials from LevelData.xml and call AddTerrainSurface() for each terrain layer, this cause the loading of the
		assiciated materials
	-from this point every time a CGF(model) is loaded the 3d engine will call EnumPhysMaterial() with as argument the name of the material
		specified in 3dsMax, 
		CXSurfaceMgr will:
			-check if the material exists.
			-if exists will check if is already loaded
			-if is loaded will simply return the surface idx
			-if not will load the material,generate a new surface idx and return it.
*/
class CXSurfaceMgr : 
public IScriptObjectDumpSink,
public IPhysMaterialEnumerator
{
	//typedef std::vector<string> TerrainSurfaces;
	//typedef TerrainSurfaces::iterator TerrainSurfacesItor;
	typedef std::map<int,string> PhysicalSurfecesMap;
	typedef PhysicalSurfecesMap::iterator PhysicalSurfecesMapItor;
	typedef std::map<string,int> MaterialsMap;
	typedef MaterialsMap::iterator MaterialsMapItor;

	struct MatDesc
	{
		string sScriptFilename;
		int surfaceId;
	};

	typedef std::map<string,MatDesc> MaterialsNamesMap;
	typedef MaterialsNamesMap::iterator MaterialsNamesMapItor;

	typedef std::map<int,SMatProps> PhysicalSurfacesPropsMap;
	typedef PhysicalSurfacesPropsMap::iterator PhysicalSurfacesPropsMapItor;
public:
	CXSurfaceMgr();
	virtual ~CXSurfaceMgr();

	/*!initialize the class
	*/
	void Init(IScriptSystem *pScriptSystem,I3DEngine *p3DEngine,IPhysicalWorld *pPhysicalWorld);
	/*!add anew terrain material
		NOTE:
			this function must be call before any other EnumPhysMaterial() because the terrain surface ids
			are sequential starting from 0 so the have to be created in the correct order.
		@param sMaterial the name of the material
	*/
	void SetTerrainSurface( const string &sMaterial,int nSurfaceID);
	/*!scan a directory and load all material paths
		@param sFolder the folde tha has to be scanned
		@param bReaload [legacy]
	*/
	bool LoadMaterials( const string& sFolder,bool bReload=false,bool bAddMaterials=false );

	//! Load default materials (mat_default,mat_water)
	void LoadDefaults();
	
	/*! return the material script object specifying the material name
		@param sMaterialName the material's name
		@return the material script object if succeded and null if failed
	*/
	IScriptObject * GetMaterialByName( const char *sMaterialName );
	/*! return the material script object specifying the surface id
		@param nSurfaceID the material's surface id
		@return the material script object if succeded and null if failed
	*/
	IScriptObject * GetMaterialBySurfaceID(int nSurfaceID);
	/*! return the material surface id object specifying the material name
		@param sMaterialName the material's name
		@return the material surface id or the default material surface id if the surfac id is not found
		*/
	int GetSurfaceIDByMaterialName(const char *sMaterialName);

	/*! return the burnable property from gameplay_physic table from material script
		@param nSurfaceID the material's surface id
		@return the burnable property from gameplay_physic table from material script
		*/
	bool IsMaterialBurnable(int nSurfaceID);

	/*! reloads material's physics properties into the physics engine; script file is NOT reloaded if it was loaded before
		@param sMaterialName the material's name
	*/
	void ReloadMaterialPhysics(const char *sMaterialName);
	void ReloadMaterials();

	bool GetMaterialParticlesTbl(int nSurfaceID, const char* tblName, ParticleParams &sParamOut, IGame* pGame, ISystem* pSystem);

	/*!LEGACY FUNCTION
	*/
	void InitPhisicalSurfaces();
	void Reset();

	//IScriptObjectDumpSink
	void OnElementFound(const char *sName,ScriptVarType type);
	void OnElementFound(int nIdx,ScriptVarType type){/*ignore non string indexed values*/};
	//IPhysMaterialEnumerator
	int EnumPhysMaterial(const char * szPhysMatName);
	bool IsCollidable(int nMatId);
	int	GetMaterialCount();
	const char* GetMaterialNameByIndex( int index );
	unsigned MemStat();

private:
	//! @return Id of loaded material.
	int LoadMaterial(const string &sMaterialName,bool bLoadAlsoIfDuplicate=false,int nForceID=-1);
	//! @retun Id of added material.
	int AddMaterial( const string &sMaterial,int nForceID=-1);
	void SetMaterialGameplayPhysic( int nId, _SmartScriptObject &table );
	int GetDefaultMaterial();

	/*! return the material of a cetain position in the height fiels
	@param fX the x coodinate into the height field
	@param fY the y coodinate into the height field
	@return the material script object if succeded and null if failed
	*/
	IScriptObject * GetTerrainMaterial(const float fX,const float fY);
	/*! return the material name of a cetain position in the height fiels
	NOTE: this function is for debug pourposes only
	@param fX the x coodinate into the height field
	@param fY the y coodinate into the height field
	@return the material name
	*/
	string &___GetTerrainMaterialName(const float fX,const float fY);

	IScriptSystem *m_pScriptSystem;
	I3DEngine *m_p3DEngine;
	IPhysicalWorld *m_pPhysicalWorld;
	//Material table(eg, mat_stuff)
	IScriptObject *m_pMaterialScriptObject;
	//Materials table
	IScriptObject *m_pObjectMaterials;
//	TerrainSurfaces m_vTerrainSurfaces;
	PhysicalSurfecesMap m_mapPhysSurfaces;
	MaterialsMap m_mapMaterials;
	MaterialsNamesMap m_mapMaterialsNames;
	PhysicalSurfacesPropsMap m_mapMaterialProps;
	int m_nLastFreeSurfaceID;

	int m_mat_default_id;
};

#endif // !defined(AFX_XSURFACEMGR_H__FA141F16_72C8_44B1_98D3_CEB5E61093DA__INCLUDED_)
