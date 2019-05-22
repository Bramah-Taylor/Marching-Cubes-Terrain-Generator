// Textures
Texture3D<float> noiseTexture : register(t0);
Texture2D<int> triTableTexture : register(t1);

// Sampler set up on the CPU side
SamplerState sampleType : register(s0);

// Parameters set by the user
cbuffer ParamBuffer : register(b0)
{

	float isoValue;
	float meshSize;
	float meshScaleFactor;
	float padding;

};

// Edge table from Paul Bourke's source: http://paulbourke.net/geometry/polygonise/
static int edgeTable[256] = {
	0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
	0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
	0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
	0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
	0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
	0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
	0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
	0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
	0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
	0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
	0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
	0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
	0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
	0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
	0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
	0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
	0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
	0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
	0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
	0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
	0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
	0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
	0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
	0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
	0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
	0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
	0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
	0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
	0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
	0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
	0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
	0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0 };

struct InputType
{

	float4 position : SV_POSITION;

};

struct OutputType
{

	float4 position : SV_POSITION;
	float3 normal : NORMAL;

};

// Triangle table texture loading
// Source: https://github.com/Tsarpf/MarchingCubesGPU/blob/master/GPUMarchingCubes/GPUMarchingCubes/GeometryShader.hlsl
int triTableValue(int i, int j)
{

	if (i >= 256 || j >= 16)
	{

		return -1;

	}

	return triTableTexture.Load(int3(j, i, 0));

}

// Trilinear vertex interpolation, from Paul Bourke's reference implementation
float3 VertexInterp(float isoValue, float3 p1, float3 p2, float valp1, float valp2)
{

	float mu = 0.0f;
	float3 p = 0.0f;

	if (abs(isoValue - valp1) < 0.00001)
		return(p1);
	if (abs(isoValue - valp2) < 0.00001)
		return(p2);
	if (abs(valp1 - valp2) < 0.00001)
		return(p1);

	mu = (isoValue - valp1) / (valp2 - valp1);
	p.x = p1.x + mu * (p2.x - p1.x);
	p.y = p1.y + mu * (p2.y - p1.y);
	p.z = p1.z + mu * (p2.z - p1.z);

	return(p);

}

// Calculate a new normal vertex by sampling either side of the cell in each dimension
float3 CalculateNormal(float3 position)
{

	float normal[3];
	// Initialise values for fractional Brownian motion
	normal[0] = 0.0f;
	normal[1] = 0.0f;
	normal[2] = 0.0f;

	normal[0] = noiseTexture.SampleLevel(sampleType, float3(position.x + 1.0f, position.y, position.z) / meshSize, 0)
		- noiseTexture.SampleLevel(sampleType, float3(position.x - 1.0f, position.y, position.z) / meshSize, 0);
	normal[1] = noiseTexture.SampleLevel(sampleType, float3(position.x, position.y + 1.0f, position.z) / meshSize, 0)
		- noiseTexture.SampleLevel(sampleType, float3(position.x, position.y - 1.0f, position.z) / meshSize, 0);
	normal[2] = noiseTexture.SampleLevel(sampleType, float3(position.x, position.y, position.z + 1.0f) / meshSize, 0)
		- noiseTexture.SampleLevel(sampleType, float3(position.x, position.y, position.z - 1.0f) / meshSize, 0);

	float length = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	normal[0] /= length;
	normal[1] /= length;
	normal[2] /= length;

	return float3(normal[0], normal[1], normal[2]);

}

[maxvertexcount(15)]
void main(point InputType input[1], inout TriangleStream<OutputType> triStream)
{
	OutputType output;

	// Define the corner positions
	float3 cornerPositions[8];
	cornerPositions[0] = float3(input[0].position.x, input[0].position.y, input[0].position.z + 1.0f);
	cornerPositions[1] = float3(input[0].position.x + 1.0f, input[0].position.y, input[0].position.z + 1.0f);
	cornerPositions[2] = float3(input[0].position.x + 1.0f, input[0].position.y, input[0].position.z);
	cornerPositions[3] = input[0].position.xyz;
	cornerPositions[4] = float3(input[0].position.x, input[0].position.y + 1.0f, input[0].position.z + 1.0f);
	cornerPositions[5] = float3(input[0].position.x + 1.0f, input[0].position.y + 1.0f, input[0].position.z + 1.0f);
	cornerPositions[6] = float3(input[0].position.x + 1.0f, input[0].position.y + 1.0f, input[0].position.z);
	cornerPositions[7] = float3(input[0].position.x, input[0].position.y + 1.0f, input[0].position.z);

	// Set the corner values by sampling corner positions
	float cornerValues[8];
	cornerValues[0] = noiseTexture.SampleLevel(sampleType, cornerPositions[0] / meshSize, 0);
	cornerValues[1] = noiseTexture.SampleLevel(sampleType, cornerPositions[1] / meshSize, 0);
	cornerValues[2] = noiseTexture.SampleLevel(sampleType, cornerPositions[2] / meshSize, 0);
	cornerValues[3] = noiseTexture.SampleLevel(sampleType, cornerPositions[3] / meshSize, 0);
	cornerValues[4] = noiseTexture.SampleLevel(sampleType, cornerPositions[4] / meshSize, 0);
	cornerValues[5] = noiseTexture.SampleLevel(sampleType, cornerPositions[5] / meshSize, 0);
	cornerValues[6] = noiseTexture.SampleLevel(sampleType, cornerPositions[6] / meshSize, 0);
	cornerValues[7] = noiseTexture.SampleLevel(sampleType, cornerPositions[7] / meshSize, 0);

	// Scale the positions to their correct location in world space
	cornerPositions[0] *= meshScaleFactor;
	cornerPositions[1] *= meshScaleFactor;
	cornerPositions[2] *= meshScaleFactor;
	cornerPositions[3] *= meshScaleFactor;
	cornerPositions[4] *= meshScaleFactor;
	cornerPositions[5] *= meshScaleFactor;
	cornerPositions[6] *= meshScaleFactor;
	cornerPositions[7] *= meshScaleFactor;

	// Determine the cube configuration of the cell/voxel
	int cubeIndex = 0;
	if (cornerValues[0] < isoValue)
	{
		cubeIndex |= 1;
	}
	if (cornerValues[1] < isoValue)
	{
		cubeIndex |= 2;
	}
	if (cornerValues[2] < isoValue)
	{
		cubeIndex |= 4;
	}
	if (cornerValues[3] < isoValue)
	{
		cubeIndex |= 8;
	}
	if (cornerValues[4] < isoValue)
	{
		cubeIndex |= 16;
	}
	if (cornerValues[5] < isoValue)
	{
		cubeIndex |= 32;
	}
	if (cornerValues[6] < isoValue)
	{
		cubeIndex |= 64;
	}
	if (cornerValues[7] < isoValue)
	{
		cubeIndex |= 128;
	}

	// Check that the cell is not empty
	if (cubeIndex != 0 && cubeIndex != 255)
	{

		float3 vertlist[12];

		// Find the vertices where the surface intersects the cube from the edges using trilinear interpolation
		// Bottom far
		if (edgeTable[cubeIndex] & 1)
		vertlist[0] = VertexInterp(isoValue, cornerPositions[0], cornerPositions[1], cornerValues[0], cornerValues[1]);
		// Bottom right
		if (edgeTable[cubeIndex] & 2)
		vertlist[1] = VertexInterp(isoValue, cornerPositions[1], cornerPositions[2], cornerValues[1], cornerValues[2]);
		// Bottom near
		if (edgeTable[cubeIndex] & 4)
		vertlist[2] = VertexInterp(isoValue, cornerPositions[2], cornerPositions[3], cornerValues[2], cornerValues[3]);
		// Bottom left
		if (edgeTable[cubeIndex] & 8)
		vertlist[3] = VertexInterp(isoValue, cornerPositions[3], cornerPositions[0], cornerValues[3], cornerValues[0]);
		// Top far
		if (edgeTable[cubeIndex] & 16)
		vertlist[4] = VertexInterp(isoValue, cornerPositions[4], cornerPositions[5], cornerValues[4], cornerValues[5]);
		// Top right
		if (edgeTable[cubeIndex] & 32)
		vertlist[5] = VertexInterp(isoValue, cornerPositions[5], cornerPositions[6], cornerValues[5], cornerValues[6]);
		// Top near
		if (edgeTable[cubeIndex] & 64)
		vertlist[6] = VertexInterp(isoValue, cornerPositions[6], cornerPositions[7], cornerValues[6], cornerValues[7]);
		// Top left
		if (edgeTable[cubeIndex] & 128)
		vertlist[7] = VertexInterp(isoValue, cornerPositions[7], cornerPositions[4], cornerValues[7], cornerValues[4]);
		// Centre far left
		if (edgeTable[cubeIndex] & 256)
		vertlist[8] = VertexInterp(isoValue, cornerPositions[0], cornerPositions[4], cornerValues[0], cornerValues[4]);
		// Centre far right
		if (edgeTable[cubeIndex] & 512)
		vertlist[9] = VertexInterp(isoValue, cornerPositions[1], cornerPositions[5], cornerValues[1], cornerValues[5]);
		// Centre near right
		if (edgeTable[cubeIndex] & 1024)
		vertlist[10] = VertexInterp(isoValue, cornerPositions[2], cornerPositions[6], cornerValues[2], cornerValues[6]);
		// Centre near left
		if (edgeTable[cubeIndex] & 2048)
		vertlist[11] = VertexInterp(isoValue, cornerPositions[3], cornerPositions[7], cornerValues[3], cornerValues[7]);

		// Calculate polygons from the detected vertices
		for (int i = 0; triTableValue(cubeIndex, i) != -1; i += 3)
		{

			float3 vposition = vertlist[triTableValue(cubeIndex, i)];
			output.position = float4(vposition, 1.0f);
			output.normal = CalculateNormal(vposition / meshScaleFactor);
			triStream.Append(output);

			float3 v2position = vertlist[triTableValue(cubeIndex, i + 1)];
			output.position = float4(v2position, 1.0f);
			output.normal = CalculateNormal(v2position / meshScaleFactor);
			triStream.Append(output);

			float3 v3position = vertlist[triTableValue(cubeIndex, i + 2)];
			output.position = float4(v3position, 1.0f);
			output.normal = CalculateNormal(v3position / meshScaleFactor);
			triStream.Append(output);

			triStream.RestartStrip();

		}

	}

}