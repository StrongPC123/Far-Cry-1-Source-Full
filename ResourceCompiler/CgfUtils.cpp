/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "StringUtils.h"
#include "CgfUtils.h"
 
using namespace CryStringUtils;

//////////////////////////////////////////////////////////////////////////
// given the chunk, parses it and returns the array of pointers to the strings with the bone names 
// in the supplied array. Returns true when successful
// PARAMETERS:
//  chunkHeader - the header of the chunk in the chunk table
//  pChunk      - the raw chunk data in the file
//  nChunkSize  - the size of the raw data piece in the file
//  arrNames    - the array that will be resized according to the number of entries, and each element of this array
//                will be the pointer to the corresponding 0-terminated name string.
bool LoadBoneNameList (const CHUNK_HEADER& chunkHeader, const void* pChunk, unsigned nChunkSize, std::vector<const char*>& arrNames)
{
	switch (chunkHeader.ChunkVersion)
	{
		case BONENAMELIST_CHUNK_DESC_0744::VERSION:
		{
			// the old chunk version - fixed-size name list
			const BONENAMELIST_CHUNK_DESC_0744* pNameChunk = (const BONENAMELIST_CHUNK_DESC_0744*)(pChunk);

			int nGeomBones = pNameChunk->nEntities;

			// just fill in the pointers to the fixed-size string buffers
			const NAME_ENTITY* pGeomBoneNameTable = (const NAME_ENTITY*)(pNameChunk+1);
			if(nGeomBones < 0 || nGeomBones > 0x800)
				return false;
			arrNames.resize(nGeomBones);
			for (int i = 0; i < nGeomBones; ++i)
				arrNames[i] = pGeomBoneNameTable[i].name;
		}
		break;

		case BONENAMELIST_CHUNK_DESC_0745::VERSION:
		{
			// the new memory-economizing chunk with variable-length packed strings following tightly each other
			const BONENAMELIST_CHUNK_DESC_0745* pNameChunk = (const BONENAMELIST_CHUNK_DESC_0745*)(pChunk);
			int nGeomBones = pNameChunk->numEntities;

			// we know how many strings there are there; the whole bunch of strings may 
			// be followed by pad zeros, to enable alignment
			arrNames.resize(nGeomBones, "");
			
			// scan through all the strings, each string following immediately the 0 terminator of the previous one
			const char* pNameListEnd = ((const char*)pNameChunk) + nChunkSize;
			const char* pName = (const char*)(pNameChunk+1);
			int nName = 0;
			while (*pName && pName < pNameListEnd && nName < nGeomBones)
			{
				arrNames[nName] = pName;
				pName += strnlen(pName, pNameListEnd) + 1;
				++nName;
			}
			if (nName < nGeomBones)
			{
				// the chunk is truncated
#ifdef _CRY_ANIMATION_BASE_HEADER_
				// if this is in the engine, log the error
				g_GetLog()->LogWarning ("\003inconsistent bone name list chunk: only %d out of %d bone names have been read.", nName, nGeomBones);
#endif
				return false;
			}
		}
		break;
	}

	return true;
}



// Attempts to load the material from the given material chunk to the "me" structure
MatChunkLoadErrorEnum LoadMatEntity (const CHUNK_HEADER& chunkHeader, const void* pChunkData, unsigned nChunkSize, MAT_ENTITY& me)
{
	memset(&me, 0, sizeof(MAT_ENTITY));

	switch (chunkHeader.ChunkVersion) 
	{
	case MTL_CHUNK_DESC_0746::VERSION:
	{
    const MTL_CHUNK_DESC_0746* pMatChunk = (const MTL_CHUNK_DESC_0746*)pChunkData;
    
    me.m_New = 2;
    strcpy(me.name, pMatChunk->name);
    switch (pMatChunk->MtlType)
    {
      case MTL_STANDARD:
				{
        me.IsStdMat = true;
        me.col_d = pMatChunk->col_d;
        me.col_a = pMatChunk->col_a;
        //me.col_a.g=0;
        //me.col_a.b=0;
        me.col_s = pMatChunk->col_s;
        
        me.specLevel = pMatChunk->specLevel;
        me.specShininess = pMatChunk->specShininess*100;
        me.opacity = pMatChunk->opacity;
        me.selfIllum = pMatChunk->selfIllum;
        me.flags = pMatChunk->flags;
        
        me.Dyn_Bounce = pMatChunk->Dyn_Bounce;
        me.Dyn_StaticFriction = pMatChunk->Dyn_StaticFriction;
        me.Dyn_SlidingFriction = pMatChunk->Dyn_SlidingFriction;
        /* //Timur[10/24/2001] 
        strcpy(me.map_a, pMatChunk->tex_a.name);
        strcpy(me.map_d, pMatChunk->tex_d.name);
        strcpy(me.map_o, pMatChunk->tex_o.name);
        strcpy(me.map_b, pMatChunk->tex_b.name);
        strcpy(me.map_s, pMatChunk->tex_s.name);
        strcpy(me.map_g, pMatChunk->tex_g.name);
        strcpy(me.map_c, pMatChunk->tex_c.name);
        strcpy(me.map_e, pMatChunk->tex_rl.name);
        strcpy(me.map_rr, pMatChunk->tex_rr.name);
        strcpy(me.map_det, pMatChunk->tex_det.name);
        */
        me.map_a = pMatChunk->tex_a;
        me.map_d = pMatChunk->tex_d;
        me.map_o = pMatChunk->tex_o;
        me.map_b = pMatChunk->tex_b;
        me.map_s = pMatChunk->tex_s;
        me.map_g = pMatChunk->tex_g;
        me.map_detail = pMatChunk->tex_fl;
        me.map_e = pMatChunk->tex_rl;
        me.map_subsurf = pMatChunk->tex_subsurf;
        me.map_displ = pMatChunk->tex_det;
        
        me.nChildren = pMatChunk->nChildren;

				me.alpharef = pMatChunk->alphaTest;
				}        
        return MCLE_Success;
        
			default:
				return MCLE_IgnoredType;
    }
	}
	break;
	case MTL_CHUNK_DESC_0745::VERSION:
  {
    const MTL_CHUNK_DESC_0745* pMatChunk = (const MTL_CHUNK_DESC_0745*)pChunkData;
    
    me.m_New = 1;
    strcpy(me.name, pMatChunk->name);
    switch (pMatChunk->MtlType)
    {
      case MTL_STANDARD:
        me.IsStdMat = true;
        me.col_d = pMatChunk->col_d;
        me.col_a = pMatChunk->col_a;
				//me.col_a.g=0;
				//me.col_a.b=0;
        me.col_s = pMatChunk->col_s;
        
        me.specLevel = pMatChunk->specLevel;
        me.specShininess = pMatChunk->specShininess*100;
        me.opacity = pMatChunk->opacity;
        me.selfIllum = pMatChunk->selfIllum;
        me.flags = pMatChunk->flags;
        
        me.Dyn_Bounce = pMatChunk->Dyn_Bounce;
        me.Dyn_StaticFriction = pMatChunk->Dyn_StaticFriction;
        me.Dyn_SlidingFriction = pMatChunk->Dyn_SlidingFriction;
				me.map_a = pMatChunk->tex_a;
        me.map_d = pMatChunk->tex_d;
        me.map_o = pMatChunk->tex_o;
        me.map_b = pMatChunk->tex_b;
        me.map_s = pMatChunk->tex_s;
        me.map_g = pMatChunk->tex_g;
        me.map_detail = pMatChunk->tex_c;
        me.map_e = pMatChunk->tex_rl;
        me.map_subsurf = pMatChunk->tex_subsurf;
        me.map_displ = pMatChunk->tex_det;

        me.nChildren = pMatChunk->nChildren;

        return MCLE_Success;
        
			default:
				return MCLE_IgnoredType;
    }
  }
	break;
	default:
		return MCLE_UnknownVersion;
  }
}


// if the given string is null, assigns the "" to the pointer
void chkNullString (const char*&pszString)
{
	if (!pszString)
		pszString = "";
}

static void rtrim (char* szString)
{
	for (char* p = szString + strlen (szString) - 1; p >= szString && isspace(*p); --p)
		*p = '\0';
}

static void ltrim (const char*& szString)
{
	while (*szString && isspace(*szString))
		++szString;
}


CMatEntityNameTokenizer::CMatEntityNameTokenizer ():
	m_szMtlName (NULL),
	szName (""),
	szTemplate (""),
	szPhysMtl (""),
	nSortValue (0),
	bInvert (false)
	{
	}


void CMatEntityNameTokenizer::tokenize (const char* szMtlFullName)
{
	if (m_szMtlName)
	{
		free (m_szMtlName);
		m_szMtlName = NULL;
	}	
	if (!szMtlFullName)
		return;

	int nLen = (int)strlen(szMtlFullName);
	m_szMtlName = (char*)malloc (nLen+1);
	memcpy (m_szMtlName, szMtlFullName, nLen + 1);

	szName = NULL;
	szTemplate = NULL;
	szPhysMtl = NULL;
	nSortValue = 0;
	bInvert = false;

	// the state machine will parse the whole string
	enum StateEnum
	{
		kUnknown,
		kName,
		kTemplate,
		kPhysMtl,
		kIndex
	};

	StateEnum nState = kName;  // by default, the string begins with name
	this->szName = m_szMtlName;

	for (char* p = m_szMtlName; *p; ++p)
	{
		switch (*p)
		{
			case '(': // template name begins
				this->szTemplate = p+1;
				nState = kTemplate;
				*p = '\0';
				break;

			case '[': // render priority number begins
				*p = '\0';
				nState = kIndex;
				break;

			case '/':
				this->szPhysMtl = p+1;
				*p = '\0';
				nState = kPhysMtl;
				break;

		default:
		switch (nState)
		{
		case kName:
			switch (*p)
			{
			/*case ' ': // there are no spaces in the name
				*p = '\0';
				break;*/

			case ')':
			case ']':
#ifdef _CRY_ANIMATION_BASE_HEADER_
					g_GetLog()->LogError ("Invalid material name (unexpected closing bracket) \"%s\"", szMtlFullName);
#endif
				break;
			};
		break;

		case kTemplate:
			switch (*p)
			{
			case ')':
				nState = kUnknown;
				*p = '\0';
				break;
			}
		break;

		case kIndex:
			{
				switch (*p)
				{
				case ']':
					nState = kUnknown;
					break;
				
				case ' ':
					break;

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					this->nSortValue *= 10;
					this->nSortValue += *p - '0';
					break;

				default:
					nState = kUnknown;
#ifdef _CRY_ANIMATION_BASE_HEADER_
					g_GetLog()->LogError ("Invalid material name (unexpected symbol in index field) \"%s\"", szMtlFullName);
#endif
					break;
				}
			}
		break;

		};
		break;
		}
	}

	// take into account hte old form $s_... of setting the template name
	// if there was no () template, then use this one (after _)
	if ((!this->szTemplate || !this->szTemplate[0]) &&
		this->szName[0] == '$' && tolower(this->szName[1]) == 's' && this->szName[2] == '_')
	{
		this->szTemplate = this->szName + 3;
	}

	// make sure all the strings get their pointers - if there's no name, then it will be an empty name
	chkNullString(this->szName);
	chkNullString(this->szTemplate);
	chkNullString(this->szPhysMtl);

	// a special case (one more) - template name preceded by # means (or meant) the inverted template
	if (this->szTemplate[0] == '#')
	{
		this->szTemplate++;
		this->bInvert = true;
	}

	// trim unneeded left and right leading spaces
	rtrim ((char*)this->szName);
	rtrim ((char*)this->szTemplate);
	rtrim ((char*)this->szPhysMtl);

	ltrim (this->szName);
	ltrim (this->szTemplate);
	ltrim (this->szPhysMtl);
}

CMatEntityNameTokenizer::~CMatEntityNameTokenizer ()
{
	if (m_szMtlName)
		free (m_szMtlName);
}

// operator that sorts the materials for rendering
bool CMatEntityNameTokenizer::operator < (const CMatEntityNameTokenizer& right)const
{
	if (this->nSortValue < right.nSortValue)
		return true;

	if (this->nSortValue > right.nSortValue)
		return false;

	int nComp = stricmp (this->szTemplate, right.szTemplate);

	if (nComp < 0)
		return true;
	if (nComp > 0)
		return false;

	nComp = stricmp (this->szName, right.szName);

	if (nComp < 0)
		return true;
	if (nComp > 0)
		return false;
	
	return false; // they're equal
}

// given the in-permutation, constructs the inverse out-permutation, so that:
// pOut[pIn[i]] == i
// pIn[pOut[i]] == i
void ConstructReversePermutation (const unsigned* pIn, unsigned* pOut, unsigned num)
{
	unsigned i;
#ifdef _DEBUG
	// we'll check the correctness of permutation by checking if there are duplicate entries in pIn
	// if there are no duplicate entries, according to Dirichle principle, there are no missed entries in pOut
	for (i = 0; i < num; ++i)
		pOut[i] = (unsigned)-1;
#endif

	for (i = 0; i < num; ++i)
	{
		assert (pIn[i] < num);
		assert ((int)pOut[pIn[i]] ==(unsigned)-1);
		pOut[pIn[i]] = i;
	}

}

// Remaps the materials according to the given permutation
// the permutation is perm[new index] == old index
void RemapMatEntities (MAT_ENTITY* pMtls, unsigned numMtls, unsigned* pPerm)
{
	MAT_ENTITY* pOldMtls = new MAT_ENTITY[numMtls];
	memcpy (pOldMtls, pMtls, sizeof(MAT_ENTITY)*numMtls);

	for (unsigned nNewMtl = 0; nNewMtl < numMtls; ++nNewMtl)
		memcpy (pMtls + nNewMtl, pOldMtls + pPerm[nNewMtl], sizeof(MAT_ENTITY));

	delete[]pOldMtls;	
}

// copies the matrix from  SBoneInitPosMatrix to Matrix
void copyMatrix (Matrix44& matDst, const SBoneInitPosMatrix& matSrc)
{
	for (unsigned i = 0; i < 4; ++i)
	{
		matDst(i,0) = matSrc[i][0];
		matDst(i,1) = matSrc[i][1];
		matDst(i,2) = matSrc[i][2];
	}
	matDst(0,3) = 0;
	matDst(1,3) = 0;
	matDst(2,3) = 0;
	matDst(3,3) = 1;
}

const char* getMtlType (unsigned nMtlType)
{
	switch (nMtlType)
	{
	case MTL_UNKNOWN:
		return "UNKNOWN";
	case MTL_STANDARD:
		return "STANDARD";
	case MTL_MULTI:
		return "MULTI";
	case MTL_2SIDED:
		return "2SIDED";
	default:
		return "#Unknown#";
	}
}

const char* getTexType (unsigned char nTexType)
{
	switch (nTexType)
	{
	case TEXMAP_AUTOCUBIC:
		return "TEXMAP_AUTOCUBIC";
	case TEXMAP_CUBIC:
		return "TEXMAP_CUBIC";
	case TEXMAP_ENVIRONMENT:
		return "TEXMAP_ENVIRONMENT";
	case TEXMAP_SCREENENVIRONMENT:
		return "!TEXMAP_SCREENENVIRONMENT(unsupported)!";
	default:
		return "#Unknown#";
	}
}

string getLightType (LightTypes nType)
{
	switch (nType)
	{
	case LT_OMNI:
		return "Omni";
	case LT_SPOT:
		return "Spot";
	case LT_DIRECT:
		return "Direct";
	case LT_AMBIENT:
		return "Ambient";
	default:
		{
			char szBuffer[32];
			printf (szBuffer, "Unknown(%d)", nType);
			return szBuffer;
		}
	}
}

string getMtlFlags (int nFlags)
{
	string strResult;
	if (nFlags & MTLFLAG_WIRE)
		strResult += "MTLFLAG_WIRE|";
	if (nFlags & MTLFLAG_2SIDED)
		strResult += "MTLFLAG_2SIDED|";
	if (nFlags & MTLFLAG_FACEMAP)
		strResult += "MTLFLAG_FACEMAP|";
	if (nFlags & MTLFLAG_FACETED)
		strResult += "MTLFLAG_FACETED|";
	if (nFlags & MTLFLAG_ADDITIVE)
		strResult += "MTLFLAG_ADDITIVE|";
	if (nFlags & MTLFLAG_SUBTRACTIVE)
		strResult += "MTLFLAG_SUBTRACTIVE|";

	if (!strResult.empty ())
		strResult.resize (strResult.length ()-1);

	return strResult;
}
