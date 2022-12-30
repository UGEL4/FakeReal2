#pragma once
#include "Function/Render/RenderType.h"
#include <string>

namespace FakeReal
{
	class RHI;
	class RenderResource
	{
	public:
		virtual ~RenderResource() = 0;

		virtual void UploadGameobjectRenderResource(SharedPtr<RHI> rhi, size_t asset_id, RenderMeshData& meshData) = 0;
		virtual void UploadGameobjectRenderResource(SharedPtr<RHI> rhi, size_t asset_id, RenderMaterialData& materialData) = 0;
		virtual void SetMaterialDescriptorSetLayout(void* pDescriptorSetLayout) = 0;
	public:
		RenderMeshData LoadMeshData();
		StaticMeshData LoadStaticMeshData();

		RenderMaterialData LoadMaterialData(const std::string& file);
		SharedPtr<TextureData> LoadTexture(const std::string& file, bool isSRGB = false);
	};
}