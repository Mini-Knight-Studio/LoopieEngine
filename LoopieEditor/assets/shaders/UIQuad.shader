[vertex]
#version 460 core
/// DO NOT MODIFY
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Matrices
{
    mat4 lp_Projection;
    mat4 lp_View;
};

uniform mat4 lp_Transform;
uniform bool lp_Skinned;
///

out vec2 v_TexCoord;

void main()
{
    if(!lp_Skinned){
        v_TexCoord = a_TexCoord;
        gl_Position = lp_Projection * lp_View * lp_Transform * vec4(a_Position, 1.0);
    }
}

[fragment]
#version 460 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform vec4 u_Color;
uniform sampler2D u_Albedo;

void main()
{
    vec4 tex = texture(u_Albedo, v_TexCoord);
    vec4 col = tex * u_Color;

    if (col.a <= 0.001)
    {
        discard;
    }

    FragColor = col;
}