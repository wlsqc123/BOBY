//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Mesh.h"

CMesh::CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

CMesh::~CMesh()
{
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
}

void CMesh::ReleaseUploadBuffers() 
{
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dVertexUploadBuffer = NULL;
	m_pd3dIndexUploadBuffer = NULL;
};

void CMesh::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	for (int i = 0; i < meshes.size(); i++)
	{
		pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView[i]);
		if (m_pd3dIndexBuffer)
		{
			pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView[i]);
			pd3dCommandList->DrawIndexedInstanced(static_cast<UINT>(meshes[i].indices.size()), 1, 0, 0, 0);
		}
		else
		{
			pd3dCommandList->DrawInstanced(static_cast<UINT>(meshes[i].vertices.size()), 1, m_nOffset, 0);
		}
	}
}

void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, UINT nInstances)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	for (int i = 0; i < meshes.size(); i++)
	{
		pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView[i]);
		if (m_pd3dIndexBuffer)
		{
			pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView[i]);
			pd3dCommandList->DrawIndexedInstanced(static_cast<UINT>(meshes[i].indices.size()), nInstances, 0, 0, 0);
		}
		else
		{
			pd3dCommandList->DrawInstanced(static_cast<UINT>(meshes[i].vertices.size()), nInstances, m_nOffset, 0);
		}
	}
}

void CMesh::CalculateTriangleListVertexNormals(Vector3* pxmf3Normals, Vector3* pxmf3Positions, int nVertices)
{
	int nPrimitives = nVertices / 3;
	UINT nIndex0, nIndex1, nIndex2;
	for (int i = 0; i < nPrimitives; i++)
	{
		nIndex0 = i * 3 + 0;
		nIndex1 = i * 3 + 1;
		nIndex2 = i * 3 + 2;
		Vector3 xmf3Edge01 = pxmf3Positions[nIndex1] - pxmf3Positions[nIndex0];
		Vector3 xmf3Edge02 = pxmf3Positions[nIndex2] - pxmf3Positions[nIndex0];
		pxmf3Normals[nIndex0] = pxmf3Normals[nIndex1] = pxmf3Normals[nIndex2] = Vector3::CrossNormal(xmf3Edge01, xmf3Edge02);
	}
}

void CMesh::CalculateTriangleListVertexNormals(Vector3* pxmf3Normals, Vector3* pxmf3Positions, UINT nVertices, UINT* pnIndices, UINT nIndices)
{
	UINT nPrimitives = (pnIndices) ? (nIndices / 3) : (nVertices / 3);
	Vector3 xmf3SumOfNormal, xmf3Edge01, xmf3Edge02, xmf3Normal;
	UINT nIndex0, nIndex1, nIndex2;
	for (UINT j = 0; j < nVertices; j++)
	{
		xmf3SumOfNormal = Vector3(0.0f, 0.0f, 0.0f);
		for (UINT i = 0; i < nPrimitives; i++)
		{
			nIndex0 = pnIndices[i * 3 + 0];
			nIndex1 = pnIndices[i * 3 + 1];
			nIndex2 = pnIndices[i * 3 + 2];
			if (pnIndices && ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j)))
			{
				xmf3Edge01 = pxmf3Positions[nIndex1] - pxmf3Positions[nIndex0];
				xmf3Edge02 = pxmf3Positions[nIndex2] - pxmf3Positions[nIndex0];
				xmf3Normal = Vector3::CrossNormal(xmf3Edge01, xmf3Edge02);
				xmf3SumOfNormal = xmf3SumOfNormal + xmf3Normal;
			}
		}
		pxmf3Normals[j] = xmf3SumOfNormal.normalized();
	}
}

void CMesh::CalculateTriangleStripVertexNormals(Vector3* pxmf3Normals, Vector3* pxmf3Positions, UINT nVertices, UINT* pnIndices, UINT nIndices)
{
	UINT nPrimitives = (pnIndices) ? (nIndices - 2) : (nVertices - 2);
	Vector3 xmf3SumOfNormal(0.0f, 0.0f, 0.0f);
	UINT nIndex0, nIndex1, nIndex2;
	for (UINT j = 0; j < nVertices; j++)
	{
		xmf3SumOfNormal = Vector3(0.0f, 0.0f, 0.0f);
		for (UINT i = 0; i < nPrimitives; i++)
		{
			nIndex0 = ((i % 2) == 0) ? (i + 0) : (i + 1);
			if (pnIndices) nIndex0 = pnIndices[nIndex0];
			nIndex1 = ((i % 2) == 0) ? (i + 1) : (i + 0);
			if (pnIndices) nIndex1 = pnIndices[nIndex1];
			nIndex2 = (pnIndices) ? pnIndices[i + 2] : (i + 2);
			if ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j))
			{
				Vector3 xmf3Edge01 = pxmf3Positions[nIndex1] - pxmf3Positions[nIndex0];
				Vector3 xmf3Edge02 = pxmf3Positions[nIndex2] - pxmf3Positions[nIndex0];
				Vector3 xmf3Normal = Vector3::CrossNormal(xmf3Edge01, xmf3Edge02);
				xmf3SumOfNormal = xmf3SumOfNormal + xmf3Normal;
			}
		}
		pxmf3Normals[j] = xmf3SumOfNormal.normalized();
	}
}

void CMesh::CalculateVertexNormals(Vector3* pxmf3Normals, Vector3* pxmf3Positions, int nVertices, UINT* pnIndices, int nIndices)
{
	switch (m_d3dPrimitiveTopology)
	{
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		if (pnIndices)
			CalculateTriangleListVertexNormals(pxmf3Normals, pxmf3Positions, nVertices, pnIndices, nIndices);
		else
			CalculateTriangleListVertexNormals(pxmf3Normals, pxmf3Positions, nVertices);
		break;
	case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		CalculateTriangleStripVertexNormals(pxmf3Normals, pxmf3Positions, nVertices, pnIndices, nIndices);
		break;
	default:
		break;
	}
}

void CMesh::CalculateTriangleListTBNs(int nVertices, Vector3* pxmf3Positions, Vector2* pxmf2TexCoords, Vector3* pxmf3Tangents, Vector3* pxmf3BiTangents, Vector3* pxmf3Normals)
{
	float fu10, fv10, fu20, fv20, fx10, fx20, fy10, fy20, fz10, fz20;
	for (int i = 0; i < nVertices; i += 3)
	{
		fu10 = pxmf2TexCoords[i + 1].x - pxmf2TexCoords[i].x;
		fv10 = pxmf2TexCoords[i + 1].y - pxmf2TexCoords[i].y;
		fu20 = pxmf2TexCoords[i + 2].x - pxmf2TexCoords[i].x;
		fv20 = pxmf2TexCoords[i + 2].y - pxmf2TexCoords[i].y;
		fx10 = pxmf3Positions[i + 1].x - pxmf3Positions[i].x;
		fy10 = pxmf3Positions[i + 1].y - pxmf3Positions[i].y;
		fz10 = pxmf3Positions[i + 1].z - pxmf3Positions[i].z;
		fx20 = pxmf3Positions[i + 2].x - pxmf3Positions[i].x;
		fy20 = pxmf3Positions[i + 2].y - pxmf3Positions[i].y;
		fz20 = pxmf3Positions[i + 2].z - pxmf3Positions[i].z;
		float fInvDeterminant = 1.0f / (fu10 * fv20 - fu20 * fv10);
		pxmf3Tangents[i] = pxmf3Tangents[i + 1] = pxmf3Tangents[i + 2] = Vector3(fInvDeterminant * (fv20 * fx10 - fv10 * fx20), fInvDeterminant * (fv20 * fy10 - fv10 * fy20), fInvDeterminant * (fv20 * fz10 - fv10 * fz20));
		pxmf3BiTangents[i] = pxmf3BiTangents[i + 1] = pxmf3BiTangents[i + 2] = Vector3(fInvDeterminant * (-fu20 * fx10 + fu10 * fx20), fInvDeterminant * (-fu20 * fy10 + fu10 * fy20), fInvDeterminant * (-fu20 * fz10 + fu10 * fz20));
		pxmf3Normals[i] = Vector3::CrossNormal(pxmf3Tangents[i], pxmf3BiTangents[i]);
	}
}
//////////////////////////////////////////////////////////////////////////////////
//
CCubeMeshTextured::CCubeMeshTextured(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;
	MeshData meshdata;
	VertexInfo vi;
	meshdata.vertices.reserve(36);
	
	vi.position = Vector3(-fx, +fy, -fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, -fz); vi.texcoord0 = Vector2(1.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, -fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, -fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, -fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, -fz); vi.texcoord0 = Vector2(0.0f, 1.0f); meshdata.vertices.push_back(vi);
														 
	vi.position = Vector3(-fx, +fy, +fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, +fz); vi.texcoord0 = Vector2(1.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, -fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, +fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, -fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, -fz); vi.texcoord0 = Vector2(0.0f, 1.0f); meshdata.vertices.push_back(vi);

	vi.position = Vector3(-fx, -fy, +fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, +fz); vi.texcoord0 = Vector2(1.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, +fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, +fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, +fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, +fz); vi.texcoord0 = Vector2(0.0f, 1.0f); meshdata.vertices.push_back(vi);

	vi.position = Vector3(-fx, -fy, -fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, -fz); vi.texcoord0 = Vector2(1.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, +fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, -fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, +fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, +fz); vi.texcoord0 = Vector2(0.0f, 1.0f); meshdata.vertices.push_back(vi);

	vi.position = Vector3(-fx, +fy, +fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, -fz); vi.texcoord0 = Vector2(1.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, -fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, +fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, -fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, +fz); vi.texcoord0 = Vector2(0.0f, 1.0f); meshdata.vertices.push_back(vi);

	vi.position = Vector3(+fx, +fy, -fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, +fz); vi.texcoord0 = Vector2(1.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, +fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, -fz); vi.texcoord0 = Vector2(0.0f, 0.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, +fz); vi.texcoord0 = Vector2(1.0f, 1.0f); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, -fz); vi.texcoord0 = Vector2(0.0f, 1.0f); meshdata.vertices.push_back(vi);

	meshes.push_back(meshdata);
	m_nVertices = static_cast<UINT>(meshes[0].vertices.size());

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList,meshes[0].vertices.data(), m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferView];
	m_d3dVertexBufferView[0].BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView[0].StrideInBytes = m_nStride;
	m_d3dVertexBufferView[0].SizeInBytes = m_nStride * m_nVertices;
}

CCubeMeshTextured::~CCubeMeshTextured()
{
}

CCubeMeshNormalTextured::CCubeMeshNormalTextured(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) :CMesh(pd3dDevice, pd3dCommandList)
{
	MeshData meshdata;
	VertexInfo vi;
	meshdata.vertices.reserve(36);

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	Vector3 pxmf3Positions[36];
	int i = 0;
	pxmf3Positions[i++] = Vector3(-fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, -fz);

	pxmf3Positions[i++] = Vector3(-fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, -fz);
	pxmf3Positions[i++] = Vector3(-fx, -fy, -fz);

	pxmf3Positions[i++] = Vector3(-fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, -fz);

	pxmf3Positions[i++] = Vector3(-fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(-fx, +fy, -fz);

	pxmf3Positions[i++] = Vector3(-fx, -fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, +fz);

	pxmf3Positions[i++] = Vector3(-fx, -fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(-fx, +fy, +fz);

	pxmf3Positions[i++] = Vector3(-fx, -fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, +fz);

	pxmf3Positions[i++] = Vector3(-fx, -fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, +fz);
	pxmf3Positions[i++] = Vector3(-fx, -fy, +fz);

	pxmf3Positions[i++] = Vector3(-fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(-fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(-fx, -fy, -fz);

	pxmf3Positions[i++] = Vector3(-fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(-fx, -fy, -fz);
	pxmf3Positions[i++] = Vector3(-fx, -fy, +fz);

	pxmf3Positions[i++] = Vector3(+fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, +fz);

	pxmf3Positions[i++] = Vector3(+fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, -fz);

	Vector2 pxmf2TexCoords[36];
	i = 0;
	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	Vector3 pxmf3Normals[36];
	CalculateVertexNormals(pxmf3Normals, pxmf3Positions, 36, NULL, 0); //포지션가지고 노말계산
	for (int i = 0; i < 36; i++)
	{
		vi.position = pxmf3Positions[i]; vi.normal = pxmf3Normals[i]; vi.texcoord0 = pxmf2TexCoords[i];
		meshdata.vertices.push_back(vi);
	}
	meshes.push_back(meshdata);
	m_nVertices = static_cast<UINT>(meshes[0].vertices.size());
	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, meshes[0].vertices.data(), m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferView];
	m_d3dVertexBufferView[0].BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView[0].StrideInBytes = m_nStride;
	m_d3dVertexBufferView[0].SizeInBytes = m_nStride * m_nVertices;
}

CCubeMeshNormalTextured::~CCubeMeshNormalTextured()
{
}


CCubeMeshNormalMapTextured::CCubeMeshNormalMapTextured(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth):CMesh(pd3dDevice, pd3dCommandList)
{
	MeshData meshdata;
	VertexInfo vi;
	meshdata.vertices.reserve(36);

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	m_BoundingBox = BoundingOrientedBox(XMFLOAT3(0, 0, 0), XMFLOAT3(fWidth / 2, fHeight / 2, fDepth / 2), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	Vector3 pxmf3Positions[36];
	int i = 0;
	pxmf3Positions[i++] = Vector3(-fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, -fz);

	pxmf3Positions[i++] = Vector3(-fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, -fz);
	pxmf3Positions[i++] = Vector3(-fx, -fy, -fz);

	pxmf3Positions[i++] = Vector3(-fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, -fz);

	pxmf3Positions[i++] = Vector3(-fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(-fx, +fy, -fz);

	pxmf3Positions[i++] = Vector3(-fx, -fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, +fz);

	pxmf3Positions[i++] = Vector3(-fx, -fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(-fx, +fy, +fz);

	pxmf3Positions[i++] = Vector3(-fx, -fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, +fz);

	pxmf3Positions[i++] = Vector3(-fx, -fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, +fz);
	pxmf3Positions[i++] = Vector3(-fx, -fy, +fz);

	pxmf3Positions[i++] = Vector3(-fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(-fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(-fx, -fy, -fz);

	pxmf3Positions[i++] = Vector3(-fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(-fx, -fy, -fz);
	pxmf3Positions[i++] = Vector3(-fx, -fy, +fz);

	pxmf3Positions[i++] = Vector3(+fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, +fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, +fz);

	pxmf3Positions[i++] = Vector3(+fx, +fy, -fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, +fz);
	pxmf3Positions[i++] = Vector3(+fx, -fy, -fz);

	Vector2 pxmf2TexCoords[36];
	i = 0;
	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);

	pxmf2TexCoords[i++] = Vector2(0.0f, 0.0f);
	pxmf2TexCoords[i++] = Vector2(1.0f, 1.0f);
	pxmf2TexCoords[i++] = Vector2(0.0f, 1.0f);

	Vector3 pxmf3Normals[36], pxmf3Tangents[36], pxmf3BiTangents[36];
	CalculateTriangleListTBNs(36, pxmf3Positions, pxmf2TexCoords, pxmf3Tangents, pxmf3BiTangents, pxmf3Normals);
	//CalculateVertexNormals(pxmf3Normals, pxmf3Positions, 36, NULL, 0);
	for (int i = 0; i < 36; i++)
	{
		vi.position = pxmf3Positions[i]; vi.normal = pxmf3Normals[i]; vi.texcoord0 = pxmf2TexCoords[i]; vi.tangent = pxmf3Tangents[i]; vi.bitangent = pxmf3BiTangents[i];
		meshdata.vertices.push_back(vi);
	}

	meshes.push_back(meshdata);
	m_nVertices = static_cast<UINT>(meshes[0].vertices.size());
	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, meshes[0].vertices.data(), m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferView];
	m_d3dVertexBufferView[0].BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView[0].StrideInBytes = m_nStride;
	m_d3dVertexBufferView[0].SizeInBytes = m_nStride * m_nVertices;
}

CCubeMeshNormalMapTextured::~CCubeMeshNormalMapTextured()
{
}



CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, Vector3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE *pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	//m_nWidth => 1, m_nLength => 1 로 바꿈(10/12) 바닥 높낮이 삭제
	//11-4 하이트맵이 아닌 텍스처 바닥으롬나 구현 
	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pHeightMapPixels[x + (y* m_nWidth)];
		}
	}
	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

Vector3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(Vector3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	Vector3 xmf3Edge1 = Vector3(0.0f, y3 - y1, m_xmf3Scale.z);
	Vector3 xmf3Edge2 = Vector3(m_xmf3Scale.x, y2 - y1, 0.0f);
	Vector3 xmf3Normal = Vector3::CrossNormal(xmf3Edge1, xmf3Edge2);

	return(xmf3Normal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z*m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z*m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1)*m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1)*m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, Vector3 xmf3Scale, void *pContext) : CMesh(pd3dDevice, pd3dCommandList)
{
	MeshData meshdata;
	VertexInfo vi;
	m_nVertices = nWidth * nLength;
	meshdata.vertices.reserve(m_nVertices);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	int cxHeightMap = pHeightMapImage->GetHeightMapWidth();
	int czHeightMap = pHeightMapImage->GetHeightMapLength();

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			fHeight = OnGetHeight(x, z, pContext);
			//fHeight =410;
			vi.position = Vector3((x*m_xmf3Scale.x), fHeight, (z*m_xmf3Scale.z));
			vi.normal = pHeightMapImage->GetHeightMapNormal(x, z);
			vi.texcoord0 = Vector2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			vi.texcoord1 = Vector2(float(x) / float(m_xmf3Scale.x), float(z) / float(m_xmf3Scale.z));
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
			meshdata.vertices.push_back(vi);
		}
	}

	m_nIndices = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
	meshdata.indices.reserve(m_nIndices);

	for (int z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) 
				{
					meshdata.indices.push_back((UINT)(x + (z * nWidth)));
				}
				meshdata.indices.push_back((UINT)(x + (z * nWidth)));
				meshdata.indices.push_back((UINT)((x + (z * nWidth)) + nWidth));
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1))
				{ 
					meshdata.indices.push_back((UINT)(x + (z * nWidth)));
				}
				meshdata.indices.push_back((UINT)(x + (z * nWidth)));
				meshdata.indices.push_back((UINT)((x + (z * nWidth)) + nWidth));
			}
		}
	}

	meshes.push_back(meshdata);
	m_nVertices = static_cast<UINT>(meshes[0].vertices.size());
	m_nIndices = static_cast<UINT>(meshes[0].indices.size());
	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, meshes[0].vertices.data(), m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW[meshes.size()];
	m_d3dIndexBufferView = new D3D12_INDEX_BUFFER_VIEW[meshes.size()];

	m_d3dVertexBufferView[0].BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView[0].StrideInBytes = m_nStride;
	m_d3dVertexBufferView[0].SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, meshes[0].indices.data(), sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView[0].BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView[0].Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView[0].SizeInBytes = sizeof(UINT) * m_nIndices;
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void *pContext)
{
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	BYTE *pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	Vector3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z*nWidth)] * xmf3Scale.y;
	return(fHeight);
}

Vector4 CHeightMapGridMesh::OnGetColor(int x, int z, void *pContext)
{
	Vector3 xmf3LightDirection = Vector3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = xmf3LightDirection.normalized();
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	Vector3 xmf3Scale = pHeightMapImage->GetScale();
	Vector4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);
	float fScale = Vector3::Dot(pHeightMapImage->GetHeightMapNormal(x, z), xmf3LightDirection);
	fScale += Vector3::Dot(pHeightMapImage->GetHeightMapNormal(x + 1, z), xmf3LightDirection);
	fScale += Vector3::Dot(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1), xmf3LightDirection);
	fScale += Vector3::Dot(pHeightMapImage->GetHeightMapNormal(x, z + 1), xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	Vector4 xmf4Color = xmf4IncidentLightColor * fScale;
	return(xmf4Color);
}

CSkyBoxMesh::CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) :CMesh(pd3dDevice, pd3dCommandList)
{
	MeshData meshdata;
	VertexInfo vi;
	meshdata.vertices.reserve(36);

	Vector3 *xmf3Positions = new Vector3[m_nVertices];

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	// Front Quad (quads point inward)
	vi.position = Vector3(-fx, +fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fx, +fx); meshdata.vertices.push_back(vi);
	// Back Quad						 			
	vi.position = Vector3(+fx, +fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fx, -fx); meshdata.vertices.push_back(vi);
	// Left Quad						 			
	vi.position = Vector3(-fx, +fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fx, +fx); meshdata.vertices.push_back(vi);
	// Right Quad						 			
	vi.position = Vector3(+fx, +fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fx, -fx); meshdata.vertices.push_back(vi);
	// Top Quad							 			
	vi.position = Vector3(-fx, +fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fx, +fx); meshdata.vertices.push_back(vi);
	// Bottom Quad						 			
	vi.position = Vector3(-fx, -fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fx, -fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fx, +fx); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fx, -fx); meshdata.vertices.push_back(vi);

	meshes.push_back(meshdata);
	m_nVertices = static_cast<UINT>(meshes[0].vertices.size());

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, meshes[0].vertices.data(), m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferView];
	m_d3dVertexBufferView[0].BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView[0].StrideInBytes = m_nStride;
	m_d3dVertexBufferView[0].SizeInBytes = m_nStride * m_nVertices;
}

CSkyBoxMesh::~CSkyBoxMesh()
{
}

CFbxHierarchyMesh::CFbxHierarchyMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, aiMesh* mesh, const aiScene* scene, UINT materialnum, std::map<std::string, UINT>& boneDataMap):CMesh(pd3dDevice, pd3dCommandList)
{
	m_nStride = sizeof(VertexInfo);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	meshes.emplace_back(processMesh(mesh, scene, materialnum, boneDataMap));
	m_d3dVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW[meshes.size()];
	m_d3dIndexBufferView = new D3D12_INDEX_BUFFER_VIEW[meshes.size()];
	for (int i = 0; i < meshes.size(); i++)
	{
		m_nIndices = static_cast<UINT>(meshes[i].indices.size());
		m_nVertices = static_cast<UINT>(meshes[i].vertices.size());
		m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, meshes[i].vertices.data(), m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
		m_d3dVertexBufferView[i].BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
		m_d3dVertexBufferView[i].StrideInBytes = m_nStride;
		m_d3dVertexBufferView[i].SizeInBytes = m_nStride * m_nVertices;

		m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, meshes[i].indices.data(), sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
		m_d3dIndexBufferView[i].BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
		m_d3dIndexBufferView[i].Format = DXGI_FORMAT_R32_UINT;
		m_d3dIndexBufferView[i].SizeInBytes = sizeof(UINT) * m_nIndices;
	}

}

MeshData CFbxHierarchyMesh::processMesh(aiMesh* mesh, const aiScene* scene, UINT materialnum, std::map<std::string, UINT>& boneDataMap)
{
	MeshData meshdata;
	std::string s = mesh->mName.C_Str();
	s += " ";
	//fbx에서 인덱스정보 저장
	for (UINT i = 0; i < mesh->mNumFaces; ++i)
	{
		for (UINT j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
		{
			meshdata.indices.emplace_back(mesh->mFaces[i].mIndices[j]);
		}
	}
	meshdata.vertices.reserve(mesh->mNumVertices);
	for (UINT i = 0; i < mesh->mNumVertices; ++i)
	{
		VertexInfo vi;
		vi.position = { mesh->mVertices[i].x,mesh->mVertices[i].y, mesh->mVertices[i].z };
		vi.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
		vi.texcoord0 = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		vi.tangent = { mesh->mTangents[i].x,mesh->mTangents[i].y,mesh->mTangents[i].z };
		vi.bitangent = { mesh->mBitangents[i].x,mesh->mBitangents[i].y,mesh->mBitangents[i].z };
		vi.MaterialNum = materialnum;
		meshdata.vertices.push_back(vi);
	}
	if (mesh->HasBones()) {
		std::string searchMeshName = mesh->mName.C_Str();
		if (searchMeshName.find("mostruo2") == string::npos)
		{
			for (UINT j = 0; j < mesh->mNumBones; j++)
			{
				BoneWeightInfo boneinfo;
				aiBone* aibone = mesh->mBones[j];
				boneinfo.boneNum = j;
				boneinfo.weights = 0.0f;
				boneinfo.boneName = aibone->mName.C_Str();
				aibone->mOffsetMatrix;
				for (UINT k = 0; k < aibone->mNumWeights; k++)
				{
					if (aibone->mWeights[k].mWeight >= 0.001f)
					{
						UINT temp = aibone->mWeights[k].mVertexId;
						if (meshdata.vertices[temp].weights.x == 0.0f)
						{
							meshdata.vertices[temp].boneNum.x = boneDataMap[aibone->mName.C_Str()];
							meshdata.vertices[temp].weights.x = aibone->mWeights[k].mWeight;
						}
						else if (meshdata.vertices[temp].weights.y == 0.0f)
						{
							meshdata.vertices[temp].boneNum.y = boneDataMap[aibone->mName.C_Str()];
							meshdata.vertices[temp].weights.y = aibone->mWeights[k].mWeight;
						}
						else if (meshdata.vertices[temp].weights.z == 0.0f)
						{
							meshdata.vertices[temp].boneNum.z = boneDataMap[aibone->mName.C_Str()];
							meshdata.vertices[temp].weights.z = aibone->mWeights[k].mWeight;
						}
						else if (meshdata.vertices[aibone->mWeights[k].mVertexId].weights.w == 0.0f)
						{
							meshdata.vertices[temp].boneNum.w = boneDataMap[aibone->mName.C_Str()];
							meshdata.vertices[temp].weights.w = aibone->mWeights[k].mWeight;				
						}

						else if (meshdata.vertices[temp].weights.x < aibone->mWeights[k].mWeight&& meshdata.vertices[temp].weights.x < meshdata.vertices[temp].weights.y
							&& meshdata.vertices[temp].weights.x < meshdata.vertices[temp].weights.z && meshdata.vertices[temp].weights.x < meshdata.vertices[temp].weights.w)
						{
							meshdata.vertices[temp].boneNum.x = boneDataMap[aibone->mName.C_Str()];
							meshdata.vertices[temp].weights.x = aibone->mWeights[k].mWeight;
						}
						else if (meshdata.vertices[temp].weights.y < aibone->mWeights[k].mWeight && meshdata.vertices[temp].weights.y < meshdata.vertices[temp].weights.x
							&& meshdata.vertices[temp].weights.y < meshdata.vertices[temp].weights.z && meshdata.vertices[temp].weights.y < meshdata.vertices[temp].weights.w)
						{
							meshdata.vertices[temp].boneNum.y = boneDataMap[aibone->mName.C_Str()];
							meshdata.vertices[temp].weights.y = aibone->mWeights[k].mWeight;
						}
						else if (meshdata.vertices[temp].weights.z < aibone->mWeights[k].mWeight && meshdata.vertices[temp].weights.z < meshdata.vertices[temp].weights.x
							&& meshdata.vertices[temp].weights.z < meshdata.vertices[temp].weights.y && meshdata.vertices[temp].weights.z < meshdata.vertices[temp].weights.w)
						{
							meshdata.vertices[temp].boneNum.z = boneDataMap[aibone->mName.C_Str()];
							meshdata.vertices[temp].weights.z = aibone->mWeights[k].mWeight;
						}
						else if (meshdata.vertices[temp].weights.w < aibone->mWeights[k].mWeight && meshdata.vertices[temp].weights.w < meshdata.vertices[temp].weights.x
							&& meshdata.vertices[temp].weights.w < meshdata.vertices[temp].weights.y && meshdata.vertices[temp].weights.w < meshdata.vertices[temp].weights.z)
						{
							meshdata.vertices[temp].boneNum.w = boneDataMap[aibone->mName.C_Str()];
							meshdata.vertices[temp].weights.w = aibone->mWeights[k].mWeight;
						}
					}
				}
			}
		}
	}

	return meshdata;
}

CBoundingBoxMesh::CBoundingBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) :CMesh(pd3dDevice, pd3dCommandList)
{
	MeshData meshdata;
	VertexInfo vi;
	meshdata.vertices.reserve(36);

	Vector3* xmf3Positions = new Vector3[m_nVertices];

	float fx = fWidth , fy = fHeight, fz = fDepth ;
	// Front Quad (quads point inward)
	vi.position = Vector3(-fx, +fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	// Back Quad						 	n ormal			3
	vi.position = Vector3(-fx, +fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	// Left Quad						 	n ormal			3
	vi.position = Vector3(-fx, -fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	// Right Quad						 	n ormal			3
	vi.position = Vector3(-fx, -fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	// Top Quad							 vi.cnormal= Vector431, 0, 0,;			
	vi.position = Vector3(-fx, +fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, +fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(-fx, -fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	// Bottom Quad						 vi.cnormal= Vector431, 0, 0,;			
	vi.position = Vector3(+fx, +fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, +fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, +fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);
	vi.position = Vector3(+fx, -fy, -fz); vi.normal = Vector3(1, 0, 0); meshdata.vertices.push_back(vi);

	meshes.push_back(meshdata);
	m_nVertices = static_cast<UINT>(meshes[0].vertices.size());

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, meshes[0].vertices.data(), m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferView];
	m_d3dVertexBufferView[0].BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView[0].StrideInBytes = m_nStride;
	m_d3dVertexBufferView[0].SizeInBytes = m_nStride * m_nVertices;
}

CBoundingBoxMesh::~CBoundingBoxMesh()
{
}


CGeometryBillboardMesh::CGeometryBillboardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, vector<GeometryVertexInfo> geometryVertex) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = geometryVertex.size();
	m_nStride = sizeof(GeometryVertexInfo);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, geometryVertex.data(), m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW[m_nVertexBufferView];
	m_d3dVertexBufferView[0].BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView[0].StrideInBytes = m_nStride;
	m_d3dVertexBufferView[0].SizeInBytes = m_nStride * m_nVertices;
}

CGeometryBillboardMesh::~CGeometryBillboardMesh()
{
}

void CGeometryBillboardMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView[0]);


	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);

}

CGridMesh::CGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float width, float depth, int m, int n) : CMesh(pd3dDevice, pd3dCommandList)
{
	MeshData meshdata;
	VertexInfo vi;
	m_nVertices = m * n;
	m_nIndices = (m - 1) * (n - 1) * 2;
	meshdata.vertices.reserve(m_nVertices);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			vi.position = Vector3(x, 0.f, z);
			vi.normal = Vector3(0.f, 1.f, 0.f);
			vi.tangent = Vector3(1.f, 0.f, 0.f);
			vi.texcoord0 = Vector2(j * du, i * dv);
			meshdata.vertices.push_back(vi);
		}
	}
	meshdata.indices.reserve(m_nIndices);

	for (UINT i = 0; i < m - 1; ++i)
	{
		for (UINT j = 0; j < n - 1; ++j)
		{
			meshdata.indices.push_back(i * n + j);
			meshdata.indices.push_back(i * n + j + 1);
			meshdata.indices.push_back((i + 1) * n + j);

			meshdata.indices.push_back((i + 1) * n + j);
			meshdata.indices.push_back(i * n + j + 1);
			meshdata.indices.push_back((i + 1) * n + j + 1);
		}
	}

	meshes.push_back(meshdata);

	m_nVertices = static_cast<UINT>(meshes[0].vertices.size());
	m_nIndices = static_cast<UINT>(meshes[0].indices.size());
	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, meshes[0].vertices.data(), m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView = new D3D12_VERTEX_BUFFER_VIEW[meshes.size()];
	m_d3dIndexBufferView = new D3D12_INDEX_BUFFER_VIEW[meshes.size()];

	m_d3dVertexBufferView[0].BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView[0].StrideInBytes = m_nStride;
	m_d3dVertexBufferView[0].SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, meshes[0].indices.data(), sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView[0].BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView[0].Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView[0].SizeInBytes = sizeof(UINT) * m_nIndices;
}

CGridMesh::~CGridMesh()
{
}
