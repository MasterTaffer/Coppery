#version 330 core
layout(location = 0) out vec4 color;

#if GFXS_Normal==1
    layout(location = 1) out vec4 normal;
    uniform sampler2D TextureNormal;
#endif

#if GFXS_Specular==1
    layout(location = 2) out vec4 specular;
    uniform sampler2D TextureSpecular;
#endif

uniform sampler2D Texture;
uniform vec4 FadeColor;

in vec2 texCoord;
void main()
{
	color = texture(Texture,texCoord);
	
	if (color.a == 0.0f)
		discard;
    float fade = FadeColor.a;
    color.rgb = FadeColor.rgb*fade+color.rgb*(1-fade);
		
	#if GFXS_Normal==1
        normal = texture(TextureNormal,texCoord);
    #endif

    #if GFXS_Specular==1
	    specular = texture(TextureSpecular,texCoord);
    #endif
}
