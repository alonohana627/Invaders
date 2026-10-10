#version 450
#include "gamelib/sprite.glsl"
#include "gamelib/sdf.glsl"

// An alien bomb, animated as it falls: the rolling bomb twists, the plunger
// pumps a crossbar along its shaft, the squiggly one wriggles.
//   params    x type (0 rolling, 1 plunger, 2 squiggly), y phase (advances
//             with the distance fallen), z glow width (canvas units)
//   color     hot core color; color2 glow color

layout(location = 0) in vec2 inUv;
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 p = spriteLocal(inUv);
    float glow = sprite.params.z;
    vec2 half_ = sprite.size * 0.5 - glow; // the bomb's own half size (7 x 15 by default)
    int type = int(sprite.params.x + 0.5);
    float phase = sprite.params.y;
    float w = half_.x;
    float h = half_.y;
    float r = max(w * 0.28, 1.6);
    float d;
    if (type == 0) {
        // A shaft with a bar that turns around it: its width follows a cosine.
        float shaft = sdSegment(p, vec2(0.0, -h + r), vec2(0.0, h - r), r);
        float c = cos(phase * 6.2831853);
        float y = fract(phase) * 2.0 - 1.0;
        float bar = sdSegment(p, vec2(-w * c, y * h * 0.5 - 3.0), vec2(w * c, y * h * 0.5 + 3.0), r);
        d = min(shaft, bar);
    } else if (type == 1) {
        float shaft = sdSegment(p, vec2(0.0, -h + r), vec2(0.0, h - r), r);
        float y = (abs(fract(phase) * 2.0 - 1.0) * 2.0 - 1.0) * (h - 2.0 * r);
        float bar = sdSegment(p, vec2(-w + r, y), vec2(w - r, y), r);
        d = min(shaft, bar);
    } else {
        // Four segments zig-zagging down, the zigzag sliding with the phase.
        float step_ = 2.0 * h / 4.0;
        float shift = fract(phase) * 2.0;
        d = 1e9;
        for (int i = 0; i < 4; ++i) {
            float k = float(i) + shift;
            vec2 a = vec2((mod(k, 2.0) < 1.0 ? -1.0 : 1.0) * (w - r), -h + r + float(i) * step_);
            vec2 b = vec2(-a.x, a.y + step_ - r * 0.5);
            d = min(d, sdSegment(p, a, b, r));
        }
    }
    vec3 core = mix(sprite.color.rgb, vec3(1.0), 0.3 * (1.0 - smoothstep(0.0, r, -d)));
    vec4 col = vec4(sprite.color2.rgb, 0.7 * glowFalloff(d, glow));
    outColor = over(vec4(core, coverage(d)), col);
}
