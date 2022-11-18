#pragma once
#include "Function/Render/RHI.h"
#include <vulkan/vulkan.h>
#include <vulkan/vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <vector>

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
		friend class MainCameraPass;
	public:
		virtual ~VulkanRHI() override final;

		virtual void Initialize(const RHIInitInfo& info) override final;
		virtual void Clear() override final;

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

	private:
		bool CheckValidationLayerSupport() const;
		std::vector<const char*> GetRequireExtenstion() const;
		bool IsDeviceSuitable(VkPhysicalDevice pDevice) const;
		QueueFamilyIndex FindQueueFamilies(VkPhysicalDevice pDevice) const;
		bool CheckDeviceExtenstionSupport(VkPhysicalDevice pDevice) const;
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice pDevice) const;
		VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
		VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availabePresentModes) const;
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		VkFormat FindDepthFormat();
		VkFormat FindSupportFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	private:
		uint32_t mVulkanApiVersion;
		GLFWwindow* m_pWindow;

		VkInstance m_pVKInstance;
		VkDebugUtilsMessengerEXT m_pDebugUtils;
		VkSurfaceKHR m_pSurface;
		VkPhysicalDevice m_pPhysicalDevice;
		VkDevice m_pDevice;
		VkFormat mDepthImageFormat{ VK_FORMAT_UNDEFINED };

		//¶ÓÁÐ×å
		VkQueue m_pGraphicQueue;
		VkQueue m_pPresentQueue;

		VkSwapchainKHR m_pSwapchain;
		std::vector<VkImage> mSwapchainImages;
		std::vector<VkImageView> mSwapchainImageViews;
		VkFormat mSwapchainImageFormat;
		VkExtent2D mSwapchainImageExtent;
		VkRect2D mScissor;
		VkViewport mViewport;

		VkImage m_pDepthImage;
		VkImageView m_pDepthImageView;
		VkDeviceMemory m_pDepthImageMemory;

		static const uint8_t S_MAX_FRAME_IN_FLIGHT{ 3 };

		VkCommandPool m_pDefaultGraphicCommandPool;
		VkCommandPool m_pCommandPools[S_MAX_FRAME_IN_FLIGHT];
		VkCommandBuffer m_pCommandBuffers[S_MAX_FRAME_IN_FLIGHT];

		VkDescriptorPool m_pDescriptorPool;

		VkSemaphore mAvailableSemaphores[S_MAX_FRAME_IN_FLIGHT];
		VkSemaphore mFinishedSemaphores[S_MAX_FRAME_IN_FLIGHT];
		VkFence mInFlightFences[S_MAX_FRAME_IN_FLIGHT];

		VmaAllocator m_pAssetsAllocator;
	};
}