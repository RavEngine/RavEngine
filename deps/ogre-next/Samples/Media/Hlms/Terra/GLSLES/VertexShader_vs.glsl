@insertpiece( SetCrossPlatformSettings )
@insertpiece( SetCompatibilityLayer )

@property( GL3+ )
out gl_PerVertex
{
	vec4 gl_Position;
};
@end

layout(std140) uniform;

//To render a 2x2 (quads) terrain:
//You'll normally need 6 vertices per line + 2 for degenerates.
//You'll need 8 vertices per line.
//So you'll need a total of 16 vertices.

//To render a 4x2 (quads) terrain:
//You'll need 10 vertices per line.
//If we include degenerate vertices, you'll need 12 per line
//So you'll need a total of 24 vertices.
//in int gl_VertexID;

@property( GL_ARB_base_instance )
	in uint drawId;
@end

@insertpiece( custom_vs_attributes )

out block
{
@insertpiece( Terra_VStoPS_block )
} outVs;

// START UNIFORM DECLARATION
@insertpiece( PassDecl )
@insertpiece( TerraInstanceDecl )
uniform sampler2D heightMap;
@insertpiece( custom_vs_uniformDeclaration )
@property( !GL_ARB_base_instance )uniform uint baseInstance;@end
// END UNIFORM DECLARATION

@piece( VertexTransform )
	//Lighting is in view space
	outVs.pos		= ( vec4(worldPos.xyz, 1.0f) * passBuf.view ).xyz;
@property( !hlms_dual_paraboloid_mapping )
	gl_Position = vec4(worldPos.xyz, 1.0f) * passBuf.viewProj;@end
@property( hlms_dual_paraboloid_mapping )
	//Dual Paraboloid Mapping
	gl_Position.w	= 1.0f;
	gl_Position.xyz	= outVs.pos;
	float L = length( gl_Position.xyz );
	gl_Position.z	+= 1.0f;
	gl_Position.xy	/= gl_Position.z;
	gl_Position.z	= (L - NearPlane) / (FarPlane - NearPlane);@end
@end

void main()
{
@property( !GL_ARB_base_instance )
    uint drawId = baseInstance + uint( gl_InstanceID );
@end

    @insertpiece( custom_vs_preExecution )

    CellData cellData = instance.cellData[drawId];

	//Map pointInLine from range [0; 12) to range [0; 9] so that it reads:
	// 0 0 1 2 3 4 5 6 7 8 9 9
	uint pointInLine = uint(gl_VertexID) % (cellData.numVertsPerLine.x); //cellData.numVertsPerLine.x = 12
	pointInLine = uint(clamp( int(pointInLine) - 1, 0, int(cellData.numVertsPerLine.x - 3u) ));

	uvec2 uVertexPos;

	uVertexPos.x = pointInLine >> 1u;
    //Even numbers are the next line, odd numbers are current line.
	uVertexPos.y = (pointInLine & 0x01u) == 0u ? 1u : 0u;
	uVertexPos.y += uint(gl_VertexID) / cellData.numVertsPerLine.x;
	//uVertexPos.y += floor( (float)gl_VertexID / (float)cellData.numVertsPerLine ); Could be faster on GCN.

@property( use_skirts )
	//Apply skirt.
	bool isSkirt =( pointInLine.x <= 1u ||
					pointInLine.x >= (cellData.numVertsPerLine.x - 4u) ||
					uVertexPos.y == 0u ||
					uVertexPos.y == (cellData.numVertsPerLine.z + 2u) );

	//Now shift X position for the left & right skirts
	uVertexPos.x = uint( max( int(uVertexPos.x) - 1, 0 ) );
	uVertexPos.x = min( uVertexPos.x, ((cellData.numVertsPerLine.x - 7u) >> 1u) );
	// uVertexPos.x becomes:
	// 0 0 0 1 1 2 2 3 3 4 4 4
	// 0 0 0 0 0 1 1 2 2 3 3 3
	// 0 0 0 0 0 1 1 2 2 2 2 2

	//Now shift Y position for the front & back skirts
	uVertexPos.y = uint( max( int(uVertexPos.y) - 1, 0 ) );
	uVertexPos.y = min( uVertexPos.y, cellData.numVertsPerLine.z );
@end

	uint lodLevel = cellData.numVertsPerLine.y;
	uVertexPos = uVertexPos << lodLevel;

	uVertexPos.xy = uvec2( clamp( ivec2(uVertexPos.xy) + cellData.xzTexPosBounds.xy,
                           ivec2( 0, 0 ), cellData.xzTexPosBounds.zw ) );

    vec3 worldPos;
	worldPos.y = texelFetch( heightMap, ivec2( uVertexPos.xy ), 0 ).x;
@property( use_skirts )
	worldPos.y = isSkirt ? uintBitsToFloat(cellData.numVertsPerLine.w) : worldPos.y;
@end
	worldPos.xz = uVertexPos.xy;
    worldPos.xyz = worldPos.xyz * cellData.scale.xyz + cellData.pos.xyz;

	@insertpiece( VertexTransform )

	outVs.uv0.xy = vec2( uVertexPos.xy ) * vec2( cellData.pos.w, cellData.scale.w );

	@insertpiece( DoShadowReceiveVS )

@property( hlms_pssm_splits )	outVs.depth = gl_Position.z;@end

	//outVs.drawId = drawId;

	@insertpiece( custom_vs_posExecution )
}
