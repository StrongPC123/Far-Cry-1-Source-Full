// ---------------------------------------------------------------------------------------------
//	Crytek CryENGINE source code
//	History:
//	- Created by Michael Glueck
// ---------------------------------------------------------------------------------------------

#include "stdafx.h"							// precompiled headers
#include "TexelSampler.h"					// CSimpleTriangleRastizer


void CTexelSampler::Init(const EAATYPE ceAAType)
{
	m_vSamples.clear();
	//process according to desired processing method
	//pay attention to add them sorted by u and then v
	switch(ceAAType)
	{
	case NONE:
		m_vSamples.push_back(SSample());	//default constructor did everything
		break;
	case MED:
		{
			SSample sample;
			sample.bTexelCenter = false;
			sample.fX = 0.f - 0.5f;
			sample.fY = 0.f - 0.5f;
			sample.fWeight = 0.125f;	//1/8 for corner texels
			m_vSamples.push_back(sample);
			sample.fX = 1.f - 0.5f;
			sample.fY = 0.f - 0.5f;
			m_vSamples.push_back(sample);				
			sample.fX = 0.5f - 0.5f;
			sample.fY = 0.5f - 0.5f;
			sample.fWeight = 0.5f;		//1/2 weight for center texel
			sample.bTexelCenter = true;
			m_vSamples.push_back(sample);				
			sample.fX = 0.f - 0.5f;
			sample.fY = 1.f - 0.5f;
			sample.fWeight = 0.125f;
			sample.bTexelCenter = false;
			m_vSamples.push_back(sample);
			sample.fX = 1.f - 0.5f;
			sample.fY = 1.f - 0.5f;
			m_vSamples.push_back(sample);
		}
		break;
	case HIGH:
		{
			//it is assumed to be equal to n^2
			SSample sample;
			//uniform grid sampling
			sample.fWeight = 1.f/(2.0f*(float)(HIGH-1));
			static const float scfGrid = sqrtf((float)HIGH);
			static const unsigned int scuiCount = (unsigned int)scfGrid;
			sample.bTexelCenter = false;
			float y = 1.f/(2.f * scfGrid);
			for(int v=0;v<scuiCount;v++)
			{
				float x = 1.f/(2.f * scfGrid);
				for(int u=0;u<scuiCount;u++)
				{
					x += 2.f/(2.f * scfGrid);
					sample.fX = x - 0.5f;
					sample.fY = y - 0.5f;
					m_vSamples.push_back(sample);
				}
				y += 2.f/(2.f * scfGrid);
			}
			//alter weight of center sample
			m_vSamples[(HIGH-1)/2].fWeight = 0.5f;
			m_vSamples[(HIGH-1)/2].bTexelCenter = true;
		}
		break;
	default:
		//use no AA in this case (shouldn't happen anyway)
		m_vSamples.push_back(SSample());		//default constructor did everything
	}
}