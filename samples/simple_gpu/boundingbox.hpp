#pragma once
#include "glm/glm.hpp"

struct BoundingBox
{
    glm::vec3 min {FLT_MAX};
    glm::vec3 max {FLT_MIN};

    void Update(glm::vec3 newPoint)
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

    static BoundingBox BoundingBoxTransform(BoundingBox const& b, glm::mat4 const& m)
    {
        // we follow the "BoundingBox::Transform"

        glm::vec3 const g_BoxOffset[8] = { glm::vec3(-1.0f, -1.0f, 1.0f),
                                           glm::vec3(1.0f, -1.0f, 1.0f),
                                           glm::vec3(1.0f, 1.0f, 1.0f),
                                           glm::vec3(-1.0f, 1.0f, 1.0f),
                                           glm::vec3(-1.0f, -1.0f, -1.0f),
                                           glm::vec3(1.0f, -1.0f, -1.0f),
                                           glm::vec3(1.0f, 1.0f, -1.0f),
                                           glm::vec3(-1.0f, 1.0f, -1.0f) };

        size_t const CORNER_COUNT = 8;

        // Load center and extents.
        // Center of the box.
        glm::vec3 center((b.max.x + b.min.x) * 0.5,
                         (b.max.y + b.min.y) * 0.5,
                         (b.max.z + b.min.z) * 0.5);

        // Distance from the center to each side.
        // half extent //more exactly
        glm::vec3 extents((b.max.x - b.min.x) * 0.5,
                          (b.max.y - b.min.y) * 0.5,
                          (b.max.z - b.min.z) * 0.5);

        glm::vec3 min;
        glm::vec3 max;

        // Compute and transform the corners and find new min/max bounds.
        for (size_t i = 0; i < CORNER_COUNT; ++i)
        {
            glm::vec3 corner_before = extents * g_BoxOffset[i] + center;
            glm::vec4 corner_with_w = m * glm::vec4(corner_before.x, corner_before.y, corner_before.z, 1.0);
            glm::vec3 corner        = glm::vec3(corner_with_w.x / corner_with_w.w,
                                                corner_with_w.y / corner_with_w.w,
                                                corner_with_w.z / corner_with_w.w);

            if (0 == i)
            {
                min = corner;
                max = corner;
            }
            else
            {
                min = glm::vec3(std::min(min[0], corner[0]), std::min(min[1], corner[1]), std::min(min[2], corner[2]));
                max = glm::vec3(std::max(max[0], corner[0]), std::max(max[1], corner[1]), std::max(max[2], corner[2]));
            }
        }

        BoundingBox b_out;
        b_out.max = max;
        b_out.min = min;
        return b_out;
    }
};