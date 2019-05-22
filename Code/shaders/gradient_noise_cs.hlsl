// Gradient noise compute shader
// Calculates gradient noise volumes using Perlin/Simplex noise algorithms
#include "noise_fx.hlsl"

cbuffer noiseBuffer : register(b0)
{

	// fBm values
	float amplitude;
	float frequency;
	float persistence;
	int octaves;
	// meshScaleFactor is included here to make sure we sample the same volume regardless of size
	float meshScaleFactor;
	float3 offsets;
	// dimsY determines height increment values, explained below
	int dimsY;
	float3 noiseScaleFactors;
	// Bools for determining fBm type
	bool isRidged;
	bool isSimplex;
	// Height increment value modifiers
	float heightBase;
	float heightMultiplier;

};

// Fractional Brownian motion function
// Expanded to implement both Perlin and Simplex noise, plus ridged turbulence
float fBm(float3 input)
{

	float noiseValue = 0.0f;
	float noise2 = 0.0f;
	float value = 0.0f;
	float localAmplitude = amplitude;
	float localFrequency = frequency;

	// Loop for the number of octaves, running the noise function as many times as desired (8 is usually sufficient)
	for (int k = 0; k < octaves; k++)
	{

		// Check whether we should use Simplex noise or Perlin noise
		if (isSimplex)
		{

			noiseValue = snoise3(float3(((input.x * meshScaleFactor * noiseScaleFactors.x) + offsets.x) * localFrequency,
				((input.y * meshScaleFactor * noiseScaleFactors.y) + offsets.y) * localFrequency,
				((input.z * meshScaleFactor * noiseScaleFactors.z) + offsets.z) * localFrequency)
			) * localAmplitude;

			// Check if we're doing ridged turbulence implementation
			if (isRidged)
			{

				noise2 = snoise3(float3(((input.x * meshScaleFactor * noiseScaleFactors.x) + offsets.x) * localFrequency,
					((input.y * meshScaleFactor * noiseScaleFactors.y) + offsets.y + 150.0f) * localFrequency,
					((input.z * meshScaleFactor * noiseScaleFactors.z) + offsets.z) * localFrequency)
				) * localAmplitude;

			}

		}
		else
		{

			noiseValue = noise3(float3(((input.x * meshScaleFactor * noiseScaleFactors.x) + offsets.x) * localFrequency,
				((input.y * meshScaleFactor * noiseScaleFactors.y) + offsets.y) * localFrequency,
				((input.z * meshScaleFactor * noiseScaleFactors.z) + offsets.z) * localFrequency)
			) * localAmplitude;

			if (isRidged)
			{

				noise2 = noise3(float3(((input.x * meshScaleFactor * noiseScaleFactors.x) + offsets.x) * localFrequency,
					((input.y * meshScaleFactor * noiseScaleFactors.y) + offsets.y + 150.0f) * localFrequency,
					((input.z * meshScaleFactor * noiseScaleFactors.z) + offsets.z) * localFrequency)
				) * localAmplitude;

			}

		}

		// Return the greater of the two noise values, then find absolute value
		// This gives ridges in some forms of terrain
		if (isRidged)
		{

			if (noise2 > noiseValue)
			{

				noiseValue = noise2;

			}

			value += abs(noiseValue);

		}
		else
		{

			value += noiseValue;

		}

		// Calculate a new amplitude based on the input persistence/gain value
		// amplitudeLoop will get smaller as the number of layers (i.e. k) increases
		localAmplitude *= persistence;
		// Calculate a new frequency based on a lacunarity value of 2.0
		// This gives us 2^k as the frequency
		// i.e. Frequency at k = 4 will be f * 2^4 as we have looped 4 times
		localFrequency *= 2.0f;

	}

	return value;

}

// The texture we're writing to
RWTexture3D<float> outputTexture : register(u0);

// Total threads per block is 512
[numthreads(8, 8, 8)]
void main(uint3 DTid : SV_DispatchThreadID)
{

	// Get the noise value
	float value = fBm((float3)DTid.xyz);

	// Example of a domain warping algorithm - doesn't seem to have an actual effect on the terrain
	//float value = fBm((float3)DTid.xyz + fBm((float3)DTid.xyz + fBm((float3)DTid.xyz)));

	// Calculate an increment value based on height
	// Lower values of y will tend to negative, higher values will tend to positive, i.e. both will be further from the default isovalue
	// This can effectively be used to trim noise values at the top and bottom of the texture
	// This gives us a terrain that looks like an actual terrain
	float increment = DTid.y / (float)dimsY;
	float heightValue = heightBase + (increment * heightMultiplier);

	// Output final value to 3D index in the texture provided by thread indexing
	outputTexture[DTid.xyz] = value + heightValue;

}