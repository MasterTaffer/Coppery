#version 330 core
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 VertexUV;
layout(location = 2) in vec2 WorldCoord;

out vec2 texCoord;
out vec3 relativeLightPos;

uniform vec3 LightPos;

void main()
{
	relativeLightPos = LightPos - vec3(WorldCoord,0);
	
	
	texCoord = VertexUV;
	gl_Position = vec4(VertexPosition,1);
}

