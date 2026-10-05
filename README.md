# SmartFlow — C++ Traffic and Graphics Lab

**A computer graphics group project: one intersection, moving vehicles, and mathematics you can explore.**

SmartFlow is a native **C++17 desktop application** for studying traffic movement and computer graphics. It contains an animated intersection, an interactive graphics laboratory, and a project brief with the five current group members. Users can change traffic demand and signal timing, inspect vehicles, and experiment with three graphics algorithms.

Our idea began with a traffic-system simulation: turn a familiar road junction into a practical way to explain computer graphics. The first prototype used SVG and JavaScript. Following the lecturer's C++ requirement, the project was implemented separately in C++ with raylib for the window, input and drawing. This repository contains that native version. The original browser project remains preserved separately; the C++ application does not depend on it.

![SmartFlow C++ application showing the traffic intersection, controls and live statistics](docs/images/smartflow-cpp.png)

## What the project demonstrates

| Area | What you can see or do |
| --- | --- |
| Traffic animation | Watch left-hand traffic follow twelve routes: straight, left and right from four approaches. |
| Signal control | Observe green, amber and all-red clearance phases, with vehicles stopping and queuing. |
| Interactive simulation | Start, pause, reset, advance one second, change demand and adjust green duration. |
| Vehicle inspection | Select a car to see its route, position, rotation, speed and stopped time. |
| Live statistics | Follow active vehicles, queues, offscreen demand, completed trips and average completed-trip wait. |
| Graphics transformations | See translation and rotation in vehicles, plus scaling and viewport clipping in the scene. |
| Graphics algorithms | Experiment with Bresenham lines, midpoint circles and Cohen–Sutherland line clipping. |

## Open the application

Download the runnable Windows project ZIP from [Releases](https://github.com/ElijahNjugi/smartflow-cpp/releases), then extract it completely. Double-click **Open SmartFlow.cmd**, or open **bin/SmartFlow.exe** directly. The application starts paused so you can explain the scene before starting traffic.

If you cloned the repository or used GitHub's **Download ZIP**, build first using the instructions below. A source checkout does not contain `bin/`, because generated executables are excluded from Git. If the launcher says the executable is missing, run **Build SmartFlow.cmd** after setting up the required compiler.

Running the built application does not require Node.js, npm, a browser, an internet connection, or a C++ compiler. It does need a compatible Windows computer and graphics driver; this build uses the OpenGL 3.3 rendering backend.

## Build from source

Repository collaborators can clone the project with Git, then enter its folder:

```powershell
git clone https://github.com/ElijahNjugi/smartflow-cpp.git
cd smartflow-cpp
```

Double-click **Build SmartFlow.cmd**. It compiles and runs the model and graphics tests, builds raylib if needed, and creates `bin/SmartFlow.exe`.

Alternatively, open PowerShell in this folder and run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
```

You need an **x64 MinGW-w64 toolchain** supporting C++17. The script looks for `g++.exe` on `PATH`, then checks the common MSYS2 location `C:/msys64/ucrt64/bin/g++.exe`. That compiler directory must also contain `gcc.exe` and `ar.exe`. Raylib source is included locally and compiled into a static library. The script does not install a compiler or change system configuration.

On a machine with the compiler elsewhere, pass its location:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Compiler "C:/path/to/g++.exe"
```

To run just the calculation tests, without building the graphics window:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -TestsOnly
```

Use `-RebuildLibrary` when raylib's source or its compiler configuration has changed. A normal rebuild reuses the compiled raylib archive and recompiles SmartFlow.

## Try the three workspaces

### Simulation

Start or pause traffic, reset the model, or advance one simulated second while paused. Change demand from 4 to 60 arrivals per minute, use Quiet/Normal/Rush presets, and change green duration from 6 to 30 seconds. New green-duration settings take effect at the next green phase.

Playback speeds are 0.5×, 1×, 2× and 4×. The scene contains left-hand traffic, twelve routes, turning vehicles, signals, buildings and landscaping. Toggle route paths, scene labels and the junction boundary. Zoom with the plus/minus buttons and restore the initial view with **Fit**.

Click a vehicle to inspect its approach, turn, position, rotation, speed and stopped time. The dashboard reports active vehicles, stopped vehicles, demand waiting offscreen, completed trips and average wait for completed trips. “Next phase” advances signal control through its clearance rules; it does not simply give another approach an immediate green.

### Graphics lab

Choose **Bresenham line**, **Midpoint circle**, or **Line clipping**. Click the grid to set alternating line endpoints or move a circle's centre. Use the number controls for precise coordinates and the radius slider for circles. **Reset geometry** restores the starting values.

The green marks show the calculated result and the gold geometry provides a visual reference. The three algorithms are implemented in our C++ source; raylib paints the calculated results. Read the [graphics mathematics explanation](docs/graphics-mathematics.md) for worked examples.

### Project brief

This page explains the idea, scope, graphics concepts and assigned responsibilities. Switching away from the Simulation workspace pauses it. When the application loses focus, its traffic model stops advancing until focus returns.

## Keyboard shortcuts

| Key | Action |
| --- | --- |
| `1`, `2`, `3` | Open Simulation, Graphics lab, or Project brief |
| `F11` | Toggle borderless fullscreen |
| `Esc` | Clear vehicle selection |
| `Space` | Start/pause traffic in Simulation |
| `R` | Reset traffic in Simulation |
| `S` | Advance one simulated second when paused |
| `N` | Request the next signal phase |
| `L` | Toggle route paths |
| `H` | Toggle scene labels |
| `B` | Toggle junction boundary |
| `+` / `-` | Zoom in/out; the `=` key also zooms in |

Traffic shortcuts apply in the Simulation workspace. Close the application with the window's close button.

## Main files

| File or folder | Responsibility |
| --- | --- |
| `src/main.cpp` | Window, application loop, controls, graphics lab, project brief and UI smoke checks |
| `src/simulation.cpp` | Routes, vehicle movement, demand, queues, signals and statistics |
| `include/simulation.hpp` | Public model types, constants and simulation interface |
| `src/scene.cpp` | Map and vehicle drawing, world-to-screen view and vehicle selection |
| `include/scene.hpp` | Scene drawing and selection declarations |
| `src/graphics.cpp` | Custom Bresenham, midpoint-circle and Cohen–Sutherland algorithms |
| `include/graphics.hpp` | Graphics algorithm inputs and result types |
| `tests/` | Model and graphics checks that run without a window |
| `scripts/build.ps1` | Compiler commands, test execution and native application build |
| `vendor/raylib-5.5/` | Vendored official raylib source and licence |
| `build/` | Generated objects, library and test executables; ignored by Git |
| `bin/` | Built Windows application and raylib licence copy; ignored by Git |

There is no backend or database. The application holds its simulation state in memory. Closing it ends that run; there is no save/resume feature.

## Our group and contribution areas

| Student number | Group member | Responsibility |
| --- | --- | --- |
| 165899 | **Macharia Grace Wairimu** | Project concept, purpose and introduction |
| 164099 | **Mwangi Billiart B. Muthoni** — [@BilliartMuthoni](https://github.com/BilliartMuthoni) | Mathematics, route geometry and graphics algorithms |
| 146412 | **Njugi Elijah Muiru** — [@ElijahNjugi](https://github.com/ElijahNjugi) | Core C++ programming, simulation and integration |
| 168377 | **Kihumba Kevin Renana** | Scene design, interface and feature scope |
| 152710 | **Kisotu Samuel Lemayian** — [@kisotu2](https://github.com/kisotu2) | Documentation, testing and demonstration preparation |

The sections below map our allocated areas to the implemented project; each member's commits and pull requests record their actual contributions.

### Grace — concept, purpose and introduction

Grace's area connects the technical application to the problem the group is presenting: how can an animated junction make graphics concepts and traffic behaviour easier to understand?

- **Project purpose:** explain the intended audience, the traffic-system idea and its value as a computer graphics learning project.
- **Scope:** explain why the work focuses on one junction rather than an entire city, and distinguish an educational model from a prediction of real road conditions.
- **Introduction:** connect the original idea to the C++ implementation and introduce the features before the technical demonstration.
- **Relevant files:** the introduction and scope sections of this `README.md`, the context in [`docs/migration-guide.md`](docs/migration-guide.md), and the idea-and-audience text in `briefView()` in [`src/main.cpp`](src/main.cpp).

**How this fits the application:** the Project brief gives the audience the reason for the project, its boundaries and the graphics concepts to look for during the demonstration.

### Billiart — mathematics and graphics algorithms

Billiart's area covers the calculations that turn positions and shapes into meaningful graphics. The graphics mathematics is separate from the drawing library so the results can be tested without opening a window.

- **Bresenham line algorithm:** `bresenhamLine()` calculates connected integer grid points between two endpoints using an error value.
- **Midpoint circle algorithm:** `midpointCircle()` calculates one octant and uses eight-way symmetry to produce a circle's points without duplicate coordinates.
- **Cohen–Sutherland clipping:** `clipLine()` uses endpoint region codes and boundary intersections to return the visible part of a line, or reject it.
- **Route geometry:** `cubic()`, `rotate()`, `createRoute()` and `Route::pointAt()` in the simulation module provide Bézier curves, rotated approaches, sampled distances, positions and headings for vehicle movement.
- **Relevant files:** [`include/graphics.hpp`](include/graphics.hpp), [`src/graphics.cpp`](src/graphics.cpp), [`tests/graphics_test.cpp`](tests/graphics_test.cpp), [`docs/graphics-mathematics.md`](docs/graphics-mathematics.md), and the route-geometry sections of [`src/simulation.cpp`](src/simulation.cpp).

**How this fits the application:** the Graphics lab displays the algorithm results in green against a gold geometric reference. Route calculations supply the positions and rotation angles used by moving cars.

### Elijah — core programming, traffic model and integration

Elijah's area is the main technical implementation: connecting the simulation model, native application loop, controls, scene and graphics lab into one working C++ program.

- **Model structure:** `TrafficSimulation`, `Vehicle`, `Route` and `Statistics` hold the traffic state and define how other modules access it.
- **Animation timing:** `step()` accumulates elapsed time, while `fixedStep()` advances the traffic model in fixed 1/60-second increments.
- **Vehicle behaviour:** `spawnArrivals()`, `leaderGap()` and `moveVehicle()` handle demand, queues, acceleration, braking, route progress and conservative vehicle separation.
- **Signals and statistics:** `advanceSignals()` applies the green–amber–all-red sequence and junction-clearance checks; `refreshState()` updates the dashboard values.
- **Application integration:** `main()` and the application views connect keyboard and mouse input to the model, graphics lab and scene renderer.
- **Build integration:** the build script compiles the C++ modules, runs tests and links the native executable with raylib.
- **Relevant files:** [`include/simulation.hpp`](include/simulation.hpp), [`src/simulation.cpp`](src/simulation.cpp), application-control sections of [`src/main.cpp`](src/main.cpp), [`scripts/build.ps1`](scripts/build.ps1), and [`tests/simulation_test.cpp`](tests/simulation_test.cpp).

**How this fits the application:** pressing Start changes the running state, the loop advances the model, and the scene paints the latest vehicle and signal data. The program runs entirely as compiled C++.

### Kevin — scene, interface and feature scope

Kevin's area turns the model's numbers into a clear, interactive visual scene and defines how users explore the features.

- **Scene construction:** `drawMap()` assembles roads, markings, buildings, trees and other environmental details from drawing primitives.
- **Vehicles and signals:** `drawCar()` draws a reusable vehicle at its model position and rotation; `drawSignals()` displays the current signal states.
- **Viewing and interaction:** `sceneCamera()` maps the world to the viewport, `drawScene()` combines the layers, and `pickVehicle()` supports selection for the inspector.
- **Interface:** layout, labels, buttons, sliders, toggles, navigation and metrics in `main.cpp` make the model and lab understandable to users.
- **Feature explanation:** the scope includes the simulation controls, layer switches, graphics lab and project brief.
- **Relevant files:** [`include/scene.hpp`](include/scene.hpp), [`src/scene.cpp`](src/scene.cpp), and the interface/layout sections of [`src/main.cpp`](src/main.cpp).

**How this fits the application:** the renderer reads model data and displays it. Changing a colour or layout does not require rewriting the traffic rules.

### Samuel — documentation, testing and demonstration

Samuel's area makes the project reproducible, explainable and ready for a group demonstration.

- **User documentation:** explain how to launch, build and use the C++ application and how it differs from the original browser project.
- **Test coordination:** organise repeatable checks for traffic routes, timing, queues, algorithm results and interface behaviour with the relevant module leads.
- **Verification records:** document which checks ran, their results, the tested environment and the limits of the evidence.
- **Demonstration preparation:** guide the audience from a paused junction through traffic controls, vehicle inspection and the three graphics algorithms.
- **Submission preparation:** keep source files, build instructions, documentation and third-party licences together, and separate generated output from editable source.
- **Relevant files:** this `README.md`, [`docs/migration-guide.md`](docs/migration-guide.md), [`docs/verification.md`](docs/verification.md), [`tests/`](tests/), [`scripts/package.ps1`](scripts/package.ps1), and coordination of `smokeTest()` in [`src/main.cpp`](src/main.cpp).

**How this fits the application:** another group member or lecturer can reproduce a build, follow a demonstration and connect the reported behaviour with the test evidence.

### Working together

Some files are shared. Billiart's route geometry and Elijah's movement logic both appear in `simulation.cpp`; Kevin's interface and Elijah's application integration both appear in `main.cpp`. Agree on the section being changed, work on a personal branch, run the relevant checks and open a pull request for review. A clear contribution record identifies the member, changed files, purpose, checks and commit or pull-request link.

## How the code fits together

```text
Mouse and keyboard input
          |
          v
main.cpp — application loop, views and controls
     |                         |
     v                         v
simulation.cpp             graphics.cpp
traffic state and rules    calculated pixels / clipped segments
     |                         |
     v                         v
scene.cpp                  Graphics lab in main.cpp
     |                         |
     +------------+------------+
                  v
            raylib drawing
                  v
          Native desktop window
```

The `.hpp` headers declare shared types and functions; the `.cpp` files implement them. The simulation and graphics algorithm modules use the C++ standard library and can be tested without raylib. Raylib supplies the window, input and rendering facilities.

## Testing and verification

The [verification record](docs/verification.md) documents the recorded Windows checks and their limits. Automated checks cover:

- **Traffic:** all twelve routes, reproducible resets, comparison with a reference state from the original model, fixed-step timing, signal clearance, delayed green-setting changes, dense traffic and invalid inputs.
- **Graphics:** line endpoints and connectivity, circle symmetry and unique samples, clipping crossings and rejection, boundary cases and invalid inputs.
- **Interface:** start/pause/reset, sliders and presets, stepping, selection, viewing controls, all three lab modes and workspace navigation.

After a full build, repeat the native interface check with:

```powershell
./bin/SmartFlow.exe --smoke-test build/ui-check
```

This runs an automated interaction sequence in a hidden native window and writes `ui-results.txt` plus screenshots under `build/ui-check/`. Failure returns a nonzero exit code and writes `SmartFlow-error.log`. It does not establish compatibility with every computer or replace all manual usability checks.

## A short group demonstration

1. **Grace:** introduce the traffic-system idea and learning purpose.
2. **Kevin:** explain the junction, available controls and feature scope.
3. **Elijah:** start traffic, change demand, inspect a vehicle, and explain the model and application loop.
4. **Billiart:** show route geometry and demonstrate a line, a circle and a clipping example in the lab.
5. **Samuel:** explain the verification process, build/run instructions and project limitations, then close the demonstration.

## Documentation and submission

- [Migration guide](docs/migration-guide.md): how the browser version maps to C++, file ownership and a simple submission workflow.
- [Graphics mathematics](docs/graphics-mathematics.md): the three custom algorithms and their tests.
- [Verification record](docs/verification.md): check results and the limits of what has been verified.
- [Third-party notices](THIRD_PARTY.md): dependency attribution and licence information.

The original PowerPoint, Word script, handbook and other browser-project materials remain unchanged. They still describe SVG and JavaScript. Use them as historical reference; a C++ presentation should have a separate, updated copy when requested.

For a GitHub source submission, include `src/`, `include/`, `tests/`, `scripts/`, `docs/`, the launch/build command files, `.gitignore`, this README, and the vendored raylib source with its licences. Do not add generated `build/` or `bin/` files to normal source commits. A separate runnable Windows release or ZIP can include the built application and its licence files. Keep third-party code clearly identified as raylib.

After a successful full build, create the runnable project ZIP with:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/package.ps1
```

It creates `SmartFlow-Cpp-Project.zip` in the repository's parent folder. The archive includes source, documentation, vendored raylib and the compiled application. Extract it before launching.

## Scope and dependencies

This is a teaching simulation of one junction. It uses conservative junction reservations and has not been calibrated to predict a real road's capacity or journey times. The scene recreates the project's visual idea with native drawing operations, so individual pixels and typography can differ from the browser version.

Raylib 5.5 supplies the window, input and drawing API; its official source is stored locally in `vendor/raylib-5.5`. The application compiles this dependency into its executable. Windows system graphics libraries remain operating-system dependencies. The UI reads the installed Segoe UI font when available and otherwise uses raylib's default font; no Windows font file is redistributed.
