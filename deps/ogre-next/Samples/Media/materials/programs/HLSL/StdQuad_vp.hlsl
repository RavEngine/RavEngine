cbuffer MatrixBuffer
{
	matrix worldViewProj;
};

struct VS_OUTPUT {
    float4 Pos : SV_Position;
    float2 texCoord : TEXCOORD0;
};

struct VS_OUTPUT2 {
    float4 Pos : SV_Position;
    float2 texCoord : TEXCOORD0;
	float2 texCoord2 : TEXCOORD1;
};

struct VS_OUTPUT3 {
    float4 Pos : SV_Position;
    float2 texCoord : TEXCOORD0;
	float2 texCoord2 : TEXCOORD1;
	float2 texCoord3 : TEXCOORD2;
};

struct VS_OUTPUT4 {
    float4 Pos : SV_Position;
    float2 texCoord : TEXCOORD0;
	float2 texCoord2 : TEXCOORD1;
	float2 texCoord3 : TEXCOORD2;
	float2 texCoord4 : TEXCOORD3;
};

VS_OUTPUT StdQuad_vp
(
    float4 inPos : POSITION,
	float2 inUv0 : TEXCOORD0
)
{
	VS_OUTPUT Out;
    // Use standardise transform, so work accord with render system specific (RS depth, requires texture flipping, etc)
	inPos.w = 1.0f;
    Out.Pos = mul(worldViewProj, inPos);

    // Convert to image-space
    Out.texCoord = inUv0;
	
	return Out;		
}

VS_OUTPUT2 StdQuad_Tex2_vp
(
    float4 inPos : POSITION,
	float2 inUv0 : TEXCOORD0
)
{
	VS_OUTPUT2 Out;
    // Use standardise transform, so work accord with render system specific (RS depth, requires texture flipping, etc)
	inPos.w = 1.0f;
    Out.Pos = mul(worldViewProj, inPos);

    // The input positions adjusted by texel offsets, so clean up inaccuracies
    inPos.xy = sign(inPos.xy);

    // Convert to image-space
    Out.texCoord = inUv0;
    Out.texCoord2 = inUv0;
	
	return Out;		
}

VS_OUTPUT2 StdQuad_Tex2a_vp
(
    float4 inPos : POSITION,
	float2 inUv0 : TEXCOORD0
)
{
	VS_OUTPUT2 Out;
    // Use standardise transform, so work accord with render system specific (RS depth, requires texture flipping, etc)
	inPos.w = 1.0f;
    Out.Pos = mul(worldViewProj, inPos);

    // The input positions adjusted by texel offsets, so clean up inaccuracies
    inPos.xy = sign(inPos.xy);

    // Convert to image-space
    Out.texCoord = inUv0;
    Out.texCoord2 = inPos.xy;
	
	return Out;	
}

VS_OUTPUT3 StdQuad_Tex3_vp
(
    float4 inPos : POSITION,
	float2 inUv0 : TEXCOORD0
)
{
	VS_OUTPUT3 Out;
    // Use standardise transform, so work accord with render system specific (RS depth, requires texture flipping, etc)
	inPos.w = 1.0f;
    Out.Pos = mul(worldViewProj, inPos);

    // Convert to image-space
    Out.texCoord = inUv0;
    Out.texCoord2 = inUv0;
    Out.texCoord3 = inUv0;
	
	return Out;	
}

VS_OUTPUT4 StdQuad_Tex4_vp
(
    float4 inPos : POSITION,
	float2 inUv0 : TEXCOORD0
)
{
	VS_OUTPUT4 Out;
    // Use standardise transform, so work accord with render system specific (RS depth, requires texture flipping, etc)
	inPos.w = 1.0f;
    Out.Pos = mul(worldViewProj, inPos);

    // Convert to image-space
    Out.texCoord = inUv0;
    Out.texCoord2 = inUv0;
    Out.texCoord3 = inUv0;
    Out.texCoord4 = inUv0;
	
	return Out;	
}
