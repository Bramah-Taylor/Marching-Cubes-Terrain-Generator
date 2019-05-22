#include "GradientNoise.h"

GradientNoise::GradientNoise(ID3D11Device* device, HWND hwnd) : BaseComputeShader(device, hwnd)
{

	cBuffer = nullptr;
	texture = nullptr;
	permutationTexture = nullptr;
	textureSRV = nullptr;
	textureUAV = nullptr;
	permutationSRV = nullptr;

	amplitude = 0.0f;
	frequency = 0.0f;
	persistence = 0.0f;
	octaves = 0;

	meshScaleFactor = 1.0f;
	noiseOffsets = XMFLOAT3(0.0f, 0.0f, 0.0f);
	noiseScaleFactors = XMFLOAT3(1.0f, 1.0f, 1.0f);

	heightBase = -0.7f;
	heightMultiplier = 3.0f;

	isRidged = false;
	isSimplex = false;

	initShader(L"gradient_noise_cs.cso", 0);

	CreatePermutationTexture(device);

}

GradientNoise::~GradientNoise()
{

	// Release state objects
	releaseConstantBuffer();
	releaseTexture();

	if (permutationTexture)
	{

		permutationTexture->Release();
		permutationTexture = nullptr;

	}

	if (permutationSRV)
	{

		permutationSRV->Release();
		permutationSRV = nullptr;

	}

}

void GradientNoise::initShader(WCHAR* filename, int elements)
{

	// Load the compute shader from file
	loadComputeShader(filename);

}

void GradientNoise::Run(ID3D11DeviceContext* deviceContext)
{

	// Set the shader
	deviceContext->CSSetShader(computeShader, nullptr, 0);
	// Set the shader's buffers and views
	deviceContext->CSSetConstantBuffers(0, 1, &cBuffer);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &textureUAV, nullptr);
	deviceContext->CSSetShaderResources(0, 1, &permutationSRV);

	// Launch the shader
	deviceContext->Dispatch(dimsX / 8, dimsY / 8, dimsZ / 8);

	// Reset the shader now we're done
	deviceContext->CSSetShader(nullptr, nullptr, 0);

	// Reset the shader views
	ID3D11UnorderedAccessView* ppUAViewnullptr[1] = { nullptr };
	deviceContext->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);

}

void GradientNoise::UpdateNoiseValues(float lamplitude, float lfrequency, float lpersistence, int loctaves, float lmeshScaleFactor, XMFLOAT3 scaleFactors, 
	XMFLOAT3 offsets, bool ridged, bool simplex, float hBase, float hMult)
{

	amplitude = lamplitude;
	frequency = lfrequency;
	persistence = lpersistence;
	octaves = loctaves;
	meshScaleFactor = lmeshScaleFactor;
	noiseScaleFactors = scaleFactors;
	noiseOffsets = offsets;
	isRidged = ridged;
	isSimplex = simplex;
	heightBase = hBase;
	heightMultiplier = hMult;

	// Rewrite the constant buffer's values now that we're updating them
	InitConstantBuffer();

}

void GradientNoise::UpdateMeshValues(int x, int y, int z)
{

	dimsX = x;
	dimsY = y;
	dimsZ = z;

	Init3DTexture();

}

void GradientNoise::releaseConstantBuffer()
{

	if (cBuffer)
	{

		cBuffer->Release();
		cBuffer = nullptr;

	}

}

void GradientNoise::releaseTexture()
{

	if (textureSRV)
	{

		textureSRV->Release();
		textureSRV = nullptr;

	}
	if (textureUAV)
	{

		textureUAV->Release();
		textureUAV = nullptr;

	}
	if (texture)
	{

		texture->Release();
		texture = nullptr;

	}

}

void GradientNoise::InitConstantBuffer()
{
	
	HRESULT result;

	releaseConstantBuffer();

	// Initialise the buffer data for the constant buffer
	BufferType cBufferData;
	cBufferData.amplitude = amplitude;
	cBufferData.frequency = frequency;
	cBufferData.persistence = persistence;
	cBufferData.octaves = octaves;
	cBufferData.meshScaleFactor = meshScaleFactor;
	cBufferData.noiseOffsets = noiseOffsets;
	cBufferData.dimsY = dimsY;
	cBufferData.noiseScaleFactors = noiseScaleFactors;
	cBufferData.isRidged = isRidged;
	cBufferData.isSimplex = isSimplex;
	cBufferData.heightBase = heightBase;
	cBufferData.heightMultiplier = heightMultiplier;

	// Create the noise buffer
	result = CreateConstantBuffer(sizeof(BufferType), &cBufferData, &cBuffer);
	if (result != S_OK)
	{

		MessageBox(NULL, L"Failed to create buffer", L"Failed", MB_OK);
		exit(0);

	}

}

void GradientNoise::Init3DTexture()
{

	HRESULT result;

	releaseTexture();

	// Initialise the texture (will be empty at first)
	result = CreateTexture3D(dimsX, dimsY, dimsZ, DXGI_FORMAT_R32_FLOAT, &texture);
	if (result != S_OK)
	{

		MessageBox(NULL, L"Failed to create texture on compute shader initialisation", L"Failed", MB_OK);
		exit(0);

	}
	// Create an unordered access view to that texture
	result = CreateTexture3DUAV(dimsZ, DXGI_FORMAT_R32_FLOAT, &texture, &textureUAV);
	if (result != S_OK)
	{

		MessageBox(NULL, L"Failed to create texture UAV on compute shader initialisation", L"Failed", MB_OK);
		exit(0);

	}

	// Create the shader resource view for access in other shaders
	result = CreateTexture3DSRV(DXGI_FORMAT_R32_FLOAT, &texture, &textureSRV);
	if (result != S_OK)
	{

		MessageBox(NULL, L"Failed to create texture SRV after compute shader execution", L"Failed", MB_OK);
		exit(0);

	}

}

ID3D11ShaderResourceView* GradientNoise::getTexture()
{

	return textureSRV;

}

void GradientNoise::CreatePermutationTexture(ID3D11Device* device)
{

	int perm[512] = { 151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	D3D11_TEXTURE1D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = 512;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_SINT;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	// Set the initial data of the texture to be the permutation table array
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	initData.pSysMem = perm;

	// Create the texture
	HRESULT hr = device->CreateTexture1D(&desc, &initData, &permutationTexture);
	if (hr != S_OK)
	{

		MessageBox(NULL, L"Failed to create permutation texture", L"Failed", MB_OK);
		exit(0);

	}

	// Create a shader resource view to that texture
	hr = device->CreateShaderResourceView(permutationTexture, NULL, &permutationSRV);
	if (hr != S_OK)
	{

		MessageBox(NULL, L"Failed to create permutation texture SRV", L"Failed", MB_OK);
		exit(0);

	}

}