
layout(location = 0) in vec3 inNormal;

layout(location = 0) out vec4 outcolor;
layout(location = 1) out vec4 outnormal;
void main()
{
	outcolor = vec4(1,1,1,1);
	outnormal = vec4(inNormal, 1);
}

