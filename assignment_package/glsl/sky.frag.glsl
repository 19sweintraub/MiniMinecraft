#version 330

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform int u_Time;

//out vec4 outColor;
layout(location = 0) out vec4 outColor;
in vec2 fs_UV;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;
const int DAY_CYCLE_ITERATIONS = 8000; // 8,000 time steps for a complete 24 hour day cycle
                                         // around 2 minutes in real life

// Sunrise palette
// Sunrise color palette inspired by my sunrise photo: https://photos.app.goo.gl/h3ni441jrapvqM419
// Fourth channel is for transparency alpha, it should be 1
const vec4 sunrise[5] = vec4[](vec4(255, 206, 121, 255) / 255.0, // golden yellow
                              vec4(255, 230, 176, 255) / 255.0, // yellow
                              vec4(250, 238, 226, 255) / 255.0, // light yellow
                              vec4(214, 225, 229, 255) / 255.0, // lighter blue
                              vec4(159, 197, 218, 255) / 255.0); // light blue

// Noon palette
// Noon colors inspired by my daytime panorama: http://phillybydrone.com/panoramas/sora-west/
const vec4 noon[5] = vec4[](vec4(209, 227, 245, 255) / 255.0, // lighest blue
                            vec4(187, 214, 243, 255) / 255.0, // lighter blue
                            vec4(171, 200, 240, 255) / 255.0, // light blue
                            vec4(138, 179, 231, 255) / 255.0, // blue
                            vec4(78, 148, 220, 255) / 255.0); // darker blue

// Sunset palette
// Dusk colors inspired by my dusk panorama: http://phillybydrone.com/panoramas/1213-walnut/
const vec4 sunset[5] = vec4[](vec4(229, 187, 227, 255) / 255.0, // pink
                            vec4(193, 163, 225, 255) / 255.0, // light purple
                            vec4(139, 136, 213, 255) / 255.0, // purple
                            vec4(82, 110, 194, 255) / 255.0, // blue
                            vec4(44, 91, 169, 255) / 255.0); // darker blue

// Midnight palette
// Normally we might think of midnight as pure black, but I think that realistically there is still some color in the sky
// Inspiration photo: https://www.instagram.com/p/BqjIVTxFhPXA1j4Hgv_MkNWV7Tjyo_PGgLqbvA0/
const vec4 midnight[5] = vec4[](vec4(2, 39, 81, 255) / 255.0, // blue
                            vec4(0, 31, 75, 255) / 255.0, // darker blue
                            vec4(3, 22, 55, 255) / 255.0, // even darker blue
                            vec4(1, 15, 42, 255) / 255.0, // even even darker blue
                            vec4(2, 14, 32, 255) / 255.0); // darkest blue, but not yet quite black



// Sun color taken from here: https://img.purch.com/h/1400/aHR0cDovL3d3dy5saXZlc2NpZW5jZS5jb20vaW1hZ2VzL2kvMDAwLzA5MC80MjQvb3JpZ2luYWwvc3Vuc2hpbmUuanBlZw==
// During noon, it is true that the sun appears white
const vec4 sunColor = vec4(255, 255, 255, 255) / 255.0;

// 4 cases of sun directions
const vec3 sunriseSunDir = vec3(-1.0, 0.0, 0.0); // east
const vec3 noonSunDir = vec3(0.0, 1.0, 0.0); // above
const vec3 sunsetSunDir = vec3(1.0, 0.0, 0.0); // west
const vec3 midnightSunDir = vec3(0.0, -1.0, 0.0); // below

vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x);
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y);
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

vec4 uvToColor(vec2 uv, vec4[5] color_palette) {
    // Assuming that uv that is input has values between 0 and 1
    if(uv.y < 0.5) {
        return color_palette[0];
    }
    else if(uv.y < 0.55) {
        return mix(color_palette[0], color_palette[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(color_palette[1], color_palette[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(color_palette[2], color_palette[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(color_palette[3], color_palette[4], (uv.y - 0.65) / 0.1);
    }
    else if(uv.y <= 1.00) {
        return color_palette[4];
    }
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}


// Source for 3d noise function: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float mod289(float x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 perm(vec4 x)
{
    return mod289(((x * 34.0) + 1.0) * x);
}

float noise3D(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = vec4(a.x, a.x, a.y, a.y) + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + vec4(a.z, a.z, a.z, a.z);
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    float output_float = o4.y * d.y + o4.x * (1.0 - d.y);
    // To animate clouds slowly, apply the u_Time variable
    // sin is normally in the -1 to 1 range. So multiplying by 0.5 then adding 0.5 shifts to 0 to 1 range
    output_float = 0.5 + 0.5 * sin(u_Time * 0.003 + TWO_PI * output_float);
    return output_float;
}

float noiseFBM(vec3 uv) {
    float sum = 0;
    float freq = 4;
    float amp = 0.5;
    for(int i = 0; i < 8; i++) {
        sum += noise3D(uv * freq) * amp;
        freq *= 2;
        amp *= 0.5;
    }
    return sum;
}

void main()
{
    // gl_FragCoord is window-relative coordinates of the current coordinate
    // Source: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/gl_FragCoord.xhtml
    vec2 ndc = (gl_FragCoord.xy / vec2(u_Dimensions)) * 2.0 - 1.0; // -1 to 1 NDC

    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    vec3 rayDir = normalize(p.xyz - u_Eye);
    vec2 uv = sphereToUV(rayDir);

    vec2 noiseSlope = vec2(noiseFBM(rayDir));
    noiseSlope *= 2.0;
    noiseSlope -= vec2(1.5); // A more negative number moves the clouds higher in the sky
    noiseSlope *= 0.1; // A smaller multiplication factor makes the clouds more detailed

    vec2 uv_with_noise = uv + noiseSlope;

    // Different colors for different times of day. Will use these to interpolate based on time
    vec4 sunriseColor = uvToColor(uv_with_noise, sunrise);
    vec4 noonColor = uvToColor(uv_with_noise, noon);
    vec4 sunsetColor = uvToColor(uv_with_noise, sunset);
    vec4 midnightColor = uvToColor(uv_with_noise, midnight);

    vec3 sunDir = vec3(0, 0, 0);
    float u_Time_mod = u_Time % DAY_CYCLE_ITERATIONS; // some value between 0 and 8000 I think
    if (u_Time_mod < 2000) { // between sunrise to noon (6am to 12pm)
        float interp_amount = (u_Time_mod - 0.0) / 2000.0;
        sunDir = mix(sunriseSunDir, noonSunDir, interp_amount);
        // interpolate outColor between sunrise (sunset) and noon
        outColor = mix(sunriseColor, noonColor, interp_amount);
    } else if (u_Time_mod < 4000) { // between noon to sunset (12pm - 6pm)
        float interp_amount = (u_Time_mod - 2000.0) / 2000.0;
        sunDir = mix(noonSunDir, sunsetSunDir, interp_amount);
        outColor = mix(noonColor, sunsetColor, interp_amount);
    } else if (u_Time_mod < 6000) { // between sunset to midnight (6pm - 12am)
        float interp_amount = (u_Time_mod - 4000.0) / 2000.0;
        sunDir = mix(sunsetSunDir, midnightSunDir, interp_amount);
        outColor = mix(sunsetColor, midnightColor, interp_amount);
    } else if (u_Time_mod < 8000) { // between midnight to sunrise (12am - 6am)
        float interp_amount = (u_Time_mod - 6000.0) / 2000.0;
        sunDir = mix(midnightSunDir, sunriseSunDir, interp_amount);
        outColor = mix(midnightColor, sunriseColor, interp_amount);
    }
    sunDir = normalize(sunDir);

    float sunSize = 30; // in degrees, for the dot product angle
    float angle = acos(dot(rayDir, sunDir)) * 360.0 / PI; // angle between 0 and 180 degrees. Positive.

    if (angle < sunSize) {
        // This is angle from center of sun to 30 degrees outside center of sun
        if (angle < 7.5) {
            outColor = sunColor;
        }
        else { // angle is between 7.5 and 30.0
            outColor = mix(sunColor, outColor, (angle - 7.5) / 22.5);
        }
    }
    // I made a conscious decision to take out the functionality that renders
    // half of the sky (opposite the direction of sunrise or sunset) as darker,
    // because I feel that in many sunrises and sunsets the colors are generally
    // pretty similar throughout. It seemed to be an extra amount of hard coding
    // that would not make the world look as nice, as I feel that the aesthetics
    // of a uniformly colored sky is much nicer than a sky with different halves.
}
