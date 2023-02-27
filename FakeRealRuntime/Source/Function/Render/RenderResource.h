#pragma once
#include "Function/Render/RenderType.h"
#include <string>

namespace FakeReal
{
	class RHI;
	class RenderEntity;
	class RenderResource
	{
	public:
		virtual ~RenderResource() = 0;

		virtual void UploadGameobjectRenderResource(SharedPtr<RHI> rhi, const RenderEntity& renderEntity, RenderMeshData& meshData) = 0;
		virtual void UploadGameobjectRenderResource(SharedPtr<RHI> rhi, const RenderEntity& renderEntity, RenderMaterialData& materialData) = 0;
		virtual void SetMaterialDescriptorSetLayout(void* pDescriptorSetLayout) = 0;
		virtual void ReleaseAllResources() = 0;
	public:
		RenderMeshData LoadMeshData(const MeshResourceDesc& meshRes);
		StaticMeshData LoadStaticMeshData(const std::string& file);

		RenderMaterialData LoadMaterialData(const MaterialResourceDesc& materialRes);
		SharedPtr<TextureData> LoadTexture(const std::string& file, bool isSRGB = false);
	};
}