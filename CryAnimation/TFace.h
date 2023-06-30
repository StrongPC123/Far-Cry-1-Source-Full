#ifndef _T_FACE_HDR_
#define _T_FACE_HDR_

#pragma pack(push,1)
template <typename T>
struct TFace
{
	T v[3];
	TFace(T v0, T v1, T v2) {v[0] = v0; v[1] = v1; v[2] = v2;}
	TFace() {}
	TFace(const CryFace& face)
	{
		for (unsigned i = 0; i < 3; ++i)
			v[0] =(T)face[i];
	}
	template <class T1>
	void operator = (const T1& f)
	{
		v[0] = f.v[0];
		v[1] = f.v[1];
		v[2] = f.v[2];
	}
	//unsigned operator [] (unsigned nIndex) const {assert(nIndex < 3); return nVertex[nIndex];}
	operator T* () {return v;}
	operator const T* ()const {return v;}
};
#pragma pack(pop)

#endif