#version 330

in vec2 fs_UV;

out vec4 out_Col;

uniform sampler2D u_RenderedTexture;
uniform int u_Time;
uniform ivec2 u_Dimensions;

void main()
{

    // No-Op
    out_Col = texture(u_RenderedTexture, fs_UV);

}
