# How it works (simple English)

This file explains each required graphics topic, where it is in the code, and
how to show it while the program is running.

| Requirement | Where to look | How to show it live |
|---|---|---|
| Lighting | `shaders/common.glsl` → `shade()`, every scene's `lighting()` | each scene has its own sun/sky/fog; **L** = shadows on/off |
| Transformation | `src/core/MathLib.h`, gates, rotor, debris, cameras | gates rise, rotor spins, debris tumbles, mouse orbit |
| Gouraud & Phong | `shaders/gouraud.vert/.frag` vs `shaders/lit.vert/.frag` | press **G** (MIXED → PHONG → GOURAUD) |
| Motion / animation | every `update(dt)`, `ParticleSystem`, `SpillwayWater`, `Debris` | everything moves; the water fronts follow physics |
| Interaction | `App.cpp` (`onKey`, `onMouseButton`, …) + each scene's `buttons()` | on-screen buttons, keys, mouse orbit / zoom |
| Ray tracing | `shaders/raytrace.frag`, `src/scenes/Scene7_RayTracedFlow.cpp` | scene **7** |

---

## 1. The frame, step by step (`App::render`)

1. The scene sets up the **camera** (`camera()`) and the **lights** (`lighting()`).
2. **Shadow pass**: the scene is drawn from the sun's point of view into a
   2048×2048 depth texture (`Renderer::beginShadowPass`). Only depth is stored.
3. **Main pass**: the sky, then solid objects, then transparent things (water,
   glass, particles, light shafts) with blending and no depth writes.
4. **HUD**: rectangles, lines and text in pixel coordinates (`ui/Ui.cpp`).
5. **Fade**: a black rectangle whose transparency changes between scenes.

Scene 7 is different: one full-screen triangle is drawn and the ray tracer
fragment shader computes every pixel.

## 2. Lighting

`shade()` in `shaders/common.glsl` is the **Blinn-Phong** lighting equation:

```
colour = albedo * ambient                      (light from sky + ground)
       + albedo * sunColour * max(N·L, 0) * shadow       (diffuse)
       + sunColour * (N·H)^shininess * specular * shadow (specular highlight)
       + point lights with distance fall-off            (lamps in the powerhouse)
       + sky reflection * Fresnel                        (metal shine)
       + caustics                                        (under water)
```

* **Directional light (sun)**: `Lighting::sunDir`, `sunColor`. Each scene has
  its own: an evening light for scene 1, a sunny day for scene 2, blue light
  from above for scene 3, studio light for the cut-aways.
* **Hemisphere ambient**: surfaces facing up get sky colour, surfaces facing
  down get ground colour.
* **Point lights**: up to 4 (`uPointPos`, `uPointColor`). Examples are the
  powerhouse ceiling lamps and the tunnel light under water.
* **Shadows**: shadow mapping with 3×3 PCF soft edges (`shadowFactor()`).
* **Fog**: distant objects fade into the fog colour. Under water, red light is
  absorbed first, so far things turn blue-green (`applyFog()`).
* **Tone mapping**: the ACES film curve plus gamma correction (`finalColor()`),
  so bright highlights roll off softly like in a film.

## 3. Transformations

All matrices are hand-written in `src/core/MathLib.h`: `translate`, `scale`,
`rotateX/Y/Z`, `rotateAxis` (Rodrigues), `lookAt`, `perspective`, `ortho`, and
`normalMatrix` (inverse-transpose, so normals stay correct when scaled).

Examples you can see:

* **Spillway gates** (`Dam::draw`): one gate mesh is drawn three times with
  `translate(bay position + lift)`.
* **Generator rotor and turbine** (`Powerhouse::draw`):
  `translate(unit) * rotateY(angle)`, which turns around the shaft every frame.
* **Intake gate** (`IntakeGate::draw`): a *hierarchical* transformation. The
  piston rods use the same `up` matrix as the gate leaf, so they move with it.
* **Debris** (`Debris::draw`): `translate(pos) * rotateY * rotateZ * rotateX * scale`.
  The rotation is animated, and when the branch hits the rack it is blended
  into its "leaning" pose.
* **MeshBuilder** (`src/core/Mesh.cpp`) builds shapes with a transform too:
  `alignY()` turns a cylinder so it points from A to B (lattice towers, bolts,
  tree limbs).
* **Camera**: `lookAt` plus the user's orbit (yaw/pitch/zoom/pan in `Camera.h`).

## 4. Gouraud vs Phong shading

* **Phong** (`lit.vert` + `lit.frag`): the normal is passed to the fragment
  shader and the full lighting is computed **for every pixel**. Highlights are
  sharp and round.
* **Gouraud** (`gouraud.vert` + `gouraud.frag`): the lighting is computed
  **only at the vertices** in the vertex shader, and the GPU blends those
  colours across the triangle. It is cheaper, but highlights look smeared or
  disappear between vertices.

In the default **MIXED** mode, the big smooth **terrain** and the **sea bed**
use Gouraud (where it looks fine and saves work) and everything else uses
Phong. Press **G** to switch everything to Phong, then everything to Gouraud.
Watch the steel pipe highlights and the generator to see the difference. The
current mode is shown on the bottom-right button.

## 5. Motion and animation

* **Spillway water** (`SpillwayWater.cpp`): the front of the water moves with
  `v = sqrt(v0² + 2·g·h)`, where *h* is how far it has dropped. It starts slow
  at the crest and is fast at the bottom. When the gates close, the "tail"
  runs down the same way, so the chute drains.
* **Penstock water** (`Scene6_Penstock.cpp`): the same physics. The rounded
  nose is made in `water.vert`, where vertices past the front are pulled
  towards the pipe axis.
* **Gate-burst water** (`Scene5_IntakeGate.cpp`): a mesh whose heights are
  rebuilt every frame (bulge, advancing front, rising level). The jet speed
  comes from Torricelli's law.
* **Waves and ripples**: sine waves move the surface vertices (`water.vert`),
  and animated noise bends the normals (`water.frag`).
* **Particles** (`render/ParticleSystem.cpp`): spray, mist, splashes, bubbles
  and dust. Each one has velocity, gravity, drag and a lifetime, and is drawn
  as a soft camera-facing square.
* **Debris physics** (`Debris.cpp`): current, buoyancy, bobbing, tumbling, and
  stopping at the rack.
* **Cameras**: smooth Catmull-Rom spline paths (`evalCamPath`), and a chase
  camera in scene 6.
* **Scene changes**: a fade to black and back.

## 6. Interaction

* **On-screen buttons** for every scene, which you can click or trigger with
  the key shown under them: STOP/START WATER, OPEN/CLOSE GATE, THROW TRASH,
  RAKE CLEAN, X-RAY, REPLAY, RAISE/LOWER HEAD, NEXT and so on.
* **Mouse**: left drag orbits, right drag pans, the wheel zooms. **C** goes back
  to the director's camera.
* **Keys**: G shading, L shadows, H HUD, P auto-play, 1–7 scenes, N/B next and
  back, R restart, F12 screenshot.
* **Output back to the user**: the TELEMETRY panel shows live values computed
  from the simulation (gate %, flow, speed, pressure, trapped pieces, power).

## 7. Ray tracing (scene 7)

`shaders/raytrace.frag` is a complete **Whitted-style ray tracer** that runs on
the GPU, one ray per pixel:

1. **Primary ray**: from the camera through the pixel (built from the camera's
   right/up/forward vectors).
2. **Intersections are solved exactly with maths**, with no triangles:
   * the pipe is an infinite cylinder (`cylinder()`, a quadratic equation),
     split into glass sections and steel sleeves by the distance along the
     axis;
   * flanges and bands are capped cylinders (`cappedCyl()`);
   * the supports are boxes (`box()`, the slab method);
   * the floor and wall are planes.
3. **Shadow ray** (`shadowRay()`): from the hit point towards the light. If
   something blocks it, the point is in shadow. Glass lets blue light through.
4. **Reflection ray**: the floor is glossy, so a second ray is traced from it
   using Fresnel strength. You can see the pipe mirrored in the floor.
5. **Refraction**: at the glass the ray bends into the water (`refract()`,
   index 1.33, Snell's law), crosses the water, and bends out again on the
   other side. Total internal reflection is handled too.
6. **Volume ray-marching** (`marchWater()`): inside the water the ray moves in
   36 small steps and adds up the light of the glowing water streams.
   `waterGlow()` builds those streams from 3D noise stretched along the flow
   and moving at the flow speed.

The C++ side (`Scene7_RayTracedFlow.cpp`) sends the camera, the pipe axis and
the flow speed (from v = √(2gh)) to the shader. UP/DOWN change the head, so
the streams speed up or slow down.

## 8. Other techniques used

* **Procedural textures** (`shaders/materials.glsl`): concrete (formwork
  panels, joints, tie holes, rain streaks), brushed steel, paint wear, bark,
  rock/grass by slope, soil with pebbles, floor tiles, wire mesh (cut out with
  `discard`), copper coils, hologram grid, leaves.
* **Water shader** (`water.frag`): Fresnel mix of the reflected sky and the
  water colour, sun glitter, foam streaks and foam patches, a cut-face mode
  for the cut-away diagrams, Snell's window when seen from below.
* **Caustics**: an animated pattern added below the water line.
* **Procedural geometry** (`core/Mesh.cpp`): boxes, cylinders, cones, spheres,
  tori, annuli, grids, tubes swept along rounded paths, extruded dam profiles,
  and noisy rocks (`objects/Rocks.cpp`).
* **Hand-made font** (`ui/Font.cpp`): 5×7 pixel letters written as text
  pictures and turned into a texture at start-up.
* **Screenshots** (`core/Screenshot.cpp`): a BMP file writer, no image library.
