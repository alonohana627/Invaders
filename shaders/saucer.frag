#version 450
#include "gamelib/sprite.glsl"
#include "gamelib/sdf.glsl"

// The mystery ship: a flat hull under a glass dome, with a row of lights
// that chase along the hull.
//   params    x glow width (canvas units), y flash 0..1 (white),
//             z chase phase (the lights move with it)
//   color     hull color; color2 glow color

layout(location = 0) in vec2 inUv;
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 p = spriteLocal(inUv);
    float glow = sprite.params.x;
    float s = (sprite.size.x - 2.0 * glow) / 96.0; // designed 96 x 40
    vec2 q = p / s;
    float hull = sdEllipse(q - vec2(0.0, 5.0), vec2(48.0, 12.0));
    float dome = max(sdEllipse(q - vec2(0.0, -4.0), vec2(21.0, 15.0)), q.y - 2.0);
    float d = min(hull, dome) * s;

    vec3 body = sprite.color.rgb * (1.1 - 0.4 * clamp((q.y + 20.0) / 40.0, 0.0, 1.0));
    // The dome is glass: paler and brighter.
    body = mix(body, mix(sprite.color.rgb, vec3(1.0), 0.55), (1.0 - smoothstep(-1.0, 1.0, dome)) * 0.7);

    // Seven lights along the hull; one bright spot runs through them.
    float slot = clamp(floor((q.x + 35.0) / 10.0 + 0.5), 0.0, 6.0);
    vec2 light = vec2(-35.0 + slot * 10.0, 6.0);
    float lit = 0.25 + 0.75 * step(0.5, fract((slot - sprite.params.z) / 3.5));
    float lamp = sdCircle(q - light, 2.8) * s;
    body = mix(body, mix(vec3(0.05, 0.0, 0.02), vec3(1.0, 0.95, 0.7), lit), coverage(lamp));
    body = mix(body, vec3(1.0), sprite.params.y);

    vec4 col = vec4(sprite.color2.rgb, 0.6 * glowFalloff(d, glow));
    outColor = over(vec4(body, coverage(d)), col);
}
