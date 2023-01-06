#pragma once
#include "Function/Render/RHI.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <functional>

namespace FakeReal
{
	struct QueueFamilyIndex
	{
		int32_t graphicsFamily;
		int32_t presentFamily;

		QueueFamilyIndex() : graphicsFamily(-1) {}

		bool IsComplete() const
		{
			return graphicsFamily >= 0 && presentFamily >= 0;
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formates;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanRHI : public RHI
	{
	public:
		virtual ~VulkanRHI() override final;

		virtual void Initialize(const RHIInitInfo& info) override final;
		virtual void Clear() override final;
		virtual void PrepareContext() override final;
		virtual void DeviceWaitIdle() override final;

		void WaitForFences();
		void ResetCommandPool();
		bool PrepareFrame(std::function<void()> passUpdateAfterRecreateSwapchain);
		void SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain);
		void WindowSizeCallback(int w, int h);
		void FramebufferResize(bool resize) { mFramebufferResized = resize; }
	private:
		void CreateVKInstance();
		void SetupDebugCallback();
		VkResult CreateDebugUtilsMessengerEXT(
			VkInstance pInstance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* ppCallback);
		void DestroyDebugUtilsMessengerEXT();

		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicDevice();
		void CreateSwapChain();
		void CreateSwapChainImageView();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateDescriptorPool();
		void CreateSyncPrimitives();
		void CreateDepthResource();
		void CreateAssetAllocator();
		void RecreateSwapchain();

	public:
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer pCommandBuffer);

	private:
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequireExtenstion();
		bool IsDeviceSuitable(VkPhysicalDevice pDevice);
		QueueFamilyIndex FindQueueFamilies(VkPhysicalDevice pDevice);
		bool CheckDeviceExtenstionSupport(VkPhysicalDevice pDevice);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice pDevice);
		VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
		VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availabePresentModes) const;
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		VkFormat FindDepthFormat();
		VkFormat FindSupportFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	public:
		uint32_t mVulkanApiVersion;
		GLFWwindow* m_pWindow;

		VkInstance m_pVKInstance;
		VkDebugUtilsMessengerEXT m_pDebugUtils;

		VkSurfaceKHR m_pSurface;
		VkPhysicalDevice m_pPhysicalDevice;
		VkDevice m_pDevice;

		VkQueue m_pGraphicQueue;
		VkQueue m_pPresentQueue;

		VkSwapchainKHR m_pSwapchain;
		std::vector<VkImage> mSwapchainImages;
		std::vector<VkImageView> mSwapchainImageViews;
		VkFormat mSwapchainImageFormat;
		VkExtent2D mSwapchainImageExtent;
		VkRect2D mScissor;
		VkViewport mViewport;

		VkFormat mDepthImageFormat{ VK_FORMAT_UNDEFINED };
		VkImage m_pDepthImage;
		VkImageView m_pDepthImageView;
		VkDeviceMemory m_pDepthImageMemory;

		VkCommandPool m_pDefaultCommandPool;
		static const uint8_t S_MAX_FRAME_IN_FLIGHT{ 3 };
		VkCommandPool m_pCommandPools[S_MAX_FRAME_IN_FLIGHT];
		VkCommandBuffer m_pCommandBuffers[S_MAX_FRAME_IN_FLIGHT];
		VkCommandBuffer m_pCurCommandBuffer;
		int mCurrFrame{ 0 };
		uint32_t mCurSwapchainImageIndex{ 0 };

		VkDescriptorPool m_pDescriptorPool;

		VkSemaphore mImageAvailableSemaphores[S_MAX_FRAME_IN_FLIGHT];
		VkSemaphore mRenderFinishedSemaphores[S_MAX_FRAME_IN_FLIGHT];
		VkFence mInFlightFences[S_MAX_FRAME_IN_FLIGHT];

		VmaAllocator m_pAssetsAllocator;

		bool mFramebufferResized{ false };

	private:
		const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> m_deviceExtenstion = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}