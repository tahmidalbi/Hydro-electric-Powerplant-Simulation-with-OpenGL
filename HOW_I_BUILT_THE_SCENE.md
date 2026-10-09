# How the "Confluence" Scene Was Built — Explained From Zero

This document explains, assuming **no prior knowledge of computer graphics**, how the 3D scene in the `Confluence/` folder is made: what every file does, which file makes which object, and how it all ends up on your screen.

> Scope: this covers the `Confluence/` project (the one tracked in git). `Siyam's Project/` is a different, separate project (a candle scene) and is not covered here.

---

## Part 1 — The Big Picture

### 1.1 What the program is

A window opens and shows a 3D landscape: a **dam** holding back a **lake**, water pouring through **spillway gates** into a **river**, a **water wheel**, **wind turbines**, a field of **solar panels**, **trees**, **rocks**, a **fence**, a **boat**, **clouds**, and a **sky**. You can drag the mouse to orbit around it and scroll to zoom.

**Current state:** the scene is fully built and lit, but it is **static**. The turbine blades, wheel, and gates are drawn at fixed angles/positions (see Part 7). Your last commit message, "upto object creation", reflects that.

### 1.2 The tools used

| Tool | What it is | Plain-English role |
|---|---|---|
| **C++** | Programming language | All the code |
| **OpenGL 3.3** | Graphics API | The way we talk to the graphics card (GPU) |
| **GLFW** | Small library | Opens the window, reads mouse/keyboard |
| **GLAD** | Small library | Loads the OpenGL functions so C++ can call them |
| **GLM** | Math library | Vectors and matrices (`vec3`, `mat4`) |
| **GLSL** | Shader language | Tiny programs that run *on the GPU* to colour things (`shaders/*.vert`, `*.frag`) |
| **CMake** | Build system | Reads `CMakeLists.txt` and compiles everything into `confluence.exe` |

### 1.3 The core idea: everything is triangles

The GPU only knows how to draw **triangles**. Every object you see — a mountain, a turbine blade, a cloud — is a big list of triangles. To describe a triangle you need three **vertices** (corner points). Each vertex in this project stores three things (see `struct Vertex` in [Mesh.h](Confluence/src/core/Mesh.h)):

1. **position** — where the corner is in 3D (x, y, z)
2. **normal** — an arrow pointing directly away from the surface at that corner. Lighting needs this to know which way a surface faces.
3. **color** — the base colour at that corner

Then a list of **indices** says "make triangles by connecting vertices #0, #1, #2, then #0, #2, #3…". A vertex list plus an index list is a **mesh**.

### 1.4 The two-step pattern used for every object

Every object file does the same two things:

1. **Build** (once, at startup): generate the triangles on the CPU (`MeshData`), then upload them to the GPU (`Mesh`).
2. **Draw** (every frame): tell the GPU "draw that mesh, at this position, with this material" via `drawMesh(...)`.

### 1.5 Coordinate system

From [Layout.h](Confluence/src/objects/Layout.h): **x = right, y = up, z = toward the camera**. So the scene stretches away from you along **negative z**. The camera starts at `(0, 23, 0)` (23 units high) looking toward `(0, 9.5, -80)`.

---

## Part 2 — Folder Map

```
Confluence/
├── CMakeLists.txt          How to compile the project
├── include/, src/glad.c    GLAD (OpenGL loader) – third-party, not written by you
├── shaders/                GPU programs (lighting, sky)
└── src/
    ├── main.cpp            The program's entry point + main loop
    ├── core/               Reusable machinery (no specific object)
    └── objects/            One file pair per scene object
```

---

## Part 3 — Which Object Is In Which File

This is the answer to "what is where". Each object has a `.h` (declaration: "this class exists and has these functions") and a `.cpp` (the actual code).

| What you see on screen | File(s) | How it's made |
|---|---|---|
| **Ground / hills / valley / distant mountains** | [Terrain.cpp](Confluence/src/objects/Terrain.cpp) | A big grid of points whose heights come from math + noise |
| **Lake, river, wheel-channel water, foam, spray mist** | [Water.cpp](Confluence/src/objects/Water.cpp) | Displaced water grids + translucent puffs |
| **Falling-water sheets** (the shape used by spillways & wheel outflow) | [Cascade.cpp](Confluence/src/objects/Cascade.cpp) | A curved ribbon following a side-profile curve |
| **Dam wall, spillway piers, gate houses, crest road, railings, lamps, powerhouse, wheel channel, the two steel gates, water pouring from gates** | [Dam.cpp](Confluence/src/objects/Dam.cpp) | Many boxes and wedges glued together |
| **Water wheel** (blue rim, spokes, buckets, axle) | [WaterWheel.cpp](Confluence/src/objects/WaterWheel.cpp) | Many cubes arranged in circles |
| **Solar panel array** (2 rows × 7 panels, cells, frames, rack) | [SolarArray.cpp](Confluence/src/objects/SolarArray.cpp) | Tilted flat quads + small boxes |
| **Two wind turbines** (tower, nacelle, hub, 3 blades) | [Turbine.cpp](Confluence/src/objects/Turbine.cpp) | Stretched cylinder/cube/sphere/cone primitives in a parent→child hierarchy |
| **Forest** (spruces and bushes) | [Trees.cpp](Confluence/src/objects/Trees.cpp) | A few procedural tree "prototypes" copied hundreds of times |
| **Rocks / boulders** | [Rocks.cpp](Confluence/src/objects/Rocks.cpp) | Spheres warped by noise |
| **Wooden fence** | [Fence.cpp](Confluence/src/objects/Fence.cpp) | Cubes as posts + rails |
| **Boat + wake foam** | [Boat.cpp](Confluence/src/objects/Boat.cpp) | Cubes, a 4-sided cone bow, flat spheres for foam |
| **Clouds** (4 of them) | [Cloud.cpp](Confluence/src/objects/Cloud.cpp) | Clusters of overlapping spheres |
| **Sky** (blue gradient background) | [Sky.cpp](Confluence/src/objects/Sky.cpp) + [sky.vert](Confluence/shaders/sky.vert) + [sky.frag](Confluence/shaders/sky.frag) | A full-screen triangle coloured by a GPU program |
| **Where everything is placed + who is drawn in what order** | [Scene.cpp](Confluence/src/objects/Scene.cpp) / [Scene.h](Confluence/src/objects/Scene.h) | Owns all the objects above and calls each one's `draw` |
| **Shared numbers** (positions, sky colours, water levels, dam angle) | [Layout.h](Confluence/src/objects/Layout.h) | Constants only, no drawing |

`Turbine.h` and `Cascade.h` are tiny: `Turbine` is just one function (`drawTurbine`), and `Cascade` is just one function (`buildWaterSheet`) — they aren't classes.

---

## Part 4 — The `core/` Folder (the machinery)

None of these draw a specific object; they are the tools every object uses.

### [Mesh.h](Confluence/src/core/Mesh.h) / [Mesh.cpp](Confluence/src/core/Mesh.cpp) — geometry
- `Vertex` and `MeshData`: the CPU-side list of triangles.
- **`Mesh`** (class): takes a `MeshData` and uploads it to the GPU (VAO/VBO/EBO — GPU memory buffers). `mesh.draw()` renders it.
- **Primitive factories** — the "LEGO bricks" of the whole project. Each makes a simple shape centred at the origin, roughly 1 unit big:
  - `cubeData()`, `cylinderData()`, `sphereData()`, `coneData()`, `quadData()` (flat square), `gridData()` (flat mesh of many squares), `wedgeData()` (a box with a sloped face — used for the dam's gravity profile).
- **`MeshData::append(other, model, color)`** — the most important trick in the project. It copies another mesh *into* this one after moving/rotating/scaling it and tinting its colour. That's how a tree with 500 triangles, or a whole dam from ~200 boxes, becomes **one** mesh (one draw call instead of hundreds).
- `computeSmoothNormals()` — works out the normal arrows by averaging the surrounding triangles, so curved things (rocks, terrain, water) look smooth instead of faceted.

### [MeshLibrary.h](Confluence/src/core/MeshLibrary.h) — shared bricks
Builds one of each primitive (cube, cylinder, tapered "tower" cylinder, sphere, cone, quad) at startup. Objects that are drawn **part-by-part** (turbine, wheel, dam gates) reuse these.

### [Render.h](Confluence/src/core/Render.h) / [Render.cpp](Confluence/src/core/Render.cpp) — `drawMesh`
`drawMesh(shader, mesh, model, material)` sends the position matrix and material numbers to the shader, then draws. **Every object in the scene goes through this one function.**

### [Material.h](Confluence/src/core/Material.h) — how surfaces react to light
A material is a set of numbers: `ka` (ambient), `kd` (diffuse), `ks` (specular), `shininess`, plus `alpha` (transparency). Presets exist: `WHITE_PAINT`, `STEEL`, `LAKE_WATER`, `TERRAIN`, `MATTE`, `FOLIAGE`, `GLOSSY`, `WATER`, `CLOUD`, `WATERFALL`, `MIST`.

### [Light.h](Confluence/src/core/Light.h) — a light source
Type (sun or point), direction/position, colours. `main.cpp` makes two: the **sun** and a dim **sky fill** light.

### [Shader.h](Confluence/src/core/Shader.h) / [Shader.cpp](Confluence/src/core/Shader.cpp)
Loads a `.vert` and `.frag` file, compiles them, and gives easy `setVec3(...)`, `setMat4(...)` helpers to pass numbers to the GPU.

### [Camera.h](Confluence/src/core/Camera.h) / [Camera.cpp](Confluence/src/core/Camera.cpp)
An **orbit camera**: it sits on an imaginary sphere around a target point. Dragging changes yaw/pitch, scrolling changes the radius. It produces the **view matrix** (the "where am I looking from" transform).

### [Noise.h](Confluence/src/core/Noise.h) — controlled randomness
Real terrain isn't smooth or perfectly random. This file provides:
- `Noise::value` / `Noise::fbm` — smooth, natural-looking random variation (fbm = several layers of noise added together at different scales)
- `Noise::Rng` — a seeded random number generator. Because seeds are fixed, **the scene is identical every run.**

### [Screenshot.h](Confluence/src/core/Screenshot.h) / [Screenshot.cpp](Confluence/src/core/Screenshot.cpp)
Run with `--screenshot file.bmp` and it renders one frame offscreen and saves it as an image (used to compare against the proposal's concept picture).

---

## Part 5 — The Shaders (how light and colour are computed)

Shaders are small programs that run on the GPU, once per vertex (`.vert`) and once per pixel (`.frag`).

### [phong.vert](Confluence/shaders/phong.vert) — vertex shader
For each vertex: moves it into world space (`uModel`), rotates its normal, then projects it onto the screen (`uProjection * uView`). If in **Gouraud** mode it also calculates the lighting here, at the vertex.

### [phong.frag](Confluence/shaders/phong.frag) — fragment (pixel) shader
Calculates the final colour of each pixel using the **Phong lighting model**, which adds three terms:

| Term | Meaning | Look |
|---|---|---|
| **Ambient** | Constant background light | Nothing is ever pitch black |
| **Diffuse** | Brightness = how directly the surface faces the light (`dot(N, L)`) | Shading on curved things |
| **Specular** | Shiny highlight when the reflection points at your eye | The glint on water, steel, panels |

Three **shading modes** (keys `1`/`2`/`3`):
- **Flat** — one lighting value per triangle (faceted look)
- **Gouraud** — light computed at the vertices, colours blended across the triangle
- **Phong** — light computed at every pixel using a blended normal (smoothest; default)

Also: press **`B`** for the Blinn-Phong variant, and there's **fog** (distant things fade to haze colour) and **transparency** (`uAlpha`) for water sheets and spray.

### [sky.vert](Confluence/shaders/sky.vert) / [sky.frag](Confluence/shaders/sky.frag)
The sky isn't a model. One triangle covers the whole screen, and for each pixel the shader works out which direction you're looking and blends from pale horizon-blue to deep zenith-blue.

---

## Part 6 — How Each Object Is Actually Made

### 6.1 Terrain — [Terrain.cpp](Confluence/src/objects/Terrain.cpp)
1. `terrainHeight(x, z)` is a **formula** that returns the ground height at any spot. It combines: a riverbed/lake bed, a left hillside, a right cliff, distant hills (bell-curve "bumps" called `gauss`), a far mountain range, and noise for small bumps. It takes the highest of these (`std::max`).
2. `buildTerrain()` lays a grid of points (`axisSamples`) — **dense near the camera, sparse far away** to save triangles — and calls `terrainHeight` at each one.
3. `terrainNormal` finds the slope by sampling neighbouring heights.
4. `terrainColor` paints each vertex: grass (mottled by noise), forest far away, rock on steep slopes and outcrops, wet/rocky band near water.

### 6.2 Water — [Water.cpp](Confluence/src/objects/Water.cpp) + [Cascade.cpp](Confluence/src/objects/Cascade.cpp)
- **Lake**: one big flat grid at `LAKE_LEVEL` height.
- **River**: a grid with wobbly heights (noise "chop"). The `churn()` function says how turbulent each spot is (most where the falls land); `foamAmount()` turns that into white foam patches mixed with blue-green water colour.
- **Wheel channel water**: a smaller frothy grid around the wheel.
- **Waterfalls**: `buildWaterSheet()` in Cascade.cpp takes a **side-view curve** (a list of `(distance, height)` points), smooths it with a Catmull-Rom spline, and extrudes it sideways into a ribbon of water with streaky white/blue colouring. It's used for the wheel outflow and the two spillway chutes.
- **Spray**: ~245 overlapping translucent squashed spheres near the falls, drawn last so they blend softly.

### 6.2b The dam-local coordinate frame
The dam sits diagonally (`DAM_YAW_DEG = -37.9°`). Rather than compute rotated coordinates for every part, the dam and water are built in **dam-local space** (x along the dam, y up, z downstream) and then rotated once using `Layout::damRoot()`. This is a **parent transform**.

### 6.3 Dam — [Dam.cpp](Confluence/src/objects/Dam.cpp)
Built from a `Builder` helper with `box()` and `wedge()`:
- **Main wall** — wedge-shaped blocks (vertical upstream face, sloped downstream face), each 5 m wide with slightly different tone to look like construction joints.
- **Spillway** — a lower middle section with 3 **piers**, each with a gate-house (box + roof + windows) on top and a service bridge across.
- **Crest** — asphalt road, parapet, yellow railings (`railing()` makes posts + two rails) and lamp posts.
- **Powerhouse** — boxes, roof, windows, railings.
- **Wheel channel** — a raised concrete sill, bearing pier, outer wall.
- **Gates** — two steel slabs drawn separately each frame (so they *could* move). Position: `gate = gateSlot * T(0, 2.9 + 3.2*gateOpening, 0)`. That's a **hierarchy**: dam root → gate slot → gate.
- **Spillway water** — two `buildWaterSheet` ribbons, drawn only if the gates are open.

### 6.4 Water wheel — [WaterWheel.cpp](Confluence/src/objects/WaterWheel.cpp)
The wheel is **one mesh** built from cubes placed around a circle using rotation: two rims on each side (40 and 32 segments), 10 spokes per side, 28 buckets between, and a central hub. The axle is a separate cylinder. At draw time the wheel mesh is rotated by `spinAngle` around its axis.

### 6.5 Solar array — [SolarArray.cpp](Confluence/src/objects/SolarArray.cpp)
For each of the 2×7 panels: a light backsheet quad, then a grid of 8×6 dark-blue **cell** quads slightly in front (the gaps show the backsheet, giving grid lines), then four frame edges. The whole array is tilted 30°. A metal rack of rails and legs is added underneath. Cell colours vary slightly with the random generator.

### 6.6 Wind turbine — [Turbine.cpp](Confluence/src/objects/Turbine.cpp)
A **hierarchy** with four levels, each drawn relative to its parent:
1. Tapered **tower** (cylinder with a smaller top)
2. **Nacelle** (box) on the tower top
3. **Hub** (sphere) + pointed **spinner** (cone) at the nacelle front
4. Three **blades** (flattened cones), each rotated 120° apart around the hub by `spinAngle`

If the tower moves, everything above moves with it; if `spinAngle` changes, only the blades turn. That's the whole point of a hierarchy. `drawTurbine` is called **twice** in [Scene.cpp](Confluence/src/objects/Scene.cpp) with different positions/angles to make two turbines.

### 6.7 Trees — [Trees.cpp](Confluence/src/objects/Trees.cpp)
1. Builds a few **prototype** trees once: 6 detailed spruces, 4 simpler far spruces, 5 broadleaf bushes.
   - A **spruce** = trunk + stacked "skirts" of triangles fanning out from the trunk, with a ragged edge that alternates long tips and short notches, tips drooping, and colour going dark-inside to light-tips.
   - A **broadleaf** = several noise-warped spheres for lumpy foliage, darker underneath.
2. Scans four rectangular **regions** of the hillsides on a jittered grid. For each spot, noise decides if a tree grows there; it's rejected by `keepClear()` (dam, foreground, solar, turbines, fence, and the line of sight to the dam), or if it's underwater or on a steep cliff.
3. Each surviving spot copies a random prototype with random rotation, size, and tint via `append`. All trees end up in **one mesh**.

### 6.8 Rocks — [Rocks.cpp](Confluence/src/objects/Rocks.cpp)
A hand-written list of 13 positions. For each, a sphere is squashed flat (`y *= 0.62`) and its points pushed in/out by noise to become lumpy; normals recomputed; placed on the terrain surface.

### 6.9 Fence — [Fence.cpp](Confluence/src/objects/Fence.cpp)
Follows a short 4-point path. Posts are stretched cubes spaced along it, each snapped to the ground height. Two rails per gap are stretched cubes rotated (yaw and pitch) to span between neighbouring posts, even on a slope.

### 6.10 Boat — [Boat.cpp](Confluence/src/objects/Boat.cpp)
Hull (stretched cube), blue stripe, bow (a 4-sided cone laid on its side), cabin, windshield (tilted thin cube), and a V-shaped wake of flattened white spheres behind it.

### 6.11 Clouds — [Cloud.cpp](Confluence/src/objects/Cloud.cpp)
22 big spheres plus 50 small ones piled toward the middle-top, then the bottom is flattened and colour goes from grey-blue underneath to white on top. Four clouds are made with different `seed`s and sizes.

### 6.12 Sky — [Sky.cpp](Confluence/src/objects/Sky.cpp)
Turns depth-testing off, draws the full-screen triangle with the sky shader, and turns it back on. Being drawn first, everything else paints over it.

---

## Part 7 — Putting It Together: [Scene.cpp](Confluence/src/objects/Scene.cpp)

**Constructor** (runs once): calculates each object's placement matrix ("root"):
- Turbines: stand on the ground at their `Layout` spot and turn to face the camera (`atan2`).
- Wheel: attached to the dam frame at `Dam::wheelMount()`.
- Solar array: on the terrain at its spot, yawed 53°.
- Boat: on the lake surface, rotated 90°.
- Clouds: four seeds/sizes/positions in the sky.

**`draw()`** (each frame) draws in this order — order matters:
1. Terrain → Water → Dam → Wheel → Trees → Rocks → Fence → Solar → Turbines → Boat (all **opaque**)
2. Clouds (fog turned off for them)
3. **Transparent pass** (blending on): spillway water → wheel waterfall → spray (spray doesn't write depth so it blends softly).

Transparent things must go last so they can blend with what's already behind them.

### Where "static" comes from
In `Scene::draw` the animated values are **constants**:
- `wheel.draw(..., 0.3f)` — wheel angle fixed
- `drawTurbine(..., 0.35f)` and `1.35f` — blade angles fixed
- `Layout::GATE_OPENING = 0.45f` — gates half-open, fixed
- Boat root and cloud roots are never changed

The functions already accept these values as parameters, so making things move only needs a time variable in `main.cpp` feeding them.

---

## Part 8 — The Program's Life: [main.cpp](Confluence/src/main.cpp)

1. Open a window (OpenGL 3.3 core, 4× anti-aliasing, size matching the concept image's aspect ratio).
2. Load OpenGL via GLAD; enable depth testing (nearer things hide farther things).
3. Build everything: `Shader`, `MeshLibrary`, `Scene`, `Sky`. Set the camera. Create the sun and fill lights.
4. **Loop until you close the window**, every frame:
   - Read input → clear screen → compute projection (perspective) and view (camera) matrices
   - Draw the sky, then set all the shader uniforms (matrices, lights, fog, mode), then `scene.draw(...)`
   - Swap buffers (show the finished picture)

**Controls**

| Input | Action |
|---|---|
| Left-drag | Orbit camera |
| Scroll | Zoom |
| `1` / `2` / `3` | Flat / Gouraud / Phong shading |
| `B` | Toggle Blinn-Phong |
| `R` | Reset camera |
| `Esc` | Quit |

`g_termMask` (ambient|diffuse|specular) exists in the code and the shader supports it, but **no key changes it yet**.

---

## Part 9 — Key Concepts Glossary

| Term | Meaning |
|---|---|
| **Vertex** | A corner point with position, normal, colour |
| **Mesh** | Vertices + indices = a 3D shape |
| **Normal** | Arrow pointing away from a surface; drives lighting |
| **Model matrix** | Moves/rotates/scales an object into the world (`T * R * S`) |
| **View matrix** | Moves the world so the camera is at the origin |
| **Projection matrix** | Makes far things smaller (perspective) |
| **Hierarchy** | Children drawn relative to a parent's transform |
| **Baking** | Combining many small parts into one mesh ahead of time (`append`) |
| **Procedural** | Generated by code/math rather than modelled by hand |
| **Noise / fbm** | Natural-looking pseudo-randomness |
| **Uniform** | A value sent from C++ to a shader |
| **Draw call** | One "draw this mesh" order to the GPU (fewer = faster) |
| **Alpha / blending** | Transparency |

---

## Part 10 — Where to Look When You Want to Change Something

| I want to… | Edit |
|---|---|
| Move the turbines / solar / boat | [Layout.h](Confluence/src/objects/Layout.h) |
| Change sky or fog colour | [Layout.h](Confluence/src/objects/Layout.h) |
| Change sun direction or colour | `main.cpp` (the `sun` block) |
| Open/close the gates | `Layout::GATE_OPENING` in Layout.h |
| Make hills higher | `hillsHeight` in Terrain.cpp |
| Add or remove trees | `regions[]` in Trees.cpp |
| Add a rock | `ROCKS[]` in Rocks.cpp |
| Change how shiny something is | Material presets in Material.h |
| Add a brand-new object | New `.h/.cpp` in `objects/`, add it to `Scene.h`, place it in `Scene.cpp` (CMake picks up new `.cpp` files automatically, after re-running CMake) |
