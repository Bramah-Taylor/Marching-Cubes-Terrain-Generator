// Voxel compute shader
// Creates an indexed 3D volume of vertices that can be used as an input point list for a geometry shader
cbuffer ParamBuffer : register(b0)
{

	float3 dimensions;
	float padding;

}

// The buffers we're writing to
RWByteAddressBuffer vertexBuffer : register(u0);
RWByteAddressBuffer indexBuffer : register(u1);

[numthreads(8, 8, 8)]
void main(uint3 DTid : SV_DispatchThreadID)
{

	// Determine what this vertex's index is based on thread ID
	uint indexByte = ((DTid.z * dimensions.y * dimensions.x) + (DTid.y * dimensions.x) + DTid.x) * 4;

	// Store the vertex in a vertex buffer
	vertexBuffer.Store4(indexByte * 4, uint4(DTid.x, DTid.y, DTid.z, 1));
	// And store an index value accordingly in the index buffer
	indexBuffer.Store(indexByte, uint((DTid.z * dimensions.y * dimensions.x) + (DTid.y * dimensions.x) + DTid.x));

}