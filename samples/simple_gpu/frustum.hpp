#pragma once

/**
    只使用投影矩阵推导出相应的视锥体平面是在视线空间(View Space)内
*/

#include "plane.hpp"
#include "Math/Matrix.h"

struct Frustum
{
    Plane left;
    Plane right;
    Plane bottom;
    Plane top;
    Plane near;
    Plane far;

    Frustum() = default;
    Frustum(const FakeReal::math::Matrix4X4& projMatrix)
    {
        Initialize(projMatrix);
    }

    void Initialize(const FakeReal::math::Matrix4X4& projMatrix)
    {
        // https://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf  B.1 Plane Extraction for OpenGL
        left.normal.x = projMatrix[0][0] + projMatrix[3][0];
        left.normal.y = projMatrix[0][1] + projMatrix[3][1];
        left.normal.z = projMatrix[0][2] + projMatrix[3][2];
        left.distance = projMatrix[0][3] + projMatrix[3][3];

        right.normal.x = projMatrix[3][0] - projMatrix[0][0];
        right.normal.y = projMatrix[3][1] - projMatrix[0][1];
        right.normal.z = projMatrix[3][2] - projMatrix[0][2];
        right.distance = projMatrix[3][3] - projMatrix[0][3];

        top.normal.x = projMatrix[3][0] - projMatrix[1][0];
        top.normal.y = projMatrix[3][1] - projMatrix[1][1];
        top.normal.z = projMatrix[3][2] - projMatrix[1][2];
        top.distance = projMatrix[3][3] - projMatrix[1][3];

        bottom.normal.x = projMatrix[1][0] + projMatrix[3][0];
        bottom.normal.y = projMatrix[1][1] + projMatrix[3][1];
        bottom.normal.z = projMatrix[1][2] + projMatrix[3][2];
        bottom.distance = projMatrix[1][3] + projMatrix[3][3];

        near.normal.x = projMatrix[2][0] + projMatrix[3][0];
        near.normal.y = projMatrix[2][1] + projMatrix[3][1];
        near.normal.z = projMatrix[2][2] + projMatrix[3][2];
        near.distance = projMatrix[2][3] + projMatrix[3][3];

        far.normal.x = projMatrix[3][0] - projMatrix[2][0];
        far.normal.x = projMatrix[3][1] - projMatrix[2][1];
        far.normal.x = projMatrix[3][2] - projMatrix[2][2];
        far.distance = projMatrix[3][3] - projMatrix[2][3];

        // Normalize the plane equations, if requested
        //if (normalize == true)
        {
            NormalizePlane(left);
            NormalizePlane(right);
            NormalizePlane(top);
            NormalizePlane(bottom);
            NormalizePlane(near);
            NormalizePlane(far);
        }
    }

    void NormalizePlane(Plane& plane)
    {
        float mag = glm::length(plane.normal);
        plane.normal /= mag;
        plane.distance /= mag;
    }
};