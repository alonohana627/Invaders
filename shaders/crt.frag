#version 450
#include "gamelib/sprite.glsl"
#include "gamelib/noise.glsl"
#include "gamelib/sdf.glsl"

// The cabinet's screen, in two passes over the whole canvas.
// Background (mode 0): a deep blue-black tube with a phosphor haze in the
// middle and the colored bands of the arcade's cellophane overlay - red over
// the mystery ship's lane, green over the shields and the cannon.
// Overlay (mode 1, drawn over the field): scanlines, a slow bright refresh
// bar and the dark curved corners of the tube.
//   params    x mode, y top of the green band, z bottom of the red band
//             (canvas units), w a flash 0..1 that brightens the screen
//   color     screen color; color2 haze color

layout(location = 0) in vec2 inUv;
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 p = inUv * sprite.size + sprite.position; // canvas units
    vec2 v = (inUv - 0.5) * vec2(1.6, 1.0);
    float r2 = dot(v, v);
    if (sprite.params.x < 0.5) {
        vec3 col = sprite.color.rgb + sprite.color2.rgb * 0.55 * max(0.0, 1.0 - r2 * 1.4);
        // Cellophane bands, with soft edges.
        float red = 1.0 - smoothstep(sprite.params.z - 30.0, sprite.params.z + 30.0, p.y);
        float green = smoothstep(sprite.params.y - 40.0, sprite.params.y + 40.0, p.y);
        col += vec3(0.022, 0.0, 0.006) * red + vec3(0.0, 0.016, 0.007) * green;
        col += vec3(0.25, 0.35, 0.5) * sprite.params.w * 0.15;
        outColor = vec4(dither(col, gl_FragCoord.xy, 0.06), 1.0);
        return;
    }
    // Overlay: darkness only (straight alpha over the field), plus a faint
    // light bar. Scanlines every 3 framebuffer pixels, so they stay crisp
    // at any window size.
    float scan = 0.5 + 0.5 * cos(gl_FragCoord.y * 2.0943951);
    float dark = 0.10 * scan;
    // The tube's corners: dark beyond a rounded rectangle.
    vec2 c = abs(inUv - 0.5) * 2.0;
    float corner = smoothstep(0.82, 1.02, length(max(c - vec2(0.7, 0.55), 0.0) / vec2(0.3, 0.45)));
    dark = max(dark, 0.85 * corner);
    // The refresh bar: a soft band of light rolling down every 9 seconds,
    // drawn over the darkness (never inside the dark corners).
    float bar = 1.0 - smoothstep(0.0, 0.12, abs(fract(sprite.time * 0.11) * 1.4 - 0.2 - inUv.y));
    vec4 light = vec4(0.7, 0.85, 1.0, bar * 0.022 * (1.0 - corner));
    outColor = over(light, vec4(0.0, 0.0, 0.0, dark));
}
