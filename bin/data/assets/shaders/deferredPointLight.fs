#version 330 core
#pragma include lighting.glsl

layout(location = 0) out vec4 color;

uniform sampler2D Texture;
uniform sampler2D TextureDepth;

#if GFXS_FOV==1
	uniform sampler2D TextureFOV;
#endif

#if GFXS_Normal==1
    uniform sampler2D TextureNormal;
#endif

#if GFXS_Specular==1
    uniform sampler2D TextureSpecular;
#endif

uniform vec3 LightColor;
uniform float LightPower;
uniform vec2 SpecularData;

in vec2 texCoord;
in vec3 relativeLightPos;

void main()
{
	color = lightingSimple(texCoord, relativeLightPos, LightPower, LightColor, Texture, TextureDepth);
}
