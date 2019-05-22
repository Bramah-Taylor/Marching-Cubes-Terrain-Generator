#include "lightshader.h"

LightShader::LightShader(ID3D11Device* device, HWND hwnd, InputLayoutType inputLayout) : BaseShader(device, hwnd)
{

	initShader(L"light_vs.cso", L"light_ps.cso", inputLayout);

}

LightShader::~LightShader()
{

	// Release the sampler state
	if (sampleState)
	{

		sampleState->Release();
		sampleState = 0;

	}

	// Release the matrix constant buffer
	if (matrixBuffer)
	{

		matrixBuffer->Release();
		matrixBuffer = 0;

	}

	// Release the input layout
	if (layout)
	{

		layout->Release();
		layout = 0;

	}

	// Release the light constant buffer.
	if (lightBuffer)
	{

		lightBuffer->Release();
		lightBuffer = 0;

	}

	// Release the camera buffer
	if (cameraBuffer)
	{

		cameraBuffer->Release();
		cameraBuffer = 0;

	}

	//Release base shader components
	BaseShader::~BaseShader();

}


void LightShader::initShader(WCHAR* vsFilename, WCHAR* psFilename, InputLayoutType inputLayout)
{

	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename, inputLayout);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	// Setup camera buffer
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;

	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

}


void LightShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix,
	ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* texture3, ID3D11ShaderResourceView* texture4,
	Light* light, Camera* camera, float meshScaleFactor)
{

	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* lightPtr;
	CameraBufferType* cameraPtr;
	unsigned int bufferNumber;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;

	// Unlock the constant buffer
	deviceContext->Unmap(matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &matrixBuffer);

	//Additional
	// Send light data to pixel shader
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->diffuse = light->getDiffuseColour();
	lightPtr->direction = light->getDirection();
	lightPtr->ambient = light->getAmbientColour();
	lightPtr->specularPower = light->getSpecularPower();
	lightPtr->specularColor = light->getSpecularColour();
	deviceContext->Unmap(lightBuffer, 0);
	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &lightBuffer);

	// Set up camera buffer and send data across to the vertex shader
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->cameraPosition = camera->getPosition();
	cameraPtr->meshScaleFactor = meshScaleFactor;
	deviceContext->Unmap(cameraBuffer, 0);
	bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &cameraBuffer);

	// Set shader texture resources in the pixel shader
	deviceContext->PSSetShaderResources(0, 1, &texture1);
	deviceContext->PSSetShaderResources(1, 1, &texture2);
	deviceContext->PSSetShaderResources(2, 1, &texture3);
	deviceContext->PSSetShaderResources(3, 1, &texture4);

}

void LightShader::render(ID3D11DeviceContext* deviceContext, int indexCount)
{

	// Set the sampler state in the pixel shader
	deviceContext->PSSetSamplers(0, 1, &sampleState);

	// Set the vertex input layout
	deviceContext->IASetInputLayout(layout);

	// Set the vertex and pixel shaders that will be used to render
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->PSSetShader(pixelShader, NULL, 0);

	// if Hull shader is not null then set HS and DS
	if (hullShader)
	{

		deviceContext->HSSetShader(hullShader, NULL, 0);
		deviceContext->DSSetShader(domainShader, NULL, 0);

	}
	else
	{

		deviceContext->HSSetShader(NULL, NULL, 0);
		deviceContext->DSSetShader(NULL, NULL, 0);

	}

	// if geometry shader is not null then set GS
	if (geometryShader)
	{

		deviceContext->GSSetShader(geometryShader, NULL, 0);

	}
	else
	{

		deviceContext->GSSetShader(NULL, NULL, 0);

	}

	// Render the geometry using DrawAuto due to stream output geometry
	deviceContext->DrawAuto();

}



