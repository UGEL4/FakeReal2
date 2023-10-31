#pragma once

#include "Math/Transform.h"
#include "Math/Matrix.h"
#include <string>

namespace FakeReal
{
    struct TransformComponent
    {
        math::Transform transform;

        math::Matrix4X4 GetMatrix() const
        {
            math::Matrix4X4 rotation = glm::toMat4(transform.rotation);
            return glm::translate(math::Matrix4X4(1.f), transform.position) * rotation * glm::scale(math::Matrix4X4(1.f), transform.scale);
        }
    };

    struct MeshComponent
    {
        std::string meshFile;
    };
}