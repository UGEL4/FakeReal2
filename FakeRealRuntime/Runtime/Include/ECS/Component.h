#pragma once

#include "Math/Transform.h"
#include "Math/Matrix.h"
#include <string>
#include <vector>

namespace FakeReal
{
    struct TransformComponent
    {
        math::Transform transform;

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;

        math::Matrix4X4 GetMatrix() const
        {
            math::Matrix4X4 rotation = glm::toMat4(transform.rotation);
            return glm::translate(math::Matrix4X4(1.f), transform.position) * rotation * glm::scale(math::Matrix4X4(1.f), transform.scale);
        }
    };

    struct GameObjectMaterialDesc
    {
        std::string baseColorTextureFile;
        std::string normalTextureFile;
        std::string metallicTextureFile;
        std::string roughnessTextureFile;
        bool withTexture {false};
    };

    struct GameObjectPartDesc
    {
        std::string meshFile;
        math::Transform transform;
        GameObjectMaterialDesc material;
    };

    struct SubMeshComponent
    {
        std::string meshFile;
        std::string materialFile;
        math::Transform transform;
    };

    struct MeshComponent
    {
        std::vector<SubMeshComponent> rawMeshes;

        MeshComponent() = default;
        MeshComponent(const MeshComponent&) = default;
    };
}