#pragma once

#include <include/Drawing.h>

// Lets the user grab the bob with the left mouse button and drag it to a new start angle.
// Returns true while the bob is hovered or being dragged (used to highlight it).
bool handleBobDrag(Simulation& sim, const AnimationView& view);
bool isDraggingBob();
