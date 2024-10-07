// Adapted from "Sky and Ground" https://www.shadertoy.com/view/4sKGWt
// by Morgan McGuire, @CasualEffects
// License: BSD

layout(early_fragment_tests) in;

float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

float noise(vec2 x) {
    vec2 i = floor(x), f = fract(x);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 p) {
    const mat2 m2 = mat2(0.8, -0.6, 0.6, 0.8);
    
    float f = 0.5000 * noise(p); p = m2 * p * 2.02;
    f += 0.2500 * noise(p); p = m2 * p * 2.03;
    f += 0.1250 * noise(p); p = m2 * p * 2.01;
    f += 0.0625 * noise(p);
    return f / 0.9375;
}

vec3 render(in vec3 light, in vec3 ro, in vec3 rd) {
    vec3 col;
    
    
    // Sky with haze
    col = vec3(0.3, 0.55, 0.8) * (1.0 - 0.8 * -rd.y) * 0.9;
    
    // Sun
//    float sundot = clamp(dot(rd, light), 0.0, 1.0);
//    col += 0.25 * vec3(1.0, 0.7, 0.4) * pow(sundot, 8.0);
//    col += 0.75 * vec3(1.0, 0.8, 0.5) * pow(sundot, 64.0);
       
    if (rd.y > 0){  // Don't render clouds below the horizon line
        // Clouds
        col = mix(col, vec3(1.0, 0.95, 1.0), 0.5 *
                  smoothstep(0.5, 0.8, fbm((ro.xz + rd.xz * (250000.0 - ro.y) / rd.y) * 0.000008)));
    }
        
    
    // Horizon/atmospheric perspective
    col = mix(col, vec3(0.7, 0.75, 0.8), pow(1.0 - max(abs(rd.y), 0.0), 8.0));
    
    return col;
}


SkyboxFragmentOut frag(SkyboxInput params)
{
    const vec3 ro = constants.camPos;
    const vec3 rd = params.skyRay;
        
    vec3 light = normalize(vec3(-0.8,0.3,-0.3));

    vec3 col = render(light, ro, rd);
    
    SkyboxFragmentOut fs_out;
    
    fs_out.color = vec4(col,1);
    
    return fs_out;
}
