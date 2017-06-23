#version 330

out vec4 fragColor;
in vec2 v_uvs;
uniform sampler2D u_texture;
in vec3 a_outNormal;
uniform float u_transparency;

void main(void)
{
	vec4 texture_color = texture2D(u_texture, v_uvs);
	fragColor = vec4(texture_color.xyz, u_transparency);

}
