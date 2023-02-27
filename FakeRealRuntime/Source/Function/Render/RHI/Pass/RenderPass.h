#pragma once

#include "Core/Base/BaseDefine.h"
#include "Function/Render/RenderCommon.h"
#include <vector>

namespace FakeReal
{
	class RHI;
	class RenderResource;

	struct RenderPassInitInfo
	{};

	struct RenderPassCommonInfo
	{
		SharedPtr<RHI> rhi;
	};

	struct RenderVisibleNodes
	{
		std::vector<RenderMeshNode>* pMainCameraVisibleMeshNodes;
	};

	class RenderPass
	{
	public:
		RenderPass() = default;
		virtual ~RenderPass() = 0;
		virtual void Initialize(RenderPassCommonInfo* pInfo) = 0;
		virtual void Clear() = 0;
		virtual void PreparePassData(SharedPtr<RenderResource> pRenderResource) = 0;
		virtual void PassUpdateAfterRecreateSwapchain() = 0;
		void SetCommonInfo(RenderPassCommonInfo info);

	public:
		static RenderVisibleNodes mVisibleNodes;
	protected:
		SharedPtr<RHI> m_pRHI;
	};
}