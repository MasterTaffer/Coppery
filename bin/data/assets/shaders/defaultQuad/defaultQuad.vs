#version 330 core
layout(location = 0) in vec2 VertexPosition;
layout(location = 1) in vec2 VertexUV;


uniform mat4 MatProjection;
uniform vec2 Camera;

uniform vec2 Size;
uniform vec2 Position;
uniform float Depth;
uniform float Rotation;

out vec2 texCoord;

void main()
{
    mat2 rotationMatrix = mat2(cos(Rotation), -sin(Rotation), sin(Rotation), cos(Rotation));
    vec2 pos =  VertexPosition.xy;
    pos *= Size / 2.0;
    pos = rotationMatrix * pos;

    pos += Position;

	gl_Position = MatProjection * vec4(pos - Camera, Depth, 1);
	texCoord = VertexUV;
}

