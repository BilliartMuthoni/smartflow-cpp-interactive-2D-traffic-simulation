# From the SmartFlow browser project to C++

## Why there are two folders

The original idea is unchanged: use an animated traffic junction to demonstrate computer graphics and basic traffic rules. The lecturer requires C++, so the project now has a separate native implementation.

```text
Comp graphics/
  traffic-simulation/       preserved SVG + JavaScript browser project
  traffic-simulation-cpp/   native C++ project
```

The [browser project's README](../../traffic-simulation/README.md) still belongs to the first folder. Its source, slides, Word documents and existing packages are preserved. The new application lives entirely in the second folder. Links between the documentation help you find the two versions; neither application needs to execute the other.

Changing programming languages does not mean abandoning the idea or the mathematics. Vehicles still have positions, speeds and routes. Signals still follow phases. A circle is still a circle. The main change is how those ideas are expressed and how the computer draws them.

## What C++ replaces

| Browser implementation | C++ implementation | What this means |
| --- | --- | --- |
| `index.html` | Layout and control functions in `src/main.cpp`; map drawing in `src/scene.cpp` | The window is drawn by C++ rather than constructed from HTML elements. |
| `styles.css` | Colours, dimensions, text and drawing choices in `src/main.cpp` and `src/scene.cpp` | Appearance is defined through C++ constants and rendering calls. There is no CSS parser. |
| `src/app.js` | `src/main.cpp`, with scene responsibilities in `src/scene.cpp` | Native mouse/keyboard input replaces browser event listeners. The application loop updates and draws the interface. |
| `src/simulation.js` | `include/simulation.hpp` and `src/simulation.cpp` | Structs, vectors and a simulation class store and update traffic data. |
| `src/graphics.js` | `include/graphics.hpp` and `src/graphics.cpp` | The custom line, circle and clipping algorithms calculate their results in C++. |
| SVG shapes and transforms | Raylib shapes and scene transformations in `src/scene.cpp` | Vehicles and scenery are drawn directly in a native graphics window. |
| Browser animation callbacks | Frame loop in `main.cpp`, with fixed simulation steps in `simulation.cpp` | Drawing and traffic updates continue while the window is running. |
| JavaScript test files | `tests/simulation_test.cpp` and `tests/graphics_test.cpp` | C++ test executables check behaviour and return a failure code if a check fails. |
| Browser build and launch scripts | `scripts/build.ps1`, `Build SmartFlow.cmd`, `Open SmartFlow.cmd` | Compilation produces a Windows executable, which can then run independently of the compiler. |

The `.hpp` files declare the shared types and functions: they explain what other source files can call. The `.cpp` files contain the implementations: they explain how that work happens. The compiler checks and translates the source files, and the linker combines them with raylib into `bin/SmartFlow.exe`.

## Follow one run from opening to movement

1. **Open SmartFlow.cmd starts the executable.** The command file changes to the project directory, checks that the executable exists, and launches it. It does not start a web server.
2. **`main.cpp` opens a window.** Raylib creates the native graphics window. The app chooses its UI font and creates an internal drawing canvas so the interface can scale with the window.
3. **The simulation is constructed.** `TrafficSimulation` starts with seed 42, demand of 24 arrivals per minute and a ten-second green duration. Starting with the same seed and settings makes the generated arrival sequence reproducible.
4. **The scene is drawn while paused.** `main.cpp` draws the tabs, controls and statistics. `scene.cpp` draws the junction and any current vehicles from the model's state.
5. **The user starts traffic.** The button or Space key changes the running state. The frame loop passes elapsed time, adjusted by playback speed, to the simulation.
6. **The model advances in fixed steps.** `simulation.cpp` processes time in 1/60-second increments. It handles arrival demand, signals, queues, vehicle motion, junction clearance and statistics. It does not need raylib to do these calculations.
7. **Routes produce vehicle poses.** A vehicle's progress along its sampled route determines its x/y position and rotation. Curved turns use sampled cubic Bézier geometry.
8. **The renderer paints the latest state.** `scene.cpp` translates, rotates and draws vehicles. The viewport and zoom map world coordinates to the part of the screen containing the junction.
9. **Controls change settings or the view.** Mouse and keyboard handlers can pause, reset, adjust demand and timing, toggle layers, choose a vehicle or move to the lab. Leaving the Simulation workspace pauses the traffic model.
10. **Closing the window ends the run.** The app releases its graphics resources. No database or backend is involved, and it does not save the simulation for later resumption.

The screen is refreshed independently of the logical model's fixed steps. This makes the model easier to test and reduces dependence on the computer's drawing frame rate.

## Where the graphics concepts appear

**Primitive shapes and colour:** roads, buildings, vehicles, signals and vegetation are assembled from drawing operations in `scene.cpp`.

**Translation and rotation:** vehicles move to their model coordinates and rotate to follow the road. Drawing a vehicle around its local origin makes it easier to reuse the same shape at different positions and angles.

**Scaling and viewing:** scene coordinates are fitted into the viewport, then adjusted by zoom. The UI also scales its drawing canvas to the available window size. The scene's clipping limits what is painted outside its viewport.

**Bézier curves:** `simulation.cpp` defines and samples smooth routes through the junction. Distance along a route is converted into a position and direction for drawing.

**Raster algorithms and clipping:** `graphics.cpp` implements Bresenham line rasterisation, midpoint circles and Cohen–Sutherland line clipping. The lab calls these functions and paints their results. Its gold line/circle is a visual reference, while the green marks show calculated output. Read [graphics mathematics](graphics-mathematics.md) for the variables, decisions, examples and test coverage.

Raylib supplies the drawing and input facilities. The traffic rules and the three demonstrated graphics algorithms remain in the project's own source files.

## What each member should study and contribute

| Member | Study and review | Suitable meaningful contributions |
| --- | --- | --- |
| **Grace — Macharia Grace Wairimu (165899)** | The idea, intended audience, manageable scope, README introduction and project-brief wording | Clarify the problem statement, improve the scope explanation, and review presentation introduction text for a future C++ copy. |
| **Billiart — Mwangi Billiart B. Muthoni (164099)** | `graphics.hpp`, `graphics.cpp`, graphics tests, route geometry in `simulation.cpp`, and the mathematics explanation | Check worked examples, explain decision variables and Bézier sampling, improve algorithm validation, and review relevant mathematical code. |
| **Elijah — Njugi Elijah Muiru (146412)** | `main.cpp`, `simulation.hpp`, `simulation.cpp`, build integration and model tests | Lead implementation and integration, resolve model/interface issues, improve signal or motion logic, and validate complete builds. |
| **Kevin — Kihumba Kevin Renana (168377)** | `scene.hpp`, `scene.cpp`, interface/layout sections in `main.cpp`, and feature planning | Improve scene appearance, interaction and layout; review the scope and feature explanation previously allocated to Aira. |
| **Samuel — Kisotu Samuel Lemayian (152710)** | README, this guide, verification record, test cases and demo sequence | Record repeatable checks, improve user instructions, coordinate testing, and prepare the demonstration. |

There are five current members. Assigned responsibility is a starting point for study and collaboration, rather than evidence that someone already authored every line in that area. The same file can contain contributions from more than one person. For example, Billiart's route mathematics and Elijah's movement integration both touch `simulation.cpp`; agree on sections before editing concurrently.

## Build, test and demonstrate

From the project folder, run **Build SmartFlow.cmd**, or:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
```

The build requires an x64 MinGW-w64 C++17 toolchain. The script checks `PATH` and the common MSYS2 location `C:/msys64/ucrt64/bin`, or accepts a compiler path through `-Compiler`. It first compiles and runs the two calculation test programs, then compiles the vendored raylib 5.5 source if its static archive does not yet exist, and finally builds the application. Generated files go into `build/` and `bin/`; the source is not overwritten.

The repository includes the official raylib source under `vendor/`, so building from this prepared folder does not require downloading raylib. A compiler is necessary for building, but people receiving the built executable do not need a compiler. The script does not install a toolchain or modify system packages.

For a short demonstration:

1. Open the app and explain the intersection while it is paused.
2. Start traffic and identify the signal phase, moving vehicles and queue statistics.
3. Change demand, inspect one vehicle, then pause and step forward one second.
4. Toggle route paths to connect movement with the geometry.
5. Open the Graphics lab; draw a line, move a circle's centre, and demonstrate accepted and rejected clipping cases.
6. Open the Project brief and identify the five responsibilities.

Use the [verification record](verification.md) for completed checks and any known limitations. A test's existence is not the same as a successful recorded run. Keep the evidence aligned with the version that will be submitted.

## Prepare the GitHub submission

Agree which repository and branch will contain the C++ version. If both implementations share one repository, keep the two folders separate. If the C++ project has its own repository, start its README by identifying it as the native version.

Commit the editable project sources, headers, tests, build/launch scripts, documentation, `.gitignore`, and the vendored raylib source with its licence files. `build/` and `bin/` are already excluded by `.gitignore`; keep temporary checks, logs and generated test executables out of ordinary source commits too. A runnable release ZIP can be distributed separately.

Each member should work from the same current base, make an understandable change in their assigned area, run relevant checks, and review the difference before committing. A useful sequence is:

```text
Get the current project → create a personal branch → edit your assigned part
→ run checks → inspect the diff → commit with an accurate message
→ push the branch → request review → merge after review
```

Good commit messages describe an actual change, such as “Explain midpoint circle symmetry with a radius-two example” or “Keep vehicle inspector inside the scene panel.” If importing supplied code, say that it was an import and describe any work you personally performed. Do not create artificial history or claim authorship of raylib.

For each contribution, record the member, changed files or sections, purpose, checks run, commit identifier and pull-request link if used. This makes the submission easier to explain and lets reviewers connect the written responsibilities with the actual repository history. No GitHub upload is performed by these documentation files or the build script.

## Presentation and document status

The existing PowerPoint and Word presentation script describe SVG and JavaScript. They remain unchanged, as do the browser handbook and original project files. Their technical wording should not be presented as a description of the C++ implementation.

A future C++ presentation copy should describe C++17, raylib, the native application loop, and the C++ source-file structure while retaining the same traffic-system idea. The five-person roster should be used in that copy. Creating those revised presentation files is a separate step when requested.

## Practical limits

The model demonstrates a single junction with conservative reservations, reproducible demand and graphics concepts. It is not calibrated against a real junction. The native interface recreates the original design but is not guaranteed to match browser rendering pixel for pixel. The current launcher and build scripts target Windows; running on another operating system would require another build setup.

The executable links raylib and the compiler runtime statically, while normal Windows system libraries and graphics drivers remain platform requirements. It loads Segoe UI from the user's Windows installation when available, with raylib's default font as a fallback. No proprietary Windows font is included in the project.
