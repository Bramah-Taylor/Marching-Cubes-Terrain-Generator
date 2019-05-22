#include "VoxelComputeShader.h"

VoxelComputeShader::VoxelComputeShader(ID3D11Device* device, HWND hwnd) : BaseComputeShader(device, hwnd)
{

	rawVertexBuffer = nullptr;
	rawIndexBuffer = nullptr;
	paramBuffer = nullptr;

	dimsX = 0;
	dimsY = 0;
	dimsZ = 0;

	initShader(L"voxel_compute_cs.cso", 0);

}

VoxelComputeShader::~VoxelComputeShader()
{

	releaseBuffers();

}

void VoxelComputeShader::initShader(WCHAR* filename, int elements)
{

	// Load the compute shader from file
	loadComputeShader(filename);

}

void VoxelComputeShader::Run(ID3D11DeviceContext* deviceContext)
{

	// Set the shader
	deviceContext->CSSetShader(computeShader, nullptr, 0);
	// Set the shader's UAVs and constant buffer
	deviceContext->CSSetConstantBuffers(0, 1, &paramBuffer);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &vertexBufferUAV, nullptr);
	deviceContext->CSSetUnorderedAccessViews(1, 1, &indexBufferUAV, nullptr);

	// Launch the shader
	deviceContext->Dispatch(dimsX / 8, dimsY / 8, dimsZ / 8);

	// Reset the shader now we're done
	deviceContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* ppUAViewnullptr[1] = { nullptr };
	deviceContext->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);
	deviceContext->CSSetUnorderedAccessViews(1, 1, ppUAViewnullptr, nullptr);

}

void VoxelComputeShader::initRawBuffer()
{

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	HRESULT result;

	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	// Set up the description of the static vertex buffer
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(XMFLOAT4) * dimsX * dimsY * dimsZ;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	vertexBufferDesc.StructureByteStride = 0;
	// Now create the vertex buffer
	result = renderer->CreateBuffer(&vertexBufferDesc, nullptr, &rawVertexBuffer);
	if (result != S_OK)
	{

		MessageBox(NULL, L"Failed to create vertex buffer", L"Voxel Compute Shader", MB_OK);
		exit(0);

	}

	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	// Set up the description of the index buffer
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * dimsX * dimsY * dimsZ;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER | D3D11_BIND_UNORDERED_ACCESS;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data
	// Create the index buffer
	result = renderer->CreateBuffer(&indexBufferDesc, nullptr, &rawIndexBuffer);
	if (result != S_OK)
	{

		MessageBox(NULL, L"Failed to create index buffer", L"Voxel Compute Shader", MB_OK);
		exit(0);

	}

}

void VoxelComputeShader::UpdateMeshValues(int x, int y, int z)
{

	dimsX = x;
	dimsY = y;
	dimsZ = z;

	releaseBuffers();

	HRESULT result;

	initRawBuffer();

	result = CreateBufferUAV(rawVertexBuffer, &vertexBufferUAV);
	if (result != S_OK)
	{

		MessageBox(NULL, L"Failed to create buffer UAV", L"Voxel Compute Shader", MB_OK);
		exit(0);

	}

	result = CreateBufferUAV(rawIndexBuffer, &indexBufferUAV);
	if (result != S_OK)
	{

		MessageBox(NULL, L"Failed to create buffer UAV", L"Voxel Compute Shader", MB_OK);
		exit(0);

	}

	// Initialise the buffer data for the constant buffer
	ParamBuffer paramBufferData;
	paramBufferData.dimensions = XMFLOAT3(dimsX, dimsY, dimsZ);
	paramBufferData.padding = 0.0f;

	// Create the noise buffer
	result = CreateConstantBuffer(sizeof(ParamBuffer), &paramBufferData, &paramBuffer);
	if (result != S_OK)
	{

		MessageBox(NULL, L"Failed to create constant buffer buffer", L"Voxel Compute Shader", MB_OK);
		exit(0);

	}

}

void VoxelComputeShader::releaseBuffers()
{

	if (rawVertexBuffer)
	{

		rawVertexBuffer->Release();
		rawVertexBuffer = nullptr;

	}

	if (rawIndexBuffer)
	{

		rawIndexBuffer->Release();
		rawIndexBuffer = nullptr;

	}

	if (vertexBufferUAV)
	{

		vertexBufferUAV->Release();
		vertexBufferUAV = nullptr;

	}

	if (indexBufferUAV)
	{

		indexBufferUAV->Release();
		indexBufferUAV = nullptr;

	}

	if (paramBuffer)
	{

		paramBuffer->Release();
		paramBuffer = nullptr;

	}

}

ID3D11Buffer* VoxelComputeShader::getVertexBuffer()
{

	return rawVertexBuffer;

}

ID3D11Buffer* VoxelComputeShader::getIndexBuffer()
{

	return rawIndexBuffer;

}

int VoxelComputeShader::getIndexCount()
{

	return dimsX * dimsY * dimsZ;

}