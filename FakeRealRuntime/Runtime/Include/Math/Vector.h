#pragma once

#include "Misc/Types.h"
#include "Math/glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/geometric.hpp"

namespace FakeReal
{
    namespace math
    {
        using Vector3 = glm::vec3;
        using Vector4 = glm::vec4;

        inline float Dot(const Vector3& a, const Vector3& b)
        {
            return glm::dot(a, b);
        }
    }
}