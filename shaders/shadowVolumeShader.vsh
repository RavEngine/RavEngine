$input a_position
$output plane1, plane2, planeCap, planeData

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>

BUFFER_RO(vertbuffer,float,12);
BUFFER_RO(indbuffer,int,13);
BUFFER_RO(light_databuffer,float,14);	
uniform vec4 NumObjects;		// x = start index into light_databuffer, y = number of total instances, z = number of lights shadows are being calculated for

vec3 calcNormal(vec3 u, vec3 v){

	return normalize(vec3(
		u.y * v.z - u.z * v.y,
		u.z * v.x - u.x * v.z,
		u.x * v.y - u.y * v.x
	));
}

vec4 genPlane(vec3 a, vec3 b, vec3 c){

	// get the coefficients
	float A = (b.y-a.y)*(c.z-a.z) - (c.y-a.y)*(b.z-a.z);
	float B = (b.z-a.z)*(c.x-a.x) - (c.z-a.z)*(b.x-a.x);
	float C = (b.x-a.x)*(c.x-a.y) - (c.x-a.x)*(b.y-a.y);
	float D = A*a.x + B*a.y + C*a.z;

	return vec4(A,B,C,D);
}

void main()
{
	// get the assigned triangle
	int index = indbuffer[gl_InstanceID * 3];
	vec3 points[3];

	points[0] = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);
	index = indbuffer[gl_InstanceID * 3 + 1];
	points[1] = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);
	index = indbuffer[gl_InstanceID * 3 + 2];
	points[2] = vec3(vertbuffer[index*3],vertbuffer[index*3+1],vertbuffer[index*3+2]);

	// TODO: calculate the vector to translate the higher-order vertices along
	// for directional light, this is just the light's forward vector
	// for spot and point lights, this is the vector from the center of the light to the vertex being processed

	vec3 normal = calcNormal(points[0] - points[1], points[0] - points[2]);

	//TODO: this is the directional light implementation, add ifdefs for the other light types
	index = (gl_InstanceID / (int)NumObjects.y) * 3;
	vec3 toLight = vec3(light_databuffer[index],light_databuffer[index+1],light_databuffer[index+2]);
	vec3 dirvec = toLight * -30;

	// if the triangle is facing the wrong way, we don't want to have it cast shadows (reduce the number of volumes generated)
	float nDotL = max(dot(normal, toLight), 0);
	if (nDotL < 0.2){
		gl_Position = vec4(0,0,0,1);	// don't shade this by placing it at the origin
	}
	else{
		float INSET = -0.02;
		// inset the triangle by the normal a small amount, then extend it along the light ray
		gl_Position = mul(u_viewProj, vec4((points[gl_VertexID % 3] + normal * INSET) + dirvec * (gl_VertexID < 3 ? 0 : 1),1));

		// write positions
		vec4 vp0 = mul(u_viewProj, vec4(points[0] + normal * INSET,1));
		vec4 vp1 = mul(u_viewProj, vec4(points[1] + normal * INSET,1));
		vec4 vp2 = mul(u_viewProj, vec4(points[2] + normal * INSET,1));

		vec4 vp3 = mul(u_viewProj, vec4((points[0] + normal * INSET) + dirvec, 1));
		vec4 vp4 = mul(u_viewProj, vec4((points[1] + normal * INSET) + dirvec, 1));
		vec4 vp5 = mul(u_viewProj, vec4((points[2] + normal * INSET) + dirvec, 1));

		vp0 = vec4(vp0.xyz / vp0.w,vp0.w);
		vp1 = vec4(vp1.xyz / vp1.w,vp1.w);
		vp2 = vec4(vp2.xyz / vp2.w,vp2.w);
		vp3 = vec4(vp3.xyz / vp3.w,vp3.w);
		vp4 = vec4(vp4.xyz / vp4.w,vp4.w);
		vp5 = vec4(vp5.xyz / vp5.w,vp5.w);

		// always include the top cap
		planeCap = genPlane(vp0.xyz,vp1.xyz,vp2.xyz);

		// TODO: fix -- top-face vert to 2 end verts, for each
		vec3 side1Normal = calcNormal(vp0.xyz - vp3.xyz, vp0.xyz - vp4.xyz);
		vec3 side2Normal = calcNormal(vp0.xyz - vp5.xyz, vp0.xyz - vp3.xyz);
		vec3 side3Normal = calcNormal(vp2.xyz - vp1.xyz, vp2.xyz - vp4.xyz);

		// which planes are backplanes?
		bool oneIncluded = false;
		planeData.x = 0;				// x component stores the number of backplanes excluding the top cap
		if (side1Normal.z < 0){
			plane1 = genPlane(vp0.xyz,vp3.xyz,vp4.xyz);
			oneIncluded = true;
			planeData.x++;
		}
		if (side2Normal.z < 0){
			if (oneIncluded){
				plane2 = genPlane(vp5.xyz,vp3.xyz,vp0.xyz);
			}
			else{
				plane1 = genPlane(vp5.xyz,vp3.xyz,vp0.xyz);
			}
			oneIncluded = true;
			planeData.x++;
		}
		if (side3Normal.z < 0 && planeData.x < 2){	// only include this one if 2 more planes were not included
			if (oneIncluded){
				plane2 = genPlane(vp2.xyz,vp1.xyz,vp4.xyz);
			}
			else{
				plane1 = genPlane(vp2.xyz,vp1.xyz,vp4.xyz);
			}
			oneIncluded = true;
			planeData.x++;
		}
	}
}