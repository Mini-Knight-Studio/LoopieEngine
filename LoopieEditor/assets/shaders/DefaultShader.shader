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

layout (std140, binding = 1) uniform Lighting
{
    vec4 lp_CameraWorldPos;
    vec4 lp_LightDir;
    vec4 lp_LightCol;
};

uniform mat4 lp_Transform;
uniform mat4 lp_Bones[100]; 
uniform bool lp_Skinned;
///


out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_WorldPos;

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

    vec4 worldPos = lp_Transform * localPos;
    gl_Position = lp_Projection * lp_View * worldPos;

    v_TexCoord = a_TexCoord;
    v_Normal = normalize(mat3(lp_Transform) * skinnedNormal);
    v_WorldPos = worldPos.xyz;
}


[fragment]
#version 460 core
in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_WorldPos;

out vec4 FragColor;

layout (std140, binding = 0) uniform Matrices
{
    mat4 lp_Projection;
    mat4 lp_View;
};

layout (std140, binding = 1) uniform Lighting
{
    vec4 lp_CameraWorldPos;
    vec4 lp_LightDir;
    vec4 lp_LightCol;
};

uniform sampler2D u_Albedo;
uniform vec4 u_Color = vec4(1.0);

void main()
{
    vec4 texColor;

    texColor = texture(u_Albedo, v_TexCoord);
        if (texColor.a < 0.5f)
            discard;
    
    // Ambient
    vec3 ambient = lp_LightCol.xyz * lp_LightCol.w;

    // Diffuse
    vec3 normal = normalize(v_Normal);
    // Normal = mat3(transpose(inverse(model))) * aNormal; // If light looks wrong due to scaling
    float diff = max(dot(normal, lp_LightDir.xyz), 0.0);
    vec3 diffuse = diff * lp_LightCol.xyz;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(lp_CameraWorldPos.xyz - v_WorldPos);
    vec3 reflectDir = reflect(-lp_LightDir.xyz, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lp_LightCol.xyz;  

    // Resulting Color with Lighting
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;

    FragColor = vec4(result, texColor.a) * u_Color;
}
