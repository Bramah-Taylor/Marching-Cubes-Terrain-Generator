// Noise effect file
// Note: Effects officially deprecated as of DX11; use this file with caution
#ifndef _NOISE_FX_
#define _NOISE_FX_

// 1D texture containing the permutation table data
Texture1D<int> permutationTexture : register(t0);

// Basic hash function
float hash(float n)
{

	return frac(sin(n)*43758.5453);

}

// Gradient noise function - NOT Perlin noise
// Original function by Inigo Quilez
float gnoise(float3 x)
{
	// The noise function returns a value in the range -1.0f -> 1.0f
	// Note that this function has a high level of regularity to its output along axes

	float3 p = floor(x);
	float3 f = frac(x);

	f = f * f*(3.0 - 2.0*f);
	float n = p.x + p.y*57.0 + 113.0*p.z;

	return lerp(lerp(lerp(hash(n + 0.0), hash(n + 1.0), f.x),
		lerp(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
		lerp(lerp(hash(n + 113.0), hash(n + 114.0), f.x),
			lerp(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);

}

// Interpolation function from improved Perlin noise (6t^5 - 15t^4 + 10t^3)
float fade(float t)
{

	return (t * t * t * (t * (t * 6 - 15) + 10));

}

float grad3(int hash, float x, float y, float z) {
	int h = hash & 15;									// Convert low 4 bits of hash code into 12 simple
	float u = h < 8 ? x : y;							// gradient directions, and compute dot product.
	float v = h < 4 ? y : h == 12 || h == 14 ? x : z;	// Fix repeats at h = 12 to 15
	return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

// 3D Perlin noise function
// Adapted from C implementation by Stefan Gustavson
// Available here: https://github.com/stegu/perlin-noise/blob/master/src/noise1234.c
float noise3(float3 input)
{

	int3 i0, i1;
	float3 f0, f1;
	float s, t, r;
	float nxy0, nxy1, nx0, nx1, n0, n1;

	i0 = floor(input);			// Integer part of input
	f0 = input - i0;			// Fractional part of input
	f1 = f0 - 1.0f;
	i1 = (i0 + 1) & 0xff;		// Wrap to 0..255
	i0 = i0 & 0xff;

	r = fade(f0.z);
	t = fade(f0.y);
	s = fade(f0.x);

	nxy0 = grad3(permutationTexture.Load(int2(i0.x + permutationTexture.Load(int2(i0.y + permutationTexture.Load(int2(i0.z, 0)), 0)), 0)), f0.x, f0.y, f0.z);
	nxy1 = grad3(permutationTexture.Load(int2(i0.x + permutationTexture.Load(int2(i0.y + permutationTexture.Load(int2(i1.z, 0)), 0)), 0)), f0.x, f0.y, f1.z);
	nx0 = lerp(nxy0, nxy1, r);

	nxy0 = grad3(permutationTexture.Load(int2(i0.x + permutationTexture.Load(int2(i1.y + permutationTexture.Load(int2(i0.z, 0)), 0)), 0)), f0.x, f1.y, f0.z);
	nxy1 = grad3(permutationTexture.Load(int2(i0.x + permutationTexture.Load(int2(i1.y + permutationTexture.Load(int2(i1.z, 0)), 0)), 0)), f0.x, f1.y, f1.z);
	nx1 = lerp(nxy0, nxy1, r);

	n0 = lerp(nx0, nx1, t);

	nxy0 = grad3(permutationTexture.Load(int2(i1.x + permutationTexture.Load(int2(i0.y + permutationTexture.Load(int2(i0.z, 0)), 0)), 0)), f1.x, f0.y, f0.z);
	nxy1 = grad3(permutationTexture.Load(int2(i1.x + permutationTexture.Load(int2(i0.y + permutationTexture.Load(int2(i1.z, 0)), 0)), 0)), f1.x, f0.y, f1.z);
	nx0 = lerp(nxy0, nxy1, r);

	nxy0 = grad3(permutationTexture.Load(int2(i1.x + permutationTexture.Load(int2(i1.y + permutationTexture.Load(int2(i0.z, 0)), 0)), 0)), f1.x, f1.y, f0.z);
	nxy1 = grad3(permutationTexture.Load(int2(i1.x + permutationTexture.Load(int2(i1.y + permutationTexture.Load(int2(i1.z, 0)), 0)), 0)), f1.x, f1.y, f1.z);
	nx1 = lerp(nxy0, nxy1, r);

	n1 = lerp(nx0, nx1, t);

	// Requires rescaling of approximately 0.936 to match original Perlin noise output value range
	return 0.936f * (lerp(n0, n1, s));

}

// 3D Simplex noise function
// Adapted from C implementation by Stefan Gustavson
// Available here: https://github.com/stegu/perlin-noise/blob/master/src/simplexnoise1234.c
// Additionally adapted from GLSL implementation by Stefan Gustavson
// Available here: https://github.com/stegu/webgl-noise/blob/master/src/noise3D.glsl
float snoise3(float3 input)
{

	const float2  C = float2(1.0f / 6.0f, 1.0f / 3.0f);
	const float4  D = float4(0.0f, 0.5f, 1.0f, 2.0f);

	float n0, n1, n2, n3; // Noise contributions from the four corners

	// First corner
	int3 i = floor(input + dot(input, C.yyy));
	float3 x0 = input - i + dot(i, C.xxx);

	// Other corners
	float3 g = step(x0.yzx, x0.xyz);
	float3 l = 1.0 - g;
	float3 i1 = min(g.xyz, l.zxy);
	float3 i2 = max(g.xyz, l.zxy);

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	float3 x1 = x0 - i1 + C.xxx;
	float3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	float3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

	// Calculate the contribution from the four corners
	float t0 = 0.5f - x0.x * x0.x - x0.y * x0.y - x0.z * x0.z;
	float t1 = 0.5f - x1.x * x1.x - x1.y * x1.y - x1.z * x1.z;
	float t2 = 0.5f - x2.x * x2.x - x2.y * x2.y - x2.z * x2.z;
	float t3 = 0.5f - x3.x * x3.x - x3.y * x3.y - x3.z * x3.z;

	// Wrap the integer indices at 256, to avoid indexing perm[] out of bounds
	int ii = i.x & 0xff;
	int jj = i.y & 0xff;
	int kk = i.z & 0xff;

	if (t0 < 0.0f)
	{

		n0 = 0.0f;

	}
	else
	{

		t0 *= t0;
		n0 = t0 * t0 * grad3(permutationTexture.Load(int2(ii + permutationTexture.Load(int2(jj + permutationTexture.Load(int2(kk, 0)), 0)), 0)), x0.x, x0.y, x0.z);

	}

	if (t1 < 0.0f)
	{

		n1 = 0.0f;

	}
	else
	{

		t1 *= t1;
		n1 = t1 * t1 * grad3(permutationTexture.Load(int2(ii + i1.x + permutationTexture.Load(int2(jj + i1.y + permutationTexture.Load(int2(kk + i1.z, 0)), 0)), 0)), x1.x, x1.y, x1.z);

	}

	if (t2 < 0.0f)
	{

		n2 = 0.0f;

	}
	else
	{

		t2 *= t2;
		n2 = t2 * t2 * grad3(permutationTexture.Load(int2(ii + i2.x + permutationTexture.Load(int2(jj + i2.y + permutationTexture.Load(int2(kk + i2.z, 0)), 0)), 0)), x2.x, x2.y, x2.z);

	}

	if (t3 < 0.0f)
	{

		n3 = 0.0f;

	}
	else
	{

		t3 *= t3;
		n3 = t3 * t3 * grad3(permutationTexture.Load(int2(ii + 1 + permutationTexture.Load(int2(jj + 1 + permutationTexture.Load(int2(kk + 1, 0)), 0)), 0)), x3.x, x3.y, x3.z);

	}

	// Add contributions from each corner to get the final noise value.
	// The result is scaled to stay just inside [-1,1]
	return 72.0f * (n0 + n1 + n2 + n3);

}

#endif