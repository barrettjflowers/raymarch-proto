# raylib-scenes

A 3D scene renderer using [raylib](https://www.raylib.com/) with a full-screen GLSL fragment shader.
<img width="2043" height="1227" alt="image" src="https://github.com/user-attachments/assets/9ddf4cd8-4f85-476a-9f83-2778bd822896" />


## Rendering Technique

This program uses **raymarching of signed distance fields (SDFs)** — specifically **sphere tracing** — with no voxel data or voxel interpolation involved.

Three separate render passes are composited:

1. **Main pass** (`uPassType < 0.5`, 320x200) — SDF raymarching of a ground plane and three boxes. The raymarcher steps along each ray using `t += map(p)` where `map(p)` is the conservative distance to the nearest SDF surface, with 64 max iterations and a hit threshold of 0.001. Normals are computed via finite differences of the SDF.

2. **Sky pass** (`uPassType > 1.5`, 480x300) — Procedural starfield with twinkling and a pulsing moon/sun, all drawn without raymarching.

The passes are rendered to separate `RenderTexture2D` targets at different resolutions and composited onto the screen with `DrawTexturePro`, giving a stylistic multi-resolution look.

## Controls

- **W/S** — move forward/backward
- **A/D** — strafe left/right
