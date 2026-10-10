#version 450
#include "gamelib/sprite.glsl"
#include "gamelib/sdf.glsl"

// The player's cannon (also the spare cannons in the HUD): a wide base, a
// shoulder and a short barrel, with a bright top edge and a glow.
//   params    x glow width (canvas units), y flash 0..1 (white),
//             z body width (canvas units; the shape scales with it)
//   color     body color; color2 glow color

layout(location = 0) in vec2 inUv;
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 p = spriteLocal(inUv);
    float glow = sprite.params.x;
    float s = sprite.params.z / 78.0; // designed 78 units wide, 44 high
    vec2 q = p / s;
    float base = sdRoundBox(q - vec2(0.0, 9.0), vec2(39.0, 13.0), 6.0);
    float shoulder = sdRoundBox(q - vec2(0.0, -5.0), vec2(25.0, 8.0), 6.0);
    float barrel = sdRoundBox(q - vec2(0.0, -15.0), vec2(5.5, 8.0), 3.0);
    float d = min(min(base, shoulder), barrel) * s;

    // Light from above: the upper edges are brighter.
    float edge = 1.0 - smoothstep(0.0, 3.0 * s, abs(d));
    vec3 body = sprite.color.rgb * (1.1 - 0.35 * clamp((q.y + 22.0) / 44.0, 0.0, 1.0));
    body = mix(body, mix(sprite.color.rgb, vec3(1.0), 0.6), edge * smoothstep(4.0, -10.0, q.y) * 0.8);
    body = mix(body, vec3(1.0), sprite.params.y);
    vec4 col = vec4(sprite.color2.rgb, 0.5 * glowFalloff(d, glow));
    outColor = over(vec4(body, coverage(d)), col);
}
