// Empty mesh
// A mesh with no defined vertex buffer or index buffer by default
// Instead, this mesh holds pointers to buffers created by shaders and serves as an interface for rendering
#ifndef _EMPTY_MESH_H_
#define _EMPTY_MESH_H_

#include "../DXFramework/BaseMesh.h"

class EmptyMesh
{

public:

	EmptyMesh(ID3D11Device* device, bool isIndexed, bool isVoxel);
	~EmptyMesh();

	void initBuffers(ID3D11DeviceContext* deviceContext, ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer = NULL);
	void sendData(ID3D11DeviceContext* deviceContext);
	// Will return 0 by default - make sure to set an index count
	int getIndexCount();

	void setTopology(D3D11_PRIMITIVE_TOPOLOGY topologyInput);
	void setIndexCount(int count);

protected:

	// Pointer to a vertex buffer created by another source (e.g. a compute shader or geometry shader with stream output)
	ID3D11Buffer* vertexBuffer;
	// Ditto for index buffer
	ID3D11Buffer* indexBuffer;

	D3D11_PRIMITIVE_TOPOLOGY topology;

	// Checks to make sure the input assembler stage gets the right info sent to it
	bool isIndexedMesh;
	bool isVoxelMesh;
	int indexCount;

};

#endif // !_EMPTY_MESH_H_