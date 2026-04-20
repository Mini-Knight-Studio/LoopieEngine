[vertex]
#version 460 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec4 a_Color;

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

uniform mat4 lp_Transform;
uniform vec2 u_Tiling= vec2(1);

out vec2 v_TexCoord;
out vec3 v_WorldPos;
out mat3 v_TBNMatrix;

uniform float lp_Time;

void main()
{
    vec4 localPos =  vec4(a_Position, 1.0);
    vec4 worldPos = lp_Transform * localPos;
    gl_Position = lp_Projection * lp_View * worldPos;

    v_TexCoord = a_TexCoord * u_Tiling;
    vec3 normal = mat3(lp_Transform) * a_Normal;
    vec3 tangent = mat3(lp_Transform) * a_Tangent;
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

uniform sampler2D u_WaterMapA;
uniform sampler2D u_WaterMapB;
uniform sampler2D u_Albedo;         // For water albedo (can be used or not)
uniform sampler2D lp_SceneDepth;    // For foam
uniform sampler2D u_FoamTexture;
uniform bool u_UseAlbedo = false;

uniform vec2 u_TilingA= vec2(1);
uniform vec2 u_TilingB= vec2(1);
uniform vec2 u_TilingAlbedo = vec2(1);
uniform vec2 u_TilingFoam = vec2(10.0, 10.0);

uniform vec2 directionA = vec2(-0.08, 0.1);
uniform vec2 directionB = vec2(-0.08, 0.06);
uniform vec2 directionAlbedo = vec2(-0.08, 0.06);

uniform float u_AlbedoDistortionStrength = 0.02;

uniform float speedA = 1.06;
uniform float speedB = 0.93;
uniform float speedAlbedo = 1.0;

uniform float u_Roughness = 128.0; // highlight, smaller value = broader spotlight (feels more shiny)

uniform vec4 u_FoamColor = vec4 (1.0);
uniform vec3 u_Specular = vec3(0.5);

uniform float u_MinAlpha = 0.39;
uniform float u_MaxAlpha = 1.0;
uniform float u_Fresnel = 0.2;
uniform float u_FoamThreshold = 1.43;
uniform float u_FoamDistortionStrength = 0.9;

uniform vec3 u_ShallowColor = vec3(0.2, 0.5, 0.9);  // light blue
uniform vec3 u_DeepColor = vec3(2.0);    // Might be too high, should be 1.0, but this way the albedo texture is seen more clearly
uniform float u_DepthMaxDistance = 3.0;             // depth at which color fully saturates

uniform float lp_Time = 0;
uniform float lp_Near = 0.3;
uniform float lp_Far = 200.0;

float linearDepth(float d, float near, float far) 
{
    return near * far / (far - d * (far - near));
}

void main()
{

    vec4 texNormalA = texture(u_WaterMapA, (v_TexCoord * u_TilingA + directionA * lp_Time * speedA));
    vec4 texNormalB = texture(u_WaterMapB, (v_TexCoord * u_TilingB + directionB * lp_Time * speedB));
    vec2 waterDistortion = (texNormalA.rg + texNormalB.rg - 1.0) * u_AlbedoDistortionStrength;
    vec4 texAlbedo = texture(u_Albedo, (v_TexCoord * u_TilingAlbedo + directionAlbedo * lp_Time * speedAlbedo) + waterDistortion);

    vec3 normalA = normalize(v_TBNMatrix * (texNormalA.xyz * 2.0 - 1.0));
    vec3 normalB = normalize(v_TBNMatrix * (texNormalB.xyz * 2.0 - 1.0));

    vec3 blendedNormal = normalize(vec3(normalA + normalB) * 0.5);
    vec3 viewDir = normalize(lp_CameraWorldPosLightCount.xyz - v_WorldPos);

    // Foam
    vec2 foamDirection = normalize(directionA + directionB);
    vec2 foamUV = v_TexCoord * u_TilingFoam + foamDirection * lp_Time * speedA;
    vec3 foamSample = texture(u_FoamTexture, foamUV).rgb;
    vec2 screenUV = gl_FragCoord.xy / textureSize(lp_SceneDepth, 0);
    float sceneDepthRaw = texture(lp_SceneDepth, screenUV).r;
    float sceneDepthLinear = linearDepth(sceneDepthRaw, lp_Near, lp_Far);
    float waterDepthLinear = linearDepth(gl_FragCoord.z, lp_Near, lp_Far);
    float depthDiff = sceneDepthLinear  - waterDepthLinear;
    
    float foamDistortion = (texNormalA.r + texNormalB.r) * 0.5;
    float foamAmount = 1.0 - smoothstep(0.0, u_FoamThreshold, depthDiff + foamDistortion * u_FoamDistortionStrength);

    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    for (int i = 0; i < lp_CameraWorldPosLightCount.w; ++i)
    {
        if (int(lp_lights[i].l_PositionType.w) == 0) // Ambiental
        {
            totalDiffuse += lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;
        }
        else if (int(lp_lights[i].l_PositionType.w) == 1) // Directional
        {
            vec3 lightDir = -lp_lights[i].l_DirectionInnerCone.xyz; // toward the light

            // Diffuse
            float diff = max(dot(blendedNormal, lightDir), 0.0);
            vec3 diffuse = diff * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;

            // Specular
            vec3 reflectDir = reflect(-lightDir, blendedNormal);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Roughness);
            vec3 specular = u_Specular * spec * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;  

            totalDiffuse += diffuse;
            totalSpecular += specular;
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
            float diff = max(dot(blendedNormal, lightDir), 0.0);
            vec3 diffuse = diff * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;

            // Specular
            vec3 reflectDir = reflect(-lightDir, blendedNormal);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Roughness);
            vec3 specular = u_Specular * spec * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;  

            float angle = dot(lp_lights[i].l_DirectionInnerCone.xyz, -lightDir);
            float innerAngleCone = cos(radians(lp_lights[i].l_DirectionInnerCone.w));
            float outerAngleCone = cos(radians(lp_lights[i].l_AttenuationOuterCone.w));

            float spotlightAttenuation = smoothstep(outerAngleCone, innerAngleCone, angle);

            totalDiffuse += diffuse * attenuation * spotlightAttenuation;
            totalSpecular += specular * attenuation * spotlightAttenuation;
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
            float diff = max(dot(blendedNormal, lightDir), 0.0);
            vec3 diffuse = diff * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;

            // Specular
            vec3 reflectDir = reflect(-lightDir, blendedNormal);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Roughness);
            vec3 specular = u_Specular * spec * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;  

            totalDiffuse += diffuse * attenuation;
            totalSpecular += specular * attenuation;
        }
    }

    float fresnel = u_Fresnel + (1.0 - u_Fresnel) * pow(1.0 - dot(blendedNormal, viewDir), 5.0);
    float depthFactor = clamp(depthDiff / u_DepthMaxDistance, 0.0, 1.0);
    float depthAlpha = mix(u_MinAlpha, u_MaxAlpha, depthFactor);
    float transparency = clamp(fresnel + depthAlpha, u_MinAlpha, u_MaxAlpha);
    vec3 depthColor = mix(u_ShallowColor, u_DeepColor, depthFactor);
    vec3 surfaceColor = u_UseAlbedo ? texAlbedo.rgb * depthColor : depthColor;
    vec3 waterResult = totalDiffuse * surfaceColor + totalSpecular;
    vec3 finalColor = mix(waterResult, foamSample * u_FoamColor.rgb, foamAmount);
    float finalAlpha = mix(transparency, 1.0, foamAmount);
    FragColor = vec4(finalColor, finalAlpha);
}