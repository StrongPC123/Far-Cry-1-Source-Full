// OBJExporter1.cpp: implementation of the COBJExporter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OBJExporter1.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COBJExporter::COBJExporter()
{

}

COBJExporter::~COBJExporter()
{

}

bool COBJExporter::WriteOBJ(const char * pszFileName)
{
	//////////////////////////////////////////////////////////////////////
	// Write the OBJ file to disk
	//////////////////////////////////////////////////////////////////////

	FILE *hFile = NULL;
	VectorIt itVec;
	FaceIt itFace;
	TexCoordIt itTexCoord;
	CString szMTL;
	char ppszBuffer[3][32];

	ASSERT(pszFileName);

	CLogFile::FormatLine("Exporting OBJ file to '%s'", pszFileName);

	// Open the file
	hFile = fopen(pszFileName, "w");

	if (!hFile)
	{
		CLogFile::FormatLine("Error while opening file '%s' !", pszFileName);
		ASSERT(hFile);
		return false;
	}

	// Write header
	fprintf(hFile, "# Object file exported by CryEdit\n\n");

	// Create MTL library filename
	szMTL = pszFileName;
	szMTL = Path::ReplaceExtension( szMTL,"mtl" );
	szMTL = Path::GetFile(szMTL);

	// Write material library import statement
	fprintf(hFile, "mtllib %s\n\n", (const char*)szMTL);

	// Write material usage statement
	fprintf(hFile, "usemtl Terrain\n\n");

	// Write all vertices
	for (itVec=m_vVertices.begin(); itVec!=m_vVertices.end(); itVec++)
	{
		fprintf(hFile, "v %s %s %s\n", 
			StripZero(ppszBuffer[0], (* itVec).fX),
			StripZero(ppszBuffer[1], (* itVec).fY), 
			StripZero(ppszBuffer[2], (* itVec).fZ));
	}
	fprintf(hFile, "# %i Vertices\n\n", m_vVertices.size());

	// Write all texture coordinates
	for (itTexCoord=m_vTexCoords.begin(); itTexCoord!=m_vTexCoords.end(); itTexCoord++)
	{
		fprintf(hFile, "vt %s %s\n",
			StripZero(ppszBuffer[0], (* itTexCoord).fU),
			StripZero(ppszBuffer[1], (* itTexCoord).fV));
	}
	fprintf(hFile, "# %i Texture coordinates\n\n", m_vTexCoords.size());

	// Write all faces, convert the indices to one based indices
	for (itFace=m_vFaces.begin(); itFace!=m_vFaces.end(); itFace++)
	{
		fprintf(hFile, "f %i/%i %i/%i %i/%i\n",
			(* itFace).iVertexIndices[0] + 1, (* itFace).iTexCoordIndices[0] + 1,
			(* itFace).iVertexIndices[1] + 1, (* itFace).iTexCoordIndices[1] + 1,
			(* itFace).iVertexIndices[2] + 1, (* itFace).iTexCoordIndices[2] + 1);
	}
	fprintf(hFile, "# %i Faces", m_vFaces.size());

	fclose(hFile);

	// Create MTL library filename
	szMTL = Path::ReplaceExtension( pszFileName,"mtl" );


	// Open the material file
	hFile = fopen(szMTL, "w");

	if (!hFile)
	{
		CLogFile::FormatLine("Error while opening file '%s' !", (const char*)szMTL);
		ASSERT(hFile);
		return false;
	}

	// Write header
	fprintf(hFile, "# Material file exported by CryEdit\n\n");

	// Write terrain material
	fprintf(hFile, "newmtl Terrain\n");
	fprintf(hFile, "Ka 1.000000 1.000000 1.000000\n");
	fprintf(hFile, "Kd 1.000000 1.000000 1.000000\n");
	fprintf(hFile, "Ks 1.000000 1.000000 1.000000\n");
	fprintf(hFile, "Ns 0.000000\n");
	fprintf(hFile, "map_Kd Terrain.bmp");

	fclose(hFile);
	
	return true;
}

char * COBJExporter::StripZero(char pszOutputBuffer[32], float fNumber)
{
	////////////////////////////////////////////////////////////////////////
	// Convert a float into a string representation and remove all 
	// uneccessary zeroes and the decimal dot if it is not needed
	////////////////////////////////////////////////////////////////////////

	long i;

	sprintf(pszOutputBuffer, "%f", fNumber);

	for (i=strlen(pszOutputBuffer) - 1; i>=0; i--)
	{
		if (pszOutputBuffer[i] == '0')
			pszOutputBuffer[i] = '\0';
		else if (pszOutputBuffer[i] == '.')
		{
			pszOutputBuffer[i] = '\0';
			break;
		}
		else
			break;
	}

	return pszOutputBuffer;
}