#include "Scene.h"

Entity Scene::CreateEntity(const std::string& name)
{
    entt::entity neEntity = mRegistry.create();
    Entity tmp {neEntity, this};
    mEntityMap[name] = tmp;
    return tmp;
}