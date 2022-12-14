#include "FRPch.h"
#include "VulkanRHI.h"
#include "VulkanUtils.h"
#include "Core/Base/Macro.h"
#include "Function/Render/WindowSystem.h"
#include <array>

namespace FakeReal
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
		void*                                            pUserData
	)
	{
		LOG_INFO("\tValidationLayer: {}", pCallbackData->pMessage)
		return VK_FALSE;
	}

	VulkanRHI::~VulkanRHI()
	{

	}

	void VulkanRHI::Initialize(const RHIInitInfo& info)
	{
		m_pWindow = info.pWindowSystem->GetWindow();

		std::array<int, 2> windowSize = info.pWindowSystem->GetWindowSize();
		mViewport	= { 0.f, 0.f, (float)windowSize[0], (float)windowSize[1], 0.f, 1.f };
		mScissor	= { {0, 0}, {(uint32_t)windowSize[0], (uint32_t)windowSize[1]} };

		CreateVKInstance();
		SetupDebugCallback();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicDevice();
		CreateSwapChain();
		CreateSwapChainImageView();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateDescriptorPool();
		CreateSyncPrimitives();
		CreateDepthResource();
		CreateAssetAllocator();

		info.pWindowSystem->RegisterWindowSizeCallback(BIND_EVENT_FN(VulkanRHI::WindowSizeCallback));
	}

	void VulkanRHI::Clear()
	{
		vmaDestroyAllocator(m_pAssetsAllocator);

		vkDestroyImageView(m_pDevice, m_pDepthImageView, nullptr);
		vkDestroyImage(m_pDevice, m_pDepthImage, nullptr);
		vkFreeMemory(m_pDevice, m_pDepthImageMemory, nullptr);

		for (uint8_t i = 0; i < S_MAX_FRAME_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_pDevice, mImageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(m_pDevice, mRenderFinishedSemaphores[i], nullptr);
			vkDestroyFence(m_pDevice, mInFlightFences[i], nullptr);
		}

		vkDestroyDescriptorPool(m_pDevice, m_pDescriptorPool, nullptr);

		vkDestroyCommandPool(m_pDevice, m_pDefaultCommandPool, nullptr);
		for (uint8_t i = 0; i < S_MAX_FRAME_IN_FLIGHT; i++)
		{
			vkDestroyCommandPool(m_pDevice, m_pCommandPools[i], nullptr);
		}

		for (auto& imageView : mSwapchainImageViews)
		{
			vkDestroyImageView(m_pDevice, imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_pDevice, m_pSwapchain, nullptr);

		vkDestroyDevice(m_pDevice, nullptr);
		vkDestroySurfaceKHR(m_pVKInstance, m_pSurface, nullptr);
		DestroyDebugUtilsMessengerEXT();
		vkDestroyInstance(m_pVKInstance, nullptr);
	}

	void VulkanRHI::PrepareContext()
	{
		m_pCurCommandBuffer = m_pCommandBuffers[mCurrFrame];
	}

	void VulkanRHI::DeviceWaitIdle()
	{
		assert(vkDeviceWaitIdle(m_pDevice) == VK_SUCCESS);
	}

	void VulkanRHI::WaitForFences()
	{
		if (vkWaitForFences(m_pDevice, 1, &mInFlightFences[mCurrFrame], VK_TRUE, std::numeric_limits<uint64_t>::max()) != VK_SUCCESS)
		{
			LOG_ERROR("failed to synchronize");
			throw std::runtime_error("failed to synchronize");
		}
	}

	void VulkanRHI::ResetCommandPool()
	{
		if (vkResetCommandPool(m_pDevice, m_pCommandPools[mCurrFrame], 0) != VK_SUCCESS)
		{
			LOG_ERROR("failed to synchronize");
			throw std::runtime_error("failed to synchronize");
		}
	}

	bool VulkanRHI::PrepareFrame(std::function<void()> passUpdateAfterRecreateSwapchain)
	{
		VkResult result = vkAcquireNextImageKHR(m_pDevice, m_pSwapchain, std::numeric_limits<uint64_t>::max(), 
			mImageAvailableSemaphores[mCurrFrame], nullptr, &mCurSwapchainImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			//mFramebufferResized = false;
			RecreateSwapchain();
			passUpdateAfterRecreateSwapchain();
			return true;
		}
		else if (result == VK_SUBOPTIMAL_KHR)
		{
			RecreateSwapchain();
			passUpdateAfterRecreateSwapchain();

			// NULL submit to wait semaphore
			VkPipelineStageFlags stageFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			VkSubmitInfo info			= {};
			info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.commandBufferCount		= 0;
			info.pCommandBuffers		= nullptr;
			info.waitSemaphoreCount		= 1;
			info.pWaitSemaphores		= &mImageAvailableSemaphores[mCurrFrame];
			info.pWaitDstStageMask		= stageFlags;
			info.signalSemaphoreCount	= 0;
			info.pSignalSemaphores		= nullptr;

			assert(vkResetFences(m_pDevice, 1, &mInFlightFences[mCurrFrame]) == VK_SUCCESS);

			assert(vkQueueSubmit(m_pGraphicQueue, 1, &info, mInFlightFences[mCurrFrame]) == VK_SUCCESS);

			mCurrFrame = (mCurrFrame + 1) % S_MAX_FRAME_IN_FLIGHT;
			return true;
		}
		else
		{
			assert(result == VK_SUCCESS);
		}

		VkCommandBufferBeginInfo cmdinfo = {};
		cmdinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdinfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		cmdinfo.pInheritanceInfo = nullptr;

		assert(vkBeginCommandBuffer(m_pCommandBuffers[mCurrFrame], &cmdinfo) == VK_SUCCESS);
		return false;
	}

	void VulkanRHI::SubmitRendering(std::function<void()> passUpdateAfterRecreateSwapchain)
	{
		if (vkEndCommandBuffer(m_pCommandBuffers[mCurrFrame]) != VK_SUCCESS)
		{
			assert(0);
		}
		//????????????
		VkPipelineStageFlags stageFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };//??semaphores????????????
		VkSubmitInfo info			= {};
		info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.commandBufferCount		= 1;
		info.pCommandBuffers		= &m_pCommandBuffers[mCurrFrame];
		info.waitSemaphoreCount		= 1;
		info.pWaitSemaphores		= &mImageAvailableSemaphores[mCurrFrame];
		info.pWaitDstStageMask		= stageFlags;
		info.signalSemaphoreCount	= 1;
		info.pSignalSemaphores		= &mRenderFinishedSemaphores[mCurrFrame];

		assert(vkResetFences(m_pDevice, 1, &mInFlightFences[mCurrFrame]) == VK_SUCCESS);

		if (vkQueueSubmit(m_pGraphicQueue, 1, &info, mInFlightFences[mCurrFrame]) != VK_SUCCESS)
		{
			assert(0);
		}

		//????
		VkPresentInfoKHR presentInfo	= {};
		presentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount		= 1;
		presentInfo.pSwapchains			= &m_pSwapchain;
		presentInfo.pImageIndices		= &mCurSwapchainImageIndex;
		presentInfo.waitSemaphoreCount	= 1;
		presentInfo.pWaitSemaphores		= &mRenderFinishedSemaphores[mCurrFrame];
		presentInfo.pResults			= nullptr;
		VkResult pr_result = vkQueuePresentKHR(m_pPresentQueue, &presentInfo);
		if (pr_result == VK_ERROR_OUT_OF_DATE_KHR || pr_result == VK_SUBOPTIMAL_KHR || mFramebufferResized)
		{
			mFramebufferResized = false;
			RecreateSwapchain();
			passUpdateAfterRecreateSwapchain();
		}
		else
		{
			assert(pr_result == VK_SUCCESS);
		}

		mCurrFrame = (mCurrFrame + 1) % S_MAX_FRAME_IN_FLIGHT;
	}

	void VulkanRHI::WindowSizeCallback(int w, int h)
	{
		mFramebufferResized = true;
	}

	void VulkanRHI::CreateVKInstance()
	{
		if (mEnableValidationLayers && !CheckValidationLayerSupport())
		{
			LOG_ERROR("Validation layers required, but not available!");
			throw std::runtime_error("Validation layers required, but not available!");
		}

		mVulkanApiVersion		= VK_API_VERSION_1_3;
		VkApplicationInfo info	= {};
		info.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		info.pApplicationName	= "FR_Renderer";
		info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
		info.pEngineName		= "No_Engine";
		info.engineVersion		= VK_MAKE_API_VERSION(0, 1, 3, 0);
		info.apiVersion			= mVulkanApiVersion;

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> ExtensionsProp(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, ExtensionsProp.data());

		std::vector<const char*> Extensions = GetRequireExtenstion();

		VkInstanceCreateInfo createInfo		= {};
		createInfo.sType					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo			= &info;
		createInfo.ppEnabledExtensionNames	= Extensions.data();
		createInfo.enabledExtensionCount	= static_cast<uint32_t>(Extensions.size());
		if (mEnableValidationLayers)
		{
			createInfo.enabledLayerCount	= static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames	= m_validationLayers.data();
		}

		LOG_DEBUG("Extension count:{}", extensionCount);
		for (const auto& p : ExtensionsProp)
		{
			LOG_DEBUG("\tExtension name:{}", p.extensionName);

		}

		LOG_DEBUG("Enable extension count:{}", createInfo.enabledExtensionCount);
		for (uint32_t i = 0; i < createInfo.enabledExtensionCount; i++)
		{
			LOG_DEBUG("\t Enable extension name:{}", createInfo.ppEnabledExtensionNames[i]);
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_pVKInstance) != VK_SUCCESS)
		{
			LOG_ERROR("VKInstance create failed!");
			throw std::runtime_error("VKInstance create failed!");
		}
	}

	void VulkanRHI::SetupDebugCallback()
	{
		VkDebugUtilsMessengerCreateInfoEXT info = {};
		info.sType				= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback	= DebugCallback;
		info.pUserData			= nullptr;

		if (CreateDebugUtilsMessengerEXT(m_pVKInstance, &info, nullptr, &m_pDebugUtils) != VK_SUCCESS)
		{
			LOG_ERROR("Falied to setup debug callback!");
			throw std::runtime_error("Falied to setup debug callback!");
		}
	}

	VkResult VulkanRHI::CreateDebugUtilsMessengerEXT(VkInstance pInstance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* ppCallback)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(pInstance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(pInstance, pCreateInfo, pAllocator, ppCallback);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanRHI::DestroyDebugUtilsMessengerEXT()
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_pVKInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(m_pVKInstance, m_pDebugUtils, nullptr);
		}
	}

	void VulkanRHI::CreateSurface()
	{
		if (glfwCreateWindowSurface(m_pVKInstance, m_pWindow, nullptr, &m_pSurface) != VK_SUCCESS)
		{
			LOG_ERROR("VkSurfaceKHR create failed!");
			throw std::runtime_error("VkSurfaceKHR create failed!");
		}
	}

	void VulkanRHI::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_pVKInstance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			LOG_ERROR("No physical device support!");
			throw std::runtime_error("No physical device support!");
			return;
		}

		std::vector<VkPhysicalDevice> Devices(deviceCount);
		vkEnumeratePhysicalDevices(m_pVKInstance, &deviceCount, Devices.data());

		for (const auto& device : Devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_pPhysicalDevice = device;
				break;
			}
		}

		if (m_pPhysicalDevice == VK_NULL_HANDLE)
		{
			LOG_ERROR("Failed to find suitable GPU!");
			throw std::runtime_error("Failed to find suitable GPU!");
		}
	}

	void VulkanRHI::CreateLogicDevice()
	{
		QueueFamilyIndex index = FindQueueFamilies(m_pPhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
		std::set<int32_t> UniqueQueueFamilise = { index.graphicsFamily, index.presentFamily };

		float priority = 1.f;
		for (int32_t index : UniqueQueueFamilise)
		{
			VkDeviceQueueCreateInfo createInfo = {};
			createInfo.sType			= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createInfo.queueCount		= 1;
			createInfo.queueFamilyIndex = index;
			createInfo.pQueuePriorities = &priority;
			QueueCreateInfos.emplace_back(createInfo);
		}

		VkPhysicalDeviceFeatures features = {};
		features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo		= {};
		deviceCreateInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos		= QueueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount	= static_cast<uint32_t>(QueueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures		= &features;
		if (mEnableValidationLayers)
		{
			deviceCreateInfo.enabledLayerCount		= static_cast<uint32_t>(m_validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames	= m_validationLayers.data();
		}
		deviceCreateInfo.enabledExtensionCount		= static_cast<uint32_t>(m_deviceExtenstion.size());
		deviceCreateInfo.ppEnabledExtensionNames	= m_deviceExtenstion.data();

		if (vkCreateDevice(m_pPhysicalDevice, &deviceCreateInfo, nullptr, &m_pDevice) != VK_SUCCESS)
		{
			LOG_ERROR("VKDevice create failed!");
			throw std::runtime_error("VKDevice create failed!");
		}

		vkGetDeviceQueue(m_pDevice, index.graphicsFamily, 0, &m_pGraphicQueue);
		vkGetDeviceQueue(m_pDevice, index.presentFamily, 0, &m_pPresentQueue);
	}

	void VulkanRHI::CreateSwapChain()
	{
		SwapChainSupportDetails details = QuerySwapChainSupport(m_pPhysicalDevice);
		VkSurfaceFormatKHR format		= ChooseSurfaceFormat(details.formates);
		VkPresentModeKHR presentMode	= ChooseSwapChainPresentMode(details.presentModes);
		VkExtent2D extent				= ChooseSwapExtent(details.capabilities);
		uint32_t imageCoun				= details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 && imageCoun > details.capabilities.maxImageCount)
		{
			imageCoun = details.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface			= m_pSurface;
		createInfo.minImageCount	= imageCoun;
		createInfo.imageFormat		= format.format;
		createInfo.imageColorSpace	= format.colorSpace;
		createInfo.imageExtent		= extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.presentMode		= presentMode;
		createInfo.clipped			= true;

		QueueFamilyIndex index			= FindQueueFamilies(m_pPhysicalDevice);
		uint32_t queueFamilyIndices[]	= { index.graphicsFamily, index.presentFamily };
		if (index.graphicsFamily != index.presentFamily)
		{
			createInfo.queueFamilyIndexCount	= 2;
			createInfo.pQueueFamilyIndices		= queueFamilyIndices;
			createInfo.imageSharingMode			= VK_SHARING_MODE_CONCURRENT;
		}
		else
		{
			createInfo.queueFamilyIndexCount	= 0;
			createInfo.pQueueFamilyIndices		= nullptr;
			createInfo.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform		= details.capabilities.currentTransform;
		createInfo.compositeAlpha	= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.oldSwapchain		= VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_pDevice, &createInfo, nullptr, &m_pSwapchain) != VK_SUCCESS)
		{
			LOG_ERROR("VkSwapchainKHR create failed!");
			throw std::runtime_error("VkSwapchainKHR create failed!");
			return;
		}

		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(m_pDevice, m_pSwapchain, &imageCount, nullptr);
		mSwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_pDevice, m_pSwapchain, &imageCount, mSwapchainImages.data());

		mSwapchainImageFormat = format.format;
		mSwapchainImageExtent = extent;
		mScissor = { {0, 0}, {mSwapchainImageExtent.width, mSwapchainImageExtent.height} };
	}

	void VulkanRHI::CreateSwapChainImageView()
	{
		mSwapchainImageViews.resize(mSwapchainImages.size());
		for (size_t i = 0; i < mSwapchainImageViews.size(); i++)
		{
			mSwapchainImageViews[i] = VulkanUtils::CreateImageView(
				m_pDevice,
				mSwapchainImages[i],
				mSwapchainImageFormat,
				VK_IMAGE_VIEW_TYPE_2D,
				VK_IMAGE_ASPECT_COLOR_BIT,
				1,
				1
			);
		}
	}

	void VulkanRHI::CreateCommandPool()
	{
		QueueFamilyIndex index = FindQueueFamilies(m_pPhysicalDevice);
		{
			VkCommandPoolCreateInfo info	= {};
			info.sType						= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.flags						= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			info.queueFamilyIndex			= index.graphicsFamily;

			if (vkCreateCommandPool(m_pDevice, &info, nullptr, &m_pDefaultCommandPool) != VK_SUCCESS)
			{
				LOG_ERROR("Default VkCommandPool create failed!");
				throw std::runtime_error("VkCommandPool create failed!");
			}
		}
		{
			VkCommandPoolCreateInfo info	= {};
			info.sType						= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.flags						= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
			info.queueFamilyIndex			= index.graphicsFamily;
			
			for (uint8_t i = 0; i < S_MAX_FRAME_IN_FLIGHT; i++)
			{
				if (vkCreateCommandPool(m_pDevice, &info, nullptr, &m_pCommandPools[i]) != VK_SUCCESS)
				{
					LOG_ERROR("VkCommandPool create failed!");
					throw std::runtime_error("VkCommandPool create failed!");
				}
			}
		}
	}

	void VulkanRHI::CreateCommandBuffers()
	{
		VkCommandBufferAllocateInfo info	= {};
		info.sType							= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.commandBufferCount				= 1U;
		info.level							= VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		for (uint8_t i = 0; i < S_MAX_FRAME_IN_FLIGHT; i++)
		{
			info.commandPool = m_pCommandPools[i];
			if (vkAllocateCommandBuffers(m_pDevice, &info, &m_pCommandBuffers[i]) != VK_SUCCESS)
			{
				LOG_ERROR("VkCommandBuffer create failed!");
				throw std::runtime_error("VkCommandBuffer create failed!");
			}
		}
	}

	void VulkanRHI::CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSize[3];
		poolSize[0].type			= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize[0].descriptorCount = mMaxMaterialCount;
		poolSize[1].type			= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize[1].descriptorCount = 8;
		poolSize[2].type			= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		poolSize[2].descriptorCount = 4;

		VkDescriptorPoolCreateInfo info = {};
		info.sType						= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.poolSizeCount				= sizeof(poolSize) / sizeof(poolSize[0]);
		info.pPoolSizes					= poolSize;
		info.maxSets					= mMaxMaterialCount;
		info.flags						= 0U;

		if (vkCreateDescriptorPool(m_pDevice, &info, nullptr, &m_pDescriptorPool) != VK_SUCCESS)
		{
			LOG_ERROR("VkDescriptorPool create failed!");
			throw std::runtime_error("VkDescriptorPool create failed!");
		}
	}

	void VulkanRHI::CreateSyncPrimitives()
	{
		VkSemaphoreCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint8_t i = 0; i < S_MAX_FRAME_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(m_pDevice, &createInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_pDevice, &createInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_pDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
			{
				LOG_ERROR("VkSemaphore create failed!");
				throw std::runtime_error("VkSemaphore create failed!");
			}
		}
	}

	void VulkanRHI::CreateDepthResource()
	{
		mDepthImageFormat = FindDepthFormat();
		VulkanUtils::CreateImage(
			m_pPhysicalDevice, m_pDevice, 
			mSwapchainImageExtent.width, mSwapchainImageExtent.height, 1, 1, 1, VK_IMAGE_TYPE_2D,
			mDepthImageFormat, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_pDepthImage, m_pDepthImageMemory);

		m_pDepthImageView = VulkanUtils::CreateImageView(m_pDevice, m_pDepthImage, 
			mDepthImageFormat, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, 1, 1
		);
	}

	void VulkanRHI::CreateAssetAllocator()
	{
		VmaVulkanFunctions vulkanFunctions		= {};
		vulkanFunctions.vkGetInstanceProcAddr	= &vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr		= &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.vulkanApiVersion	= mVulkanApiVersion;
		allocatorCreateInfo.physicalDevice		= m_pPhysicalDevice;
		allocatorCreateInfo.device				= m_pDevice;
		allocatorCreateInfo.instance			= m_pVKInstance;
		allocatorCreateInfo.pVulkanFunctions	= &vulkanFunctions;

		vmaCreateAllocator(&allocatorCreateInfo, &m_pAssetsAllocator);
	}

	void VulkanRHI::RecreateSwapchain()
	{
		int width, height = 0;
		glfwGetFramebufferSize(m_pWindow, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			glfwWaitEvents();
		}

		//vkDeviceWaitIdle(m_pDevice);
		VkResult result = vkWaitForFences(m_pDevice, S_MAX_FRAME_IN_FLIGHT, mInFlightFences, VK_TRUE, std::numeric_limits<uint64_t>::max());
		assert(result == VK_SUCCESS);

		vkDestroyImageView(m_pDevice, m_pDepthImageView, nullptr);
		vkDestroyImage(m_pDevice, m_pDepthImage, nullptr);
		vkFreeMemory(m_pDevice, m_pDepthImageMemory, nullptr);

		for (uint8_t i = 0; i < S_MAX_FRAME_IN_FLIGHT; i++)
		{
			vkFreeCommandBuffers(m_pDevice, m_pCommandPools[i], 1, &m_pCommandBuffers[i]);
		}

		for (auto& imageView : mSwapchainImageViews)
		{
			vkDestroyImageView(m_pDevice, imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_pDevice, m_pSwapchain, nullptr);

		CreateSwapChain();
		CreateSwapChainImageView();
		CreateCommandBuffers();
		CreateDepthResource();

		//vkDeviceWaitIdle(m_pDevice);
	}

	VkCommandBuffer VulkanRHI::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandBufferCount = 1;
		info.commandPool		= m_pDefaultCommandPool;

		VkCommandBuffer pCommandBuffer;
		vkAllocateCommandBuffers(m_pDevice, &info, &pCommandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(pCommandBuffer, &beginInfo);

		return pCommandBuffer;
	}

	void VulkanRHI::EndSingleTimeCommands(VkCommandBuffer pCommandBuffer)
	{
		vkEndCommandBuffer(pCommandBuffer);

		VkSubmitInfo submitInfo			= {};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &pCommandBuffer;
		vkQueueSubmit(m_pGraphicQueue, 1, &submitInfo, nullptr);
		vkQueueWaitIdle(m_pGraphicQueue);

		vkFreeCommandBuffers(m_pDevice, m_pDefaultCommandPool, 1, &pCommandBuffer);
	}

	bool VulkanRHI::CheckValidationLayerSupport()
	{
		uint32_t count = 0;
		vkEnumerateInstanceLayerProperties(&count, nullptr);
		std::vector<VkLayerProperties> Layers(count);
		vkEnumerateInstanceLayerProperties(&count, Layers.data());

		for (const char* layerName : m_validationLayers)
		{
			bool found = false;
			for (const auto& layer : Layers)
			{
				if (strcmp(layerName, layer.layerName) == 0)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> VulkanRHI::GetRequireExtenstion()
	{
		uint32_t glfwExtenstionCount = 0;
		const char** glfwExtenstions = glfwGetRequiredInstanceExtensions(&glfwExtenstionCount);

		std::vector<const char*> Extenstions(glfwExtenstions, glfwExtenstions + glfwExtenstionCount);

		//if debug
		if (mEnableValidationLayers || mEnableDebugUtilsLabel)
		{
			Extenstions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return Extenstions;
	}

	bool VulkanRHI::IsDeviceSuitable(VkPhysicalDevice pDevice)
	{
		QueueFamilyIndex index = FindQueueFamilies(pDevice);

		bool extenstionSupport = CheckDeviceExtenstionSupport(pDevice);

		VkPhysicalDeviceFeatures featuresSupport;
		vkGetPhysicalDeviceFeatures(pDevice, &featuresSupport);

		bool swapChainAdequate = false;
		if (extenstionSupport)
		{
			SwapChainSupportDetails details = QuerySwapChainSupport(pDevice);
			swapChainAdequate = !details.formates.empty() && !details.presentModes.empty();
		}
		return index.IsComplete() && extenstionSupport && swapChainAdequate && featuresSupport.samplerAnisotropy;
	}

	FakeReal::QueueFamilyIndex VulkanRHI::FindQueueFamilies(VkPhysicalDevice pDevice)
	{
		uint32_t familiesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familiesCount, nullptr);
		std::vector<VkQueueFamilyProperties> QueueFamilies(familiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &familiesCount, QueueFamilies.data());

		QueueFamilyIndex index;
		int i = 0;
		for (const auto& family : QueueFamilies)
		{
			if (family.queueCount > 0 && (family.queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				index.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, m_pSurface, &presentSupport);
			if (presentSupport)
			{
				index.presentFamily = i;
			}

			if (index.IsComplete())
			{
				break;
			}

			i++;
		}

		return index;
	}

	bool VulkanRHI::CheckDeviceExtenstionSupport(VkPhysicalDevice pDevice)
	{
		uint32_t extenstionCount = 0;
		vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extenstionCount, nullptr);
		std::vector<VkExtensionProperties> ExtenstionProps(extenstionCount);
		vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extenstionCount, ExtenstionProps.data());
		std::set<std::string> RequireExtenstion(m_deviceExtenstion.begin(), m_deviceExtenstion.end());

		for (const auto& extenstion : ExtenstionProps)
		{
			RequireExtenstion.erase(extenstion.extensionName);
		}
		return RequireExtenstion.empty();
	}

	FakeReal::SwapChainSupportDetails VulkanRHI::QuerySwapChainSupport(VkPhysicalDevice pDevice)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, m_pSurface, &details.capabilities);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, m_pSurface, &formatCount, nullptr);
		if (formatCount > 0)
		{
			details.formates.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, m_pSurface, &formatCount, details.formates.data());
		}

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, m_pSurface, &presentModeCount, nullptr);
		if (presentModeCount > 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, m_pSurface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR VulkanRHI::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
	{
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (const auto& format : availableFormats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		if (availableFormats.size() >= 1)
		{
			return availableFormats[0];
		}

		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	VkPresentModeKHR VulkanRHI::ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availabePresentModes) const
	{
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& mode : availabePresentModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return mode;
			}
			else if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				bestMode = mode;
			}
		}
		return bestMode;
	}

	VkExtent2D VulkanRHI::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
	{
		//???????????????????????????????T?????S???? ?S????????????????????????????????????????????????????????????????
		//??????????????????????????minImageExtent??maxImageExtent????????
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height = 0;
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			VkExtent2D extent2D = { width, height };
			extent2D.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, extent2D.width));
			extent2D.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, extent2D.height));

			return extent2D;
		}
	}

	VkFormat VulkanRHI::FindDepthFormat()
	{
		return FindSupportFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL, 
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	VkFormat VulkanRHI::FindSupportFormat(
		const std::vector<VkFormat>& candidates, 
		VkImageTiling tiling, 
		VkFormatFeatureFlags features
	)
	{
		for (auto& format : candidates)
		{
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(m_pPhysicalDevice, format, &properties);

			if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}
		ASSERT(0);
		return VK_FORMAT_UNDEFINED;
	}

}