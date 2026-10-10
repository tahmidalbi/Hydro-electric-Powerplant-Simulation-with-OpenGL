# Video analysis — 0:00 to 3:00

Video: *Paani Se Bijli Kaise Banti Hai? Dam Hydroelectric Power Plant (3D Animation)*,
https://youtu.be/sPXvq6cITFk (3:00 long, 1280×720, 24 fps).

The video was downloaded and cut into frames (2 frames per second for the
whole video). The important moments were then studied at full size. Below is
what happens in every shot, what it looks like, and how this project
recreates it.

**Important:** the first 10 seconds of the video are a *preview* of the
finished, running plant. At 0:10 the video starts over and explains the path
of the water from the beginning. The program follows the same idea, but turns
the rest into **one continuous journey** of the water through one world:

| Program scene | Video time | Station |
|---|---|---|
| 1 Preview | 0:00–0:10 | the running plant (spillway + powerhouse) |
| 2 Reservoir → trash rack | 0:20–0:40 | flight under water to the intake, then the rack filtering trash |
| 3 Intake gate | 0:40–0:50 | the gate opens, the conduit fills |
| 4 Penstock | 0:50–1:00 | water rushes down the pipe to the closed main inlet valve |
| 5 Ray-traced pipe | 1:00–1:10 | sci-fi view inside the penstock |
| 6 Main inlet valve | 1:10–1:20 | the valve opens, the spiral casing fills |
| 7 Francis turbine | 1:20–1:50 | water swirls through the guide vanes, the runner turns |
| 8 Shaft | 1:50–2:00 | hologram rise up the shaft to the generator |
| 9 Generator | 2:00–2:30 | rotor, stator, magnetic field, three-phase current |
| 10 Bus ducts + transformer | 2:30–2:50 | 11 kV is stepped up to 220 kV |
| 11 Switchyard + grid | 2:50–3:00 | breaker closes, the power leaves along the towers |

The dam overview with measurements (0:10–0:20) is not a separate scene. The
flight in scene 2 passes over the same dam on its way to the reservoir.

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

**In the project → Scene 1, Preview** (`src/scenes/Scene1_Preview.cpp`):

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
  drains; **START WATER** releases it again; **SHOW WATER MOVEMENT** starts the
  journey.

## Shot 2 — 0:10 to 0:20 · Dam overview with measurements

The same kind of dam in **bright daylight**, seen from the front-left. The
reservoir is visible on the left, and the gates are **closed** (the chutes are
dry). A thin **viewfinder frame** appears, and white **dimension lines with
labels** are drawn on the left part of the dam (heights and lengths). The
camera slowly pushes in.

**In the project:** not a separate scene any more. The first seconds of
scene 2 fly over this same dam (crest, spillway, crane, reservoir) on the way
to the intake.

## Shot 3 — 0:20 to 0:30 · Under water at the intake

Blue **under-water** view of the dam's upstream face. The rippling water
surface is at the top, **concrete panels** have **caustic light patterns** on
them, and **rock piles** sit in the bottom corners. In the centre is the
intake: a dark recess with a sloping hood and a **steel trash rack** (vertical
bars, top and bottom beams with **bolted brackets**, pointed bar tops, a
lifting lug). The camera moves straight in. The bars get bigger and bigger and
the camera **passes between them** into the tunnel, where a blue opening is
visible at the far end.

**In the project → Scene 2, part A** (`Scene2_ReservoirToRack.cpp`,
`objects/IntakeStructure.cpp`): the camera dives through the reservoir surface
to the intake under the yellow crane. You see under-water fog that absorbs
red light first, caustics, the surface seen from below, light shafts,
floating dust, rocks, and the trash-rack cage in front of the dark intake
mouth.

## Shot 4 — 0:30 to 0:40 · Trash rack filtering (cut-away)

A clean **cut-away diagram**. A concrete channel is sliced open, with **brown
soil** under the slabs. A light grey wall and a railing are at the top right.
A **cage of dark steel bars** stands in the middle, with a heavy concrete beam
and **two blue motors** on top. **Light-blue water** flows from left to right
with a wavy surface. A big **drift-wood branch** arrives, **splashes** into
the bars and turns until it leans against them. **Logs, leaves (green, yellow,
brown), white plastic bits and straw** follow and pile up against the bars. A
few small pieces appear on the right side.

**In the project → Scene 2, part B** (`objects/Debris.cpp`): a dissolve
changes to a cut-away side view of the same cage: the near pier and the
reservoir water beside it are removed, and the water shows a see-through cut
face. The two blue rake motors stand on the deck above. Debris arrives in
three waves and is moved by simple physics (current, buoyancy, bobbing,
tumbling). Pieces larger than the bar gap stop and pile up, and the branch
splashes and turns into its leaning pose. Small leaves pass through and sink
into the intake mouth. Button: **ADD MORE TRASH**.

## Shot 5 — 0:40 to 0:50 · Intake gate opens

Cut-away of the intake gate. The **reservoir** is on the left (water with a
see-through cut face). The **steel gate** is in the middle, with horizontal
girders and **hydraulic cylinders** above. A **diamond wire-mesh panel**
(ladder screen) stands beside it. **Dark concrete** blocks are above, a
**tiled floor** is on the right, and the conduit is dry. The gate **rises**;
water **bursts out underneath as a curved glassy sheet**, spreads over the
floor towards the right, and the conduit **fills up**.

**In the project → Scene 3** (`Scene3_IntakeGate.cpp`, `objects/IntakeGate.cpp`):
the dam section in front of the conduit lifts away. Press **OPEN GATE**: the
cylinders stand on the dam crest, and the gate rises in its shaft inside the dam. The gate and its piston rods rise together (a hierarchical
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

**In the project → Scene 4** (`Scene4_Penstock.cpp`, `objects/CutawayPipe.cpp`):
the same penstock that starts at the conduit and ends at the main inlet valve
inside the preview's powerhouse. It is a pipe swept along a path with rounded bends, cut
open on the camera side, with full rings and bolts. The water is a tube with a
rounded "nose" made in the vertex shader. Its speed is √(v₀² + 2gh), so it
accelerates down the slope. A chase camera follows the front and then pulls
back to the powerhouse. The X-ray hologram grid and glowing streaks switch on
half way. At the bottom the water stops at the closed main inlet valve (in
the video the next shot starts at that valve).

## Shot 7 — 1:00 to 1:10 · Sci-fi high-speed flow

A dark grey **engineering "hologram" space** with grid lines on the floor and
walls and floating HUD panels. A **glass penstock section** with **steel
flanges**, inside which **glowing cyan-white streams of water** race at high
speed. The floor is **glossy and reflects** the pipe, and the pipe casts
shadows. The camera glides along the pipe. At 1:10 the video cuts to the main
inlet valve.

**In the project → Scene 5** (`Scene5_RayTracedPipe.cpp`, `shaders/raytrace.frag`):
the camera moves in close to the penstock and the picture dissolves into the
ray-traced view. This is the **ray-tracing** scene. Every pixel shoots a ray at an exact
mathematical scene (glass/steel cylinder, flanges, supports, floor, wall). It
does real **reflection** (floor, glass, steel), **refraction** (Snell's law
into and out of the water), **shadow rays**, and **volume ray-marching** to
collect the glow of the moving streams. A hologram grid overlay, callouts that
follow the pipe, and interactive **head** control (UP/DOWN) change the flow
speed v = √(2gh) and the power P = ρgQH.

## Shot 8 — 1:10 to 1:20 · Main inlet valve

A cut-away of the **main inlet valve** (labelled "Main Inlet Valve") between
the end of the penstock and the turbine. The valve body is cut open so the
heavy gate can be seen. It rises, and the water pushes on into the **spiral
casing**, which fills.

**In the project → Scene 6** (`Scene6_InletValve.cpp`, `objects/MainInletValve.cpp`):
the picture dissolves from the ray tracer to the valve. **OPEN VALVE** turns
the hand-wheel and lifts the gate (translation). Only when it is more than
30 % open does the water front move on into the casing (`casingFront`).
FOLLOW TO TURBINE stays grey until the casing is full.

## Shot 9 — 1:20 to 1:40 · Spiral casing seen from the top

Top view of the **snail-shaped casing**. The water swirls round it, gets
squeezed through the ring of **stay vanes and guide vanes**, and enters the
**runner** in the middle.

**In the project → Scene 7** (`Scene7_Turbine.cpp`, `objects/TurbineUnit.cpp`):
the casing is open on top. It is a tube whose radius shrinks as it wraps
round, with 12 fixed stay vanes and 24 movable guide vanes. **OPEN GUIDE
VANES** turns all 24 vanes about their pivots (one shared angle), and the
runner (15 curved blades) speeds up with inertia towards 150 rpm. Blue
streamlines show the swirl.

## Shot 10 — 1:40 to 1:50 · Runner cross-section

A **section through the runner**: blue streamlines turn inwards through the
blades and then go **down the draft tube**.

**In the project → Scene 7, SECTION VIEW** (`X`): the near half of the
machine is hidden, and the streamlines continue down and out of the draft
tube (`draftFill`). FOLLOW UP THE SHAFT waits until the rpm is at least 140.

## Shot 11 — 1:50 to 2:00 · Up the shaft

A **blueprint / hologram** shot that rises along the turning **shaft** with
its flanges and bearings, up to the generator.

**In the project → Scene 8** (`Scene8_Shaft.cpp`): an automatic 11-second
camera rise along the real shaft in the powerhouse, past the guide bearing
and the coupling flange, through the generator floor. Glowing cyan
"hologram" scan rings move along the shaft, and a special blue-tinted
lighting gives the blueprint look. The shaft turns with the runner's angle.

## Shot 12 — 2:00 to 2:20 · Generator

A **section of the generator** in a **bright hall** with **yellow railings**
and big windows. You see the **rotor** with its poles, the **stator** with
copper windings, and a **wireframe rotor** that shows how it is built.

**In the project → Scene 9** (`Scene9_Generator.cpp`, `objects/Generator.cpp`):
the generator housing has a 120° window cut into it. The stator has 72 slots
with copper coils and end windings; the rotor has a hub, spider arms, a rim
and 40 poles with field coils. The hall (`Powerhouse.cpp`) has windows,
lamps and yellow railings.

## Shot 13 — 2:20 to 2:30 · Current in the coils

**Orange glowing lines** run through the coils: the induced current.

**In the project → Scene 9, EXCITE ROTOR**: the field current rises; blue
**magnetic field lines** appear around the poles; then **orange three-phase
current** pulses run through the stator coils. An **oscilloscope** shows the
three sine waves 120° apart, growing to 11 kV at f = 40 × 150 / 120 = 50 Hz.
FOLLOW THE POWER waits for 10.5 kV.

## Shot 14 — 2:30 to 2:50 · Bus ducts and step-up transformer

Text "Three-Phase AC" at the generator **terminals**. **Bus ducts** carry the
power through the wall to a large **green step-up transformer** with brown
**bushings**, **radiators** and **cooling fans**.

**In the project → Scene 10** (`Scene10_Transformer.cpp`, `objects/Transformer.cpp`):
the camera flies out over the roof to the yard (one continuous move). Three
bus ducts glow with the current. **X-RAY TRANSFORMER** shows the iron core
with a low-voltage winding (few turns) and a high-voltage winding (many
turns) on each leg: 11 kV → 220 kV, ratio 1 : 20. The fans turn only while
there is voltage.

## Shot 15 — 2:50 to 3:00 · Switchyard and transmission towers

A fly-through of the **switchyard** (breakers, insulators, gantries), then the
**transmission towers** carrying the line to the horizon.

**In the project → Scene 11** (`Scene11_Grid.cpp`, `objects/Switchyard.cpp`,
`objects/TransmissionLine.cpp`): disconnector, **circuit breaker**, current
transformer and surge arrester are labelled. **CLOSE BREAKER** turns its lamp
green, the energy glow runs along the six sagging conductors over 10 lattice
towers, and the camera flies with it. At the end a **summary of the energy
chain** appears (potential → kinetic → mechanical → electrical → grid), with
the plant's numbers. REPLAY JOURNEY fades back to the preview.

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
