
void writeTransparency(inout vec4 outcolor){
    #if RVE_TRANSPARENT
    // adapted from: https://learnopengl.com/Guest-Articles/2020/OIT/Weighted-Blended
    float weight = max(min(1.0, max(max(outcolor.r, outcolor.g), outcolor.b) * outcolor.a), outcolor.a) * clamp(0.03 / (1e-5 + pow(clipSpaceZ / 200, 4.0)), 1e-2, 3e3);

    // pre-multiplied 
    revealage = outcolor.a;
    outcolor = vec4(outcolor.rgb * outcolor.a, outcolor.a) * weight;
    #endif
}