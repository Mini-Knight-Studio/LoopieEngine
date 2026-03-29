[vertex]
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 lp_Transform;
uniform bool lp_Skinned;

layout(std140, binding = 0) uniform Matrices 
{
    mat4 lp_Projection;
    mat4 lp_View;
};

out vec2 v_TexCoord;

void main()
{
    if(!lp_Skinned){
        gl_Position = lp_Projection * lp_View * lp_Transform * vec4(a_Position, 1.0);
    }
    v_TexCoord = a_TexCoord;
}

[fragment]
#version 460 core
in vec2 v_TexCoord;
out vec4 o_Color;

uniform vec4 u_Color;
uniform sampler2D u_Sprite;
uniform bool u_UseSprite;

void main()
{
    vec4 finalColor = u_Color;

    if (u_UseSprite)
    {
        vec4 texColor = texture(u_Sprite, v_TexCoord);
        finalColor = vec4(texColor.rgb * u_Color.rgb, texColor.a * u_Color.a);
    }

    if (finalColor.a < 0.01)
        discard;

    o_Color = finalColor;
}
