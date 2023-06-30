// ImageObject allows the abstraction of different kinds of
// images generated during conversion

struct ImageObject
{
	//! write image to disk, overwrites any existing file
	virtual bool Save(const char *filename) = 0;
	
	//! converts the psurfSrc to the native format for this object and stores it as the right mip
	virtual HRESULT Convert(LPDIRECT3DSURFACE9 psurfSrc, int mip, int filter, D3DCUBEMAP_FACES facetype, LPDIRECT3DDEVICE9 pd3ddev) = 0;	

	//! creates a new XRGB texture of this image for images that can't be directly displayed (caller dealocates)
	virtual IDirect3DTexture9 *CopyToXRGB(LPDIRECT3DDEVICE9 pd3ddev) { return NULL; };
	
	//! gets the internal DX texture, if the object has one
  virtual LPDIRECT3DBASETEXTURE9 GetDXTex() { return NULL; };
  virtual void GetDXTexI(IDirect3DTexture9 *&pTexture) { pTexture = NULL; };
  
  //! deallocates all internally held texture objects
	virtual ~ImageObject() {};
};


// base DX Image, used for normal textures with mipmaps

struct DXImage : ImageObject
{
	LPDIRECT3DBASETEXTURE9 m_pTex;
	
	DXImage(LPDIRECT3DBASETEXTURE9 tex) : m_pTex(tex) {};
	~DXImage() { m_pTex->Release(); };

	bool Save(const char *filename)
	{
		return D3DXSaveTextureToFile(filename, D3DXIFF_DDS, m_pTex, NULL)==S_OK;
	};
	
	HRESULT Convert(LPDIRECT3DSURFACE9 psurfSrc, int mip, int filter, D3DCUBEMAP_FACES facetype, LPDIRECT3DDEVICE9 pd3ddev)
	{
		LPDIRECT3DSURFACE9 psurfDest = NULL;

		HRESULT hr = ((LPDIRECT3DTEXTURE9)m_pTex)->GetSurfaceLevel(mip, &psurfDest);

		if(hr==S_OK) hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL, psurfSrc, NULL, NULL, filter, 0);

		ReleasePpo(&psurfDest);
		return hr;
	};

  LPDIRECT3DBASETEXTURE9 GetDXTex() { return m_pTex; };
  virtual void GetDXTexI(IDirect3DTexture9 *&pTexture) { m_pTex->QueryInterface(IID_IDirect3DTexture9,(void **)&pTexture); };
};

// specialized for cubemaps

struct DXImageCube : DXImage
{
	DXImageCube(LPDIRECT3DCUBETEXTURE9 tex) : DXImage(tex) {};

	HRESULT Convert(LPDIRECT3DSURFACE9 psurfSrc, int mip, int filter, D3DCUBEMAP_FACES facetype, LPDIRECT3DDEVICE9 pd3ddev)
	{
		LPDIRECT3DSURFACE9 pcubeDest = NULL;
		HRESULT hr = ((LPDIRECT3DCUBETEXTURE9)m_pTex)->GetCubeMapSurface(facetype, mip, &pcubeDest);
		if(hr==S_OK) hr = D3DXLoadSurfaceFromSurface(pcubeDest, NULL, NULL, psurfSrc, NULL, NULL, filter, 0);
		ReleasePpo(&pcubeDest);
		return hr;
	};
};

// specialized for volumetric textures

struct DXImageVol : DXImage
{
	DXImageVol(LPDIRECT3DVOLUMETEXTURE9 tex) : DXImage(tex) {};

	HRESULT Convert(LPDIRECT3DSURFACE9 psurfSrc, int mip, int filter, D3DCUBEMAP_FACES facetype, LPDIRECT3DDEVICE9 pd3ddev)
	{
		LPDIRECT3DVOLUME9 pvolDest = NULL;
		HRESULT hr = ((LPDIRECT3DVOLUMETEXTURE9)m_pTex)->GetVolumeLevel(mip, &pvolDest);
		if(hr==S_OK) hr = D3DXLoadVolumeFromVolume(pvolDest, NULL, NULL, (LPDIRECT3DVOLUME9)psurfSrc, NULL, NULL, filter, 0);
		ReleasePpo(&pvolDest);
		return hr;
	};
};

// palettized 8 bit image, doesn't use DX internally at all

struct P8Image : ImageObject
{
	unsigned char *m_pPalette;
	unsigned char **m_ppIndices;
	int m_nWidth, m_nHeight, m_nMips;
	
	P8Image(int w, int h, int m) : m_pPalette(NULL), m_nWidth(w), m_nHeight(h), m_nMips(m)
	{
		m_ppIndices = new unsigned char *[m];
		for(int i = 0; i<m_nMips; i++) m_ppIndices[i] = NULL;
	};
	
	~P8Image()
	{
		for(int i = 0; i<m_nMips; i++) if(m_ppIndices[i]) free(m_ppIndices[i]);
		delete[] m_ppIndices;
	};
	
	bool Save(const char *filename);	
	void SavePaletizedTextureHeader(FILE *fh);
	void SavePaletizedTextureMip(FILE *fh, unsigned char *buf, int mip);
	
	HRESULT Convert(LPDIRECT3DSURFACE9 psurfSrc, int mip, int filter, D3DCUBEMAP_FACES facetype, LPDIRECT3DDEVICE9 pd3ddev);	

	IDirect3DTexture9 *CopyToXRGB(LPDIRECT3DDEVICE9 pd3ddev);
};
