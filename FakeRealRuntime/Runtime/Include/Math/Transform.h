#pragma once

#include "Math/Vector.h"
#include "Math/Quaternion.h"

namespace FakeReal
{
    namespace math
    {
        struct Transform
        {
            Vector3 position;
            Vector3 scale;
            Quaternion rotation;
        };
    }
} // namespace FakeReal