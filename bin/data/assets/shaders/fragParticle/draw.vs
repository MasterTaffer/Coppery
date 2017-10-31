#version 330

layout (location = 0) in vec2 _position;
layout (location = 1) in vec2 _velocity;
layout (location = 2) in float _time;

out vec4 position;
out vec2 velocity;
out float time;

uniform vec2 Camera;

void main()
{
    position = vec4(_position.xy - Camera, -30, 1);
    velocity = _velocity;
    time = _time;
} 
