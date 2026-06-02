[vertex]
#version 460 core

out vec2 v_UV;
void main()
{
    vec2 uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    v_UV = uv;
    gl_Position = vec4(uv * 2.0 - 1.0, 0.0, 1.0);
}

[fragment]
#version 460 core

in vec2 v_UV;
out vec4 FragColor;

uniform sampler2D lp_HDRBuffer;
uniform float lp_Exposure;

// Normal -> Lava visible up to 47.0
//void main()
//{
//    vec3 color = texture(lp_HDRBuffer, v_UV).rgb;
//    FragColor = vec4(color, 1.0);
//}

// Reinhard -> 1.0 = 0.5 -> Lava visible up to 800 ish
//void main()
//{
//    vec3 hdr = texture(lp_HDRBuffer, v_UV).rgb;
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
    vec3 hdr = texture(lp_HDRBuffer, v_UV).rgb;
    vec3 mapped = hdr * lp_Exposure;
    mapped = ACESFilm(mapped);
    FragColor = vec4(mapped, 1.0);
}