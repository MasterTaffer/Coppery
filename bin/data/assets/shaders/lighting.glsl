#pragma once

vec4 lightingSimple(vec2 coord, vec3 relativeLightPos, float power, vec3 color, sampler2D diffuse, sampler2D depth)
{
	float lightPMod = 1.0f;
	if (texture(depth, coord).r < 0.05)
		discard;

	vec3 relLite = relativeLightPos;

	vec3 eyeDir = vec3(0.0,0.0,-1.0);

	vec3 lightDir = normalize(relLite);
	
	float lightDist = length(relLite);
	float lightP = power-(lightDist/8+lightDist*lightDist/16);
	lightP /= power;
	
	lightP = clamp(lightP,0,1);

	
	lightP = pow(lightP,2);
	
	lightP*= lightPMod;
	
	if (texture(depth,coord).r <= 0.6)
	if (texture(depth,coord).r >= 0.3)
		lightP /= 1.5;
				
	vec3 normal = vec3(0,0,1);
	
	vec4 lColor = vec4(color,0)*lightP;

    float visibility = clamp(dot(normal,lightDir),0.0,1.0);
    visibility = pow(visibility,2);
	

	float ambient = 0.15;
	visibility = clamp(visibility,0.0,1.0-ambient)+ambient;

    float d1 = 0.98f;
    float d2 = 1-d1;
	
	vec4 dc = (texture(diffuse,coord)*d1+lColor*d2);	
	vec4 diffuseColor = lColor*dc*visibility;
	
	return vec4(diffuseColor.xyz, 1.0);
}

vec4 lightingSimpleC(vec2 coord, float val, vec3 color, sampler2D diffuse, sampler2D depth)
{
	if (texture(depth, coord).r < 0.05)
		discard;
	
	vec4 lColor = vec4(color, 0) * val;

    float d1 = 0.98f;
    float d2 = 1-d1;
	
	vec4 dc = (texture(diffuse,coord) * d1 + lColor * d2);	
	vec4 diffuseColor = lColor * dc;
	
	return vec4(diffuseColor.xyz, 1.0);
}


/*
// #version 330 core
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
	float lightPMod = 1.0f;
	if (texture(TextureDepth,texCoord).r < 0.05)
		discard;
	
		#if GFXS_FOV==1
		if (texture(TextureFOV,texCoord).r > 0.5)
		{
			discard;
		}
		#endif
	
	vec3 relLite = relativeLightPos;
	//relLite.z += 0 * texture(TextureDepth,texCoord).r;

	vec3 eyeDir = vec3(0.0,0.0,-1.0);

	vec3 lightDir = normalize(relLite);
	

	float lightDist = length(relLite);
	float lightP = LightPower-(lightDist/8+lightDist*lightDist/16);
	lightP /= LightPower;
	
	lightP = clamp(lightP,0,1);

	
	lightP = pow(lightP,2);
	
	lightP*= lightPMod;
	
	if (texture(TextureDepth,texCoord).r <= 0.6)
	if (texture(TextureDepth,texCoord).r >= 0.3)
		lightP /= 1.5;
				
	
	//lightP = pow(lightP,2);
    //lightP = pow(lightP,2);
	
	vec4 lColor = vec4(LightColor,0)*lightP;
	
	#if GFXS_Normal==1
	    vec3 normal = ((texture(TextureNormal,texCoord)*2)-1).xyz;
	    normal.x *=-2;
	    normal.y *=-2;
		
    #else
        vec3 normal = vec3(0,0,1);
    #endif

    float specularPower = SpecularData.y;
	float specularIntensity = SpecularData.x*lightP;

    float visibility = clamp(dot(normal,lightDir),0.0,1.0);
    visibility = pow(visibility,2);
	

    vec4 specularColor = vec4(0,0,0,0);

    #if GFXS_Specular==1
        vec3 reflection = reflect(lightDir,normal);
	    float specular = clamp ( dot (eyeDir,reflection),0,1);
	    specular = pow(specular,specularPower)*specularIntensity;
		specularColor = lColor*texture(TextureSpecular, texCoord).r*specular;
    #endif
	
	float ambient = 0.15;
	visibility = clamp(visibility,0.0,1.0-ambient)+ambient;

    float d1 = 0.98f;
    float d2 = 1-d1;
	
	vec4 dc = (texture(Texture,texCoord)*d1+lColor*d2);	
	vec4 diffuseColor = lColor*dc*visibility;
	
	color = diffuseColor+specularColor;
	color.a = 1.0f;
}


*/