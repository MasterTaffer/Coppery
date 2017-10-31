#version 330

layout(points) in;
layout(points) out;
layout(max_vertices = 1) out;

uniform float DeltaTime;

in vec2 position[];
in vec2 velocity[];
in float time[];

out vec2 out_position;
out vec2 out_velocity;
out float out_time;


void main()
{
    const float maxLifeTime = 60.0;
    if (time[0] < maxLifeTime)
    {
        out_position = position[0];

        vec2 useVelocity = velocity[0] * (maxLifeTime - time[0]) / maxLifeTime;
        out_position += useVelocity * DeltaTime;

        out_velocity = velocity[0];
        out_velocity.y += DeltaTime * 0.1;

        out_time = time[0] + DeltaTime;

        EmitVertex();
        EndPrimitive();
    }
}
