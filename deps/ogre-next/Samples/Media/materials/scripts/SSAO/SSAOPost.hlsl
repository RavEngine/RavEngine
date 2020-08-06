SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

struct noFilterIn
{
	float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 noFilter_fp (
	noFilterIn input,
    uniform Texture2D sOcclusion : register(s0)
) : SV_Target
{
    float4 oColor0 = float4(sOcclusion.Sample(g_samLinear, input.uv).xyz, 1);
	return oColor0;
} 


// a very simple 4x4 box filter
// the kernel has the following form
//   o o o o
//   o o o o
//   o o x o
//   o o o o 
// where x marks the fragment position and the o marks a sampling point

struct boxFilterIn
{
	float4 position : SV_POSITION;
    float2 screenTC : TEXCOORD0;
};

float4 boxFilter_fp
(
	boxFilterIn input,

    uniform Texture2D sOcclusion : register(s0),
    uniform float4 screenSize,
    uniform float farClipDistance
) : SV_Target
{
    float color = 0;
    for (int x = -2; x < 2; x++)
    for (int y = -2; y < 2; y++)
    {
        color += sOcclusion.SampleLevel(g_samLinear, float2(input.screenTC.x + x * screenSize.z, input.screenTC.y + y * screenSize.w), 0).x;
    }
    color /= 16;
        
    return float4(color.xxx, 1);
}


// a very simple and slightly dumb depth aware 4x4 box filter
// the kernel has the following form
//   o o o o
//   o o o o
//   o o x o
//   o o o o 
// where x marks the fragment position and the o marks a sampling point
float4 smartBoxFilter_fp
(
	boxFilterIn input,
    
    uniform Texture2D sMrt1 : register(s0), // normals + depth
    uniform Texture2D sOcclusion : register(s1),
    uniform float4 screenSize,
    uniform float farClipDistance
) : SV_Target
{
    float fragmentDepth = sMrt1.Sample(g_samLinear, input.screenTC).x;

    float color = 0;
    float weight = 0;
    for (int x = -2; x < 2; x++)
    for (int y = -2; y < 2; y++)
    {
        float sampleDepth = sMrt1.SampleLevel(g_samLinear, float2(input.screenTC.x + x * screenSize.z, input.screenTC.y + y * screenSize.w), 0).x;
        float dist = abs(fragmentDepth - sampleDepth) * farClipDistance + 0.5;
        float sampleWeight = 1 / (pow(dist, 1) + 1);
        color += sampleWeight * sOcclusion.SampleLevel(g_samLinear, float2(input.screenTC.x + x * screenSize.z, input.screenTC.y + y * screenSize.w), 0).x;
        weight += sampleWeight;
    }
    color /= weight;
        
    return float4(color.xxx, 1);
//    oColor0 = float4(tex2D(sOcclusion, screenTC).www, 1);
}


// cross bilateral filter
// gaussian blur with photometric weighting
// note: encode the viewspace z component in the accessibility texture to reduce
// the texture fetch count
float4 crossBilateralFilterX_fp
(
	noFilterIn input,
    
    uniform Texture2D sAccessibility : register(s0),
    uniform Texture2D sMRT2 : register(s1), // the view space position, xyz
    
    uniform float stepX, // inverse viewport width
    uniform float cPhotometricExponent
) : SV_Target
{
    const int kernelWidth = 13;
    float sigma = (kernelWidth - 1) / 6; // make the kernel span 6 sigma
    
    float fragmentDepth = sMRT2.Sample(g_samLinear, input.uv).z;

    float weights = 0;
    float blurred = 0;
    
    for (float i = -(kernelWidth - 1) / 2; i < (kernelWidth - 1) / 2; i++)
    {
        float geometricWeight = exp(-pow(i, 2) / (2 * pow(sigma, 2)));
        float sampleDepth = sMRT2.SampleLevel(g_samLinear, float2(input.uv.x - i * stepX, input.uv.y), 0).z;
        float photometricWeight = 1 / pow((1 + abs(fragmentDepth - sampleDepth)), cPhotometricExponent);

        weights += (geometricWeight * photometricWeight);
        blurred += sAccessibility.SampleLevel(g_samLinear, float2(input.uv.x - i * stepX, input.uv.y), 0).r * geometricWeight * photometricWeight;
    }

    blurred /= weights;
    return float4(blurred.xxx, 1);
}

float4 crossBilateralFilterY_fp
(
	noFilterIn input,
    
    uniform Texture2D sAccessibility : register(s0),
    uniform Texture2D sMRT2 : register(s1), // the view space position, xyz
    
    uniform float stepY, // inverse viewport width
    uniform float cPhotometricExponent
) : SV_Target
{
    const int kernelWidth = 13;
    float sigma = (kernelWidth - 1) / 6; // make the kernel span 6 sigma
    
    float fragmentDepth = sMRT2.Sample(g_samLinear, input.uv).z;

    float weights = 0;
    float blurred = 0;
    
    for (float i = -(kernelWidth - 1) / 2; i < (kernelWidth - 1) / 2; i++)
    {
        float geometricWeight = exp(-pow(i, 2) / (2 * pow(sigma, 2)));
        float sampleDepth = sMRT2.SampleLevel(g_samLinear, float2(input.uv.x, input.uv.y - i * stepY), 0).z;
        float photometricWeight = 1 / pow((1 + abs(fragmentDepth - sampleDepth)), cPhotometricExponent);
        
        weights += (geometricWeight * photometricWeight);
        blurred += sAccessibility.SampleLevel(g_samLinear, float2(input.uv.x, input.uv.y - i * stepY), 0).r * geometricWeight * photometricWeight;
    }

    blurred /= weights;
    return float4(blurred.xxx, 1);
}