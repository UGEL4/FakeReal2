#pragma once

#include "Math/glm/mat4x4.hpp"
#include "Math/Vector.h"
#include "Math/glm/gtc/matrix_transform.hpp"

namespace FakeReal
{
    namespace math
    {
        using Matrix4X4 = glm::mat4;

        /* Matrix4X4 CreateFromLookDirRH(const Vector3& pos, const Vector3& dir, const Vector3& up = Vector3(0.f, 1.f, 0.f))
        {
            glm::vec3 f = glm::normalize(dir);
            glm::vec3 s(glm::normalize(glm::cross(f, up)));
            glm::vec3 u(glm::cross(s, f));

            glm::mat4 Result(1);
            Result[0][0] = s.x;
            Result[1][0] = s.y;
            Result[2][0] = s.z;
            Result[0][1] = u.x;
            Result[1][1] = u.y;
            Result[2][1] = u.z;
            Result[0][2] = -f.x;
            Result[1][2] = -f.y;
            Result[2][2] = -f.z;
            Result[3][0] = -dot(s, pos);
            Result[3][1] = -dot(u, pos);
            Result[3][2] = dot(f, pos);
            return Result;
        } */
    }
}
