#pragma once

#include <vector>
#include "Core/Mate/Reflection.h"
namespace FakeReal
{
	struct CLASS(Vertex)
	{
		//PROPERTY()
		float x, y, z;		//position
		float nx, ny, nz;	//normal
		float tx, ty;		//uv
	};

	struct CLASS(MeshData)
	{
		std::vector<Vertex> mVertices;
		std::vector<uint32_t> mIndices;
	};
}