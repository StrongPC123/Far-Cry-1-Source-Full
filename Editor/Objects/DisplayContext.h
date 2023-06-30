////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   DisplayContext.h
//  Version:     v1.00
//  Created:     4/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: DisplayContext definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DisplayContext_h__
#define __DisplayContext_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "..\IconManager.h"

// forward declarations.
class CViewport;

enum DisplayFlags {
	DISPLAY_2D = 0x01,
	DISPLAY_HIDENAMES = 0x02,
	DISPLAY_BBOX = 0x04,
	DISPLAY_TRACKS = 0x08,
	DISPLAY_TRACKTICKS = 0x010,
	DISPLAY_WORLDSPACEAXIS = 0x020, //!< Set if axis must be displayed in world space.
	DISPLAY_LINKS = 0x040,
	DISPLAY_DEGRADATED = 0x080,	//!< Display Objects in degradated quality (When moving/modifying).
};

/*!
 *  DisplayContex is a structure passed to BaseObject Display method.
 *	It contains everything the object should know to display itself in a view.
 *	All fields must be filled before passing that structure to Display call.
 */
struct DisplayContext
{
	typedef struct_VERTEX_FORMAT_P3F_TEX2F PolyVertex;

	CDisplaySettings* settings;
	CViewport* view;
	IRenderer* renderer;
	I3DEngine* engine;
	CCamera*	camera;
	BBox	box;	// Bounding box of volume that need to be repainted.
	int flags;


	//! Ctor.
	DisplayContext();
	// Helper methods.
	
	//////////////////////////////////////////////////////////////////////////
	// Draw functions
	//////////////////////////////////////////////////////////////////////////
	//! Set current materialc color.
	void SetColor( float r,float g,float b,float a=1 );
	void SetColor( const Vec3 &color,float a=1 );
	void SetColor( COLORREF rgb,float a=1 );

	void SetSelectedColor( float fAlpha=1 );
	void SetFreezeColor();

	//! Get color to draw selectin of object.
	COLORREF GetSelectedColor();
	COLORREF GetFreezeColor();

	//! Draw 3D quad.
	void DrawQuad( const Vec3 &p1,const Vec3 &p2,const Vec3 &p3,const Vec3 &p4 );
	//! Draw 3D Triangle.
	void DrawTri( const Vec3 &p1,const Vec3 &p2,const Vec3 &p3 );
	//! Draw wireframe box.
	void DrawWireBox( const Vec3 &min,const Vec3 &max,float offset2nd=0 );
	void DrawSolidBox( const Vec3 &min,const Vec3 &max );
	void DrawLine( const Vec3 &p1,const Vec3 &p2 );
	void DrawLine( const Vec3 &p1,const Vec3 &p2,const CFColor &col1,const CFColor &col2 );
	void DrawLine( const Vec3 &p1,const Vec3 &p2,COLORREF rgb1,COLORREF rgb2 );
	void DrawPolyLine( const Vec3 *pnts,int numPoints );
	
	void DrawWireQuad2d( const CPoint &p1,const CPoint &p2,float z );
	void DrawLine2d( const CPoint &p1,const CPoint &p2,float z );
	void DrawWireCircle2d( const CPoint &center,float radius,float z );

	//! Draw polygon with texturing.
	//! @param points Array of polygon vertices, must contain at least numPoints entries.
	void DrawTexturePolygon( PolyVertex *points,int numPoints );

	// Draw circle from lines on terrain.
	void DrawTerrainCircle( const Vec3 &pos,float radius,float height );
	void DrawCircle( const Vec3 &pos,float radius );

	//! Draw rectangle on top of terrain.
	//! Coordinates are in world space.
	void DrawTerrainRect( float x1,float y1,float x2,float y2,float height );

	void DrawWireSphere( const Vec3 &pos,float radius );

	void PushMatrix( const Matrix44 &tm );
	void PopMatrix();

	// Draw special 3D objects.
	void DrawBall( const Vec3 &pos,float radius );
	
	//! Draws 3d arrow.
	void DrawArrow( const Vec3 &src,const Vec3 &trg,float fHeadScale=1 );

	//! Draw 3D icon.
	void DrawIcon( EIcon icon,const Vec3 &pos,float fScale=1 );

	void RenderObject( int objectType,const Vec3 &pos,float scale );
	void RenderObject( int objectType,const Matrix44 &tm );

	void DrawTextLabel( const Vec3& pos,float size,const char *text );
	void Draw2dTextLabel( float x,float y,float size,const char *text );
	void SetLineWidth( float width );

	//! Is givven bbox visible in this display context.
	bool IsVisible( const BBox &bounds );

	//! Gets current render state.
	uint GetState() const;
	//! Set a new render state.
	//! @param returns previous render state.
	uint SetState( uint state );
	//! Set a new render state flags.
	//! @param returns previous render state.
	uint SetStateFlag( uint state );
	//! Clear specified flags in render state.
	//! @param returns previous render state.
	uint ClearStateFlag( uint state );

private:
	float m_color[4];
	uint m_renderState;

	int m_currentMatrix;
	//! Matrix stack.
	Matrix44 m_matrixStack[16];
};

#endif // __DisplayContext_h__
