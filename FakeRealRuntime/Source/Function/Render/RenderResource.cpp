#include "FRPch.h"
#include "RenderResource.h"
#include "Function/Render/stb_image.h"
#include "Core/Mate/Serializer/Serializer.h"
#include "Resource/ResourceType/Data/MeshData.h"
#include "Resource/AssetManager/AssetManager.h"
#include "Function/Global/GlobalRuntimeContext.h"

namespace FakeReal
{

	RenderResource::~RenderResource()
	{

	}

	RenderMeshData RenderResource::LoadMeshData(const MeshResourceDesc& meshRes)
	{
		RenderMeshData mesh;
		mesh.mStaticMeshData = LoadStaticMeshData(meshRes.mFile);

		return mesh;
	}

	StaticMeshData RenderResource::LoadStaticMeshData(const std::string& file)
	{
		/*static std::vector<MeshVertexDataDefinition> g_Vertices =
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
		*/
		StaticMeshData staticMesh = {};

		MeshData meshData;
		bool result = g_global_runtime_context.m_pAssetManager->LoadAsset(file, meshData);
		if (!result)
		{
			throw std::runtime_error(("Load mesh data failed: file = " + file).c_str());
		}

		//组装mesh
		size_t stride = sizeof(MeshVertexDataDefinition);
		staticMesh.mVertexBuffer = MakeShared<DataBuffer>(meshData.mVertices.size() * stride);
		staticMesh.mIndexBuffer = MakeShared<DataBuffer>(meshData.mIndices.size() * sizeof(uint32_t));
		//toto: fix: MeshVertexDataDefinition and MeshData they maybe different layout;
		memcpy(staticMesh.mVertexBuffer->m_pData, meshData.mVertices.data(), staticMesh.mVertexBuffer->mSize);
		memcpy(staticMesh.mIndexBuffer->m_pData, meshData.mIndices.data(), staticMesh.mIndexBuffer->mSize);

		return staticMesh;
	}

	RenderMaterialData RenderResource::LoadMaterialData(const MaterialResourceDesc& materialRes)
	{
		RenderMaterialData material;
		material.mBaseColorTexture = LoadTexture(materialRes.mBaseColorTextureFile, true);

		return material;
	}

	SharedPtr<TextureData> RenderResource::LoadTexture(const std::string& file, bool isSRGB)
	{
		SharedPtr<TextureData> texture = MakeShared<TextureData>();

		//todo: opitional
		stbi_set_flip_vertically_on_load(true);
		int width, height, comp;
		texture->m_pPixels = stbi_load(file.c_str(), &width, &height, &comp, STBI_rgb_alpha);
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