// Marching cubes vertex shader
// Simple geometry pass

struct InputType
{
	uint4 position : POSITION;
};

struct OutputType
{
	float4 position : SV_POSITION;
};

OutputType main(InputType input)
{
	OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices
	output.position = float4(input.position.x, input.position.y, input.position.z, input.position.w);

	return output;
}