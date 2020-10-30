$input v_normal, v_position

/*
 * Copyright 2011-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.sh"

void main()
{
	vec3 normal = normalize(v_normal);

	vec4 color = vec4(0.5,0.5,0.5,1.0);
	vec4 ambientColor = vec4(0.1,0.1,0.1,1);
	vec4 lightdir = vec4(u_view[2][0],u_view[2][1],u_view[2][2],0);

	gl_FragColor = mul(dot(normal, lightdir), color) + ambientColor;	
}
