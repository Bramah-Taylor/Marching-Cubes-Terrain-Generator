#include "EmptyMesh.h"


EmptyMesh::EmptyMesh(ID3D11Device* device, bool isIndexed, bool isVoxel)
{

	isIndexedMesh = isIndexed;
	isVoxelMesh = isVoxel;
	indexCount = 0;

	topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

}

EmptyMesh::~EmptyMesh()
{



}

void EmptyMesh::initBuffers(ID3D11DeviceContext* deviceContext, ID3D11Buffer* vBuffer, ID3D11Buffer* iBuffer)
{

	// Set the vertex buffer; we've already set up this buffer in another shader
	// So we just use a pointer to it instead and will call DrawAuto() in the light shader
	vertexBuffer = vBuffer;

	// Alternatively if the mesh was created by a compute shader, we can't call DrawAuto()
	// Instead we can call DrawIndexed() provided an index buffer
	if (isIndexedMesh)
	{

		indexBuffer = iBuffer;

	}

}

void EmptyMesh::sendData(ID3D11DeviceContext* deviceContext)
{

	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	if (isVoxelMesh)
	{

		// The voxel mesh is only outputting position values
		stride = sizeof(XMFLOAT4);

	}
	else
	{

		// The marching cubes shader outputs position and normals
		stride = sizeof(XMFLOAT4) + sizeof(XMFLOAT3);

	}

	offset = 0;

	// Set the vertex buffer
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetPrimitiveTopology(topology);

	// Set the index buffer, if we have one
	if (isIndexedMesh)
	{

		deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	}

}

int EmptyMesh::getIndexCount()
{

	return indexCount;

}

void EmptyMesh::setIndexCount(int count)
{

	indexCount = count;

}

void EmptyMesh::setTopology(D3D11_PRIMITIVE_TOPOLOGY topologyInput)
{

	topology = topologyInput;

}