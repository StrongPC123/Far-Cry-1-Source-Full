////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ShaderTemplateTool.h
//  Version:     v0.01
//  Created:     13/2/2002 by Martin Mittring
//  Compilers:   Visual C++ 6.0
//  Description: Shader template parameter tweaker
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ShaderTemplateTool.h"													// CShaderTemplateTool
#include "Viewport.h"
#include "ShaderTemplatePanel.h"												// CShaderTemplatePanel
#include "PropertiesPanel.h"														// CPropertiesPanel
#include "Heightmap.h"
#include "Objects\DisplayContext.h"

#include "ShaderEnum.h"																	// CShaderEnum
//#define EFQ_RegisteredTemplates 14

#include <vector>																				// STL vector<>
//using namespace std;																		// STL

struct IStatObj;

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CShaderTemplateTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CShaderTemplateTool::CShaderTemplateTool()
{
	m_panelId = 0;
	m_panel = 0;
	
//	m_pointerPos.Set(0,0,0);

//	m_bMakeHole = true;
//	m_brushRadius = 1;
	GetIEditor()->ClearSelection();
}

//////////////////////////////////////////////////////////////////////////
CShaderTemplateTool::~CShaderTemplateTool()
{
//	m_pointerPos.Set(0,0,0);
	if (GetIEditor()->IsUndoRecording())
		GetIEditor()->CancelUndo();
}

//////////////////////////////////////////////////////////////////////////
void CShaderTemplateTool::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;

	if (!m_panelId)
	{
	XmlParser xml;
//	XmlNodeRef root = xml.parse( "shadertemplates.xml" );

	/*
	CString str="\
<Root>\
	<First type=\"INT\" value=\"12\"/>\
	<FlattenArea type=\"Float\" value=\"12\"/>\
	<Third type=\"INT\" value=\"12\"/>\
	<Proximity type=\"Float\" value = \"10\"/>\
	\
	<SoundList type=\"List\">\
		<Sound1 type=\"String\" value=\"First\"/>\
		<Sound2 type=\"String\" value=\"Second\"/>\
		<Sound3 type=\"String\" value=\"Third\"/>\
	</SoundList>\
	\
	<FirstName type=\"String\" value=\"BlahBlah\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<Diameter type=\"INT\" value=\"12\"/>\
	<OmniLight type=\"BOOL\" value=\"true\"/>\
	<Radius type=\"Float\" value=\"12\"/>\
	<Color type=\"Color\" value=\"254,122,48\"/>\
	<Texture type=\"File\" value=\"test_texture.jpg\" FileFilters=\"Bitmaps (*.bmp)|*.bmp|PGM Files (*.pgm)|*.pgm|All files (*.*)|*.*\"/>\
	<Blah type=\"File\" value=\"test_texture.jpg\" FileFilters=\"Bitmaps (*.bmp)|*.bmp|PGM Files (*.pgm)|*.pgm|All files (*.*)|*.*\"/>\
</Root>";
	*/

	//std::vector<char>	xmlBuffer;
	CString str="<Root></Root>";

//	xmlBuffer.resize( str.GetLength() );
//	memcpy( &xmlBuffer[0],str.GetBuffer(0),str.GetLength() );

	XmlNodeRef xmlnode = xml.parseBuffer(str);

	
		m_panel = new CShaderTemplatePanel(this,xmlnode,AfxGetMainWnd());
		m_panelId = m_ie->AddRollUpPage( ROLLUP_TERRAIN,"Shader Template Properties",m_panel );	// fuck ROLLUP_TERRAIN has to be changed
		AfxGetMainWnd()->SetFocus();
	}

	CShaderEnum *shaderenum=GetIEditor()->GetShaderEnum();


	int nCount=shaderenum->EnumShaders();

	// add shadername to combobox
	for(int i=0;i<nCount;i++)
	{
		CString refcopy = shaderenum->GetShader(i);

		m_panel->m_TemplateCombo.AddString(refcopy);
	}


	/*
	{
		FILE *out=fopen("c:\\temp\\shaders.txt","w");

		for(int i=0;i<nCount;i++)
		{
			CString refcopy = shaderenum->GetShader(i);

			fprintf(out,"%s\n",refcopy);

			SShader *sshader=GetIEditor()->GetRenderer()->EF_LoadShader(refcopy.GetBuffer(64),-1,eSH_World,0);

			if(sshader)
			{
				TArray<SShaderParam *> &shaderparams=sshader->mfGetPublicParams();

			//	struct SShaderParam 
			//    string m_Name;
			//    EParamType m_Type;
			//    UParamVal m_Value;

				int nParamCount=shaderparams.Num();

				for(int e=0;e<nParamCount;e++)
				{
					SShaderParam *ptr=shaderparams[e];

					fprintf(out,"  - %s\n",ptr->m_Name.c_str());
				}

//				delete sshader;
      }
		}

		fclose(out);
	}
	*/

	m_panel->m_TemplateCombo.SetCurSel(0);
}

//////////////////////////////////////////////////////////////////////////
void CShaderTemplateTool::EndEditParams()
{
	if (m_panelId)
	{
		m_ie->RemoveRollUpPage(ROLLUP_TERRAIN,m_panelId);
		m_panel = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CShaderTemplateTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
//	m_pointerPos = view->ViewToWorld( point );
	if (event == eMouseLDown || (event == eMouseMove && (flags&MK_LBUTTON)))
	{
		/*
		if (flags&MK_CONTROL)
		{
			bool bMakeHole = m_bMakeHole;
			m_bMakeHole = false;
			Modify();
			m_bMakeHole = bMakeHole;
		}
		else
		{
			Modify();
		}
		*/
//		if (!GetIEditor()->IsUndoRecording())
//			GetIEditor()->BeginUndo();
//		Modify();
	}
	else
	{
//		if (GetIEditor()->IsUndoRecording())
//			GetIEditor()->AcceptUndo( "Terrain Hole" );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CShaderTemplateTool::Display( DisplayContext &dc )
{

}

//////////////////////////////////////////////////////////////////////////
bool CShaderTemplateTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	bool bProcessed = false;
/*	if (nChar == VK_ADD)
	{
		if (m_brushRadius < 100)
			m_brushRadius += 0.5f;
		bProcessed = true;
	}
	if (nChar == VK_SUBTRACT)
	{
		if (m_brushRadius > 0.5f)
			m_brushRadius -= 0.5f;
		bProcessed = true;
	}
	if (nChar == VK_CONTROL && !(nFlags&(1<<14))) // only once (no repeat).
	{
		m_bMakeHole = !m_bMakeHole;
		m_panel->SetMakeHole(m_bMakeHole);
	}
	if (bProcessed && m_panel)
	{
		m_panel->m_radius.SetPos(m_brushRadius);
	}
*/
	return bProcessed;
}

bool CShaderTemplateTool::OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
/*
	if (nChar == VK_CONTROL)
	{
		m_bMakeHole = !m_bMakeHole;
		m_panel->SetMakeHole(m_bMakeHole);
	}
	*/
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CShaderTemplateTool::Modify()
{
	/*
	float fx1 = (m_pointerPos.y - m_brushRadius)/UNIT_SIZE;
	float fy1 = (m_pointerPos.x - m_brushRadius)/UNIT_SIZE;
	float fx2 = (m_pointerPos.y + m_brushRadius)/UNIT_SIZE;
	float fy2 = (m_pointerPos.x + m_brushRadius)/UNIT_SIZE;

	CHeightmap *heightmap = m_ie->GetHeightmap();
	int x1 = MAX(fx1,0);
	int y1 = MAX(fy1,0);
	int x2 = MIN(fx2,heightmap->GetWidth()-1);
	int y2 = MIN(fy2,heightmap->GetHeight()-1);

	heightmap->MakeHole( x1,y1,x2-x1,y2-y1,m_bMakeHole );
	*/
}






/*
// open file open dialog
void CShaderTemplateTool::ChooseInputFile( void )
{
	assert(m_ie);

	CString file,relFile;

	if (CFileUtil::SelectSingleFile( EFILE_TYPE_GEOMETRY,relFile ))
	{
		OnFileChanged(relFile);
	}
}
*/

// refresh the material combobox and callOnMaterialChanged()
void CShaderTemplateTool::OnFileChanged( CString inRelFilename )
{
	// todo **************************
/*
	m_sFilename=inRelFilename;


	XmlParser xml;

	XmlNodeRef ref=xml.parse(m_sFilename);

	if (ref != 0 && ref->isTag("shadertemplateparameters"))
	{
		for(int i = 0; i < ref->getChildCount(); i++)
		{
			XmlNodeRef child = ref->getChild(i);
			
			if(child->isTag("associated"))
			{
				CString cgfname;

				if(child->getAttr( "mesh",cgfname ))
				{
				}
			}
			else if(child->isTag("associate"))
			{
			}
			else if(child->isTag("parameterset"))
			{
			}

		}
	}
*/
/*	I3DEngine *pEngine=m_ie->GetSystem()->GetI3DEngine();

	assert(pEngine);

	IStatObj *pObject = pEngine->MakeObject( inRelFilename );


	{
		vector<IStatObj *> objlist;

		objlist.push(pObject);

		pEngine->SaveCGF(objlist);
	}

	if (pObject)
		m_ie->GetSystem()->GetI3DEngine()->ReleaseObject( m_object );
*/
}




void CShaderTemplateTool::OnBnClickedShadertemplLoad( void )
{
	CString file,relFile;

	if (CFileUtil::SelectSingleFile( EFILE_TYPE_GEOMETRY,file ))
	{
		OnFileChanged(m_sFilename);
	}
}

void CShaderTemplateTool::OnBnClickedShadertemplSave( void )
{
	AfxMessageBox("todo");
}

CString CShaderTemplateTool::getFilename( void )
{
	return(m_sFilename);
}
