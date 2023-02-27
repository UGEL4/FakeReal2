#pragma once

#include "Core/Base/BaseDefine.h"
#include <string>

namespace FakeReal
{
	enum class FR_PIXEL_FORMAT : uint8_t
	{
		FR_PIXEL_FORMAT_UNKNOW = 0,
		FR_PIXEL_FORMAT_R8G8B8_UNORM,
		FR_PIXEL_FORMAT_R8G8B8_SRGB,
		FR_PIXEL_FORMAT_R8G8B8A8_UNORM,
		FR_PIXEL_FORMAT_R8G8B8A8_SRGB,
	};

	enum class FR_IMAGE_TYPE : uint8_t
	{
		FR_IMAGE_TYPE_UNKNOW = 0,
		FR_IMAGE_TYPE_2D,
	};

	class DataBuffer
	{
	public:
		size_t mSize{ 0 };
		void* m_pData{ nullptr };

		DataBuffer() = delete;
		DataBuffer(size_t size)
		{
			mSize = size;
			m_pData = malloc(mSize);
		}

		~DataBuffer()
		{
			if (m_pData)
			{
				free(m_pData);
				m_pData = nullptr;
				mSize = 0;
			}
		}

		bool IsValid() const { return m_pData != nullptr; }
	};

	class TextureData
	{
	public:
		uint32_t		mWidth{ 0 };
		uint32_t		mHeight{ 0 };
		uint32_t		mDepth{ 0 };
		FR_PIXEL_FORMAT	mFormat{ FR_PIXEL_FORMAT::FR_PIXEL_FORMAT_UNKNOW };
		FR_IMAGE_TYPE	mImageType{ FR_IMAGE_TYPE::FR_IMAGE_TYPE_UNKNOW };
		void*			m_pPixels{ nullptr };

		TextureData() = default;
		~TextureData()
		{
			if (m_pPixels)
			{
				free(m_pPixels);
				m_pPixels = nullptr;
			}
		}

		bool IsValid() const { return m_pPixels != nullptr; }
	};

	struct MeshVertexDataDefinition
	{
		float x, y, z;    // position
		float nx, ny, nz; // normal
		//float tx, ty, tz; // tangent
		float u, v;       // UV coordinates
	};

	struct MeshResourceDesc
	{
		std::string mFile;

		bool operator ==(const MeshResourceDesc& other) const { return mFile == other.mFile; }
		size_t GetHashValue() const { return std::hash<std::string> {}(mFile); }
	};

	struct MaterialResourceDesc
	{
		std::string mBaseColorTextureFile;

		bool operator ==(const MaterialResourceDesc& other) const { return mBaseColorTextureFile == other.mBaseColorTextureFile; }
		size_t GetHashValue() const { return std::hash<std::string> {}(mBaseColorTextureFile); }
	};

	struct StaticMeshData
	{
		SharedPtr<DataBuffer> mVertexBuffer;
		SharedPtr<DataBuffer> mIndexBuffer;
	};

	struct RenderMeshData
	{
		StaticMeshData mStaticMeshData;
	};

	struct RenderMaterialData
	{
		SharedPtr<TextureData> mBaseColorTexture;
	};
}

template<>
struct std::hash<FakeReal::MeshResourceDesc>
{
	size_t operator()(const FakeReal::MeshResourceDesc& other) const noexcept { return other.GetHashValue(); }
};

template<>
struct std::hash<FakeReal::MaterialResourceDesc>
{
	size_t operator()(const FakeReal::MaterialResourceDesc& other) const noexcept { return other.GetHashValue(); }
};