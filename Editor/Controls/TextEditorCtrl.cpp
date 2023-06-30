// C:\Dev\Crytek\Editor\Controls\RichEditCtrlEx.cpp : implementation file
//

#include "stdafx.h"
#include "TextEditorCtrl.h"

#define GRP_KEYWORD		0
#define GRP_CONSTANTS	1
#define GRP_DIRECTIVE	2
#define GRP_PRAGMA		3


// CTextEditorCtrl

IMPLEMENT_DYNAMIC(CTextEditorCtrl, CRichEditCtrl)
CTextEditorCtrl::CTextEditorCtrl()
{
	/*
	//reconfigure CSyntaxColorizer's default keyword groupings
	LPTSTR sKeywords = "for,for,else,main,struct,enum,switch,auto,"
		"template,explicit,this,bool,extern,thread,break,false,"
		"throw,case,namespace,true,catch,new,try,float,noreturn,"
		"char,operator,typedef,class,friend,private,const,goto,"
		"protected,typename,if,public,union,continue,inline,"
		"unsigned,using,directive,default,int,return,delete,short,"
		"signed,virtual,sizeof,void,do,static,double,long,while";
	LPTSTR sDirectives = "#define,#elif,#else,#endif,#error,#ifdef,"
		"#ifndef,#import,#include,#line,#pragma,#undef";
	LPTSTR sPragmas = "comment,optimize,auto_inline,once,warning,"
		"component,pack,function,intrinsic,setlocale,hdrstop,message";

	m_sc.ClearKeywordList();
	m_sc.AddKeyword(sKeywords,RGB(0,0,255),GRP_KEYWORD);
	m_sc.AddKeyword(sDirectives,RGB(0,0,255),GRP_DIRECTIVE);
	m_sc.AddKeyword(sPragmas,RGB(0,0,255),GRP_PRAGMA);
	m_sc.AddKeyword("REM,Rem,rem",RGB(255,0,255),4);
	*/
	LPTSTR sKeywords = "Shader,ShadeLayer,HW,LightStyle,ValueString,Orient,Origin,Params,Array,Template,Templates,"
		"Version,CGVProgram,CGVPParam,Name,"
		"DeclareLightMaterial,Side,Ambient,Diffuse,Specular,Emission,Shininess,"
		"Layer,Map,RGBGen,RgbGen,AlphaGen,NoDepthTest,Blend,TexCoordMod,Scale,UScale,VScale,ShiftNoise,Noise,SRange,TRange,"
		"Cull,Sort,State,NoCull,ShadowMapGen,Conditions,Vars,DepthWrite,NoColorMask,Portal,LMNoAlpha,"
		"TexColorOp,TexStage,TexType,TexFilter,TexGen,UpdateStyle,EvalLight,Style,TexDecal,Tex1Decal,TexBump,"
		"RCParam,RCombiner,RShader,TSParam,Reg,Comp,DepthMask,AlphaFunc,Light,LightType,ClipPlane,PlaneS,PlaneT,"
		"PolygonOffset,NoLightmap,ShineMap,Turbulence,tcMod,Procedure,TessSize,Spark,Sequence,Maps,Time,Loop,"
		"Mask,Public,float,RenderParams,User,"
		"rgbGen,blend,map,"
		"Translate,Identity,Rotate,RotateX,RotateY,RotateZ,Div,DeformGen,Scroll,UScroll,VScroll,Angle"
		"Type,Level,Amp,Phase,Freq,DeformVertexes,FlareSize,NoLight,Const,Start,"
		"Matrix,FLOAT,BYTE,Verts,Vertex,Normal,Normals,Color,Texture0,Texture1,Texture2,Texture3,Texture4,TNormals";
	
	LPTSTR sConstants = "Decal,None,Nearest,TwoSided,RCRGBToAlpha,OcclusionTest,NoSet,Replace,FromClient,"
		"Opaque,MonitorNoise,Point,Front,Back,Water,TriLinear,"
		"MuzzleFlash,FromObj,Modulate,Base,SphereMap,Add,Glare,Additive,Intensity,White,Sin,Cos,Tan,"
		"$Diffuse,$None,$Specular,$Whiteimage,$Environment,$Glare,$Opacity,$Flare";

	LPTSTR sDirectives = "#define,#elif,#else,#endif,#error,#ifdef,"
		"#ifndef,#import,#include,#line,#pragma,#undef";

	m_sc.ClearKeywordList();
	m_sc.AddKeyword(sKeywords,RGB(0,0,255),GRP_KEYWORD);
	m_sc.AddKeyword(sConstants,RGB(180,0,110),GRP_CONSTANTS);
	m_sc.AddKeyword(sDirectives,RGB(160,0,160),GRP_DIRECTIVE);
	
	m_sc.SetCommentColor( RGB(0,128,128) );
	m_sc.SetStringColor( RGB(0,128,0) );

	m_bModified = true;
}

CTextEditorCtrl::~CTextEditorCtrl()
{
}


BEGIN_MESSAGE_MAP(CTextEditorCtrl, CRichEditCtrl)
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()



static DWORD CALLBACK StreamReadFunction(DWORD_PTR dwCookie,
														LPBYTE lpBuf,	//the buffer to fill
														LONG nCount,	//the no. of bytes to read
														LONG* nRead) // no. of bytes read
{
	CCryFile* fp = (CCryFile*)dwCookie;
	*nRead = fp->Read(lpBuf,nCount);
	return 0;
}

static DWORD CALLBACK StreamWriteFunction(DWORD_PTR dwCookie,
																	 LPBYTE lpBuf,	//the buffer to fill
																	 LONG nCount,	//the no. of bytes to write
																	 LONG* nWrite) // no. of bytes writed
{
	CFile* pFile = (CFile*) dwCookie;
	pFile->Write(lpBuf,nCount);
	*nWrite = nCount;
	return 0;
}

// CTextEditorCtrl message handlers

void CTextEditorCtrl::LoadFile( const CString &sFileName )
{
	if (stricmp(m_filename,sFileName) == 0)
		return;

	m_filename = sFileName;
	SetRedraw(FALSE);

	CCryFile file(sFileName,"rb");
	if (file.Open(sFileName,"rb"))
	{
		m_es.dwCookie = (DWORD_PTR)&file;
		m_es.pfnCallback = StreamReadFunction;
		StreamIn(SF_TEXT,m_es);
	}

	Parse();
	m_bModified = false;
}

//////////////////////////////////////////////////////////////////////////
void CTextEditorCtrl::SaveFile( const CString &sFileName )
{
	if (sFileName.IsEmpty())
		return;

	if (!CFileUtil::OverwriteFile( sFileName ))
		return;

	CFile file(sFileName,CFile::modeCreate|CFile::modeWrite);
	m_es.dwCookie = (DWORD_PTR)&file;
	m_es.pfnCallback = StreamWriteFunction;
	StreamOut(SF_TEXT,m_es);
	file.Close();

	m_bModified = false;
}

//////////////////////////////////////////////////////////////////////////
void CTextEditorCtrl::Parse()
{
	//turn off response to onchange events
	long mask = GetEventMask();
	SetEventMask( mask & (~ENM_CHANGE) );

	//set redraw to false to reduce flicker, and to speed things up
	SetRedraw(FALSE);

	//call the colorizer
	m_sc.Colorize(0,-1,this);

	//do some cleanup
	SetSel(0,0);
	SetRedraw(TRUE);
	RedrawWindow();

	SetEventMask( mask | ENM_CHANGE );
}

//////////////////////////////////////////////////////////////////////////
void CTextEditorCtrl::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	CRichEditCtrl::PreSubclassWindow();

	//set the event mask to accept ENM_CHANGE messages
	SetEventMask( GetEventMask()|ENM_CHANGE );
}

//////////////////////////////////////////////////////////////////////////
void CTextEditorCtrl::OnChange()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CRichEditCtrl::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	CHARRANGE cr;
	GetSel(cr);
	
	//get the current line of text from the control
	int len = LineLength();
	int start = LineIndex();
	//call the colorizer
	m_sc.Colorize(start,start + len,this);

	SetSel(cr);
	m_bModified = true;
}

//////////////////////////////////////////////////////////////////////////
UINT CTextEditorCtrl::OnGetDlgCode()
{
	// TODO: Add your message handler code here and/or call default

	return CRichEditCtrl::OnGetDlgCode() | DLGC_WANTTAB;
}

//////////////////////////////////////////////////////////////////////////
void CTextEditorCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'S' || nChar == 's')
	{
		// Check if control is also pressed.
		GetAsyncKeyState(VK_CONTROL);
		if (GetAsyncKeyState(VK_CONTROL))
		{
			// Save file.
			if (IsModified())
			{
				SaveFile( GetFilename() );
			}
		}
	};

	CRichEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
