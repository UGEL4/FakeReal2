#include "FRPch.h"
#include "RenderResource.h"
#include "Function/Render/stb_image.h"

namespace FakeReal
{

	RenderResource::~RenderResource()
	{

	}

	RenderMeshData RenderResource::LoadMeshData()
	{
		RenderMeshData mesh;
		mesh.mStaticMeshData = LoadStaticMeshData();

		return mesh;
	}

	StaticMeshData RenderResource::LoadStaticMeshData()
	{
		static std::vector<MeshVertexDataDefinition> g_Vertices =
		{
			//前
			{-0.5f, -0.5f, 0.5f,		1.f, 0.f, 0.f,		0.f, 0.f },
			{ 0.5f, -0.5f, 0.5f,		0.f, 1.f, 0.f,		1.f, 0.f },
			{ 0.5f,  0.5f, 0.5f,		0.f, 0.f, 1.f,		1.f, 1.f },
			{-0.5f,  0.5f, 0.5f,		1.f, 1.f, 0.f,		0.f, 1.f },

			//后
			{ 0.5f, -0.5f, -0.5f,		0.f, 1.f, 0.f,		0.f, 0.f },
			{-0.5f, -0.5f, -0.5f,		1.f, 0.f, 0.f,		1.f, 0.f },
			{-0.5f,  0.5f, -0.5f,		1.f, 1.f, 0.f,		1.f, 1.f },
			{ 0.5f,  0.5f, -0.5f,		0.f, 0.f, 1.f,		0.f, 1.f },

			//左
			{-0.5f,  -0.5f,  0.5f,		1.f, 1.f, 0.f,		1.f, 0.f },
			{-0.5f,   0.5f,  0.5f,		1.f, 1.f, 0.f,		1.f, 1.f },
			{-0.5f,   0.5f, -0.5f,		1.f, 1.f, 0.f,		0.f, 1.f },
			{-0.5f,  -0.5f, -0.5f,		1.f, 1.f, 0.f,		0.f, 0.f },

			//右
			{0.5f,  -0.5f,  0.5f,		1.f, 1.f, 0.f,		0.f, 0.f },
			{0.5f,  -0.5f, -0.5f,		1.f, 1.f, 0.f,		1.f, 0.f },
			{0.5f,   0.5f, -0.5f,		1.f, 1.f, 0.f,		1.f, 1.f },
			{0.5f,   0.5f,  0.5f,		1.f, 1.f, 0.f,		0.f, 1.f },

			//上
			{-0.5f,  -0.5f, -0.5f,		1.f, 1.f, 0.f,		0.f, 0.f },
			{ 0.5f,  -0.5f, -0.5f,		1.f, 1.f, 0.f,		1.f, 0.f },
			{ 0.5f,  -0.5f,  0.5f,		1.f, 1.f, 0.f,		1.f, 1.f },
			{-0.5f,  -0.5f,  0.5f,		1.f, 1.f, 0.f,		0.f, 1.f },

			//下
			{-0.5f,   0.5f,  0.5f,		1.f, 1.f, 0.f,		0.f, 0.f },
			{ 0.5f,   0.5f,  0.5f,		1.f, 1.f, 0.f,		1.f, 0.f },
			{ 0.5f,   0.5f, -0.5f,		1.f, 1.f, 0.f,		1.f, 1.f },
			{-0.5f,   0.5f, -0.5f,		1.f, 1.f, 0.f,		0.f, 1.f },
		};

		static std::vector<uint16_t> g_Indices =
		{
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4,
			8, 9, 10, 10, 11, 8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20
		};

		//组装mesh
		size_t stride = sizeof(MeshVertexDataDefinition);
		StaticMeshData meshData;
		meshData.mVertexBuffer	= MakeShared<DataBuffer>(g_Vertices.size() * stride);
		meshData.mIndexBuffer	= MakeShared<DataBuffer>(g_Indices.size() * sizeof(uint16_t));
		memcpy(meshData.mVertexBuffer->m_pData, g_Vertices.data(), meshData.mVertexBuffer->mSize);
		memcpy(meshData.mIndexBuffer->m_pData, g_Indices.data(), meshData.mIndexBuffer->mSize);

		return meshData;
	}

	RenderMaterialData RenderResource::LoadMaterialData(const std::string& file)
	{
		RenderMaterialData material;
		material.mBaseColorTexture = LoadTexture(file, true);

		return material;
	}

	SharedPtr<TextureData> RenderResource::LoadTexture(const std::string& file, bool isSRGB)
	{
		SharedPtr<TextureData> texture = MakeShared<TextureData>();

		int width, height, comp;
		texture->m_pPixels = stbi_load("texture/huaji.jpg", &width, &height, &comp, STBI_rgb_alpha);
		if (!texture->m_pPixels)
		{
			return nullptr;
		}
		texture->mWidth = width;
		texture->mHeight = height;
		texture->mDepth = 1;
		texture->mImageType = FR_IMAGE_TYPE::FR_IMAGE_TYPE_2D;
		texture->mFormat = isSRGB ? FR_PIXEL_FORMAT::FR_PIXEL_FORMAT_R8G8B8A8_SRGB : FR_PIXEL_FORMAT::FR_PIXEL_FORMAT_R8G8B8A8_UNORM;

		return texture;
	}

}