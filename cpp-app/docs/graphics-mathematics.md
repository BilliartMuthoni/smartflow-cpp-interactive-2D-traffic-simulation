# SmartFlow: graphics mathematics in C++

Assigned mathematics lead: **Mwangi Billiart B. Muthoni (164099)**.

This module is a C++17 port of the three custom graphics algorithms in the JavaScript project. The original project is preserved. The C++ version calculates its own raster coordinates and clipped endpoints; the rendering library displays those results.

## Files and their purpose

- `include/graphics.hpp` declares the public types and functions. A declaration tells other C++ files which inputs a function accepts and which result it returns.
- `src/graphics.cpp` implements the calculations. It uses only the C++ standard library and has no raylib dependency.
- `tests/graphics_test.cpp` verifies the results independently of a graphics window. Its checks continue to run in release builds because they do not depend on the `assert` macro.

The functions live in `smartflow::graphics`, a namespace that keeps their names separate from rendering-library functions. `Pixel` stores integer `x` and `y` coordinates. `ClipRect` stores a rectangular viewport's minimum and maximum coordinates. `Segment` stores two endpoints as decimal (`double`) coordinates.

## 1. Bresenham line rasterisation

**Purpose:** turn a straight line between two integer endpoints into a sequence of connected grid pixels.

`bresenhamLine(x0, y0, x1, y1)` returns a `std::vector<Pixel>`. A vector is a resizable sequence. Both endpoints are included, and the same routine handles horizontal, vertical, shallow, steep and reversed lines.

The implementation calculates the horizontal distance `dx`, the negative vertical distance `dy`, and the directions `sx` and `sy`, each of which is either +1 or -1. It starts with `error = dx + dy`. For every pixel, it stores the current position and calculates `twice = 2 * error`.

- If `twice >= dy`, it advances horizontally and adds `dy` to the error.
- If `twice <= dx`, it advances vertically and adds `dx` to the error.
- Both conditions may be true, producing a diagonal step.

The error tracks the difference between the ideal mathematical line and the integer grid. This avoids rounding a newly calculated decimal slope at every step. The arithmetic uses wider integers internally so subtracting integer endpoints does not overflow the error calculation.

**Worked example:** `bresenhamLine(0, 0, 4, 2)` produces `(0,0), (1,1), (2,1), (3,2), (4,2)`. The exact tie decisions match the original JavaScript algorithm.

**Presentation explanation:** “Bresenham decides which grid points best represent a straight line. We keep an error value and use it to decide whether the next point moves horizontally, vertically, or both.”

The public C++ API accepts integer inputs. The JavaScript function accepted numbers and rounded them first. If a caller starts with fractional world coordinates, it must choose its integer raster coordinates before calling this C++ function; the algorithm itself receives the final integers.

## 2. Midpoint circle and eight-way symmetry

**Purpose:** calculate the grid pixels on a circle's circumference.

`midpointCircle(cx, cy, radius)` starts at `(radius, 0)` relative to the circle centre, with `decision = 1 - radius`. It calculates only one octant and reflects each point into the other seven octants:

```text
( x, y)  ( y, x)  (-y, x)  (-x, y)
(-x,-y)  (-y,-x)  ( y,-x)  ( x,-y)
```

The centre `(cx, cy)` is added to each relative point. After each iteration, `y` increases. If the decision value is negative, the algorithm keeps the current `x`; otherwise it decreases `x`. The decision update selects the grid location closer to the ideal circumference. It stops once `x < y`, because the remaining octants have already been covered by symmetry.

Some reflections are identical on axes and diagonals. An `unordered_set` remembers which coordinate pairs have already been added. The vector stores the first occurrence of each pair, preserving the JavaScript module's output order. This avoids painting the same point repeatedly.

**Worked example:** a radius of 1 produces four unique points: `(cx+1,cy)`, `(cx,cy+1)`, `(cx-1,cy)` and `(cx,cy-1)`. A radius of 0, or a negative radius, produces just the centre. A radius of 2 produces twelve unique pixels.

**Presentation explanation:** “A circle is symmetric, so we calculate points in one eighth of it and reflect them into the other seven parts. The midpoint decision chooses the next grid position.”

If a circle would produce a coordinate outside the range of a C++ `int`, the function reports an overflow error instead of silently wrapping that coordinate.

## 3. Cohen–Sutherland line clipping

**Purpose:** calculate which part of a line is visible inside a rectangular viewport.

`clipLine(x0, y0, x1, y1, box)` returns a `std::optional<Segment>`. An optional either contains a visible line segment or is empty (`std::nullopt`) when the line is invisible. A line lying on a rectangle boundary is visible.

Each endpoint receives a four-bit region code:

| Bit value | Position relative to the rectangle |
| --- | --- |
| 1 | Left of `xmin` |
| 2 | Right of `xmax` |
| 4 | Below the minimum numeric y value (`y < ymin`) |
| 8 | Above the maximum numeric y value (`y > ymax`) |

With screen coordinates, y usually increases downward. Therefore bit 4 appears above the viewport on screen, while bit 8 appears below it. The comparisons, rather than the everyday meanings of “up” and “down”, determine the algorithm.

The algorithm uses three cases:

1. If both region codes are zero, both endpoints are inside and the current segment is accepted.
2. If the codes share a bit, both endpoints are outside on the same side and the segment is rejected.
3. Otherwise, an outside endpoint is replaced with its intersection with one rectangle boundary. Its code is recalculated, and the process repeats.

For a horizontal boundary at `y = boundaryY`, the intersection is:

```text
x = x0 + (x1 - x0) * (boundaryY - y0) / (y1 - y0)
y = boundaryY
```

For a vertical boundary at `x = boundaryX`, the intersection is:

```text
y = y0 + (y1 - y0) * (boundaryX - x0) / (x1 - x0)
x = boundaryX
```

**Worked example:** clipping the horizontal line `(-5,5)` to `(15,5)` against a rectangle from `(0,0)` to `(10,10)` returns `(0,5)` to `(10,5)`. A parallel line at `y = -1` is rejected. A single point inside the rectangle is accepted; a single point outside is rejected.

The API rejects non-finite values and reversed rectangle bounds. Zero-width or zero-height rectangles are allowed. A 16-iteration guard, retained from the JavaScript version, reports failure if clipping does not converge.

**Presentation explanation:** “Clipping removes the parts of a line outside the viewing rectangle. Region codes tell us whether to accept it, reject it, or calculate a boundary intersection.”

## Why the mathematics is separate from raylib

Raylib provides the window, input handling and drawing operations for the C++ application. It does not replace these three coursework algorithms. This module returns calculated coordinates without opening a window or choosing colours. The renderer can then draw the returned samples or segment.

This separation helps us explain which calculations are ours, test them without a display, and change the appearance without changing the mathematics. C++ stores this information in structs and vectors; JavaScript used objects and arrays. The underlying decisions and valid-input results remain the same for integer raster inputs.

## What the tests check

- Bresenham: every endpoint combination in a small coordinate grid, covering all octants; endpoint inclusion; connected and unique steps; pixel count; distance from the ideal line; tie behaviour; and coordinates near integer limits.
- Midpoint circle: unique samples; all eight symmetries; cardinal points; radius accuracy; small and negative radii; insertion order; and coordinate overflow reporting.
- Clipping: unchanged inside segments; horizontal, vertical and diagonal crossings; viewport edges and corner tangencies; invisible segments; point segments; zero-size viewport dimensions; endpoint reversal; collinearity; ordered bounds; and rejection of non-finite inputs.

These are graphics calculation checks. They do not replace tests of traffic movement, signal timing, queueing or user controls, which belong to their respective modules.
