layout(location = 0) out vec4 outcolor;

layout(early_fragment_tests) in;
void main()
{
    outcolor = vec4(0,181/255.0,226/256.0,0);
}
