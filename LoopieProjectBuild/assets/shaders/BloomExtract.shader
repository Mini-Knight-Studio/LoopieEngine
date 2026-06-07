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

uniform sampler2D lp_hdrBuffer;
uniform float lp_BloomThreshold;

void main()
{
    vec3 hdr = texture(lp_hdrBuffer, v_TexCoord).rgb;
    float luma = dot(hdr, vec3(0.2126, 0.7152, 0.0722)); // This is the standard perceptual luminance coefficients
    vec3 result = (luma > lp_BloomThreshold) ? hdr : vec3(0.0);

    FragColor = vec4(result, 1.0);
}