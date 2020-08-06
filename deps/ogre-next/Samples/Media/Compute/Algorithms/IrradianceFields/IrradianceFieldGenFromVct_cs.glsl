@insertpiece( SetCrossPlatformSettings )
@insertpiece( DeclUavCrossPlatform )

#define ushort2 uvec2

@insertpiece( PreBindingsHeaderCS )

uniform samplerBuffer directionsBuffer;

uniform sampler3D vctProbe;

@property( vct_anisotropic )
	uniform sampler3D vctProbeX;
	uniform sampler3D vctProbeY;
	uniform sampler3D vctProbeZ;
@end

layout (@insertpiece(uav0_pf_type)) uniform restrict writeonly image2D irradianceField;
layout (@insertpiece(uav1_pf_type)) uniform restrict writeonly image2D irradianceFieldDepth;

shared float4 g_diffuseDepth[@value( threads_per_group_x ) * @value( threads_per_group_y ) * @value( threads_per_group_z )];

layout( local_size_x = @value( threads_per_group_x ),
		local_size_y = @value( threads_per_group_y ),
		local_size_z = @value( threads_per_group_z ) ) in;

@insertpiece( HeaderCS )

//in uvec3 gl_NumWorkGroups;
//in uvec3 gl_WorkGroupID;
//in uvec3 gl_LocalInvocationID;
//in uvec3 gl_GlobalInvocationID;
//in uint  gl_LocalInvocationIndex;

void main()
{
	@insertpiece( BodyCS )
}
