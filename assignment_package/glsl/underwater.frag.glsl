#version 330

in vec2 fs_UV;

out vec4 out_Col;

uniform sampler2D u_RenderedTexture;
uniform int u_Time;
uniform ivec2 u_Dimensions;

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
}

float worleyNoise(float scale, float lum) {
    vec2 st = fs_UV * scale;
    vec2 gridCord = floor(st);
    vec2 pointCord = fract(st);
    float minDist = lum;

    for (int i= -1; i <= 1; i++) {
        for (int j= -1; j <= 1; j++) {

            // Find neighbor
            vec2 neighbor = vec2(float(j),float(i));

            // generate random point in the cell
            vec2 point = random2(gridCord + neighbor);

            // animate point
            //point = 0.5 + 0.5*sin(u_Time/10.0 + 6.2831*point);

            // establish vector between random point and the coordinate
            vec2 diff = neighbor + point - pointCord;

            // Distance to the point
            float dist = length(diff);

            // Keep the closer distance
            if (dist < minDist) {
                minDist = dist;
            }
        }
    }

    return minDist;
}

void main()
{

    // Water Motion
    vec2 samp = fs_UV;
    float dist = worleyNoise(6.0, 1.0);
    samp.x += dist*sin(samp.y * 3.14159 + u_Time/70.0) / 50.0;
    samp.x = clamp(samp.x, 0, 1);

    vec3 color = vec3(0, 0, 0);

    // Gaussian Kernel for blurr
    float kernel[121] = float[](
            0.006849,	0.007239,	0.007559,	0.007795,	0.007941,	0.00799,	0.007941,	0.007795,	0.007559,	0.007239,	0.006849,
            0.007239,	0.007653,	0.00799,	0.00824,	0.008394,	0.008446,	0.008394,	0.00824,	0.00799,	0.007653,	0.007239,
            0.007559,	0.00799,	0.008342,	0.008604,	0.008764,	0.008819,	0.008764,	0.008604,	0.008342,	0.00799,	0.007559,
            0.007795,	0.00824,	0.008604,	0.008873,	0.009039,	0.009095,	0.009039,	0.008873,	0.008604,	0.00824,	0.007795,
            0.007941,	0.008394,	0.008764,	0.009039,	0.009208,	0.009265,	0.009208,	0.009039,	0.008764,	0.008394,	0.007941,
            0.00799,	0.008446,	0.008819,	0.009095,	0.009265,	0.009322,	0.009265,	0.009095,	0.008819,	0.008446,	0.00799,
            0.007941,	0.008394,	0.008764,	0.009039,	0.009208,	0.009265,	0.009208,	0.009039,	0.008764,	0.008394,	0.007941,
            0.007795,	0.00824,	0.008604,	0.008873,	0.009039,	0.009095,	0.009039,	0.008873,	0.008604,	0.00824,	0.007795,
            0.007559,	0.00799,	0.008342,	0.008604,	0.008764,	0.008819,	0.008764,	0.008604,	0.008342,	0.00799,	0.007559,
            0.007239,	0.007653,	0.00799,	0.00824,	0.008394,	0.008446,	0.008394,	0.00824,	0.00799,	0.007653,	0.007239,
            0.006849,	0.007239,	0.007559,	0.007795,	0.007941,	0.00799,	0.007941,	0.007795,	0.007559,	0.007239,	0.006849);

    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {

            // convert to pixel space
            int x = int(samp[0] * u_Dimensions[0]);
            int y = int(samp[1] * u_Dimensions[1]);
            vec3 colorSamp = texelFetch(u_RenderedTexture, ivec2(x, y) + ivec2(i-5, j-5), 0 ).rgb;
            color += kernel[i + 11*j]*colorSamp;
        }
    }

    out_Col = vec4(color, 1);
    out_Col.b += 0.4;

}
