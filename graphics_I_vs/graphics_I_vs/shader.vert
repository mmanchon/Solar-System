#version 330
 
in vec3 vertexPos;
in vec3 color;

out vec3 colorInt;
uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat3 u_normal_matrix;
in vec2 uvs;
out vec2 v_uvs;
in vec3 a_normal;
out vec3 a_outNormal;
out vec3 v_world_vertex_pos;


void main()
{
	v_uvs = uvs;
	// position of the vertex
	a_outNormal = u_normal_matrix * a_normal;
	gl_Position =  u_projection  * u_view * u_model * vec4( vertexPos , 1.0 );
	v_world_vertex_pos = (u_model * vec4 (vertexPos,1.0)).xyz;
	// pass the colour to the fragment shader
	//colorInt = color;
}

