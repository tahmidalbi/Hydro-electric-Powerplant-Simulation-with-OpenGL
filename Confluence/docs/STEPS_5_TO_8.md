# Confluence — Steps 5 to 8 explained

This file explains everything that was added for **Steps 5–8** of the roadmap, in simple words.
It continues [STEPS_1_TO_4.md](STEPS_1_TO_4.md).
For every step you get: **what it does, why it exists, the important formula, the files, the
keys, and the proof that it works.**

| Step | Topic | One-line summary |
|---|---|---|
| 5 | Wind | Wind speed drives both rotors through a real turbine power curve; clouds drift with the wind. |
| 6 | Hydro | The spillway gate moves at a limited speed; water, spray and the wheel all follow it. |
| 7 | Water, boat, solar | The lake has moving waves; the boat floats on them and cannot go on land; the solar panels tilt. |
| 8 | Interaction and HUD | Mouse picking, perspective/orthographic, five camera views, an on-screen status panel and help. |

Everything is still **C++17 + OpenGL 3.3 + GLFW + GLAD + GLM**. No new library, no Python.
Even the on-screen text is drawn by our own code with a hand-made pixel font.

---

## 1. Build and run

Same commands as before. Open **PowerShell** in the `Confluence` folder:

```powershell
$env:PATH = "C:\msys64\mingw64\bin;$env:PATH"
cmake -S . -B build -G "MinGW Makefiles"    # only needed once, or after adding files
cmake --build build -j 1
cd build
.\confluence.exe
```

- Run it **from the `build` folder**. CMake copies `shaders/` there after each build.
- New `.cpp` files are found automatically (CMake `GLOB` per folder). After **adding** a new file,
  run the `cmake -S . -B build ...` line again once so CMake sees it.

### Extra ways to run it

| Command | What it does |
|---|---|
| `.\confluence.exe --selftest` | Runs **38 checks** for Steps 1–8. No window opens. Prints PASS/FAIL. |
| `.\confluence.exe --screenshot out.bmp [options]` | Draws one frame into a picture and exits. |
| `.\confluence.exe --view dam --gate 100 --gate-target 100 --advance 3` | Starts in a chosen situation. |

New options (Steps 5–8):

| Option | Meaning |
|---|---|
| `--view overview\|turbine\|dam\|solar\|boat` | Start at this camera view |
| `--ortho` | Orthographic projection |
| `--select turbine1\|turbine2\|gate\|wheel\|solar\|boat` | Start with an object selected |
| `--no-hud`, `--help-overlay` | Hide the status panel / show the help panel |
| `--paused` | Start paused |
| `--wind M/S` | Start wind speed (0–30) |
| `--gate %`, `--gate-target %` | Gate opening now / where it is moving to |
| `--auto-gate` | Start the automatic gate demonstration |
| `--tilt DEG` | Solar panel tilt (kept inside the safe limits) |
| `--manual-boat THROTTLE` | Manual boat with this throttle (-1..1) |
| `--advance SECONDS` | Run the simulation this long before drawing (for screenshots) |

---

## 2. Controls

Click the window first so it receives the keys. Press **F11** at any time to see this list
on the screen. The **status panel** (top right) shows the live numbers; **Backspace** hides it.

| Key | Action | Step |
|---|---|---|
| Left-drag / mouse wheel | Orbit / zoom the camera | – |
| `R` | Back to the overview | – |
| `Space`, `[` `]`, `Enter` | Pause, slower/faster, reset scene | 1 |
| `1` `2` `3`, `B`, `K`, `L`, arrows/PgUp/PgDn | Shading, Blinn, light terms, lights, lamp | 3–4 |
| `X`, `N`, `F6` (+ `+` `-` `O` `M`) | Wireframe, normals, gallery | 2–4 |
| **`Q` / `E`** (hold) | Wind down / up (6 m/s per second, range 0–30) | **5** |
| **`G`** | Gate: open fully / shut fully | **6** |
| **`F` / `H`** (hold) | Gate target lower / higher | **6** |
| **`J`** | Automatic gate demo (shut → open → shut, every 40 s) | **6** |
| **`V`** | Boat: patrol (autopilot) ↔ manual | **7** |
| **`W` `S` / `A` `D`** (hold) | Manual boat: throttle / rudder | **7** |
| **`Z` / `C`** (hold) | Solar panels tilt down / up (12° per second) | **7** |
| **Right-click** | Select the object under the mouse | **8** |
| **`Tab`** / `Shift+Tab` | Select next / previous object | **8** |
| **`F1`…`F5`** | Views: overview, turbine, dam + wheel, solar, boat (follows) | **8** |
| **`P`** | Perspective ↔ orthographic | **8** |
| **`Backspace`** | Hide / show the status panel | **8** |
| **`F11`** | Help panel on screen (also printed in the console) | **8** |
| **`F12`** | Save a screenshot (`confluence_shot_N.bmp` in `build`) | **8** |
| `Esc` | Quit | – |

The console still prints one line for every key, for example `> gate: CLOSING (target 0%)`.

---

## 3. Where the code is (folder map)

Only new or changed files are listed. Each folder has one job.

```
Confluence/
  shaders/
    surface_common.glsl   NEW  waves, flowing-water streaks, sky reflection, sky colour,
                               selection glow -- shared by all programs
    phong.vert/.frag      CHANGED  wave mode: the lake vertices move up and down
    flat.vert             CHANGED  same waves in Flat mode
    sky.frag              CHANGED  uses the shared skyColour() (so water reflects the same sky)
    particles.vert/.frag  NEW  spray, mist and wake particles
    hud.vert/.frag        NEW  2D panels and text on top of the 3D picture
  src/
    App.h                 CHANGED  + camera director, particles, effects, screenshot request
    main.cpp              CHANGED  loop: input -> update (plant, effects, camera) -> render
    core/
      SceneState.h        CHANGED  PlantState: wind, gate, wheel, boat, tilt...; selection
      Camera.cpp/.h       CHANGED  perspective OR orthographic projection
      Material.h          CHANGED  + reflectivity (water mirrors the sky)
      Render.cpp/.h       CHANGED  + flow uniforms for moving water
      Shader.cpp/.h       CHANGED  several common .glsl files; remembers uniform locations
    systems/              <- "how things behave" (no OpenGL drawing in here)
      WindSystem.*        NEW  Step 5: wind -> rotor speed, clouds
      HydroSystem.*       NEW  Step 6: gate movement, flow, wheel speed, auto demo
      Waves.*             NEW  Step 7: the wave formula (same numbers as the shader)
      BoatSystem.*        NEW  Step 7: autopilot, manual driving, shore check, floating
      Particles.*         NEW  a fixed pool of 3000 particles
      Effects.*           NEW  Steps 6-7: decides where spray, mist and wake appear
      CameraDirector.*    NEW  Step 8: the five views, smooth glide, never under ground
      Animation.cpp       CHANGED  calls Wind -> Hydro -> Boat each frame; tilt limits
    input/
      Input.cpp           CHANGED  all the new keys and the right-click
      Picking.*           NEW  Step 8: mouse ray and ray-box test
      CommandLine.*       CHANGED  the new start options
    objects/              <- "what things look like"
      Placement.*         NEW  where turbines, wheel and solar array stand (from the terrain)
      Water.*             CHANGED  fine lake grid; river foam follows the gate
      Dam.*               CHANGED  gates are drawn separately and slide up/down
      SolarArray.*        CHANGED  split into fixed rack + moving panels; follows the hillside
      Scene.*             CHANGED  draws with the live state; selection glow; cloud parallax
      Boat.cpp, Rocks.cpp CHANGED  old baked wake removed; one rock moved off the solar array
    rendering/
      FrameRenderer.*     CHANGED  sky -> scene -> particles -> HUD
      ParticleRenderer.*  NEW  draws all particles in one draw call
    ui/
      BitmapFont.*        NEW  our own 5x7 pixel font (no text library)
      HudRenderer.*       NEW  draws rectangles and text in screen pixels
      HudOverlay.*        NEW  status panel, wind compass, selection panel, help panel
      Hud.cpp             CHANGED  console help for Steps 1-8
    tests/SelfTest.cpp    CHANGED  + 21 checks for Steps 5-8 (38 in total)
  captures/               pictures used as proof (section 9)
```

**Rule of thumb:** to change *how something behaves*, look in `systems/`.
To change *how it looks*, look in `objects/` or `shaders/`. Keys are in `input/Input.cpp`.

---

## 4. How one frame works now

```
main loop (main.cpp)
 ├─ Clock: real dt and simulation dt (paused -> simulation dt = 0)
 ├─ Input::processHeld     keys held down: Q/E, F/H, W/S/A/D, Z/C, lamp
 ├─ Animation::update      (simulation dt)
 │    ├─ WindSystem   wind eases to target -> rotor speeds -> rotor angles, cloud drift
 │    ├─ HydroSystem  gate moves towards target -> flow -> wheel speed -> wheel angle
 │    ├─ BoatSystem   helm -> speed/heading -> shore check -> float on the waves
 │    └─ solar tilt clamped to the safe limits
 ├─ Effects::update        (simulation dt) spawn / move / kill particles
 ├─ CameraViews::update    (REAL dt) glide, follow the boat, stay above ground
 └─ FrameRenderer::render
      sky -> terrain, water, dam, gates, wheel, turbines, solar, boat, trees ...
          -> transparent falls -> particles (sorted far to near) -> HUD on top
```

The camera uses **real** time so you can still fly around while the plant is paused.
Everything in the plant uses **simulation** time, so it pauses, speeds up and slows down
together (Step 1).

All the plant's numbers live in **one struct**, `PlantState` in `core/SceneState.h`.
Systems change it; the drawing code only reads it. That is why the self-test can run the
whole plant for minutes without opening a window.

---

## 5. Step 5 — Wind, turbines and clouds

**What:** `Q` / `E` change the wind. The rotors speed up or slow down like real machines,
and the clouds drift faster in strong wind.

**The power curve** (`WindSystem::targetRotorSpeed`) — this is how real turbines behave:

| Wind (m/s) | Rotor | Why |
|---|---|---|
| below 3 (cut-in) | stopped | not enough wind to be worth turning |
| 3 → 12 | 6 → 16 rpm, straight line | more wind, more speed |
| 12 → 25 (rated) | 16 rpm | the machine is at full power; more speed would damage it |
| above 25 (cut-out) | stopped | storm protection: the rotor is braked; HUD shows **STORM** in orange |

```
t   = clamp((wind - 3) / (12 - 3), 0, 1)
rpm = 6 + (16 - 6) * t                 rad/s = rpm * 2π / 60
```

**Smooth changes — no jumps.** A 40-tonne rotor cannot change speed instantly, so the
speed *moves towards* the target:

```
speed = speed + (target - speed) * (1 - e^(-dt / 3 s))
angle = angle + speed * dt
```

- The `e^(-dt/τ)` form gives the **same result at 30 FPS or 144 FPS** (Step 1's rule).
- The angle is **added up** (`angle += speed·dt`), never `speed × totalTime`. Otherwise the
  rotor would jump to a new angle the moment the speed changes.
- Turbine 2 turns at **0.93 ×** turbine 1, so the two do not look like copies.

**Clouds:** `cloud speed = 0.5 + 0.55 × wind` (m/s). Each cloud has a **parallax** factor:
far clouds move less, which gives depth. When a cloud leaves the sky at x = −520 it re-enters at
+520, far out of view (`WindSystem::cloudX`, using `fmod`).

**Wind compass** (under the status panel): the arrow points where the wind blows and gets
longer in stronger wind.

**Files:** `systems/WindSystem.*`, `objects/Scene.cpp` (rotor and cloud drawing), `ui/HudOverlay.cpp`.

**Proof:** self-test Step 5 (4 checks); captures `50_scene_t6.png` (normal) and `56_storm.png`
(28 m/s: rotors stopped, STORM label).

---

## 6. Step 6 — Gate, water flow and water wheel

**What:** the spillway gates slide up and down. **Everything connected to the water follows
the gate**: the falling water, the white spray and mist, the river foam, the small fall
behind the wheel and the wheel's speed.

**The gate cannot teleport.** You set a *target* (`G`, `F`/`H`); the gate moves towards it at
most **12.5 % per second** (shut → open in 8 s), like real gate motors:

```
step    = 0.125 * dt
opening = opening + clamp(target - opening, -step, +step)
opening = clamp(opening, 0, 1)          // never outside its slot
flow    = opening
```

**What `flow` controls:**

| Thing | How it follows the flow | Code |
|---|---|---|
| Gate slabs | slide up by `opening` × bay height | `Dam::drawGates` |
| Spillway water | transparency and streak speed rise with the flow | `Dam.cpp` |
| Spray, mist | particles per second = `230 × flow`, `45 × flow` | `Effects.cpp` |
| Wheel channel fall | thinner when barely open; gone when shut | `Water::drawFalls` |
| River foam | white water fades to calm blue as the flow drops | `surface_common.glsl` (`uFoam`) |
| Wheel | target speed = `flow × 1.15 rad/s` (≈ 11 rpm), eased in over 2.5 s | `HydroSystem` |

**`J` — automatic demo:** the target follows a slow wave, `0.5 − 0.5·cos(2π·t / 40 s)`,
so the gate goes shut → open → shut by itself. Pressing `G`, `F` or `H` takes control back.

**Particles (spray, mist, wheel splash, boat wake):**
- One pool of **3000** particles is made at start-up and re-used, so no memory is allocated
  during the animation.
- Each particle has a position, velocity, age and lifetime. Every frame:
  `velocity.y -= gravity·dt`, `position += velocity·dt`; it fades in, then fades out, and dies.
- Spray particles **face the camera** (billboards). Wake particles lie **flat on the water**.
- They are see-through, so they are **sorted far → near** and drawn in **one draw call**
  (`rendering/ParticleRenderer.cpp`). Wrong order would make near particles hide far ones.
- When paused, the particles freeze too (they use simulation time).

**Files:** `systems/HydroSystem.*`, `systems/Effects.*`, `systems/Particles.*`,
`rendering/ParticleRenderer.*`, `objects/Dam.*`, `objects/Water.*`, `shaders/particles.*`.

**Proof:** self-test Step 6 (5 checks), e.g. *"gate open: 1399 spray particles; 18 s after
shutting: 0"*. Captures `57_gate_closed.png`, `63_gate_full.png`, `64_auto_gate.png`.

---

## 7. Step 7 — Lake waves, the boat and the solar tracker

### 7a. Waves

The lake height is the **sum of three sine waves** going in different directions:

```
h(x, z, t) = lakeLevel + Σ  A_i · sin( k_i · (d_i · (x, z)) − ω_i · t + φ_i )

k = 2π / wavelength                (how many crests per metre)
ω = k · sqrt(g · wavelength / 2π)  (deep-water rule: long waves travel faster)
```

| Wave | Direction | Height A | Wavelength |
|---|---|---|---|
| 1 | with the wind (−x, a bit sideways) | 0.13 m | 12 m |
| 2 | diagonal | 0.07 m | 7 m |
| 3 | across | 0.04 m | 4.5 m |

- **Stronger wind → taller waves:** every A is multiplied by `clamp(0.35 + wind/12, 0.35, 1.6)`.
- The **vertex shader** moves each lake vertex to `h`. That is why the lake is now a fine
  grid (1.2 m squares near the camera). Big triangles could not bend.
- The **normal** comes from the slope: `n = normalize(−∂h/∂x, 1, −∂h/∂z)`. The derivative of
  `A·sin(...)` is `A·k·d·cos(...)`, so it is exact, not guessed.
- **The CPU and the GPU use the same numbers.** `Waves::uploadToShader` sends them, so the
  boat (CPU) floats on exactly the waves you see (GPU).
- **Reflection:** water mirrors the sky more when you look along it, as in real life
  (Fresnel/Schlick: `F = 0.02 + 0.98·(1 − cosθ)^5`). It uses the same `skyColour()` as the sky.
- **Far away the waves fade out** (250–420 m), and the tiny ripples fade out at 40–160 m.
  Otherwise the distant water flickers (too many waves per pixel = aliasing).

### 7b. The boat

**Two modes** (`V`):
- **Patrol** — an autopilot drives around an ellipse (centre (40, −145), radii 38 × 26 m) at
  4 m/s. It looks 14 m ahead on the route and turns the rudder towards that point.
- **Manual** — `W`/`S` throttle (max 8 m/s forward, 2.5 m/s astern), `A`/`D` rudder.

Driving is simple but feels like a boat:
- speed changes by at most **1.6 m/s per second**;
- the rudder works better at speed (`effectiveness = clamp(|speed|/3, 0.25, 1)`);
- in reverse the steering is reversed, as on a real boat.

**The boat can never go on land.** Before moving, `hullFits` tests **4 points** (bow, stern,
left, right) with `isNavigable`: the ground there must be at least **0.8 m below** the lake
surface, and the boat must stay **9 m away from the dam**. If the next position fails, the boat
stops (HUD: *STOPPED: SHORE OR DAM AHEAD*). You can still turn and back away.

**Floating:** the wave height is read at bow, stern, left and right:

```
height = average of the 4
pitch  = atan((bow − stern) / hull length)       (+ a little bow-up when fast)
roll   = atan((left − right) / hull width)
model  = T(x, height, z) · Ry(heading) · Rx(−pitch) · Rz(roll)
```

The **wake** is made of flat foam particles dropped behind the stern (7 per metre). They spread
sideways, fade, and stay on the moving waves.

### 7c. Solar tracker

Real solar farms often use **single-axis trackers**: the panels turn about a long tube to face
the sun. `Z` / `C` turn them.

- The array is now **two meshes**: the **rack** (posts and tube, fixed to the ground) and the
  **panels** (glass, frames, rails, which turn). A moving part cannot be baked into a fixed mesh.
- **Rotation about a pivot** (Step 2's idea, used for real):
  ```
  panels = root · T(tube centre) · Rz(slope) · Rx(tilt − 90°) · T(panel centre → origin)
  ```
  Read right to left: move the panel centre to the origin, tilt it about x (the tube),
  roll it to follow the slope, then move it onto the tube.
- **The tube follows the hillside.** The ground falls **4.5 m** from one end of the array
  to the other. With a level tube, one end was buried at any tilt. Now the tube is 2 m above
  the ground at both ends, and every post reaches down into the real ground.
- **Tilt limits are calculated, not guessed.** `SolarArray::tiltLimits` checks tilts from
  10° upwards. For each tilt it measures the gap between the panel and the ground on a 21 × 9
  grid of points, and keeps the steepest tilt that leaves at least **0.35 m**. Result:
  **10° … 33°** (start: 30°).

**Files:** `systems/Waves.*`, `systems/BoatSystem.*`, `systems/Effects.*`,
`objects/SolarArray.*`, `shaders/surface_common.glsl`, `shaders/phong.vert`.

**Proof:** self-test Step 7 (7 checks), e.g. *"boat height vs wave height under it: within
0.115 m"*, *"full throttle for 3 minutes: stopped at the shore, hull never on land"*.
Captures `51_view_boat.png`, `61_flat_waves.png`, `54_view_solar_10.png`, `55_view_solar_max.png`.

---

## 8. Step 8 — Picking, projection, camera views and the HUD

### 8a. Mouse picking (right-click)

How do we know which object is under the mouse? We shoot a **ray** from the camera through
the pixel.

1. **Pixel → NDC** (−1…+1). Window y grows down, OpenGL y grows up, so y is flipped.
   (On high-DPI screens the window coordinates are converted to framebuffer pixels first.)
2. **NDC → world:** multiply the near point (z = −1) and the far point (z = +1) by
   `inverse(Projection · View)`, then divide by `w`. The ray goes from near to far.
   This works for **both** perspective and orthographic, with no special case.
3. **Ray vs box:** every selectable object has a box in its *own* frame. The ray is moved into
   that frame with `inverse(model)` (point: w = 1, direction: w = 0). Then the **slab test**:
   for x, y and z, find where the ray enters and leaves that pair of planes. If the three
   intervals overlap, it hits.
4. The **nearest** hit wins. It glows with a soft pulsing orange (`uHighlight`), and a panel at the
   bottom left shows its live numbers and keys.

`Tab` cycles through the six objects without the mouse.

### 8b. Perspective ↔ orthographic (`P`)

- **Perspective:** far things are smaller (`glm::perspective`, 48° vertical field of view).
- **Orthographic:** parallel rays, no shrinking. The box size follows the zoom
  (`halfHeight = radius · tan(fov/2)`), so switching keeps about the same framing.
  The box also extends **behind** the eye. An ortho camera has no real eye point, so hills
  between the orbit point and the target must not be cut away.
- The sky always uses perspective (a sky has no size in ortho).

### 8c. Camera views (`F1`–`F5`)

Each view is an **eye** and a **target**, worked out from the objects' real positions (so if
an object moves, its view moves with it). Changing view **glides** in 1.8 s:

```
s      = smoothstep(progress)              slow start, slow end
target = mix(from, to, s)
radius = exp(mix(log r0, log r1, s))       distance blends evenly (200 m -> 20 m does not rush)
yaw    = the short way round (350° -> 10° is +20°, not −340°)
```

- **F5** follows the boat; after arriving you can still orbit and zoom around it.
- **Camera never goes under the ground:** if the eye would be inside the hill, the pitch is
  raised in half-degree steps until the eye is 1.5 m above the terrain.
- Scrolling or dragging during a glide cancels it, so you are always in control.

### 8d. The HUD (on-screen panels) — without a text library

- `ui/BitmapFont.cpp` stores every letter as **7 rows of 5 bits** (like an old LCD). A `1` bit is
  drawn as a small square. That is the whole font, with no library and no image file.
- `ui/HudRenderer.cpp` collects rectangles in **pixel coordinates** and draws them in one
  call with an orthographic matrix `(0, width, height, 0)`. Depth test is off and blending is on,
  so panels are see-through.
- `ui/HudOverlay.cpp` builds the panels:
  - **status** (top right): view, shading, time, wind, gate, flow, boat, tilt, with bars;
  - **wind compass**;
  - **selection** panel (bottom left);
  - **help** panel (`F11`);
  - **hint line** with the FPS (bottom).
- A soft **vignette** (darker corners) is drawn first; it gives the cinematic look.

**Files:** `input/Picking.*`, `core/Camera.*`, `systems/CameraDirector.*`, `ui/*`,
`shaders/hud.*`, `rendering/FrameRenderer.cpp`.

**Proof:** self-test Step 8 (5 checks), e.g. *"hub → WIND TURBINE 1, boat → BOAT, sky →
NOTHING"*, *"centre pixel's ray points at the camera target in both"*. Captures
`52_view_dam.png`, `53_view_turbine.png`, `58_help_overlay.png`, `59_ortho.png`,
`62_select_wheel.png`.

---

## 9. Checks that were run

1. **Build:** `cmake --build build -j 1` with no errors.
2. **Strict compile:** all 43 `.cpp` files checked with `g++ -Wall -Wextra -Wshadow`:
   **0 warnings**.
3. **Self-test:** `.\confluence.exe --selftest` → **38 of 38 checks passed** (17 from Steps 1–4,
   21 new):

```
STEP 5 - wind, turbines, clouds
  [PASS] rotor curve: cut-in, rated, cut-out            2 m/s: 0 rpm, 3: 6 rpm, 12-20: 16 rpm, 26 (storm): 0
  [PASS] wind change: rotor speeds up smoothly          largest change in one frame 0.0018 rad/s; after 12 s 15.9 of 16 rpm
  [PASS] the two turbines are independent               turbine 2 turns at 14.8 rpm (factor 0.93)
  [PASS] clouds drift and wrap out of sight             x stays in [-520, 520]; largest step while |x| < 300: 0.50 m
STEP 6 - gate, flow, wheel
  [PASS] gate travels at its limited speed              0.45 -> 1.00 at 12.5%/s: at 4 s 0.95, at 6 s 1.00
  [PASS] gate stays inside its slot (0..1)              pushed far past both ends: clamped
  [PASS] wheel speed follows the gate                   open: 11.0 rpm; shut: slows smoothly to 0.00 rpm
  [PASS] J: automatic gate demonstration                gate went from 0.00 to 1.00 by itself
  [PASS] spray follows the flow                         gate open: 1399 spray particles; 18 s after shutting: 0
STEP 7 - waves and boat
  [PASS] wave normal = slope of the wave height         largest difference over 50 points: 0.00022
  [PASS] stronger wind -> taller waves                  scale 0.43 in calm air, 1.60 in a gale
  [PASS] patrol route is on open water                  360 of 360 route points fit the hull
  [PASS] patrol: 2 minutes on the water                 travelled 480 m, never left navigable water
  [PASS] the boat rides the drawn waves                 boat height vs wave height under it: within 0.115 m
  [PASS] manual: the shore stops the boat               full throttle for 3 minutes: stopped at the shore, hull never on land
  [PASS] solar tilt limits keep the panels off the slope range 10..33 deg (includes the 30 deg start); edge clearance at the limits >= 0.39 m
STEP 8 - picking, projection, camera views
  [PASS] picking finds the object under the ray         hub -> WIND TURBINE 1, boat -> BOAT, sky -> NOTHING
  [PASS] mouse ray works in perspective and ortho       centre pixel's ray points at the camera target in both
  [PASS] camera glides to a view and stops there        distance 81 m -> 43 m (after 1 s) -> 27.2 m
  [PASS] camera never goes under the ground             eye 1.6 m above the terrain
  [PASS] particle pool is bounded and pauses            1409 alive of 3000 slots; a paused frame changes nothing
38 of 38 checks passed.
```

4. **Live window test:** the real window was opened and keys were sent to it automatically
   (`F3`, `G`, `Tab`, `F5`, `V`, `F11`, `F12`, `Esc`). Every key printed its console line, three
   `F12` screenshots were saved, and `Esc` closed the program cleanly. The window ran at about
   **60 FPS**.
5. **Pictures** of every feature were rendered and looked at one by one (`captures/50`–`64`).

### Pictures (in `captures/`)

| File | Shows |
|---|---|
| `50_scene_t6.png` | The overview after 6 s: rotors, falls, spray, HUD, compass |
| `51_view_boat.png` | F5: the boat on the waves with its wake |
| `52_view_dam.png` | F3: gates, falls, powerhouse, wheel |
| `53_view_turbine.png` | F2: a rotor up close |
| `54_view_solar_10.png`, `55_view_solar_max.png` | Solar tracker at 10° and at its 33° limit |
| `56_storm.png` | 28 m/s: rotors stopped, STORM label |
| `57_gate_closed.png`, `63_gate_full.png`, `64_auto_gate.png` | Gate shut (calm river), fully open, J demo |
| `58_help_overlay.png` | F11 help panel |
| `59_ortho.png` | Orthographic projection |
| `60_gallery.png` | The Step 2–4 gallery still works with the new shaders |
| `61_flat_waves.png` | Flat shading also shows the waves |
| `62_select_wheel.png` | Selected water wheel with its panel |

### Problems found while testing and fixed

- **Whole scene turned cream-coloured.** When nothing was selected, "glow if this is the
  selected object" was true for *everything*, because "None == None". Fixed:
  `on = who != None && who == selected`.
- **Patrol route crossed land.** The first ellipse touched the shore. I mapped where
  `isNavigable` is true and moved the route. The self-test now checks 360 points of it.
- **Solar panels dug into the hill.** The ground falls 4.5 m along the array, so even the old
  fixed array was partly underground. Fixed by making the tube follow the slope, sinking
  the posts into the real ground, measuring the clearance over the whole panel, and moving one
  rock that sat under the array.
- **Lake flickered in the distance** (aliasing). Fixed by fading the waves out far away and
  making them a bit smaller.
- **F3 camera ended inside a hill / trees.** The eye is now placed over the river.
- **Orthographic view cut off the near hill.** The ortho box now also reaches behind the eye.
- **River stayed white with the gate shut.** The foam now follows the flow.
- **HUD covered the turbines.** It moved to the top right.

---

## 10. Honest notes and limits

- **No new library.** GLFW (window), GLAD (OpenGL loader) and GLM (vector/matrix types) were
  already used. The font, HUD, particles, waves, picking and camera code are all our own.
- **The river's foam pattern is baked** into the river mesh's colours, and its strength is
  scaled by the gate in the shader. The pattern itself does not change shape; it drifts.
- **The waves fade out far away** (250–420 m). This is on purpose, to avoid flicker.
- **In a storm the rotors stop**, but the blade pitch angle is not animated (the blades do not
  visibly turn edge-on).
- **The boat's shore check uses 4 points**, which is enough for this small hull. A long ship
  would need more.
- **Orthographic view** shows the edge of the modelled world at the bottom, because it sees
  further than perspective does.
- **Sliders on screen were not added.** All values are set with keys and shown as bars.
- The **sun does not move** (no day/night). That would be a later step.

---

## 11. Short answers for the viva

- **Why ease speeds with `1 − e^(−dt/τ)`?** It is smooth and gives the same result at any
  frame rate.
- **Why `angle += speed·dt` and not `speed·t`?** With `speed·t`, changing the speed would make
  the rotor jump.
- **Why a target and a limited speed for the gate?** Real gates move slowly, and every effect
  that follows the gate then changes smoothly too.
- **How do the waves get their normals?** From the derivative of the sine sum:
  `n = normalize(−∂h/∂x, 1, −∂h/∂z)`.
- **Why the same wave code on CPU and GPU?** The GPU draws the waves, but the boat (CPU) must
  float on exactly those waves.
- **How does the boat know where land is?** It reads the terrain height at 4 hull points. The
  water there must be at least 0.8 m deep.
- **How does picking work?** Pixel → NDC → `inverse(P·V)` gives a ray. The ray is moved into each
  object's frame with `inverse(M)`, then a slab test finds the hit; the nearest hit wins.
- **Why sort particles?** They are transparent. Far ones must be drawn first so near ones
  blend over them.
- **Perspective vs orthographic?** Perspective divides by depth (far = small); orthographic
  does not (parallel rays), which is useful for technical views.
- **How is text drawn without a library?** Each letter is a 5×7 grid of bits; each `1` becomes a
  small rectangle.

## 12. Next: Step 9 and later

Good next additions, with their place in the code:

- **Power lines and a small substation** connecting the turbines, solar array and
  powerhouse (new files in `objects/`). This is what makes it read as a real power plant.
- **Day/night:** move `uSunDirection` over time in `systems/Environment.cpp`.
- **Blade pitch** when the wind is above 25 m/s (in `objects/Turbine.cpp`).
