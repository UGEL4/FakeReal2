#pragma once
#include <vector>
#include "boundingbox.hpp"
#include "camera.hpp"
#include "plane.hpp"

class Culler
{
public:
    void GetAllVisibleAABB(std::vector<BoundingBox>& aabbArray);
    inline void ClearVisibleSet()
    {
        mAABBArray.clear();
    }

    inline void AddVisibleAABB(BoundingBox* aabb)
    {
        mAABBArray.push_back(aabb);
    }

    void PushCameraPlane(Camera& cam);
    void ClearAllPanel()
    {
        mPlaneNum = 0;
    }
    bool IsVisible(const BoundingBox& aabb) const;

    void GetGeometryContent(Camera& cam);
private:
    std::vector<BoundingBox*> mAABBArray;

    uint32_t mPlaneNum {0};
    Plane mPlanes[32];
    Camera* mCamera;
};