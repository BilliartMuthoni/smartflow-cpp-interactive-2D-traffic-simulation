#pragma once

#include "raylib.h"
#include "simulation.hpp"

namespace smartflow {

// The simulation owns the data; this layer only turns world coordinates into art.
// The viewport is a screen-space rectangle. Zoom remains centred on (500, 400).
void drawScene(const TrafficSimulation& simulation, Rectangle viewport, float zoom,
               bool showRoutes, bool showLabels, bool showBoundary, int selectedId);

// Returns a live vehicle id, or -1 when no car is beneath the screen-space point.
int pickVehicle(const TrafficSimulation& simulation, Rectangle viewport, float zoom,
                Vector2 mouse);

} // namespace smartflow
