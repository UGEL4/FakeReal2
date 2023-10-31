#pragma once

#include "ECS/Entity.h"
#include <unordered_map>

using namespace FakeReal;

class Scene
{
public:
    Entity CreateEntity(const std::string& name);
private:
    entt::registry mRegistry;
    std::unordered_map<std::string, entt::entity> mEntityMap;
};