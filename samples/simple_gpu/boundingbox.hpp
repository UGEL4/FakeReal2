#pragma once
#include "Math/Vector.h"
#include "Math/Matrix.h"

using namespace FakeReal;

struct BoundingBox
{
    math::Vector3 min {FLT_MAX};
    math::Vector3 max {-FLT_MAX};

    void Update(math::Vector3 newPoint)
    {
        if (min.x > newPoint.x) min.x = newPoint.x;
        if (min.y > newPoint.y) min.y = newPoint.y;
        if (min.z > newPoint.z) min.z = newPoint.z;
        if (max.x < newPoint.x) max.x = newPoint.x;
        if (max.y < newPoint.y) max.y = newPoint.y;
        if (max.z < newPoint.z) max.z = newPoint.z;
    }

    void Merge(const BoundingBox& rhs)
    {
        if (min.x > rhs.min.x) min.x = rhs.min.x;
        if (min.y > rhs.min.y) min.y = rhs.min.y;
        if (min.z > rhs.min.z) min.z = rhs.min.z;
        if (max.x < rhs.max.x) max.x = rhs.max.x;
        if (max.y < rhs.max.y) max.y = rhs.max.y;
        if (max.z < rhs.max.z) max.z = rhs.max.z;
    }

    static BoundingBox BoundingBoxTransform(BoundingBox const& b, math::Matrix4X4 const& m)
    {
        // we follow the "BoundingBox::Transform"

        math::Vector3 const g_BoxOffset[8] = { math::Vector3(-1.0f, -1.0f, 1.0f),
                                           math::Vector3(1.0f, -1.0f, 1.0f),
                                           math::Vector3(1.0f, 1.0f, 1.0f),
                                           math::Vector3(-1.0f, 1.0f, 1.0f),
                                           math::Vector3(-1.0f, -1.0f, -1.0f),
                                           math::Vector3(1.0f, -1.0f, -1.0f),
                                           math::Vector3(1.0f, 1.0f, -1.0f),
                                           math::Vector3(-1.0f, 1.0f, -1.0f) };

        size_t const CORNER_COUNT = 8;

        // Load center and extents.
        // Center of the box.
        math::Vector3 center((b.max.x + b.min.x) * 0.5,
                         (b.max.y + b.min.y) * 0.5,
                         (b.max.z + b.min.z) * 0.5);

        // Distance from the center to each side.
        // half extent //more exactly
        math::Vector3 extents((b.max.x - b.min.x) * 0.5,
                          (b.max.y - b.min.y) * 0.5,
                          (b.max.z - b.min.z) * 0.5);

        math::Vector3 min;
        math::Vector3 max;

        // Compute and transform the corners and find new min/max bounds.
        for (size_t i = 0; i < CORNER_COUNT; ++i)
        {
            math::Vector3 corner_before = extents * g_BoxOffset[i] + center;
            math::Vector4 corner_with_w = m * math::Vector4(corner_before.x, corner_before.y, corner_before.z, 1.0);
            math::Vector3 corner        = math::Vector3(corner_with_w.x / corner_with_w.w,
                                                corner_with_w.y / corner_with_w.w,
                                                corner_with_w.z / corner_with_w.w);

            if (0 == i)
            {
                min = corner;
                max = corner;
            }
            else
            {
                min = math::Vector3(std::min(min[0], corner[0]), std::min(min[1], corner[1]), std::min(min[2], corner[2]));
                max = math::Vector3(std::max(max[0], corner[0]), std::max(max[1], corner[1]), std::max(max[2], corner[2]));
            }
        }

        BoundingBox b_out;
        b_out.max = max;
        b_out.min = min;
        return b_out;
    }

    inline void GetCorners(math::Vector3* Corners) const
    {
        math::Vector3 const g_BoxOffset[8] = { math::Vector3(-1.0f, -1.0f, 1.0f),
                                           math::Vector3(1.0f, -1.0f, 1.0f),
                                           math::Vector3(1.0f, 1.0f, 1.0f),
                                           math::Vector3(-1.0f, 1.0f, 1.0f),
                                           math::Vector3(-1.0f, -1.0f, -1.0f),
                                           math::Vector3(1.0f, -1.0f, -1.0f),
                                           math::Vector3(1.0f, 1.0f, -1.0f),
                                           math::Vector3(-1.0f, 1.0f, -1.0f) };

        math::Vector3 vCenter = math::Vector3(
        (max.x + min.x) * 0.5f,
        (max.y + min.y) * 0.5f,
        (max.z + min.z) * 0.5f);
        math::Vector3 vExtents = math::Vector3(
        (max.x - min.x) * 0.5f,
        (max.y - min.y) * 0.5f,
        (max.z - min.z) * 0.5f);

        for (size_t i = 0; i < 8; ++i)
        {
            Corners[i] = vExtents * g_BoxOffset[i] + vCenter;
        }
    }

    BoundingBox GetMin(const BoundingBox& other)
    {
        BoundingBox tmp;
        tmp.min = min;
        tmp.max = max;
        if (tmp.min.x < other.min.x) tmp.min.x = other.min.x;
        if (tmp.min.y < other.min.y) tmp.min.y = other.min.y;
        if (tmp.min.z < other.min.z) tmp.min.z = other.min.z;

        if (tmp.max.x > other.max.x) tmp.max.x = other.max.x;
        if (tmp.max.y > other.max.y) tmp.max.y = other.max.y;
        if (tmp.max.z > other.max.z) tmp.max.z = other.max.z;
        return tmp;
    }

    math::Vector3 GetCenter() const
    {
        return (max + min) * 0.5f;
    }

    inline int RelationWithRay(const math::Vector3& ori, const math::Vector3& dir, float& hitt0, float& hitt1) const
    {
        hitt0 = -FLT_MAX;
        hitt1 = FLT_MAX;
        float t0;
        float t1;
        float tmp;
        math::Vector3 min = this->min;
        math::Vector3 max = this->max;
        for (int i = 0; i < 3; i++)
        {
            if (glm::abs(dir[i]) < std::numeric_limits<float>().epsilon())
            {
                if ((ori[i] < min[i]) || (ori[i] > max[i])) return 0;
            }
            t0 = (min[i] - ori[i]) / dir[i];
            t1 = (max[i] - ori[i]) / dir[i];
            if (t0 > t1)
            {
                tmp = t0;
                t0  = t1;
                t1  = tmp;
            }
            if (t0 > hitt0) hitt0 = t0;
            if (t1 < hitt1) hitt1 = t1;
            if (hitt0 > hitt1) return 0;
            if (hitt1 < 0) return 0;
        }

        if (hitt1 < 0.0f)
        {
            return 0;
        }
        if (hitt0 < 0.0f)
        {
            hitt0 = hitt1;
        }

        return 1;
    }

    /*
        \brief  通过寻找包围盒上最接近平面法向量的对角线，实现平面与AABB的相交测试
        \return 1  若AABB在平面的正半空间上
        \return -1 若AABB在平面的负半空间上
        \return 0  若平面与AABB相交
    */
    inline int RelationWithPlane(const math::Vector3& n, float d) const
    {
        math::Vector3 minP, maxP;
        for (uint32_t i = 0; i < 3; i++)
        {
            if (n[i] >= 0.f)
            {
                minP[i] = min[i];
                maxP[i] = max[i];
            }
            else
            {
                minP[i] = max[i];
                maxP[i] = min[i];
            }
        }

        if (math::Dot(n, minP) + d > 0.f)
        {
            return 1;
        }
        else if (math::Dot(n, maxP) + d < 0.f)
        {
            return -1;
        }
        return 0;
    }
};