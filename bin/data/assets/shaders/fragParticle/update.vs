#version 330

layout (location = 0) in vec2 _position;
layout (location = 1) in vec2 _velocity;
layout (location = 2) in float _time;

out vec2 position;
out vec2 velocity;
out float time;

void main()
{
    position = _position;
    velocity = _velocity;
    time = _time;
} 
