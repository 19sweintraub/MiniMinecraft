#version 330
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform vec4 u_Color;       // When drawing the cube instance, we'll set our uniform color to represent different block types.
uniform int u_Time;

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec2 vs_UV;              // The array of vertex texture coordinates passed to the shader

in vec2 vs_AnimCos;

in vec2 vs_Col;            // using this to pass the z coordinate...

out vec2 fs_UV;             // The UV of each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec2 fs_AnimCos;
out vec4 fs_Col;


// sun direction as a function of u_Time
// If this function is changed here, also need to change in lambert.vert.glsl for actual light direction
vec3 getSunDir() {
    // Want sun to be at sunrise at u_Time = 0
    // Want sun to be at noon at u_Time = 2000 (directly overhead)
    // Want sun to be at sunset at u_Time = 4000
    vec3 sunDir = vec3(0, 0, 1);
    int dayCycleIterations = 8000; // 8000 iterations of time for a day, if a day cycle is around 2 minutes in real life

    // 4 cases of sun directions
    const vec3 sunriseSunDir = vec3(-1.0, 0.0, 0.0); // east
    const vec3 noonSunDir = vec3(0.0, 1.0, 0.0); // above
    const vec3 sunsetSunDir = vec3(1.0, 0.0, 0.0); // west
    const vec3 midnightSunDir = vec3(0.0, -1.0, 0.0); // below

    float u_Time_mod = u_Time % dayCycleIterations; // some value between 0 and 8000 I think
    if (u_Time_mod < 2000) {
        sunDir = mix(sunriseSunDir, noonSunDir, (u_Time_mod - 0.0) / 2000.0);
    } else if (u_Time_mod < 4000) {
        sunDir = mix(noonSunDir, sunsetSunDir, (u_Time_mod - 2000.0) / 2000.0);
    } else if (u_Time_mod < 6000) {
        sunDir = mix(sunsetSunDir, midnightSunDir, (u_Time_mod - 4000.0) / 2000.0);
    } else if (u_Time_mod < 8000) {
        sunDir = mix(midnightSunDir, sunriseSunDir, (u_Time_mod - 6000.0) / 2000.0);
    }
    return normalize(sunDir);
}

void main()
{
    fs_UV = vs_UV;
    fs_AnimCos = vs_AnimCos;

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.


    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below

    fs_LightVec = vec4(getSunDir(), 0);  // The direction of our virtual light, which is used to compute the shading of
                                         // the geometry in the fragment shader.

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices

    // using this to store the z coordinate of the frag for application of fog
    fs_Col = gl_Position;

//    fs_UV = vec2(1, 1);
//    fs_Nor = vec4(1, 1, 1, 1);
//    fs_LightVec = vec4(1, 1, 1, 1);
//    fs_AnimCos = vec2(1, 1);
//    fs_Col = vec4(1, 1, 1, 1);
}
