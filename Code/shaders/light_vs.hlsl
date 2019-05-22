// Light vertex shader
// Standard vertex shader; calculate necessary information per vertex for the pixel shader, then pass down the pipeline

cbuffer MatrixBuffer : register(b0)
{

    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;

};

cbuffer CameraBuffer : register(b1)
{

	float3 cameraPosition;
	float meshScaleFactor;

};

struct InputType
{

    float4 position : POSITION;
	float3 normal : NORMAL;

};

struct OutputType
{

    float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 tex : TEXCOORD0;
	float3 realPosition : TEXCOORD1;

};

OutputType main(InputType input)
{

    OutputType output;

	// realPosition is used to determine the height of the vertex in world space
	// This allows for height based texturing
	output.realPosition = input.position.xyz;

	// Calculate the position of the vertex in the world
	float4 worldPosition = mul(input.position, worldMatrix);

    // Calculate the position of the vertex against the world, view, and projection matrices
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	 // Calculate the normal vector against the world matrix only
    output.normal = mul(input.normal, (float3x3)worldMatrix);

	output.tex = input.position.xyz;
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    return output;

}