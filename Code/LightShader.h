// LightShader
// Basic single diffuse light shader setup
// Used for rendering the terrain
#ifndef _LIGHTSHADER_H_
#define _LIGHTSHADER_H_

#include "../DXFramework/BaseShader.h"
#include "../DXFramework/Light.h"
#include "../DXFramework/Camera.h"

using namespace std;
using namespace DirectX;


class LightShader : public BaseShader
{

private:

	// Struct used for creating light cbuffer for the GPU
	struct LightBufferType
	{

		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float specularPower;
		XMFLOAT4 specularColor;

	};

	// Struct used for creating the camera cbuffer for the GPU
	struct CameraBufferType
	{

		XMFLOAT3 cameraPosition;
		float meshScaleFactor;

	};

public:

	LightShader(ID3D11Device* device, HWND hwnd, InputLayoutType inputLayout);
	~LightShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, 
		ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2, ID3D11ShaderResourceView* texture3, ID3D11ShaderResourceView* texture4, 
		Light* light, Camera* camera, float meshScaleFactor);
	void render(ID3D11DeviceContext* deviceContext, int vertexCount);

private:

	void initShader(WCHAR*, WCHAR*, InputLayoutType inputLayout);

private:

	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;

};

#endif