

LitOutput frag(){
    LitOutput fs_out;

    fs_out.color = vec4(0,0,0,1);
    fs_out.normal = vec3(0,0,0);
    fs_out.roughness = 0;
    fs_out.specular = 0;
    fs_out.metallic = 0;
    fs_out.ao = 0;

    return fs_out;
}