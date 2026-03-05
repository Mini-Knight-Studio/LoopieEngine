[vertex]
#version 460 core
/// DO NOT MODIFY
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec4 a_Color;

// GPU Skinning
layout (location = 5) in vec4 a_BoneIDs;
layout (location = 6) in vec4 a_Weights;

layout (std140, binding = 0) uniform Matrices
{
    mat4 lp_Projection;
    mat4 lp_View;
};

uniform mat4 lp_Transform;
uniform mat4 lp_Bones[100]; 
uniform bool lp_Skinned;
///


out vec2 v_TexCoord;
out vec3 v_Normal;


void main()
{
    mat4 skinMatrix = mat4(1.0);

    if (lp_Skinned)
    {
        skinMatrix =
              a_Weights[0] * lp_Bones[int(a_BoneIDs[0])] +
              a_Weights[1] * lp_Bones[int(a_BoneIDs[1])] +
              a_Weights[2] * lp_Bones[int(a_BoneIDs[2])] +
              a_Weights[3] * lp_Bones[int(a_BoneIDs[3])];
    }

    vec4 localPos = skinMatrix * vec4(a_Position, 1.0);
    vec3 skinnedNormal = mat3(skinMatrix) * a_Normal;

    gl_Position = lp_Projection * lp_View * lp_Transform * localPos;

    v_TexCoord = a_TexCoord;
    v_Normal = normalize(mat3(lp_Transform) * skinnedNormal);
}


[fragment]
#version 460 core
in vec2 v_TexCoord;
in vec3 v_Normal;
out vec4 FragColor;

uniform sampler2D u_Albedo;
uniform vec4 u_Color = vec4(1.0);

void main()
{
    vec4 texColor;

    texColor = texture(u_Albedo, v_TexCoord);
        if (texColor.a < 0.5f)
            discard;

    vec3 lightDir = normalize(vec3(0.3, 0.7, 0.5));
    vec3 normal = normalize(v_Normal);

    float diff = max(dot(normal, lightDir), 0.4);
    diff = mix(1.0, diff, 0.8);

    vec3 litColor = texColor.rgb * diff;
    FragColor = vec4(litColor, texColor.a) * u_Color;
}
