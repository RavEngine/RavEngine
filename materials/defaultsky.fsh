// Adapted from "Volumetric Overcast Clouds" https://www.shadertoy.com/view/Xttcz2
// By Valentingalea
// License: MIT

layout(early_fragment_tests) in;
#define u_time 0

#define SHADERTOY

//
// Volumetric Clouds Experiment
//
//                 _                                  
//               (`  ).                   _           
//              (     ).              .:(`  )`.       
// )           _(       '`.          :(   .    )      
//         .=(`(      .   )     .--  `.  (    ) )      
//        ((    (..__.:'-'   .+(   )   ` _`  ) )                 
// `.     `(       ) )       (   .  )     (   )  ._   
//   )      ` __.:'   )     (   (   ))     `-'.-(`  ) 
// )  )  ( )       --'       `- __.'         :(      )) 
// .-'  (_.'          .')                    `(    )  ))
//                   (_  )                     ` __.:'          
//                                         
// --..,___.--,--'`,---..-.--+--.,,-,,..._.--..-._.-a:f--.
//
// A mashup of ideas from different sources:
// * Magnus Wrenninge - Production Volume Rendering 
//	 http://magnuswrenninge.com/productionvolumerendering
// * Andrew Schneider - The Real-time Volumetric Cloudscapes of Horizon: Zero Dawn
//   http://advances.realtimerendering.com/s2015/The%20Real-time%20Volumetric%20Cloudscapes%20of%20Horizon%20-%20Zero%20Dawn%20-%20ARTR.pdf
// * Scratchapixel - Simulating the Colors of the Sky
//   https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky
// * Ian McEwan, Ashima Arts - Array and textureless GLSL 2D/3D/4D simplex 
//   https://github.com/ashima/webgl-noise
// * and of course lots of iteration and tweaking
//   https://github.com/valentingalea/shaderbox
//	
#define SHADERTOY

#define _in(T) const in T
#define _inout(T) inout T
#define _out(T) out T
#define _begin(type) type (
#define _end )
#define _mutable(T) T
#define _constant(T) const T
#define mul(a, b) (a) * (b)
precision mediump float;


#define PI 3.14159265359

struct ray_t {
	vec3 origin;
	vec3 direction;
};
#define BIAS 1e-4 // small offset to avoid self-intersections

struct sphere_t {
	vec3 origin;
	float radius;
	int material;
};

struct plane_t {
	vec3 direction;
	float distance;
	int material;
};

struct hit_t {
	float t;
	int material_id;
	vec3 normal;
	vec3 origin;
};
#define max_dist 1e8
_constant(hit_t) no_hit = _begin(hit_t)
	float(max_dist + 1e1), // 'infinite' distance
	-1, // material id
	vec3(0., 0., 0.), // normal
	vec3(0., 0., 0.) // origin
_end;

// ----------------------------------------------------------------------------
// Various 3D utilities functions
// ----------------------------------------------------------------------------

ray_t get_primary_ray(
	_in(vec3) cam_local_point,
	_inout(vec3) cam_origin,
	_inout(vec3) cam_look_at
){
	vec3 fwd = normalize(cam_look_at - cam_origin);
	vec3 up = vec3(0, 1, 0);
	vec3 right = cross(up, fwd);
	up = cross(fwd, right);

	ray_t r = _begin(ray_t)
		cam_origin,
		normalize(fwd + up * cam_local_point.y + right * cam_local_point.x)
	_end;
	return r;
}

_constant(mat3) mat3_ident = mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);


mat2 rotate_2d(
	_in(float) angle_degrees
){
	float angle = radians(angle_degrees);
	float _sin = sin(angle);
	float _cos = cos(angle);
	return mat2(_cos, -_sin, _sin, _cos);
}

mat3 rotate_around_z(
	_in(float) angle_degrees
){
	float angle = radians(angle_degrees);
	float _sin = sin(angle);
	float _cos = cos(angle);
	return mat3(_cos, -_sin, 0, _sin, _cos, 0, 0, 0, 1);
}

mat3 rotate_around_y(
	_in(float) angle_degrees
){
	float angle = radians(angle_degrees);
	float _sin = sin(angle);
	float _cos = cos(angle);
	return mat3(_cos, 0, _sin, 0, 1, 0, -_sin, 0, _cos);
}

mat3 rotate_around_x(
	_in(float) angle_degrees
){
	float angle = radians(angle_degrees);
	float _sin = sin(angle);
	float _cos = cos(angle);
	return mat3(1, 0, 0, 0, _cos, -_sin, 0, _sin, _cos);
}

// http://http.developer.nvidia.com/GPUGems3/gpugems3_ch24.html
vec3 linear_to_srgb(
	_in(vec3) color
){
	const float p = 1. / 2.2;
	return vec3(pow(color.r, p), pow(color.g, p), pow(color.b, p));
}
vec3 srgb_to_linear(
	_in(vec3) color
){
	const float p = 2.2;
	return vec3(pow(color.r, p), pow(color.g, p), pow(color.b, p));
}

#ifdef __cplusplus
vec3 faceforward(
	_in(vec3) N,
	_in(vec3) I,
	_in(vec3) Nref
){
	return dot(Nref, I) < 0 ? N : -N;
}
#endif

float checkboard_pattern(
	_in(vec2) pos,
	_in(float) scale
){
	vec2 pattern = floor(pos * scale);
	return mod(pattern.x + pattern.y, 2.0);
}

float band (
	_in(float) start,
	_in(float) peak,
	_in(float) end,
	_in(float) t
){
	return
	smoothstep (start, peak, t) *
	(1. - smoothstep (peak, end, t));
}

// from https://www.shadertoy.com/view/4sSSW3
// original http://orbit.dtu.dk/fedora/objects/orbit:113874/datastreams/file_75b66578-222e-4c7d-abdf-f7e255100209/content
void fast_orthonormal_basis(
	_in(vec3) n,
	_out(vec3) f,
	_out(vec3) r
){
	float a = 1. / (1. + n.z);
	float b = -n.x*n.y*a;
	f = vec3(1. - n.x*n.x*a, b, -n.x);
	r = vec3(b, 1. - n.y*n.y*a, -n.y);
}


// ----------------------------------------------------------------------------
// Volumetric utilities
// ----------------------------------------------------------------------------

float isotropic_phase_func(float mu)
{
	return
	           1.
	/ //-------------------
	        4. * PI;
}

float rayleigh_phase_func(float mu)
{
	return
	        3. * (1. + mu*mu)
	/ //------------------------
	           (16. * PI);
}

float henyey_greenstein_phase_func(float mu)
{
	// Henyey-Greenstein phase function factor [-1, 1]
	// represents the average cosine of the scattered directions
	// 0 is isotropic scattering
	// > 1 is forward scattering, < 1 is backwards
	const float g = 0.76;

	return
	                     (1. - g*g)
	/ //---------------------------------------------
	     ((4. + PI) * pow(1. + g*g - 2.*g*mu, 1.5));
}

float schlick_phase_func(float mu)
{
	// Schlick Phase Function factor
	// Pharr and  Humphreys [2004] equivalence to g from Henyey-Greenstein
	const float g = 0.76;
	const float k = 1.55*g - 0.55 * (g*g*g);

	return
	                  (1. - k*k)
	/ //-------------------------------------------
	     (4. * PI * (1. + k*mu) * (1. + k*mu));
}

struct volume_sampler_t {
	vec3 origin; // start of ray
	vec3 pos; // current pos of acccumulation ray
	float height;

	float coeff_absorb;
	float T; // transmitance

	vec3 C; // color
	float alpha;
};

volume_sampler_t begin_volume(
	_in(vec3) origin,
	_in(float) coeff_absorb
){
	volume_sampler_t v = _begin(volume_sampler_t)
		origin, origin, 0.,
		coeff_absorb, 1.,
		vec3(0., 0., 0.), 0.
	_end;
	return v;
}

float illuminate_volume(
	_inout(volume_sampler_t) vol,
	_in(vec3) V,
	_in(vec3) L
);

void integrate_volume(
	_inout(volume_sampler_t) vol,
	_in(vec3) V,
	_in(vec3) L,
	_in(float) density,
	_in(float) dt
){
	// change in transmittance (follows Beer-Lambert law)
	float T_i = exp(-vol.coeff_absorb * density * dt);
	// Update accumulated transmittance
	vol.T *= T_i;
	// integrate output radiance (here essentially color)
	vol.C += vol.T * illuminate_volume(vol, V, L) * density * dt;
	// accumulate opacity
	vol.alpha += (1. - T_i) * (1. - vol.alpha);
}


#define cld_march_steps (32)
#define cld_coverage (.3125)
#define cld_thick (70.)
#define cld_absorb_coeff (1.)
#define cld_wind_dir vec3(0, 0, -u_time * .2)
#define cld_sun_dir normalize(vec3(0, 0/*abs(sin(u_time * .3))*/, -1))
_mutable(float) coverage_map;

// ----------------------------------------------------------------------------
// Noise function by iq from https://www.shadertoy.com/view/4sfGzS
// ----------------------------------------------------------------------------

float hash(
	_in(float) n
){
	return fract(sin(n)*753.5453123);
}

float noise_iq(
	_in(vec3) x
){
	vec3 p = floor(x);
	vec3 f = fract(x);
	f = f*f*(3.0 - 2.0*f);

#if 1
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+157.0), hash(n+158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
#else
	vec2 uv = (p.xy + vec2(37.0, 17.0)*p.z) + f.xy;
	vec2 rg = textureLod( iChannel0, (uv+.5)/256., 0.).yx;
	return mix(rg.x, rg.y, f.z);
#endif
}

#define gnoise(x) noise_iq(x)

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0, 0, 0, 0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }
#define noise(x) snoise(x)

// ----------------------------------------------------------------------------
// Fractional Brownian Motion
// depends on custom basis function
// ----------------------------------------------------------------------------

#define DECL_FBM_FUNC(_name, _octaves, _basis) float _name(_in(vec3) pos, _in(float) lacunarity, _in(float) init_gain, _in(float) gain) { vec3 p = pos; float H = init_gain; float t = 0.; for (int i = 0; i < _octaves; i++) { t += _basis * H; p *= lacunarity; H *= gain; } return t; }

DECL_FBM_FUNC(fbm, 4, noise(p))
DECL_FBM_FUNC(fbm_clouds, 5, abs(noise(p)))

vec3 render_sky_color(
	_in(vec3) eye_dir
){
	_constant(vec3) sun_color = vec3(1., .7, .55);
	float sun_amount = 0;

	vec3 sky = mix(vec3(.0, .1, .4), vec3(.3, .6, .8), 1.0 - eye_dir.y);
	sky += sun_color * min(pow(sun_amount, 1500.0) * 5.0, 1.0);
	sky += sun_color * min(pow(sun_amount, 10.0) * .6, 1.0);

	return sky;
}

float density_func(
	_in(vec3) pos,
	_in(float) h
){
	vec3 p = pos * .001 + cld_wind_dir;
	float dens = fbm_clouds(p * 2.032, 2.6434, .5, .5);
	
	dens *= smoothstep (cld_coverage, cld_coverage + .035, dens);

	//dens *= band(.2, .3, .5 + coverage_map * .5, h);

	return dens;
}

float illuminate_volume(
	_inout(volume_sampler_t) cloud,
	_in(vec3) V,
	_in(vec3) L
){
	return exp(cloud.height) / 1.95;
}

vec4 render_clouds(
	_in(ray_t) eye
){
	const int steps = cld_march_steps;
	const float march_step = cld_thick / float(steps);

	vec3 projection = eye.direction / eye.direction.y;
	vec3 iter = projection * march_step;

	float cutoff = dot(eye.direction, vec3(0, 1, 0));

	volume_sampler_t cloud = begin_volume(
		eye.origin + projection * 100.,
		cld_absorb_coeff);

	//coverage_map = gnoise(projection);
	//return vec4(coverage_map, coverage_map, coverage_map, 1);

	for (int i = 0; i < steps; i++) {
		cloud.height = (cloud.pos.y - cloud.origin.y)
			/ cld_thick;
		float dens = density_func(cloud.pos, cloud.height);

		integrate_volume(
			cloud,
			eye.direction, cld_sun_dir,
			dens, march_step);

		cloud.pos += iter;

		if (cloud.alpha > .999) break;
	}

	return vec4(cloud.C, cloud.alpha * smoothstep(.0, .2, cutoff));
}

void setup_camera(
	_inout(vec3) eye,
	_inout(vec3) look_at
){
	eye = vec3(0, 1., 0);
	look_at = vec3(0, 1.6, -1);
}

void setup_scene()
{
}

vec3 render_impl(
	_in(ray_t) eye_ray
){
	vec3 sky = render_sky_color(eye_ray.direction);
	if (dot(eye_ray.direction, vec3(0, 1, 0)) < 0.05) return sky;

	vec4 cld = render_clouds(eye_ray);
	vec3 col = mix(sky, cld.rgb, cld.a);

	return col;
}

#define FOV 1. // 45 degrees
// ----------------------------------------------------------------------------
// Main Rendering function
// depends on external defines: FOV
// ----------------------------------------------------------------------------

vec3 render(
	vec3 ro,
	vec3 rd
){

	ray_t ray;
	ray.origin = ro;
	ray.direction = rd;

	vec3 color = render_impl(ray);

	color = linear_to_srgb(color);

	return color;
}




SkyboxFragmentOut frag(SkyboxInput params)
{
    const vec3 ro = constants.camPos;
    const vec3 rd = params.skyRay;
        
    SkyboxFragmentOut fs_out;

    vec3 col = render(vec3(0,0,0), rd);

    fs_out.color = vec4(col.xyz,1);
    
    return fs_out;
}
