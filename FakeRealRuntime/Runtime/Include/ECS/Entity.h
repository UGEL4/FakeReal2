#pragma once

#include "ECS/entt/entt.hpp"

namespace FakeReal
{
    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, void* scene = nullptr)
            : mEntityHandle(handle)
            , mScene(scene)
        {
        }
        Entity(const Entity& other) = default;

        operator entt::entity() const { return mEntityHandle; }

    private:
        entt::entity mEntityHandle{ entt::null };
        void* mScene{ nullptr };
    };
} // namespace FakeReal