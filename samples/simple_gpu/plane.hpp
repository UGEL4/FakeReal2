#pragma once
#include "Math/Vector.h"
struct Plane
{
    FakeReal::math::Vector3 normal {0.f};
    float distance {0.f};
};