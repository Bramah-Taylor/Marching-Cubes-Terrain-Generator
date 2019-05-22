// Base compute shader file
// Handles loading functionality & creation of various DX11 resources and buffers
#ifndef _BASE_COMPUTE_SHADER_H_
#define _BASE_COMPUTE_SHADER_H_

#include <d3d11.h>
#include <D3Dcompiler.h>
#include <dxgi.h>
#include <directxmath.h>
#include <fstream>
#include "../DXFramework/DXF.h"

class BaseComputeShader
{

public:

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

	BaseComputeShader(ID3D11Device* device, HWND hwnd);
	virtual ~BaseComputeShader();

	virtual void Run(ID3D11DeviceContext* deviceContext) = 0;

protected:

	virtual void initShader(WCHAR* csFilename, int elements) = 0;

	// Loads the compute shader object from file
	void loadComputeShader(WCHAR* filename);

	// Creates a structured buffer for read access on the GPU
	HRESULT CreateStructuredBuffer(UINT elementSize, UINT count, void* pInitData, ID3D11Buffer** buffer);
	// Creates a raw buffer for read access on the GPU
	HRESULT CreateRawBuffer(UINT elementSize, void* initialisationData, ID3D11Buffer** buffer);
	// Creates a constant buffer for read access on the GPU
	HRESULT CreateConstantBuffer(UINT elementSize, void* initialisationData, ID3D11Buffer** buffer);

	// Creates an empty texture2D object to be filled by a shader
	HRESULT CreateTexture2D(UINT width, UINT height, DXGI_FORMAT format, ID3D11Texture2D** texture);
	// Creates an unordered access view to a texture2D object, allowing for compute shader read/write access
	HRESULT CreateTexture2DUAV(DXGI_FORMAT format, ID3D11Texture2D** texture, ID3D11UnorderedAccessView** unorderedAccessView);
	// Creates a shader resource view to a texture2D object, allowing for normal shader sampling access
	// Note that the SRV CANNOT be used while a UAV is accessing this texture resource
	HRESULT CreateTexture2DSRV(DXGI_FORMAT format, ID3D11Texture2D** texture, ID3D11ShaderResourceView** shaderResourceView);
	
	// Creates an empty texture3D object to be filled by a shader
	HRESULT CreateTexture3D(UINT width, UINT height, UINT depth, DXGI_FORMAT format, ID3D11Texture3D** texture);
	// Creates an unordered access view to a texture3D object, allowing for compute shader read/write access
	HRESULT CreateTexture3DUAV(UINT depth, DXGI_FORMAT format, ID3D11Texture3D** texture, ID3D11UnorderedAccessView** unorderedAccessView);
	// Creates a shader resource view to a texture3D object, allowing for normal shader sampling access
	// Note that the SRV CANNOT be used while a UAV is accessing this texture resource
	HRESULT CreateTexture3DSRV(DXGI_FORMAT format, ID3D11Texture3D** texture, ID3D11ShaderResourceView** shaderResourceView);

	// Creates a shader resource view to enable the structured & raw buffers to be read on the GPU
	HRESULT CreateBufferSRV(ID3D11Buffer* buffer, ID3D11ShaderResourceView** shaderResourceView);
	// Creates an unordered access view that enables read/write access to the buffer on the GPU
	HRESULT CreateBufferUAV(ID3D11Buffer* buffer, ID3D11UnorderedAccessView** unorderedAccessView, bool isAppendBuffer = false);

	// Copies data from buffer parameter into a new buffer, allowing for writes to data structures on the CPU
	ID3D11Buffer* CopyToSystemBuffer(ID3D11DeviceContext* deviceContext, ID3D11Buffer* buffer);

	ID3D11Device* renderer;
	HWND hwnd;

	ID3D11ComputeShader* computeShader;

};

#endif
