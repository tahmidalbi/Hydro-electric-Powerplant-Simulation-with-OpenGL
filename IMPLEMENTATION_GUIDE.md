# Implementation Guide — "Confluence"
### 3D Hybrid Renewable Energy Power Plant Simulation — from zero to a demoable scene, phase by phase

**Read this alongside `3D_Hybrid_Renewable_Energy_Project_Proposal_Md_Tahmid_Islam...pdf`.** The proposal says *what* and *why*; this document says *how* and *in what order*. Project codename used below: **Confluence** (wind, water, and sun meeting in one scene).

> This guide follows the same phase-by-phase, checkpoint-driven structure used for the `Broadside` naval-gunnery project (`Tawhid.md`) so the two projects are built to the same standard of rigor — a correctly derived Phong illumination model, real hierarchical transforms, tessellation you can prove, and continuous time-driven animation with no pre-baked frames. The *scene* is entirely different (a hydro dam, two wind turbines, a solar array, a reservoir, a boat, and a cloud instead of ships and cannon fire), but the underlying Computer Graphics techniques are the same family, because they're driven by the same course requirements.

---

## How to Use This Guide

**The order is demo-first, not feature-first.** Each phase ends with a **✅ DONE WHEN** checkpoint you should be able to verify visually in under 30 seconds. Don't start phase N+1 until phase N's checkpoint passes — this is exactly how `Broadside` avoided a last-week scramble.

| Milestone | After phase | What you can demo |
|---|---|---|
| 🎯 M1 | Phase 5 | A lit 3D object (e.g. a solar panel) with a visible, moving specular highlight |
| 🎯 M2 | Phase 8 | A reservoir with rolling waves and a boat bobbing on them |
| 🎯 M3 | Phase 13 | **The full hybrid plant running: turbines spinning, water wheel turning, cloud drifting, boat patrolling, and the dam gate opening/closing on command** |
| 🎯 M4 | Phase 14 | The full demonstration suite (shading modes, tessellation, term-mask, lighting) — the grading centrepiece |

---

# PHASE 0 — Toolchain Setup

**Goal:** compile and run an empty OpenGL window. **Time:** half a day — do not underestimate this.

## 0.1 What You Need

| Component | Purpose | Why this one |
|---|---|---|
| **OpenGL 3.3 Core** | The API | Core profile forces the modern shader pipeline — required for per-fragment Phong |
| **GLFW** | Window + input | Simplest cross-platform windowing |
| **GLAD** | Function loader | Loads GL function pointers at runtime |
| **GLM** | Math (vec/mat) | Header-only, mirrors GLSL syntax exactly |
| **CMake** | Build system | Portable, easy to submit |
| **VS Code** | Editor | With two extensions it configures, builds, runs, and debugs without leaving the window |
| **MSVC Build Tools 2022** (Windows) | Compiler | VS Code does not compile anything itself |

> **"OpenGL only" rule:** GLFW/GLAD/GLM are a window, a function loader, and a math header — not a graphics engine. All rendering is your own GL calls and your own GLSL. Do **not** add Unity, Unreal, Ogre, Bullet, or any scene-graph/physics library. This matches what §7 of the proposal already commits to: "C/C++ using OpenGL."

> **Why not `glBegin`/`glEnd` + `glLight`?** The fixed-function pipeline only gives you Gouraud shading at best, and cannot do per-fragment Phong. Writing your own GLSL is how you earn the lighting marks described in §5 of the proposal.

## 0.2 Windows (VS Code) — Recommended

### Step 0 — Check what you already have

```powershell
$vsw = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsw) { & $vsw -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property displayName }
else { "NO MSVC toolchain installed" }

cmake --version
code --list-extensions | Select-String "cmake-tools|cpptools$|shader"
```

You need a non-empty result from the first block, a CMake version, and both `ms-vscode.cpptools` and `ms-vscode.cmake-tools`.

### Step 1 — Install the compiler (no IDE)

```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools --override "--passive --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```

### Step 2 — Install CMake

```powershell
winget install --id Kitware.CMake
```

Close and reopen the terminal, then `cmake --version`.

### Step 3 — Install VS Code + three extensions

```bash
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension slevesque.shader
```

### Step 4 — Get the three libraries

1. **GLFW:** pre-compiled 64-bit Windows binaries from `glfw.org/download`. Unzip to `external/glfw/` — you need `include/` and `lib-vc2022/glfw3.lib`.
2. **GLAD:** `glad.dav1d.de` → Language `C/C++`, Specification `OpenGL`, API gl `Version 3.3`, Profile `Core`, tick **Generate a loader** → Generate. `include/glad/` and `include/KHR/` go to `include/`; `src/glad.c` goes to `src/`.
3. **GLM:** `github.com/g-truc/glm/releases` (header-only, unzip to `external/glm/`).

> Keep GLAD at **gl 3.3 Core, no extensions**. A higher API or an all-extension header can compile fine on your machine and fail on the grader's.

### Step 5 — First configure and build in VS Code

1. `File → Open Folder…` → the `confluence/` folder (containing `CMakeLists.txt`).
2. `Ctrl+Shift+P` → **CMake: Select a Kit** → **`Visual Studio Build Tools 2022 Release - amd64`** (must be amd64, not x86).
3. `Ctrl+Shift+P` → **CMake: Configure**.
4. **Build:** `F7`. **Run:** `Shift+F5`. **Debug:** `F5`.

## 0.3 Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libglm-dev
# GLAD still comes from glad.dav1d.de (same settings as above)
```

## 0.4 `.vscode/` Project Configuration

**`.vscode/extensions.json`**
```json
{ "recommendations": ["ms-vscode.cpptools", "ms-vscode.cmake-tools", "slevesque.shader"] }
```

**`.vscode/settings.json`**
```json
{
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.configureOnOpen": true,
  "C_Cpp.default.cppStandard": "c++17",
  "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
  "files.associations": { "*.vert": "glsl", "*.frag": "glsl" }
}
```

**`.vscode/launch.json`** — the `cwd` line matters because shaders are copied next to the executable:
```json
{
  "version": "0.2.0",
  "configurations": [{
    "name": "Debug Confluence",
    "type": "cppvsdbg",
    "request": "launch",
    "program": "${command:cmake.launchTargetPath}",
    "args": [],
    "stopAtEntry": false,
    "cwd": "${command:cmake.launchTargetDirectory}",
    "environment": []
  }]
}
```

## 0.5 Project Structure

```
confluence/
├── CMakeLists.txt
├── .vscode/
│   ├── extensions.json
│   ├── settings.json
│   └── launch.json
├── docs/
│   ├── 3D_Hybrid_Renewable_Energy_Project_Proposal_...pdf
│   └── IMPLEMENTATION_GUIDE.md
├── include/
│   ├── glad/glad.h
│   └── KHR/khrplatform.h
├── src/
│   ├── glad.c
│   ├── main.cpp
│   ├── Shader.h
│   ├── Mesh.h
│   ├── Camera.h
│   ├── Material.h
│   ├── Wave.h
│   └── (feature headers added per phase: Turbine.h, Dam.h, Boat.h, Cloud.h, Scene.h)
├── shaders/
│   ├── phong.vert
│   └── phong.frag
├── external/
│   ├── glm/
│   └── glfw/                 ← Windows only
└── build/                    ← generated; add to .gitignore
```

## 0.6 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(confluence)
set(CMAKE_CXX_STANDARD 17)

add_executable(confluence
    src/main.cpp
    src/glad.c
)

target_include_directories(confluence PRIVATE
    include
    external/glm
)

find_package(OpenGL REQUIRED)

if(WIN32)
    target_include_directories(confluence PRIVATE external/glfw/include)
    target_link_libraries(confluence
        ${CMAKE_SOURCE_DIR}/external/glfw/lib-vc2022/glfw3.lib
        OpenGL::GL)
else()
    find_package(glfw3 REQUIRED)
    target_link_libraries(confluence glfw OpenGL::GL dl)
endif()

add_custom_command(TARGET confluence POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:confluence>/shaders)
```

**✅ DONE WHEN:** a window opens with a solid coloured background (`glClearColor`) and closes cleanly on ESC.

**Common failures:** *"gladLoadGLLoader failed"* → called a GL function before loading GLAD. *Black screen, no errors* → forgot `glfwSwapBuffers`. *Linker error on glfw3.lib* → 32/64-bit kit mismatch, pick **amd64**.

---

# PHASE 1 — The Render Loop Skeleton

**Goal:** a correctly structured, continuously running loop (this satisfies the proposal §7 requirement: "continuous render loop... time-based animation").

```cpp
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Confluence", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    float lastFrame = 0.0f, timeScale = 1.0f, pausedAt = 0.0f;
    bool paused = false;

    while (!glfwWindowShouldClose(window)) {
        float real = (float)glfwGetTime();
        float dt   = (real - lastFrame) * (paused ? 0.0f : timeScale);
        lastFrame  = real;

        processInput(window, dt);
        static float simTime = 0.0f;
        simTime += dt;
        updateScene(simTime, dt);           // ALL motion computed here, from simTime

        glClearColor(0.55f, 0.75f, 0.92f, 1.0f);   // sky
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderScene(simTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
```

**The golden rule:** every autonomous animated quantity (turbine spin, water waves, cloud drift, water-wheel spin) must be a pure function of `simTime`. Only interactive state — the dam gate's open/closed target, the current `timeScale`, camera orbit angles — may be stored and advanced by `dt`. An array of pre-computed poses indexed by frame number is not allowed.

**✅ DONE WHEN:** the loop runs at a steady framerate, `dt` prints sensible values (~0.016 at 60 FPS), and pausing freezes everything driven by `simTime`.

---

# PHASE 2 — Shader Loading + First Triangle

Write a minimal `Shader` class: read `.vert`/`.frag` files, `glCreateShader`, `glCompileShader`, **check the compile log from day one**, `glCreateProgram`, `glLinkProgram`, helper setters (`setMat4`, `setVec3`, `setFloat`, `setInt`).

**✅ DONE WHEN:** a coloured triangle appears, and a deliberate shader typo prints a readable compile error instead of a silent black screen.

---

# PHASE 3 — Camera + MVP Matrices

```cpp
glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)W/(float)H, 0.1f, 300.0f);
glm::mat4 view       = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0,1,0));
glm::mat4 model      = glm::mat4(1.0f);
```

Implement an **orbit camera**: keep `radius`, `yaw`, `pitch`; compute position with spherical coordinates; mouse-drag → yaw/pitch, scroll → radius. You will want this to inspect the specular highlight on the solar panels and the metallic gate from different angles — specular is view-dependent by definition.

**✅ DONE WHEN:** you can orbit around a placeholder cube in 3D with no stretching and sane near/far clipping.

---

# PHASE 4 — Mesh Library

**Goal:** every primitive the scene needs, generated with **correct normals**, parameterised by tessellation.

```cpp
struct Vertex { glm::vec3 position; glm::vec3 normal; };

class Mesh {
    unsigned int VAO, VBO, EBO;
    int indexCount;
public:
    Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned>& idx);
    void draw() const;
};
```

## 4.1 Meshes to Generate and What They Build

| Function | Used for |
|---|---|
| `makeCube()` | Dam body, gate slab, nacelle housing, boat hull/cabin, solar-panel frame, support legs, ground blocks |
| `makeCylinder(int segments)` | **Tessellation demo object.** Turbine towers, turbine hub, hydro-wheel axle, boat mast, spillway pipe |
| `makeSphere(int stacks, int slices)` | Cloud puffs, sun disc accent, decorative rocks |
| `makeCone(int segments)` | Nacelle nose cone, background hills |
| `makeQuad()` | Solar-panel glass face, blade face |
| `makeGrid(int N)` | **The reservoir** — flat at generation, displaced in the shader (Phase 8); also reused flat (`N=1`) for the ground plane |
| `makeRing(int segments)` | Hydro-wheel paddle mounts; ripple/foam ring at the spillway (optional) |

## 4.2 Normals — Same Rule as Any Lit Scene

For **analytic surfaces**:
- Sphere: `normal = normalize(position)`
- Cylinder side: `normal = normalize(vec3(x, 0, z))`; caps: `(0, ±1, 0)`

For **non-analytic** meshes (cube faces already get flat normals per-face by duplicating vertices; use this only if you build anything from a shared-vertex mesh):

```cpp
// N_v = (Σ N_i) / ||Σ N_i||
void computeSmoothNormals(std::vector<Vertex>& v, const std::vector<unsigned>& idx) {
    for (auto& vert : v) vert.normal = glm::vec3(0.0f);
    for (size_t i = 0; i < idx.size(); i += 3) {
        glm::vec3 a = v[idx[i]].position, b = v[idx[i+1]].position, c = v[idx[i+2]].position;
        glm::vec3 faceN = glm::normalize(glm::cross(b - a, c - a));
        v[idx[i]].normal += faceN; v[idx[i+1]].normal += faceN; v[idx[i+2]].normal += faceN;
    }
    for (auto& vert : v) vert.normal = glm::normalize(vert.normal);
}
```

**Put this function in your report** — it's a direct, citable implementation of the normal-averaging technique from the shading-model lecture.

**✅ DONE WHEN:** a sphere, cylinder, and cube each render as a correct solid silhouette, and changing the segment count on the cylinder visibly changes its polygon count in wireframe (`glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)`).

---

# PHASE 5 — 🎯 MILESTONE 1: The Phong Shader

**Goal:** one lit object with a visible, moving specular highlight. This is the heart of the grade — build it early, build it once, and reuse it for everything (turbines, dam, boat, water, panels).

## 5.1 `phong.vert`

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 uModel, uView, uProjection;
uniform mat3 uNormalMatrix;      // (M^-1)^T — correct under non-uniform scale
uniform int  uShadingMode;       // 0=Flat, 1=Gouraud, 2=Phong

struct Light {
    int   type;            // 0 = directional (sun), 1 = point
    vec3  direction;
    vec3  position;
    vec3  diffuse, specular;
    vec3  attenuation;     // (a0, a1, a2)
    float enabled;
};
uniform Light uLights[2];   // [0] = sun, [1] = accent/floodlight
uniform vec3  uGlobalAmbient;
uniform vec3  uViewPos;
uniform vec3  uKa, uKd, uKs, uEmission;
uniform float uShininess;
uniform int   uUseBlinn;
uniform int   uTermMask;         // bit 0 ambient, 1 diffuse, 2 specular

out vec3 vFragPos;
out vec3 vNormal;
out vec3 vGouraudColor;

vec3 computeLighting(vec3 N, vec3 fragPos);

void main() {
    vFragPos = vec3(uModel * vec4(aPos, 1.0));
    vNormal  = uNormalMatrix * aNormal;

    if (uShadingMode == 1)   // Gouraud: evaluate illumination HERE, at the vertex
        vGouraudColor = computeLighting(normalize(vNormal), vFragPos);

    gl_Position = uProjection * uView * vec4(vFragPos, 1.0);
}
```

## 5.2 `phong.frag` — The Core Illumination Model

```glsl
#version 330 core
in vec3 vFragPos;
in vec3 vNormal;
in vec3 vGouraudColor;
out vec4 FragColor;

// ... identical uniform block as the vertex shader ...

vec3 computeLighting(vec3 N, vec3 fragPos) {
    vec3 V = normalize(uViewPos - fragPos);

    vec3 result = vec3(0.0);
    if ((uTermMask & 1) != 0) result += uKa * uGlobalAmbient;   // ambient
    result += uEmission;

    for (int i = 0; i < 2; ++i) {
        if (uLights[i].enabled < 0.5) continue;

        vec3  L;
        float attenuation = 1.0;

        if (uLights[i].type == 0) {
            L = normalize(-uLights[i].direction);       // sun: parallel rays
        } else {
            vec3 toLight = uLights[i].position - fragPos;
            float d = length(toLight);
            L = toLight / d;
            vec3 a = uLights[i].attenuation;
            attenuation = 1.0 / (a.x + a.y * d + a.z * d * d);
        }

        float NdotL = max(dot(N, L), 0.0);
        if ((uTermMask & 2) != 0)
            result += attenuation * uKd * uLights[i].diffuse * NdotL;   // diffuse (Lambert)

        if (NdotL > 0.0 && (uTermMask & 4) != 0) {
            float spec;
            if (uUseBlinn == 1) {
                vec3 H = normalize(L + V);
                spec = pow(max(dot(N, H), 0.0), uShininess);
            } else {
                vec3 R = reflect(-L, N);
                spec = pow(max(dot(R, V), 0.0), uShininess);
            }
            result += attenuation * uKs * uLights[i].specular * spec;    // specular
        }
    }
    return result;
}

void main() {
    vec3 color;
    if (uShadingMode == 0) {
        vec3 faceN = normalize(cross(dFdx(vFragPos), dFdy(vFragPos)));  // Flat: per-face normal
        color = computeLighting(faceN, vFragPos);
    } else if (uShadingMode == 1) {
        color = vGouraudColor;                                          // Gouraud: interpolated colour
    } else {
        color = computeLighting(normalize(vNormal), vFragPos);          // Phong: interpolated normal
    }
    FragColor = vec4(color, 1.0);
}
```

> GLSL has no `#include`. Keep `computeLighting` and the uniform block as a C++ string constant and prepend it to both shader sources at load time — one copy of the lighting math, used everywhere.

## 5.3 The Normal Matrix

```cpp
glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
```
Compute this **on the CPU, once per object per frame**. You'll need it constantly — the turbine towers, the dam body, and the boat hull are all non-uniformly scaled cubes/cylinders.

**✅ DONE WHEN 🎯 M1:** a single solar-panel-glass-material sphere under the sun light shows a clear lit side, a dark side, and a **specular highlight that moves as you orbit the camera**. Pressing `1`/`2`/`3` gives three visibly different results.

**If the highlight doesn't move when you orbit**, you forgot to update `uViewPos` per frame.

---

# PHASE 6 — Materials and the Second Light

**Goal:** give every scene object a distinct, physically-plausible material, and light the scene with the sun plus one accent light.

```cpp
struct Material { glm::vec3 ka, kd, ks, emission; float shininess; };

// Concrete dam body — dull, low specular
const Material CONCRETE = {{0.14f,0.14f,0.13f},{0.55f,0.54f,0.50f},{0.06f,0.06f,0.06f},{0,0,0}, 6.0f};

// Metal gate / turbine hub / nacelle housing — sharp cool highlight
const Material STEEL = {{0.20f,0.20f,0.22f},{0.35f,0.36f,0.38f},{0.85f,0.86f,0.88f},{0,0,0}, 90.0f};

// Turbine tower / crane — warm brass-like accent (verbatim classic material values)
const Material BRASS = {{0.329412f,0.223529f,0.027451f},{0.780392f,0.568627f,0.113725f},
                         {0.992157f,0.941176f,0.807843f},{0,0,0}, 27.8974f};

// Solar panel glass — near-black, very tight, bright highlight
const Material PANEL_GLASS = {{0.02f,0.03f,0.05f},{0.05f,0.08f,0.14f},{0.85f,0.88f,0.95f},{0,0,0}, 140.0f};

// Turbine blades — matte white plastic-like
const Material BLADE_WHITE = {{0.18f,0.18f,0.18f},{0.85f,0.85f,0.85f},{0.10f,0.10f,0.10f},{0,0,0}, 10.0f};

// Boat hull — dull painted wood
const Material HULL_WOOD = {{0.12f,0.08f,0.05f},{0.42f,0.27f,0.16f},{0.10f,0.09f,0.08f},{0,0,0}, 8.0f};

// Reservoir water — low diffuse, very high, tight specular (the sun glitter)
const Material WATER = {{0.02f,0.05f,0.08f},{0.06f,0.14f,0.20f},{0.90f,0.94f,0.98f},{0,0,0}, 160.0f};

// Cloud — near-pure ambient/diffuse white, negligible specular
const Material CLOUD = {{0.30f,0.30f,0.30f},{0.95f,0.95f,0.97f},{0.05f,0.05f,0.05f},{0,0,0}, 4.0f};

// Ground/grass
const Material GRASS = {{0.05f,0.10f,0.04f},{0.22f,0.45f,0.16f},{0.02f,0.02f,0.02f},{0,0,0}, 4.0f};
```

Set up two lights (PRD §5 asks specifically for a directional sun light; the second light is what lets you demonstrate multi-light summation and attenuation, which the course's illumination-model slides require):

```cpp
Light sun;
sun.type = 0;                       // directional
sun.direction = glm::normalize(glm::vec3(-0.4f, -1.0f, -0.3f));
sun.diffuse = {1.00f, 0.95f, 0.85f};
sun.specular = {1.0f, 1.0f, 1.0f};
sun.enabled = 1.0f;

Light beacon;                       // accent point light: dam floodlight / turbine warning lamp
beacon.type = 1;
beacon.position = damGateLightWorldPos;
beacon.diffuse = {0.9f, 0.95f, 1.0f};
beacon.specular = {1.0f, 1.0f, 1.0f};
beacon.attenuation = {1.0f, 0.09f, 0.032f};
beacon.enabled = 1.0f;
```

Hoist view/projection/light uniforms **outside** the per-object draw loop; only material uniforms change per object.

**✅ DONE WHEN:** a row of test spheres — Steel / Brass / Panel Glass / Blade White / Water / Cloud — under the same light looks obviously different: steel has a tight cool highlight, brass a broad warm one, panel glass an almost mirror-sharp one, cloud almost none. Screenshot this row for your report as the material comparison.

---

# PHASE 7 — Static Scene Hierarchy (No Animation Yet)

**Goal:** place every object correctly and verify the parent-child chains before adding any motion.

Build one function per structure, each returning/using an unscaled **frame matrix** so children never inherit a parent's non-uniform scale (the single most common hierarchy bug — scale only inside the final `drawMesh` call).

## 7.1 Wind Turbine — Tower → Nacelle → Hub → 3 Blades

```cpp
void drawTurbine(const glm::mat4& turbineRoot, float spinAngle) {
    // Level 1: tower
    glm::mat4 tower = turbineRoot;
    drawMesh(cylinderMesh, tower * glm::scale(glm::mat4(1), {0.25f, 0.25f, 6.0f})
                                  * glm::rotate(glm::mat4(1), glm::radians(-90.0f), {1,0,0}),
             STEEL);   // cylinder built along local Z; stand it upright

    // Level 2: nacelle (child of the tower top, not of the tower's scale)
    glm::mat4 nacelle = tower * glm::translate(glm::mat4(1), {0, 6.0f, 0});
    drawMesh(cubeMesh, nacelle * glm::scale(glm::mat4(1), {0.5f, 0.5f, 1.2f}), STEEL);

    // Level 3: hub (front of the nacelle)
    glm::mat4 hub = nacelle * glm::translate(glm::mat4(1), {0, 0, 0.7f});
    drawMesh(cylinderMesh, hub * glm::scale(glm::mat4(1), {0.22f, 0.22f, 0.3f}), BRASS);

    // Level 4: three blades, 120° apart, all sharing the SAME spin angle — the hierarchy demo
    for (int i = 0; i < 3; ++i) {
        float blade0 = glm::radians(i * 120.0f);
        glm::mat4 blade = hub
                         * glm::rotate(glm::mat4(1), spinAngle + blade0, glm::vec3(0,0,1))
                         * glm::translate(glm::mat4(1), {0, 1.6f, 0});
        drawMesh(cubeMesh, blade * glm::scale(glm::mat4(1), {0.14f, 3.2f, 0.05f}), BLADE_WHITE);
    }
}
```

## 7.2 Hydro Water Wheel — Axle → N Paddles

```cpp
void drawWaterWheel(const glm::mat4& wheelRoot, float spinAngle) {
    glm::mat4 axle = wheelRoot * glm::rotate(glm::mat4(1), glm::radians(90.0f), {0,1,0});
    drawMesh(cylinderMesh, axle * glm::scale(glm::mat4(1), {0.15f,0.15f,1.2f}), STEEL);

    const int PADDLES = 8;
    for (int i = 0; i < PADDLES; ++i) {
        float a = spinAngle + glm::radians(i * 360.0f / PADDLES);
        glm::mat4 paddle = wheelRoot
                          * glm::rotate(glm::mat4(1), a, glm::vec3(0,0,1))
                          * glm::translate(glm::mat4(1), {0, 1.0f, 0});
        drawMesh(cubeMesh, paddle * glm::scale(glm::mat4(1), {0.9f, 0.35f, 0.08f}), STEEL);
    }
}
```

## 7.3 Dam — Body → Gate Track → Gate (Static Position First)

```cpp
void drawDam(const glm::mat4& damRoot, float gateHeight /* 0 = closed, 1 = open */) {
    drawMesh(cubeMesh, damRoot * glm::scale(glm::mat4(1), {2.0f, 5.0f, 10.0f}), CONCRETE);

    glm::mat4 gateSlot = damRoot * glm::translate(glm::mat4(1), {1.05f, -1.5f, 3.0f});
    glm::mat4 gate = gateSlot * glm::translate(glm::mat4(1), {0, gateHeight * 2.2f, 0});
    drawMesh(cubeMesh, gate * glm::scale(glm::mat4(1), {0.15f, 1.6f, 1.2f}), STEEL);
}
```

## 7.4 Boat, Cloud, Solar Array — Same Pattern

```cpp
void drawBoat(const glm::mat4& boatRoot) {
    drawMesh(cubeMesh, boatRoot * glm::scale(glm::mat4(1), {0.8f, 0.35f, 2.0f}), HULL_WOOD);
    glm::mat4 cabin = boatRoot * glm::translate(glm::mat4(1), {0, 0.35f, -0.3f});
    drawMesh(cubeMesh, cabin * glm::scale(glm::mat4(1), {0.5f, 0.3f, 0.6f}), HULL_WOOD);
}

void drawCloud(const glm::mat4& cloudRoot) {
    static const glm::vec3 puffs[5] = {{0,0,0},{0.6f,0.1f,0},{-0.6f,0.1f,0},{0.2f,0.3f,0.3f},{-0.2f,0.25f,-0.2f}};
    for (auto& p : puffs)
        drawMesh(sphereMesh, cloudRoot * glm::translate(glm::mat4(1), p)
                                        * glm::scale(glm::mat4(1), glm::vec3(0.7f)), CLOUD);
}

void drawSolarPanel(const glm::mat4& panelRoot) {
    glm::mat4 tilt = panelRoot * glm::rotate(glm::mat4(1), glm::radians(25.0f), {1,0,0});
    drawMesh(cubeMesh, tilt * glm::scale(glm::mat4(1), {1.6f, 0.05f, 1.0f}), STEEL);   // frame
    glm::mat4 face = tilt * glm::translate(glm::mat4(1), {0, 0.03f, 0});
    drawMesh(quadMesh, face * glm::rotate(glm::mat4(1), glm::radians(-90.0f), {1,0,0})
                             * glm::scale(glm::mat4(1), {1.5f, 0.9f, 1.0f}), PANEL_GLASS);
}
```

**✅ DONE WHEN:** the whole plant is visible and correctly proportioned in one static frame — dam with a gate sitting in its track, two turbines with three blades each at fixed angles, a solar array, a boat on the (flat, not-yet-animated) reservoir, a hydro wheel at the outlet, and a cloud. Temporarily hardcoding `turbineRoot = rotate(45°) * turbineRoot` should visibly carry the whole turbine — nacelle, hub, and all three blades — with it.

---

# PHASE 8 — 🎯 MILESTONE 2: Reservoir Waves + Boat Bobbing

**Goal:** a living reservoir surface, and a boat that responds to it.

## 8.1 Wave Displacement in the Vertex Shader (GPU-side)

Add to `phong.vert`:

```glsl
uniform int   uIsWater;
uniform float uTime;
const float A1 = 0.14, K1 = 0.6, W1 = 1.0;
const float A2 = 0.09, K2 = 0.9, W2 = 1.6;

// ... inside main(), before computing vFragPos:
vec3 pos = aPos;
vec3 nrm = aNormal;
if (uIsWater == 1) {
    pos.y = A1 * sin(K1 * pos.x + W1 * uTime)
          + A2 * sin(K2 * pos.z + W2 * uTime + 1.3);
    float dydx = A1 * K1 * cos(K1 * pos.x + W1 * uTime);
    float dydz = A2 * K2 * cos(K2 * pos.z + W2 * uTime + 1.3);
    nrm = normalize(vec3(-dydx, 1.0, -dydz));
}
```

**Debug tip:** temporarily output `FragColor = vec4(N * 0.5 + 0.5, 1.0)` — a correct wave surface shows smooth colour bands rolling across it. A flat single colour means the normal isn't being recomputed.

## 8.2 Matching CPU Function for Boat Bobbing

```cpp
// MUST match the GLSL constants exactly
float waveHeight(float x, float z, float t) {
    return 0.14f*sinf(0.6f*x + 1.0f*t) + 0.09f*sinf(0.9f*z + 1.6f*t + 1.3f);
}
float waveSlopeX(float x, float t) { return 0.14f*0.6f*cosf(0.6f*x + 1.0f*t); }
float waveSlopeZ(float z, float t) { return 0.09f*0.9f*cosf(0.9f*z + 1.6f*t + 1.3f); }

glm::mat4 boatMatrix(glm::vec3 pos, float heading, float t) {
    float y     = waveHeight(pos.x, pos.z, t);
    float roll  = atanf(waveSlopeX(pos.x, t)) * 0.6f;
    float pitch = atanf(waveSlopeZ(pos.z, t)) * 0.6f;

    glm::mat4 M = glm::translate(glm::mat4(1), {pos.x, y, pos.z});
    M = glm::rotate(M, heading, glm::vec3(0,1,0));
    M = glm::rotate(M, roll,  glm::vec3(0,0,1));
    M = glm::rotate(M, pitch, glm::vec3(1,0,0));
    return M;
}
```

**✅ DONE WHEN 🎯 M2:** the reservoir ripples, the boat rides it convincingly, and the sun-streak specular glitters on the moving water. Good moment to show early progress.

---

# PHASE 9 — Continuous Rotation: Turbine Blades + Hydro Water Wheel

**Goal:** the two purely rotational animations from the proposal's §3 table, driven only by `simTime`.

```cpp
float turbine1Spin = simTime * TURBINE1_RATE;             // radians/sec, e.g. 2.4
float turbine2Spin = simTime * TURBINE2_RATE + PI * 0.3f;  // different rate/phase — not lockstep
float wheelSpin     = simTime * WHEEL_RATE;

drawTurbine(turbine1Root, turbine1Spin);
drawTurbine(turbine2Root, turbine2Spin);
drawWaterWheel(wheelRoot, wheelSpin);
```

Give the two turbines **different rates or phases** — visually proves they're independent scene objects (matches the proposal's "A second turbine rotates as an independent scene object").

**✅ DONE WHEN:** both turbines spin continuously and independently, and the hydro wheel spins beside the outlet, all three purely as a function of `simTime` (freeze one via pause and the others keep going only if you deliberately test that; all three freeze together on `P`, but temporarily zeroing one rate proves independence).

---

# PHASE 10 — Cloud Drift

**Goal:** translation with a clean loop, no visible pop.

```cpp
glm::vec3 cloudPos(float t) {
    float range = 24.0f;
    float x = fmodf(t * CLOUD_SPEED, range) - range * 0.5f;   // wraps, then re-centers
    return glm::vec3(x, SKY_HEIGHT, -6.0f);
}
```

Because `makeCloud` is a soft blob of spheres, wrapping `x` linearly is invisible as long as the cloud fully leaves the frustum before it re-enters — verify this at your default camera framing.

**✅ DONE WHEN:** the cloud drifts steadily across the sky and loops back with no visible jump cut.

---

# PHASE 11 — Boat Patrol (Translation + Rotation)

**Goal:** the boat actually travels across the reservoir and can change heading, as required by the proposal's §3 table ("Translation + rotation... can slightly change direction").

```cpp
glm::vec3 boatPathPos(float t) {
    float r = 6.0f;
    return glm::vec3(sinf(0.2f * t) * r, 0.0f, cosf(0.2f * t) * r * 0.6f);   // gentle loop on the reservoir
}
float boatHeading(float t) {
    glm::vec3 p0 = boatPathPos(t), p1 = boatPathPos(t + 0.05f);
    glm::vec2 d = glm::vec2(p1.x - p0.x, p1.z - p0.z);
    return atan2f(d.x, d.y);   // face direction of travel
}
```

Combine with Phase 8's `boatMatrix`: sample `waveHeight`/slopes at the boat's *current* path position each frame so the boat bobs correctly wherever it currently is on the reservoir — this is the same "hierarchy feeds the physics" idea as a ship sampling the ocean under it.

**✅ DONE WHEN:** the boat travels a visible loop across the reservoir, turns to face its direction of travel, and keeps bobbing correctly throughout.

---

# PHASE 12 — Composite Transformations: Solar Array + Supporting Structures

**Goal:** demonstrate translation + rotation + scaling composed correctly, satisfying the proposal's §4 "Transformation composition" and "Scaling" bullets outside of the already-animated objects.

Arrange a grid of solar panels (reuse `drawSolarPanel` from Phase 7) with per-instance translation only — no animation needed here, but the construction itself is the scaling/composition demo:

```cpp
void drawSolarArray(const glm::mat4& arrayRoot) {
    const int ROWS = 2, COLS = 3;
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c) {
            glm::vec3 offset(c * 2.0f - (COLS-1), 0.0f, r * 1.6f);
            drawSolarPanel(arrayRoot * glm::translate(glm::mat4(1), offset));
        }
}
```

Add support legs (thin scaled cylinders under each panel) and ground/backdrop hills (scaled cones/spheres) the same way — every one of these is a translate → rotate → scale composition on a shared unit mesh, which is exactly what the proposal's §4 lists as a required concept.

**✅ DONE WHEN:** the solar array reads clearly as a grid of tilted panels with visible support structure, built entirely from the Phase 4 primitives with no new mesh types.

---

# PHASE 13 — 🎯 MILESTONE 3: Interactive Dam Gate + Water Outlet

**Goal:** the one genuinely interactive mechanism in the scene — the dam gate opens and closes on a keypress, smoothly, and the water outlet visibly reacts. This is the project's centrepiece, playing the same role `Broadside`'s cannon fire does.

## 13.1 Gate State Machine (Interactive State — Allowed to Persist)

```cpp
enum class GateState { Closed, Opening, Open, Closing };
GateState gateState = GateState::Closed;
float     gateProgress = 0.0f;     // 0 = fully closed, 1 = fully open
float     gateEventTime = 0.0f;

void toggleGate(float now) {
    if (gateState == GateState::Closed || gateState == GateState::Closing) gateState = GateState::Opening;
    else                                                                    gateState = GateState::Closing;
    gateEventTime = now;
}

void updateGate(float now) {
    const float DURATION = 2.5f;
    float tau = glm::clamp((now - gateEventTime) / DURATION, 0.0f, 1.0f);
    float eased = tau * tau * (3.0f - 2.0f * tau);      // smoothstep — reads as mechanical, not linear

    if (gateState == GateState::Opening) { gateProgress = eased;       if (tau >= 1.0f) gateState = GateState::Open; }
    if (gateState == GateState::Closing) { gateProgress = 1.0f - eased; if (tau >= 1.0f) gateState = GateState::Closed; }
}
```

This is exactly the kind of stored state Phase 1's golden rule allows: `gateProgress` at any instant is a deterministic function of `now - gateEventTime`, not a pre-baked frame table — re-querying the same `now` always reproduces the same gate height.

## 13.2 Water Outlet Reacting to the Gate

Reuse the Phase 8 wave shader on a second, smaller `makeGrid` quad positioned at the spillway, but only render it (or only give it visible flow speed) proportionally to `gateProgress`:

```cpp
void drawSpillway(const glm::mat4& outletRoot, float t, float gateProgress) {
    if (gateProgress < 0.01f) return;                       // gate closed: no flow to show
    float flowSpeed = 1.0f + gateProgress * 3.0f;            // faster visual flow the more open the gate is
    drawWaterQuad(outletRoot, t * flowSpeed, WATER);         // same wave-displaced material as the reservoir
}
```

## 13.3 Wire the Key

```cpp
// 'G' toggles gate open/closed
if (keyPressedThisFrame(GLFW_KEY_G)) toggleGate(simTime);
```

**✅ DONE WHEN 🎯 M3:** pressing `G` smoothly raises the gate over ~2.5 seconds, the spillway visibly starts flowing once it's open, pressing `G` again smoothly closes it and the flow stops, and by this point every animated object from the proposal's §3 table — both turbines, the cloud, the dam gate, the boat, the hydro wheel — is running simultaneously in one scene.

---

# PHASE 14 — 🎯 MILESTONE 4: Demonstration Suite

**Goal:** the controls that turn a nice scene into a high-scoring *Computer Graphics* project by making every technique provable on demand, live, without touching code.

## 14.1 Runtime Tessellation Control

```cpp
int bladeSegments = 6;     // blade cross-section / turbine cylinders
int reservoirRes   = 48;

void rebuildMeshes() {                  // called ONLY on keypress
    cylinderMesh  = makeCylinder(bladeSegments);
    sphereMesh    = makeSphere(bladeSegments/2, bladeSegments);
    reservoirMesh = makeGrid(reservoirRes);
}
// '+' : bladeSegments = min(64, bladeSegments*2); reservoirRes = min(128, reservoirRes*2); rebuildMeshes();
// '-' : bladeSegments = max(6,  bladeSegments/2);  reservoirRes = max(8,   reservoirRes/2);  rebuildMeshes();
```

## 14.2 Key Bindings

| Key | Effect |
|---|---|
| `1`/`2`/`3` | Shading mode: Flat / Gouraud / Phong |
| `B` | Toggle Blinn-Phong vs. classic Phong specular |
| `L` | Cycle lights: sun only → sun + beacon → beacon only |
| `K` | Cycle term mask: ambient → ambient+diffuse → all → specular-only |
| `+` / `-` | Tessellation: turbine/hydro-wheel cylinder segments and reservoir grid resolution |
| `G` | Toggle dam gate open/close (Phase 13) |
| `H` | Freeze turbine + water-wheel rotation (hierarchy-off demo — everything else keeps moving) |
| `P` | Pause the whole scene |
| `[` / `]` | Global animation speed (`timeScale`) down/up |
| mouse drag / wheel | Camera orbit / zoom |

**The `K` term mask is worth special attention** — cycling it live recreates the ambient/diffuse/specular/final breakdown from the illumination-model lecture, directly on your own scene. Strong demo moment.

```cpp
// K cycles: 1 (ambient) → 3 (ambient+diffuse) → 7 (all) → 4 (specular only) → 1
```

## 14.3 Verify the Two Money Shots

**Demo A — Gouraud misses the water highlight.** Press `-` until `reservoirRes` is 8–16. Orbit until the sun-streak on the reservoir is prominent. Press `2` (Gouraud) — the streak should break up or vanish. Press `3` (Phong) — it returns. *If it doesn't vanish under Gouraud, your water `shininess` is too low or your grid is too fine.*

**Demo B — Mach banding on the turbine tower.** Set cylinder segments to 6–8, press `1` (Flat). Photograph the faceting and the perceived bright/dark bands at the edges — cite the shading-model lecture's explanation of why Flat produces this and Phong doesn't.

**✅ DONE WHEN 🎯 M4:** you can perform a full demo script — orbit, toggle all three shading modes, cycle the term mask, toggle Blinn, change tessellation live, open and close the gate, freeze the hierarchy with `H`, and pause/resume — without touching the code, and never break 20 draw calls (Phase 16).

---

# PHASE 15 — HUD / On-Screen Indications

**Goal:** make internal state readable without opening the code. The window title is enough — it avoids pulling in a font/texture subsystem.

Report: shading mode, Blinn on/off, active light(s), term mask, tessellation level, gate state (`CLOSED`/`OPENING xx%`/`OPEN`/`CLOSING xx%`), `timeScale`, paused y/n, draw calls, FPS.

```cpp
std::string title = "Confluence | Shading: " + shadingName(mode)
    + " | Lights: " + lightName(activeLights)
    + " | Terms: " + termMaskName(mask)
    + " | Gate: " + gateStateName(gateState) + " " + std::to_string((int)(gateProgress*100)) + "%"
    + " | Speed: " + std::to_string(timeScale) + "x"
    + (paused ? " | PAUSED" : "")
    + " | Draws: " + std::to_string(drawCallCount)
    + " | FPS: " + std::to_string((int)fps);
glfwSetWindowTitle(window, title.c_str());
```

**✅ DONE WHEN:** a grader can identify shading mode, lighting state, tessellation, and gate state without asking.

---

# PHASE 16 — Optimization Pass

Checklist:

- [ ] Document every unique mesh and every object that reuses it (both turbines, all solar panels, all hydro-wheel paddles share meshes).
- [ ] Record idle vs. gate-open (spillway flowing) draw/triangle counts.
- [ ] Confirm `GL_CULL_FACE` and `GL_DEPTH_TEST` remain enabled.
- [ ] Set view/projection/light uniforms once per frame, not per object; only material + model uniforms change per draw.
- [ ] Rebuild meshes only on `+`/`-` keypresses, never per frame.
- [ ] Confirm no `new`/`malloc` or growing container inside the render loop.
- [ ] Measure average FPS at 1280×720 with everything running (both turbines, wheel, cloud, boat, gate open, spillway flowing).

**✅ DONE WHEN:** the scene sustains at least 60 FPS on the demo machine with every animated object running simultaneously, and the report contains the measured draw-call/triangle/FPS numbers.

---

# PHASE 17 — Report and Viva Preparation

The final report should contain:

1. A requirement traceability matrix against the proposal's §3/§4/§6/§8.
2. The illumination equation and the material table (§6 of this guide), with the same structure used in the course's illumination-model lecture.
3. Flat/Gouraud/Phong and Mach-banding evidence (Phase 14.3, Demo B), plus the reservoir Gouraud-vs-Phong specular evidence (Demo A).
4. A hierarchy diagram for the wind turbine (tower → nacelle → hub → 3 blades) and the hydro wheel (axle → 8 paddles).
5. A screenshot of the boat mid-loop, bobbing on the wave-displaced reservoir, with its heading visibly following its path.
6. Three screenshots of the dam gate: closed, mid-open (with `gateProgress`), fully open with the spillway flowing.
7. The normal-averaging formula (§4.2) and the normal-matrix line (§5.3), each with one sentence explaining why they're needed.
8. Final draw-call, triangle, and FPS measurements from Phase 16.
9. One paragraph on local vs. global illumination (why there are no shadows — this is a local Phong model, same as any course project built this way).

## Likely Viva Questions

| Question | Your answer |
|---|---|
| "Why no shadows?" | This is a local illumination model (ambient/diffuse/specular per fragment). Shadows require a separate global technique not required by the proposal. |
| "Is the animation pre-computed?" | No frame/keyframe table exists. Turbine spin, wheel spin, cloud drift, and wave height are all pure functions of `simTime`. Only the gate's open/close *event time* is stored — its height is still computed fresh every frame. |
| "Show translation, rotation, and scaling." | Move the cloud (translation), spin a turbine blade (rotation), and point at the solar-panel frame construction, which scales a unit cube/quad to size. |
| "Show hierarchical transforms." | Rotate a turbine root and show the nacelle, hub, and all three blades follow automatically, still spinning correctly relative to the hub. |
| "Why does one turbine function build two turbines?" | Geometry and hierarchy are shared (`drawTurbine`); each call supplies only its root transform and spin phase/rate. |
| "Why is Phong slower than Gouraud?" | Lighting is evaluated per fragment instead of per vertex — provable with the reservoir Gouraud/Phong specular demo. |
| "Why does Gouraud miss the water highlight?" | A highlight that falls between vertices is never sampled before colour interpolation smears it away. |
| "What is the normal matrix for?" | `(M^-1)^T` keeps normals correct under the non-uniform scaling used on the towers, gate, and hulls. |
| "How is the gate animation not a hack?" | It's a smoothstep-eased function of elapsed time since the last toggle event, re-evaluated every frame — not a stored sequence of positions. |

---

# Suggested Schedule

| Block | Phases | Outcome |
|---|---|---|
| **Block 1** | 0–3 | Window, shaders, camera working |
| **Block 2** | 4–6 | 🎯 **M1** — lit objects with specular; full material set done |
| **Block 3** | 7 | Whole plant placed correctly, statically |
| **Block 4** | 8 | 🎯 **M2** — animated reservoir + bobbing boat *(good early demo)* |
| **Block 5** | 9–12 | Turbines, water wheel, cloud, boat patrol, solar array composition |
| **Block 6** | 13 | 🎯 **M3** — interactive dam gate + spillway (the core mechanism) |
| **Block 7** | 14 | 🎯 **M4** — demonstration suite |
| **Block 8** | 15–17 | HUD, optimization measurements, report, and viva rehearsal |

---

# Emergency Descope Plan

If you fall behind, simplify in this order. The proposal's §8 "at least six independently animated objects" cannot be removed:

1. Spillway flow visuals, ripple ring at the outlet, decorative hills/rocks — cosmetic
2. Blinn-Phong toggle (`B`) and light-cycle (`L`) — bonus demo polish
3. Boat path complexity — fall back to a straight back-and-forth translation instead of a curved loop
4. Solar-array grid size — reduce rows/columns, keep the composition technique
5. Reduce tessellation range exposed by `+`/`-` rather than removing the control entirely

**Never cut:** the two independently spinning wind turbines, the hydro water wheel, the moving cloud, the translating+rotating boat, the interactive dam gate, the animated reservoir, the three shading modes, the Phong illumination model with ambient/diffuse/specular, the hierarchical transform on the turbine, or the material variety.
