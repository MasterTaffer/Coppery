#version 330 core

layout(location = 0) out vec4 color;

#if GFXS_Normal==1
    layout(location = 1) out vec4 normal;
    uniform vec3 DefaultNormal;
#endif

#if GFXS_Specular==1
    layout(location = 2) out vec4 specular;
#endif

in float alphaMod;

void main()
{
	color.rgb = vec3(alphaMod * 4.0, alphaMod * 3.0, alphaMod * 1.5);
    color.a =  4.0 * alphaMod;
    if (color.a < 0)
        discard;

    #if GFXS_Normal==1
	    normal = vec4(DefaultNormal, 0);
    #endif
    #if GFXS_Specular==1
	    specular = vec4(1,1,1,0);
    #endif
}
