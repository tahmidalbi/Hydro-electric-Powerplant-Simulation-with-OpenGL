# Confluence — Steps 1 to 4 explained

This file explains everything that was added for **Steps 1–4** of the roadmap
([Confluence_Graphics_Project_Roadmap.pdf](../../docs/Confluence_Graphics_Project_Roadmap.pdf),
in the main project's `docs` folder), in simple words.
For every step you get: **what it does, why it exists, the important formula, the files, the
keys, and the proof that it works.**

Result: **Milestone A is reached.** On the same coarse shiny object, Gouraud and Phong
clearly give different highlights, and you can point to the vertex and fragment
calculations in the code.

---

## 1. Build and run

The project uses the same tools as before: **C++17, OpenGL 3.3, GLFW (window),
GLAD (OpenGL loader), GLM (vector/matrix types)**, built with **MSYS2 MinGW g++** and CMake.
No new library was added. There is no Python and no UI/text library.

Open **PowerShell** in the `Confluence` folder:

```powershell
$env:PATH = "C:\msys64\mingw64\bin;$env:PATH"
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j 1
cd build
.\confluence.exe
```

- Run it **from the `build` folder**, because it loads `shaders/` from there. CMake copies the shaders
  into `build/shaders` after every build.
- In VS Code you can still use the CMake Tools buttons, as before.
- `-j 1` builds one file at a time. With `-j 8` the build sometimes failed on this OneDrive folder
  (files being locked). The single-file build always worked.

### Extra ways to run it

| Command | What it does |
|---|---|
| `.\confluence.exe --selftest` | Runs 17 maths checks for Steps 1–4. No window opens. Prints PASS/FAIL. |
| `.\confluence.exe --screenshot out.bmp` | Draws one frame into a picture and exits. |
| `.\confluence.exe --screenshot g.bmp --gallery --shading gouraud --detail low` | Any view can be reproduced exactly; good for report pictures. |

All options: `--gallery`, `--shading flat|gouraud|phong`, `--terms all|ambient|diffuse|specular`,
`--lights all|sun|fill|lamp`, `--detail low|medium|high`, `--blinn`, `--wireframe`, `--normals`,
`--wrong-order`, `--wrong-normals`, `--advance SECONDS`.

---

## 2. Controls

Click the window first so it receives the keys. The **window title shows the live status**,
for example:
`Confluence | GALLERY | Gouraud | terms: all | lights: sun+fill+lamp | t=12.4s x1 PAUSED | 98 FPS | LOW detail: sphere 96 tris, cylinder 24, water patch 2`.
The **console** prints the full key list at start and one line for every key you press.

| Key | Action | Step |
|---|---|---|
| Left-drag / mouse wheel | Orbit / zoom the camera | – |
| `R` | Reset the camera | – |
| `Space` | Pause / resume the simulation (camera and lamp still move) | 1 |
| `[` / `]` | Time scale ÷2 / ×2 (0.125× … 8×) | 1 |
| `Enter` | Reset scene: time 0, rotors at start, lamp at start, time ×1 | 1 |
| `F6` | Open / close the graphics comparison **gallery** | 2, 4 |
| `X` | Wireframe (see the triangles) | 2 |
| `N` | Normals as colours | 2 |
| `+` / `-` | Gallery mesh detail LOW / MEDIUM / HIGH | 2, 4 |
| `O` | Gallery: **wrong** matrix order `S*R*T` | 2 |
| `M` | Gallery: **wrong** normal matrix on the stretched ellipsoid | 2 |
| `K` | Lighting terms: all → ambient only → diffuse only → specular only | 3 |
| `L` | Lights: sun+fill+lamp → sun only → fill only → lamp only | 3 |
| `B` | Specular formula: Phong ↔ Blinn-Phong | 3 |
| Arrow keys, `PageUp`/`PageDown` | Move the inspection lamp (point light) | 3 |
| `1` / `2` / `3` | **Flat / Gouraud / Phong** shading | 4 |
| `F11` | Print the help again | – |
| `Esc` | Quit | – |

These keys follow the roadmap's "Suggested controls" page, so later steps can add theirs
without clashes. `O` and `M` are extra and only matter in the gallery.

---

## 3. Where the code is (folder map)

```
Confluence/
  shaders/
    lighting_common.glsl   NEW  the ONE lighting formula (+ fog / debug colour helpers)
    phong.vert/.frag       CHANGED  Gouraud + Phong program (uses lighting_common)
    flat.vert/.geom/.frag  NEW  strict Flat program (geometry shader = 1 colour per triangle)
    sky.vert/.frag         unchanged
  src/
    main.cpp               REWRITTEN  setup + the loop: input -> update -> render
    App.h                  NEW  everything the program remembers (clock, state, cameras)
    core/
      Clock.h/.cpp         NEW  Step 1: real time vs simulation time, pause, time scale
      SceneState.h         NEW  Step 1: all changing numbers (rotor angles, modes, lamp)
      Transform.h/.cpp     NEW  Step 2: hand-written translate/rotate/scale/pivot/shear/mirror
      Shader.h/.cpp        CHANGED  optional geometry shader + pastes lighting_common.glsl
      Light.h              CHANGED  3 lights (sun, fill, lamp)
      Material.h           CHANGED  concrete, panel glass, lamp glow, gallery materials
      Render.h/.cpp        CHANGED  drawMesh uses Transform::normalMatrix
      Mesh.h/.cpp          CHANGED  triangleCount(); start-up speed fix (see section 9)
    systems/
      Animation.h/.cpp     NEW  Step 1: angle += speed * dt
      Environment.h/.cpp   NEW  Step 3: builds sun / fill / lamp lights
    input/
      Input.h/.cpp         NEW  keys and mouse (moved out of main.cpp)
      CommandLine.h/.cpp   NEW  --selftest, --screenshot and view options
    rendering/
      FrameRenderer.h/.cpp NEW  draws one frame (picks the Flat or the Phong program)
    ui/
      Hud.h/.cpp           NEW  window-title status + console help
      DemoGallery.h/.cpp   NEW  F6 gallery: Step 2 and Step 4 exhibits
    tests/
      SelfTest.h/.cpp      NEW  --selftest: 17 numeric checks
    objects/
      Scene.h/.cpp         CHANGED  draw() reads PlantState (rotor angles, gate, wheel)
      Turbine.cpp          CHANGED  uses Transform, hierarchy explained in comments
      Dam.cpp, SolarArray.cpp   CHANGED  only their material
  captures/                NEW  screenshots used as evidence (section 8)
  docs/STEPS_1_TO_4.md     this file
```

**Rule used everywhere:** each folder has one kind of job. `core` = basic building blocks,
`objects` = things in the plant, `systems` = things that change over time / lighting,
`input` = keyboard and mouse, `rendering` = drawing a frame, `ui` = what the user sees about
the program, `tests` = checks.

---

## 4. How one frame works

```
main.cpp loop, every frame:

  1. INPUT    glfwPollEvents()                -> key callbacks change App state (Input.cpp)
              clock.tick(glfwGetTime())       -> realDelta, simulationDelta     (Clock.cpp)
              Input::handleHeldKeys()         -> arrow keys move the lamp
  2. UPDATE   Animation::update(plant, simulationDelta)   -> rotor angles advance
  3. RENDER   renderer->render(...)           -> reads state, draws, never changes it
              glfwSwapBuffers()
  4. STATUS   window title updated ~4 times per second
```

The most important idea: **drawing only reads the state, it never changes it.** Only the
update step and the input step write to `SceneState`.

---

## 5. Step 1 — Time and state

**What it does.** The plant now has a clock. The two turbine rotors turn continuously. You
can pause, speed up, slow down and reset time, and the camera still works while paused.

**Why it exists.** Before, every pose was a fixed number (`0.35`, `1.35`, …), so nothing
could move. Every later step (wind, gate, boat, ray snapshot) needs a clean "time + state" layer.

**Two kinds of time** ([Clock.h](../src/core/Clock.h)):
- *real time* is what the wall clock says. The camera and the lamp use it.
- *simulation time* is what the plant feels. It is 0 while paused and ×2 at double speed.

**The formula** ([Clock.cpp](../src/core/Clock.cpp), [Animation.cpp](../src/systems/Animation.cpp)):

```
realDelta       = currentRealTime - previousRealTime       (clamped to 0 ... 0.1 s)
simulationDelta = paused ? 0 : realDelta * timeScale
simulationTime += simulationDelta
rotorAngle     += rotorSpeed * simulationDelta              (then wrapped into 0 ... 2π)
```

- **Why `angle += speed * dt` and not `angle = speed * totalTime`?** If the speed changes later
  (Step 5: wind), the second form would make the rotor jump. Adding small steps never jumps.
- **Why clamp to 0.1 s?** If the window is dragged or the PC stalls for 2 seconds, the plant
  moves 0.1 s instead of jumping 2 s.
- **Why wrap the angle?** After hours, a huge float angle loses precision and the rotor would
  stutter. 2π more looks exactly the same.

**State** ([SceneState.h](../src/core/SceneState.h)) holds:
- `PlantState`: simulation time, both rotor angles and speeds, wheel angle and gate opening.
  The wheel and gate are still fixed; Step 6 will drive them, but they already live here.
- `ViewSettings`: shading mode, lighting terms, lights, wireframe, normals, gallery options.
- `LampState`: lamp position in the scene and in the gallery.

**Files:** `core/Clock.*`, `core/SceneState.h`, `systems/Animation.*`, `main.cpp`, `App.h`,
`objects/Scene.*`.

**Proof** (`--selftest`, section 7):
- the rotor angle after 10 s is the same at 30, 60, 144 FPS and with uneven frames (difference 0.00009 rad);
- pause freezes it;
- resume continues with exactly one normal frame step (no jump);
- a 2 s stall counts as 0.1 s;
- ×2 speed gives 10 s simulated in 5 s;
- reset restores 0.35 / 1.35 rad.

Pictures: `captures/01_scene_phong_t0.png` and `02_scene_phong_t2s.png` show the blades in a
different position after 2 seconds.

---

## 6. Step 2 — Transformations

**What it does.** Every transform is written out by hand
([Transform.cpp](../src/core/Transform.cpp)), so you can explain each matrix entry. The
gallery (F6) shows each idea on its own object.

**Why it exists.** "Translation, rotation, scaling, composition and hierarchy" must be shown
and explained, not just used.

**The formulas:**

```
worldPoint = Model * localPoint
Model      = Translate * Rotate * Scale        (the RIGHTMOST one acts first)
clipPoint  = Projection * View * Model * localPoint
pivotTurn  = Translate(pivot) * Rotate * Translate(-pivot)
normals    = (M^-1)^T  (inverse-transpose of the model matrix)
```

| Function | Matrix idea |
|---|---|
| `translate(offset)` | offset in the 4th column; points (w=1) move, directions (w=0) do not |
| `scale(factors)` | factors on the diagonal; never 0 (no inverse → broken normals) |
| `rotateX/Y/Z(angle)` | cos/sin in two rows; angle in **radians** (`degreesToRadians()` for degrees) |
| `rotateAboutPivot(p, R)` | move pivot to origin, turn, move back — the pivot does not move |
| `shearXByY(k)` | `x' = x + k*y`: a box leans, its base stays |
| `reflectAcrossPlaneX(x0)` | mirror; flips triangle winding (determinant −1) |
| `normalMatrix(M)` | `(M^-1)^T`, keeps normals perpendicular under non-uniform scale |

**Hierarchy in the turbine** ([Turbine.cpp](../src/objects/Turbine.cpp)):
`root → tower`, `root → nacelle → hub → 3 blades`. Each child is "parent × local offset", so
moving the root moves everything. The hub frame's origin **is** the rotor pivot, so the blades
spin in place. Size (`scale`) is applied only on the part's own draw call. A tall tower
therefore does not stretch the nacelle sitting on it.

**The gallery, back row** ([DemoGallery.cpp](../src/ui/DemoGallery.cpp)), left to right:

| Exhibit | What to say |
|---|---|
| Turbine whose root slides and turns | Only the root is animated; tower, nacelle, hub and blades follow → hierarchy. |
| Gate in a turning frame | The gate moves only along the frame's **local** y, so it stays in its slot. |
| Green bar on a gold post | `T*R*S`: spins in place. Press **O** → `S*R*T`: it wanders off the post and gets squashed. |
| Blue panel on a hinge | Rotation about a **pivot** (the hinge line), not about its centre. |
| Yellow leaning box | **Shear**. |
| Green L, mirror, mirrored L | **Reflection**: the copy is a mirror image, not a rotation. |
| Red stretched ellipsoid (front right) | Non-uniform scale. Press **M** → plain model matrix for normals → the highlight shrinks and moves to the wrong place. |

`X` (wireframe), `N` (normals as colours) and `+`/`-` (detail) work everywhere.
*Detail changes the number of triangles; scale changes the size.*

**Proof:**
- self-test: hand-written matrices = glm (difference 0);
- `T*R*S` keeps the bar centre on its post, `S*R*T` moves it 7 units;
- pivot stays fixed;
- moving the root moves the hub by exactly the same offset;
- `(M^-1)^T` normal · tangent = 0, while the plain matrix gives 2.53;
- shear and mirror numbers are correct.

Pictures: `20_gallery_correct_order_t2.png`, `21_gallery_wrong_order_t2.png`,
`22_gallery_wrong_normals.png`, `14_gallery_wireframe_medium.png`, `15_gallery_normals.png`.

---

## 7. Step 3 — Lighting

**What it does.** Three lights, distinct materials, and keys to take the lighting apart.

| Light | Type | Job |
|---|---|---|
| `[0]` sun | directional | One direction for the whole plant; decides which sides are lit. |
| `[1]` sky fill | directional, weak, bluish | Keeps shaded faces from going black. No highlight. |
| `[2]` inspection lamp | **point** | Movable with the arrow keys and PageUp/PageDown. Gets weaker with distance. A small glowing bulb shows where it is. |

Plus a small **global ambient** term.

**The formula** (one copy only, in [lighting_common.glsl](../shaders/lighting_common.glsl)):

```
colour = ka·albedo·ambient                              (K: ambient)
       + emission                                       (always on: lamp bulb, clouds)
       + Σ lights  attenuation · ( kd·albedo·lightColour·max(N·L, 0)      (K: diffuse)
                                 + ks·lightSpecular·spec )                (K: specular)

spec (Phong)       = max(R·V, 0)^shininess,   R = reflect(-L, N)
spec (Blinn, B key)= max(N·H, 0)^shininess,   H = normalize(L + V)
attenuation (lamp) = 1 / (1 + 0.045·d + 0.0075·d²)    → 1.0 at the bulb, 0.5 at 9 m, 0.25 at 17 m
```

**Emission vs. a light.** The bulb only *looks* bright (emission). It does not light anything
up. The separate point light `[2]` lights the surfaces around it.

**Materials** ([Material.h](../src/core/Material.h)). Read `ks` and `shininess` together:

| Material | ks | shininess | Look |
|---|---|---|---|
| CONCRETE (dam) | 0.08 | 8 | weak, broad highlight |
| WHITE_PAINT (turbines) | 0.35 | 32 | moderate |
| STEEL (gates) | 0.75 | 70 | strong, tight |
| PANEL_GLASS (solar) | 0.85 | 120 | strong, very tight |
| LAKE_WATER / WATER | ~0.6 | 70–90 | glint |

**One formula, three stages.** GLSL 3.30 has no `#include`. [Shader.cpp](../src/core/Shader.cpp)
therefore reads `lighting_common.glsl` and pastes it right after the `#version` line of every stage.
The old project had the lighting function copied twice; now there is only one.

**Files:** `shaders/lighting_common.glsl`, `core/Light.h`, `core/Material.h`, `core/Shader.*`,
`systems/Environment.*`.

**Proof:** `K` isolates each term (`35_scene_ambient_only.png`, `36_scene_diffuse_only.png`,
`30_scene_specular_only.png`). `L` → lamp only (`31_scene_lamp_only.png`) shows a warm pool
of light around the powerhouse that fades with distance. Moving the lamp moves the pool.
Orbiting the camera moves highlights; diffuse does not move. The self-test checks the attenuation
numbers.

---

## 8. Step 4 — Flat, Gouraud and Phong

**What it does.** `1`, `2` and `3` switch **only the shading method**. Same mesh, same material,
same lights, same camera.

| Mode | Where the lighting formula is evaluated | File |
|---|---|---|
| **Flat** (`1`) | **once per triangle**: at the triangle's centre, with its face normal; all 3 corners get that one colour | [flat.geom](../shaders/flat.geom) |
| **Gouraud** (`2`) | **once per vertex**; the 3 corner colours are blended across the triangle | [phong.vert](../shaders/phong.vert) |
| **Phong** (`3`) | **once per pixel**; the normals are blended, re-normalised and lit | [phong.frag](../shaders/phong.frag) |

> The **Phong lighting model** (the formula in section 7) is not the same as **Phong shading**
> (evaluating it per pixel). Gouraud shading uses the same Phong lighting model, only at the vertices.

**Strict Flat, explained.** A *geometry shader* sees the whole triangle at once. It computes:
- the face normal `normalize(cross(p1−p0, p2−p0))`, flipped if it points opposite to the
  vertex normals (winding check);
- the centre `(p0+p1+p2)/3`;
- the lighting **once**.

Even a nearby point light cannot vary across one triangle. The old version only *reconstructed*
a face normal per pixel, which still let a point light vary inside a triangle. That is fixed now.

**The gallery, front row** (F6):
- red **coarse sphere**, blue **coarse cylinder**, and a flat **water patch** (`+`/`-` change their detail);
- LOW = sphere 96 triangles, cylinder 24, water patch **2 triangles**.

**Why the water patch proves it.** The lamp starts at a position calculated with the "mirror
trick" (`DemoGallery::lampStartPosition()`). Its reflection then lands **exactly in the middle of
the patch**, far from all 4 corners:
- **Phong** computes the light at that pixel → a bright highlight (`10_gallery_phong_low.png`).
- **Gouraud** only computes it at the 4 corners, where it is ≈ 0, and blends zeros → **no
  highlight at all** (`11_gallery_gouraud_low.png`).
- **Flat**: each of the 2 triangles has one single colour (`12_gallery_flat_low.png`).
- At HIGH detail (32×32 cells), Gouraud finds the highlight again (`13_gallery_gouraud_high.png`).
  More triangles help Gouraud by **sampling more often**; they do not turn it into Phong.

The self-test computes the same thing on the CPU. Patch centre: **Phong 1.000, Gouraud 0.000**.

**Files:** `shaders/phong.vert`, `phong.frag`, `flat.vert`, `flat.geom`, `flat.frag`,
`lighting_common.glsl`, `rendering/FrameRenderer.cpp` (picks the program), `ui/DemoGallery.*`.

**Milestone A — done:** the same coarse shiny object shows a clear Gouraud/Phong difference,
and the vertex (`phong.vert`) and fragment (`phong.frag`) calculations are easy to point at.

---

## 9. Checks that were run

1. **Build:** no errors. All new files also compile cleanly with `-Wall -Wextra -Wshadow`.
2. **`--selftest`: 17 of 17 checks pass** (frame-rate independence, pause/resume, stall clamp,
   time scale, reset, matrices vs glm, matrix order, pivot, hierarchy, normal matrix, shear,
   reflection, Gouraud vs Phong, attenuation).
3. **Screenshots** of every mode were rendered and looked at (`captures/`).
   `00_before_steps.png` is the scene **before** these steps, for comparison.
4. **The real window** was started and keys were sent to it. Every key printed its message and
   the title followed (shading, terms, lights, pause, time scale, gallery, wireframe, normals,
   O, M, Enter, F6). It runs at about 95–100 FPS on this laptop (AMD Radeon, OpenGL 3.3).

### Two problems found in the old code and fixed

- **Start-up took about 11 seconds.** `MeshData::append` reserved *exactly* "size + new" memory on
  every append. That made the vector copy itself completely each time, which is quadratic.
  The forest is built from thousands of appends: 9.5 s. Now capacity at least doubles when it
  grows, and start-up takes **about 1.4 s**. The picture is byte-for-byte the same as before the fix.
- **Rare `std::bad_alloc` crash at start-up.** Seen twice in about 70 runs, including with the
  original code. It never happened under the debugger. It was most likely caused by those huge
  repeated copies on a laptop with little free memory. After the fix: 0 failures in 30 runs.
  If it ever appears again, the program now says "out of memory" instead of a confusing message.

---

## 10. Honest notes and limits

- **No new library.** GLFW, GLAD and GLM were already part of the project and are still used:
  GLFW for the window, GLAD to load OpenGL, GLM for vector/matrix *types*. The transforms
  themselves are written by hand in `Transform.cpp`. Only `glm::inverse` (inside
  `normalMatrix`) and `glm::perspective`/`lookAt` (camera, unchanged) are glm functions.
- **The "HUD" is the window title + console**, because a text library would be a new library.
  On-screen text and sliders can come in Step 12.
- **The solar array is one baked mesh**, so its rack shares the panel-glass finish. Step 7
  splits it (needed for the tilt hinge anyway).
- **Emission is always on**, so clouds and spray stay visible even in "specular only".
- In Flat mode the fog still varies per pixel (fog is not lighting). The gallery has no fog.
- `Enter` resets time, poses and the lamp, but keeps pause and the view modes.
- The **water wheel and gates still stand still**. Step 5 adds wind for the rotors, and Step 6
  connects the gate and the wheel. Their numbers are already in `PlantState`.
- The gallery's back-row exhibits are small in the default view. Drag and scroll to look closer,
  and `R` returns to the preset.

---

## 11. Short answers for the viva

- **Why two times?** So pausing stops the plant but not the camera, and the speed of the plant
  never depends on the FPS.
- **Why `T*R*S`?** The rightmost matrix acts first: size it, turn it about its own centre,
  then put it in place.
- **Why `(M^-1)^T` for normals?** Non-uniform scaling would otherwise tilt the normals, and
  the lighting would stop matching the shape (M key).
- **Gouraud vs Phong?** Same formula; Gouraud evaluates it at the vertices and blends colours,
  Phong blends normals and evaluates it at every pixel. A highlight between vertices is lost
  by Gouraud (the water patch).
- **Strict Flat?** The geometry shader lights each triangle once, at its centre, with its face normal.
- **Phong vs Blinn?** Phong uses the reflection vector R·V; Blinn uses the half vector N·H
  (cheaper, broader highlight).
- **Point vs directional light?** A directional light has only a direction and the same strength
  everywhere; a point light has a position and fades as `1/(a0 + a1·d + a2·d²)`.

## 12. Next: Step 5

Add `windStrength` to `PlantState`, set `rotorSpeedRadPerSecond` from it in
`Animation::update`, and use Q/E in `Input.cpp`. The clock, state and update layers from Step 1
are already in place for this.
