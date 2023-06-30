#include "RenderPCH.h"
#include "RendElement.h"
#include "CRESky.h"
#include "i3dengine.h"

void CRESky::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 0;
}

bool CRESky::mfDraw(SShader *ef, SShaderPass *sfm)
{   
 	int bPrevClipPl = gRenDev->m_RP.m_ClipPlaneEnabled;
	if (bPrevClipPl)
		gRenDev->EF_SetClipPlane(false, NULL, false);

	if(sfm >= &ef->m_Passes[1] && ef->m_Sky)
	{ // draw sky sphere vertices at pass 1
		bool bPrevFog = gRenDev->EnableFog(false);
		DrawSkySphere(ef->m_Sky->m_fSkyLayerHeight);
		gRenDev->EnableFog(bPrevFog);
		if (bPrevClipPl)
			gRenDev->EF_SetClipPlane(true, &gRenDev->m_RP.m_CurClipPlane.m_Normal.x, gRenDev->m_RP.m_bClipPlaneRefract);
		return true;
	}

	// pass 0 - skybox

	if (!ef->m_Sky || !ef->m_Sky->m_SkyBox[0])
	{
		if (bPrevClipPl)
			gRenDev->EF_SetClipPlane(true, &gRenDev->m_RP.m_CurClipPlane.m_Normal.x, gRenDev->m_RP.m_bClipPlaneRefract);
		return false;
	}

	bool bPrevFog = gRenDev->EnableFog(false);

	gRenDev->SetColorOp(eCO_MODULATE, eCO_MODULATE, eCA_Texture | (eCA_Constant<<3), eCA_Texture | (eCA_Constant<<3));

	if (gRenDev->m_bHeatVision)
		gRenDev->SetMaterialColor(0.3f,0.3f,0.3f,1.0f);
	else
		gRenDev->SetMaterialColor(1,1,1,m_fAlpha);
	if(m_fAlpha<1.f)
		gRenDev->EF_SetState(GS_NODEPTHTEST | GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
	else
		gRenDev->EF_SetState(GS_NODEPTHTEST);
  gRenDev->SetCullMode(R_CULL_BACK);

#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)
  CPShader *fpSky = NULL;
  if (gRenDev->m_RP.m_PersFlags & RBPF_HDR)
  {
    if (!(ef->m_Flags & EF_SKY_HDR))
      fpSky = PShaderForName(gRenDev->m_RP.m_PS_HDR_SkyFake, "CGRC_HDR_SkyFake_PS20");
    else
      fpSky = PShaderForName(gRenDev->m_RP.m_PS_HDR_Sky, "CGRC_HDR_Sky_PS20");
    if (fpSky)
      fpSky->mfSet(true, 0);
  }
#endif

	const float fSkyBoxSize = SKY_BOX_SIZE;

	{ // top
		struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
		{
			{Vec3(fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize), 1, 1.f-1},
			{Vec3(-fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize), 0, 1.f-1},
			{Vec3(fSkyBoxSize, fSkyBoxSize, fSkyBoxSize), 1, 1.f-0},
			{Vec3(-fSkyBoxSize, fSkyBoxSize, fSkyBoxSize), 0, 1.f-0}
		};

		gRenDev->SetTexture(ef->m_Sky->m_SkyBox[2]->m_Bind);
		gRenDev->SetTexClampMode(true);
		gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),4);
	}

	Vec3d camera = gRenDev->GetCamera().GetPos();
	camera.z = max(0,camera.z);

	float fWaterCamDiff = max(0,camera.z-m_fTerrainWaterLevel);

	float fMaxDist = iSystem->GetI3DEngine()->GetMaxViewDist()/1024.f;
	float P = (fWaterCamDiff)/128 + max(0,(fWaterCamDiff)*0.03f/fMaxDist);
	float D = (fWaterCamDiff)/10.0f*fSkyBoxSize/124.0f - P + 8;

	P*=m_fSkyBoxStretching;

	if(m_fTerrainWaterLevel>camera.z && SRendItem::m_RecurseLevel==1)
	{
		P = (fWaterCamDiff);
		D = (fWaterCamDiff);
	}

	{ // s
		struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
		{ 
			Vec3(-fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize), 1.0, 1.f-1.0,
			Vec3(fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize), 0.0, 1.f-1.0,
			Vec3(-fSkyBoxSize,-fSkyBoxSize,-P),           1.0, 1.f-0.5,
			Vec3(fSkyBoxSize,-fSkyBoxSize,-P),           0.0, 1.f-0.5,
			Vec3(-fSkyBoxSize,-fSkyBoxSize,-D),           1.0, 1.f-0.5,
			Vec3(fSkyBoxSize,-fSkyBoxSize,-D),           0.0, 1.f-0.5
		};

		gRenDev->SetTexture(ef->m_Sky->m_SkyBox[1]->m_Bind);
		gRenDev->SetTexClampMode(true);
		gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),6);
	}
	{ // e
		struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
		{ 
			Vec3(-fSkyBoxSize, fSkyBoxSize, fSkyBoxSize), 1.0, 1.f-0.0,
			Vec3(-fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize), 0.0, 1.f-0.0,
			Vec3(-fSkyBoxSize, fSkyBoxSize,-P),           1.0, 1.f-0.5,
			Vec3(-fSkyBoxSize,-fSkyBoxSize,-P),           0.0, 1.f-0.5,
			Vec3(-fSkyBoxSize, fSkyBoxSize,-D),           1.0, 1.f-0.5,
			Vec3(-fSkyBoxSize,-fSkyBoxSize,-D),           0.0, 1.f-0.5
		};

		gRenDev->SetTexture(ef->m_Sky->m_SkyBox[1]->m_Bind);
		gRenDev->SetTexClampMode(true);
		gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),6);
	}
	{ // n
		struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
		{ 
			Vec3(fSkyBoxSize, fSkyBoxSize, fSkyBoxSize), 1.0, 1.f-1.0,
			Vec3(-fSkyBoxSize, fSkyBoxSize, fSkyBoxSize), 0.0, 1.f-1.0,
			Vec3(fSkyBoxSize, fSkyBoxSize,-P),           1.0, 1.f-0.5,
			Vec3(-fSkyBoxSize, fSkyBoxSize,-P),           0.0, 1.f-0.5,
			Vec3(fSkyBoxSize, fSkyBoxSize,-D),           1.0, 1.f-0.5,
			Vec3(-fSkyBoxSize, fSkyBoxSize,-D),           0.0, 1.f-0.5
		};

		gRenDev->SetTexture(ef->m_Sky->m_SkyBox[0]->m_Bind);
		gRenDev->SetTexClampMode(true);
		gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),6);
	}
	{ // w
		struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
		{ 
			Vec3(fSkyBoxSize,-fSkyBoxSize, fSkyBoxSize), 1.0, 1.f-0.0,
			Vec3(fSkyBoxSize, fSkyBoxSize, fSkyBoxSize), 0.0, 1.f-0.0,
			Vec3(fSkyBoxSize,-fSkyBoxSize,-P),           1.0, 1.f-0.5,
			Vec3(fSkyBoxSize, fSkyBoxSize,-P),           0.0, 1.f-0.5,
			Vec3(fSkyBoxSize,-fSkyBoxSize,-D),           1.0, 1.f-0.5,
			Vec3(fSkyBoxSize, fSkyBoxSize,-D),           0.0, 1.f-0.5
		};

		gRenDev->SetTexture(ef->m_Sky->m_SkyBox[0]->m_Bind);
		gRenDev->SetTexClampMode(true);
		gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),6);
	}
#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)
  if (fpSky)
    fpSky->mfSet(false, 0);
#endif

	DrawFogLayer();

	DrawBlackPortal();

	gRenDev->EnableFog(bPrevFog);

	if (bPrevClipPl)
		gRenDev->EF_SetClipPlane(true, &gRenDev->m_RP.m_CurClipPlane.m_Normal.x, gRenDev->m_RP.m_bClipPlaneRefract);

	return true;
}


bool CRESky::DrawFogLayer()
{   
	if(!m_parrFogLayer)
		m_parrFogLayer = new list2<struct_VERTEX_FORMAT_P3F_COL4UB>;
	if(!m_parrFogLayer2)
		m_parrFogLayer2 = new list2<struct_VERTEX_FORMAT_P3F_COL4UB>;

	m_parrFogLayer->Clear();
	m_parrFogLayer2->Clear();

	const float fFogLayerRadius = SKY_BOX_SIZE;

	if(gRenDev->m_FS.m_FogEnd>=256)
		return true;

	Vec3d camera = gRenDev->GetCamera().GetPos();
	camera.z = max(0,camera.z);

	float fLayerZ = -(max(0,gRenDev->m_FS.m_FogEnd-64)/(256-64))*SKY_BOX_SIZE - camera.z/35;

	bool bRGB = (gRenDev->GetFeatures() & RFT_RGBA) != 0;

	for(int i=0; i<=360; i+=30)
	{
		float rad = (i) * (gf_PI/180);
		struct_VERTEX_FORMAT_P3F_COL4UB tmp;
		tmp.xyz.x = cry_sinf(-rad)*fFogLayerRadius;
		tmp.xyz.y = cry_cosf(-rad)*fFogLayerRadius;
	
		if(bRGB)
		{
			tmp.color.bcolor[0] = uchar(gRenDev->m_FS.m_FogColor.r*255);
			tmp.color.bcolor[1] = uchar(gRenDev->m_FS.m_FogColor.g*255);
			tmp.color.bcolor[2] = uchar(gRenDev->m_FS.m_FogColor.b*255);
		}
		else
		{
			tmp.color.bcolor[2] = uchar(gRenDev->m_FS.m_FogColor.r*255);
			tmp.color.bcolor[1] = uchar(gRenDev->m_FS.m_FogColor.g*255);
			tmp.color.bcolor[0] = uchar(gRenDev->m_FS.m_FogColor.b*255);
		}

		tmp.color.bcolor[3] = uchar(255.f);

		tmp.xyz.z = fLayerZ-SKY_BOX_SIZE/2;
		m_parrFogLayer2->Add(tmp);
		tmp.xyz.z = fLayerZ+SKY_BOX_SIZE/2;
		m_parrFogLayer2->Add(tmp);

		m_parrFogLayer->Add(tmp);
		tmp.xyz.z = fLayerZ+SKY_BOX_SIZE;
		tmp.color.bcolor[3] = 0;
		m_parrFogLayer->Add(tmp);
	}

#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)
  CPShader *fpSky = NULL;
  if (gRenDev->m_RP.m_PersFlags & RBPF_HDR)
  {
    fpSky = PShaderForName(gRenDev->m_RP.m_PS_HDR_BaseCol, "CGRC_HDR_BaseCol_PS20");
    if (fpSky)
      fpSky->mfSet(true);
  }
#endif

	gRenDev->EF_SetState(GS_NODEPTHTEST | GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);
	gRenDev->SelectTMU(0);
  gRenDev->m_TexMan->m_Text_White->Set();
	gRenDev->EnableTMU(true);
	gRenDev->DrawTriStrip(&(CVertexBuffer (m_parrFogLayer->GetElements(),VERTEX_FORMAT_P3F_COL4UB)),m_parrFogLayer->Count());
	gRenDev->DrawTriStrip(&(CVertexBuffer (m_parrFogLayer2->GetElements(),VERTEX_FORMAT_P3F_COL4UB)),m_parrFogLayer2->Count());

#if !defined(PS2) && !defined (GC) && !defined (NULL_RENDERER)
  if (fpSky)
    fpSky->mfSet(false);
#endif

	return true;
}

CRESky::~CRESky()
{
	delete m_parrFogLayer;
	delete m_parrFogLayer2;
}

// render black occlusion volumes mostly to hide seams in indoors
bool CRESky::DrawBlackPortal()
{
	if(!m_arrvPortalVerts[0][0].xyz.x)
		return true;

	gRenDev->ResetToDefault();

	for(int i=0; i<MAX_SKY_OCCLAREAS_NUM; i++)
	{
		if(!m_arrvPortalVerts[i][0].xyz.x)
			return true;

		gRenDev->EF_SetState(GS_DEPTHWRITE);
		gRenDev->SetCullMode(R_CULL_NONE);
		gRenDev->SelectTMU(0);
		gRenDev->EnableTMU(false);
		gRenDev->DrawTriStrip(&(CVertexBuffer (m_arrvPortalVerts[i],VERTEX_FORMAT_P3F_COL4UB)),4);
	}
	return true;
}

void CRESky::DrawSkySphere(float fHeight)
{
	float nWSize = 256/16;  

	float a_in  = 1, a_out = 1;

	struct_VERTEX_FORMAT_P3F_COL4UB vert;
	vert.color.bcolor[0]=255;
	vert.color.bcolor[1]=255;
	vert.color.bcolor[2]=255;

	list2<struct_VERTEX_FORMAT_P3F_COL4UB> lstVertData;

	for(float r=0; r<3; r++)
	{
		a_in = a_out;
		a_out = 1.f-r/2;
		a_out*=a_out; a_out*=a_out; a_out*=a_out;

		lstVertData.Clear();

		for(int i=0; i<=360; i+=40)
		{
			float rad = (i) * (M_PI/180);

			vert.xyz.x = cry_sinf(rad)*nWSize*r;
			vert.xyz.y = cry_cosf(rad)*nWSize*r;
			vert.xyz.z = fHeight + 8 - (r)*8;
			vert.color.bcolor[3] = uchar(a_in*255.0f);
			lstVertData.Add(vert);

			vert.xyz.x = cry_sinf(rad)*nWSize*(r+1);
			vert.xyz.y = cry_cosf(rad)*nWSize*(r+1);
			vert.xyz.z = fHeight + 8 - (r+1)*8;
			vert.color.bcolor[3] = uchar(a_out*255.0f);
			lstVertData.Add(vert);
		}

		gRenDev->DrawTriStrip(&CVertexBuffer(&lstVertData[0],VERTEX_FORMAT_P3F_COL4UB),lstVertData.Count());
	}
}
