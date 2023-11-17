#include "culler.hpp"

void Culler::GetAllVisibleAABB(std::vector<BoundingBox>& aabbArray) const
{
    for (auto& aabb : mAABBArray)
    {
        aabbArray.emplace_back(aabb);
    }
}

void Culler::PushCameraPlane(Camera& cam)
{
    Plane planes[6];
    cam.GetPlane(planes);
    for (uint32_t i = 0; i < 6; i++)
    {
        mPlanes[i] = planes[i];
        mPlaneNum++;
    }
    mCamera = &cam;
}

bool Culler::IsVisible(const BoundingBox& aabb) const
{
    uint32_t num = 0;
    for (uint32_t i = 0; i < mPlaneNum; i++)
    {
        int result = aabb.RelationWithPlane(mPlanes[i].normal, mPlanes[i].distance);
        if (result == -1)
        {
            return false;
        }
        if (aabb.RelationWithPlane(mPlanes[i].normal, mPlanes[i].distance) != -1)
        {
            num++;
        }
    }
    return num > 0;
}

void Culler::GetGeometryContent(Camera& cam)
{

}