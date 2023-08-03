#pragma once
#include <string_view>
#include <vector>
#include <cmath>

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t materialIndex;
};

class Model
{
public:
    Model(const std::string_view file);
    ~Model();

private:
    void LoadModel(const std::string_view file);

private:
    std::vector<Mesh> mMeshes;
};

struct Sphere
{
    static std::vector<Vertex> GenSphereVertices()
    {
        std::vector<Vertex> vertices;
        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
/*         for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                Vertex v {};
                v.x = xPos;
                v.y = yPos;
                v.z = zPos;
                v.nx = xPos;
                v.ny = yPos;
                v.nz = zPos;
                v.u = xSegment;
                v.v = ySegment;
                vertices.emplace_back(v);
            }
        } */

        uint32_t SPHERE_DIV = 13;
        uint32_t i;
        float ai, si, ci;
        uint32_t j;
        float aj, sj, cj;

        // Generate coordinates
        for (j = 0; j <= SPHERE_DIV; j++) {
          aj = j * PI / SPHERE_DIV;
          sj = std::sin(aj);
          cj = std::cos(aj);
          for (i = 0; i <= SPHERE_DIV; i++) {
            ai = i * 2 * PI / SPHERE_DIV;
            si = std::sin(ai);
            ci = std::cos(ai);

            Vertex v;
            v.x = si * sj;  // X
            v.y = cj;       // Y
            v.z = ci * sj;  // Z
            vertices.emplace_back(v);
          }
        }
        return vertices;
    }

    static std::vector<uint32_t> GenSphereIndices()
    {
        std::vector<uint32_t> indices;
/*         bool oddRow = false;
        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        } */

        uint32_t p1, p2;
        uint32_t SPHERE_DIV = 13;
        uint32_t j;
        uint32_t i;
        for (j = 0; j < SPHERE_DIV; j++) {
            for (i = 0; i < SPHERE_DIV; i++) {
              p1 = j * (SPHERE_DIV+1) + i;
              p2 = p1 + (SPHERE_DIV+1);

              indices.push_back(p1);
              indices.push_back(p2);
              indices.push_back(p1 + 1);

              indices.push_back(p1 + 1);
              indices.push_back(p2);
              indices.push_back(p2 + 1);
            }
        }
        return indices;
    }

    static std::vector<Vertex> GenCubeVertices()
    {
        std::vector<Vertex> vertices = {
            { -0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0 },
            { 0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 0.0 },
            { 0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 1.0 },
            { -0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 1.0 },
            { 0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 0.0 },
            { -0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 1.0, 0.0 },
            { -0.5, 0.5, -0.5, 0.0, 0.0, -1.0, 1.0, 1.0 },
            { 0.5, 0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 1.0 },
            { -0.5, -0.5, 0.5, -1.0, 0.0, 0.0, 1.0, 0.0 },
            { -0.5, 0.5, 0.5, -1.0, 0.0, 0.0, 1.0, 1.0 },
            { -0.5, 0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 1.0 },
            { -0.5, -0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 0.0 },
            { 0.5, -0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 0.0 },
            { 0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 0.0 },
            { 0.5, 0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 1.0 },
            { 0.5, 0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 1.0 },
            { -0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 0.0, 0.0 },
            { 0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 1.0, 0.0 },
            { 0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 1.0, 1.0 },
            { -0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 0.0, 1.0 },
            { -0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 0.0, 0.0 },
            { 0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 1.0, 0.0 },
            { 0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 1.0 },
            { -0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 1.0 }
        };

        return vertices;
    }

    static std::vector<uint32_t> GenCubeIndices()
    {
        std::vector<uint32_t> indeces = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            8, 9, 10, 10, 11, 8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20
        };
        return indeces;
    }
};