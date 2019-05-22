// Compute shader that creates a 3D gradient noise texture using gradient noise methods
// Texture can be accessed by other shaders using the SRV accessed with the getTexture function

#ifndef _NOISE_COMPUTE_H_
#define _NOISE_COMPUTE_H_

#include "BaseComputeShader.h"

class GradientNoise : public BaseComputeShader
{
private:

	// Holds the fBm noise values
	struct BufferType
	{

		float amplitude;
		float frequency;
		float persistence;
		int octaves;
		float meshScaleFactor;
		XMFLOAT3 noiseOffsets;
		int dimsY;
		XMFLOAT3 noiseScaleFactors;
		int isRidged;
		int isSimplex;
		float heightBase;
		float heightMultiplier;

	};

public:

	GradientNoise(ID3D11Device* device, HWND hwnd);
	~GradientNoise();

	void initShader(WCHAR* csFilename, int elements);
	void Run(ID3D11DeviceContext* deviceContext);

	// Update the noise values when they're changed by user input
	void UpdateNoiseValues(float amplitude, float frequency, float persistence, int octaves, float meshScaleFactor, XMFLOAT3 noiseScaleFactors, 
		XMFLOAT3 offsets, bool ridged, bool simplex, float hBase, float hMult);
	// Update the mesh values when the mesh size is changed
	void UpdateMeshValues(int x, int y, int z);

	// Get the shader resource view created from the shader output for use in pixel shaders
	ID3D11ShaderResourceView* getTexture();

	// Release redundant resources to reduce memory usage
	void releaseTexture();
	void releaseConstantBuffer();

private:

	// Creates the constant buffer for noise parameters in the shader
	void InitConstantBuffer();
	// Creates the 3D texture and resource views we'll be using
	void Init3DTexture();

	void CreatePermutationTexture(ID3D11Device* device);

	// Buffers & textures
	ID3D11Buffer* cBuffer;							// Buffer holding noise values
	ID3D11Texture3D* texture;						// Output texture object accessed by views
	ID3D11Texture1D* permutationTexture;			// Texture format of the permutation table to be passed to the compute shader

	// Views
	ID3D11UnorderedAccessView* textureUAV;			// UAV to the texture output - compute shader use only
	ID3D11ShaderResourceView* textureSRV;			// SRV to the texture output for pixel shader use
	ID3D11ShaderResourceView* permutationSRV;		// SRV to the permutation texture - compute shader use only

	// Noise values
	float amplitude;
	float frequency;
	float persistence;
	int octaves;
	float meshScaleFactor;
	XMFLOAT3 noiseOffsets;
	XMFLOAT3 noiseScaleFactors;
	bool isRidged;
	bool isSimplex;
	// Height increment values
	float heightBase;
	float heightMultiplier;

	// Mesh size values - determines number of thread groups to dispatch
	// And hence final output texture size
	int dimsX;
	int dimsY;
	int dimsZ;

};

#endif // !_NOISE_COMPUTE_H_