//calculate sampling positions using fragment pos and view dimensions
	vec2 texcoord = vec2(gl_FragCoord.x / ubo.viewRect[2], gl_FragCoord.y / ubo.viewRect[3]);

    float sampledDepthForPos = texture(sampler2D(t_depth,g_sampler), texcoord).x;
    vec2 viewTexcoord = (gl_FragCoord.xy - ubo.viewRegion.xy) / ubo.viewRegion.zw;
    mat4 invViewProj = mat4(invViewProj_elts[0],invViewProj_elts[1],invViewProj_elts[2],invViewProj_elts[3]);
    vec3 sampledPos = ComputeWorldSpacePos(viewTexcoord,sampledDepthForPos, invViewProj);


    // get all the PBR stuff
    vec3 albedo = texture(sampler2D(t_albedo,g_sampler), texcoord).xyz;   
    vec3 normal = texture(sampler2D(t_normal,g_sampler), texcoord).xyz;
    vec4 roughnessSpecularMetallicAO = texture(sampler2D(t_roughnessSpecularMetallicAO,g_sampler), texcoord);
    float roughness = roughnessSpecularMetallicAO.x;
    float specular = roughnessSpecularMetallicAO.y;
    float metallic = roughnessSpecularMetallicAO.z;
    float AO = roughnessSpecularMetallicAO.w;