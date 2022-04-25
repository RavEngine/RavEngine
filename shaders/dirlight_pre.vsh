$input a_position

void main()
{
    gl_Position = vec4(a_position.xy, 1, 1.0);    //depth = 1
}
