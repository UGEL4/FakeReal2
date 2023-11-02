#pragma once

#include "Math/Vector.h"
#include "Math/Quaternion.h"

namespace FakeReal
{
    namespace math
    {
        struct Transform
        {
            Vector3 position {0.f, 0.f, 0.f};
            Vector3 scale {1.f, 1.f, 1.f};
            Quaternion rotation {1.f, 0.f, 0.f, 0.f}; 
        };
    }
} // namespace FakeReal