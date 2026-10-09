# HYDRO // Dam to Turbine — OpenGL Graphics Project

A real-time 3D recreation of the first **1 minute 10 seconds** of the video
*"Paani Se Bijli Kaise Banti Hai? Dam Hydroelectric Power Plant (3D Animation)"*
(https://youtu.be/sPXvq6cITFk), written in **C++ and OpenGL 3.3 only**.

There are no model files and no image files. Every object is built from code,
every texture is calculated in a shader, and the font is drawn by hand. GLFW
is used only to open the window and read the keyboard and mouse. GLAD only
loads the OpenGL functions. It is the same set-up as the Confluence project.
All the maths (vectors, matrices, transformations) is our own
(`src/core/MathLib.h`, no GLM).

---

## 1. How to run it

**Easiest way:** double-click `run.bat` in this folder.
It builds the program the first time and then starts it.

From a terminal:

```bat
build.bat            :: compile  ->  build\hydro.exe
run.bat              :: start the program
```

Requirement: MSYS2 MinGW-w64 at `C:\msys64\mingw64` (with GLFW), which is
already installed on this PC. CMake also works:
`cmake -S . -B build -G "MinGW Makefiles"` and then `mingw32-make -C build`.

Extra options:

| command | what it does |
|---|---|
| `build\hydro.exe --size 1920 1080` | different window size |
| `build\hydro.exe --bench` | prints the real frame time of every scene |
| `build\hydro.exe --shot 4 8 pic.bmp` | saves scene 4 at 8 s to a picture |
| `... --press 1.0 1` (after a `--shot`) | presses button 1 at 1.0 s in that shot |

---

## 2. How to use it (the story)

Every scene has its **own buttons** at the bottom of the screen. Click them,
or press the key shown under the button.

| # | Scene (video time) | What you see | Buttons |
|---|---|---|---|
| 1 | **Spillway release** (0:00–0:10) | Water falls 50 m down three spillway chutes and sprays into the river. The camera follows it down to the cut-away powerhouse, where the turbine and the copper generator rotor spin. | **STOP WATER / START WATER** `[SPACE]`: the gates close and the chutes drain, or the gates reopen. **FOLLOW WATER TO INTAKE >** `[ENTER]` |
| 2 | **Dam survey** (0:10–0:20) | Daylight view of the dam with measuring lines: dam height, head, crest length, spillway. | **DIVE TO THE INTAKE >** `[ENTER]` (moves on by itself after 10 s) |
| 3 | **Intake, under water** (0:20–0:30) | Under the reservoir: light shafts, caustics and floating dust. The camera swims to the trash rack and between its bars into the tunnel. | **SWIM THROUGH THE RACK >** `[ENTER]` (moves on by itself) |
| 4 | **Trash rack filtration** (0:30–0:40) | Cut-away channel. A drift-wood branch, logs, plastic and straw hit the steel bars and stay there. Small leaves slip through. | **THROW MORE TRASH** `[T]`, **RAKE CLEAN** `[K]`, **GO TO INTAKE GATE >** `[ENTER]` |
| 5 | **Intake gate** (0:40–0:50) | Hydraulic cylinders lift the steel gate. Water bursts out underneath, races along the floor and fills the conduit. | **OPEN GATE / CLOSE GATE** `[O]`, then it continues into the penstock by itself |
| 6 | **Penstock** (0:50–1:00) | Water rushes down the cut-away steel pipe and gets faster as it drops (v = √(2gh)). Half way down the pipe turns into an X-ray hologram. | **X-RAY VIEW** `[X]`, **REPLAY FLOW** `[SPACE]`, **HIGH-SPEED VIEW >** `[ENTER]` |
| 7 | **High-speed flow, ray traced** (1:00–1:10) | Sci-fi glass penstock. Glowing water streams race through it. The whole picture is ray traced on the GPU. | **RAISE HEAD +10 M** `[UP]`, **LOWER HEAD −10 M** `[DOWN]`, **RESTART JOURNEY** `[ENTER]` |

### Keys that work everywhere

| key / mouse | action |
|---|---|
| left mouse drag | orbit the camera around the scene |
| right mouse drag, W A S D Q E | move the camera |
| mouse wheel | zoom |
| **C** | back to the director's (cinematic) camera |
| **G** | shading: MIXED → all PHONG → all GOURAUD (compare them!) |
| **L** | shadows on / off |
| **H** | hide / show the HUD |
| **P** | AUTO play: watch the whole film like the video, buttons get pressed for you |
| **1 … 7** | jump to a scene |
| **N / →**, **B / ←** | next / previous scene |
| **R** | restart the current scene |
| **F12** | screenshot into `captures\` |
| **Esc** | quit |

---

## 3. Where things are

```
new/
├── build.bat, run.bat, CMakeLists.txt
├── shaders/             GLSL programs (lighting, water, sky, particles, ray tracer, HUD)
├── src/
│   ├── main.cpp         command line, starts App
│   ├── App.cpp/.h       window, input, scene switching, fades, HUD layout
│   ├── core/            MathLib (vectors/matrices), Mesh + MeshBuilder, Shader, Camera, Screenshot
│   ├── render/          Renderer (lights, shadow map, materials, water), ParticleSystem
│   ├── objects/         ONE FILE PER OBJECT: Dam, Terrain, Powerhouse, Switchyard,
│   │                    SpillwayWater, CutawayPipe, TrashRack, Debris, IntakeGate, Rocks, DamSite
│   ├── scenes/          ONE FILE PER SCENE: Scene1_Spillway ... Scene7_RayTracedFlow
│   └── ui/              hand-made pixel Font, Ui (rectangles / lines / text)
├── docs/
│   ├── VIDEO_ANALYSIS.md   the video studied second by second (0:00–1:10)
│   └── HOW_IT_WORKS.md     every graphics topic explained, with file names
└── captures/            pictures rendered by the program itself
```

Read `docs/HOW_IT_WORKS.md` next. It explains lighting, transformations,
Gouraud vs Phong, animation, interaction and ray tracing in simple English,
and shows where each one is in the code.
