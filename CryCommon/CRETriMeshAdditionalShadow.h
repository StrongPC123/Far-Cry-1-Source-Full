/*
#ifndef __CRETRIMESHADDITIONALSHADOW_H__
#define __CRETRIMESHADDITIONALSHADOW_H__


class CShadowVolEdge;

//////////////////////////////////////////////////////////////////////
class CRETriMeshAdditionalShadow : public CRendElement
{

public:

  CRETriMeshAdditionalShadow()
  {
    mfSetType(eDATA_TriMeshAdditionalShadow);
    mfUpdateFlags(FCEF_TRANSFORM | FCEF_NODEL);
		m_vOrigin(0,0,0);
		m_nNumEdges=m_nNumFaces=0;
		m_pShadowVolEdgesList=NULL;
		m_pFacesList=NULL;		
		m_pTexture=NULL;
  }

  virtual ~CRETriMeshAdditionalShadow()
  {
  }

  virtual void mfPrepare();
  virtual bool mfDraw(SShader *ef, SShaderPass *sfm);

	Vec3d m_vOrigin;

	int	m_nNumEdges;
	CShadowVolEdge *m_pShadowVolEdgesList;

	int m_nNumFaces;
	CObjFace	*m_pFacesList;
	ITexPic		*m_pTexture;

private:
	
  friend class CRender3D;	
};

#endif  // __CRETRIMESHADDITIONALSHADOW_H__
*/