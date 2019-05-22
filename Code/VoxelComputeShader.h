// Voxel compute shader
// Creates an indexed 3D point list of vertices to be used in conjunction with geometry shaders
#ifndef _VOXEL_COMPUTE_SHADER_H_
#define _VOXEL_COMPUTE_SHADER_H_

#include "BaseComputeShader.h"

class VoxelComputeShader: public BaseComputeShader
{

private:

	struct ParamBuffer
	{

		XMFLOAT3 dimensions;
		float padding;

	};

public:

	VoxelComputeShader(ID3D11Device* device, HWND hwnd);
	~VoxelComputeShader();

	void initShader(WCHAR* csFilename, int elements);
	void Run(ID3D11DeviceContext* deviceContext);

	// Update the mesh values when the mesh size is changed
	void UpdateMeshValues(int x, int y, int z);

	// Returns the output buffers - to be used as vertex and index buffers in an EmptyMesh
	ID3D11Buffer* getVertexBuffer();
	ID3D11Buffer* getIndexBuffer();
	int getIndexCount();

	void releaseBuffers();

private:

	void initRawBuffer();

	ID3D11Buffer* rawVertexBuffer;						// The buffer holding the vertices, indexed by byte
	ID3D11Buffer* rawIndexBuffer;						// Ditto for indices
	ID3D11Buffer* paramBuffer;							// A buffer holding scaling parameters

	ID3D11UnorderedAccessView* vertexBufferUAV;			// UAVs for writing to the raw buffers in the compute shader
	ID3D11UnorderedAccessView* indexBufferUAV;			// Note there are no corresponding SRVs as vertex & index buffers do not require them

	// Mesh size values - determines number of thread groups to dispatch
	int dimsX;
	int dimsY;
	int dimsZ;

	int indexCount;

};

#endif