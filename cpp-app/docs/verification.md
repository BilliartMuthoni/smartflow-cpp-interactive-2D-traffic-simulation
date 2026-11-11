# C++ verification record

Verified on 21 September 2026 on this Windows computer with MinGW-w64 GCC 15.2.0 and raylib 5.5. These results concern the C++ version in this folder.

## Calculation tests

`scripts/build.ps1` compiled and ran both test programs successfully.

The traffic test program passed all eight groups:

1. Twelve continuous routes, correct left-hand lanes and destinations.
2. Reproducible seeded resets.
3. A golden state taken from the original JavaScript model after 90 simulated seconds.
4. Identical fixed-step state across different frame-time partitions.
5. Three seconds of amber and at least two seconds of all-red clearance during manual phase advancement.
6. Green-duration settings taking effect only when a new green phase starts.
7. Five simulated minutes of dense traffic, including separation, signal obedience, all twelve routes, service for every approach and completed trips without deadlock.
8. Bounded configuration values and rejection of invalid elapsed time.

The graphics test program passed its Bresenham, midpoint-circle and Cohen-Sutherland checks. These include 6,561 endpoint-pair line cases, circle symmetry and duplicate detection, radius checks, clipping edge cases, 625 directed clipping pairs and invalid inputs. The model and graphics tests do not need a window or raylib.

## Native interface check

The built application ran its `--smoke-test` mode successfully in a hidden native graphics window. That mode drives the same controls used during ordinary operation, supplies deterministic test input and verifies resulting state. It exercised:

- Start, pause, one-second stepping and reset.
- Demand preset and slider, green-duration slider and playback-speed selection.
- Safe next-phase request, zoom, fit and scene-layer switches.
- Selection of a visible vehicle and its inspector.
- All three graphics-lab modes, grid coordinate input and geometry reset.
- Navigation among simulation, lab and brief, including pausing traffic when leaving the simulation.

The check exported the actual application's rendered simulation, line lab, circle lab, clipping lab and project brief. All five images were reviewed for readable content, clipping and overlap. The project brief contains the five current members.

To repeat the check from the project folder:

```powershell
./bin/SmartFlow.exe --smoke-test build/ui-check
```

A passing check writes `build/ui-check/ui-results.txt` and the five PNGs. A failure returns a nonzero exit code and writes `SmartFlow-error.log`. These generated files are excluded from source submissions. This is an automated interaction check, not a claim that every keyboard shortcut, fullscreen state, screen resolution or accessibility tool has been manually tested.

## Executable and original-file checks

The Windows executable was compiled successfully with the project's C++17 warning flags. Inspection of its imported libraries found only Windows system DLLs and Universal C Runtime components. It does not need a separate raylib DLL, MinGW runtime DLLs, Node.js or JavaScript files. It still needs a compatible graphics driver for OpenGL 3.3 and a compatible 64-bit Windows environment.

SHA256 comparison of 110 original browser-project source, distribution, documentation and presentation files found zero changes. The C++ application uses its own files. The existing JavaScript archive and the supplied presentation were not rewritten.

## Limits

These tests support this educational implementation on the tested computer. They do not establish compatibility with every other computer or validate real-road traffic predictions. The native UI uses different rendering from SVG, so typography and individual pixels can differ. The old presentation and Word materials still describe the preserved browser version.
