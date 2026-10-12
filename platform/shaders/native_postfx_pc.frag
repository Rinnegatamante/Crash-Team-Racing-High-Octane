in vec2 v_uv;
out vec4 fragColor;
uniform sampler2D s0;
uniform vec4 uCrop;
uniform vec2 uOutputSize;
uniform vec2 uFramebufferSize;
uniform float iTime;
uniform float Gamma;
uniform float Hue;
uniform float Saturation;
uniform float Radius;
uniform float Softness;
uniform float Opacity;
uniform vec3 uGreenRow;
uniform vec3 uBlueRow;

#if POSTFX_EFFECT == 1 || POSTFX_EFFECT == 2
vec2 crtCurve(vec2 uv)
{
    vec2 p = (uv - 0.5) * 2.2;
    p.x *= 1.0 + 0.04 * p.y * p.y;
    p.y *= 1.0 + 0.0625 * p.x * p.x;
    return p * 0.46 + 0.5;
}
#endif

#if POSTFX_EFFECT == 7
vec3 rgbToHsv(vec3 rgb)
{
    const float epsilon = 1e-10;
    vec4 p = mix(vec4(rgb.bg, -1.0, 2.0 / 3.0),
                 vec4(rgb.gb, 0.0, -1.0 / 3.0), step(rgb.b, rgb.g));
    vec4 q = mix(vec4(p.xyw, rgb.r), vec4(rgb.r, p.yzx), step(p.x, rgb.r));
    float c = q.x - min(q.w, q.y);
    float h = abs((q.w - q.y) / (6.0 * c + epsilon) + q.z);
    return vec3(h, c / (q.x + epsilon), q.x);
}

vec3 hsvToRgb(vec3 hsv)
{
    vec3 rgb = clamp(vec3(abs(hsv.x * 6.0 - 3.0) - 1.0,
                          2.0 - abs(hsv.x * 6.0 - 2.0),
                          2.0 - abs(hsv.x * 6.0 - 4.0)), 0.0, 1.0);
    return ((rgb - 1.0) * hsv.y + 1.0) * hsv.z;
}
#endif

void main()
{
    vec2 localUV = (v_uv - uCrop.xy) / uCrop.zw;

#if POSTFX_EFFECT == 1 || POSTFX_EFFECT == 2
    vec2 uv = crtCurve(localUV);
    vec2 fetchUV = uCrop.xy + uv * uCrop.zw;
    vec2 framebufferTexel = 1.0 / uFramebufferSize;
    vec2 sampleMin = uCrop.xy + 0.5 * framebufferTexel;
    vec2 sampleMax = uCrop.xy + uCrop.zw - 0.5 * framebufferTexel;
    vec3 center = texture(s0, clamp(fetchUV, sampleMin, sampleMax)).rgb;
#if POSTFX_EFFECT == 1
    vec3 shifted = texture(s0, clamp(fetchUV + vec2(0.0015, -0.0010) * uCrop.zw,
                                      sampleMin, sampleMax)).rgb;
    vec3 color = vec3(shifted.r, center.g, center.b) * 1.065 + 0.05;
#else
    vec2 bleedOffset = vec2(2.0 * framebufferTexel.x, 0.0);
    vec3 left = texture(s0, clamp(fetchUV - bleedOffset, sampleMin, sampleMax)).rgb;
    vec3 right = texture(s0, clamp(fetchUV + bleedOffset, sampleMin, sampleMax)).rgb;
    vec3 bleed = mix(center, 0.5 * (left + right), 0.42);
    bleed += vec3(dot(center - bleed, vec3(0.299, 0.587, 0.114)));
    vec3 color = bleed * 1.065 + 0.05;
#endif
    color = clamp(color * (0.6 + 0.4 * color), 0.0, 1.0);
    float vig = max(16.0 * uv.x * uv.y * (1.0 - uv.x) * (1.0 - uv.y), 0.0);
    float vignette = sqrt(sqrt(vig));
    float phase = abs(fract(uv.y * uOutputSize.y * 0.2387324146 + 0.25 * iTime) - 0.5);
    float scanline = 0.38 + 0.8 * phase;
    int pixelX = int(floor(gl_FragCoord.x));
    float mask = 1.0 - 0.65 * float(pixelX & 1);
    float valid = step(0.0, uv.x) * step(uv.x, 1.0) * step(0.0, uv.y) * step(uv.y, 1.0);
    fragColor = vec4(color * (vignette * scanline * mask * valid * 2.8) *
                     vec3(0.95, 1.05, 0.95), 1.0);

#elif POSTFX_EFFECT == 3
    vec3 color = texture(s0, v_uv).rgb;
    fragColor = vec4(color.r, dot(color, uGreenRow), dot(color, uBlueRow), 1.0);

#elif POSTFX_EFFECT == 4
    const vec3 luma = vec3(0.299, 0.587, 0.114);
    vec2 texel = 1.0 / uFramebufferSize;
    vec2 sampleMin = uCrop.xy + 0.5 * texel;
    vec2 sampleMax = uCrop.xy + uCrop.zw - 0.5 * texel;
    vec3 nw = texture(s0, clamp(v_uv + vec2(-texel.x, -texel.y), sampleMin, sampleMax)).rgb;
    vec3 ne = texture(s0, clamp(v_uv + vec2( texel.x, -texel.y), sampleMin, sampleMax)).rgb;
    vec3 sw = texture(s0, clamp(v_uv + vec2(-texel.x,  texel.y), sampleMin, sampleMax)).rgb;
    vec3 se = texture(s0, clamp(v_uv + vec2( texel.x,  texel.y), sampleMin, sampleMax)).rgb;
    vec4 sampleColor = texture(s0, v_uv);
    float lumaNW = dot(nw, luma);
    float lumaNE = dot(ne, luma);
    float lumaSW = dot(sw, luma);
    float lumaSE = dot(se, luma);
    float lumaM = dot(sampleColor.rgb, luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    vec2 dir = vec2(-((lumaNW + lumaNE) - (lumaSW + lumaSE)),
                     (lumaNW + lumaSW) - (lumaNE + lumaSE));
    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 / 8.0), 1.0 / 128.0);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, -8.0, 8.0) * texel;
    vec3 rgbA = 0.5 * (
        texture(s0, clamp(v_uv + dir * (1.0 / 3.0 - 0.5), sampleMin, sampleMax)).rgb +
        texture(s0, clamp(v_uv + dir * (2.0 / 3.0 - 0.5), sampleMin, sampleMax)).rgb);
    float contrast = clamp((lumaMax - lumaMin) * 12.0 - 0.1, 0.0, 1.0);
    fragColor = vec4(mix(sampleColor.rgb, rgbA, contrast), sampleColor.a);

#elif POSTFX_EFFECT == 5
    fragColor = vec4(pow(texture(s0, v_uv).rgb, vec3(1.0 / Gamma)), 1.0);

#elif POSTFX_EFFECT == 6
    vec3 color = texture(s0, v_uv).rgb;
    float grey = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b;
    fragColor = vec4(vec3(grey), 1.0);

#elif POSTFX_EFFECT == 7
    vec3 hsv = rgbToHsv(texture(s0, v_uv).rgb);
    hsv.x *= Hue;
    hsv.y *= Saturation;
    fragColor = vec4(hsvToRgb(hsv), 1.0);

#elif POSTFX_EFFECT == 8
    const vec3 subpixelOffset = vec3(0.25, -1.0 / 12.0, -5.0 / 12.0);
    vec2 pixelScale = uOutputSize * vec2(640.0 / 960.0, 480.0 / 544.0);
    vec3 subpixelPhase = fract(localUV.x * pixelScale.x + subpixelOffset - 0.25);
    vec3 subpixelWave = 4.0 * abs(subpixelPhase - 0.5) - 1.0;
    float linePhase = fract(localUV.y * pixelScale.y - 0.25);
    float lineWave = 4.0 * abs(linePhase - 0.5) - 1.0;
    float yFactor = (16.0 + lineWave) / 17.0;
    vec3 xFactor = (4.0 + subpixelWave) / 5.0;
    fragColor = vec4(yFactor * xFactor * texture(s0, v_uv).rgb, 1.0);

#elif POSTFX_EFFECT == 9
    fragColor = vec4(1.0 - texture(s0, v_uv).rgb, 1.0);

#elif POSTFX_EFFECT == 10
    vec3 color = texture(s0, v_uv).rgb;
    fragColor = vec4(dot(color, vec3(0.393, 0.769, 0.189)),
                     dot(color, vec3(0.349, 0.686, 0.168)),
                     dot(color, vec3(0.272, 0.534, 0.131)), 1.0);

#elif POSTFX_EFFECT == 11 || POSTFX_EFFECT == 12
#if POSTFX_EFFECT == 11
    vec2 textureSize = uOutputSize * vec2(640.0 / 960.0, 480.0 / 544.0);
#else
    vec2 textureSize = uOutputSize;
#endif
    vec2 shiftedUV = localUV - vec2(0.25 / textureSize.x, 0.0);
    vec2 localTexel = shiftedUV * textureSize;
    vec2 flooredTexel = floor(localTexel);
    vec2 subpixel = fract(localTexel);
    const float preScale = 1.25;
    const float regionRange = 0.5 - 0.5 / preScale;
    vec2 centerDist = subpixel - 0.5;
    vec2 blendFactor = (centerDist - clamp(centerDist, -regionRange, regionRange)) * preScale + 0.5;
    vec2 adjustedTexel = flooredTexel + blendFactor;
    vec2 halfPixel = 0.5 / uOutputSize;
    vec2 sampleLocal = clamp(adjustedTexel / textureSize, halfPixel, 1.0 - halfPixel);
    vec2 sampleUV = uCrop.xy + sampleLocal * uCrop.zw;
    vec3 color = texture(s0, sampleUV).rgb;
    vec2 phase = fract(vec2(shiftedUV.x * textureSize.x,
                           shiftedUV.y * (uOutputSize.y * 0.5)) - 0.25);
    vec2 wave = 4.0 * abs(phase - 0.5) - 1.0;
    float brightness = 1.0 + dot(vec2(0.1, 0.15), wave);
    fragColor = vec4(color * brightness, 1.0);

#elif POSTFX_EFFECT == 13
    vec3 color = texture(s0, v_uv).rgb;
    float dist = length(localUV - 0.5);
    float vignette = 1.0 - smoothstep(Radius - Softness, Radius, dist);
    fragColor = vec4(mix(color, color * vignette, Opacity), 1.0);

#else
    fragColor = texture(s0, v_uv);
#endif
}
