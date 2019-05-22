// Main application file
// Program logic happens here
#ifndef _APP1_H_
#define _APP1_H_

#define TESTING_ false

// Includes
#include "../DXFramework/DXF.h"
#include "../DXFramework/PointMesh.h"
#include "LightShader.h"
#include "MCShader.h"
#include "EmptyMesh.h"
#include "GradientNoise.h"
#include "VoxelComputeShader.h"
#include "TriTableTexture.h"

class App1 : public BaseApplication
{
public:

	App1();
	virtual ~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in);

	bool frame();

protected:

	bool render();
	void gui();
	// Run function encapsulating all of the steps necessary to generate a new mesh
	void Run();
	void TestRun();

private:

	// Shaders
	VoxelComputeShader* voxelComputeShader;			// This shader generates the initial voxel point list
	GradientNoise* gradientNoiseShader;				// This shader generates the 3D noise volume which marching cubes will sample
	MCShader* marchingCubesShader;					// This shader generates the final output mesh
	LightShader* lightShader;						// Final rendering shader

	// The scene's directional light
	Light* mainLight;

	// Meshes
	EmptyMesh* voxelMesh;
	EmptyMesh* outputMesh;

	// Textures
	TriTableTexture* triTableTexture;

	// Keeps track of light direction based on user input
	float lightDirection;

	bool isWireframe;
	bool recalculateSurface;

	// Values for calculating the isosurface in the geometry shader
	float isovalue;
	// Mesh size also used for setting number of voxels and size of compute shader's output texture and number of threads to dispatch
	int meshSize;

	float meshScaleFactor;
	XMFLOAT3 noiseScaleFactors;

	// Noise values
	float frequency;
	float amplitude;
	int octaves;
	float persistence;
	XMFLOAT3 offsets;

	// Checks for the type of noise we'll be using in the gradient noise shader
	bool isRidged;
	bool isSimplex;

	// Values for the height increment method in the gradient noise shader
	float heightBase;
	float heightMultiplier;

#if TESTING_

	XMFLOAT3 testCases[100];

	ofstream fileStream;

	float elapsedTime;
	float timeInterval;
	int testCase;

#endif
	
};

#endif