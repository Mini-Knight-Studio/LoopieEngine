[vertex]
#version 460 core

out vec2 v_TexCoord;
void main()
{
    vec2 uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    v_TexCoord = uv;
    gl_Position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
}

[fragment]
#version 460 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D lp_HDRBuffer;
uniform sampler2D lp_BloomBuffer;
uniform sampler2D lp_SceneDepth;
uniform float lp_Exposure;
uniform float lp_BloomStrength;

uniform mat4  lp_InvProjection; 
uniform mat4  lp_InvView;

uniform vec3  lp_FogColor;
uniform float lp_FogStart;
uniform float lp_FogEnd;
uniform float lp_FogEnabled;   
uniform float lp_FogHeightTop;
uniform float lp_FogHeightBottom;
uniform float lp_FogHeightStrength;

// Normal -> Lava visible up to 47.0
//void main()
//{
//    vec3 color = texture(lp_HDRBuffer, v_TexCoord).rgb;
//    FragColor = vec4(color, 1.0);
//}

// Reinhard -> 1.0 = 0.5 -> Lava visible up to 800 ish
//void main()
//{
//    vec3 hdr = texture(lp_HDRBuffer, v_TexCoord).rgb;
//    vec3 mapped = hdr * lp_Exposure;         
//    mapped = mapped / (mapped + vec3(1.0));  
//    FragColor = vec4(mapped, 1.0);
//}

// Aces -> 1.0 = 0.8~ -> Lava visible up to 150
vec3 ACESFilm(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 hdr   = texture(lp_HDRBuffer, v_TexCoord).rgb;
    vec3 bloom = texture(lp_BloomBuffer, v_TexCoord).rgb;
    //vec3 combined = mix(hdr, bloom, lp_BloomStrength);  // -> more artistical
    vec3 combined = hdr + bloom * lp_BloomStrength;  // -> more physically reallistic
    combined *= lp_Exposure;
    combined = ACESFilm(combined);

    // Distance fog, applied AFTER ACES so the fog color is exactly what we decide it to.
    float depth  = texture(lp_SceneDepth, v_TexCoord).r;                 
    vec4  ndc    = vec4(v_TexCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0); 
    vec4  viewPos = lp_InvProjection * ndc;
    viewPos /= viewPos.w;                                           
    float viewDist = viewPos.z;  

    float fogFactor = clamp((viewDist - lp_FogStart) / (lp_FogEnd - lp_FogStart), 0.0, 1.0);
    vec4 worldPos = lp_InvView * viewPos;   
    float heightFog = clamp((lp_FogHeightTop - worldPos.y) / (lp_FogHeightTop - lp_FogHeightBottom), 0.0, 1.0) * lp_FogHeightStrength;
    
    fogFactor = max(fogFactor, heightFog);

    combined = mix(combined, lp_FogColor, fogFactor * lp_FogEnabled);

    FragColor = vec4(combined, 1.0);
}