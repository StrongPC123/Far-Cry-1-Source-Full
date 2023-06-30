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

#ifndef __ShaderTemplateTool_h__
#define __ShaderTemplateTool_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "EditTool.h"

#include <list>														// STL list<>
//using namespace std;


class CShaderParameterSet
{
public:

private:
	TArray <SShaderParam *> m_ShaderParams;
};


//////////////////////////////////////////////////////////////////////////
class CShaderTemplateTool : public CEditTool
{
	DECLARE_DYNCREATE(CShaderTemplateTool)
public:

	//! constructor
									CShaderTemplateTool				();

	//! destructor
	virtual					~CShaderTemplateTool			();

	virtual void		BeginEditParams						( IEditor *ie,int flags );
	virtual void		EndEditParams							( void );

	virtual void		Display										( DisplayContext &dc );

	// Ovverides from CEditTool
	bool						MouseCallback							( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	// Key down.
	bool						OnKeyDown									( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	bool						OnKeyUp										( CViewport *view,uint nChar,uint nRepCnt,uint nFlags );
	
	// Delete itself.
	void						Release										( void ) { delete this; };

	void						Modify										( void );

	//! open file open dialog and call OnFileChanged()
//	void						ChooseInputFile						( void );

	//! refresh the material combobox and call OnMaterialChanged()
	void						OnFileChanged							( CString inRelFilename );

	//! refresh the choosen templatename and call OnTemplateChanged()
	void						OnMaterialChanged					( void );

	//! refresh the xml
	void						OnTemplateChanged					( void );


	void						OnBnClickedShadertemplLoad( void );
	void						OnBnClickedShadertemplSave( void );

	CString					getFilename								( void );

private:
//	Vec3													m_pointerPos;
//	float													m_brushRadius;
//	bool													m_bMakeHole;

	IEditor *											m_ie;

	int														m_panelId;
	class CShaderTemplatePanel *	m_panel;
	CString												m_sFilename;

	std::list<CShaderParameterSet *>		m_ShaderParameterSetList;
};


#endif // __ShaderTemplateTool_h__
