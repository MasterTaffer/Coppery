#version 330 core
layout(location = 0) out vec4 color;

#if GFXS_Normal==1
    layout(location = 1) out vec4 normal;
    uniform vec3 DefaultNormal;
#endif

#if GFXS_Specular==1
    layout(location = 2) out vec4 specular;
#endif

uniform vec4 FadeColor;

void main()
{
	color = FadeColor;
    #if GFXS_Normal==1
	    normal = vec4(DefaultNormal,0);
    #endif
    #if GFXS_Specular==1
	    specular = FadeColor;
    #endif
}
