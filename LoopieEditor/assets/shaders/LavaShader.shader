[vertex]
#version 460 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_Normal;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec4 a_Color;

struct Light
{
    vec4 l_ColorIntensity; // Color + Intensity
    vec4 l_PositionType; // Position + Type
    vec4 l_DirectionInnerCone; // Direction + Inner Cone Angle
    vec4 l_AttenuationOuterCone; // Attenuation + Outer Cone Angle
    vec4 l_SMapAndSColor; // ShadowMap number + Shadow Color
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
    vec4 l_ColorIntensity; // Color + Intensity
    vec4 l_PositionType; // Position + Type
    vec4 l_DirectionInnerCone; // Direction + Inner Cone Angle
    vec4 l_AttenuationOuterCone; // Attenuation + Outer Cone Angle
    vec4 l_SMapAndSColor; // ShadowMap number + Shadow Color
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

uniform sampler2D u_LavaMapA; // The two scrolling normal maps that drive the molten surface ripple and specular
uniform sampler2D u_LavaMapB;
uniform sampler2D u_Emissive; // The glow texture (the bright cracks)

// Foam
uniform sampler2D lp_SceneDepth; 
uniform vec4 u_FoamColor = vec4(1.0, 0.85, 0.6, 1.0);
uniform float u_FoamDistortionStrength = 0.18; // Amplitude of the per-fragment edge ripple. Small = clean shoreline, Large = jagged / inconsistent
uniform float u_FoamFadeDistance = 0.82; // How far from the shore the foam band extends
uniform float u_FoamFalloff = 0.89; // How sharply foam fades across the band. High = thin hard edge, low = soft wide gradient
uniform vec2 u_FoamScroll = vec2(0.22, 0.2); // Speed and direction the noise field drifts. AKA "how fast does it move"
uniform float u_FoamNoiseScale = 1.5; // Spatial frequency of the froth pattern. High = fine small cells, low = large blobs
uniform float u_FoamBreathSpeed = 2.2; // Rate of the global intensity pulse over time
uniform float u_FoamMin = 0.5; // Min foam opacity at shoreline
uniform float u_FoamMax = 0.900; // Max foam opacity at shoreline
uniform float u_FoamPhaseScale = 0.03; // How much the breath phase varies across space. 0 = whole rim pulses in unison, high = traveling wave along the shore

uniform vec2 u_TilingA= vec2(1); // UV repeat count per map. Higher = smaller, denser pattern
uniform vec2 u_TilingB= vec2(1); // UV repeat count per map. Higher = smaller, denser pattern
uniform vec2 u_TilingEmissive = vec2(1); // UV repeat count per map. Higher = smaller, denser pattern

uniform vec2 directionA = vec2(0.05, 0.1); //  Scroll direction of each normal map.
uniform vec2 directionB = vec2(-0.08, 0.06); //  Scroll direction of each normal map.

uniform float speedA = 0.05; // Scroll speed of each normal map
uniform float speedB = 0.12; // Scroll speed of each normal map

uniform float u_Shininess = 32.0; // Highlight, smaller value = broader spotlight (feels more shiny)

uniform vec4 u_Color = vec4(1.0); // Base surface color, multiplied into the lit (non-glowing) lava
uniform vec3 u_Specular = vec3(0.5); // Global UV scale applied to everything

uniform float u_EmissiveIntensity = 0.5;

uniform float lp_Time;

vec3 ReconstructWorldPos(vec2 uv, float depth) 
{
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 world = inverse(lp_Projection * lp_View) * ndc;
    return world.xyz / world.w;   
}

float hash(vec2 p) 
{
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float valueNoise(vec2 p) 
{
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash(i), b = hash(i + vec2(1,0));
    float c = hash(i + vec2(0,1)), d = hash(i + vec2(1,1));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

void main()
{

    vec4 texNormalA = texture(u_LavaMapA, (v_TexCoord * u_TilingA + directionA * lp_Time * speedA));
    vec4 texNormalB = texture(u_LavaMapB, (v_TexCoord * u_TilingB + directionB * lp_Time * speedB));
    vec2 heatDistortion = (texNormalA.rg + texNormalB.rg - 1.0) * 0.02;
    vec4 texEmissive = texture(u_Emissive, (v_TexCoord * u_TilingEmissive + directionB * lp_Time * speedB) + heatDistortion);

    vec3 normalA = normalize(v_TBNMatrix * (texNormalA.xyz * 2.0 - 1.0));
    vec3 normalB = normalize(v_TBNMatrix * (texNormalB.xyz * 2.0 - 1.0));

    vec3 blendedNormal = normalize(vec3(normalA + normalB) * 0.5);
    vec3 viewDir = normalize(lp_CameraWorldPosLightCount.xyz - v_WorldPos);

    // Foam
    vec2 screenUV = gl_FragCoord.xy / textureSize(lp_SceneDepth, 0);
    float sceneDepthRaw = texture(lp_SceneDepth, screenUV).r;

    vec3 belowPos = ReconstructWorldPos(screenUV, sceneDepthRaw);   
    vec3 surfNormal = normalize(v_TBNMatrix[2]);                    
    float submerged = dot(v_WorldPos - belowPos, surfNormal);       

    vec2 uv1 = v_TexCoord * u_FoamNoiseScale       + u_FoamScroll        * lp_Time;
    vec2 uv2 = v_TexCoord * u_FoamNoiseScale * 2.1 - u_FoamScroll.yx * 1.6 * lp_Time + 17.0;
    float fbm    = valueNoise(uv1) * 0.6 + valueNoise(uv2) * 0.4;
    float wobble = (fbm - 0.5) * 2.0 * u_FoamDistortionStrength; 

    float band = pow(1.0 - clamp((submerged + wobble) / u_FoamFadeDistance, 0.0, 1.0), u_FoamFalloff);

    float phase = dot(v_WorldPos.xz, vec2(u_FoamPhaseScale));
    float breath = sin(lp_Time * u_FoamBreathSpeed + phase) * 0.6 + sin(lp_Time * u_FoamBreathSpeed * 1.93 + phase) * 0.4;
    float breath01 = breath * 0.5 + 0.5;

    float intensity  = mix(u_FoamMin, u_FoamMax, breath01);
    float foamAmount = band * intensity;

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
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess);
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
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess);
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
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess);
            vec3 specular = u_Specular * spec * lp_lights[i].l_ColorIntensity.xyz * lp_lights[i].l_ColorIntensity.w;  

            totalDiffuse += diffuse * attenuation;
            totalSpecular += specular * attenuation;
        }
    }

    float pulse = 1.0 + 0.15 * sin(lp_Time * 1.5); // 0.15 = amplitude, 1.5 = time. Touch as needed.
    vec3 result = totalDiffuse * u_Color.xyz + totalSpecular;
    float emissiveMask = clamp(length(texEmissive.rgb), 0.0, 1.0);
    result = mix(result, texEmissive.rgb * u_EmissiveIntensity * pulse, emissiveMask);
    result = mix(result, u_FoamColor.rgb, foamAmount);
    FragColor = vec4(result, 1.0);
}