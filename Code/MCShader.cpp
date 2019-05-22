// Marching Cubes shader
#include "MCShader.h"


MCShader::MCShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd) : BaseShader(device, hwnd)
{
	
	initShader(L"marching_cubes_vs.cso", L"marching_cubes_ps.cso", L"marching_cubes_gs.cso", deviceContext);

}


MCShader::~MCShader()
{

	// Release all DirectX resources used
	if (matrixBuffer)
	{

		matrixBuffer->Release();
		matrixBuffer = 0;

	}

	if (layout)
	{

		layout->Release();
		layout = 0;

	}

	if (paramBuffer)
	{

		paramBuffer->Release();
		paramBuffer = 0;

	}

	releaseOutputBuffer();

	if (streamOutputGeometryShader)
	{

		streamOutputGeometryShader->Release();
		streamOutputGeometryShader = 0;

	}

	if (triTableSRV)
	{

		triTableSRV->Release();
		triTableSRV = 0;

	}

	//Release base shader components
	BaseShader::~BaseShader();

}

void MCShader::initShader(WCHAR* vsFilename, WCHAR* psFilename, InputLayoutType inputLayout)
{

	// empty

}

void MCShader::initShader(WCHAR* vsFilename, WCHAR* psFilename, WCHAR* gsFilename, ID3D11DeviceContext* deviceContext)
{

	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC paramBufferDesc;
	
	// Load (+ compile) shader files
	loadVertexShader(vsFilename, POS4XUINT);
	loadSOGeometryShader(gsFilename, deviceContext);
	loadPixelShader(psFilename);

	// Create a texture sampler state description
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader
	paramBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	paramBufferDesc.ByteWidth = sizeof(ParamBufferType);
	paramBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	paramBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	paramBufferDesc.MiscFlags = 0;
	paramBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class
	renderer->CreateBuffer(&paramBufferDesc, NULL, &paramBuffer);

}

void MCShader::setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* rockTexture,
	float isoValue, float meshSize, float scaleFactor)
{

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ParamBufferType* paramBufferPtr;

	// Get the parameters from the application
	result = deviceContext->Map(paramBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	paramBufferPtr = (ParamBufferType*)mappedResource.pData;
	paramBufferPtr->isoValue = isoValue;
	paramBufferPtr->meshSize = meshSize;
	paramBufferPtr->meshScaleFactor = scaleFactor;
	paramBufferPtr->padding = 0.0f;
	deviceContext->Unmap(paramBuffer, 0);

	// Now set the constant buffer in the geometry shader with the updated values.
	deviceContext->GSSetConstantBuffers(0, 1, &paramBuffer);

	// Set the texture resources
	deviceContext->GSSetShaderResources(0, 1, &noiseTexture);
	deviceContext->GSSetShaderResources(1, 1, &triTableSRV);

}

void MCShader::loadSOGeometryShader(WCHAR* filename, ID3D11DeviceContext* deviceContext)
{

	ID3D10Blob* geometryShaderBuffer;

	// check file extension for correct loading function.
	std::wstring fn(filename);
	std::string::size_type idx;
	std::wstring extension;

	idx = fn.rfind('.');

	if (idx != std::string::npos)
	{

		extension = fn.substr(idx + 1);

	}
	else
	{

		// No extension found
		MessageBox(hwnd, L"Error finding geometry shader file", L"ERROR", MB_OK);
		exit(0);

	}

	// Load the texture in
	if (extension != L"cso")
	{

		MessageBox(hwnd, L"Incorrect geometry shader file type", L"ERROR", MB_OK);
		exit(0);

	}

	// Reads compiled shader into buffer (bytecode)
	HRESULT result = D3DReadFileToBlob(filename, &geometryShaderBuffer);
	if (result != S_OK)
	{

		MessageBox(NULL, filename, L"File not found", MB_OK);
		exit(0);

	}

	D3D11_SO_DECLARATION_ENTRY SODeclarationEntry[2] =
	{

		{ 0, "SV_POSITION", 0, 0, 4, 0 },
		{ 0, "NORMAL", 0, 0, 3, 0 }

	};

	// Create the geometry shader from the buffer
	result = renderer->CreateGeometryShaderWithStreamOutput(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), SODeclarationEntry, _countof(SODeclarationEntry),
		NULL, 0, D3D11_SO_NO_RASTERIZED_STREAM, NULL, &streamOutputGeometryShader);
	if (result != S_OK)
	{

		MessageBox(NULL, filename, L"Failed to create stream output geometry shader", MB_OK);
		exit(0);

	}

	geometryShaderBuffer->Release();
	geometryShaderBuffer = 0;

}

ID3D11Buffer* MCShader::getOutputBuffer()
{

	return outputBuffer;

}

void MCShader::releaseOutputBuffer()
{

	if (outputBuffer)
	{

		outputBuffer->Release();
		outputBuffer = nullptr;

	}

}

void MCShader::reInitOutputBuffer(int x, int y, int z)
{

	releaseOutputBuffer();

	voxelsX = x;
	voxelsY = y;
	voxelsZ = z;

	// Define a divisor to set the scale of the SO buffer according to sampling dimensions
	// This needs to change due to the fact as the scale of the resolution of the sampling dimensions increases,
	// the number of empty cells also increases, i.e. more voxels correlates with fewer triangles per voxel.
	// Using this, we can save memory by allocating proportionally less space for larger volumes
	int divisorHeuristic = 1;

	if (voxelsX >= 64)
	{

		// This will need to change if we're considering multiple dimensions
		// Fortunately, right now all dimensions are equal, so we can run this code on only the X dimension
		divisorHeuristic = voxelsX / 32;

	}

	// Create the stream output buffer
	HRESULT result;
	D3D11_BUFFER_DESC outputBufferDesc;
	ZeroMemory(&outputBufferDesc, sizeof(outputBufferDesc));
	outputBufferDesc.ByteWidth = (sizeof(XMFLOAT4) + sizeof(XMFLOAT3)) * ((voxelsX * voxelsY * voxelsZ) / divisorHeuristic);// Vertex size * number of voxels
	outputBufferDesc.Usage = D3D11_USAGE_DEFAULT;															// This is almost certainly going to be a bigger buffer than necessary
	outputBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	outputBufferDesc.CPUAccessFlags = 0;
	outputBufferDesc.MiscFlags = 0;
	outputBufferDesc.StructureByteStride = 0;

	// Create the buffer
	result = renderer->CreateBuffer(&outputBufferDesc, NULL, &outputBuffer);
	if (result != S_OK)
	{
		MessageBox(NULL, L"Failed to create stream output buffer", L"Fail", MB_OK);
		exit(0);
	}

}

void MCShader::render(ID3D11DeviceContext* deviceContext, int indexCount)
{

	// Set the vertex input layout
	deviceContext->IASetInputLayout(layout);

	deviceContext->GSSetSamplers(0, 1, &sampleState);

	// Set the vertex and pixel shaders that will be used to render
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->GSSetShader(streamOutputGeometryShader, NULL, 0);

	UINT offset[1] = { 0 };
	deviceContext->SOSetTargets(1, &outputBuffer, offset);

	// Render the geometry
	deviceContext->DrawIndexed(indexCount, 0, 0);

	ID3D11Buffer* pNullBuffer = 0;
	deviceContext->SOSetTargets(1, &pNullBuffer, offset);

	ID3D11ShaderResourceView* emptySRV[1] = { nullptr };

	deviceContext->GSSetShaderResources(0, 1, emptySRV);
	deviceContext->GSSetShaderResources(1, 1, emptySRV);

}

void MCShader::setTriTableTexture(ID3D11ShaderResourceView* triTableTexture)
{

	triTableSRV = triTableTexture;

}