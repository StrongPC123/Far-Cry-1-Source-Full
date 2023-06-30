#pragma once

#include <assert.h>				
#include <vector>

/**
*	class contains the functionality for retrieving samples relative to a texel center 
*	uses prepared raster table
*	texel ranges from -0.5,-0.5 to 0.5,0.5, therefore texel center located at 0.0,0.0
*	one is guaranteed to be set as texel center
*/
class CTexelSampler
{
public:
	//!< describes one sample
	typedef struct SSample
	{
		float fX;			//!< u-position relative to texel center
		float fY;			//!< v-position relative to texel center
		float fWeight;		//!< weight for this sample
		bool bTexelCenter;	//!< true if to use as texel center, default true to guarantee at least one valid sample
		SSample():fX(0.f),fY(0.f),fWeight(1.0f),bTexelCenter(true){}
	}SSample;
	
	//!< enumerates the available antialiasing approaches
	typedef enum EAATYPE
	{
		NONE = 1,		//!< only texel center gets processed
		MED = 5,		//!< texel center and all corners gets sampled
		HIGH = 9		//!< texel center and 8 additional samples get used
	}EAATYPE;
	/**
	* constructor which initializes the sample table
	* @param ceAAType desired accuracy for antialiasing (determines number of samples to take)
	*/
	CTexelSampler(const EAATYPE ceAAType = MED)
	{
		Init(ceAAType);
	}
	/**
	* reinitializes the sample table
	* @param ceAAType desired accuracy for antialiasing (determines number of samples to take)
	*/
	void Reinitialize(const EAATYPE ceAAType = MED)
	{
		Init(ceAAType);
	}
	/**
	* returns number of sampels per texel, sorted to make reuse possible
	* @return number of samples
	*/
	const unsigned int NumberOfSamples() const
	{
		return m_vSamples.size();
	}
	/**
	* retrieves an const iterator to the samples
	* @param const iterator to the sample vector
	*/
	std::vector<SSample>::const_iterator BeginIterator() const
	{
		return m_vSamples.begin();
	}
	/**
	* retrieves an const iterator to the samples
	* @param const iterator to the sample vector
	*/
	std::vector<SSample>::const_iterator EndIterator() const
	{
		return m_vSamples.end();
	}

private:
	/**
	* initializes sampler, called from Reinitialize and contructor
	* @param ceAAType desired accuracy for antialiasing (determines number of samples to take)
	*/
	void Init(const EAATYPE ceAAType);

	std::vector<SSample> m_vSamples;					//!< sample offset related to respective texel center, sorted by u and then v
};

typedef std::vector<CTexelSampler::SSample>::const_iterator SampleIter;