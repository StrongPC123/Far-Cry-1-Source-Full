/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_CHAR_RENDER_ELEMENT_HDR_
#define _CRY_CHAR_RENDER_ELEMENT_HDR_


// buffer incarnation: the buffer with the last rendered frame
// NOTE: this structure doesn't manage the leaf buffer; it's just a container
// THe leaf buffer must be destructed manually through the methods of this class,
// the destructor won't do that for you
class CryCharRenderElement
{
public:
	// creates an empty element, without the leaf buffer attached
	CryCharRenderElement ();
	~CryCharRenderElement ();

	// returns true if the leaf buffer can be deleted immediately
	bool canDestruct();

	// detaches the leaf buffer from this object (forgets it)
	void detach();

	// deletes the current buffer, cleans up the object; can only be called when canDelete() == true
	void destruct ();

	// creates the buffer with the given number of vertices and vertex format
	// can only be called for a clean (without initialized leaf buffer) object
	//void create (int nVertCount, int nVertFormat, const char* szSource, unsigned numMaterials, bool bOnlyVideoBuffer = true);

	// creates the buffer with the given number of vertices and vertex format
	// can only be called for a clean (without initialized leaf buffer) object
	// initializes the system buffer with the data from the array
	void create (int nVertCount, const struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F* pSource, const char* szSource, unsigned numMaterials, bool bOnlyVideoBuffer = true);

	// recreates videobuffer
	void recreate();

	// returns the number of vertices allocated in the current buffer, if any is allocated
	unsigned numVertices()const;

	// returns the number of materials in the leaf buffer
	unsigned numMaterials()const;

	// sets the number of used materials in the leaf buffer, initializes the materials (creates ocleaves for them)
	void resizeMaterials (unsigned numMaterials, IShader* pShader);
	
	CLeafBuffer* getLeafBuffer() {return m_pLeafBuffer;}

	int getVertexFormat() {return m_pLeafBuffer->m_nVertexFormat;}

	void update(bool bLock, bool bCopyToVideoBuffer);

	void lock (bool bCopyToVideoBuffer) {update (true, bCopyToVideoBuffer);}
	void unlock (bool bCopyToVideoBuffer) {update (false, bCopyToVideoBuffer);}

	void render (CCObject* pObj);

	void assignMaterial (unsigned nMaterial, IShader* pShader, int nTextureId, int nFirstIndex, int numIndices, int nFirstVertex, int numVertices);
	void updateIndices (const unsigned short* pIndices, unsigned numIndices);
protected:

	// the leaf buffer for rendering the decals
	CLeafBuffer* m_pLeafBuffer;
	
	// this is the frame at which this leaf buffer was last rendered;
	// if it's the current frame, it cannot be deleted now.
	int m_nLeafBufferLastRenderFrame;
	
	// this is the number of vertices/indices allocated in the current leaf buffer
	// these are kept here becaue CLeafBuffer doesn't have any normal interface to retrieve them
	unsigned m_numVertBufVertices;
};


#endif