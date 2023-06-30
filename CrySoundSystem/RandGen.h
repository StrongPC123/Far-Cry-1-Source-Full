#pragma once

#ifndef WIN64
typedef unsigned int uint32;
#endif

#define N (624)                 // length of state vector

class CPseudoRandGen
{
private:
	uint32 state[N+1];     // state vector + 1 extra to not violate ANSI C
	uint32 *next;          // next random value is computed from here
	int left;					     // can *next++ this many times before reloading
private:
	uint32 Reload();
public:
	CPseudoRandGen();
	virtual ~CPseudoRandGen();
	void Seed(uint32 seed);
	uint32 Rand();
	float Rand(float fMin, float fMax);
};
