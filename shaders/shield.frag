#version 450
#include "gamelib/sprite.glsl"
#include "gamelib/noise.glsl"

// One row of a shield: up to 32 cells, each with a strength of 0 (gone) to 3
// (intact) in two bits of `bits` (cell 0 in the lowest bits). The quad is
// exactly the row, and every pixel is either a cell or nothing, so rows and
// cells join without anti-aliased seams; erosion shows cell by cell. Each
// cell is lit like a phosphor dot: brighter in its middle.
//   params    x cell size (canvas units), y row index (varies the crumbs)
//   color     intact color; color2 crumbled color

layout(location = 0) in vec2 inUv;
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 local = inUv * sprite.size; // from the row's top-left
    float cell = sprite.params.x;
    uint index = uint(clamp(floor(local.x / cell), 0.0, 31.0));
    uint word = index < 16u ? sprite.bits.x : sprite.bits.y;
    uint strength = (word >> (2u * (index & 15u))) & 3u;
    if (strength == 0u) {
        outColor = vec4(0.0);
        return;
    }
    vec2 inCell = (fract(local / cell) - 0.5) * 2.0; // -1..1 inside the cell
    float dot_ = 1.0 - 0.28 * dot(inCell, inCell);
    float health = float(strength) / 3.0;
    vec3 col = mix(sprite.color2.rgb, sprite.color.rgb, health * health);
    // Weakened cells flicker in brightness from cell to cell: crumbs.
    float crumb = hash12(uvec2(index, uint(sprite.params.y)) + uvec2(97u, 13u));
    col *= dot_ * (strength == 3u ? 1.0 : 0.7 + 0.5 * crumb);
    outColor = vec4(col, 1.0);
}
