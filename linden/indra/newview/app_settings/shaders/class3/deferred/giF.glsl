/** 
 * @file giF.glsl
 *
 * Copyright (c) 2007-$CurrentYear$, Linden Research, Inc.
 * $License$
 */

#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect depthMap;
uniform sampler2DRect normalMap;
uniform sampler2DRect lightMap;
uniform sampler2DRect specularRect;

uniform sampler2D noiseMap;

uniform sampler2D		diffuseGIMap;
uniform sampler2D		specularGIMap;
uniform sampler2D		normalGIMap;
uniform sampler2D		depthGIMap;

uniform sampler2D		lightFunc;

// Inputs
varying vec2 vary_fragcoord;

uniform vec2 screen_res;

uniform vec4 sunlight_color;

uniform mat4 inv_proj;
uniform mat4 gi_mat;  //gPipeline.mGIMatrix - eye space to sun space
uniform mat4 gi_mat_proj; //gPipeline.mGIMatrixProj - eye space to projected sun space
uniform mat4 gi_norm_mat; //gPipeline.mGINormalMatrix - eye space normal to sun space normal matrix
uniform mat4 gi_inv_proj; //gPipeline.mGIInvProj - projected sun space to sun space
uniform float gi_radius;
uniform float gi_intensity;
uniform int gi_samples;
uniform vec2 gi_kern[25];
uniform vec2 gi_scale;
uniform vec3 gi_quad;
uniform vec3 gi_spec;
uniform float gi_direction_weight;
uniform float gi_light_offset;
uniform float gi_range;

vec4 getPosition(vec2 pos_screen)
{
	float depth = texture2DRect(depthMap, pos_screen.xy).a;
	vec2 sc = pos_screen.xy*2.0;
	sc /= screen_res;
	sc -= vec2(1.0,1.0);
	vec4 ndc = vec4(sc.x, sc.y, 2.0*depth-1.0, 1.0);
	vec4 pos = inv_proj * ndc;
	pos /= pos.w;
	pos.w = 1.0;
	return pos;
}

vec4 getGIPosition(vec2 gi_tc)
{
	float depth = texture2D(depthGIMap, gi_tc).a;
	vec2 sc = gi_tc*2.0;
	sc -= vec2(1.0, 1.0);
	vec4 ndc = vec4(sc.x, sc.y, 2.0*depth-1.0, 1.0);
	vec4 pos = gi_inv_proj*ndc;
	pos.xyz /= pos.w;
	pos.w = 1.0;
	return pos;
}

vec3 giAmbient(vec3 pos, vec3 norm)
{
	vec4 gi_c = gi_mat_proj * vec4(pos, 1.0);
	gi_c.xyz /= gi_c.w;

	vec4 gi_pos = gi_mat*vec4(pos,1.0);
	vec3 gi_norm = (gi_norm_mat*vec4(norm,1.0)).xyz;
	gi_norm = normalize(gi_norm);
	
	vec4 c_spec = texture2DRect(specularRect, vary_fragcoord.xy);
	gi_pos.xyz += (texture2D(noiseMap, vary_fragcoord.xy/128.0).x)*gi_spec.z*gi_norm.xyz;
	vec2 tcx = gi_norm.xy;
	vec2 tcy = gi_norm.yx;
	
	vec4 eye_pos = gi_mat*vec4(0,0,0,1.0);
	
	vec3 eye_dir = normalize(gi_pos.xyz-eye_pos.xyz);
	vec3 eye_ref = reflect(eye_dir, gi_norm);
	
	//vec3 eye_dir = vec3(0,0,-1);
	//eye_dir = (gi_norm_mat*vec4(eye_dir, 1.0)).xyz;
	//eye_dir = normalize(eye_dir);
	
	//float round_x = gi_scale.x;
	//float round_y = gi_scale.y;
	
	vec3 debug = texture2D(normalGIMap, gi_c.xy).rgb;
	//debug.xz = vec2(0.0,0.0);
	//debug = fract(debug);
	
	float round_x = 1.0/64.0;
	float round_y = 1.0/64.0;
	
	//gi_c.x = floor(gi_c.x/round_x+0.5)*round_x;
	//gi_c.y = floor(gi_c.y/round_y+0.5)*round_y;

	float da = texture2DRect(lightMap, vary_fragcoord.xy).r;

	vec3 fdiff = vec3(da);
	
	if (da > 0.0)
	{
		vec3 ha = -eye_dir;
		ha.z += 1.0;
		ha = normalize(ha);
		
		float sa = dot(ha,gi_norm);
		da = min(da, texture2D(lightFunc, vec2(sa, c_spec.a)).a);
		fdiff += da*(c_spec.rgb*c_spec.a*2.0);
	}
	
	float fda = da;
	
	vec3 rcol = vec3(0,0,0);
	
	float fsa = 0.0;
	

	for (int i = -1; i <= 1; i += 1 )
	{
		for (int j = -1; j <= 1; j+= 1)
		{
			vec2 tc = vec2(i, j)*0.75+gi_norm.xy;
			vec3 nz = texture2D(noiseMap, vary_fragcoord.xy/128.0+tc*0.5).xyz;
			tc += gi_norm.xy*nz.z;
			tc += nz.xy*2.0;
			tc /= gi_samples;
			tc += gi_c.xy;
			
			vec3 lnorm = -normalize(texture2D(normalGIMap, tc.xy).xyz*2.0-1.0);
			vec3 lpos = getGIPosition(tc.xy).xyz;
							
			vec3 at = lpos-gi_pos.xyz;
			float dist = dot(at,at);
			float da = clamp(1.0/(gi_spec.x*dist), 0.0, 1.0);
			
						
			if (da > 0.01)
			{ //possible contribution of indirect light to this surface
				vec3 ldir = at;
				
				float ld = -dot(ldir, lnorm);
				
				if (ld < 0.0)
				{  					
					float ang_atten = dot(ldir, gi_norm);
				
					if (ang_atten > 0.0)
					{  
						vec4 spec = texture2D(specularGIMap, tc.xy);
						at = normalize(at);
						vec3 diff;		
												
						{ //contribution from indirect source to visible pixel
							vec3 ha = at;
							ha.z -= 1.0;
							ha = normalize(ha);
							float sa = dot(ha,lnorm);
							da = min(da, texture2D(lightFunc, vec2(sa, spec.a)).a);
						
							diff = texture2D(diffuseGIMap, tc.xy).rgb+spec.rgb*spec.a*2.0;
						}
						
						if (da > 0.0)
						{ //contribution from visible pixel to eye
							vec3 ha = normalize(at-eye_dir);
							float sa = dot(ha, gi_norm);
							da = min(da, texture2D(lightFunc, vec2(sa, c_spec.a)).a);
							fda += da;
							fdiff += da*(c_spec.rgb*c_spec.a*2.0+vec3(1,1,1))*diff.rgb;
						}
					}
				}
			}
		}
	}

	//fdiff /= max(gi_spec.y*fda, gi_quad.z);
	//fdiff = clamp(fdiff, vec3(0), vec3(1));
	fdiff *= 64.0;
	fdiff *= sunlight_color.rgb;
	
	vec3 ret = fda*fdiff;
	//ret = ret*ret*gi_quad.x+ret*gi_quad.y+gi_quad.z;			

	//fda *= nz.z;
	
	//rcol.rgb *= gi_intensity;
	//return rcol.rgb+vary_AmblitColor.rgb*0.25;
	//return vec4(debug, 0.0);
	//return vec4(fda*fdiff, 0.0);
	return clamp(ret,vec3(0.0), vec3(1.0));
	//return debug.xyz;
}

void main() 
{
	vec2 pos_screen = vary_fragcoord.xy;
	vec4 pos = getPosition(pos_screen);
	
	float rad = gi_range*0.5;
	
	vec3 norm = texture2DRect(normalMap, pos_screen).xyz*2.0-1.0;
	float dist = max(length(pos.xyz)-rad, 0.0);
	
	float da = clamp(1.0-dist/rad, 0.0, 1.0);
	gl_FragData[0].xyz = da > 0.0 ? giAmbient(pos, norm)*da : vec3(0,0,0);
}
