#version 450
#include "gamelib/sprite.glsl"
#include "gamelib/sdf.glsl"

// One alien: a glowing creature in two animation frames, drawn from signed
// distances (no bitmaps). The quad is the alien's rectangle grown by the glow.
//   params    x type (0 squid, 1 crab, 2 octopus), y frame (0 or 1),
//             z glow width (canvas units), w flash 0..1 (white)
//   color     body color; color2 glow color

layout(location = 0) in vec2 inUv;
layout(location = 0) out vec4 outColor;

// Mirrors x, so every creature is drawn from its right half.
float limb(vec2 q, vec2 a, vec2 b, float r)
{
    return sdSegment(vec2(abs(q.x), q.y), a, b, r);
}

// The squid: a pointed head over a short skirt, eyes as holes, two pairs of
// tentacles that swing out and in.
float squid(vec2 q, float frame)
{
    vec2 h = q - vec2(0.0, -0.22);
    float c = 0.70710678;
    float head = sdRoundBox(vec2(c * (h.x - h.y), c * (h.x + h.y)), vec2(0.5), 0.14);
    float skirt = sdRoundBox(q - vec2(0.0, 0.22), vec2(0.78, 0.2), 0.12);
    float body = min(head, skirt);
    float legs = frame < 0.5 ? min(limb(q, vec2(0.55, 0.35), vec2(0.8, 0.88), 0.13), limb(q, vec2(0.18, 0.38), vec2(0.3, 0.86), 0.12))
                             : min(limb(q, vec2(0.55, 0.35), vec2(0.36, 0.9), 0.13), limb(q, vec2(0.18, 0.38), vec2(0.62, 0.84), 0.12));
    float d = min(body, legs);
    float eyes = sdCircle(vec2(abs(q.x) - 0.27, q.y + 0.02), 0.13);
    return max(d, -eyes);
}

// The crab: a rounded body with antennae, claws raised in one frame and
// lowered in the other, two short legs.
float crab(vec2 q, float frame)
{
    float body = sdRoundBox(q - vec2(0.0, 0.02), vec2(0.78, 0.42), 0.26);
    float antennae = min(limb(q, vec2(0.3, -0.36), vec2(0.55, -0.82), 0.1), sdCircle(vec2(abs(q.x) - 0.58, q.y + 0.86), 0.12));
    float claws = frame < 0.5 ? limb(q, vec2(0.75, 0.0), vec2(1.18, -0.58), 0.14) : limb(q, vec2(0.75, 0.08), vec2(1.18, 0.62), 0.14);
    float legs = frame < 0.5 ? limb(q, vec2(0.4, 0.38), vec2(0.6, 0.86), 0.12) : limb(q, vec2(0.4, 0.38), vec2(0.18, 0.86), 0.12);
    float d = min(min(body, antennae), min(claws, legs));
    float eyes = sdCircle(vec2(abs(q.x) - 0.32, q.y + 0.04), 0.13);
    return max(d, -eyes);
}

// The octopus: a wide dome on four pairs of legs that spread and pinch.
float octopus(vec2 q, float frame)
{
    float dome = max(sdEllipse(q - vec2(0.0, -0.05), vec2(1.18, 0.62)), q.y - 0.3);
    float legs = frame < 0.5 ? min(limb(q, vec2(0.28, 0.25), vec2(0.55, 0.88), 0.13), limb(q, vec2(0.85, 0.2), vec2(1.3, 0.78), 0.13))
                             : min(limb(q, vec2(0.28, 0.25), vec2(0.1, 0.88), 0.13), limb(q, vec2(0.85, 0.2), vec2(0.68, 0.88), 0.13));
    float d = min(dome, legs);
    float eyes = sdCircle(vec2(abs(q.x) - 0.42, q.y + 0.12), 0.15);
    return max(d, -eyes);
}

void main()
{
    vec2 p = spriteLocal(inUv);
    float glow = sprite.params.z;
    float halfHeight = sprite.size.y * 0.5 - glow; // the body spans -1..1 in q.y
    vec2 q = p / halfHeight;
    int type = int(sprite.params.x + 0.5);
    float frame = sprite.params.y;
    float dq = type == 0 ? squid(q, frame) : (type == 1 ? crab(q, frame) : octopus(q, frame));
    float d = dq * halfHeight; // back to canvas units for coverage and glow

    // Body: brighter towards the top, a thin bright rim, the flash on top.
    vec3 body = sprite.color.rgb * (1.15 - 0.3 * (q.y * 0.5 + 0.5));
    float rim = 1.0 - smoothstep(0.0, 2.5, abs(d));
    body = mix(body, mix(sprite.color.rgb, vec3(1.0), 0.55), rim * 0.6);
    body = mix(body, vec3(1.0), sprite.params.w);
    vec4 col = vec4(sprite.color2.rgb, 0.55 * glowFalloff(d, glow));
    col = over(vec4(body, coverage(d)), col);
    outColor = col;
}
