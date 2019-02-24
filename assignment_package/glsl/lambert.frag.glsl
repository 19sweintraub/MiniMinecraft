#version 330
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D u_Texture;
uniform int u_Time;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec2 fs_UV;
in vec2 fs_AnimCos;
in vec4 fs_Col;

layout(location = 0) out vec4 color; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

void main()
{

    vec4 diffuseColor = texture(u_Texture, fs_UV);

    // if the block is animated, sample the textures to the right as a function of time
    if (fs_AnimCos[0] == 1) {
        vec2 animUV = fs_UV + (1/16.f) * 2 * mod(u_Time/1000.0, 0.5f) * vec2(1, 0);
        animUV.y += (0.5)*sin(4*2*3.14159 + u_Time/100.0) / 150.0;
        diffuseColor = texture(u_Texture, animUV);
    }

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    float ambientTerm = 0.3;

    float exp = fs_AnimCos[1];

    float specularIntensity = max(pow(diffuseTerm, exp), 0);

    float lightIntensity = diffuseTerm + ambientTerm + specularIntensity;   //Add a small float value to the color multiplier
                                                        //to simulate ambient lighting. This ensures that faces that are not
                                                        //lit by our point light are not completely black.


    // Compute final shaded color
    vec4 out_Col = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);


    // Apply fog
    float zVal = clamp(fs_Col[2], 0, 70.0);
    out_Col.rgb = (out_Col.rgb + zVal * vec3(0.015, 0.015, 0.015)) / 2.0;
    //color = out_Col.rgb;

    color = out_Col;

}
