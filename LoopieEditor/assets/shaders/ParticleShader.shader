[vertex]
#version 460 core
layout(location = 0) in vec3 a_Position;


uniform mat4 lp_Transform;

layout(std140, binding = 0) uniform Matrices 
{
    mat4 lp_Projection;
    mat4 lp_View;
};



void main()
{
    gl_Position = lp_Projection * lp_View * lp_Transform * vec4(a_Position, 1.0);
   
}

[fragment]
#version 460 core
out vec4 o_Color;

uniform vec4 u_Color;

void main()
{
    o_Color = u_Color; 
}

