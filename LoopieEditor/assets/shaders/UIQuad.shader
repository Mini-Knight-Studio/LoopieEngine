[vertex]
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Matrices
{
    mat4 lp_Projection;
    mat4 lp_View;
};

uniform mat4 lp_Transform;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = lp_Projection * lp_View * lp_Transform * vec4(a_Position, 1.0);
}

[fragment]
#version 460 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform vec4 u_Color;

void main()
{
    FragColor = u_Color;
}