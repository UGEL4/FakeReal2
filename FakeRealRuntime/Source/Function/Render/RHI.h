#pragma once
#include <memory>

namespace FakeReal
{
	class WindowSystem;

	struct RHIInitInfo
	{
		std::shared_ptr<WindowSystem> pWindowSystem;
	};
	class RHI
	{
	public:
		virtual ~RHI() = 0;
		virtual void Initialize(const RHIInitInfo& info) = 0;
		virtual void Clear() = 0;
		virtual void PrepareContext() = 0;
		virtual void DeviceWaitIdle() = 0;

	protected:
		bool mEnableValidationLayers{ true };
		bool mEnableDebugUtilsLabel{ true };
		uint32_t mMaxMaterialCount{ 256 };
	};

	inline RHI::~RHI() = default;
}