uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform int       iFrame;                // shader playback frame
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform samplerXX iChannel0..3;          // input channel. XX = 2D/Cube
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)

float calcWaveFalloff(in float wave_radius, in float coeff, in float power){
    return 1.0/pow((1.0+coeff*wave_radius), power);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    float slowiTime = iTime/4.0;
    // Sawtooth calc of time
    float offset = (slowiTime - floor(slowiTime)) / slowiTime;
    float time = slowiTime * offset;

    // Wave design params
    vec3 waveParams = vec3(10.0, 0.8, 0.1 );

    // Find coordinate, flexible to different resolutions
    float maxSize = max(iResolution.x, iResolution.y);
    vec2 uv = fragCoord.xy / maxSize;

    // Find center, flexible to different resolutions
    vec2 center = iResolution.xy / maxSize / 2.;

    // Distance to the center
    float center_dist = distance(uv, center);

    // Original color
    vec4 texture_color = texture(iChannel0, uv);
    
    vec2 distorted_uv = uv;
    
    float coef1 = 0.01;
    
    float distortion = 2.0;
    
    float wave_radius = time;
    float point_radius = center_dist;
    float radius_diff = point_radius - wave_radius;
    float wave_falloff = calcWaveFalloff(wave_radius, 8.0, 2.0);
    vec2 dir = normalize(uv - center);
    distorted_uv += (distortion*dir*wave_falloff*(coef1/(coef1 + radius_diff)));
    texture_color = texture(iChannel0, distorted_uv);

    fragColor = texture_color;
}