[vertex]
#version 460 core
/// DO NOT MODIFY
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec4 a_Color;

// GPU Skinning
layout (location = 5) in ivec4 a_BoneIDs;
layout (location = 6) in vec4 a_Weights;

layout (std140, binding = 0) uniform Matrices
{
    mat4 lp_Projection;
    mat4 lp_View;
};

uniform mat4 lp_Transform;
uniform mat4 lp_Bones[100]; 
///

uniform bool u_SkinnedMode = false;

out vec2 v_TexCoord;
out vec3 v_Normal;


void main()
{
    mat4 skinMatrix = mat4(1.0);

    if (u_SkinnedMode)
    {

        skinMatrix = 
              a_Weights.x * lp_Bones[a_BoneIDs.x] +
              a_Weights.y * lp_Bones[a_BoneIDs.y] +
              a_Weights.z * lp_Bones[a_BoneIDs.z] +
              a_Weights.w * lp_Bones[a_BoneIDs.w];
    }

    vec4 skinnedPos = skinMatrix * vec4(a_Position, 1.0);
    vec3 skinnedNormal = mat3(skinMatrix) * a_Normal;

    gl_Position = lp_Projection * lp_View * lp_Transform * skinnedPos;
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
