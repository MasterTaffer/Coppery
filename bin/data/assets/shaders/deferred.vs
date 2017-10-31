#version 330 core
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 VertexUV;
layout(location = 2) in vec2 WorldCoord;

uniform vec2 FOVCaster;

out vec2 texCoord;
out vec2 relativeObserver;
out vec2 worldPos;

void main()
{
	relativeObserver = FOVCaster-WorldCoord;
	worldPos = WorldCoord;
	texCoord = VertexUV;
	gl_Position = vec4(VertexPosition,1);
}

