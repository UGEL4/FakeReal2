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
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
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
        }
        return vertices;
    }

    static std::vector<uint32_t> GenSphereIndices()
    {
        std::vector<uint32_t> indices;
        bool oddRow = false;
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
        }
        return indices;
    }
};