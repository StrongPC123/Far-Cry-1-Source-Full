
//
// Crytek Source code
//
// written my Martin Mittring
//
// **** tiled heightmap
// check out of memory with exception handling
//
//
// Dependencies: HorizonTracker.h
//


#pragma once

#include <assert.h>										// assert()
#include "HorizonTracker.h"						// CHorizonTracker

#include <math.h>											// sin()
#include <vector>											// STL vector<>

// you can use CHemisphereSolid 
// or implement your own CHemisphereSink_Solid type (e.g. sum colors from given diffuse texture, sum circular spot for soft shadow, ... ) 

// Full Sky is 255
// brightness is equally distributed over the hemisphere
// sample precision is 8.8 fixpoint
// can be used as template argument (THemisphere)
class CHemisphereSink_Solid
{
public:

	typedef	unsigned short	SampleType;			//!< 8.8 fix point

	// ---------------------------------------------------------------------------------

	//! constructor
	//! \param indwAngleSteps [1..[ 
	CHemisphereSink_Solid( const DWORD indwAngleSteps )
	{
		// to scale the input to the intermediate range

		m_fScale=		 256.0f													// 256 for 8bit fix point, 
								*255.0f													// 255 for max unsigned char
								/((float)indwAngleSteps)				// AddToIntermediate is called indwAngleSteps times
								/(float)(gf_PI_DIV_2*gf_PI_DIV_2);				// scale form [0..PI/2[ * [0..PI/2[  to  1
	}

	//!
	//!
	void InitSample( SampleType &inoutValue ) const
	{
		inoutValue=0;		// 8.8 fix point
	}

	//! \infAngle infAngle in rad (not needed here because every direction is equal)
	void SetAngle( const float infAngle )
	{
	}

	//! \param infSlope [0..[
	//! \param inoutValue result is added to this value
	void AddWedgeArea( const float infSlope, SampleType &inoutValue ) const
	{
		float fWedgeHorizonAngle=(float)(gf_PI_DIV_2-atanf(-infSlope));			// [0..PI/2[

		inoutValue += (SampleType)(m_fScale*(fWedgeHorizonAngle*fWedgeHorizonAngle));
	}

	// ---------------------------------------------------------------------------------

protected:

	float				m_fScale;			//!< 	to scale the input to the intermediate range

}; // CHemisphereSink_Solid




// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------

// only area between min and max angle is adding to the result
// useful for simulating a area light source
class CHemisphereSink_Slice : public CHemisphereSink_Solid
{
public:

	//! constructor
	CHemisphereSink_Slice( const DWORD indwAngleSteps ) :CHemisphereSink_Solid(indwAngleSteps)
	{
		m_fAngleAreaSubtract=0;
		m_fFullAngleArea=0;
	}

	void SetMinAndMax( const float infMinAngle, const float infMaxAngle )
	{
		m_fAngleAreaSubtract=infMinAngle*infMinAngle;

		m_fFullAngleArea=infMaxAngle*infMaxAngle - m_fAngleAreaSubtract;

		m_fFullAngleArea=max(m_fFullAngleArea,0.0001f);		// to avoid divide by zero

		m_fScale*=(gf_PI_DIV_2*gf_PI_DIV_2) / m_fFullAngleArea;
	}

	void AddWedgeArea( const float infSlope, SampleType &inoutValue ) const
	{
		float fWedgeHorizonAngle=(float)(gf_PI_DIV_2-atanf(-infSlope));			// [0..PI/2[

		float fAngleWedgeArea=( fWedgeHorizonAngle*fWedgeHorizonAngle - m_fAngleAreaSubtract);

		if(fAngleWedgeArea>0)
		{
			if(fAngleWedgeArea<m_fFullAngleArea) inoutValue += (SampleType)(m_fScale*fAngleWedgeArea);
				else inoutValue += (SampleType)(m_fScale*m_fFullAngleArea);
		}
	}

private:

	float			m_fAngleAreaSubtract;				//!<
	float			m_fFullAngleArea;						//!<
};


// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------





template <class THemisphereSink>
class CHeightmapAccessibility
{
public:
	//typedef typename THemisphereSink::SampleType SampleType;
	typedef	unsigned short	SampleType;			//!< 8.8 fix point

	//! constructor
	//! \param indwAngleSteps try 10 for average quality or more for better quality
	//! \param indwWidth has to be power of two
	//! \param indwHeight has to be power of two
	CHeightmapAccessibility( const DWORD indwWidth, const DWORD indwHeight,
		const DWORD indwAngleSteps, 
		const float infAngleStart=0.0f, const float infAngleEnd=(float)(gf_PI_MUL_2) )			:m_Sink(indwAngleSteps)
	{
		m_fAngleStart=infAngleStart;
		m_fAngleEnd=infAngleEnd;
		m_dwAngleSteps=indwAngleSteps;
		assert(indwWidth>=0);
		assert(indwHeight>=0);
		m_dwResultWidth=indwWidth;
		m_dwResultHeight=indwHeight;

		m_ResultBuffer.resize(m_dwResultWidth*m_dwResultHeight);				// check out of memory with exception handling

		assert(m_ResultBuffer.size()==m_dwResultWidth*m_dwResultHeight);
		m_bTiling=false;
	}




	//! check out of memory with exception handling
	//! \param indwWidth has to be power of two
	//! \param indwHeight has to be power of two
	//! \return true=success, false= out of memory
	bool Calc( const float *inpHeightmap )
	{
		assert(inpHeightmap);

		assert(m_fAngleEnd>=m_fAngleStart);

		// clear buffer
		{
			for(DWORD i=0;i<m_dwResultWidth*m_dwResultHeight;i++)
				m_Sink.InitSample(m_ResultBuffer[i]);
		}

		bool bProgress = false;
		if (m_dwResultWidth >= 512)
			bProgress = true;

		CWaitProgress progress( "Calculating Sky Accessibility",false );
		if (bProgress)
			progress.Start();

		// the horizon is subdivided in wedges
		for(DWORD dwWedge=0;dwWedge<m_dwAngleSteps;dwWedge++)
		{
			if (bProgress)
			{
				if (!progress.Step((100*dwWedge)/m_dwAngleSteps))
					return false;
			}

			float fAngle=(float)(dwWedge+0.5f)/(float)m_dwAngleSteps * (m_fAngleEnd-m_fAngleStart) + m_fAngleStart;
			float fDx=cosf(fAngle), fDy=sinf(fAngle);

			if(fabs(fDx)<fabs(fDy))			
			{
				// lines are mainly vertical
				float dx=fDx/fabsf(fDy);

				CalcHeightmapAccessWedge( inpHeightmap,		//
																	0,fDy>0?1:-1,		// line direction
																	dx>0?1:-1,0,		// line start direction (to fill area) = line step direction
																	fabsf(dx),m_dwResultWidth,m_dwResultHeight);			//
			}
			else
			{
				// lines are mainly horizontal
				float dy=fDy/fabsf(fDx);

				CalcHeightmapAccessWedge( inpHeightmap,		//
																	fDx>0?1:-1,0,		// line direction
																	0,dy>0?1:-1,		// line start direction (to fill area) = line step direction
																	fabsf(dy),m_dwResultHeight,m_dwResultWidth);			//
			}
		}

		if (bProgress)
			progress.Stop();

		return(true);		// success
	}

	SampleType GetSampleAt( const DWORD indwX, const DWORD indwY ) const
	{
		return(m_ResultBuffer[indwX+indwY*m_dwResultWidth]);
	}

	const SampleType *GetSamplePtr() const
	{
		assert(m_ResultBuffer.size()==m_dwResultWidth*m_dwResultHeight);

		return(&(m_ResultBuffer[0]));
	}

	// ---------------------------------------------------------------------------------

	// public to make if more convenient for the user

	THemisphereSink		m_Sink;							//!< is 
	bool							m_bTiling;					//!< true with tiling, false=faster

	// ---------------------------------------------------------------------------------

private:

	// properties (for a full hemisphere m_fAngleEnd-m_fAngleStart = gf_PI_DIV_2)
	float							m_fAngleStart;			//!< in rad
	float							m_fAngleEnd;				//!< in rad
	DWORD							m_dwAngleSteps;			//!< should be >4, more m_dwAngleSteps results in better quality and less speed

	typedef std::vector<SampleType>						CSampleBuffer;
	typedef std::vector<SampleType>::iterator	CSampleBufferIt;

	// result
	CSampleBuffer		m_ResultBuffer;			//!< 
	DWORD						m_dwResultWidth;		//!< value is power of two
	DWORD						m_dwResultHeight;		//!< value is power of two


	// ---------------------------------------------------------------------------------


	//! U is the direction we go always one pixel in positive direction (start point of the lines)
	//! V is the line direction we go one pixel in negative or in positive or not (depending on iniFix8)
	//! uses CalcHeightmapAccessWedge()
	//! \param inpHeightmap must not be 0
	//! \param infStep 0..1 (0.0f=no steps, 1.0f=45 degrees)
	//! \param indwLineCount
	//! \param indwLineLength
	//! \param infResultScale
	void CalcHeightmapAccessWedge( const float *inpHeightmap,
		const int iLineDirX, const int iLineDirY,
		const int iLineStepX, const int iLineStepY, 
		const float infStep, const DWORD indwLineCount, const DWORD indwLineLength )
	{
		assert(inpHeightmap);

		const float fLenStep=sqrtf( fabsf(infStep) + 1.0f);

		// this works only for power of two width and height
		const DWORD dwXMask=m_dwResultWidth-1;
		const DWORD dwYMask=m_dwResultHeight-1;
		const DWORD dwXShift=GetIntLog2(m_dwResultWidth);
		const DWORD dwYShift=GetIntLog2(m_dwResultHeight);

		CHorizonTracker HorizonTracker;
		// many lines fill the whole block
		for(DWORD dwLine=0;dwLine<indwLineCount;dwLine++)
		{
			HorizonTracker.Clear();
			int iX,iY;
			
			// start position of each line
			if(iLineDirY==0)				// mainly horizonal lines
			{
				iX = iLineDirX>0 ? 0 : m_dwResultWidth-1;
				iY = dwLine;
			}
			else										// mainly vertical lines
			{
				iX = dwLine;
				iY = iLineDirY>0 ? 0 : m_dwResultHeight-1;
			}

			float fFilterValue=0.5f;		// 0..1
			float fLen=0.0f;

			// one line
			if(!m_bTiling)
			for(DWORD dwLinePos=0; dwLinePos<indwLineLength; dwLinePos++,fLen+=fLenStep)
			{
				assert(fFilterValue>=0 && fFilterValue<=1.0f);

				// get two height samples
				float fHeight1=inpHeightmap[ (    iX    & dwXMask) + ((    iY    & dwYMask)<<dwXShift) ];
				float fHeight2=inpHeightmap[ ((iX+iLineStepX) & dwXMask) + (((iY+iLineStepY) & dwYMask)<<dwXShift) ];

				// get linear filtered height
				float fFilteredHeight=fHeight1*(1.0f-fFilterValue) + fHeight2*fFilterValue;

				if((iX>>dwXShift)!=0 || (iY>>dwYShift)!=0)
				{ 
					HorizonTracker.Clear();
					iX&=dwXMask;
					iY&=dwYMask;
				}

				float fSlope=HorizonTracker.Insert(fLen,fFilteredHeight);


				m_Sink.AddWedgeArea(fSlope,m_ResultBuffer[iX + (iY<<dwXShift)]);

				// advance one pixel
				iX+=iLineDirX;iY+=iLineDirY;

				fFilterValue+=infStep;

				if(fFilterValue>1.0f)
				{
					fFilterValue-=1.0f;
					iX+=iLineStepX;iY+=iLineStepY;
				}
			}


			// one line (two times longer to be sure the result is correct with tiling - optimizable)
			if(m_bTiling)
			for(DWORD dwLinePos=0; dwLinePos<indwLineLength*2; dwLinePos++,fLen+=fLenStep)
			{
				assert(fFilterValue>=0 && fFilterValue<=1.0f);

				// get two height samples
				float fHeight1=inpHeightmap[ (    iX    & dwXMask) + ((    iY    & dwYMask)<<dwXShift) ];
				float fHeight2=inpHeightmap[ ((iX+iLineStepX) & dwXMask) + (((iY+iLineStepY) & dwYMask)<<dwXShift) ];

				// get linear filtered height
				float fFilteredHeight=fHeight1*(1.0f-fFilterValue) + fHeight2*fFilterValue;

				float fSlope=HorizonTracker.Insert(fLen,fFilteredHeight);

				if(dwLinePos>=indwLineLength)	// first half of line length is neccessary to make it tileable
					m_Sink.AddWedgeArea(fSlope,m_ResultBuffer[(iX& dwXMask) + ((iY&dwYMask)<<dwXShift)]);

				// advance one pixel
				iX+=iLineDirX;iY+=iLineDirY;

				fFilterValue+=infStep;

				if(fFilterValue>1.0f)
				{
					fFilterValue-=1.0f;
					iX+=iLineStepX;iY+=iLineStepY;
				}
			}

		}
	}

}; // CHeightmapAccessibility


