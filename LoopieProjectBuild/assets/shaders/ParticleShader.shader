[vertex]
#version 460 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
// Instanced attributes
layout(location = 2) in mat4 a_InstanceTransform; // Uses locations 2, 3, 4, 5
layout(location = 6) in vec4 a_InstanceColor;

layout(std140, binding = 0) uniform Matrices 
{
    mat4 lp_Projection;
    mat4 lp_View;
};

out vec2 v_TexCoord;
out vec4 v_Color; // Pass the color to the fragment shader

void main()
{
    // Use the instanced transform!
    gl_Position = lp_Projection * lp_View * a_InstanceTransform * vec4(a_Position, 1.0);
    v_TexCoord = a_TexCoord;
    v_Color = a_InstanceColor;
}

[fragment]
#version 460 core
in vec2 v_TexCoord;
in vec4 v_Color; // Receive from vertex shader
out vec4 o_Color;

uniform sampler2D u_Sprite;
uniform bool u_UseSprite;

void main()
{
    vec4 finalColor = v_Color;

    if (u_UseSprite)
    {
        vec4 texColor = texture(u_Sprite, v_TexCoord);
        finalColor = texColor * v_Color;
    }

    if (finalColor.a < 0.01)
        discard;

    finalColor.rgb *= finalColor.a;

    o_Color = finalColor;
}
