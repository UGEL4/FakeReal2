#pragma once

/**
    只使用投影矩阵推导出相应的视锥体平面是在视线空间(View Space)内
*/

#include "plane.hpp"
#include "Math/Matrix.h"

struct Frustum
{
    Plane planes[6];//left, right,  bottom, top, near, far

    Frustum() = default;
    Frustum(const FakeReal::math::Matrix4X4& projMatrix)
    {
        Initialize(projMatrix, false);
    }

    void Initialize(const FakeReal::math::Matrix4X4& projMatrix, bool bNormalize = false)
    {
        // https://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf  B.1 Plane Extraction for OpenGL
        // left
        planes[0].normal.x = projMatrix[0][0] + projMatrix[3][0];
        planes[0].normal.y = projMatrix[0][1] + projMatrix[3][1];
        planes[0].normal.z = projMatrix[0][2] + projMatrix[3][2];
        planes[0].distance = projMatrix[0][3] + projMatrix[3][3];

        //right
        planes[1].normal.x = projMatrix[3][0] - projMatrix[0][0];
        planes[1].normal.y = projMatrix[3][1] - projMatrix[0][1];
        planes[1].normal.z = projMatrix[3][2] - projMatrix[0][2];
        planes[1].distance = projMatrix[3][3] - projMatrix[0][3];

        //top
        planes[3].normal.x = projMatrix[3][0] - projMatrix[1][0];
        planes[3].normal.y = projMatrix[3][1] - projMatrix[1][1];
        planes[3].normal.z = projMatrix[3][2] - projMatrix[1][2];
        planes[3].distance = projMatrix[3][3] - projMatrix[1][3];

        //botttom
        planes[2].normal.x = projMatrix[1][0] + projMatrix[3][0];
        planes[2].normal.y = projMatrix[1][1] + projMatrix[3][1];
        planes[2].normal.z = projMatrix[1][2] + projMatrix[3][2];
        planes[2].distance = projMatrix[1][3] + projMatrix[3][3];

        //near
        planes[4].normal.x = projMatrix[2][0] + projMatrix[3][0];
        planes[4].normal.y = projMatrix[2][1] + projMatrix[3][1];
        planes[4].normal.z = projMatrix[2][2] + projMatrix[3][2];
        planes[4].distance = projMatrix[2][3] + projMatrix[3][3];

        //far
        planes[5].normal.x = projMatrix[3][0] - projMatrix[2][0];
        planes[5].normal.x = projMatrix[3][1] - projMatrix[2][1];
        planes[5].normal.x = projMatrix[3][2] - projMatrix[2][2];
        planes[5].distance = projMatrix[3][3] - projMatrix[2][3];

        // Normalize the plane equations, if requested
        if (bNormalize)
        {
            NormalizePlane(planes[0]);
            NormalizePlane(planes[1]);
            NormalizePlane(planes[2]);
            NormalizePlane(planes[3]);
            NormalizePlane(planes[4]);
            NormalizePlane(planes[5]);
        }
    }

    void NormalizePlane(Plane& plane)
    {
        float mag = glm::length(plane.normal);
        plane.normal /= mag;
        plane.distance /= mag;
    }
};