#include "BaseComputeShader.h"

BaseComputeShader::BaseComputeShader(ID3D11Device* device, HWND hwnd)
{

	renderer = device;
	hwnd = hwnd;

	computeShader = nullptr;

}

BaseComputeShader::~BaseComputeShader()
{

	// Free the compute shader
	if (computeShader)
	{

		computeShader->Release();
		computeShader = nullptr;

	}

}

void BaseComputeShader::loadComputeShader(WCHAR* filename)
{

	ID3DBlob* computeShaderBuffer;

	// Check file extension for correct loading function
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

		// No extension found, output error
		MessageBox(hwnd, L"Error finding compute shader file", L"ERROR", MB_OK);
		exit(0);

	}

	// Load the shader object
	if (extension != L"cso")
	{

		MessageBox(hwnd, L"Incorrect compute shader file type", L"ERROR", MB_OK);
		exit(0);

	}

	// Reads compiled shader into buffer (bytecode)
	HRESULT result = D3DReadFileToBlob(filename, &computeShaderBuffer);
	if (result != S_OK)
	{

		MessageBox(NULL, filename, L"File not found", MB_OK);
		exit(0);

	}
	// Create the compute shader from the buffer
	result = renderer->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, &computeShader);
	if (result != S_OK)
	{

		MessageBox(NULL, filename, L"Failed to create compute shader", MB_OK);
		exit(0);

	}

	// Release shader bytecode now that we have a compute shader attached to the renderer
	computeShaderBuffer->Release();
	computeShaderBuffer = 0;

}

HRESULT BaseComputeShader::CreateStructuredBuffer(UINT elementSize, UINT count, void* initialisationData, ID3D11Buffer** buffer)
{

	// Set up the buffer description based on input parameters
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.ByteWidth = elementSize * count;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = elementSize;

	// Create the buffer depending on whether we're using initialisation data or not
	if (initialisationData)
	{

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = initialisationData;
		return renderer->CreateBuffer(&bufferDesc, &initData, buffer);

	}
	else
	{

		return renderer->CreateBuffer(&bufferDesc, nullptr, buffer);

	}

}

HRESULT BaseComputeShader::CreateRawBuffer(UINT elementSize, void* initialisationData, ID3D11Buffer** buffer)
{

	// Set up the buffer description based on input parameters
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.ByteWidth = elementSize;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	// Create the buffer depending on whether we're using initialisation data or not
	if (initialisationData)
	{

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = initialisationData;
		return renderer->CreateBuffer(&bufferDesc, &initData, buffer);

	}
	else
	{

		return renderer->CreateBuffer(&bufferDesc, nullptr, buffer);

	}

}

HRESULT BaseComputeShader::CreateConstantBuffer(UINT elementSize, void* initialisationData, ID3D11Buffer** buffer)
{

	// Set up the buffer description based on input parameters
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.ByteWidth = elementSize;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Create the buffer depending on whether we're using initialisation data or not
	if (initialisationData)
	{

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = initialisationData;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;
		return renderer->CreateBuffer(&cbDesc, &initData, buffer);

	}
	else
	{

		return renderer->CreateBuffer(&cbDesc, nullptr, buffer);

	}

}

HRESULT BaseComputeShader::CreateTexture2D(UINT width, UINT height, DXGI_FORMAT format, ID3D11Texture2D** texture)
{

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	return renderer->CreateTexture2D(&textureDesc, 0, texture);

}

HRESULT BaseComputeShader::CreateTexture2DUAV(DXGI_FORMAT format, ID3D11Texture2D** texture, ID3D11UnorderedAccessView** unorderedAccessView)
{

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	return renderer->CreateUnorderedAccessView(*texture, &uavDesc, unorderedAccessView);

}

HRESULT BaseComputeShader::CreateTexture2DSRV(DXGI_FORMAT format, ID3D11Texture2D** texture, ID3D11ShaderResourceView** shaderResourceView)
{

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	return renderer->CreateShaderResourceView(*texture, &srvDesc, shaderResourceView);

}

HRESULT BaseComputeShader::CreateTexture3D(UINT width, UINT height, UINT depth, DXGI_FORMAT format, ID3D11Texture3D** texture)
{

	D3D11_TEXTURE3D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Depth = depth;
	textureDesc.MipLevels = 1;
	textureDesc.Format = format;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	return renderer->CreateTexture3D(&textureDesc, 0, texture);

}

HRESULT BaseComputeShader::CreateTexture3DUAV(UINT depth, DXGI_FORMAT format, ID3D11Texture3D** texture, ID3D11UnorderedAccessView** unorderedAccessView)
{

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Texture3D.MipSlice = 0;
	uavDesc.Texture3D.FirstWSlice = 0;
	uavDesc.Texture3D.WSize = depth;

	return renderer->CreateUnorderedAccessView(*texture, &uavDesc, unorderedAccessView);

}

HRESULT BaseComputeShader::CreateTexture3DSRV(DXGI_FORMAT format, ID3D11Texture3D** texture, ID3D11ShaderResourceView** shaderResourceView)
{

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.MipLevels = 1;

	return renderer->CreateShaderResourceView(*texture, &srvDesc, shaderResourceView);

}

HRESULT BaseComputeShader::CreateBufferSRV(ID3D11Buffer* buffer, ID3D11ShaderResourceView** shaderResourceView)
{

	// Get buffer description from the buffer (assuming we created it before)
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	buffer->GetDesc(&bufferDesc);

	// Set up the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;

	// Check whether we're using a raw or structured buffer so that the shader resource view is described appropriately
	if (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{

		// Raw buffer
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;	// Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
		srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		srvDesc.BufferEx.NumElements = bufferDesc.ByteWidth / 4;

	}
	else
	{

		if (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{

			// Structured buffer
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;	// Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
			srvDesc.BufferEx.Flags = 0;
			srvDesc.BufferEx.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;

		}
		else
		{

			return E_INVALIDARG;

		}

	}

	return renderer->CreateShaderResourceView(buffer, &srvDesc, shaderResourceView);

}

HRESULT BaseComputeShader::CreateBufferUAV(ID3D11Buffer* buffer, ID3D11UnorderedAccessView** unorderedAccessView, bool isAppendBuffer)
{

	// Get buffer description from the buffer (assuming we created it before)
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	buffer->GetDesc(&bufferDesc);

	// Set up the unordered access view
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;

	// Check whether we're using a raw or structured buffer so that the unordered access view is described appropriately
	if (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{

		// Raw buffer
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / 4;

	}
	else
	{

		if (bufferDesc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{

			// Structured buffer
			uavDesc.Format = DXGI_FORMAT_UNKNOWN;	// Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer

												// Append/consume buffer flag
			if (isAppendBuffer)
			{
				// Gives a performance improvement if the data we're using doesn't need to be indexed
				uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;

			}
			else
			{

				uavDesc.Buffer.Flags = 0;

			}

			uavDesc.Buffer.NumElements = bufferDesc.ByteWidth / bufferDesc.StructureByteStride;

		}
		else
		{

			return E_INVALIDARG;

		}

	}

	return renderer->CreateUnorderedAccessView(buffer, &uavDesc, unorderedAccessView);

}

ID3D11Buffer* BaseComputeShader::CopyToSystemBuffer(ID3D11DeviceContext* deviceContext, ID3D11Buffer* buffer)
{

	// Initialise the system memory buffer we'll be writing to
	ID3D11Buffer* tempBuffer = nullptr;

	// Set up the temporary buffer
	D3D11_BUFFER_DESC tempBufferDesc;
	ZeroMemory(&tempBufferDesc, sizeof(tempBufferDesc));
	buffer->GetDesc(&tempBufferDesc);
	tempBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	tempBufferDesc.Usage = D3D11_USAGE_STAGING;
	tempBufferDesc.BindFlags = 0;
	tempBufferDesc.MiscFlags = 0;

	// Copy memory from the target buffer only if the buffer was created suffessfully
	if (SUCCEEDED(renderer->CreateBuffer(&tempBufferDesc, nullptr, &tempBuffer)))
	{

		deviceContext->CopyResource(tempBuffer, buffer);

	}

	return tempBuffer;

}