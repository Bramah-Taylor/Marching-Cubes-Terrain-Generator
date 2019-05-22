// Marching cubes pixel shader
// This shader ***WILL NOT RUN***
// Rasteriser state set to D3D11_SO_NO_RASTERIZED_STREAM when the geometry shader is created

Texture2D mainTexture : register(t0);

SamplerState SampleType : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float3 tex : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{

	float4 textureColor;

	float4 xaxis = mainTexture.Sample(SampleType, input.tex.yz / 5.0f);
	float4 yaxis = mainTexture.Sample(SampleType, input.tex.xz / 5.0f);
	float4 zaxis = mainTexture.Sample(SampleType, input.tex.xy / 5.0f);

	float3 blendWeights = pow(abs(input.normal), 1.0f);
	blendWeights /= (blendWeights.x + blendWeights.y + blendWeights.z);

	float4 tex = xaxis * blendWeights.x + yaxis * blendWeights.y + zaxis * blendWeights.z;

	// Sample the grass color from the texture using the sampler at this texture coordinate location.
	textureColor = tex;

	return textureColor;

}