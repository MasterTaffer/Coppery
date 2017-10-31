#version 330 core
layout(location = 0) in vec3 VertexPosition;

uniform mat4 MatProjection;
uniform vec2 Camera;
void main()
{
	gl_Position = MatProjection * vec4(VertexPosition.xy-Camera,VertexPosition.z,1);
}
