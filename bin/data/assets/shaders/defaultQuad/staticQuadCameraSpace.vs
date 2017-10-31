#version 330 core
layout(location = 0) in vec2 VertexPosition;
layout(location = 1) in vec2 VertexUV;


uniform mat4 MatProjection;

uniform vec2 MinUV;
uniform vec2 MaxUV;

uniform vec2 Size;
uniform vec2 Position;

uniform float Depth;


out vec2 texCoord;

void main()
{
    vec2 pos =  VertexPosition.xy;
    pos *= Size / 2.0;
    pos += Position;

	gl_Position = MatProjection * vec4(pos, Depth, 1);

	texCoord = mix(MinUV, MaxUV, VertexUV);
}

