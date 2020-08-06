@insertpiece( SetCrossPlatformSettings )
@insertpiece( SetCompatibilityLayer )

layout(std140) uniform;
#define FRAG_COLOR		0

@property( hlms_vpos )
in vec4 gl_FragCoord;
@end

@property( !hlms_shadowcaster )
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;
@end @property( hlms_shadowcaster )
layout(location = FRAG_COLOR, index = 0) out float outColour;
@end

// START UNIFORM DECLARATION
@insertpiece( custom_ps_uniformDeclaration )
// END UNIFORM DECLARATION
@property( !hlms_shadowcaster || !hlms_shadow_uses_depth_texture || exponential_shadow_maps )
	in block
	{
		@insertpiece( VStoPS_block )
	} inPs;
@end

@property( !hlms_shadowcaster || alpha_test )
	@foreach( num_textures, n )
		@property( is_texture@n_array )
			uniform sampler2DArray textureMapsArray@n;
		@else
			uniform sampler2D textureMaps@n;
		@end
	@end
@end

@insertpiece( DefaultHeaderPS )

@property( hlms_shadowcaster )
	@insertpiece( DeclShadowCasterMacros )
@end

void main()
{
	@insertpiece( custom_ps_preExecution )
	@property( !hlms_shadowcaster || alpha_test )
		@insertpiece( DefaultBodyPS )
	@end
	@property( hlms_shadowcaster )
		@insertpiece( DoShadowCastPS )
	@end
	@insertpiece( custom_ps_posExecution )
}
