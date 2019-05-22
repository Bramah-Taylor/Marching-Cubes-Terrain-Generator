// The texture we're writing to
struct BufferType
{

	float i;

};

StructuredBuffer<BufferType> dataBuffer : register(t0);
RWTexture2D<float> outputTexture : register(u0);

[numthreads(1, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{

	// Output value to 2D index in the texture provided by thread indexing
	if (dataBuffer[DTid.x].i < 11 && dataBuffer[DTid.x].i > -2)
	{

		outputTexture[DTid.xy] = dataBuffer[DTid.x].i;

	}
	else
	{

		outputTexture[DTid.xy] = 0.0f;

	}

}