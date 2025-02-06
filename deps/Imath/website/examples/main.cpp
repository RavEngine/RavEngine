// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.

#include <iostream>

void color3_example();
void color4_example();
void euler_example();
void frustum_example();
void interval_example();
void line3_example();
void matrix22_example();
void matrix33_example();
void matrix44_example();
void plane3_example();
void quat_example();
void shear6_example();
void sphere3_example();
void vec2_example();
void vec3_example();
void vec4_example();
void half_example();

int
main (int argc, char* argv[])
{
    std::cout << "imath examples..." << std::endl;

    color3_example();
    color4_example();
    euler_example();
    frustum_example();
    interval_example();
    line3_example();
    matrix22_example();
    matrix33_example();
    matrix44_example();
    plane3_example();
    quat_example();
    shear6_example();
    sphere3_example();
    vec2_example();
    vec3_example();
    vec4_example();
    half_example();
    
    std::cout << "done." << std::endl;
    
    return 0;
}
