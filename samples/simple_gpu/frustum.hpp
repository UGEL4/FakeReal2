#pragma once

#include "plane.hpp"

struct Frustum
{
    Plane left;
    Plane right;
    Plane bottom;
    Plane top;
    Plane near;
    Plane far;
};