#version 330 core
layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec2 VertexUV;

uniform mat4 MatProjection;
uniform vec2 Camera;

out vec2 texCoord;


void main()
{
	texCoord = VertexUV;
	gl_Position = MatProjection * vec4(VertexPosition.xy-Camera,VertexPosition.z,1);
}
