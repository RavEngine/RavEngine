#include <metal_stdlib>
using namespace metal;

struct VS_INPUT
{
	float4 position [[attribute(VES_POSITION)]];
	float3 normal	[[attribute(VES_NORMAL)]];
};

struct PS_INPUT
{
	float3 cameraDir;

	float4 gl_Position [[position]];
};

struct Params
{
	float2 rsDepthRange;
	float4x4 worldViewProj;
};

vertex PS_INPUT main_metal
(
	VS_INPUT input [[stage_in]],
	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
	PS_INPUT outVs;

	outVs.gl_Position.xy= ( p.worldViewProj * float4( input.position.xy, 0.0f, 1.0f ) ).xy;
	outVs.gl_Position.z	= p.rsDepthRange.y;
	outVs.gl_Position.w	= 1.0f;
	outVs.cameraDir		= input.normal;

	return outVs;
}
