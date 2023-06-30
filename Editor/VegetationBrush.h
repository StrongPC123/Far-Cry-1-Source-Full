////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   vegetationbrush.h
//  Version:     v1.00
//  Created:     20/2/2002 by Timur.
//  Compilers:   Visual C++.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __vegetationbrush_h__
#define __vegetationbrush_h__
#pragma once

class CVegetationObject;
	
/*!
	CVegetation Brush.
*/
class CVegetationBrush : public CRefCountBase
{
public:
	CVegetationBrush();
	virtual ~CVegetationBrush();

	void CopyFrom( CVegetationBrush *brush );

	void SetName( const CString &name ) { m_name = name; };
	const CString& GetName() const { return m_name; };

	virtual void Serialize( XmlNodeRef &node,bool bLoading );

	int GetObjectCount() { return m_objects.size(); }
	CVegetationObject* GetObject( int i ) { return m_objects[i]; }

	// Operators.
	void AddObject( CVegetationObject *obj );
	void RemoveObject( CVegetationObject *obj );
	void ClearObjects() { m_objects.clear(); }

	void SetElevation( float min,float max ) { m_elevationMin = min; m_elevationMax = max; };
	void SetSlope( float min,float max ) { m_slopeMin = min; m_slopeMax = max; };
	void SetDensity( float dens ) { m_density = dens; };

	//! Accessors.
	float GetElevationMin() const { return m_elevationMin; };
	float GetElevationMax() const { return m_elevationMax; };
	float GetSlopeMin() const { return m_slopeMin; };
	float GetSlopeMax() const { return m_slopeMax; };

	float GetDensity() const { return m_density; };

	//! Return true when the brush can paint on a location with the supplied parameters
	bool IsPlaceValid( float height,float slope ) const
	{
		if (height < m_elevationMin || height > m_elevationMax)
			return false;
		if (slope < m_slopeMin || slope > m_slopeMax)
			return false;
		return true;
	}

	void SetHidden( bool bHidden );
	bool IsHidden() const { return m_bHidden; };

	void SetSelected( bool bSelected ) { m_bSelected = bSelected; }
	bool IsSelected() const { return m_bSelected; }

private:
	CString m_name;

	typedef std::vector<CVegetationObject*> Objects;
	Objects m_objects;

	// Elevation range (0 - 255)
	float m_elevationMin;
	float m_elevationMax;

	// Slope range (0 - 255)
	float m_slopeMin;
	float m_slopeMax;

	// Density from (0 - 100)
	float m_density;

	bool m_bHidden;

	//! True if Selected.
	bool m_bSelected;
};

#endif // __vegetationbrush_h__
