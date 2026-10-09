# How it works (simple English)

This file explains each required graphics topic, where it is in the code, and
how to show it while the program is running.

| Requirement | Where to look | How to show it live |
|---|---|---|
| Lighting | `shaders/common.glsl` → `shade()`, `World::lighting()` | daylight above water, blue light under water, lamps in the hall; **L** = shadows on/off |
| Transformation | `src/core/MathLib.h`, gates, valve, guide vanes, runner, rotor, fans, debris, cameras | gates and valve rise, 24 guide vanes turn, runner and rotor spin, the dam section lifts away |
| Gouraud & Phong | `shaders/gouraud.vert/.frag` vs `shaders/lit.vert/.frag` | press **G** (MIXED → PHONG → GOURAUD) |
| Motion / animation | `World::update()`, `ParticleSystem`, `SpillwayWater`, `Debris`, `FlowLines` | water fronts move with real physics, the turbine speeds up, current glows; **[ ]** change the speed |
| Interaction | `App.cpp` (`onKey`, `onMouseButton`, …) + each scene's `buttons()` | on-screen buttons, keys, mouse orbit / zoom, speed control |
| Ray tracing | `shaders/raytrace.frag`, `src/scenes/Scene5_RayTracedPipe.cpp` | scene **5** |

---

## 1. One world, one journey (continuity)

`src/objects/World.cpp` holds the **whole plant**: terrain, reservoir, dam,
intake, gate, conduit, penstock, valve, turbine, generator, transformer,
switchyard, power line and river. All 11 scenes look at this one object, so
nothing is duplicated and the water (and later the electricity) really
travels:

```
reservoir -> trash-rack cage -> intake gate -> conduit -> penstock
  -> MAIN INLET VALVE -> spiral casing -> guide vanes -> RUNNER -> draft tube -> river
runner -> shaft -> GENERATOR rotor                          (mechanical energy)
generator -> bus ducts -> STEP-UP TRANSFORMER -> switchyard
  -> CIRCUIT BREAKER -> transmission line                   (electrical energy)
```

* **Cause and effect** (`World::update`). Each step only happens after the
  step before it:

  | effect | needs |
  |---|---|
  | conduit fills | intake gate open |
  | penstock fills | conduit full |
  | spiral casing fills (`casingFront`) | valve more than 30 % open AND penstock full |
  | water flows through the runner (`flow01()`) | casing full AND valve open; amount = guide-vane opening |
  | runner speeds up (`rotorSpeed`) | flow; it slowly approaches `flow × full speed` (inertia) |
  | voltage (`voltageKV()`) | 11 kV × field current × (rpm / 150) |
  | power line live (`gridFront`) | breaker closed AND more than 9 kV |
  | power (`powerMW()`) | P = ρ·g·Q·H·η, only when the breaker is closed |

* **One data sheet** (`src/objects/PlantData.h`). Every number on screen
  comes from here, so the numbers always agree: head 53 m, 180 m³/s, 92 %
  efficiency → 86 MW; 40 poles × 150 rpm / 120 = 50 Hz; 11 kV → 220 kV.
  The 3D rotor turns slower than 150 rpm so the eye can follow it
  (`VISUAL_OMEGA`), but the displayed rpm is converted back.
* **Jumping to a scene** (keys 1–0): `World::completeUpTo(stage)` does every
  earlier step instantly (gate open, pipe full, …), so the scene starts in
  the right state.
* **SHOW WATER MOVEMENT** (`World::beginJourney`) closes everything: intake
  gate, valve, guide vanes, field current, breaker. The water drains and the
  machines run down, and then the journey starts again from the reservoir.
* **Camera continuity**: when the story moves on, `App::continueTo()` hands
  the current camera position to the next scene (`fromEye`/`fromTarget`).
  The next camera path starts from there, so the camera keeps flying
  (generator → over the roof → transformer → switchyard → along the line).
* **Cut-aways**: the intake's near half and the water next to it can be
  hidden (`waterCut`, `nearSideHidden`). The dam block lifts away
  (`cutLift`). The valve body, spiral casing (open on top), generator housing
  (a 120° window) and powerhouse roof edge are built open, like the video's
  cut-away diagrams.
* **Cross-dissolve** (`render/FrameCapture.cpp`, `shaders/blit.frag`): where
  the picture style changes (under water → cut-away, raster → ray tracing,
  ray tracing → valve), the last frame is kept in a texture and faded out
  over the new one.

## 2. The frame, step by step (`App::update` + `App::render`)

1. `update`: the scene advances its story by `dt × SPEED`, and the
   director's camera is evaluated.
2. **Shadow pass**: the world is drawn from the sun into a 2048×2048 depth
   texture (`Renderer::beginShadowPass`). It is centred on the place the
   scene looks at (`Scene::shadowCentre()`).
3. **Main pass** into an off-screen texture: the sky, then solid objects,
   then transparent things (water, glass, glowing flow lines, particles) with
   blending.
4. **Post-processing** (`render/PostFX.cpp`), explained in section 9.
5. The picture is copied for possible dissolves (`FrameCapture::captureLive`).
6. **HUD**: buttons, telemetry, pointer labels and the oscilloscope, drawn in
   pixel coordinates (`ui/Ui.cpp`).

Scene 5 is different: one full-screen triangle is drawn and the ray tracer
fragment shader computes every pixel.

**Culling** (`World::drawOpaque`): things that cannot be seen are not drawn.
The detailed machines (valve, turbine, generator) are only drawn near the
powerhouse (`nearHouse`). The transformer, yard and power line are skipped
while the camera is inside the hall (`insideHall`). This keeps the program
fast (33–58 fps at 1600×900 on a GTX 1650).

## 3. Lighting

`shade()` in `shaders/common.glsl` is the **Blinn-Phong** equation:

```
colour = albedo * ambient                      (light from sky + ground)
       + albedo * sunColour * max(N·L, 0) * shadow       (diffuse)
       + sunColour * (N·H)^shininess * specular * shadow (specular highlight)
       + point lights with distance fall-off            (powerhouse lamps)
       + sky reflection * Fresnel                        (metal shine)
       + emissive                                        (glowing lines, lamps)
       + caustics                                        (under water)
```

* **Sun** (directional light) and **hemisphere ambient**. There is one
  daylight for the whole world (`World::lighting`). When the camera dives
  below the reservoir surface, it switches to blue under-water light with
  thick fog that absorbs red first, plus caustics.
* **Point lights**: lamps inside the powerhouse. The bright hall in the video
  (2:00) gets its light from them and from the windows.
* **Shadows**: shadow mapping with 3×3 PCF soft edges (`shadowFactor()`).
* **Tone mapping**: the ACES film curve plus gamma (`finalColor()`).

## 4. Transformations

All matrices are hand-written in `src/core/MathLib.h`: `translate`, `scale`,
`rotateX/Y/Z`, `rotateAxis`, `lookAt`, `perspective`, `ortho`, `normalMatrix`.

* **Spillway gates** (`Dam::draw`): one mesh drawn three times with
  `translate(bay + lift)`.
* **Dam cut block** (`Dam::draw`): `translate(0, ease(cutLift) · 160, 0)`
  lifts the whole section away.
* **Intake gate** (`IntakeGate::draw`): a hierarchical transformation.
  `base * translate(0, lift, 0)` is used for the leaf AND the piston rods, so
  the rods move with the gate into the fixed cylinders.
* **Main inlet valve** (`MainInletValve::draw`): the gate and stem are
  translated up by `open × STROKE`. The hand-wheel is
  `translate(top) * rotateY(wheelAngle)`; it only turns while the gate moves.
* **Guide vanes** (`TurbineUnit::draw`): all 24 vanes use the same mesh. Each
  one is placed on the circle and turned about its own pivot:
  `translate(pivot_k) * rotateY(-(θ_k + 90° + β))`, with
  β = 8° + opening × 38°. The operating ring above them turns a little with
  the opening, like the real linkage.
* **Runner, shaft, generator rotor**: `translate(centre) * rotateY(rotorAngle)`.
  They all use the SAME angle, so they turn together, as on one shaft.
* **Transformer fans**: `rotate` by `fanAngle`, which only grows while there
  is voltage.
* **Breaker** (`Switchyard::draw`): the status lamps fade from red (open) to
  green (closed) with `breaker01`.
* **Debris** (`Debris::draw`): `parent * translate(pos) * rotateY * rotateZ * rotateX * scale`.
* **MeshBuilder** (`core/Mesh.cpp`): shapes are added through a transform
  (`setTransform`, and `alignY()` points a cylinder from A to B). This builds
  the 72 stator slots and 40 rotor poles by rotating one piece around the
  axis.
* **Camera**: `lookAt` plus the user's orbit (yaw/pitch/zoom/pan).

## 5. Gouraud vs Phong shading

* **Phong** (`lit.vert` + `lit.frag`): the full lighting is calculated **for
  every pixel**, so highlights are sharp.
* **Gouraud** (`gouraud.vert` + `gouraud.frag`): the lighting is calculated
  **only at the vertices** and blended across the triangle. It is cheaper,
  but highlights blur. Look at the shiny runner or the generator copper
  with **G** to see the difference.

In **MIXED** mode the large smooth terrain uses Gouraud and everything else
uses Phong. Press **G** to compare.

## 6. Motion and animation

* **Free-fall water fronts**: spillway (`SpillwayWater`) and penstock
  (`World::pipeSpeedAt`) use `v = sqrt(v0² + 2·g·h)`.
* **Gate jet** (`World::update`, `conduitProfile`): Torricelli's law,
  v = √(2gh).
* **Spiral casing filling** (`TurbineUnit::drawWater`): the water surface
  grows along the snail-shaped casing up to `casingFront`.
* **Inertia**: the runner does not jump to full speed; each frame it moves a
  part of the way: `speed += (target − speed) · dt · 0.45`. The field
  current, valve, vanes and breaker all move at limited rates (`clampf`).
* **Glowing flow lines** (`objects/FlowLines.cpp`, `PAT_FLOW` in
  `materials.glsl`): thin tubes along paths, with bright pulses running
  along them: `fract(distance − time × speed)`. One technique shows four
  things:
  - blue **water streamlines** through the casing, runner and draft tube,
  - blue **magnetic field lines** around the spinning poles,
  - orange **three-phase current** in the stator coils and bus ducts (the
    pulses move at the frequency),
  - the **energy front** along the power line. A `front` value hides
    everything beyond the place the energy has reached.
* **Oscilloscope** (`Scene9_Generator::hud`): three sine waves,
  `sin(2π(cycles·u − t·speed) − p·120°)`. The height is the voltage, so it
  grows when you excite the rotor.
* **Particles**: spray, mist, outflow, gate spray, debris splashes, and
  under-water dust.
* **Debris physics**: current, buoyancy, bobbing, tumbling, stopping at the
  bars, and small leaves sinking into the intake.
* **Cameras**: Catmull-Rom spline paths (`evalCamPath`). In scene 11 the
  camera follows the energy along the line (`TransmissionLine::pointAlong`).
* **SPEED control**: every scene update gets `dt × speed` (0.25×–2×), so the
  whole world moves in slow or fast motion.

## 7. Interaction

* **Scene buttons**, which you can click or trigger with their key (see the
  table in the README): STOP/START WATER, OPEN GATE, X-RAY, OPEN VALVE, OPEN
  GUIDE VANES, SECTION VIEW, EXCITE ROTOR, X-RAY TRANSFORMER, CLOSE BREAKER,
  FOLLOW …, REPLAY JOURNEY.
* **Disabled buttons** (`Button::enabled` + `hint`): a FOLLOW button is grey
  until its cause has happened, and the text under it says what to do.
  Examples: FOLLOW TO TURBINE waits until the casing is full, and FOLLOW THE
  POWER waits for 10.5 kV.
* **Speed**: the speed chips or the **[ ]** keys.
* **Mouse**: orbit, pan and zoom. **C** goes back to the director's camera.
* **Output**: the TELEMETRY panel shows live values from the simulation
  (rpm, flow, frequency, voltage, power, how far the line is energised).

## 8. Ray tracing (scene 5)

`shaders/raytrace.frag` is a **Whitted-style ray tracer** on the GPU, one ray
per pixel:

1. **Primary ray** from the camera through the pixel.
2. **Exact intersections**: the pipe as an infinite cylinder (a quadratic
   equation), split into glass and steel sections; flanges and bands as
   capped cylinders; supports as boxes (slab method); the floor and wall as
   planes.
3. **Shadow rays** towards the light. Glass lets blue light through.
4. **Reflection rays** from the glossy floor, with Fresnel strength.
5. **Refraction** into and out of the water (Snell's law, n = 1.33), with
   total internal reflection.
6. **Volume ray-marching** (36 steps) through the water, adding up the glow
   of the moving streams (`waterGlow()`).

UP/DOWN change the head; the C++ side computes v = √(2gh) and the shader's
streams speed up or slow down.

## 9. Post-processing: the cinematic look (`render/PostFX.cpp`)

The 3D picture is first drawn into a texture (an FBO). Then it passes
through three small shaders:

1. **Bright pass** (`post_bright.frag`): keeps only the parts brighter than
   a threshold (lamps, glowing current, sun glints) at a quarter of the
   screen size.
2. **Blur** (`post_blur.frag`): a Gaussian blur, first horizontally and then
   vertically (two passes are much cheaper than one 2D blur).
3. **Composite** (`post_composite.frag`): original + blurred glow (**bloom**),
   darker corners (**vignette**) and a little moving noise (**film grain**).

It costs about 1 ms. Press **V** to compare with and without it, or start
with `--nopost`.

## 10. Other techniques

* **Procedural textures** (`materials.glsl`): concrete, steel, paint, bark,
  rock/grass/silt, soil, tiles, wire mesh (cut out with `discard`), copper
  windings, laminated stator steel, hologram grid, leaves, flow pulses.
* **Water shader** (`water.frag`): Fresnel reflection, glitter, foam, cut-face
  mode for the cut-away, Snell's window from below.
* **Procedural geometry** (`core/Mesh.cpp`): boxes, cylinders, spheres, tori,
  tubes along rounded paths, and extruded dam profiles. The spiral casing is
  a tube whose radius shrinks as it wraps round. The runner blades are
  curved strips. The power-line wires hang in sagging curves (a parabola,
  close to a catenary) between lattice towers.
* **Hand-made font** (`ui/Font.cpp`) and **BMP writer** (`core/Screenshot.cpp`).
