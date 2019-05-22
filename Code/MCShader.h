// Marching cubes shader
// Generates geometry using the marching cubes algorithm
#ifndef _MARCHING_CUBES_SHADER_H_
#define _MARCHING_CUBES_SHADER_H_

#include "../DXFramework/BaseShader.h"

using namespace std;
using namespace DirectX;

class MCShader : public BaseShader
{

	// Buffer to hold the geometry shader parameters
	// Will later include variables for multidimensional scaling and cell scaling
	struct ParamBufferType
	{

		float isoValue;
		float meshSize;
		float meshScaleFactor;
		float padding;

	};

public:

	MCShader(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND hwnd);
	~MCShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* noiseTexture, ID3D11ShaderResourceView* rockTexture,
		float isoValue, float meshSize, float scaleFactor);

	// Returns the stream output buffer - to be used as a vertex buffer in an EmptyMesh
	ID3D11Buffer* getOutputBuffer();
	void setTriTableTexture(ID3D11ShaderResourceView* triTableTexture);

	// Re-initialise the output buffer if the amount of geometry we expect to output changes
	void reInitOutputBuffer(int x, int y, int z);

	void render(ID3D11DeviceContext* deviceContext, int indexCount);

	void releaseOutputBuffer();

private:

	void initShader(WCHAR*, WCHAR*, InputLayoutType inputLayout);
	void initShader(WCHAR* vs, WCHAR* ps, WCHAR* gs, ID3D11DeviceContext* deviceContext);
	// Function for loading the geometry shader from file
	void loadSOGeometryShader(WCHAR* filename, ID3D11DeviceContext* deviceContext);

private:

	// Buffers
	ID3D11Buffer* outputBuffer;
	ID3D11Buffer* paramBuffer;

	// Geometry shader
	ID3D11GeometryShader* streamOutputGeometryShader;

	// Triangle table texture to be passed to the shader
	ID3D11ShaderResourceView* triTableSRV;
	
	// Values for mesh scaling purposes
	int voxelsX;
	int voxelsY;
	int voxelsZ;
	float meshScaleFactor;

};

#endif // !_MARCHING_CUBES_SHADER_H_