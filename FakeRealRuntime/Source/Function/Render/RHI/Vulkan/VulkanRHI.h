#pragma once
#include "Function/Render/RHI.h"
#include <vulkan/vulkan.h>
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
	public:
		virtual ~VulkanRHI() override final;

		virtual void Initialize(const RHIInitInfo& info) override final;

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
		void CreateImageView(VkImage pImage, VkImageView& pImageView, VkFormat format, VkImageAspectFlags aspectMask, uint32_t mipLevels);
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();

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

		//������
		VkQueue m_pGraphicQueue;
		VkQueue m_pPresentQueue;

		VkSwapchainKHR m_pSwapchain;
		std::vector<VkImage> mSwapchainImages;
		std::vector<VkImageView> mSwapchainImageViews;
		VkFormat mSwapchainImageFormat;
		VkExtent2D mSwapchainImageExtent;

		VkRenderPass m_pRenderPass;

		VkDescriptorSetLayout m_pDescriptorSetLayout;

		VkPipeline m_pPipeline;
		VkPipelineLayout m_pPipelineLayout;
	};
}