// GLSL shader version 1.40 (for OpenGL 3.1)
#version 140

// Vertex attributes (these names must match our vertex format attributes)
in vec2 position;
in vec3 color;

// Vertex output to the fragment shader
out vec3 vertexColor;

// Vertex shader main function
void main()
{
	gl_Position = vec4(position, 0, 1);
	vertexColor = color;
}
