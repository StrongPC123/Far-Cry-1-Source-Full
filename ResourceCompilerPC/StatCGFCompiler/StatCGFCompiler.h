#ifndef STAT_CGF_COMPILER
#define STAT_CGF_COMPILER

#include "IConvertor.h"

struct ConvertContext;
class CStatCFGCompiler : public IConvertor
{
public:
	class Error
	{
	public:
		Error (int nCode);
		Error (const char* szFormat, ...);
		const char* c_str()const {return m_strReason.c_str();}
	protected:
		string m_strReason;
	};

	CStatCFGCompiler(void);
	~CStatCFGCompiler(void);
	bool Process( ConvertContext &cc );
	void Release() { delete this; };
	bool GetOutputFile( ConvertContext &cc );
	int GetNumPlatforms() const;
	Platform GetPlatform( int index ) const;
	virtual int GetNumExt() const { return 2; };
	virtual const char* GetExt( int index ) const { return index ? "cga" : "cgf"; };
	DWORD GetTimestamp() const;
	bool GetSourceFileTime(const char * szFileName, FILETIME & fileTime);
		
protected:
	int SetTexType(struct TextureMap3 *tm);
	bool PrepareTexSpaceBasis();
	void FindDependencies( CIndexedMesh * pIndexedMesh, ConvertContext &cc );
	void GetFileParams( ConvertContext &cc, CString & sGeomName, 
		bool & bStripify, bool & bLoadAdditinalInfo, bool & bKeepInLocalSpace);
	void ProcessCGA( ConvertContext &cc,CString &sourceFile,CString &outputFile,FILETIME fileTime,bool bStripify );
	void ProcessCGF( ConvertContext &cc,CString &sourceFile,CString &outputFile,FILETIME fileTime,CString sGeomName,
		bool bStripify, bool bLoadAdditinalInfo, bool bKeepInLocalSpace );
};

struct CSimpleREOcLeaf
{
  TArray<SMeshFace> * m_Faces;
  CMatInfo * m_pChunk;
  class CSimpleLeafBuffer * m_pBuffer;

	CSimpleREOcLeaf() {memset(this,0,sizeof(CSimpleREOcLeaf));}
	~CSimpleREOcLeaf()
	{
		delete m_Faces;
	}
};

struct CBasis
{
	CBasis()
	{
		tangent(0,0,0);
		binormal(0,0,0);
		tnormal(0,0,0);			
	}
	Vec3d tangent, binormal, tnormal;
};

class CSimpleLeafBuffer
{
public:
	CSimpleLeafBuffer(IRCLog * pLog, CIndexedMesh * pIndexedMesh, bool bStripifyAndShareVerts, bool bKeepRemapTable=false);
  ~CSimpleLeafBuffer();
	bool Serialize(int & nPos, uchar * pSerBuf, bool bSave, const char * szFolderName);
//	CBasis * m_pBasises;

protected:
  void CreateLeafBuffer( CIndexedMesh * pTriData, int Stripify, bool bShareVerts, bool bKeepRemapTable=false );
  Vec3d    *m_TempNormals;
  SMRendTexVert *m_TempTexCoords;
  UCol          *m_TempColors;
	int m_SecVertCount;
  list2<CMatInfo> * m_pMats;

	IRCLog * m_pLog;

  bool compute_tangent( const float * v0, const float * v1, const float * v2, 
                        const float * t0, const float * t1, const float * t2, 
                        Vec3d & tangent, Vec3d & binormal, Vec3d & tnormal, Vec3d & face_normal);

	void CompactBuffer(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F * _vbuff, int * _vcount, 
		list2<unsigned short> * pindices, bool bShareVerts[128], uint *uiInfo, CBasis * pBasises);

  list2<unsigned short> *m_pIndices, *m_pIndicesPreStrip;
  void *m_pD3DIndBuf;
	list2<unsigned short> & GetIndices() { return *m_pIndices; }
	CVertexBuffer * m_pSecVertBuffer;
	void StripifyMesh(int StripType, CBasis *pTangNonStrip);
	void CalcFaceNormals();
	bool CreateTangBuffer(CBasis * pBasises);
  Vec3d * m_pLoadedColors;
	Vec3d m_vBoxMin, m_vBoxMax;
	int FindInBuffer(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F &opt, CBasis &origBasis, uint nMatInfo, uint *uiInfo, struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F* _vbuff, CBasis *_vbasis, int _vcount, list2<unsigned short> * pHash, TArray<uint>& ShareNewInfo);
  uint * m_arrVertStripMap;
	int m_nPrimetiveType;
	bool PrepareTexSpaceBasis();
	void CorrectTangentBasisesForPolyBump( TangData * pDuplTangData = 0);

	CSimpleLeafBuffer *m_pVertexContainer;
  CVertexBuffer * m_pVertexBuffer;
	uint * m_arrVtxMap; //!< [Anton] mapping table leaf buffer vertex idx->original vertex idx

  _inline CSimpleLeafBuffer *GetVertexContainer(void)
  {
    if (m_pVertexContainer)
      return m_pVertexContainer;
    return this;
  }

	byte *GetNormalPtr(int& Stride, int Id=0, bool bSys=true)
  {
    CSimpleLeafBuffer *lb = GetVertexContainer();
    byte *pData;
    SBufInfoTable * pOffs;
    if (bSys)
    {
      pData = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
      pOffs = &gBufInfoTable[lb->m_pSecVertBuffer->m_vertexformat];
      Stride = m_VertexSize[lb->m_pSecVertBuffer->m_vertexformat];
    }
    else
    {
      pData = (byte *)lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
      pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
      Stride = m_VertexSize[lb->m_pVertexBuffer->m_vertexformat];
    }
    if (pOffs->OffsNormal)
    {
      return &pData[Id*Stride+pOffs->OffsNormal];
    }

    Stride = sizeof(Vec3d);
    return (byte*)&lb->m_TempNormals[Id];
  }

  byte *GetPosPtr(int& Stride, int Id=0, bool bSys=true)
  {
    CSimpleLeafBuffer *lb = GetVertexContainer();
    byte *pData;
    if (bSys)
    {
      pData = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
      Stride = m_VertexSize[lb->m_pSecVertBuffer->m_vertexformat];
    }
    else
    {
      pData = (byte *)lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
      Stride = m_VertexSize[lb->m_pVertexBuffer->m_vertexformat];
    }
    return &pData[Id*Stride];
  }

  byte *GetBinormalPtr(int& Stride, int Id=0, bool bSys=true)
  {
    CSimpleLeafBuffer *lb = GetVertexContainer();
    byte *pData;
    if (bSys)
    {
      pData = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
    }
    else
    {
      pData = (byte *)lb->m_pVertexBuffer->m_VS[VSF_TANGENTS].m_VData;
    }
    Stride = sizeof(SPipTangents);
    return &pData[Id*Stride+12];
  }
  byte *GetTangentPtr(int& Stride, int Id=0, bool bSys=true)
  {
    CSimpleLeafBuffer *lb = GetVertexContainer();
    byte *pData;
    if (bSys)
    {
      pData = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
    }
    else
    {
      pData = (byte *)lb->m_pVertexBuffer->m_VS[VSF_TANGENTS].m_VData;
    }
    Stride = sizeof(SPipTangents);
    return &pData[Id*Stride];
  }
  byte *GetTNormalPtr(int& Stride, int Id=0, bool bSys=true)
  {
    CSimpleLeafBuffer *lb = GetVertexContainer();
    byte *pData;
    if (bSys)
    {
      pData = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
    }
    else
    {
      pData = (byte *)lb->m_pVertexBuffer->m_VS[VSF_TANGENTS].m_VData;
    }
    Stride = sizeof(SPipTangents);
    return &pData[Id*Stride+24];
  }

  byte *GetColorPtr(int & Stride, int Id=0, bool bSys=true)
  {
    CSimpleLeafBuffer *lb = GetVertexContainer();
    SBufInfoTable * pOffs;
    byte *pData;
    if (bSys)
    {
      pData = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
      pOffs = &gBufInfoTable[lb->m_pSecVertBuffer->m_vertexformat];
      Stride = m_VertexSize[lb->m_pSecVertBuffer->m_vertexformat];
    }
    else
    {
      pData = (byte *)lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
      pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
      Stride = m_VertexSize[lb->m_pVertexBuffer->m_vertexformat];
    }
    if (pOffs->OffsColor)
    {
      return &pData[Id*Stride+pOffs->OffsColor];
    }

    Stride = sizeof(UCol);
    return (byte*)&lb->m_TempColors[Id];
  }

  byte *GetUVPtr(int & Stride, int Id=0, bool bSys=true)
  {
    CSimpleLeafBuffer *lb = GetVertexContainer();
    SBufInfoTable * pOffs;
    byte *pData;
    if (bSys)
    {
      pData = (byte *)lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
      pOffs = &gBufInfoTable[lb->m_pSecVertBuffer->m_vertexformat];
      Stride = m_VertexSize[m_pSecVertBuffer->m_vertexformat];
    }
    else
    {
      pData = (byte *)lb->m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
      pOffs = &gBufInfoTable[lb->m_pVertexBuffer->m_vertexformat];
      Stride = m_VertexSize[m_pVertexBuffer->m_vertexformat];
    }
    if (pOffs->OffsTC)
    {
      return &pData[Id*Stride+pOffs->OffsTC];
    }

    Stride = sizeof(SMRendTexVert);
    return (byte*)&lb->m_TempTexCoords[Id];
  }	
};


class CSimpleStatObj
{
public:
  CSimpleStatObj(IRCLog * pLog, CIndexedMesh * pTriData, const char*pGeomName) 
  { 
    memset(this,0,sizeof(CSimpleStatObj));
    m_pLog = pLog; 
    m_pTriData = pTriData; 
    m_pGeomName = (char *)pGeomName;
    Physicalize();
    InitGeometry();
  }
  void Serialize(int & nPos, uchar * pSerBuf, bool bSave);
	bool IsPhysicsExist();

protected:
  int FindInPosBuffer(const Vec3d & opt, Vec3d * _vbuff, int _vcount, list2<int> * pHash);
  void CompactPosBuffer(Vec3d * _vbuff, int * _vcount, list2<int> * pindices);

  void Physicalize();
  void InitGeometry();
  IRCLog * m_pLog;
  CIndexedMesh * m_pTriData;
  char * m_pGeomName;

  // output
  list2<Vec3d> m_lstProxyVerts[3];
  list2<int>   m_lstProxyInds[3];
  Vec3d        m_vProxyBoxMin[3];
  Vec3d        m_vProxyBoxMax[3];
  list2<unsigned char> m_lstProxyFaceMaterials[3];

  Vec3d m_vBoxMin, m_vBoxMax;

  list2<struct StatHelperInfo> m_lstHelpers;	
  list2<CDLight> m_lstLSources;
};

#endif