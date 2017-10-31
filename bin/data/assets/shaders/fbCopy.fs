#version 330 core
layout(location = 0) out vec3 color;

uniform sampler2D Texture;
in vec2 texCoord;


void main()
{
	color.rgb = texture(Texture,texCoord).rgb;
}
