#ifndef __RandGen_h__
#define __RandGen_h__
#pragma once

#define N_RAND_STATE (624)                 // length of state vector
class CSysPseudoRandGen
{
private:
	uint32 state[N_RAND_STATE+1];     // state vector + 1 extra to not violate ANSI C
	uint32 *next;          // next random value is computed from here
	int left;					     // can *next++ this many times before reloading
private:
	uint32 Reload();
public:
	CSysPseudoRandGen();
	virtual ~CSysPseudoRandGen();
	void Seed(uint32 seed);
	uint32 Rand();
	float Rand(float fMin, float fMax);
};

#endif // __RandGen_h__