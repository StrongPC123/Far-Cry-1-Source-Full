/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CGF_UTILS_HDR_
#define _CGF_UTILS_HDR_

// prerequisities
//#include <CryHeaders.h>
//#include <StlDbgAlloc.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

// given the chunk, parses it and returns the array of pointers to the strings with the bone names 
// in the supplied array. Returns true when successful
extern bool LoadBoneNameList (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize, std::vector<const char*>& arrNames);



enum MatChunkLoadErrorEnum
{
	MCLE_Success,
	MCLE_Truncated,
	MCLE_UnknownVersion,
	MCLE_IgnoredType // this material type should be ignored (multimaterial)
};


// Attempts to load the material from the given material chunk to the "me" structure
extern MatChunkLoadErrorEnum LoadMatEntity (const CHUNK_HEADER& chunkHeader, const void* pChunkData, unsigned nChunkSize, MAT_ENTITY& me);

// material name parser.
// In the CGF, the material has to have a complex name:
// name_spec template_spec phys_mat_spec render_index_spec
//
// name_spec:
//  $s_*           -- * means the template name; in this case, material name remains $s_*
//  *              -- * means just the material name
//  
// template_spec:  -- not necessary
//  (*)            -- * means the template; closing bracket may be absent
//
// phys_mat_spec:  -- not necessary
//  /name
//
// render_index_spec: -- not necessary
//  [id]
class CMatEntityNameTokenizer {

public:
	CMatEntityNameTokenizer ();
	~CMatEntityNameTokenizer ();

	void tokenize(const char* szName);

	// material name
	const char *szName;

	// template name (NULL if no template)
	const char *szTemplate;

	// physic material name
	const char *szPhysMtl;

	// render index (0 if not specified)
	int nSortValue;

	// true, if the template is to be inverted.. (# before the template name
	bool bInvert;

	// operator that sorts the materials for rendering
	bool operator < (const CMatEntityNameTokenizer& right)const;
protected:
	char* m_szMtlName;
};

// this is sorting predicate that helps form a mapping of old-new mat ids
// it's given an array of tokenizers to index into
class CMatEntityIndexSort
{
public:
	// accepts the array of initialized tokenizers and their number
	CMatEntityIndexSort (const CMatEntityNameTokenizer* pTokenizers, unsigned numSize):
		m_pTokenizers (pTokenizers),
		m_numSize (numSize)
	{
	}

	bool operator () (unsigned nLeft, unsigned nRight) const
	{
		assert (nLeft >= 0 && nLeft < m_numSize);
		assert (nRight >= 0 && nRight < m_numSize);
		return m_pTokenizers[nLeft ]<m_pTokenizers[nRight];
	}

protected:
	const CMatEntityNameTokenizer* m_pTokenizers;
	unsigned m_numSize;
};

// given the in-permutation, constructs the inverse out-permutation, so that:
// pOut[pIn[i]] == i
// pIn[pOut[i]] == i
void ConstructReversePermutation (const unsigned* pIn, unsigned* pOut, unsigned num);

// Remaps the materials according to the given permutation
// the permutation is perm[new index] == old index
void RemapMatEntities (MAT_ENTITY* pMtls, unsigned numMtls, unsigned* pPerm);

// copies the matrix from  SBoneInitPosMatrix to Matrix
extern void copyMatrix (Matrix44& matDst, const SBoneInitPosMatrix& matSrc);

struct SBasisProperties
{
#ifdef _LEAFBUFFER_H_
	SBasisProperties (const TangData& td)
	{
		init (td.tangent, td.binormal, td.tnormal);
	}
#endif

	SBasisProperties(const Vec3& vX, const Vec3& vY, const Vec3& vZ)
	{
		init (vX,vY,vZ);
	}

	static double maxCosToErrorDeg(double f)
	{
		return 90 - fabs(acos(f)*180/M_PI);
	}

	void init (const Vec3& vX, const Vec3& vY, const Vec3& vZ)
	{
		bMatrixDegraded = false;
		bLeftHanded     = false;
		fErrorDeg       = 0;
		// max of cosine between two vectors
		double fMaxCos = 0;
		float fXLength = vX.Length(), fYLength = vY.Length(), fZLength = vZ.Length();

		if (fXLength < 1e-3 || fYLength < 1e-3 || fZLength < 1e-3)
			bMatrixDegraded = true;
		else
		{
			double fMaxCosXY = fabs(vX*vY)/(fXLength*fYLength);
			fMaxCos = max (fMaxCos, fMaxCosXY);
			double fMaxCosXZ = fabs(vX*vZ)/(fXLength*fZLength);
			fMaxCos = max (fMaxCos, fMaxCosXZ);
			double fMaxCosYZ = fabs(vY*vZ)/(fYLength*fZLength);
			fMaxCos = max (fMaxCos, fMaxCosYZ);

			fErrorDeg = maxCosToErrorDeg(fMaxCos);
			fErrorDegTB = (float)maxCosToErrorDeg(fMaxCosXY);
			fErrorDegTN = (float)maxCosToErrorDeg(fMaxCosXZ);
			fErrorDegBN = (float)maxCosToErrorDeg(fMaxCosYZ);

			bLeftHanded = (vX^vY)*vZ < 0;
		}

	}

	double fErrorDeg;
	// the error degrees between the tangent, binormal and normal by pair
	float fErrorDegTB, fErrorDegTN, fErrorDegBN;
	bool bMatrixDegraded;
	bool bLeftHanded;

	bool isOrthogonal () const {return fErrorDeg < 0.5;}
	double getLeftHandednessPercentage () const {return (100-fErrorDeg*100/90);}
};

extern const char* getMtlType (unsigned nMtlType);
extern const char* getTexType (unsigned char nTexType);
extern string getLightType (LightTypes nType);
extern string getMtlFlags (int nFlags);

#endif