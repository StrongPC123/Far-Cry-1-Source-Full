////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   DisplayContext.cpp
//  Version:     v1.00
//  Created:     4/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: DisplayContext
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DisplayContext.h"

#include "..\IconManager.h"
#include "..\Viewport.h"

#include <I3DEngine.h>
//#include <gl\gl.h>

#define FREEZE_COLOR RGB(100,100,100)


//////////////////////////////////////////////////////////////////////////
DisplayContext::DisplayContext()
{
	view = 0;
	renderer = 0;
	engine = 0;
	flags = 0;
	settings = 0;
	m_renderState = 0;

	m_currentMatrix = 0;
	m_matrixStack[m_currentMatrix].SetIdentity();
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawTri( const Vec3 &p1,const Vec3 &p2,const Vec3 &p3 )
{
	struct_VERTEX_FORMAT_P3F_TEX2F verts[4];
	memset( verts,0,sizeof(verts) );
	verts[0].xyz = p1;

	verts[1].xyz = p2;

	verts[2].xyz = p3;

	renderer->DrawTriStrip(&(CVertexBuffer(verts,VERTEX_FORMAT_P3F_TEX2F)),3);
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawQuad( const Vec3 &p1,const Vec3 &p2,const Vec3 &p3,const Vec3 &p4 )
{
	struct_VERTEX_FORMAT_P3F_TEX2F verts[4];
	memset( verts,0,sizeof(verts) );
	verts[0].xyz = p1;

	verts[1].xyz = p2;

	verts[3].xyz = p3;

	verts[2].xyz = p4;

	renderer->DrawTriStrip(&(CVertexBuffer(verts,VERTEX_FORMAT_P3F_TEX2F)),4);
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawTexturePolygon( PolyVertex *points,int numPoints )
{
	assert( points != 0 );
	assert( numPoints > 0 );
	renderer->DrawTriStrip(&(CVertexBuffer(points,VERTEX_FORMAT_P3F_TEX2F)),numPoints);
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawWireBox( const Vec3 &min,const Vec3 &max,float offset )
{
	// Near
	renderer->DrawLine( Vec3(min.x,min.y,max.z),Vec3(max.x,min.y,max.z) );
	renderer->DrawLine( Vec3(min.x,max.y,max.z),Vec3(max.x,max.y,max.z) );
	renderer->DrawLine( Vec3(min.x,min.y,max.z),Vec3(min.x,max.y,max.z) );
	renderer->DrawLine( Vec3(max.x,min.y,max.z),Vec3(max.x,max.y,max.z) );

	// Far
	renderer->DrawLine( Vec3(min.x,min.y,min.z),Vec3(max.x,min.y,min.z) );
	renderer->DrawLine( Vec3(min.x,max.y,min.z),Vec3(max.x,max.y,min.z) );
	renderer->DrawLine( Vec3(min.x,min.y,min.z),Vec3(min.x,max.y,min.z) );
	renderer->DrawLine( Vec3(max.x,min.y,min.z),Vec3(max.x,max.y,min.z) );

	// Sides.
	renderer->DrawLine( Vec3(min.x,min.y,min.z),Vec3(min.x,min.y,max.z) );
	renderer->DrawLine( Vec3(max.x,min.y,min.z),Vec3(max.x,min.y,max.z) );
	renderer->DrawLine( Vec3(min.x,max.y,min.z),Vec3(min.x,max.y,max.z) );
	renderer->DrawLine( Vec3(max.x,max.y,min.z),Vec3(max.x,max.y,max.z) );

	// Draw another wire box with offset.
	if (offset != 0)
	{
		DrawWireBox( min-Vec3(offset,offset,offset),max+Vec3(offset,offset,offset),0 );
	}
}
	
void DisplayContext::DrawSolidBox( const Vec3 &min,const Vec3 &max )
{
	Vec3 p[8];
	p[0] = Vec3(max.x,max.y,max.z);
	p[1] = Vec3(min.x,max.y,max.z);
	p[2] = Vec3(min.x,min.y,max.z);
	p[3] = Vec3(max.x,min.y,max.z);

	p[4] = Vec3(max.x,max.y,min.z);
	p[5] = Vec3(min.x,max.y,min.z);
	p[6] = Vec3(min.x,min.y,min.z);
	p[7] = Vec3(max.x,min.y,min.z);

	DrawQuad( p[0],p[1],p[2],p[3] );
	DrawQuad( p[4],p[7],p[6],p[5] );
	
	DrawQuad( p[0],p[3],p[7],p[4] );
	DrawQuad( p[1],p[5],p[6],p[2] );

	DrawQuad( p[0],p[4],p[5],p[1] );
	DrawQuad( p[3],p[2],p[6],p[7] );
}

void DisplayContext::DrawLine( const Vec3 &p1,const Vec3 &p2 )
{
	renderer->DrawLine( p1, p2 );
}

void DisplayContext::DrawPolyLine( const Vec3 *pnts,int numPoints )
{
	assert( pnts != 0 );
	assert( numPoints > 1 );
	for (int i = 0; i < numPoints; i++)
	{
		int j = (i+1)<numPoints?(i+1):0;
		renderer->DrawLine( pnts[i],pnts[j] );
	}
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawTerrainCircle( const Vec3 &pos,float radius,float height )
{
	// Draw circle with default radius.
	Vec3 p0,p1;
	p0.x = pos.x + radius*sin(0.0f);
	p0.y = pos.y + radius*cos(0.0f);
	p0.z = engine->GetTerrainElevation( p0.x,p0.y )+height;
	float step = 10.0f/180*gf_PI;
	for (float angle = step; angle < 360.0f/180*gf_PI+step; angle += step)
	{
		p1.x = pos.x + radius*sin(angle);
		p1.y = pos.y + radius*cos(angle);
		p1.z = engine->GetTerrainElevation( p1.x,p1.y )+height;
		renderer->DrawLine( p0,p1 );
		p0 = p1;
	}
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawCircle( const Vec3 &pos,float radius )
{
	// Draw circle with default radius.
	Vec3 p0,p1;
	p0.x = pos.x + radius*sin(0.0f);
	p0.y = pos.y + radius*cos(0.0f);
	p0.z = pos.z;
	float step = 10.0f/180*gf_PI;
	for (float angle = step; angle < 360.0f/180*gf_PI+step; angle += step)
	{
		p1.x = pos.x + radius*sin(angle);
		p1.y = pos.y + radius*cos(angle);
		p1.z = pos.z;
		renderer->DrawLine( p0,p1 );
		p0 = p1;
	}
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawWireCircle2d( const CPoint &center,float radius,float z )
{
	Vec3 p0,p1,pos;
	pos.x = center.x;
	pos.y = center.y;
	pos.z = z;
	p0.x = pos.x + radius*sin(0.0f);
	p0.y = pos.y + radius*cos(0.0f);
	p0.z = z;
	float step = 10.0f/180*gf_PI;
	for (float angle = step; angle < 360.0f/180*gf_PI+step; angle += step)
	{
		p1.x = pos.x + radius*sin(angle);
		p1.y = pos.y + radius*cos(angle);
		p1.z = z;
		renderer->DrawLine( p0,p1 );
		p0 = p1;
	}
}

void DisplayContext::DrawWireSphere( const Vec3 &pos,float radius )
{
	Vec3 p0,p1;
	float step = 10.0f/180*gf_PI;
	float angle;

	// Z Axis
	p0 = pos;
	p1 = pos;
	p0.x += radius*sin(0.0f);
	p0.y += radius*cos(0.0f);
	for (angle = step; angle < 360.0f/180*gf_PI+step; angle += step)
	{
		p1.x = pos.x + radius*sin(angle);
		p1.y = pos.y + radius*cos(angle);
		p1.z = pos.z;
		renderer->DrawLine( p0,p1 );
		p0 = p1;
	}

	// X Axis
	p0 = pos;
	p1 = pos;
	p0.y += radius*sin(0.0f);
	p0.z += radius*cos(0.0f);
	for (angle = step; angle < 360.0f/180*gf_PI+step; angle += step)
	{
		p1.y = pos.y + radius*sin(angle);
		p1.z = pos.z + radius*cos(angle);
		renderer->DrawLine( p0,p1 );
		p0 = p1;
	}
	
	// Y Axis
	p0 = pos;
	p1 = pos;
	p0.x += radius*sin(0.0f);
	p0.z += radius*cos(0.0f);
	for (angle = step; angle < 360.0f/180*gf_PI+step; angle += step)
	{
		p1.x = pos.x + radius*sin(angle);
		p1.z = pos.z + radius*cos(angle);
		renderer->DrawLine( p0,p1 );
		p0 = p1;
	}
}

void DisplayContext::DrawWireQuad2d( const CPoint &pmin,const CPoint &pmax,float z )
{
	renderer->DrawLine( Vec3(pmin.x,pmin.y,z),Vec3(pmax.x,pmin.y,z) );
	renderer->DrawLine( Vec3(pmax.x,pmin.y,z),Vec3(pmax.x,pmax.y,z) );
	renderer->DrawLine( Vec3(pmax.x,pmax.y,z),Vec3(pmin.x,pmax.y,z) );
	renderer->DrawLine( Vec3(pmin.x,pmax.y,z),Vec3(pmin.x,pmin.y,z) );
}

void DisplayContext::DrawLine2d( const CPoint &pmin,const CPoint &pmax,float z )
{
	renderer->DrawLine( Vec3(pmin.x,pmin.y,z),Vec3(pmax.x,pmin.y,z) );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::SetColor( float r,float g,float b,float a )
{
	m_color[0] = r;
	m_color[1] = g;
	m_color[2] = b;
	m_color[3] = a;
	renderer->SetMaterialColor( r,g,b,a );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::SetColor( const Vec3 &color,float a )
{
	m_color[0] = color.x;
	m_color[1] = color.y;
	m_color[2] = color.z;
	m_color[3] = a;
	renderer->SetMaterialColor( color.x,color.y,color.z,a );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::SetColor( COLORREF rgb,float a )
{
	Vec3 col = Rgb2Vec(rgb);
	m_color[0] = col.x;
	m_color[1] = col.y;
	m_color[2] = col.z;
	m_color[3] = a;
	renderer->SetMaterialColor( col.x,col.y,col.z,a );
}

//////////////////////////////////////////////////////////////////////////
COLORREF DisplayContext::GetSelectedColor()
{
	float t = GetTickCount() / 1000.0f;
	float r1 = fabs(sin(t*8.0f));
	if (r1 > 255)
		r1 = 255;
	return RGB( 255,0,r1*255 );
//			float r2 = cos(t*3);
		//dc.renderer->SetMaterialColor( 1,0,r1,0.5f );
}

COLORREF DisplayContext::GetFreezeColor()
{
	return FREEZE_COLOR;
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::SetSelectedColor( float fAlpha )
{
	SetColor( GetSelectedColor(),fAlpha );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::SetFreezeColor()
{
	SetColor( FREEZE_COLOR,0.5f );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawLine( const Vec3 &p1,const Vec3 &p2,const CFColor &col1,const CFColor &col2 )
{
	renderer->DrawLineColor( p1,col1,p2,col2 );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawLine( const Vec3 &p1,const Vec3 &p2,COLORREF rgb1,COLORREF rgb2 )
{
	Vec3 c1 = Rgb2Vec(rgb1);
	Vec3 c2 = Rgb2Vec(rgb2);
	renderer->DrawLineColor( p1,c1,p2,c2 );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::PushMatrix( const Matrix44 &tm )
{
	m_currentMatrix++;
	m_matrixStack[m_currentMatrix] = tm * m_matrixStack[m_currentMatrix-1];

	renderer->PushMatrix();
	//renderer->LoadMatrix( &tm );
	//renderer->TranslateMatrix( Vec3(tm[3][0],tm[3][1],tm[3][2]) );
	renderer->MultMatrix( (float*)const_cast<Matrix44*>(&tm) );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::PopMatrix()
{
	m_currentMatrix--;
	renderer->PopMatrix();
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawBall( const Vec3 &pos,float radius )
{
	//RenderObject( STATOBJECT_SPHERE,pos,radius );
	renderer->DrawBall( pos,radius );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawArrow( const Vec3 &src,const Vec3 &trg,float fHeadScale )
{
	Vec3 dir = trg - src;
	Matrix44 tm = MatrixFromVector( dir );
	
	renderer->DrawLine( src,trg );

	tm = Matrix33::CreateScale( Vec3d(fHeadScale,fHeadScale,fHeadScale) ) * tm;

	tm.SetTranslationOLD( trg );
	//if (flags & DISPLAY_2D)
	{
		PushMatrix(tm);

		DrawLine( Vec3(0,0,0),Vec3(0,0.4f,0) );
		
		DrawLine( Vec3(-0.15f,0,0),Vec3(0,0.4f,0) );
		DrawLine( Vec3(0.15f,0,0),Vec3(0,0.4f,0) );

		DrawLine( Vec3(0,0,-0.15f),Vec3(0,0.4f,0) );
		DrawLine( Vec3(0,0,0.15f),Vec3(0,0.4f,0) );
		PopMatrix();
	}
	//else
		//RenderObject( STATOBJECT_ARROW,tm );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::RenderObject( int objectType,const Vec3 &pos,float scale )
{
	Matrix44 tm;
	tm.SetIdentity();

	tm = Matrix33::CreateScale( Vec3d(scale,scale,scale) ) * tm;

	tm.SetTranslationOLD(pos);
	RenderObject( objectType,tm );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::RenderObject( int objectType,const Matrix44 &tm )
{
	IStatObj *object = GetIEditor()->GetIconManager()->GetObject((EObject)objectType);
	if (object)
	{
		Matrix44 xform = tm * m_matrixStack[m_currentMatrix];
		SRendParams rp;
		rp.pMatrix = &xform;
		rp.vColor = Vec3(m_color[0],m_color[1],m_color[2]);
		rp.fAlpha = m_color[3];
		rp.nDLightMask = 0xFFFF;
    rp.dwFObjFlags |= FOB_TRANS_MASK;
		object->Render( rp, Vec3(zero), 0);
	}
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawIcon( EIcon icon,const Vec3 &pos,float fScale )
{
	int texId = GetIEditor()->GetIconManager()->GetIcon(icon);
	renderer->DrawLabelImage( pos,fScale,texId );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawTerrainRect( float x1,float y1,float x2,float y2,float height )
{
	Vec3 p1,p2;
	float x,y;
	
	float step = MAX( y2-y1,x2-x1 );
	if (step < 0.1)
		return;
	step = step / 100.0f;

	for (y = y1; y < y2+step; y += step)
	{
		p1.x = x1;
		p1.y = y;
		p1.z = engine->GetTerrainElevation( p1.x,p1.y ) + height;

		p2.x = x1;
		p2.y = y+step;
		p2.z = engine->GetTerrainElevation( p2.x,p2.y ) + height;
		DrawLine( p1,p2 );

		p1.x = x2+step;
		p1.y = y;
		p1.z = engine->GetTerrainElevation( p1.x,p1.y ) + height;

		p2.x = x2+step;
		p2.y = y+step;
		p2.z = engine->GetTerrainElevation( p2.x,p2.y ) + height;
		DrawLine( p1,p2 );
	}
	for (x = x1; x < x2+step; x += step)
	{
		p1.x = x;
		p1.y = y1;
		p1.z = engine->GetTerrainElevation( p1.x,p1.y ) + height;

		p2.x = x+step;
		p2.y = y1;
		p2.z = engine->GetTerrainElevation( p2.x,p2.y ) + height;
		DrawLine( p1,p2 );

		p1.x = x;
		p1.y = y2+step;
		p1.z = engine->GetTerrainElevation( p1.x,p1.y ) + height;

		p2.x = x+step;
		p2.y = y2+step;
		p2.z = engine->GetTerrainElevation( p2.x,p2.y ) + height;
		DrawLine( p1,p2 );
	}
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::DrawTextLabel( const Vec3& pos,float size,const char *text )
{
	view->DrawTextLabel( *this,pos,size,CFColor(m_color[0],m_color[1],m_color[2],m_color[3]),text );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::Draw2dTextLabel( float x,float y,float size,const char *text )
{
	renderer->Draw2dLabel( x,y,size,m_color,false,"%s",text );
  //view->DrawTextLabel( *this,pos,size,Vec3(m_color[0],m_color[1],m_color[2]),text );
}

//////////////////////////////////////////////////////////////////////////
void DisplayContext::SetLineWidth( float width )
{
	renderer->SetLineWidth( width );
}

//////////////////////////////////////////////////////////////////////////
bool DisplayContext::IsVisible( const BBox &bounds )
{
	if (flags & DISPLAY_2D)
	{
		if (box.IsIntersectBox( bounds ))
		{
			return true;
		}
	}
	else
	{
		return camera->IsAABBVisibleFast( AABB(bounds.min,bounds.max) );
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
uint DisplayContext::GetState() const
{
	return m_renderState;
}

//! Set a new render state flags.
//! @param returns previous render state.
uint DisplayContext::SetState( uint state )
{
	uint old = m_renderState;
	m_renderState = state;
	renderer->SetState( m_renderState );
	return old;
}

//! Set a new render state flags.
//! @param returns previous render state.
uint DisplayContext::SetStateFlag( uint state )
{
	uint old = m_renderState;
	m_renderState |= state;
	renderer->SetState( m_renderState );
	return old;
}

//! Clear specified flags in render state.
//! @param returns previous render state.
uint DisplayContext::ClearStateFlag( uint state )
{
	uint old = m_renderState;
	m_renderState &= ~state;
	renderer->SetState( m_renderState );
	return old;
}


char BoxSides[0x40*8] = {
	  0,0,0,0, 0,0,0,0, //00
		0,4,6,2, 0,0,0,4, //01
		7,5,1,3, 0,0,0,4, //02
		0,0,0,0, 0,0,0,0, //03
		0,1,5,4, 0,0,0,4, //04
		0,1,5,4, 6,2,0,6, //05
		7,5,4,0, 1,3,0,6, //06
		0,0,0,0, 0,0,0,0, //07
		7,3,2,6, 0,0,0,4, //08
		0,4,6,7, 3,2,0,6, //09
		7,5,1,3, 2,6,0,6, //0a
		0,0,0,0, 0,0,0,0, //0b
		0,0,0,0, 0,0,0,0, //0c
		0,0,0,0, 0,0,0,0, //0d
		0,0,0,0, 0,0,0,0, //0e
		0,0,0,0, 0,0,0,0, //0f
		0,2,3,1, 0,0,0,4, //10
		0,4,6,2, 3,1,0,6, //11
		7,5,1,0, 2,3,0,6, //12
		0,0,0,0, 0,0,0,0, //13
		0,2,3,1, 5,4,0,6, //14
		1,5,4,6, 2,3,0,6, //15
		7,5,4,0, 2,3,0,6, //16
		0,0,0,0, 0,0,0,0, //17
		0,2,6,7, 3,1,0,6, //18
		0,4,6,7, 3,1,0,6, //19
		7,5,1,0, 2,6,0,6, //1a
		0,0,0,0, 0,0,0,0, //1b
		0,0,0,0, 0,0,0,0, //1c
		0,0,0,0, 0,0,0,0, //1d
		0,0,0,0, 0,0,0,0, //1e
		0,0,0,0, 0,0,0,0, //1f
		7,6,4,5, 0,0,0,4, //20
		0,4,5,7, 6,2,0,6, //21
		7,6,4,5, 1,3,0,6, //22
		0,0,0,0, 0,0,0,0, //23
		7,6,4,0, 1,5,0,6, //24
		0,1,5,7, 6,2,0,6, //25
		7,6,4,0, 1,3,0,6, //26
		0,0,0,0, 0,0,0,0, //27
		7,3,2,6, 4,5,0,6, //28
		0,4,5,7, 3,2,0,6, //29
		6,4,5,1, 3,2,0,6, //2a
		0,0,0,0, 0,0,0,0, //2b
		0,0,0,0, 0,0,0,0, //2c
		0,0,0,0, 0,0,0,0, //2d
		0,0,0,0, 0,0,0,0, //2e
		0,0,0,0, 0,0,0,0, //2f
		0,0,0,0, 0,0,0,0, //30
		0,0,0,0, 0,0,0,0, //31
		0,0,0,0, 0,0,0,0, //32
		0,0,0,0, 0,0,0,0, //33
		0,0,0,0, 0,0,0,0, //34
		0,0,0,0, 0,0,0,0, //35
		0,0,0,0, 0,0,0,0, //36
		0,0,0,0, 0,0,0,0, //37
		0,0,0,0, 0,0,0,0, //38
		0,0,0,0, 0,0,0,0, //39
		0,0,0,0, 0,0,0,0, //3a
		0,0,0,0, 0,0,0,0, //3b
		0,0,0,0, 0,0,0,0, //3c
		0,0,0,0, 0,0,0,0, //3d
		0,0,0,0, 0,0,0,0, //3e
		0,0,0,0, 0,0,0,0, //3f
};

