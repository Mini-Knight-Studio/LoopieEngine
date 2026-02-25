[vertex]
#version 460 core
/// DO NOT MODIFY
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec4 a_Color;

layout (std140, binding = 0) uniform Matrices
{
    mat4 lp_Projection;
    mat4 lp_View;
};

uniform mat4 lp_Transform;
///

uniform float outlineThickness = 0.01;

void main()
{
    // Transform to view space
    vec4 viewPos = lp_View * lp_Transform * vec4(a_Position, 1.0);
    // Compute normal in view space
    vec3 normalViewSpace = normalize(mat3(lp_View * lp_Transform) * a_Normal);
    // Distance scaling so outline looks constant
    float dist = length(viewPos.xyz);
    // Extract projection info
    float projScale = lp_Projection[1][1]; // = 1/tan(fov/2)
    // Convert screen-space thickness to view-space offset
    float viewOffset = outlineThickness * dist / projScale;
    // Apply offset
    viewPos.xyz += normalViewSpace * viewOffset;
    // Projection
    gl_Position = lp_Projection * viewPos;

}

[fragment]
#version 460 core
out vec4 FragColor;

uniform vec4 u_Color = vec4(0.0, 0.5, 0.75, 1.0);

void main()
{
    FragColor = u_Color;
} 
