// Marching cubes GPU application
#include "App1.h"
#include <chrono>
#include <fstream>
#include <cstdlib>

App1::App1()
{

	lightShader = nullptr;
	marchingCubesShader = nullptr;
	gradientNoiseShader = nullptr;
	voxelComputeShader = nullptr;

	voxelMesh = nullptr;
	outputMesh = nullptr;
	mainLight = nullptr;
	triTableTexture = nullptr;

	srand(time(NULL));

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in)
{

	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in);

	// Initialise the shaders
	lightShader = new LightShader(renderer->getDevice(), hwnd, POS4X32_NORM3X32);
	marchingCubesShader = new MCShader(renderer->getDevice(), renderer->getDeviceContext(), hwnd);
	gradientNoiseShader = new GradientNoise(renderer->getDevice(), hwnd);
	voxelComputeShader = new VoxelComputeShader(renderer->getDevice(), hwnd);

	// Initialise the textures from file
	textureMgr->loadTexture("grass", L"../res/grass.png");
	textureMgr->loadTexture("mossyrocks", L"../res/mossyrocks.png");
	textureMgr->loadTexture("rocks", L"../res/rocks.png");
	textureMgr->loadTexture("sand", L"../res/sand.png");

	// Create triangulation table texture from array data & set it in the marching cubes shader
	triTableTexture = new TriTableTexture(renderer->getDevice());
	marchingCubesShader->setTriTableTexture(triTableTexture->getTriTable());

	// Initialise the voxel mesh using the meshSize parameter
	voxelMesh = new EmptyMesh(renderer->getDevice(), true, true);
	voxelMesh->setTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	outputMesh = new EmptyMesh(renderer->getDevice(), false, false);

	lightDirection = -1.0f;

	// Initialise the light
	mainLight = new Light();
	mainLight->setDiffuseColour(0.9f, 0.9f, 0.9f, 1.0f);
	mainLight->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f);
	mainLight->setSpecularPower(50.0f);
	mainLight->setSpecularColour(1.0f, 1.0f, 1.0f, 1.0f);
	mainLight->setDirection(0.0f, lightDirection, 1.0f + lightDirection);

	// Initial mesh size will be 64 - this value MUST be a multiple of 8
	meshSize = 64;
	meshScaleFactor = 64.0f / meshSize;
	noiseScaleFactors = XMFLOAT3(1.0f, 2.7f, 1.0f);

	// Set an initial isovalue
	// Perlin noise returns values from -1.0f to 1.0f, so set 0.0f as isovalue
	isovalue = 0.0f;

	// Initialise values for generating the noise volume texture
	frequency = 0.02f;
	amplitude = 1.0f;
	octaves = 6;
	persistence = 0.45f;
	offsets = XMFLOAT3(0.0f, 0.0f, 0.0f);
	isSimplex = false;
	isRidged = false;
	heightBase = -0.7f;
	heightMultiplier = 3.0f;

	isWireframe = false;
	// We want a surface for the first frame, so set this to be true initially
	recalculateSurface = true;

#if TESTING_

	for (int i = 0; i < 100; i++)
	{

		testCases[i] = XMFLOAT3(rand() % 2000 - 1000, rand() % 2000 - 1000, rand() % 2000 - 1000);

	}

	fileStream = ofstream("timings.csv", std::ofstream::app);

	fileStream << "voxel gen" << "," << "noise gen" << "," << "marching cubes" << "," << "buffers" << "," << "total" << endl;

	elapsedTime = 0.0f;
	testCase = 0;
	timeInterval = 0.1f;

#endif

}


App1::~App1()
{

#if TESTING_

	fileStream.close();

#endif

	if (lightShader)
	{

		delete lightShader;
		lightShader = 0;

	}

	if (marchingCubesShader)
	{

		delete marchingCubesShader;
		marchingCubesShader = 0;

	}

	if (gradientNoiseShader)
	{

		delete gradientNoiseShader;
		gradientNoiseShader = 0;

	}

	if (voxelComputeShader)
	{

		delete voxelComputeShader;
		voxelComputeShader = 0;

	}

	if (mainLight)
	{

		delete mainLight;
		mainLight = 0;

	}

	if (voxelMesh)
	{

		delete voxelMesh;
		voxelMesh = 0;

	}

	if (outputMesh)
	{

		delete outputMesh;
		outputMesh = 0;

	}

	if (triTableTexture)
	{

		delete triTableTexture;
		triTableTexture = 0;

	}

	// Run base application deconstructor
	BaseApplication::~BaseApplication();

}

// The run function executes every stem required to get a new mesh representation of the surface
// 1: Calculate a new voxel point list
// 2: Calculate a new 3D texture using the noise shader
// 3: Run the marching cubes algorithm using the voxel point list and 3D texture
// 4: Put the geometry data streamed out into a mesh for rendering
void App1::Run()
{

	// Recalculate the number of voxels to send to the GPU
	voxelComputeShader->UpdateMeshValues(meshSize, meshSize, meshSize);
	voxelComputeShader->Run(renderer->getDeviceContext());

	voxelMesh->initBuffers(renderer->getDeviceContext(), voxelComputeShader->getVertexBuffer(), voxelComputeShader->getIndexBuffer());
	voxelMesh->setIndexCount(voxelComputeShader->getIndexCount());

	// Update the noise shader's parameters
	gradientNoiseShader->UpdateMeshValues(meshSize, meshSize, meshSize);
	gradientNoiseShader->UpdateNoiseValues(amplitude, frequency, persistence, octaves, meshScaleFactor, noiseScaleFactors, offsets, isRidged, isSimplex, heightBase, heightMultiplier);

	// Run the noise shader
	gradientNoiseShader->Run(renderer->getDeviceContext());

	// Then initialise the output buffer before running the marching cubes shader
	marchingCubesShader->reInitOutputBuffer(meshSize, meshSize, meshSize);

	// Then take compute shader texture and input into marching cubes shader
	voxelMesh->sendData(renderer->getDeviceContext());
	marchingCubesShader->setShaderParameters(renderer->getDeviceContext(), gradientNoiseShader->getTexture(), textureMgr->getTexture("rocks"), isovalue, meshSize, meshScaleFactor);
	// Then run the marching cubes shader
	marchingCubesShader->render(renderer->getDeviceContext(), voxelMesh->getIndexCount());

	voxelComputeShader->releaseBuffers();
	gradientNoiseShader->releaseConstantBuffer();
	gradientNoiseShader->releaseTexture();

	// Then take GS output buffer and input into empty mesh
	outputMesh->initBuffers(renderer->getDeviceContext(), marchingCubesShader->getOutputBuffer());

}

void App1::TestRun()
{

#if TESTING_

	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

	// Recalculate the number of voxels to send to the GPU
	voxelComputeShader->UpdateMeshValues(meshSize, meshSize, meshSize);
	
	voxelComputeShader->Run(renderer->getDeviceContext());

	voxelMesh->initBuffers(renderer->getDeviceContext(), voxelComputeShader->getVertexBuffer(), voxelComputeShader->getIndexBuffer());
	voxelMesh->setIndexCount(voxelComputeShader->getIndexCount());

	std::chrono::steady_clock::time_point noiseStart = std::chrono::steady_clock::now();

	// Update the noise shader's parameters
	gradientNoiseShader->UpdateMeshValues(meshSize, meshSize, meshSize);
	gradientNoiseShader->UpdateNoiseValues(amplitude, frequency, persistence, octaves, meshScaleFactor, noiseScaleFactors, offsets, isRidged, isSimplex, heightBase, heightMultiplier);

	// Run the noise shader
	gradientNoiseShader->Run(renderer->getDeviceContext());

	std::chrono::steady_clock::time_point noiseEnd = std::chrono::steady_clock::now();

	// Then initialise the output buffer before running the marching cubes shader
	marchingCubesShader->reInitOutputBuffer(meshSize, meshSize, meshSize);

	// Then take compute shader texture and input into marching cubes shader
	voxelMesh->sendData(renderer->getDeviceContext());
	marchingCubesShader->setShaderParameters(renderer->getDeviceContext(), gradientNoiseShader->getTexture(), textureMgr->getTexture("rocks"), isovalue, meshSize, meshScaleFactor);
	// Then run the marching cubes shader
	marchingCubesShader->render(renderer->getDeviceContext(), voxelMesh->getIndexCount());

	std::chrono::steady_clock::time_point marchingCubesEnd = std::chrono::steady_clock::now();

	voxelComputeShader->releaseBuffers();
	gradientNoiseShader->releaseConstantBuffer();
	gradientNoiseShader->releaseTexture();

	// Then take GS output buffer and input into empty mesh
	outputMesh->initBuffers(renderer->getDeviceContext(), marchingCubesShader->getOutputBuffer());

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	auto timeTotal = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	auto timeNoise = std::chrono::duration_cast<std::chrono::microseconds>(noiseEnd - noiseStart).count();
	auto timeMarchingCubes = std::chrono::duration_cast<std::chrono::microseconds>(marchingCubesEnd - noiseEnd).count();
	auto timeVoxelGen = std::chrono::duration_cast<std::chrono::microseconds>(noiseStart - start).count();
	auto timeBufferRelease = std::chrono::duration_cast<std::chrono::microseconds>(end - marchingCubesEnd).count();

	fileStream << timeVoxelGen << "," << timeNoise << "," << timeMarchingCubes << "," << timeBufferRelease << "," << timeTotal << endl;

#endif

}

// Ideally frame and render should be decoupled, but as we're only rendering with no additional functionality it doesn't matter
bool App1::frame()
{

	bool result;

	result = BaseApplication::frame();
	if (!result)
	{

		return false;

	}

#if TESTING_

	elapsedTime += timer->getTime();

	if (elapsedTime > timeInterval)
	{

		elapsedTime = 0.0f;

		offsets = testCases[testCase % 100];

		TestRun();

		testCase++;

		if (testCase % 400 == 0)
		{

			fileStream << endl << endl << endl;

			if (meshSize == 256)
			{

				exit(0);

			}
			else if (meshSize == 128)
			{

				timeInterval = 1.0f;

			}

			testCase = 1;
			meshSize *= 2;
			meshScaleFactor = 64.0f / meshSize;
			isSimplex = false;
			isRidged = false;

		}
		else if (testCase % 300 == 0)
		{

			isRidged = true;

			fileStream << endl;

		}
		else if (testCase % 200 == 0)
		{

			isSimplex = true;
			isRidged = false;

			fileStream << endl;

		}
		else if (testCase % 100 == 0)
		{

			isRidged = true;

			fileStream << endl;

		} 

	}

#endif

#if !TESTING_

	if (recalculateSurface)
	{

		Run();

		recalculateSurface = false;

}

#endif

	// Render the graphics
	result = render();
	if (!result)
	{

		return false;

	}

	return true;

}

bool App1::render()
{

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Update the light direction
	mainLight->setDirection(0.0f, lightDirection, 1.0f + lightDirection);

	// Clear the scene (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position
	camera->update();

	// Set the wireframe rendering mode based on user input
	renderer->setWireframeMode(isWireframe);

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data (from mesh)
	outputMesh->sendData(renderer->getDeviceContext());
	// Set shader parameters
	lightShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("rocks"), 
		textureMgr->getTexture("grass"), textureMgr->getTexture("mossyrocks"), textureMgr->getTexture("sand"), mainLight, camera, meshScaleFactor);
	// Render object (combination of mesh geometry and shader process)
	lightShader->render(renderer->getDeviceContext(), outputMesh->getIndexCount());

	renderer->setWireframeMode(false);

	// Render GUI
	gui();

	// Flip buffers & present the rendered scene to the screen
	renderer->endScene();

	return true;

}

void App1::gui()
{

	// Force turn off on Geometry shader
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::SliderFloat("Light Direction", &lightDirection, -1.0f, 0.0f);
	ImGui::Checkbox("Wireframe", &isWireframe);
	if (ImGui::SliderFloat("Isovalue", &isovalue, -1.0f, 2.0f) ||
		ImGui::Checkbox("Ridged Turbulence", &isRidged) ||
		ImGui::Checkbox("Simplex Noise", &isSimplex))
	{

		recalculateSurface = true;

	}
	if (ImGui::InputInt("Mesh Size", &meshSize))
	{

		// Make sure that the noise shader doesn't encounter an error
		if (meshSize % 8 != 0)
		{

			meshSize = 64;

		}

		meshScaleFactor = 64.0f / meshSize;
		recalculateSurface = true;

	}
	// Contain noise values separately
	if (ImGui::CollapsingHeader("Noise Values"))
	{

		if (ImGui::InputFloat("Frequency", &frequency) ||
			ImGui::InputFloat("Amplitude", &amplitude) ||
			ImGui::InputInt("Octaves", &octaves) ||
			ImGui::InputFloat("Persistence", &persistence) ||
			ImGui::DragFloat("Offset X", &offsets.x) ||
			ImGui::DragFloat("Offset Y", &offsets.y) ||
			ImGui::DragFloat("Offset Z", &offsets.z) ||
			ImGui::DragFloat("Noise Scale X", &noiseScaleFactors.x, 0.001f) ||
			ImGui::DragFloat("Noise Scale Y", &noiseScaleFactors.y, 0.001f) ||
			ImGui::DragFloat("Noise Scale Z", &noiseScaleFactors.z, 0.001f) ||
			ImGui::InputFloat("Height Base", &heightBase) ||
			ImGui::InputFloat("Height Multiplier", &heightMultiplier))
		{

			recalculateSurface = true;

		}

	}

	// Render UI
	ImGui::Render();

}