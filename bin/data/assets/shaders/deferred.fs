#version 330 core
out vec4 color;

uniform sampler2D Texture;

#if GFXS_Normal==1
    uniform sampler2D TextureNormal;
#endif

#if GFXS_Specular==1
    uniform sampler2D TextureSpecular;
#endif

uniform sampler2D TextureFOV;
uniform sampler2D TextureDepth;
uniform vec2 FOVCaster;

uniform vec3 LightColor;
uniform float LightPower;
uniform vec3 LightPos;
uniform vec2 SpecularData;

in vec2 relativeObserver;
in vec2 texCoord;

void main()
{
	if (texture(TextureDepth,texCoord).r < 0.05)
	{
		color = texture(Texture,texCoord);
	}
	else
	{
		
		float lightDist = length(vec3(relativeObserver,LightPos.z));
		float power = LightPower;
		float lightP = power-(lightDist/8+lightDist*lightDist/16);
		
		
		lightP /= power*0.8;
		lightP = clamp(lightP,0,1);
		lightP =lightP*lightP*lightP;
		
		if (texture(TextureFOV,texCoord).r > 0.5)
			if (texture(TextureDepth,texCoord).r > 0.25)
				lightP /= 2.4;
				
		
		
		vec4 lColor = vec4(LightColor,0)*lightP;
        

    	float specularPower = SpecularData.y;
		float specularIntensity = SpecularData.x*lightP;
        vec3 eyeDir = vec3(0,0,-1);
        
		vec3 lightDir = normalize(vec3(relativeObserver,LightPos.z));

		#if GFXS_Normal==1
		    vec3 normal = ((texture(TextureNormal,texCoord)*2)-1).xyz;
		    normal.x *= 1;
		    normal.y *= 1;
        #else
            vec3 normal = vec3(0,0,1);
        #endif
	    float visibility = clamp(dot(normal,lightDir),0.0,1.0);
	    visibility = pow(visibility,2);
		
		vec4 specularColor = vec4(0,0,0,0);

        #if GFXS_Specular==1
            vec3 reflection = reflect(lightDir,normal);
	        float specular = clamp ( dot (eyeDir,reflection),0,1);
	        specular = pow(specular,specularPower)*specularIntensity*0.05;
		    specularColor =lColor*texture(TextureSpecular, texCoord).r*specular;
        #endif
		
		float ambient = 0.1;
		visibility = clamp(visibility,0.0,1.0-ambient)+ambient;

        
		
		vec4 diffuseColor = lColor*texture(Texture,texCoord)*visibility;
		
		color = diffuseColor+specularColor;
		
		color.a = 1.0f;

	}
}
