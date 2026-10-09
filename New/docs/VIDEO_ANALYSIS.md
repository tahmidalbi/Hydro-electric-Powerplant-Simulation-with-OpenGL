# Video analysis — 0:00 to 1:10

Video: *Paani Se Bijli Kaise Banti Hai? Dam Hydroelectric Power Plant (3D Animation)*,
https://youtu.be/sPXvq6cITFk (3:00 long, 1280×720, 24 fps).

The video was downloaded and cut into frames, 2 frames per second, from 0:00
to 1:12. The important moments were then studied at full size. Below is what
happens in every shot, what it looks like, and how this project recreates it.

---

## Shot 1 — 0:00 to 0:10 · Spillway and powerhouse

**0:00–0:04.** High camera, looking at a large grey concrete gravity dam. The
top has gate piers, steel gate frames and a **yellow gantry crane**. The
reservoir is dark blue behind it, and there is a dark rocky mountain on the
right. **Water starts pouring through a spillway bay** and races down a long
curved chute (ogee crest → steep chute). It is bright blue with white streaks
and sparkles. The camera moves down with the water. At the bottom right are the
powerhouse building, a switchyard with lattice transmission towers, and the
river.

**0:04–0:10.** Cut to a **cut-away** of the powerhouse. Water comes down the
dam slope from the upper left, flows through a pipe into a **silver bulb
turbine**, and leaves on the right as **white foaming water** into the river.
Above the turbine is the **generator**: a grey-blue cap on top, a **copper-red
rotor** with poles, cabinets, and a round blue motor. The lighting is soft and
neutral, with a dark background.

**In the project → Scene 1** (`src/scenes/Scene1_Spillway.cpp`):

- dam with an ogee spillway and three bays, piers, lift gates, hoists, yellow
  gantry crane, lamp posts (`objects/Dam.cpp`)
- water sheets in the chutes whose front moves with free-fall physics
  (`objects/SpillwayWater.cpp`)
- spray and mist particles where the water hits the river, foam on the river
- cut-away powerhouse: penstock, bulb turbine, shaft, copper rotor with poles
  that spin, stator with a window, cabinets with LEDs, round motor, crane
  (`objects/Powerhouse.cpp`)
- switchyard with lattice towers, transformers, insulators and wires
  (`objects/Switchyard.cpp`)
- camera: a crane shot from high above, down along the water, round to the
  cut-away
- extra (asked for by the user): **STOP WATER** closes the gates and the water
  drains; **START WATER** releases it again; **FOLLOW WATER** starts the
  journey to the intake.

## Shot 2 — 0:10 to 0:20 · Dam overview with measurements

The same kind of dam in **bright daylight**, seen from the front-left. The
reservoir is visible on the left, and the gates are **closed** (the chutes are
dry). A thin **viewfinder frame** appears, and white **dimension lines with
labels** are drawn on the left part of the dam (heights and lengths). The
camera slowly pushes in.

**In the project → Scene 2** (`Scene2_DamSurvey.cpp`): the same dam site with
the gates closed, daylight lighting, a viewfinder frame with tick marks, and
four animated dimension lines (dam height 63 m, head 53 m, crest length 300 m,
spillway 3 × 11 m) plus a reservoir level marker. The lines are 3D points
projected onto the screen every frame.

## Shot 3 — 0:20 to 0:30 · Under water at the intake

Blue **under-water** view of the dam's upstream face. The rippling water
surface is at the top, **concrete panels** have **caustic light patterns** on
them, and **rock piles** sit in the bottom corners. In the centre is the
intake: a dark recess with a sloping hood and a **steel trash rack** (vertical
bars, top and bottom beams with **bolted brackets**, pointed bar tops, a
lifting lug). The camera moves straight in. The bars get bigger and bigger and
the camera **passes between them** into the tunnel, where a blue opening is
visible at the far end.

**In the project → Scene 3** (`Scene3_Underwater.cpp`, `objects/TrashRack.cpp`,
`objects/Rocks.cpp`): under-water fog that absorbs red light first, animated
caustics on every surface, the water surface seen from below (Snell's window /
total internal reflection), light shafts, floating dust and bubbles, a rack
with bolts and brackets, rocks, and a tunnel with a glowing end. The camera
path swims through the gap between two bars.

## Shot 4 — 0:30 to 0:40 · Trash rack filtering (cut-away)

A clean **cut-away diagram**. A concrete channel is sliced open, with **brown
soil** under the slabs. A light grey wall and a railing are at the top right.
A **cage of dark steel bars** stands in the middle, with a heavy concrete beam
and **two blue motors** on top. **Light-blue water** flows from left to right
with a wavy surface. A big **drift-wood branch** arrives, **splashes** into
the bars and turns until it leans against them. **Logs, leaves (green, yellow,
brown), white plastic bits and straw** follow and pile up against the bars. A
few small pieces appear on the right side.

**In the project → Scene 4** (`Scene4_TrashRack.cpp`, `objects/Debris.cpp`):
the same layout (soil, slabs, back wall, beam, two blue motors, railing, bar
cage). The water is a box with a wavy top and a see-through front face.
Debris arrives in three waves and is moved by simple physics (current,
buoyancy, bobbing, tumbling). Pieces larger than the bar gap stop and pile up.
The branch splashes and rotates into its leaning pose. Small leaves pass
through. Extra buttons: **THROW MORE TRASH** and **RAKE CLEAN**.

## Shot 5 — 0:40 to 0:50 · Intake gate opens

Cut-away of the intake gate. The **reservoir** is on the left (water with a
see-through cut face). The **steel gate** is in the middle, with horizontal
girders and **hydraulic cylinders** above. A **diamond wire-mesh panel**
(ladder screen) stands beside it. **Dark concrete** blocks are above, a
**tiled floor** is on the right, and the conduit is dry. The gate **rises**;
water **bursts out underneath as a curved glassy sheet**, spreads over the
floor towards the right, and the conduit **fills up**.

**In the project → Scene 5** (`Scene5_IntakeGate.cpp`, `objects/IntakeGate.cpp`):
press **OPEN GATE**. The gate and its piston rods rise together (a hierarchical
transformation). The jet speed comes from Torricelli's law, v = √(2gh). The
water surface in the conduit is a mesh whose heights are recalculated every
frame: a bulge after the gate, a rounded front that advances, and a rising
level. There is spray at the gate lip, and the conduit drains again when you
close the gate.

## Shot 6 — 0:50 to 1:00 · Penstock

A big **steel pipe with its top cut open**, coming out of the intake structure.
It has **ring stiffeners and flanges with bolts** and sits on concrete saddles,
with the downstream face of the dam, walkways and railings behind it. A
**bulging front of blue water** shoots down the pipe. The camera follows it
down the slope. Around 0:56 the pipe becomes a **transparent wire-grid
(X-ray)** look, and the pipe **bends into a large round turbine housing with
radial ribs**.

**In the project → Scene 6** (`Scene6_Penstock.cpp`, `objects/CutawayPipe.cpp`):
a pipe swept along a path with rounded bends, cut open on the camera side,
with full rings, bolts and saddles. The water is a tube with a rounded "nose"
made in the vertex shader. Its speed is √(v₀² + 2gh), so it accelerates down
the slope. A chase camera follows the front and then pulls back to the wide
shot. The X-ray hologram grid and glowing streaks switch on half way. A spiral
casing drum with ribs sits at the bottom.

## Shot 7 — 1:00 to 1:10 · Sci-fi high-speed flow

A dark grey **engineering "hologram" space** with grid lines on the floor and
walls and floating HUD panels. A **glass penstock section** with **steel
flanges**, inside which **glowing cyan-white streams of water** race at high
speed. The floor is **glossy and reflects** the pipe, and the pipe casts
shadows. The camera glides along the pipe. At 1:10 the video cuts to the main
inlet valve (outside our range).

**In the project → Scene 7** (`Scene7_RayTracedFlow.cpp`, `shaders/raytrace.frag`):
this is the **ray-tracing** scene. Every pixel shoots a ray at an exact
mathematical scene (glass/steel cylinder, flanges, supports, floor, wall). It
does real **reflection** (floor, glass, steel), **refraction** (Snell's law
into and out of the water), **shadow rays**, and **volume ray-marching** to
collect the glow of the moving streams. A hologram grid overlay, callouts that
follow the pipe, and interactive **head** control (UP/DOWN) change the flow
speed v = √(2gh) and the power P = ρgQH.

---

## Visual style notes taken from the video

* Concrete: light grey with large formwork panels, faint joints, tie holes,
  rain streaks and darker damp areas near the water.
* Water: deep blue in volume, light cyan in thin sheets, white streaks and
  sparkles where it moves fast, white foam where it lands.
* Steel: medium grey and slightly brushed. The gates and racks are darker
  painted steel.
* Cut-away scenes use soft "studio" lighting on a light background.
* Every shot has a slow, smooth camera move. The scene changes are hard cuts
  or quick fades (here: a short fade to black).
