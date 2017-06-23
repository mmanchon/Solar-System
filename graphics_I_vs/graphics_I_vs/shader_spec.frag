#version 330

out vec4 fragColor;
in vec2 v_uvs;
uniform sampler2D u_texture,u_texture_normal,u_texture_spec;
uniform vec3 u_light_dir,u_cam_pos,u_light_color;
in vec3 a_outNormal;
in vec3 v_world_vertex_pos;
uniform float u_shininess,u_ambient;



mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx( p );
	vec3 dp2 = dFdy( p );
	vec2 duv1 = dFdx( uv );
	vec2 duv2 = dFdy( uv );
	// solve the linear system
	vec3 dp2perp = cross( dp2, N );
	vec3 dp1perp = cross( N, dp1 );
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
			 
	// construct a scale-invariant frame 
	float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
	return mat3( T * invmax, B * invmax, N );
}

// assume N, the interpolated vertex normal and 
// V, the view vector (vertex to eye)
vec3 perturbNormal( vec3 N, vec3 V, vec2 texcoord, vec3 normal_pixel )
{

	normal_pixel = normal_pixel * 2.0 - 1.0;
	mat3 TBN = cotangent_frame(N, V, texcoord);
	return normalize(TBN * normal_pixel);
}

void main(void)
{
		vec3 texture_normal = texture2D(u_texture_normal, v_uvs).xyz;
		vec3 N = normalize(a_outNormal);
		vec3 N_orig = N;
		vec3 L = normalize(u_light_dir);
		
		N = perturbNormal(N,v_world_vertex_pos,v_uvs,texture_normal);
		N = mix(N_orig,N,1.0);

		float NdotL = max(dot(N,L),0.0);
		
		vec3 R = reflect(L,N);
		vec3 E = normalize((u_cam_pos-v_world_vertex_pos));

		vec3 texture_color = texture2D(u_texture, v_uvs).xyz;
		vec3 ambient_color = texture_color * u_ambient;
		vec3 diffuse_color = texture_color * NdotL;
		
		float RdotE = pow(max(dot(R,-E),0.0), u_shininess);

		float texture_spec = texture2D(u_texture_spec,v_uvs).x;
		vec3 specular_color = u_light_color * RdotE * texture_spec;

		vec3 I = ambient_color + diffuse_color + specular_color;
		vec3 final_color = texture_color * I;

		fragColor = vec4(final_color, 1.0);
}
