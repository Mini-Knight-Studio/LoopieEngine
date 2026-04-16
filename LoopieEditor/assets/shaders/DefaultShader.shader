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
    vec4 l_ColorIntensity;          // Color + Intensity
    vec4 l_PositionType;            // Position + Type
    vec4 l_DirectionInnerCone;      // Direction + Inner Cone Angle
    vec4 l_AttenuationOuterCone;    // Attenuation + Outer Cone Angle
    vec4 l_SMapAndSColor;           // ShadowMap number + Shadow Color
};

layout (std140, binding = 0) uniform Matrices
{
    mat4 lp_Projection;
    mat4 lp_View;
};

layout (std140, binding = 1) uniform Lighting
{
    vec4 lp_CameraWorldPosLightCount;
    Light lp_lights[16];
};

layout (std430, binding = 2) readonly buffer BoneMatrices
{
    mat4 lp_Bones[];
};
uniform int lp_BoneOffset;

uniform mat4 lp_Transform;
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
              a_Weights[0] * lp_Bones[lp_BoneOffset + int(a_BoneIDs[0])] +
              a_Weights[1] * lp_Bones[lp_BoneOffset + int(a_BoneIDs[1])] +
              a_Weights[2] * lp_Bones[lp_BoneOffset + int(a_BoneIDs[2])] +
              a_Weights[3] * lp_Bones[lp_BoneOffset + int(a_BoneIDs[3])];
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
    vec4 l_ColorIntensity;          // Color + Intensity
    vec4 l_PositionType;            // Position + Type
    vec4 l_DirectionInnerCone;      // Direction + Inner Cone Angle
    vec4 l_AttenuationOuterCone;    // Attenuation + Outer Cone Angle
    vec4 l_SMapAndSColor;           // ShadowMap number + Shadow Color
};

layout (std140, binding = 0) uniform Matrices
{
    mat4 lp_Projection;
    mat4 lp_View;
};

layout (std140, binding = 1) uniform Lighting
{
    vec4 lp_CameraWorldPosLightCount;
    Light lp_lights[16];
};

layout (std140, binding = 3) uniform Shadows
{
    mat4 lp_LightSpaceMatrix[4];
};

uniform sampler2D u_Albedo;
uniform sampler2D u_Specular;
uniform sampler2D u_Normal;
uniform sampler2D lp_ShadowMaps[4];
uniform sampler2D u_Emissive;
uniform float u_Roughness = 32.0; // highlight, smaller value = broader spotlight (feels more shiny)
uniform float u_EmissiveIntensity = 0.0;
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

            
            // Shadows
            float shadow = 1.0;
            int shadowIndex = int(lp_lights[i].l_SMapAndSColor.x);
            if (shadowIndex >= 0)
            {
                shadow = 0.0;
                vec4 fragPosLightSpace =  lp_LightSpaceMatrix[shadowIndex] * vec4(v_WorldPos, 1.0);
                vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
                projCoords = projCoords * 0.5 + 0.5;

                vec2 texelSize = 1.0 / vec2(textureSize(lp_ShadowMaps[shadowIndex], 0));
                float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
                float currentDepth = projCoords.z;

                for (int x=-1; x<=1; ++x)
                {
                    for (int y=-1; y<=1; ++y)
                    {
                        vec2 offset = vec2(x, y) * texelSize;
                        float closestDepth = texture(lp_ShadowMaps[shadowIndex], projCoords.xy + offset).r;
                        shadow += (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
                    }
                }
                shadow /= 9.0;
            }

            result += (diffuse + specular) * mix(lp_lights[i].l_SMapAndSColor.yzw, vec3(1.0), shadow);
        }
        else if (int(lp_lights[i].l_PositionType.w) == 2) // Spot
        {
            vec3 toLight = lp_lights[i].l_PositionType.xyz - v_WorldPos;
            float d = length(toLight); // distance
            vec3 lightDir = toLight / d; // Normalizes lightDir
           
            // Attenuation
            float attenuation = 1.0 / (lp_lights[i].l_AttenuationOuterCone.x + lp_lights[i].l_AttenuationOuterCone.y * d + 
                                       lp_lights[i].l_AttenuationOuterCone.z * d * d);

            // Diffuse
            float diff = max(dot(normal, lightDir), 0.0);
            vec3 diffuse = diff * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;

            // Specular
            vec3 reflectDir = reflect(-lightDir, normal);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Roughness);
            vec3 specular = texSpecular.xyz * spec * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;  

            float angle = dot(lp_lights[i].l_DirectionInnerCone.xyz, -lightDir);
            float innerAngleCone = cos(radians(lp_lights[i].l_DirectionInnerCone.w));
            float outerAngleCone = cos(radians(lp_lights[i].l_AttenuationOuterCone.w));

            float spotlightAttenuation = smoothstep(outerAngleCone, innerAngleCone, angle);

            // Shadows
            float shadow = 1.0;
            int shadowIndex = int(lp_lights[i].l_SMapAndSColor.x);
            if (shadowIndex >= 0)
            {
                shadow = 0.0;
                vec4 fragPosLightSpace = lp_LightSpaceMatrix[shadowIndex] * vec4(v_WorldPos, 1.0);
                vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
                projCoords = projCoords * 0.5 + 0.5;

                vec2 texelSize = 1.0 / vec2(textureSize(lp_ShadowMaps[shadowIndex], 0));
                float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
                float currentDepth = projCoords.z;

                for (int x=-1; x<=1; ++x)
                {
                    for (int y=-1; y<=1; ++y)
                    {
                        vec2 offset = vec2(x, y) * texelSize;
                        float closestDepth = texture(lp_ShadowMaps[shadowIndex], projCoords.xy + offset).r;
                        shadow += (currentDepth - bias) > closestDepth ? 0.0 : 1.0;
                    }
                }
                shadow /= 9.0;
            }

            result += ((diffuse + specular) * attenuation * spotlightAttenuation) * mix(lp_lights[i].l_SMapAndSColor.yzw, vec3(1.0), shadow);

        }
        else if (int(lp_lights[i].l_PositionType.w) == 3) // Point
        {
            vec3 toLight = lp_lights[i].l_PositionType.xyz - v_WorldPos;
            float d = length(toLight); // distance
            vec3 lightDir = toLight / d; // Normalizes lightDir
           
            // Attenuation
            float attenuation = 1.0 / (lp_lights[i].l_AttenuationOuterCone.x + lp_lights[i].l_AttenuationOuterCone.y * d + 
                                       lp_lights[i].l_AttenuationOuterCone.z * d * d);

            // Diffuse
            float diff = max(dot(normal, lightDir), 0.0);
            vec3 diffuse = diff * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;

            // Specular
            vec3 reflectDir = reflect(-lightDir, normal);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Roughness);
            vec3 specular = texSpecular.xyz * spec * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;  

            result += (diffuse + specular) * attenuation;
        }
    }

    // Resulting Color with Lighting
    result = result * texColor.rgb;

    // Adding 
    vec3 emissive = texture(u_Emissive, v_TexCoord).rgb * u_EmissiveIntensity;
    result += emissive;

    FragColor = vec4(result, texColor.a) * u_Color;
}
