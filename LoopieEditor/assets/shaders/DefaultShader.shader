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

struct Light
{
    vec4 l_ColorIntensity;
    vec4 l_PositionType;
    vec4 l_DirectionInnerCone;
    vec4 l_AttenuationOuterCone;
};

layout (std140, binding = 0) uniform Matrices
{
    mat4 lp_Projection;
    mat4 lp_View;
};

layout (std140, binding = 1) uniform Lighting
{
    vec4 lp_CameraWorldPosLightCount;
    Light lp_lights[8];
};

uniform mat4 lp_Transform;
uniform mat4 lp_Bones[100]; 
uniform bool lp_Skinned;
///

uniform vec2 u_Tiling= vec2(1);

out vec2 v_TexCoord;
out vec3 v_WorldPos;
out mat3 v_TBNMatrix;

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
    vec3 skinnedTangent = mat3(skinMatrix) * a_Tangent;

    vec4 worldPos = lp_Transform * localPos;
    gl_Position = lp_Projection * lp_View * worldPos;

    v_TexCoord = a_TexCoord * u_Tiling;
    vec3 normal = normalize(mat3(lp_Transform) * skinnedNormal);
    vec3 tangent = normalize(mat3(lp_Transform) * skinnedTangent);
    v_WorldPos = worldPos.xyz;
    vec3 biTangent = cross(normal, tangent);
    v_TBNMatrix = mat3(tangent, biTangent, normal);
}


[fragment]
#version 460 core
in vec2 v_TexCoord;

in vec3 v_WorldPos;
in mat3 v_TBNMatrix;

out vec4 FragColor;

struct Light
{
    vec4 l_ColorIntensity;
    vec4 l_PositionType;
    vec4 l_DirectionInnerCone;
    vec4 l_AttenuationOuterCone;
};

layout (std140, binding = 0) uniform Matrices
{
    mat4 lp_Projection;
    mat4 lp_View;
};

layout (std140, binding = 1) uniform Lighting
{
    vec4 lp_CameraWorldPosLightCount;
    Light lp_lights[8];
};

uniform sampler2D u_Albedo;
uniform sampler2D u_Specular;
uniform sampler2D u_Normal;
uniform float u_Roughness = 32.0; // highlight, smaller value = broader spotlight (feels more shiny)
uniform vec4 u_Color = vec4(1.0);

void main()
{
    vec4 texColor;

    texColor = texture(u_Albedo, v_TexCoord);
        if (texColor.a < 0.5f)
            discard;

    vec4 texSpecular = texture(u_Specular, v_TexCoord);

    vec4 texNormal = texture(u_Normal, v_TexCoord);
    vec3 normal = v_TBNMatrix * (texNormal.xyz * 2.0 - 1.0);
    normal = normalize(normal);

    vec3 viewDir = normalize(lp_CameraWorldPosLightCount.xyz - v_WorldPos);

    vec3 result = vec3(0.0);


    for (int i = 0; i < lp_CameraWorldPosLightCount.w; ++i)
    {
        if (int(lp_lights[i].l_PositionType.w) == 0) // Ambiental
        {
            // Ambient
            result += lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;
            
        }
        else if (int(lp_lights[i].l_PositionType.w) == 1) // Directional
        {
            vec3 lightDir = -lp_lights[i].l_DirectionInnerCone.xyz; // toward the light

            // Diffuse
            float diff = max(dot(normal, lightDir), 0.0);
            vec3 diffuse = diff * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;

            // Specular
            vec3 reflectDir = reflect(-lightDir, normal);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Roughness);
            vec3 specular = texSpecular.xyz * spec * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;  
            result += (diffuse + specular);
        }
        else if (int(lp_lights[i].l_PositionType.w) == 2) // Spot
        {
        }
        else if (int(lp_lights[i].l_PositionType.w) == 3) // Point
        {
        }
    }

    // Resulting Color with Lighting
    result = result * texColor.rgb;

    FragColor = vec4(result, texColor.a) * u_Color;
}
