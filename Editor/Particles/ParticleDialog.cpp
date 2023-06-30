////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   particledialog.cpp
//  Version:     v1.00
//  Created:     17/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ParticleDialog.h"

#include "StringDlg.h"
#include "NumberDlg.h"

#include "ParticleManager.h"
#include "ParticleLibrary.h"
#include "ParticleItem.h"

#include "Objects\BrushObject.h"
#include "Objects\Entity.h"
#include "Objects\ObjectManager.h"
#include "ViewManager.h"
#include "Clipboard.h"

#include <I3DEngine.h>
//#include <IEntityRenderState.h>
#include <IEntitySystem.h>

#define IDC_PARTICLES_TREE AFX_IDW_PANE_FIRST

#define EDITOR_OBJECTS_PATH CString("Objects\\Editor\\")

IMPLEMENT_DYNAMIC(CParticleDialog,CBaseLibraryDialog);
//////////////////////////////////////////////////////////////////////////
// Particle UI structures.
//////////////////////////////////////////////////////////////////////////

/*
struct ParticleParams
{
	ParticleParams() { memset(this,0,sizeof(*this)); }
	Vec3_tpl<float> vPosition; // spawn position
	Vec3_tpl<float> vDirection; // initial direction  (normalization not important)
	float fFocus; // if 0 - particles go in all directions, if more than 20 - particles go mostly in vDirection
	Vec3_tpl<float> vColorStart; // initial color
	Vec3_tpl<float> vColorEnd; // final color
	float fSpeed; // initial speed ( +- 25% random factor applyed, m/sec )
	Vec3_tpl<float> vRotation; // rotation speed (degree/sec)
	Vec3_tpl<float> vInitAngles; // initial rotation
	int   nCount; // number of particles to spawn
	float fSize; // initial size of particles
	float fSizeSpeed; // particles will grow with this speed
	Vec3_tpl<float> vGravity; // gravity(wind) vector
	float fLifeTime; // time of life of particle
	float fFadeInTime; // particle will fade in slowly during this time
	int   nTexId; // texture id for body and trail (if used) ( if 0 - will be used default ball/glow texture )
	int   nTexAnimFramesCount; // number of frames in animated texture ( 0 if no animation )
	ParticleBlendType eBlendType; // see ParticleBlendType
	float fTailLenght; // delay of tail ( 0 - no tail, 1 meter if speed is 1 meter/sec )
	int   nParticleFlags; // see particle system flags
	bool  bRealPhysics; // use physics engine to control particles
	IStatObj * pStatObj; // if not 0 - this object will be used instead sprite
	ParticleParams * pChild; // child process definition
	float fChildSpawnPeriod; // if more than 0 - run child process every x seconds, if 0 - run it at collision
	int   nDrawLast; // add this element into second list and draw this list last
	float fBouncenes; // if 0 - particle will not bounce from the ground, 0.5 is good in most cases
	float  fTurbulenceSize; // radius of turbulence
	float  fTurbulenceSpeed; // speed of rotation
	float fDirVecScale; //the game need to store this(Alberto)
	struct IEntityRender * pEntity; // spawner entity
	struct IShader * pShader; // shader for partice
	float fPosRandomOffset; // maximum distance of random offset from original position
	IMatInfo *pMaterial; // Override material.
};
*/

struct SParticleTableUI
{
	CVariable<CString> texture;
	CVariable<CString> geometry;
	CVariable<CString> material;

	CVariableArray sizeTable;
	CVariableArray moveTable;
	CVariableArray rotateTable;
	CVariableArray visualTable;
	CVariableArray advancedTable;

	CVariableEnum<int> type;
	CVariableEnum<int> blendType;
	CVariable<float> focus;
	CVariable<bool> focus_plane;
	CVariable<bool> use_gravity_direction;
	CVariable<bool> bind_pos_to_emitter;
	CVariable<float> speed;
	CVariable<float> speed_variation;
	CVariable<float> speed_fadeout;
	CVariable<float> speed_accel;
	CVariable<float> speed_airResistance;
	CVariable<float> size;
	CVariable<float> size_variation;
	CVariable<float> sizeSpeed;
	CVariable<bool> sizeLinear;
	CVariable<float> sizeFadeIn;
	CVariable<float> sizeFadeOut;
	CVariable<float> object_scale;
	CVariable<int> count;
	CVariable<float> lifeTime;
	CVariable<float> lifeTime_variation;
	CVariable<float> fadeInTime;
	CVariable<int> frameCount;
	CVariable<float> tailLength;
	CVariable<int> tailSteps;
	CVariable<float> stretch;
	CVariable<bool> realPhysics;
	CVariable<int> drawLast;
	CVariable<float> bounciness;
	CVariable<float> turbulenceSize;
	CVariable<float> turbulenceSpeed;
	//CVariable<float> posRandomOffset;
	CVariable<bool> no_underwater;
	CVariable<bool> rigid_body;

	CVariable<Vec3> colorStart;
	CVariable<Vec3> colorEnd;

	CVariable<Vec3> rotation;
	CVariable<float> rotation_variation;
	CVariable<Vec3> angles;
	CVariable<float> angles_variation;
	CVariable<Vec3> gravity;
};

/** User Interface definition of particle system.
*/
class CParticleUIDefinition
{
public:
	enum EParticleSystemType
	{
		PRIMARY_SYSTEM = 0,
		CHILD_SYSTEM = 1,
	};
	CVariableArray uiParamsArray[2];
	SParticleTableUI uiParams[2];

	//////////////////////////////////////////////////////////////////////////
	// One per effect variables.
	CVariable<bool> bActive;
	CVariable<Vec3> positionOffset;
	CVariable<Vec3> randomPositionOffset;
	CVariable<float> spawnDelay;
	CVariable<float> spawnDelay_variation;

	CVariable<float> emitterLifeTime;
	CVariable<float> emitterLifeTime_variation;

	CVariable<float> spawnPeriod;
	CVariable<float> childSpawnPeriod;
	CVariable<float> childSpawnTime;

	CVariableArray emitterTable;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Sound variables.
	CVariableArray uiSound;
	CVariable<CString> soundFilename;
	CVariable<int> soundVolume;
	CVariable<float> soundMinRange;
	CVariable<float> soundMaxRange;
	CVariable<bool> soundLoop;
	CVariable<bool> soundEverySpawn;
	//////////////////////////////////////////////////////////////////////////

	CVarEnumList<int> enumType;
	CVarEnumList<int> enumBlendType;

	CVarBlockPtr m_vars;
	IVariable::OnSetCallback m_onSetCallback;

	//////////////////////////////////////////////////////////////////////////
	CVarBlock* CreateVars()
	{
		m_vars = new CVarBlock;

		//////////////////////////////////////////////////////////////////////////
		// Init enums.
		//////////////////////////////////////////////////////////////////////////
    enumBlendType.AddRef(); // We not using pointer.
    enumBlendType.AddItem( "AlphaBlend",ParticleBlendType_AlphaBased );
    enumBlendType.AddItem( "ColorBased",ParticleBlendType_ColorBased );
		enumBlendType.AddItem( "Additive",ParticleBlendType_Additive );
		enumBlendType.AddItem( "None",ParticleBlendType_None );

		enumType.AddRef(); // We not using pointer.
		enumType.AddItem( "Billboard",PART_FLAG_BILLBOARD );
		enumType.AddItem( "Horizontal",PART_FLAG_HORIZONTAL );
		enumType.AddItem( "Underwater",PART_FLAG_UNDERWATER );
		enumType.AddItem( "Line",PART_FLAG_LINEPARTICLE );

		//////////////////////////////////////////////////////////////////////////
		// Init tables.
		//////////////////////////////////////////////////////////////////////////
		AddVariable( m_vars,uiParamsArray[PRIMARY_SYSTEM],"Particle Settings" );
		AddVariable( m_vars,uiParamsArray[CHILD_SYSTEM],"ChildProcess Settings" );
		AddVariable( m_vars,uiSound,"Sound Settings",IVariable::DT_SIMPLE );

		AddVariable( uiParamsArray[PRIMARY_SYSTEM],bActive,"Active" );

		AddVariable( uiParamsArray[CHILD_SYSTEM],childSpawnPeriod,"Child Spawn Period" );
		AddVariable( uiParamsArray[CHILD_SYSTEM],childSpawnTime,"Child Spawn Max Time" );

		AddParticleTableVars( uiParamsArray[PRIMARY_SYSTEM],uiParams[PRIMARY_SYSTEM] );
		AddParticleTableVars( uiParamsArray[CHILD_SYSTEM],uiParams[CHILD_SYSTEM] );

		{
			CVariableArray &table = uiParamsArray[PRIMARY_SYSTEM];
			AddVariable( uiParams[PRIMARY_SYSTEM].moveTable,positionOffset,"Position Offset" );
			AddVariable( uiParams[PRIMARY_SYSTEM].moveTable,randomPositionOffset,"Random Offset" );

			emitterTable.SetFlags( IVariable::UI_BOLD );
			AddVariable( table,emitterTable,"Emitter" );
			AddVariable( emitterTable,spawnDelay,"Spawn Delay" );
			AddVariable( emitterTable,spawnDelay_variation,"Spawn Delay Variation",IVariable::DT_PERCENT );
			AddVariable( emitterTable,spawnPeriod,"Spawn Period" );
			AddVariable( emitterTable,emitterLifeTime,"Emitter Life Time" );
			AddVariable( emitterTable,emitterLifeTime_variation,"Emitter Life Time Variation",IVariable::DT_PERCENT );
		}

		AddVariable( uiSound,soundFilename,"Sound",IVariable::DT_SOUND );
		AddVariable( uiSound,soundLoop,"Loop" );
		AddVariable( uiSound,soundEverySpawn,"On Every Spawn" );
		AddVariable( uiSound,soundVolume,"Volume" );
		AddVariable( uiSound,soundMinRange,"Min Range" );
		AddVariable( uiSound,soundMaxRange,"Max Range" );

		return m_vars;
	}

	void AddParticleTableVars( CVariableArray &table,SParticleTableUI &parts )
	{
		parts.blendType.SetEnumList( &enumBlendType );
		parts.type.SetEnumList( &enumType );

		parts.sizeTable.SetFlags( IVariable::UI_BOLD );
		parts.moveTable.SetFlags( IVariable::UI_BOLD );
		parts.rotateTable.SetFlags( IVariable::UI_BOLD );
		parts.visualTable.SetFlags( IVariable::UI_BOLD );
		parts.advancedTable.SetFlags( IVariable::UI_BOLD );
		parts.tailSteps.SetLimits( 0,16 );

		//////////////////////////////////////////////////////////////////////////
		/// Add variables.
		//////////////////////////////////////////////////////////////////////////

		AddVariable( table,parts.texture,"Texture",IVariable::DT_TEXTURE );
		AddVariable( table,parts.geometry,"Geometry",IVariable::DT_OBJECT );
		AddVariable( table,parts.material,"Material",IVariable::DT_MATERIAL );

		AddVariable( table,parts.type,"Type" );
		AddVariable( table,parts.count,"Count" );
		AddVariable( table,parts.lifeTime,"Life Time" );
		AddVariable( table,parts.lifeTime_variation,"Life Time Variation",IVariable::DT_PERCENT );

		//////////////////////////////////////////////////////////////////////////
		// Movement table.
		AddVariable( table,parts.moveTable,"Movement" );
		AddVariable( parts.moveTable,parts.focus,"Focus" );
		AddVariable( parts.moveTable,parts.focus_plane,"Focus On Plane" );
		AddVariable( parts.moveTable,parts.speed,"Speed" );
		AddVariable( parts.moveTable,parts.speed_variation,"Speed Variation",IVariable::DT_PERCENT );
		AddVariable( parts.moveTable,parts.speed_fadeout,"Speed FadeOut" );
		AddVariable( parts.moveTable,parts.speed_accel,"Acceleration" );
		AddVariable( parts.moveTable,parts.speed_airResistance,"Air Resistance" );
		AddVariable( parts.moveTable,parts.gravity,"Gravity" );
		AddVariable( parts.moveTable,parts.use_gravity_direction,"Speed In Gravity Direction" );
		AddVariable( parts.moveTable,parts.turbulenceSize,"Turbulence Size" );
		AddVariable( parts.moveTable,parts.turbulenceSpeed,"Turbulence Speed" );
		AddVariable( parts.moveTable,parts.bind_pos_to_emitter,"Bind To Emitter" );
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// Appearance table.
		AddVariable( table,parts.visualTable,"Appearance" );
		AddVariable( parts.visualTable,parts.blendType,"Blend Type" );
		AddVariable( parts.visualTable,parts.fadeInTime,"FadeInTime" );
		AddVariable( parts.visualTable,parts.colorStart,"Start Color",IVariable::DT_COLOR );
		AddVariable( parts.visualTable,parts.colorEnd,"End Color",IVariable::DT_COLOR );
		AddVariable( parts.visualTable,parts.no_underwater,"Not Draw Underwater" );
		AddVariable( parts.visualTable,parts.frameCount,"Frames Count" );
		AddVariable( parts.visualTable,parts.tailLength,"Tail" );
		AddVariable( parts.visualTable,parts.tailSteps,"TailSteps" );
		AddVariable( parts.visualTable,parts.stretch,"Stretch" );
		AddVariable( parts.visualTable,parts.drawLast,"DrawLast" );
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// Size table.
		AddVariable( table,parts.sizeTable,"Size" );
		AddVariable( parts.sizeTable,parts.size,"Size" );
		AddVariable( parts.sizeTable,parts.size_variation,"Size Variation",IVariable::DT_PERCENT );
		AddVariable( parts.sizeTable,parts.sizeSpeed,"Size Speed" );
		AddVariable( parts.sizeTable,parts.sizeLinear,"Size Speed Linear" );
		AddVariable( parts.sizeTable,parts.sizeFadeIn,"Size FadeIn" );
		AddVariable( parts.sizeTable,parts.sizeFadeOut,"Size FadeOut" );
		//////////////////////////////////////////////////////////////////////////
		
		//////////////////////////////////////////////////////////////////////////
		// Rotate table.
		AddVariable( table,parts.rotateTable,"Rotate" );
		AddVariable( parts.rotateTable,parts.rotation,"Rotation" );
		AddVariable( parts.rotateTable,parts.rotation_variation,"Rotation Variation",IVariable::DT_PERCENT );
		AddVariable( parts.rotateTable,parts.angles,"Initial Angles" );
		AddVariable( parts.rotateTable,parts.angles_variation,"Initial Angles Variation",IVariable::DT_PERCENT );
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		// Advanced table.
		AddVariable( table,parts.advancedTable,"Advanced" );
		AddVariable( parts.advancedTable,parts.realPhysics,"Use Real Physics" );
		AddVariable( parts.advancedTable,parts.rigid_body,"Rigid Body Physics" );
		AddVariable( parts.advancedTable,parts.bounciness,"Bounciness" );
		AddVariable( parts.advancedTable,parts.object_scale,"Object Scale" );
		//////////////////////////////////////////////////////////////////////////

	}
	
	//////////////////////////////////////////////////////////////////////////
	void GetParticleParamsFromUI( ParticleParams &params,const SParticleTableUI &ui )
	{
		params.nParticleFlags = ui.type & (0x7);
		if (ui.sizeLinear)
			params.nParticleFlags |= PART_FLAG_SIZE_LINEAR;
		if (ui.focus_plane)
			params.nParticleFlags |= PART_FLAG_FOCUS_PLANE;
		if (ui.no_underwater)
			params.nParticleFlags |= PART_FLAG_NO_DRAW_UNDERWATER;
		if (ui.rigid_body)
			params.nParticleFlags |= PART_FLAG_RIGIDBODY;
		if (ui.use_gravity_direction)
			params.nParticleFlags |= PART_FLAG_SPEED_IN_GRAVITY_DIRECTION;
		if (ui.bind_pos_to_emitter)
			params.nParticleFlags |= PART_FLAG_BIND_POSITION_TO_EMITTER;

		params.eBlendType = (ParticleBlendType)((int)ui.blendType);
		params.fFocus = ui.focus;
		params.fSpeed = ui.speed;
		params.fSpeed.variation = ui.speed_variation;
		params.fSpeedFadeOut = ui.speed_fadeout;
		params.fSize = ui.size;
		params.fSize.variation = ui.size_variation;
		params.fSpeedAccel = ui.speed_accel;
		params.fAirResistance = ui.speed_airResistance;
		params.fSizeSpeed = ui.sizeSpeed;
		params.fSizeFadeIn = ui.sizeFadeIn;
		params.fSizeFadeOut = ui.sizeFadeOut;
		params.fObjectScale = ui.object_scale;
		params.nCount = ui.count;
		params.fLifeTime = ui.lifeTime;
		params.fLifeTime.variation = ui.lifeTime_variation;
		params.fFadeInTime = ui.fadeInTime;
		params.nTexAnimFramesCount = ui.frameCount;
		params.fTailLenght = ui.tailLength;
		params.nTailSteps = ui.tailSteps;
		params.fStretch = ui.stretch;
		params.bRealPhysics = ui.realPhysics;
		params.nDrawLast = ui.drawLast;
		params.fBouncenes = ui.bounciness;
		params.fTurbulenceSize = ui.turbulenceSize;
		params.fTurbulenceSpeed = ui.turbulenceSpeed;
		//params.fPosRandomOffset = ui.posRandomOffset;
		params.vColorStart = (Vec3)ui.colorStart;
		params.vColorEnd = (Vec3)ui.colorEnd;
		params.vRotation = (Vec3)ui.rotation;
		params.vRotation.variation = ui.rotation_variation;
		params.vInitAngles = (Vec3)ui.angles;
		params.vInitAngles.variation = ui.angles_variation;
		params.vGravity = (Vec3)ui.gravity;
	}

	//////////////////////////////////////////////////////////////////////////
	void SetParticleParamsToUI( const ParticleParams &params,SParticleTableUI &ui )
	{
		ui.type = params.nParticleFlags & (0x7);
		if (params.nParticleFlags & PART_FLAG_SIZE_LINEAR)
			ui.sizeLinear = true;
		else
			ui.sizeLinear = false;
		if (params.nParticleFlags & PART_FLAG_FOCUS_PLANE)
			ui.focus_plane = true;
		else
			ui.focus_plane = false;
		if (params.nParticleFlags & PART_FLAG_NO_DRAW_UNDERWATER)
			ui.no_underwater = true;
		else
			ui.no_underwater = false;
		if (params.nParticleFlags & PART_FLAG_RIGIDBODY)
			ui.rigid_body = true;
		else
			ui.rigid_body = false;
		if (params.nParticleFlags & PART_FLAG_SPEED_IN_GRAVITY_DIRECTION)
			ui.use_gravity_direction = true;
		else
			ui.use_gravity_direction = false;
		if (params.nParticleFlags & PART_FLAG_BIND_POSITION_TO_EMITTER)
			ui.bind_pos_to_emitter = true;
		else
			ui.bind_pos_to_emitter = false;

		ui.blendType = params.eBlendType;
		ui.focus = params.fFocus;
		ui.speed = params.fSpeed;
		ui.speed_variation = params.fSpeed.variation;
		ui.speed_fadeout = params.fSpeedFadeOut;
		ui.speed_accel = params.fSpeedAccel;
		ui.speed_airResistance = params.fAirResistance;
		ui.size = params.fSize;
		ui.size_variation = params.fSize.variation;
		ui.sizeSpeed = params.fSizeSpeed;
		ui.sizeFadeIn = params.fSizeFadeIn;
		ui.sizeFadeOut = params.fSizeFadeOut;
		ui.object_scale = params.fObjectScale;
		ui.count = params.nCount;
		ui.lifeTime = params.fLifeTime;
		ui.lifeTime_variation = params.fLifeTime.variation;
		ui.fadeInTime = params.fFadeInTime;
		ui.frameCount = params.nTexAnimFramesCount;
		ui.tailLength = params.fTailLenght;
		ui.tailSteps = params.nTailSteps;
		ui.stretch = params.fStretch;
		ui.realPhysics = params.bRealPhysics;
		ui.drawLast = params.nDrawLast;
		ui.bounciness = params.fBouncenes;
		ui.turbulenceSize = params.fTurbulenceSize;
		ui.turbulenceSpeed = params.fTurbulenceSpeed;
		//ui.posRandomOffset = params.fPosRandomOffset;
		ui.colorStart = params.vColorStart;
		ui.colorEnd = params.vColorEnd;
		ui.rotation = params.vRotation;
		ui.rotation_variation = params.vRotation.variation;
		ui.angles = params.vInitAngles;
		ui.angles_variation = params.vInitAngles.variation;
		ui.gravity = params.vGravity;
	}

	//////////////////////////////////////////////////////////////////////////
	void SetFromParticles( CParticleItem *pParticles )
	{
		IParticleEffect *pEffect = pParticles->GetEffect();
		if (!pEffect)
			return;
		SetParticleParamsToUI( pEffect->GetParticleParams(PRIMARY_SYSTEM),uiParams[PRIMARY_SYSTEM] );
		SetParticleParamsToUI( pEffect->GetParticleParams(CHILD_SYSTEM),uiParams[CHILD_SYSTEM] );
		uiParams[PRIMARY_SYSTEM].texture = pEffect->GetTexture(PRIMARY_SYSTEM);
		uiParams[CHILD_SYSTEM].texture = pEffect->GetTexture(CHILD_SYSTEM);
		uiParams[PRIMARY_SYSTEM].geometry = pEffect->GetGeometry(PRIMARY_SYSTEM);
		uiParams[CHILD_SYSTEM].geometry = pEffect->GetGeometry(CHILD_SYSTEM);
		
		uiParams[PRIMARY_SYSTEM].material = pEffect->GetMaterialName(PRIMARY_SYSTEM);
		uiParams[CHILD_SYSTEM].material = pEffect->GetMaterialName(CHILD_SYSTEM);

		bActive = pEffect->IsEnabled();

		{
			ParticleParams &params = pEffect->GetParticleParams(PRIMARY_SYSTEM);
			childSpawnPeriod = pEffect->GetParticleParams(PRIMARY_SYSTEM).fChildSpawnPeriod;
			childSpawnTime = pEffect->GetParticleParams(PRIMARY_SYSTEM).fChildSpawnTime;
			positionOffset = params.vPositionOffset;
			randomPositionOffset = params.vRandomPositionOffset;
			spawnPeriod = params.fSpawnPeriod;
			spawnDelay = params.fSpawnDelay;
			spawnDelay_variation = params.fSpawnDelay.variation;
			emitterLifeTime = params.fEmitterLifeTime;
			emitterLifeTime_variation = params.fEmitterLifeTime.variation;
		}
		IParticleEffect::SoundParams soundParams;
		pEffect->GetSoundParams( soundParams );
		soundFilename = soundParams.szSound;
		soundVolume = soundParams.volume;
		soundMinRange = soundParams.minRadius;
		soundMaxRange = soundParams.maxRadius;
		soundLoop = soundParams.bLoop;
		soundEverySpawn = soundParams.bOnEverySpawn;
	}

	//////////////////////////////////////////////////////////////////////////
	void SetToParticles( CParticleItem *pParticles )
	{
		IParticleEffect *pEffect = pParticles->GetEffect();
		if (!pEffect)
			return;
		GetParticleParamsFromUI( pEffect->GetParticleParams(PRIMARY_SYSTEM),uiParams[PRIMARY_SYSTEM] );
		GetParticleParamsFromUI( pEffect->GetParticleParams(CHILD_SYSTEM),uiParams[CHILD_SYSTEM] );
		pEffect->SetTexture( PRIMARY_SYSTEM,(CString)uiParams[PRIMARY_SYSTEM].texture );
		pEffect->SetTexture( CHILD_SYSTEM,(CString)uiParams[CHILD_SYSTEM].texture );
		pEffect->SetGeometry( PRIMARY_SYSTEM,(CString)uiParams[PRIMARY_SYSTEM].geometry );
		pEffect->SetGeometry( CHILD_SYSTEM,(CString)uiParams[CHILD_SYSTEM].geometry );

		pEffect->SetEnabled( bActive );

		CString primaryMaterial = uiParams[PRIMARY_SYSTEM].material;
		CString childMaterial = uiParams[CHILD_SYSTEM].material;

		pEffect->SetMaterialName( PRIMARY_SYSTEM,(CString)uiParams[PRIMARY_SYSTEM].material );
		pEffect->SetMaterialName( CHILD_SYSTEM,(CString)uiParams[CHILD_SYSTEM].material );

		{
			ParticleParams &params = pEffect->GetParticleParams(PRIMARY_SYSTEM);
			pEffect->GetParticleParams(PRIMARY_SYSTEM).fChildSpawnPeriod = childSpawnPeriod;
			pEffect->GetParticleParams(PRIMARY_SYSTEM).fChildSpawnTime = childSpawnTime;
			params.vPositionOffset = (Vec3)positionOffset;
			params.vRandomPositionOffset = (Vec3)randomPositionOffset;
			params.fSpawnPeriod = spawnPeriod;
			params.fSpawnDelay = spawnDelay;
			params.fSpawnDelay.variation = spawnDelay_variation;
			params.fEmitterLifeTime = emitterLifeTime;
			params.fEmitterLifeTime.variation = emitterLifeTime_variation;
		}

		IParticleEffect::SoundParams soundParams;
		soundParams.szSound = (CString)soundFilename;
		soundParams.volume = soundVolume;
		soundParams.minRadius = soundMinRange;
		soundParams.maxRadius = soundMaxRange;
		soundParams.bLoop = soundLoop;
		soundParams.bOnEverySpawn = soundEverySpawn;
		pEffect->SetSoundParams( soundParams );
		
		// Update particles.
		pParticles->Update();
	}

private:
	//////////////////////////////////////////////////////////////////////////
  void AddVariable( CVariableArray &varArray,CVariableBase &var,const char *varName,unsigned char dataType=IVariable::DT_SIMPLE )
	{
		var.AddRef(); // Variables are local and must not be released by CVarBlock.
		if (varName)
			var.SetName(varName);
		var.SetDataType(dataType);
		if (m_onSetCallback)
			var.AddOnSetCallback(m_onSetCallback);
		varArray.AddChildVar(&var);
	}
	//////////////////////////////////////////////////////////////////////////
  void AddVariable( CVarBlock *vars,CVariableBase &var,const char *varName,unsigned char dataType=IVariable::DT_SIMPLE )
	{
		var.AddRef(); // Variables are local and must not be released by CVarBlock.
		if (varName)
			var.SetName(varName);
		var.SetDataType(dataType);
		if (m_onSetCallback)
			var.AddOnSetCallback(m_onSetCallback);
		vars->AddVariable(&var);
	}
};

static CParticleUIDefinition gParticleUI;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CParticlePickCallback : public IPickObjectCallback
{
public:
	CParticlePickCallback() { m_bActive = true; };
	//! Called when object picked.
	virtual void OnPick( CBaseObject *picked )
	{
		/*
		m_bActive = false;
		CParticleItem *pParticles = picked->GetParticle();
		if (pParticles)
			GetIEditor()->OpenParticleLibrary( pParticles );
			*/
		delete this;
	}
	//! Called when pick mode cancelled.
	virtual void OnCancelPick()
	{
		m_bActive = false;
		delete this;
	}
	//! Return true if specified object is pickable.
	virtual bool OnPickFilter( CBaseObject *filterObject )
	{
		/*
		// Check if object have material.
		if (filterObject->GetParticle())
			return true;
		*/
		return false;
	}
	static bool IsActive() { return m_bActive; };
private:
	static bool m_bActive;
};
bool CParticlePickCallback::m_bActive = false;
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// CParticleDialog implementation.
//////////////////////////////////////////////////////////////////////////
CParticleDialog::CParticleDialog( CWnd *pParent )
	: CBaseLibraryDialog(IDD_DB_ENTITY, pParent)
{
	m_pPartManager = GetIEditor()->GetParticleManager();
	m_pItemManager = m_pPartManager;

	m_bRealtimePreviewUpdate = true;
	m_pGeometry = 0;
	m_pEntityRender = 0;
	
	m_drawType = DRAW_BOX;
	m_geometryFile = EDITOR_OBJECTS_PATH + "MtlBox.cgf";
	m_bOwnGeometry = true;

	m_dragImage = 0;
	m_hDropItem = 0;

	m_hCursorDefault = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	m_hCursorCreate = AfxGetApp()->LoadCursor( IDC_HIT_CURSOR );
	m_hCursorReplace = AfxGetApp()->LoadCursor(IDC_HAND_INTERNAL);

	// Immidiatly create dialog.
	Create( IDD_DB_ENTITY,pParent );
}

CParticleDialog::~CParticleDialog()
{
}

void CParticleDialog::DoDataExchange(CDataExchange* pDX)
{
	CBaseLibraryDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CParticleDialog, CBaseLibraryDialog)
	ON_COMMAND( ID_DB_ADD,OnAddItem )
	ON_COMMAND( ID_DB_PLAY,OnPlay )

	ON_COMMAND( ID_DB_MTL_DRAWSELECTED,OnDrawSelection )
	ON_COMMAND( ID_DB_MTL_DRAWBOX,OnDrawBox )
	ON_COMMAND( ID_DB_MTL_DRAWSPHERE,OnDrawSphere )
	ON_COMMAND( ID_DB_MTL_DRAWTEAPOT,OnDrawTeapot )
	ON_UPDATE_COMMAND_UI( ID_DB_PLAY,OnUpdatePlay )

	ON_COMMAND( ID_DB_SELECTASSIGNEDOBJECTS,OnSelectAssignedObjects )
	ON_COMMAND( ID_DB_MTL_ASSIGNTOSELECTION,OnAssignParticleToSelection )
	ON_COMMAND( ID_DB_MTL_GETFROMSELECTION,OnGetParticleFromSelection )
	ON_COMMAND( ID_DB_MTL_RESETMATERIAL,OnResetParticleOnSelection )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_ASSIGNTOSELECTION,OnUpdateAssignMtlToSelection )
	ON_UPDATE_COMMAND_UI( ID_DB_SELECTASSIGNEDOBJECTS,OnUpdateSelected )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_GETFROMSELECTION,OnUpdateObjectSelected )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_RESETMATERIAL,OnUpdateObjectSelected )

	ON_COMMAND( ID_DB_MTL_ADDSUBMTL,OnAddSubItem )
	ON_COMMAND( ID_DB_MTL_DELSUBMTL,OnDelSubItem )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_ADDSUBMTL,OnUpdateSelected )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_DELSUBMTL,OnUpdateSelected )

	ON_COMMAND( ID_DB_MTL_PICK,OnPickMtl )
	ON_UPDATE_COMMAND_UI( ID_DB_MTL_PICK,OnUpdatePickMtl )

	ON_NOTIFY(TVN_BEGINDRAG, IDC_PARTICLES_TREE, OnBeginDrag)
	ON_NOTIFY(NM_RCLICK , IDC_PARTICLES_TREE, OnNotifyMtlTreeRClick)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnDestroy()
{
	int temp;
	int HSplitter,VSplitter;
	m_wndHSplitter.GetRowInfo( 0,HSplitter,temp );
	m_wndVSplitter.GetColumnInfo( 0,VSplitter,temp );
	AfxGetApp()->WriteProfileInt("Dialogs\\Particles","HSplitter",HSplitter );
	AfxGetApp()->WriteProfileInt("Dialogs\\Particles","VSplitter",VSplitter );

	//ReleaseGeometry();
	CBaseLibraryDialog::OnDestroy();
}

// CTVSelectKeyDialog message handlers
BOOL CParticleDialog::OnInitDialog()
{
	CBaseLibraryDialog::OnInitDialog();

	InitToolbar();

	CRect rc;
	GetClientRect(rc);
	//int h2 = rc.Height()/2;
	int h2 = 200;

	int HSplitter = AfxGetApp()->GetProfileInt("Dialogs\\Particles","HSplitter",200 );
	int VSplitter = AfxGetApp()->GetProfileInt("Dialogs\\Particles","VSplitter",200 );

	m_wndVSplitter.CreateStatic( this,1,2,WS_CHILD|WS_VISIBLE );
	m_wndHSplitter.CreateStatic( &m_wndVSplitter,2,1,WS_CHILD|WS_VISIBLE );

	//m_imageList.Create(IDB_PARTICLES_TREE, 16, 1, RGB (255, 0, 255));
	CMFCUtils::LoadTrueColorImageList( m_imageList,IDB_PARTICLES_TREE,16,RGB(255,0,255) );

	// TreeCtrl must be already created.
	m_treeCtrl.SetParent( &m_wndVSplitter );
	m_treeCtrl.SetImageList(&m_imageList,TVSIL_NORMAL);

	m_previewCtrl.Create( &m_wndHSplitter,rc,WS_CHILD|WS_VISIBLE );
	m_previewCtrl.SetGrid(true);
	m_previewCtrl.EnableUpdate( true );

	m_propsCtrl.Create( WS_VISIBLE|WS_CHILD|WS_BORDER,rc,&m_wndHSplitter,2 );
	m_vars = gParticleUI.CreateVars();
	m_propsCtrl.AddVarBlock( m_vars );
	m_propsCtrl.ExpandAllChilds( m_propsCtrl.GetRootItem(),false );
	m_propsCtrl.EnableWindow( FALSE );

	m_wndHSplitter.SetPane( 0,0,&m_previewCtrl,CSize(100,HSplitter) );
	m_wndHSplitter.SetPane( 1,0,&m_propsCtrl,CSize(100,HSplitter) );

	m_wndVSplitter.SetPane( 0,0,&m_treeCtrl,CSize(VSplitter,100) );
	m_wndVSplitter.SetPane( 0,1,&m_wndHSplitter,CSize(VSplitter,100) );

	RecalcLayout();

	ReloadLibs();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
UINT CParticleDialog::GetDialogMenuID()
{
	return IDR_DB_ENTITY;
};

//////////////////////////////////////////////////////////////////////////
// Create the toolbar
void CParticleDialog::InitToolbar()
{
	VERIFY( m_toolbar.CreateEx(this, TBSTYLE_FLAT|TBSTYLE_WRAPABLE,
		WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC) );
	VERIFY( m_toolbar.LoadToolBar24(IDR_DB_MATERIAL_BAR) );

	// Resize the toolbar
	CRect rc;
	GetClientRect(rc);
	m_toolbar.SetWindowPos(NULL, 0, 0, rc.right, 70, SWP_NOZORDER);
	CSize sz = m_toolbar.CalcDynamicLayout(TRUE,TRUE);
	m_toolbar.SetButtonStyle( m_toolbar.CommandToIndex(ID_DB_PLAY),TBBS_CHECKBOX );

	CBaseLibraryDialog::InitToolbar();
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnSize(UINT nType, int cx, int cy)
{
	// resize splitter window.
	if (m_wndVSplitter.m_hWnd)
	{
		CRect rc;
		GetClientRect(rc);
		m_wndVSplitter.MoveWindow(rc,FALSE);
	}
	CBaseLibraryDialog::OnSize(nType, cx, cy);
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnNewDocument()
{
	//ReleaseGeometry();
	CBaseLibraryDialog::OnNewDocument();
};

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnLoadDocument()
{
	//m_pPartManager->SetCurrentParticle( 0 );
	//ReleaseGeometry();
	CBaseLibraryDialog::OnLoadDocument();
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnCloseDocument()
{
	//m_pPartManager->SetCurrentParticle( 0 );
	//ReleaseGeometry();
	CBaseLibraryDialog::OnCloseDocument();
}

//////////////////////////////////////////////////////////////////////////
HTREEITEM CParticleDialog::InsertItemToTree( CBaseLibraryItem *pItem,HTREEITEM hParent )
{
	CParticleItem *pParticles = (CParticleItem*)pItem;

	if (pParticles->GetParent())
	{
		if (!hParent || hParent == TVI_ROOT || m_treeCtrl.GetItemData(hParent) == 0)
			return 0;
	}

	HTREEITEM hMtlItem = CBaseLibraryDialog::InsertItemToTree( pItem,hParent );

	for (int i = 0; i < pParticles->GetChildCount(); i++)
	{
		CParticleItem *pSubItem = pParticles->GetChild(i);
		InsertItemToTree( pSubItem,hMtlItem );
	}
	return hMtlItem;
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnAddItem()
{
	if (!m_pLibrary)
		return;

	CStringGroupDlg dlg( _T("New Particle Name"),this );
	dlg.SetGroup( m_selectedGroup );
	//dlg.SetString( entityClass );
	if (dlg.DoModal() != IDOK || dlg.GetString().IsEmpty())
	{
		return;
	}

	CString fullName = m_pItemManager->MakeFullItemName( m_pLibrary,dlg.GetGroup(),dlg.GetString() );
	if (m_pItemManager->FindItemByName( fullName ))
	{
		Warning( "Item with name %s already exist",(const char*)fullName );
		return;
	}

	CParticleItem *pParticles = (CParticleItem*)m_pItemManager->CreateItem( m_pLibrary );

	pParticles->SetDefaults();
	
	// Make prototype name.
	SetItemName( pParticles,dlg.GetGroup(),dlg.GetString() );
	pParticles->Update();

	ReloadItems();
	SelectItem( pParticles );
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::SetParticleVars( CParticleItem *pParticles )
{
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::SelectItem( CBaseLibraryItem *item,bool bForceReload )
{
	bool bChanged = item != m_pCurrentItem || bForceReload;
	CBaseLibraryDialog::SelectItem( item,bForceReload );
	
	if (!bChanged)
		return;

	// Empty preview control.
	m_previewCtrl.SetEntity(0);
	//m_pPartManager->SetCurrentParticle( (CParticleItem*)item );

	if (!item)
	{
		m_propsCtrl.EnableWindow(FALSE);
		return;
	}

	if (!m_pEntityRender)
	{
		//LoadGeometry( m_geometryFile );
	}
	if (m_pEntityRender)
		m_previewCtrl.SetEntity( m_pEntityRender );
	
	m_propsCtrl.EnableWindow(TRUE);
	m_propsCtrl.EnableUpdateCallback(false);

	// Render preview geometry with current material
	CParticleItem *pParticles = GetSelectedParticle();

	//AssignMtlToGeometry();

	// Update variables.
	m_propsCtrl.EnableUpdateCallback(false);
	gParticleUI.SetFromParticles( pParticles );
	m_propsCtrl.EnableUpdateCallback(true);

	//gParticleUI.m_onSetCallback = functor(*this,OnUpdateProperties);

	m_propsCtrl.SetUpdateCallback( functor(*this,&CParticleDialog::OnUpdateProperties) );
	m_propsCtrl.EnableUpdateCallback(true);
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::Update()
{
	if (!m_bRealtimePreviewUpdate)
		return;

	// Update preview control.
	if (m_pEntityRender)
	{
		m_previewCtrl.Update();
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnUpdateProperties( IVariable *var )
{
	CParticleItem *pParticles = GetSelectedParticle();
	if (!pParticles)
		return;

	gParticleUI.SetToParticles( pParticles );

	//AssignMtlToGeometry();

	GetIEditor()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnPlay()
{
	m_bRealtimePreviewUpdate = !m_bRealtimePreviewUpdate;
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnUpdatePlay( CCmdUI* pCmdUI )
{
	if (m_bRealtimePreviewUpdate)
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

//////////////////////////////////////////////////////////////////////////
CParticleItem* CParticleDialog::GetSelectedParticle()
{
	CBaseLibraryItem *pItem = m_pCurrentItem;
	return (CParticleItem*)pItem;
}

//////////////////////////////////////////////////////////////////////////
IStatObj* CParticleDialog::GetGeometryFromObject( CBaseObject *pObject )
{
	assert( pObject );

	if (pObject->IsKindOf(RUNTIME_CLASS(CBrushObject)))
	{
		CBrushObject *pBrushObj = (CBrushObject*)pObject;
		return pBrushObj->GetPrefabGeom();
	}
	if (pObject->IsKindOf(RUNTIME_CLASS(CEntity)))
	{
		CEntity *pEntityObj = (CEntity*)pObject;
		if (pEntityObj->GetIEntity())
		{
			return pEntityObj->GetIEntity()->GetIStatObj(0);
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
ICryCharInstance* CParticleDialog::GetCharacterFromObject( CBaseObject *pObject )
{
	if (pObject->IsKindOf(RUNTIME_CLASS(CEntity)))
	{
		CEntity *pEntityObj = (CEntity*)pObject;
		if (pEntityObj->GetIEntity())
		{
			return pEntityObj->GetIEntity()->GetCharInterface()->GetCharacter(0);
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnDrawSelection()
{
	//if (m_drawType == DRAW_SELECTION)
		//return;
	m_drawType = DRAW_SELECTION;
	
	m_geometryFile = "";
	//ReleaseGeometry();

	m_bOwnGeometry = false;
	CSelectionGroup *pSel = GetIEditor()->GetSelection();
	if (!pSel->IsEmpty())
	{
		IStatObj *pGeometry = GetGeometryFromObject( pSel->GetObject(0) );
		if (pGeometry)
		{
			//LoadGeometry( pGeometry->GetFileName() );
		}
		//AssignMtlToGeometry();
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnDrawBox()
{
	if (m_drawType == DRAW_BOX)
		return;
	m_drawType = DRAW_BOX;
	//LoadGeometry( EDITOR_OBJECTS_PATH+"MtlBox.cgf" );
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnDrawSphere()
{
	if (m_drawType == DRAW_SPHERE)
		return;
	m_drawType = DRAW_SPHERE;
	//LoadGeometry( EDITOR_OBJECTS_PATH+"MtlSphere.cgf" );
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnDrawTeapot()
{
	if (m_drawType == DRAW_TEAPOT)
		return;
	m_drawType = DRAW_TEAPOT;
	//LoadGeometry( EDITOR_OBJECTS_PATH+"MtlTeapot.cgf" );
}

//@FIXME
/*
//////////////////////////////////////////////////////////////////////////
void CParticleDialog::LoadGeometry( const CString &filename )
{
	m_geometryFile = filename;
	ReleaseGeometry();
	m_bOwnGeometry = true;
	m_pGeometry = GetIEditor()->Get3DEngine()->MakeObject( m_geometryFile );
	if (m_pGeometry)
	{
		m_pEntityRender = GetIEditor()->Get3DEngine()->CreateEntityRender();
		m_pEntityRender->SetEntityStatObj( 0,m_pGeometry );
		m_previewCtrl.SetEntity( m_pEntityRender );
		AssignMtlToGeometry();
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::ReleaseGeometry()
{
	//@FIXME
	m_previewCtrl.SetEntity(0);
	if (m_pEntityRender)
	{
		GetIEditor()->Get3DEngine()->DeleteEntityRender(m_pEntityRender);
		m_pEntityRender = 0;
	}
	if (m_pGeometry)
	{
		// Load test geometry.
		GetIEditor()->Get3DEngine()->ReleaseObject( m_pGeometry );
	}
	m_pGeometry = 0;
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::AssignMtlToGeometry()
{
	if (!m_pEntityRender)
		return;

	CParticleItem *pParticles = GetSelectedParticle();
	if (!pParticles)
		return;

	pParticles->AssignToEntity( m_pEntityRender );
}
	*/

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnAssignParticleToSelection()
{
	CParticleItem *pParticles = GetSelectedParticle();
	if (!pParticles)
		return;

	CUndo undo( "Assign ParticleEffect" );

	CSelectionGroup *pSel = GetIEditor()->GetSelection();
	if (!pSel->IsEmpty())
	{
		for (int i = 0; i < pSel->GetCount(); i++)
		{
			AssignParticleToEntity( pParticles,pSel->GetObject(i) );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnSelectAssignedObjects()
{
	//@FIXME
	/*
	CParticleItem *pParticles = GetSelectedParticle();
	if (!pParticles)
		return;

	CBaseObjectsArray objects;
	GetIEditor()->GetObjectManager()->GetObjects( objects );
	for (int i = 0; i < objects.size(); i++)
	{
		CBaseObject *pObject = objects[i];
		if (pObject->GetParticle() != pParticles)
			continue;
		if (pObject->IsHidden() || pObject->IsFrozen())
			continue;
		GetIEditor()->GetObjectManager()->SelectObject( pObject );
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnResetParticleOnSelection()
{
	CUndo undo( "Reset Particle" );

	//@FIXME
	/*
	CSelectionGroup *pSel = GetIEditor()->GetSelection();
	if (!pSel->IsEmpty())
	{
		for (int i = 0; i < pSel->GetCount(); i++)
		{
			pSel->GetObject(i)->SetParticle( 0 );
		}
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnGetParticleFromSelection()
{
	//@FIXME
	/*
	if (!m_pLibrary)
		return;

	CSelectionGroup *pSel = GetIEditor()->GetSelection();
	if (pSel->IsEmpty())
		return;

	for (int i = 0; i < pSel->GetCount(); i++)
	{
		CParticleItem *pParticles = pSel->GetObject(i)->GetParticle();
		if (pParticles)
		{
			SelectItem( pParticles );
			return;
		}
	}

	IStatObj *pGeometry = GetGeometryFromObject( pSel->GetObject(0) );
	ICryCharInstance* pCharacter = GetCharacterFromObject( pSel->GetObject(0) );

	if (!pGeometry && !pCharacter)
		return;
	
	// If nothing was selected.
	if (IDNO == AfxMessageBox( _T("Selected Object does not have Particle\r\nDo you want to create Particle for it?"),MB_YESNO|MB_APPLMODAL|MB_ICONQUESTION ))
	{
		return;
	}

  CStringGroupDlg dlg( _T("New Particle Name"),this );
	dlg.SetGroup( m_selectedGroup );
	
	CString uniqName;
	if (pCharacter)
	{
		uniqName = m_pItemManager->MakeUniqItemName(Path::GetFileName(pCharacter->GetModel()->GetFileName()));
	}
	else if (pGeometry)
	{
		uniqName = m_pItemManager->MakeUniqItemName(Path::GetFileName(pGeometry->GetFileName()));
	}
	dlg.SetString( uniqName );
	if (dlg.DoModal() != IDOK || dlg.GetString().IsEmpty())
	{
		return;
	}

	// Make new material from this object.
	CParticleItem *pParticles = (CParticleItem*)m_pItemManager->CreateItem( m_pLibrary );
	// Make prototype name.
	SetItemName( pParticles,dlg.GetGroup(),dlg.GetString() );
	if (pCharacter)
	{
		pParticles->AssignFromGeometry( pCharacter );
	}
	else if (pGeometry)
	{
		pParticles->AssignFromGeometry( pGeometry );
	}
	pParticles->Update();
	pSel->GetObject(0)->SetParticle( pParticles );
	ReloadItems();
	SelectItem( pParticles );
	*/
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::AssignParticleToEntity( CParticleItem *pItem,CBaseObject *pObject )
{
	assert(pItem);
	assert(pObject);
	if (pObject->IsKindOf(RUNTIME_CLASS(CEntity)))
	{
		CEntity *pEntity = (CEntity*)pObject;
		IVariable *pVar = 0;
		if (pEntity->GetProperties())
			pVar = pEntity->GetProperties()->FindVariable( "ParticleEffect" );
		if (pVar && pVar->GetType() == IVariable::STRING)
		{
			pVar->Set( pItem->GetFullName() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnAddSubItem()
{
	CParticleItem *pParticles = GetSelectedParticle();
	if (!pParticles)
		return;

	//if (pParticles->GetParent())
		//pParticles = pParticles->GetParent();

	CUndo undo( "Add Sub Particle" );

	CParticleItem *pSubItem = (CParticleItem*)m_pItemManager->CreateItem( m_pLibrary );
	pSubItem->SetDefaults();

	pParticles->AddChild( pSubItem );
	pSubItem->SetName( m_pItemManager->MakeUniqItemName(pParticles->GetName()) );
	pSubItem->Update();

	ReloadItems();
	SelectItem( pSubItem );
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnDelSubItem()
{
	CParticleItem *pSubItem = GetSelectedParticle();
	if (!pSubItem)
		return;

	CUndo undo( "Remove Sub Particle" );
	CParticleItem *pParticles = pSubItem->GetParent();
	if (pParticles)
	{
		pParticles->RemoveChild(pSubItem);
		m_pItemManager->DeleteItem( pSubItem );

		ReloadItems();
		SelectItem( pParticles );
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnUpdateAssignMtlToSelection( CCmdUI* pCmdUI )
{
	if (GetSelectedParticle() && !GetIEditor()->GetSelection()->IsEmpty())
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnUpdateObjectSelected( CCmdUI* pCmdUI )
{
	if (!GetIEditor()->GetSelection()->IsEmpty())
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	CParticleItem* pParticles = (CParticleItem*)m_treeCtrl.GetItemData(hItem);
	if (!pParticles)
		return;

	m_pDraggedMtl = pParticles;

	m_treeCtrl.Select( hItem,TVGN_CARET );

	m_hDropItem = 0;
	m_dragImage = m_treeCtrl.CreateDragImage( hItem );
	if (m_dragImage)
	{
		m_hDraggedItem = hItem;
		m_hDropItem = hItem;
		m_dragImage->BeginDrag(0, CPoint(-10, -10));

		CRect rc;
		AfxGetMainWnd()->GetWindowRect( rc );
		
		CPoint p = pNMTreeView->ptDrag;
		ClientToScreen( &p );
		p.x -= rc.left;
		p.y -= rc.top;
		
		m_dragImage->DragEnter( AfxGetMainWnd(),p );
		SetCapture();
		GetIEditor()->EnableUpdate( false );
	}
	
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_dragImage)
	{
		CPoint p;

		p = point;
		ClientToScreen( &p );
		m_treeCtrl.ScreenToClient( &p );

		TVHITTESTINFO hit;
		ZeroStruct(hit);
		hit.pt = p;
		HTREEITEM hHitItem = m_treeCtrl.HitTest( &hit );
		if (hHitItem)
		{
			if (m_hDropItem != hHitItem)
			{
				if (m_hDropItem)
					m_treeCtrl.SetItem( m_hDropItem,TVIF_STATE,0,0,0,0,TVIS_DROPHILITED,0 );
				// Set state of this item to drop target.
				m_treeCtrl.SetItem( hHitItem,TVIF_STATE,0,0,0,TVIS_DROPHILITED,TVIS_DROPHILITED,0 );
				m_hDropItem = hHitItem;
				m_treeCtrl.Invalidate();
			}
		}

		CRect rc;
		AfxGetMainWnd()->GetWindowRect( rc );
		p = point;
		ClientToScreen( &p );
		p.x -= rc.left;
		p.y -= rc.top;
		m_dragImage->DragMove( p );

		SetCursor( m_hCursorDefault );
		// Check if can drop here.
		{
			CPoint p;
			GetCursorPos( &p );
			CViewport* viewport = GetIEditor()->GetViewManager()->GetViewportAtPoint( p );
			if (viewport)
			{
				CPoint vp = p;
				viewport->ScreenToClient(&vp);
				ObjectHitInfo hit( viewport,vp );
				if (viewport->HitTest( vp,hit,0 ))
				{
					if (hit.object && hit.object->IsKindOf(RUNTIME_CLASS(CEntity)))
					{
						SetCursor( m_hCursorReplace );
					}
				}
			}
		}
	}

	CBaseLibraryDialog::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	//CXTResizeDialog::OnLButtonUp(nFlags, point);

	if (m_hDropItem)
	{
		m_treeCtrl.SetItem( m_hDropItem,TVIF_STATE,0,0,0,0,TVIS_DROPHILITED,0 );
		m_hDropItem = 0;
	}

	if (m_dragImage)
	{
		CPoint p;
		GetCursorPos( &p );

		GetIEditor()->EnableUpdate( true );

		m_dragImage->DragLeave( AfxGetMainWnd() );
		m_dragImage->EndDrag();
		delete m_dragImage;
		m_dragImage = 0;
		ReleaseCapture();

		CPoint treepoint = p;
		m_treeCtrl.ScreenToClient( &treepoint );

		CRect treeRc;
		m_treeCtrl.GetClientRect(treeRc);

		if (treeRc.PtInRect(treepoint))
		{
			// Droped inside tree.
			TVHITTESTINFO hit;
			ZeroStruct(hit);
			hit.pt = treepoint;
			HTREEITEM hHitItem = m_treeCtrl.HitTest( &hit );
			if (hHitItem)
			{
				DropToItem( hHitItem,m_hDraggedItem,m_pDraggedMtl );
				m_hDraggedItem = 0;
				m_pDraggedMtl = 0;
				return;
			}
			DropToItem( 0,m_hDraggedItem,m_pDraggedMtl );
		}
		else
		{
			// Not droped inside tree.

			CWnd *wnd = WindowFromPoint( p );

			CUndo undo( "Assign ParticleEffect" );

			CViewport* viewport = GetIEditor()->GetViewManager()->GetViewportAtPoint( p );
			if (viewport)
			{
				CPoint vp = p;
				viewport->ScreenToClient(&vp);
				// Drag and drop into one of views.
				// Start object creation.
				ObjectHitInfo hit( viewport,vp );
				if (viewport->HitTest( vp,hit,0 ))
				{
					if (hit.object)
					{
						// Only parent can be assigned.
						CParticleItem *pParticles = m_pDraggedMtl;
						AssignParticleToEntity( pParticles,hit.object );
					}
				}
			}
		}
		m_pDraggedMtl = 0;
	}
	m_pDraggedMtl = 0;
	m_hDraggedItem = 0;

	CBaseLibraryDialog::OnLButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnNotifyMtlTreeRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Show helper menu.
	CPoint point;

	CParticleItem *pParticles = 0;

	// Find node under mouse.
	GetCursorPos( &point );
	m_treeCtrl.ScreenToClient( &point );
	// Select the item that is at the point myPoint.
	UINT uFlags;
	HTREEITEM hItem = m_treeCtrl.HitTest(point,&uFlags);
	if ((hItem != NULL) && (TVHT_ONITEM & uFlags))
	{
		pParticles = (CParticleItem*)m_treeCtrl.GetItemData(hItem);
	}

	if (!pParticles)
		return;

	SelectItem( pParticles );

	// Create pop up menu.
	CMenu menu;
	menu.CreatePopupMenu();
	
	if (pParticles)
	{
		CClipboard clipboard;
		bool bNoPaste = clipboard.IsEmpty();
		int pasteFlags = 0;
		if (bNoPaste)
			pasteFlags |= MF_GRAYED;

		menu.AppendMenu( MF_STRING,ID_DB_CUT,"Cut" );
		menu.AppendMenu( MF_STRING,ID_DB_COPY,"Copy" );
		menu.AppendMenu( MF_STRING|pasteFlags,ID_DB_PASTE,"Paste" );
		menu.AppendMenu( MF_STRING,ID_DB_CLONE,"Clone" ); 
		menu.AppendMenu( MF_SEPARATOR,0,"" );
		menu.AppendMenu( MF_STRING,ID_DB_RENAME,"Rename" );
		menu.AppendMenu( MF_STRING,ID_DB_REMOVE,"Delete" );
		menu.AppendMenu( MF_SEPARATOR,0,"" );
		menu.AppendMenu( MF_STRING,ID_DB_MTL_ASSIGNTOSELECTION,"Assign to Selected Objects" );
		//menu.AppendMenu( MF_STRING,ID_DB_SELECTASSIGNEDOBJECTS,"Select Assigned Objects" );
		//menu.AppendMenu( MF_STRING,ID_DB_MTL_ADDSUBMTL,"Add Sub Particle" );
	}

	GetCursorPos( &point );
	menu.TrackPopupMenu( TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this );
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnPickMtl()
{
	if (!CParticlePickCallback::IsActive())
		GetIEditor()->PickObject( new CParticlePickCallback,0,"Pick Object to Select Particle" );
	else
		GetIEditor()->CancelPick();
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnUpdatePickMtl( CCmdUI* pCmdUI )
{
	if (CParticlePickCallback::IsActive())
	{
		pCmdUI->SetCheck(1);
	}
	else
	{
		pCmdUI->SetCheck(0);
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnCopy()
{
	CParticleItem *pParticles = GetSelectedParticle();
	if (pParticles)
	{
		XmlNodeRef node = new CXmlNode( "Particle" );
		CBaseLibraryItem::SerializeContext ctx(node,false);
		ctx.bCopyPaste = true;

		CClipboard clipboard;
		pParticles->Serialize( ctx );
		clipboard.Put( node );
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::OnPaste()
{
	if (!m_pLibrary)
		return;

	CParticleItem *pItem = GetSelectedParticle();
	if (!pItem)
		return;

	CClipboard clipboard;
	if (clipboard.IsEmpty())
		return;
	XmlNodeRef node = clipboard.Get();
	if (!node)
		return;

	if (strcmp(node->getTag(),"Particle") == 0)
	{
		bool bPasteWithChilds = false;
		//if (pItem->GetParent())
			//bPasteWithChilds = false;
		node->delAttr( "Name" );

		m_pPartManager->PasteToParticleItem( pItem,node,bPasteWithChilds );
		ReloadItems();
		SelectItem(pItem);

		/*
		CParticleItem *pParentItem = 0;
		CParticleItem *pSelItem = GetSelectedParticle();
		if (pSelItem)
		{
			pParentItem = pSelItem->GetParent();
		}
		// This is material node.
		CBaseLibrary *pLib = m_pLibrary;
		CParticleItem *pParticles = m_pPartManager->LoadParticleItem( (CParticleLibrary*)pLib,node,true );
		if (pParticles)
		{
			pParticles->SetName( m_pItemManager->MakeUniqItemName(pParticles->GetName()) );
			if (pParentItem)
			{
				pParentItem->AddChild( pParticles );
			}
			ReloadItems();
			SelectItem(pParticles);
		}
		*/
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleDialog::DropToItem( HTREEITEM hItem,HTREEITEM hSrcItem,CParticleItem *pParticles )
{
	pParticles->GetLibrary()->SetModified();

	TSmartPtr<CParticleItem> pHolder = pParticles; // Make usre its not release while we remove and add it back.

	if (!hItem)
	{
		// Detach from parent.
		if (pParticles->GetParent())
			pParticles->GetParent()->RemoveChild( pParticles );

		ReloadItems();
		SelectItem( pParticles );
		return;
	}

	CParticleItem* pTargetItem = (CParticleItem*)m_treeCtrl.GetItemData(hItem);
	if (!pTargetItem)
	{
		// This is group.
		
		// Detach from parent.
		if (pParticles->GetParent())
			pParticles->GetParent()->RemoveChild( pParticles );

		// Move item to different group.
		CString groupName = m_treeCtrl.GetItemText(hItem);
		SetItemName( pParticles,groupName,pParticles->GetShortName() );

		m_treeCtrl.DeleteItem( hSrcItem );
		InsertItemToTree( pParticles,hItem );
		return;
	}
	// Ignore itself or immidiate target.
	if (pTargetItem == pParticles || pTargetItem == pParticles->GetParent())
		return;



	// Detach from parent.
	if (pParticles->GetParent())
		pParticles->GetParent()->RemoveChild( pParticles );

	// Attach to target.
	pTargetItem->AddChild( pParticles );

	ReloadItems();
	SelectItem( pParticles );
}