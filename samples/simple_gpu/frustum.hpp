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
    Frustum(const FakeReal::math::Matrix4X4& mat)
    {
        Initialize(mat, true);
    }

    void Initialize(const FakeReal::math::Matrix4X4& mat, bool bNormalize = false)
    {
        // https://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf  B.1 Plane Extraction for OpenGL
        // left
        planes[0].normal.x = mat[0].w + mat[0].x;
        planes[0].normal.y = mat[1].w + mat[1].x;
        planes[0].normal.z = mat[2].w + mat[2].x;
        planes[0].distance = mat[3].w + mat[3].x;
        /* planes[0].normal.x = mat[0][0] + mat[3][0];
        planes[0].normal.y = mat[0][1] + mat[3][1];
        planes[0].normal.z = mat[0][2] + mat[3][2];
        planes[0].distance = mat[0][3] + mat[3][3]; */

        //right
        /* planes[1].normal.x = mat[3][0] - mat[0][0];
        planes[1].normal.y = mat[3][1] - mat[0][1];
        planes[1].normal.z = mat[3][2] - mat[0][2];
        planes[1].distance = mat[3][3] - mat[0][3]; */
        planes[1].normal.x = mat[0].w - mat[0].x;
        planes[1].normal.y = mat[1].w - mat[1].x;
        planes[1].normal.z = mat[2].w - mat[2].x;
        planes[1].distance = mat[3].w - mat[3].x;


        //top
        /* planes[3].normal.x = mat[3][0] - mat[1][0];
        planes[3].normal.y = mat[3][1] - mat[1][1];
        planes[3].normal.z = mat[3][2] - mat[1][2];
        planes[3].distance = mat[3][3] - mat[1][3]; */
        planes[3].normal.x = mat[0].w - mat[0].y;
        planes[3].normal.y = mat[1].w - mat[1].y;
        planes[3].normal.z = mat[2].w - mat[2].y;
        planes[3].distance = mat[3].w - mat[3].y;

        //botttom
        /* planes[2].normal.x = mat[1][0] + mat[3][0];
        planes[2].normal.y = mat[1][1] + mat[3][1];
        planes[2].normal.z = mat[1][2] + mat[3][2];
        planes[2].distance = mat[1][3] + mat[3][3]; */
        planes[2].normal.x = mat[0].w + mat[0].y;
        planes[2].normal.y = mat[1].w + mat[1].y;
        planes[2].normal.z = mat[2].w + mat[2].y;
        planes[2].distance = mat[3].w + mat[3].y;

        //near
        /* planes[4].normal.x = mat[2][0] + mat[3][0];
        planes[4].normal.y = mat[2][1] + mat[3][1];
        planes[4].normal.z = mat[2][2] + mat[3][2];
        planes[4].distance = mat[2][3] + mat[3][3]; */
        planes[4].normal.x = mat[0].w + mat[0].z;
        planes[4].normal.y = mat[1].w + mat[1].z;
        planes[4].normal.z = mat[2].w + mat[2].z;
        planes[4].distance = mat[3].w + mat[3].z;

        //far
        /* planes[5].normal.x = mat[3][0] - mat[2][0];
        planes[5].normal.x = mat[3][1] - mat[2][1];
        planes[5].normal.x = mat[3][2] - mat[2][2];
        planes[5].distance = mat[3][3] - mat[2][3]; */
        planes[5].normal.x = mat[0].w - mat[0].z;
        planes[5].normal.y = mat[1].w - mat[1].z;
        planes[5].normal.z = mat[2].w - mat[2].z;
        planes[5].distance = mat[3].w - mat[3].z;

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