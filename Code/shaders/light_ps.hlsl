// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)
// Slope-based texturing adapted from Rastertek slope based texturing tutorial: http://www.rastertek.com/tertut14.html

Texture2D rockTexture  : register(t0);
Texture2D grassTexture : register(t1);
Texture2D slopeTexture : register(t2);
Texture2D sandTexture  : register(t3);

SamplerState SampleType : register(s0);

// Light buffer contains all lighting values from the light object
// Note that we're not actually using specularity in this shader at present
// This is to maintain lighting consistency with a comparable terrain generator for analysis
cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
    float4 diffuseColor;
    float3 lightDirection;
	float specularPower;
	float4 specularColor;
	
};

struct InputType
{

    float4 position : SV_POSITION;
	float3 normal : NORMAL;
    float3 tex : TEXCOORD0;
	float3 realPosition: TEXCOORD1;

};

float4 main(InputType input) : SV_TARGET
{

	// Final texture value
	float4 textureColor;
	// Samplex texture values
	float4 grassColor;
	float4 rockColor;
	float4 sandColor;
	// Intermediary texture values based on combination of height and slope
	float4 groundColor;
	float4 slopeColor;
	// Diffuse lighting values
    float3 lightDir;
    float lightIntensity;
    float4 color;
	float textureScale = 5.0f;

	// Triplana mapping blend weightd
	// Calculate the weights based on the normal
	float3 blendWeights = pow(abs(input.normal), 1.0f);
	// Normalize the weights
	blendWeights /= (blendWeights.x + blendWeights.y + blendWeights.z);

	// Triplanar mapping - for each texture
	// Source: http://www.martinpalko.com/triplanar-mapping/
	// Sample down each axis

	// Rock texture
	float4 xaxis = rockTexture.Sample(SampleType, input.tex.yz / textureScale);
	float4 yaxis = rockTexture.Sample(SampleType, input.tex.xz / textureScale);
	float4 zaxis = rockTexture.Sample(SampleType, input.tex.xy / textureScale);

	// Calculate the final texture from the samples using the blending weights
	rockColor = xaxis * blendWeights.x + yaxis * blendWeights.y + zaxis * blendWeights.z;

	// Grass texture
	xaxis = grassTexture.Sample(SampleType, input.tex.yz / textureScale);
	yaxis = grassTexture.Sample(SampleType, input.tex.xz / textureScale);
	zaxis = grassTexture.Sample(SampleType, input.tex.xy / textureScale);

	grassColor = xaxis * blendWeights.x + yaxis * blendWeights.y + zaxis * blendWeights.z;

	// Sand texture
	xaxis = sandTexture.Sample(SampleType, input.tex.yz / textureScale);
	yaxis = sandTexture.Sample(SampleType, input.tex.xz / textureScale);
	zaxis = sandTexture.Sample(SampleType, input.tex.xy / textureScale);

	sandColor = xaxis * blendWeights.x + yaxis * blendWeights.y + zaxis * blendWeights.z;

	// Get a value for the height (less than 0 will always be sand, above 1 will never be sand)
	float height = saturate(input.realPosition.y * 0.15f);

	// Set height to height^5 for better falloff on the sand textures
	height = height * height * height * height * height;

	// Update the ground and slope textures by linearly interpolating between relevant textures using the height values
	groundColor = lerp(sandColor, grassColor, height);
	slopeColor = lerp(sandColor, rockColor, height);

	// Calculate the slope of this point
	float slope = 1.0f - input.normal.y;
	float blendAmount = 0.0f;

	// Determine which texture to use based on height
	if (slope < 0.3f)
	{
		blendAmount = slope / 0.3f;
		textureColor = lerp(groundColor, slopeColor, blendAmount);
	}

	if ((slope < 0.7f) && (slope >= 0.3f))
	{
		blendAmount = (slope - 0.3f) * (1.0f / (0.7f - 0.3f));
		textureColor = lerp(slopeColor, rockColor, blendAmount);
	}

	if (slope >= 0.7f)
	{
		textureColor = rockColor;
	}

	// Set a default color
	color = ambientColor * textureColor;
	
	// Invert the light direction for calculations.
	lightDir = -lightDirection.xyz;

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, lightDir));

	// If diffuse light is there, add it to the light
	if (lightIntensity > -0.2f) 
	{

		// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
		float4 diffuseComponent = saturate(diffuseColor * lightIntensity);
		diffuseComponent = saturate(diffuseComponent * textureColor);
		color += saturate(diffuseComponent);

	}

    return color;

}

