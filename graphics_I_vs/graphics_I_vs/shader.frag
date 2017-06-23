#version 330

out vec4 fragColor;
in vec2 v_uvs;
uniform sampler2D u_texture;
uniform vec3 u_light_dir,u_cam_pos,u_light_color;
in vec3 a_outNormal;
in vec3 v_world_vertex_pos;
uniform float u_shininess,u_ambient;

void main(void)
{
		vec3 N = normalize(a_outNormal);
		vec3 L = normalize(u_light_dir);
		
		float NdotL = max(dot(N,L),0.0);
		
		vec3 R = reflect(L,N);
		vec3 E = normalize((u_cam_pos-v_world_vertex_pos));

		vec3 texture_color = texture2D(u_texture, v_uvs).xyz;
		vec3 ambient_color = texture_color * u_ambient;
		vec3 diffuse_color = texture_color * NdotL;
		
		float RdotE = pow(max(dot(R,-E),0.0), u_shininess);

		vec3 specular_color = u_light_color * RdotE;

		vec3 I = ambient_color + diffuse_color + specular_color;
		vec3 final_color = texture_color * I;

		fragColor = vec4(final_color, 1.0);
}
