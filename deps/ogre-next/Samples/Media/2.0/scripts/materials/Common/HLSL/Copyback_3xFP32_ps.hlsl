
Texture2D<float3>	myTexture : register(t0);
SamplerState		mySampler : register(s0);

float3 main( float2 uv : TEXCOORD0 ) : SV_Target
{
	return myTexture.Sample( mySampler, uv ).xyz;
}
