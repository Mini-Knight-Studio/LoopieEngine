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

uniform sampler2D lp_SourceTexture;
uniform vec2 lp_SrcTexelSize; 

void main()
{
    // 4-tap box: sample at the corners of the destination texel,
    // each tap landing between source texels for free bilinear averaging
    vec2 o = lp_SrcTexelSize;
    vec3 c  = texture(lp_SourceTexture, v_TexCoord + vec2(-o.x, -o.y)).rgb;
    c += texture(lp_SourceTexture, v_TexCoord + vec2( o.x, -o.y)).rgb;
    c += texture(lp_SourceTexture, v_TexCoord + vec2(-o.x,  o.y)).rgb;
    c += texture(lp_SourceTexture, v_TexCoord + vec2( o.x,  o.y)).rgb;
    FragColor = vec4(c * 0.25, 1.0);
}