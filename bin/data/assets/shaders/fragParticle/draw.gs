#version 330

out float alphaMod;

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in vec4 position[];
in vec2 velocity[];
in float time[];

uniform mat4 MatProjection;

void main()
{
    const float maxLifeTime = 60.0;

	float s2 = 2.0f;
    s2 *= (0.2 + time[0] / maxLifeTime);

    alphaMod = 1.0 - (time[0] / maxLifeTime);

    float z = position[0].z;

    vec2 useVelocity = velocity[0] * (maxLifeTime - time[0]) / maxLifeTime;
    vec2 pos;

    pos = position[0].xy + useVelocity.xy * s2;
	gl_Position = MatProjection * vec4(pos, z, 1);
	EmitVertex();


    alphaMod /= 2;
    pos = position[0].xy;
    gl_Position = MatProjection * vec4(pos, z, 1);
    EmitVertex();

	EndPrimitive();
}
