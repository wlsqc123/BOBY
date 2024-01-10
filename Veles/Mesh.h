//------------------------------------------------------- ----------------------
// File: Mesh.h
//-----------------------------------------------------------------------------

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct BoneWeightInfo
{
	UINT boneNum;
	float weights;
	std::string boneName;
};

struct VertexInfo
{
	Vector3 position = Vector3(0,0,0);
	Vector2 texcoord0 = Vector2(0, 0);
	Vector2 texcoord1 = Vector2(0, 0);
	Vector3 normal = Vector3(0, 0, 0);
	Vector3 tangent = Vector3(0, 0, 0);
	Vector3 bitangent = Vector3(0, 0, 0);
	Vector4 boneNum = { 0.0f,0.0f,0.0f,0.0f };
	Vector4 weights = { 0.0f,0.0f,0.0f,0.0f };
	UINT	MaterialNum = 0;
};

struct GeometryVertexInfo
{
	Vector3 position = Vector3(0, 0, 0);
	Vector2 size = Vector2(0, 0);
	UINT nWidth = 0;
	UINT nHeight = 0;
	float randHeight = 0.f;
	float randDir = 0.f;
};

struct MeshData
{
	std::vector<VertexInfo> vertices;
	std::vector<UINT> indices;
	std::string materialName = "NO Name";
};

class CMesh
{
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    virtual ~CMesh();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void ReleaseUploadBuffers();

	std::vector<MeshData>			meshes;
protected:
	ID3D12Resource					*m_pd3dVertexBuffer = NULL;
	ID3D12Resource					*m_pd3dVertexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		*m_d3dVertexBufferView;
	UINT							m_nVertexBufferView = 1;

	ID3D12Resource					*m_pd3dIndexBuffer = NULL;
	ID3D12Resource					*m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW			* m_d3dIndexBufferView;
	UINT							m_nIndexBufferView = 1;


	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nVertices = 0;
	UINT							m_nStride = sizeof(VertexInfo);
	UINT							m_nOffset = 0;
	UINT							m_nIndices = 0;
	BoundingOrientedBox				m_BoundingBox;
public:
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, UINT nInstances);
	void CalculateTriangleListVertexNormals(Vector3* pxmf3Normals, Vector3* pxmf3Positions, int nVertices);
	void CalculateTriangleListVertexNormals(Vector3* pxmf3Normals, Vector3* pxmf3Positions, UINT nVertices, UINT* pnIndices, UINT nIndices);
	void CalculateTriangleStripVertexNormals(Vector3* pxmf3Normals, Vector3* pxmf3Positions, UINT nVertices, UINT* pnIndices, UINT nIndices);
	void CalculateVertexNormals(Vector3* pxmf3Normals, Vector3* pxmf3Positions, int nVertices, UINT* pnIndices, int nIndices);
	void CalculateTriangleListTBNs(int nVertices, Vector3* pxmf3Positions, Vector2* pxmf2TexCoords, Vector3* pxmf3Tangents, Vector3* pxmf3BiTangents, Vector3* pxmf3Normals);
	BoundingOrientedBox GetBoundingBox() { return m_BoundingBox; }
};


class CCubeMeshTextured : public CMesh
{
public:
	CCubeMeshTextured(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshTextured();
};

class CCubeMeshNormalTextured : public CMesh
{
public:
	CCubeMeshNormalTextured(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshNormalTextured();
};


class CCubeMeshNormalMapTextured : public  CMesh
{
public:
	CCubeMeshNormalMapTextured(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshNormalMapTextured();

};

class CGeometryBillboardMesh : public CMesh
{
private:
	UINT						m_nStride = 0;
public:
	CGeometryBillboardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, vector<GeometryVertexInfo> geometryVertex);
	virtual ~CGeometryBillboardMesh();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
};

class CHeightMapImage
{
private:
	BYTE						*m_pHeightMapPixels;

	int							m_nWidth;
	int							m_nLength;
	Vector3					m_xmf3Scale;

public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, Vector3 xmf3Scale);
	~CHeightMapImage(void);

	float GetHeight(float x, float z, bool bReverseQuad = false);
	Vector3 GetHeightMapNormal(int x, int z);
	Vector3 GetScale() { return(m_xmf3Scale); }

	BYTE *GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

class CHeightMapGridMesh : public CMesh
{
protected:
	int							m_nWidth;
	int							m_nLength;
	Vector3					m_xmf3Scale;

public:
	CHeightMapGridMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, Vector3 xmf3Scale = Vector3(1.0f, 1.0f, 1.0f), void *pContext = NULL);
	virtual ~CHeightMapGridMesh();

	Vector3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }

	virtual float OnGetHeight(int x, int z, void *pContext);
	virtual Vector4 OnGetColor(int x, int z, void *pContext);
};

class CGridMesh : public CMesh
{
protected:
	int							m_nWidth;
	int							m_nLength;

public:
	CGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float width, float depth, int m, int n);
	virtual ~CGridMesh();
};

class CSkyBoxMesh : public CMesh
{
public:
	CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CSkyBoxMesh();
};

class CBoundingBoxMesh : public CMesh
{
public:
	CBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CBoundingBoxMesh();
};

class CFbxHierarchyMesh : public CMesh
{
public:
	CFbxHierarchyMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, aiMesh* mesh, const aiScene* scene , UINT materialnum, std::map<std::string, UINT>& boneDataMap);
	virtual ~CFbxHierarchyMesh() {};
	MeshData processMesh(aiMesh* mesh, const aiScene* scene, UINT materialnum, std::map<std::string, UINT>& boneDataMap);
};