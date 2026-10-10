# HYDRO // Dam to Turbine — OpenGL Graphics Project

A real-time 3D recreation of the **whole video** (3:00)
*"Paani Se Bijli Kaise Banti Hai? Dam Hydroelectric Power Plant (3D Animation)"*
(https://youtu.be/sPXvq6cITFk), written in **C++ and OpenGL 3.3 only**.

There are no model files and no image files. Every object is built from code,
every texture is calculated in a shader, and the font is drawn by hand. GLFW
is used only to open the window and read the keyboard and mouse. GLAD only
loads the OpenGL functions. All the maths (vectors, matrices,
transformations) is our own (`src/core/MathLib.h`).

The program is **one continuous world** and follows the real path of the
energy:

```
PREVIEW (the running plant)
  -> reservoir -> trash rack -> intake gate -> penstock -> ray-traced pipe
  -> main inlet valve -> Francis turbine -> shaft -> generator
  -> bus ducts + step-up transformer -> switchyard + circuit breaker -> power line
  -> summary of the whole energy chain
```

The camera flies from place to place. Every step happens only after its
cause: the casing fills only after the valve opens, the runner turns only
when the guide vanes let water through, voltage needs a spinning AND excited
rotor, and the line is live only after the breaker closes.

---

## 1. How to run it

**Easiest way:** double-click `run.bat` in this folder.
It builds the program the first time and then starts it.

From a terminal:

```bat
cd "C:\Users\albit\OneDrive\Desktop\Computer Graphics Project\new"
build.bat            :: compile  ->  build\hydro.exe
run.bat              :: start the program
```

Requirement: MSYS2 MinGW-w64 at `C:\msys64\mingw64` (with GLFW), which is
already installed on this PC.

Extra options:

| command | what it does |
|---|---|
| `build\hydro.exe --size 1920 1080` | different window size |
| `build\hydro.exe --nopost` | start without bloom / film effects (faster on weak PCs) |
| `build\hydro.exe --bench` | prints the real frame time of every scene |
| `build\hydro.exe --shot 1 120 pic.bmp` | starts at scene 1, lets the story run for 120 s on AUTO, saves a picture |
| `build\hydro.exe --autotest 60 pic.bmp` | the real frame loop on AUTO for 60 s, then a picture |

---

## 2. How to use it (the story)

Each scene has its own buttons at the bottom of the screen. Click them, or
press the key shown under the button. A grey button is not ready yet; the
text under it tells you what to do first.

| # | Scene (video time) | What happens | Buttons |
|---|---|---|---|
| 1 | **Preview: the finished plant** (0:00–0:10) | The whole plant running: spillway water, cut-away powerhouse, turbine and generator spinning. | **STOP / START WATER** `[SPACE]`, **SHOW WATER MOVEMENT >** `[ENTER]` |
| 2 | **Into the reservoir → trash rack** (0:20–0:40) | *Automatic:* the plant shuts down while the camera flies over the dam and dives under water to the intake. Then a cut-away of the trash rack: trash stops at the bars, small leaves slip through. | **ADD MORE TRASH** `[T]`, **FOLLOW TO INTAKE GATE >** `[ENTER]` |
| 3 | **Intake gate** (0:40–0:50) | The dam section lifts away and the gate is visible. | **OPEN / CLOSE GATE** `[O]`, **FOLLOW TO PENSTOCK >** (only when the gate is fully open) |
| 4 | **Penstock** (0:50–1:00) | Water rushes down the pipe (v = √(v₀² + 2gh)) and stops at the closed main inlet valve. | **X-RAY VIEW** `[X]`, **REPLAY FLOW** `[SPACE]`, **FOLLOW TO RAY-TRACED PIPE >** |
| 5 | **Ray-traced pipe** (1:00–1:10) | GPU ray-traced glass penstock with glowing water. | **RAISE / LOWER HEAD** `[UP]` `[DOWN]`, **FOLLOW TO MAIN INLET VALVE >** |
| 6 | **Main inlet valve** (1:10–1:20) | The hand-wheel turns, the valve gate rises, and the spiral casing fills. | **OPEN / CLOSE VALVE** `[O]`, **FOLLOW TO TURBINE >** (when the casing is full) |
| 7 | **Francis turbine** (1:20–1:50) | Top view of the water swirling round the casing. The 24 guide vanes open and the runner speeds up to 150 rpm. A section view shows the streamlines through the runner and down the draft tube. | **OPEN / CLOSE GUIDE VANES** `[O]`, **SECTION / TOP VIEW** `[X]`, **FOLLOW UP THE SHAFT >** (when at full speed) |
| 8 | **The shaft** (1:50–2:00) | *Automatic:* a hologram shot rising up the turning shaft to the generator. | — |
| 9 | **Generator** (2:00–2:30) | The rotor poles spin inside the copper stator. Exciting the rotor makes magnetic field lines appear and three-phase current flow. An oscilloscope shows 11 kV at 50 Hz. | **EXCITE ROTOR / SWITCH FIELD OFF** `[O]`, **FOLLOW THE POWER >** (when at full voltage) |
| 10 | **Step-up transformer** (2:30–2:50) | The camera flies out over the roof to the yard. Bus ducts carry 11 kV to the transformer, which raises it to 220 kV. Its fans spin. | **X-RAY TRANSFORMER** `[X]`, **FOLLOW TO SWITCHYARD >** |
| 11 | **To the grid** (2:50–3:00) | Disconnector, circuit breaker, CTs and arresters. Closing the breaker sends the energy along the power line, the camera flies with it, and a summary of the energy chain appears. | **CLOSE / OPEN BREAKER** `[O]`, **REPLAY JOURNEY** `[ENTER]` |

### Keys that work everywhere

| key / mouse | action |
|---|---|
| **SPEED** buttons (top right) or **[ ]** (also **− =**) | slow / fast motion: 0.25× · 0.5× · 1× · 2× |
| left mouse drag | orbit the camera |
| right mouse drag, W A S D Q E | move the camera |
| mouse wheel | zoom |
| **C** | back to the director's camera |
| **G** | shading: MIXED → all PHONG → all GOURAUD |
| **L** | shadows on / off |
| **V** | bloom, vignette and film grain on / off |
| **H** | hide / show the HUD |
| **P** | AUTO: the whole story plays by itself (about 3 minutes) |
| **1 … 9**, **0** | jump straight to scene 1–9, 10 (use N for 11) |
| **N / →**, **B / ←** | next / previous scene |
| **R** | restart the current scene |
| **F12** | screenshot into `captures\` |
| **Esc** | quit |

---

## 3. Where things are

```
new/
├── build.bat, run.bat, CMakeLists.txt
├── shaders/             GLSL: lighting (Phong/Gouraud), water, sky, particles, ray tracer,
│                        flow-line glow, bloom/post-processing, HUD, dissolve
├── src/
│   ├── main.cpp         command line, starts App
│   ├── App.cpp/.h       window, input, continuous scene changes, dissolves, speed control, HUD
│   ├── core/            MathLib (vectors/matrices), Mesh + MeshBuilder, Shader, Camera, Screenshot
│   ├── render/          Renderer (lights, shadow map, materials, water), ParticleSystem,
│   │                    FrameCapture (dissolves), PostFX (bloom, vignette, grain)
│   ├── objects/         ONE FILE PER OBJECT: Dam, IntakeStructure, TrashRack, Debris, IntakeGate,
│   │                    CutawayPipe, Powerhouse, MainInletValve, TurbineUnit, Generator,
│   │                    Transformer, Switchyard, TransmissionLine, FlowLines, SpillwayWater,
│   │                    Terrain, Rocks, PlantData (the plant's numbers),
│   │                    World (puts everything together + the cause-and-effect simulation)
│   ├── scenes/          ONE FILE PER SCENE: Scene1_Preview ... Scene11_Grid
│   └── ui/              hand-made pixel Font, Ui (rectangles / lines / text)
├── docs/
│   ├── VIDEO_ANALYSIS.md   the whole video studied shot by shot (0:00–3:00)
│   └── HOW_IT_WORKS.md     every graphics topic explained, with file names
└── captures/            pictures rendered by the program itself
```
