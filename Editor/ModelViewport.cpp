// RenderViewport.cpp : implementation file
//

#include "stdafx.h"
#include "ModelViewport.h"
#include "Controls\AnimationToolBar.h"
#include "ModelViewPanel.h"
#include "ModelViewSubmeshPanel.h"
#include "PropertiesPanel.h"
#include "ICryAnimation.h"
#include "ThumbnailGenerator.h"
#include "GameEngine.h"
#include "CryCharMorphParams.h"
#include "CryCharAnimationParams.h"
#include "FileTypeUtils.h"
#include "CryCharFxTrailParams.h"

///#include <real.h>
//#include <vector3d.h>
#include <I3DEngine.h>
//#include <I3DIndoorEngine.h>
#include <ITimer.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CModelViewport,CRenderViewport)

#define SKYBOX_NAME "InfoRedGal"

namespace
{
	int s_varsPanelId = 0;
	CPropertiesPanel* s_varsPanel = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CModelViewport

CModelViewport::CModelViewport()
{
	m_camera.Init( 800,600,3.14f/4.0f );
	m_camera.SetZMin( 0.01f );
	m_camera.SetZMax( 10000 );
	//m_camera.SetPos( Vec3d(1262,743,25) );

	m_camera.SetPos( Vec3d(0,0,0) );
	m_camera.SetAngle( Vec3d(0,0,0) );

	m_bInRotateMode = false;
	m_bInMoveMode = false;

	m_camTarget(0,0,0);

	m_bNavigateBuilding = false;

	m_rollupIndex = 0;
	m_rollupIndex2 = 0;

	m_object = 0;
	m_character = 0;
	m_weaponModel = 0;
	m_attachedCharacter = 0;

	m_modelPanel = 0;
	m_modelSubmeshPanel = 0;

	m_camRadius = 10;

	m_moveSpeed = 0.1f;
	m_currTime = 0;

	m_objectAngles.Set(0,0,0);

	m_pRESky = 0;
	m_pSkyboxName = 0;
  m_pSkyBoxShader = NULL;

	m_attachBone = "weapon_bone";

	m_viewerPos.Set(0,0,0);
	m_viewerAngles.Set(0,0,0);

	// Init variable.
	mv_lighting = true;
	mv_lightDiffuseColor = Vec3(1,1,1);
	mv_lightSpecularColor = Vec3(1,1,1);
	mv_objectAmbientColor = Vec3(0.5,0.5,0.5);
	mv_backgroundColor = Vec3(0.5,0.5,0.5);
	mv_fov = 90;
	m_currLightDiffuseColor.Set(1,1,1);
	m_currLightSpecularColor.Set(1,1,1);

	// Create first light.
	CDLight *l = new CDLight;
  l->m_Origin = Vec3(10,10,10);
	l->m_Color = m_currLightDiffuseColor;
	l->m_SpecColor = m_currLightSpecularColor;
  l->m_fRadius = 100;
	l->m_fStartRadius = 1;
	l->m_fEndRadius = 100;
  l->m_Flags |= DLF_POINT;
	m_lights.push_back( l );

	// Register variables.
	m_vars.AddVariable( mv_wireframe,_T("Wireframe") );
	m_vars.AddVariable( mv_showGrid,_T("Grid") );

	m_vars.AddVariable( mv_lighting,_T("Lighting") );
	m_vars.AddVariable( mv_animateLights,_T("AnimLights") );
	m_vars.AddVariable( mv_2LightSources,"TwoLights",_T("2 Lights"),functor(*this,&CModelViewport::On2Lights) );
	
	m_vars.AddVariable( mv_objectAmbientColor,_T("ObjectAmbient"),functor(*this,&CModelViewport::OnLightColor),IVariable::DT_COLOR );
	m_vars.AddVariable( mv_lightDiffuseColor,_T("LightDiffuse"),functor(*this,&CModelViewport::OnLightColor),IVariable::DT_COLOR );
	m_vars.AddVariable( mv_lightSpecularColor,_T("LightSpecular"),functor(*this,&CModelViewport::OnLightColor),IVariable::DT_COLOR );
	m_vars.AddVariable( mv_backgroundColor,_T("BackgroundColor"),functor(*this,&CModelViewport::OnLightColor),IVariable::DT_COLOR );

	m_vars.AddVariable( mv_showShaders,_T("ShowShaders"),functor(*this,&CModelViewport::OnShowShaders) );
	m_vars.AddVariable( mv_showSkyBox,_T("ShowSkyBox") );

	m_vars.AddVariable( mv_showNormals,_T("ShowNormals"),functor(*this,&CModelViewport::OnShowNormals) );
	m_vars.AddVariable( mv_showTangents,_T("ShowTangents"),functor(*this,&CModelViewport::OnShowTangents) );
	
	m_vars.AddVariable( mv_showShadowVolumes,_T("ShowShadowVolumes"),functor(*this,&CModelViewport::OnShowShadowVolumes) );
	m_vars.AddVariable( mv_showTextureUsage,_T("ShowTextureUsage"),functor(*this,&CModelViewport::OnShowTextureUsage) );
	m_vars.AddVariable( mv_showAllTextures,_T("ShowAllTextures"),functor(*this,&CModelViewport::OnShowAllTextures) );

	m_vars.AddVariable( mv_disableLod,"NoLod",_T("No LOD") );
	m_vars.AddVariable( mv_disableVisibility,_T("DisableVisibilty"),functor(*this,&CModelViewport::OnDisableVisibility) );
	m_vars.AddVariable( mv_fov,_T("FOV") );
}

CModelViewport::~CModelViewport()
{
	for (int i = 0; i < m_lights.size(); i++)
		delete m_lights[i];

	// Save values to registry.
	XmlNodeRef node = new CXmlNode("Vars");
	m_vars.Serialize( node,false );
	AfxGetApp()->WriteProfileString( "PreviewSettings","Vars",node->getXML() );
}


BEGIN_MESSAGE_MAP(CModelViewport, CRenderViewport)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_ANIM_BACK, OnAnimBack)
	ON_COMMAND(ID_ANIM_FAST_BACK, OnAnimFastBack)
	ON_COMMAND(ID_ANIM_FAST_FORWARD, OnAnimFastForward)
	ON_COMMAND(ID_ANIM_FRONT, OnAnimFront)
	ON_COMMAND(ID_ANIM_PLAY, OnAnimPlay)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CModelViewport message handlers

int CModelViewport::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CRenderViewport::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetIEditor()->GetGameEngine()->SetLevelLoaded( true );

	if (m_renderer)
	{
		m_pRESky = (CRESky*)m_renderer->EF_CreateRE(eDATA_Sky);
		m_pRESky->m_fAlpha = 1.0f;

		m_pSkyboxName = GetIEditor()->GetSystem()->GetIConsole()->CreateVariable("ed_skyboxname","InfRedGal",0 );
    m_pSkyBoxShader = m_renderer->EF_LoadShader(m_pSkyboxName->GetString(), eSH_World, EF_SYSTEM);
  }

	// Serialize variables from registry.
	CString xml = AfxGetApp()->GetProfileString( "PreviewSettings","Vars" );
	if (!xml.IsEmpty())
	{
		XmlParser parser;
		XmlNodeRef node = parser.parseBuffer( xml );
		if (node)
		{
			m_vars.Serialize( node,true );
		}
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::ReleaseObject()
{
	if (m_object)
	{
		m_engine->ReleaseObject( m_object );
		m_object = 0;
	}

	if (m_character)
	{
		m_pAnimationSystem->RemoveCharacter( m_character );
		m_character = 0;
	}

	if (m_weaponModel)
	{
		m_engine->ReleaseObject( m_weaponModel );
		m_weaponModel = NULL;
	}

	if (m_attachedCharacter)
	{
		m_pAnimationSystem->RemoveCharacter(m_attachedCharacter);
		m_attachedCharacter = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::LoadObject( const CString &fileName,float scale )
{
	// Enable displaying of labels.
	GetIEditor()->SetConsoleVar( "r_DisplayInfo",1 );
	
	m_bNavigateBuilding = false;

	// Load object.
	char dir[1024];
	GetCurrentDirectory( 1024,dir );
	CString file = fileName;
	CString fileLo = file;
	fileLo.MakeLower();
	char *ldir = strlwr( dir );
	if (fileLo.Find(ldir)==0)
		file.Delete( 0,strlen(dir)+1 );

	bool reload = false;
	if (m_loadedFile == file)
		reload = true;
	m_loadedFile = file;
	
	SetName( CString("Model View - ") + file );
	
	ReleaseObject();

	// Do not reload textures.
	//if (m_renderer)
		//m_renderer->FreeResources(FRR_SHADERS|FRR_TEXTURES|FRR_RESTORE);

	if (!m_modelPanel)
	{
		m_modelPanel = new ModelViewPanel(this,this);
		m_rollupIndex = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,"Model View",m_modelPanel );
	}
	if (m_modelPanel)
	{
		m_modelPanel->SetFileName( fileName );
	}

	if (!s_varsPanel)
	{
		// Regidet variable pannel.
		s_varsPanel = new CPropertiesPanel(this);
		s_varsPanel->AddVars( m_vars.GetVarBlock() );
		s_varsPanelId = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,"Properties",s_varsPanel );
	}

	if (IsPreviewableFileType(file))
	{
		// Try Load character.
		m_character = m_pAnimationSystem->MakeCharacter( file );
		if (m_character)
		{
			if (m_character->NumSubmeshes())
			{
#if 1
				int nBone = m_character->GetModel()->GetBoneByName("Bip01 L Hand");
				m_character->NewFxTrail(0, CryCharFxTrailParams (nBone, m_renderer->LoadTexture("Textures\\guncloud.dds"), 0.3f, Vec3d(0,0,0), Vec3d(1,0,0), 500));
				nBone = m_character->GetModel()->GetBoneByName("Bip01 R Hand");
				m_character->NewFxTrail(1, CryCharFxTrailParams (nBone, m_renderer->LoadTexture("Textures\\haze_mask.dds"), 0.3f, Vec3d(0,0,0), Vec3d(1,0,0), 500));
#endif

				if (!m_modelSubmeshPanel)
				{
					m_modelSubmeshPanel = new ModelViewSubmeshPanel(this,this);
					m_rollupIndex2 = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,"Body Parts",m_modelSubmeshPanel );
				}
				else
					m_modelSubmeshPanel->ReinitSubmeshes();
			}


			if (m_attachedCharacter)
				m_character->AttachObjectToBone(m_attachedCharacter, m_attachBone, false);
			if (m_weaponModel)
				m_character->AttachObjectToBone( m_weaponModel,m_attachBone,false );

			// Generate thumbnail for this cgf.
			CThumbnailGenerator thumbGen;
			thumbGen.GenerateForFile( file );
			
			if (m_modelPanel && m_character)
			{
				SetCharacterUIInfo();
			}
			
			float radius = m_character->GetRadius();
			Vec3d center = m_character->GetCenter();
			
			if (!reload)
				m_camTarget = center;
			
			m_bboxMin = center-Vec3d(radius,radius,radius);
			m_bboxMax = center+Vec3d(radius,radius,radius);
			
			if (!reload)
				m_camRadius = center.z + radius;
		}
		else
		{
			LoadStaticObject( file );
		}
	}
	else
	{
    MessageBox( "Preview of this file type not supported","Preview Error",MB_OK|MB_ICONEXCLAMATION );
		return;
	}


	if (!reload)
	{
		m_objectAngles.Set(0,0,0);

		Vec3d v = m_bboxMax - m_bboxMin;
		float radius = v.Length()/2.0f;

		//m_camTarget = (m_bboxMax + m_bboxMin) * 0.5f;
		m_camTarget(0,0,0);
		//m_camTarget.x = m_camTarget.y = 0;

		//m_camTarget.Set( 0,0,2 );
		
		m_camRadius = radius*2;
		m_camAngles(0,0,0);
		
		m_camera.SetPos( Vec3d(0,m_camRadius,0) );
		m_camera.SetAngle( Vec3d(0,0,0) );
		SetCamera( m_camera );
	}

	GetIEditor()->SetConsoleVar( "r_DisplayInfo",1 );
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::LoadStaticObject(const CString &file )
{
	// Load Static object.
	m_object = m_engine->MakeObject( file,NULL,evs_ShareAndSortForCache );
	if(!m_object)
	{
		CLogFile::WriteLine( "Loading of object failed." );
		return;
	}

	// Generate thumbnail for this cgf.
	CThumbnailGenerator thumbGen;
	thumbGen.GenerateForFile( file );

	m_bboxMin = m_object->GetBoxMin();
	m_bboxMax = m_object->GetBoxMax();

	Vec3 pos;
	Matrix44 HelperMat;
	for (int i = 0; i < 1000; i++)
	{
		const char *helperName = m_object->GetHelperById( i,pos,&HelperMat );
		if (!helperName)
			break;
		if (strnicmp(helperName,"camera",strlen("camera")) == 0)
		{
//			reload = true;
			Vec3d v = m_bboxMax - m_bboxMin;
			float radius = v.Length()/2.0f;
			m_camTarget(0,0,0);
			m_camera.SetPos( pos );
			m_camRadius = pos.Length();
			//					rot.Normilize();

			//					m_camAngles = rot.GetEulerAngles();
			//m_camAngles.x *= 180.0f/PI;
			//m_camAngles.y *= 180.0f/PI;
			//m_camAngles.z *= 180.0f/PI;

			assert(0); // not tested !!!
			AffineParts affineParts;
			affineParts.SpectralDecompose(HelperMat);
			m_camAngles = RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(affineParts.rot)));
			/*
			Matrix m1;
			Matrix m2;
			m2.Identity();

			rot.GetMatrix(m1);
			Vec3d angles=m_camAngles;
			m2.Rotate(angles);
			*/

			m_camera.SetAngle( m_camAngles );
			SetCamera( m_camera );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::SetCharacterUIInfo()
{
	int i;
	// Fill the combo box with the name of the animation sequences
	m_modelPanel->ClearAnims();

	// [Sergiy] Add all animations to the animation list box/view
	IAnimationSet* pAnimations = m_character->GetModel()->GetAnimationSet();
	if (!pAnimations)
		return;

	// mapping name->length of animation
	typedef std::map<CString, float> AnimMap;
	AnimMap mapAnims;

	int numAnimations = pAnimations->Count();
	for (i = 0; i < numAnimations; ++i)
	{
		// the length of the animation is passed in seconds
		mapAnims.insert(AnimMap::value_type(pAnimations->GetName(i),pAnimations->GetLength(i)));
	}

	// now fill all the morph targets of subskins in
	for (i = 1; i < m_character->NumSubmeshes(); ++i)
	{
		ICryCharSubmesh* pSubmesh = m_character->GetSubmesh(i);
		if (!pSubmesh)
			continue;
		IAnimationSet* pAnimations = pSubmesh->GetModel()->GetAnimationSet();
		if (!pAnimations)
			continue;

		for (unsigned j = 0; j < pAnimations->CountMorphTargets(); ++j)
			mapAnims.insert (AnimMap::value_type(pAnimations->GetNameMorphTarget(i), 0));
	}

	for (AnimMap::const_iterator it = mapAnims.begin(); it != mapAnims.end(); ++it)
		m_modelPanel->AddAnimName	(it->first, it->second);
	
	
	// Fill the bone list
	m_modelPanel->ClearBones();

	ICryCharModel *pCharModel = m_character->GetModel();
	int numBones = pCharModel->NumBones();
	for (i = 0; i < numBones; i++)
	{
		const char *str = pCharModel->GetBoneName(i);
		if (str == NULL)
			break;
		if (strlen(str) > 0)
			m_modelPanel->AddBone(str);
	}
	m_modelPanel->SelectBone( m_attachBone );
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_bNavigateBuilding)
	{
		CRenderViewport::OnLButtonDown(nFlags,point);
		return;
	}

	m_bInRotateMode = true;
	m_mousePos = point;
	if (!m_bInMoveMode)
		SetCapture();
}

void CModelViewport::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bNavigateBuilding)
	{
		CRenderViewport::OnLButtonUp(nFlags,point);
		return;
	}

	m_bInRotateMode = false;
	if (!m_bInMoveMode)
		ReleaseCapture();
}

void CModelViewport::OnMButtonDown(UINT nFlags, CPoint point) 
{
	if (m_bNavigateBuilding)
	{
		CRenderViewport::OnMButtonDown(nFlags,point);
		return;
	}

	m_bInRotateMode = true;
	m_bInMoveMode = true;
	m_mousePos = point;
	//if (!m_bInMoveMode)
	SetCapture();
}

void CModelViewport::OnMButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bNavigateBuilding)
	{
		CRenderViewport::OnMButtonUp(nFlags,point);
		return;
	}

	m_bInRotateMode = false;
	m_bInMoveMode = false;
	ReleaseCapture();
}

void CModelViewport::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bNavigateBuilding)
	{
		CRenderViewport::OnMouseMove(nFlags,point);
		return;
	}

	bool bCtrl = nFlags & MK_CONTROL;

	// TODO: Add your message handler code here and/or call default
	CViewport::OnMouseMove(nFlags, point);
	if (point == m_mousePos)
		return;

	if (m_bInRotateMode && m_bInMoveMode)
	{
		// Zoom.
		Matrix44 m = m_camera.GetVCMatrixD3D9();
		//Vec3d xdir(m[0][0],m[1][0],m[2][0]);
		Vec3d xdir(0,0,0);
		//xdir.Normalize();

		Vec3d zdir(m[0][2],m[1][2],m[2][2]);
		zdir.Normalize();

		float step = 0.002f;
		float dx = (point.x-m_mousePos.x);
		float dy = (point.y-m_mousePos.y);
//		dx = pow(dx,1.05f );
		//dy = pow(dy,1.05f );
		//m_camera.SetPos( m_camera.GetPos() + ydir*(m_mousePos.y-point.y),xdir*(m_mousePos.x-point.x) );
		m_camera.SetPos( m_camera.GetPos() + step*xdir*dx +  step*zdir*dy );
		SetCamera( m_camera );

		CPoint pnt = m_mousePos;
		ClientToScreen( &pnt );
		SetCursorPos( pnt.x,pnt.y );
	}
	else if (m_bInRotateMode)
	{
		Vec3d angles( point.y-m_mousePos.y,0,-point.x+m_mousePos.x );
		// Look
		if (bCtrl)
		{
			Vec3d angles( point.y-m_mousePos.y,0,-point.x+m_mousePos.x );
			m_objectAngles += angles;
		}
		else
		{
			Vec3d angles( point.y-m_mousePos.y,0,-point.x+m_mousePos.x );
			//m_camera.SetAngle( m_camera.GetAngles() + angles*0.2f );
			m_camAngles += angles;

			SetOrbitAngles( m_camAngles );
		}
		
		CPoint pnt = m_mousePos;
		ClientToScreen( &pnt );
		SetCursorPos( pnt.x,pnt.y );
	}
	else if (m_bInMoveMode)
	{
		// Slide.
		Matrix44 m = m_camera.GetVCMatrixD3D9();
		Vec3d xdir(m[0][0],m[1][0],m[2][0]);
		Vec3d ydir(m[0][1],m[1][1],m[2][1]);
		xdir.Normalize();
		ydir.Normalize();

		float dist = (m_camera.GetPos() - m_camTarget).Length();

		float step = 0.001f;
		float dx = (point.x-m_mousePos.x);
		float dy = (point.y-m_mousePos.y);
		//dx =  pow( dx,1.2f );
		//dy =  pow( dy,1.2f );
		//m_camera.SetPos( m_camera.GetPos() + ydir*(m_mousePos.y-point.y),xdir*(m_mousePos.x-point.x) );
		m_camera.SetPos( m_camera.GetPos() - step*xdir*dx +  step*ydir*dy );

		SetCamera( m_camera );

		// Calc camera target.
	  Vec3d angles = m_camAngles;
	  angles=ConvertToRad(angles);

		//Matrix tm;	tm.Identity();
	  //tm.Rotate(angles);
		Matrix44 tm=ViewMatrix(angles);

		Vec3d v(0,0,dist);
		v = m_camera.GetPos() - tm*v;
		m_camTarget = v;


		m_mousePos = point;
	}
}

void CModelViewport::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if (m_bNavigateBuilding)
	{
		CRenderViewport::OnRButtonDown(nFlags,point);
		return;
	}

	m_bInMoveMode = true;
	m_mousePos = point;
	if (!m_bInRotateMode)
		SetCapture();
}

void CModelViewport::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bNavigateBuilding)
	{
		CRenderViewport::OnRButtonUp(nFlags,point);
		return;
	}

	m_bInMoveMode = false;
	m_mousePos = point;
	if (!m_bInRotateMode)
		ReleaseCapture();
}

BOOL CModelViewport::OnMouseWheel(UINT nFlags, short zDelta, CPoint point) 
{
	if (m_bNavigateBuilding)
	{
		return CRenderViewport::OnMouseWheel(nFlags,zDelta,point);
	}

	// TODO: Add your message handler code here and/or call default
	Matrix44 m = m_camera.GetVCMatrixD3D9();
	Vec3d zdir(m[0][2],m[1][2],m[2][2]);
	zdir.Normalize();

	//m_camera.SetPos( m_camera.GetPos() + ydir*(m_mousePos.y-point.y),xdir*(m_mousePos.x-point.x) );
	m_camera.SetPos( m_camera.GetPos() + 0.002f*zdir*(zDelta) );
	SetCamera( m_camera );
	
	return TRUE;
}


void CModelViewport::OnRender()
{
	ProcessKeys();
	if (m_renderer)
	{
		m_camera.SetPos( GetViewerPos() );
		m_camera.SetAngle( GetViewerAngles() );

		m_camera.SetZMin( 0.0101f );
		m_camera.SetZMax( 10000 );
		
		int w = m_rcClient.right - m_rcClient.left;
		int h = m_rcClient.bottom - m_rcClient.top;	
		float fAspectRatio = (float)h/(float)w;
		if (fAspectRatio > 1.6f) fAspectRatio = 1.6f;
		m_camera.Init( w,h,DEG2RAD(mv_fov),m_camera.GetZMax(),fAspectRatio );
		m_camera.Update();
		
		Vec3 clearColor = mv_backgroundColor;
		m_renderer->SetClearColor( clearColor );
		m_renderer->SetCamera( m_camera );

		m_renderer->ClearColorBuffer( clearColor );
		m_renderer->ClearDepthBuffer();
		m_renderer->ResetToDefault();

		// Render object.
		//GetIEditor()->GetSystem()->GetIConsole()->GetCVar("r_ShowLines")->Set( 1.0f );

		if (mv_wireframe)
		{
			m_renderer->SetPolygonMode(R_WIREFRAME_MODE);
			DrawModel();
		} else {
			m_renderer->SetPolygonMode(R_SOLID_MODE);
			DrawModel();
		}
	}
	if (!m_object && !m_character)
	{
		LoadObject( "Objects\\box.cgf",1 );
	}
}

void CModelViewport::DrawModel()
{
	m_renderer->EF_StartEf();

	if (mv_showSkyBox)
		DrawSkyBox();

	if (mv_showGrid)
		DrawGrid();
	
	if (mv_animateLights)
	{
		m_currTime += GetIEditor()->GetSystem()->GetITimer()->GetFrameTime()*1.5f;
		float t = m_currTime;
		float orbit = 5;
		///Vec3 lp( Vec3(sin(t)*orbit,cos(t*1.5f)*orbit,sin(t*0.2f)*orbit) );
		//Vec3 lp( Vec3(sin(t)*orbit,cos(t)*orbit,sin(t*0.2f)*orbit) );
		Vec3 lp = Vec3( Vec3(sin(t)*orbit,cos(t)*orbit,orbit) );
		m_lights[0]->m_Origin = lp;
		//m_lights[1]->m_Origin = Vec3(cos(t*1.5)*orbit,-sin(t)*orbit,-cos(t*0.3f)*orbit);
		if (m_lights.size() > 1)
		{
			orbit = 100;
			lp = Vec3( Vec3(sin(t*0.9)*orbit,cos(t*0.7)*orbit,orbit) );
			m_lights[1]->m_Origin = -lp;
		}
	}
	
	if (mv_lighting == true)
	{
		// Add lights.
		for (int i = 0; i < m_lights.size(); i++)
		{
			if (!m_lights[i])
				continue;
			m_renderer->EF_ADDDlight( m_lights[i] );
			m_renderer->SetMaterialColor( 1,1,0,1 );
		}
	}
	
	if (m_object)
	{
		SRendParams rp;
		rp.vPos = Vec3(0,0,0);
		rp.vAngles = m_objectAngles;
	  //changed by IVO
	  //rp.vAmbientColor = mv_objectAmbientColor.;
		mv_objectAmbientColor.Get(rp.vAmbientColor);

	

		if (mv_lighting == true)
			rp.nDLightMask = 0xFFFF;
		else
			rp.nDLightMask = 0;

		//changed by IVO
	  //rp.vAmbientColor = mv_objectAmbientColor;
		mv_objectAmbientColor.Get(rp.vAmbientColor);

		rp.dwFObjFlags  = FOB_IGNOREMATERIALAMBIENT;
    rp.dwFObjFlags |= FOB_TRANS_MASK;

		// calculate LOD
		float fDistance = m_camera.GetPos().Length();
		if (mv_disableLod)
			fDistance = 0;
//		int nLod = (int)(fDistance/(4*m_object->GetRadius()));
		int nLod = max(0,(int)(fDistance/(10.f*m_object->GetRadius())));

		// enable rendering of all attached lsources as local lsources
		for(int i=0; m_object->GetLightSources(i); i++)
			((CDLight*)m_object->GetLightSources(i))->m_Flags |= DLF_LOCAL;

		// render
		m_object->Render( rp, Vec3(zero), nLod );
	}

	if (m_character)
	{
		float fDistance = m_camera.GetPos().Length();
		if (mv_disableLod)
			fDistance = 0;
		m_character->Update();

		SRendParams rp;
		rp.vPos.Set(0,0,0);
		rp.vAngles = m_objectAngles;
		rp.fDistance = fDistance;
		rp.nDLightMask = 0xFFFFFFFF;
		//changed by ivo IVO
		//rp.vAmbientColor = mv_objectAmbientColor;
		mv_objectAmbientColor.Get(rp.vAmbientColor);
	
		rp.dwFObjFlags  = FOB_IGNOREMATERIALAMBIENT;
    rp.dwFObjFlags |= FOB_TRANS_MASK;

		// enable rendering of all attached lsources as local lsources
		for(int i=0; m_character->GetBoundLight(i); i++)
			((CDLight*)m_character->GetBoundLight(i))->m_Flags |= DLF_LOCAL;

		m_character->Draw( rp,Vec3(zero) );
	}
		
	m_renderer->EF_EndEf3D(SHDF_SORT);

  if(GetIEditor()->Get3DEngine())
  {
    float nTextPosX=101-20, nTextPosY=-2, nTextStepY=3;
    GetIEditor()->Get3DEngine()->DisplayInfo(nTextPosX, nTextPosY, nTextStepY);
  }

	m_renderer->EF_ClearLightsList();

	//////////////////////////////////////////////////////////////////////////
	// Draw lights.
	//////////////////////////////////////////////////////////////////////////
	if (mv_lighting == true)
	{
		for (int i = 0; i < m_lights.size(); i++)
		{
			if (!m_lights[i])
				continue;
			m_renderer->SetMaterialColor( 1,1,0,1 );
			m_renderer->DrawBall( m_lights[i]->m_Origin,0.2f );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::DrawSkyBox()
{
	CCObject * pObj = m_renderer->EF_GetObject(true, -1);
  pObj->m_Matrix.SetTranslationMat(m_camera.GetPos());

	if (m_pSkyboxName)
	{
		m_renderer->EF_AddEf(0, m_pRESky, m_pSkyBoxShader, NULL, pObj, 0, NULL,EFSLIST_PREPROCESS);
	}
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::DrawGrid()
{
	if (!m_renderer)
		return;

	//m_renderer->SetBlendMode();
	//m_renderer->EnableBlend( false );
	m_renderer->SetState(GS_DEPTHWRITE);

	float XR = 5;
	float YR = 5;

	m_renderer->SetMaterialColor( 0.6f,0.6f,0.6f,0.3f );
	// Draw grid.
	float step = 0.1f;
	for (float x = -XR; x < XR; x+=step)
	{
		if (fabs(x) > 0.01)
			m_renderer->DrawLine( Vec3d(x,-YR,0),Vec3d(x,YR,0) );
	}
	for (float y = -YR; y < YR; y+=step)
	{
		if (fabs(y) > 0.01)
			m_renderer->DrawLine( Vec3d(-XR,y,0),Vec3d(XR,y,0) );
	}

	// Draw axis.
	m_renderer->SetMaterialColor( 1,0,0,0.3f );
	m_renderer->DrawLine( Vec3d(-XR,0,0),Vec3d(XR,0,0) );
	
	m_renderer->SetMaterialColor( 0,1,0,0.3f );
	m_renderer->DrawLine( Vec3d(0,-YR,0),Vec3d(0,YR,0) );

	m_renderer->SetMaterialColor( 0,0,1,0.3f );
	m_renderer->DrawLine( Vec3d(0,0,-YR),Vec3d(0,0,YR) );

	//m_renderer->EnableBlend( false );

	//m_renderer->DrawLine( Vec3d(-m_bboxMin.x,y,0),Vec3d(XR,y,0) );
}

void CModelViewport::SetOrbitAngles( const Vec3d &ang )
{
	float dist = (m_camera.GetPos() - m_camTarget).Length();

	Vec3d cangles = ang;
	Vec3d v(0,0,dist);
  cangles=ConvertToRad(cangles);

// Matrix44 tm; tm.Identity();
 // tm.Rotate(cangles);
	Matrix44 tm=ViewMatrix(cangles);
	
	v = tm*v;

	m_camera.SetPos( v + m_camTarget );
	m_camera.SetAngle( m_camAngles );
	SetCamera( m_camera );
}

void CModelViewport::OnAnimBack() 
{
	// TODO: Add your command handler code here
}

void CModelViewport::OnAnimFastBack() 
{
	// TODO: Add your command handler code here
	
}

void CModelViewport::OnAnimFastForward() 
{
	// TODO: Add your command handler code here
	
}

void CModelViewport::OnAnimFront() 
{
	// TODO: Add your command handler code here
	
}

void CModelViewport::OnAnimPlay() 
{
	// TODO: Add your command handler code here
	if (m_modelPanel && m_character)
	{
		// the name of the currently selected animation
		CString strAnimName = m_modelPanel->GetCurrAnimName();
		
		// the interface to the set of animations
		ICryAnimationSet* pAnimations = m_character->GetModel()->GetAnimationSet();
		int numAnimations = pAnimations->Count();		
		// Find its animation data.
		for (int i=0; i < numAnimations; i++)
		{
			if (stricmp(strAnimName,pAnimations->GetName(i)) == 0)
			{
				pAnimations->SetLoop(i, m_modelPanel->GetLooped());
				break;
			}
		}

		StartAnimation (strAnimName);
	}
}

void CModelViewport::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	CRenderViewport::OnLButtonDblClk(nFlags, point);
	m_camTarget(0,0,0);
	SetOrbitAngles( m_camAngles );
}

void CModelViewport::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (m_character)
	{
		int index = -1;
		if (nChar >= 0x30 && nChar <= 0x39)
		{
			if (nChar == 0x30)
				index = 10;
			else
				index = nChar - 0x31;
		}
		if (nChar >= VK_NUMPAD0 && nChar <= VK_NUMPAD9)
		{
			index = nChar - VK_NUMPAD0 + 10;
			if (nChar == VK_NUMPAD0)
				index = 20;
		}
		//if (nFlags& MK_CONTROL

		IAnimationSet* pAnimations = m_character->GetModel()->GetAnimationSet();
		if (pAnimations)
		{
			int numAnimations = pAnimations->Count();
			if (index >= 0 && index < numAnimations)
			{
				const char *animName = pAnimations->GetName(index);
				StartAnimation (animName);
			}
		}
	}
	CRenderViewport::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CModelViewport::StartAnimation (const char*szName)
{
	if (m_modelPanel)
	{
		int layer = m_modelPanel->GetLayer();
		float fBlendInTime = m_modelPanel->GetBlendInTime();
		float fBlendOutTime = m_modelPanel->GetBlendOutTime();
		bool bSyncPhase = m_modelPanel->GetSynchronizePhase();
		if (szName[0] == '#')
		{
			CryCharMorphParams Params(fBlendInTime, (fBlendInTime+fBlendOutTime)/2, fBlendOutTime);
			Params.nFlags |= Params.FLAGS_RECURSIVE;
			m_character->StartMorph (szName, Params);
		}
		else
		{
			CryCharAnimationParams Params (fBlendInTime, fBlendOutTime,layer );
			if (bSyncPhase)
				Params.nFlags |= Params.FLAGS_SYNCHRONIZE_WITH_LAYER_0;
			Params.nFlags |= Params.FLAGS_RECURSIVE;
			m_character->StartAnimation ( szName,Params);
		}
	}
	else
	{
		// use the defaults in the interface instead of Editor's defaults
		m_character->StartAnimation(szName, CryCharAnimationParams());
	}
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::On2Lights( IVariable *var )
{
	if (mv_2LightSources)
	{
		CDLight *l = new CDLight;
		l->m_Origin = Vec3(-100,-100,-100);
		l->m_Color = m_currLightDiffuseColor;
		//l->m_AmbColor.r = 0; l->m_AmbColor.g = 0; l->m_AmbColor.b = 0; l->m_AmbColor.a = 0;
		l->m_SpecColor = m_currLightSpecularColor;
		l->m_fRadius = 1000;
		l->m_fStartRadius = 1;
		l->m_fEndRadius = 1000;
		l->m_Flags |= DLF_POINT;
		m_lights.resize(2);
		m_lights[1] = l;
	}
	else {
		m_lights.resize(1);
	}
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::OnShowNormals( IVariable *var )
{
	bool enable = mv_showNormals;
	GetIEditor()->SetConsoleVar( "r_ShowNormals",(enable)?1:0 );
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::OnShowTangents( IVariable *var )
{
	bool enable = mv_showTangents;
	GetIEditor()->SetConsoleVar( "r_ShowTangents",(enable)?1:0 );
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::OnShowShadowVolumes( IVariable *var )
{
	bool enable = mv_showShadowVolumes;
	GetIEditor()->SetConsoleVar( "CV_ind_VisualizeShadowVolumes",(enable)?1:0 );
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::OnShowTextureUsage( IVariable *var )
{
	bool enable = mv_showTextureUsage;
	GetIEditor()->SetConsoleVar( "r_LogUsedTextures",(enable)?2:0 );
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::OnShowAllTextures( IVariable *var )
{
	bool enable = mv_showAllTextures;
	GetIEditor()->SetConsoleVar( "r_LogUsedTextures",(enable)?4:0 );
}

bool CModelViewport::AddSubmesh (const CString& model)
{
	ICryCharModel* pModel = m_pAnimationSystem->LoadModel(model);
	if (!pModel)
	{
		AfxMessageBox("Cannot open model " + model + "\nSee the log for details");
		return false;
	}
	ICryCharSubmesh* pSubmesh = m_character->NewSubmesh (pModel, true);
	if (!pSubmesh)
	{
		AfxMessageBox("Cannot add body part " + model + "\nPlease check for skeleton compatibility\nSee the log for details");
		return false;
	}
	return true;
}

bool CModelViewport::SetSubmesh (int nSlot, const CString& model)
{
	ICryCharModel* pModel = m_pAnimationSystem->LoadModel(model);
	if (!pModel)
	{
		AfxMessageBox("Cannot open model " + model + "\nSee the log for details");
		return false;
	}
	ICryCharSubmesh* pSubmesh = m_character->NewSubmesh (nSlot, pModel, true);
	if (!pSubmesh)
	{
		AfxMessageBox("Cannot add body part " + model + "\nPlease check for skeleton compatibility\nSee the log for details");
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::AttachObject( const CString &model,const CString &bone )
{
	if (!m_character)
		return;

	/*
	if (m_weaponModel)
	{
		m_engine->ReleaseObject( m_weaponModel );
		m_weaponModel = NULL;
	}

	if (m_attachedCharacter)
	{
		m_engine->GetCryAnimationManager()->RemoveCharacter(m_attachedCharacter);
		m_attachedCharacter = NULL;
	}
	*/

	IBindable* pBindable = NULL;
	m_attachedCharacter = m_pAnimationSystem->MakeCharacter(model);
	if (m_attachedCharacter)
		pBindable = m_attachedCharacter;
	if (!m_attachedCharacter)
	{
		m_weaponModel = m_engine->MakeObject (model,NULL,evs_ShareAndSortForCache);
		if (m_weaponModel)
			pBindable = m_weaponModel;
	}

	m_attachBone = bone;
	if(!pBindable)
	{
		CString str;
		str.Format( "Loading of weapon model %s failed.",(const char*)model );
		AfxMessageBox( str );
		CLogFile::WriteLine( str );
		return;
	}
	if (m_attachedCharacter)
    m_character->AttachToBone ( pBindable,m_character->GetModel()->GetBoneByName(bone));
	else
		m_character->AttachObjectToBone(pBindable, bone, false);
}

void CModelViewport::StopAnimation(int nLayer)
{
	if (!m_character)
		return;

	m_character->StopAnimation(nLayer);
}

void CModelViewport::OnLightColor( IVariable *var )
{
	Vec3 d = mv_lightDiffuseColor;
	Vec3 s = mv_lightSpecularColor;
	m_currLightDiffuseColor.Set( d.x,d.y,d.z );
	m_currLightSpecularColor.Set( s.x,s.y,s.z );

	for (int i=0;i<m_lights.size();i++)
	{
		CDLight *l = m_lights[i];
		l->m_Color = m_currLightDiffuseColor;
		l->m_SpecColor = m_currLightSpecularColor;
	}
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::OnShowShaders( IVariable *var )
{
	bool bEnable = mv_showShaders;
	GetIEditor()->SetConsoleVar("r_ProfileShaders",bEnable);
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::OnDisableVisibility( IVariable *var )
{
	bool bDisableVisibility = mv_disableVisibility;
//	GetIEditor()->Get3DEngine()->GetBuildingManager()->EnableVisibility( !bDisableVisibility );
}
void CModelViewport::OnDestroy()
{
	ReleaseObject();
	if (m_pRESky)
		m_pRESky->Release();

	if (m_rollupIndex)
	{
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,m_rollupIndex );
		m_rollupIndex = 0;
	}
	if (m_rollupIndex2)
	{
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,m_rollupIndex2 );
		m_rollupIndex2 = 0;
	}
	if (s_varsPanelId)
	{
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,s_varsPanelId );
		s_varsPanelId = 0;
	}
	m_modelPanel = 0;
	m_modelSubmeshPanel = 0;
	s_varsPanel = 0;

	CRenderViewport::OnDestroy();
}

void CModelViewport::OnSubmeshSetChanged()
{
	SetCharacterUIInfo();
}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::OnActivate()
{

}

//////////////////////////////////////////////////////////////////////////
void CModelViewport::OnDeactivate()
{

}

void CModelViewport::Update()
{
	CRenderViewport::Update();
}