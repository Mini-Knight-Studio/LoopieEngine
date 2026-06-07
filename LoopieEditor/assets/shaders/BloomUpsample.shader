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
void main()
{
    FragColor = vec4(texture(lp_SourceTexture, v_TexCoord).rgb, 1.0);
}