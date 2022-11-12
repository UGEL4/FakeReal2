#include "FRPch.h"
#include "VulkanRHI.h"
#include "Core/Base/Macro.h"
#include "Function/Render/WindowSystem.h"

namespace FakeReal
{
	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
		void*                                            pUserData
	)
	{
		LOG_ERROR("\tValidationLayer: {}", pCallbackData->pMessage)
		return VK_FALSE;
	}

	VulkanRHI::~VulkanRHI()
	{

	}

	void VulkanRHI::Initialize(const RHIInitInfo& info)
	{
		m_pWindow = info.pWindowSystem->GetWindow();
		CreateVKInstance();
	}

	void VulkanRHI::CreateVKInstance()
	{
		if (mEnableValidationLayers && !CheckValidationLayerSupport())
		{
			LOG_ERROR("Validation layers required, but not available!");
			throw std::runtime_error("Validation layers required, but not available!");
		}

		mVulkanApiVersion = VK_API_VERSION_1_3;
		VkApplicationInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		info.pApplicationName = "FR_Renderer";
		info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
		info.pEngineName = "No_Engine";
		info.engineVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);
		info.apiVersion = mVulkanApiVersion;

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> ExtensionsProp(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, ExtensionsProp.data());

		std::vector<const char*> Extensions = GetRequireExtenstion();

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &info;
		createInfo.ppEnabledExtensionNames = Extensions.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

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
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = DebugCallback;
		info.pUserData = nullptr;

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
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createInfo.queueCount = 1;
			createInfo.queueFamilyIndex = index;
			createInfo.pQueuePriorities = &priority;
			QueueCreateInfos.emplace_back(createInfo);
		}

		VkPhysicalDeviceFeatures features = {};
		features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures = &features;
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtenstion.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtenstion.data();

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
		VkSurfaceFormatKHR format = ChooseSurfaceFormat(details.formates);
		VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(details.presentModes);
		VkExtent2D extent = ChooseSwapExtent(details.capabilities);
		uint32_t imageCoun = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 && imageCoun > details.capabilities.maxImageCount)
		{
			imageCoun = details.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_pSurface;
		createInfo.minImageCount = imageCoun;
		createInfo.imageFormat = format.format;
		createInfo.imageColorSpace = format.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.presentMode = presentMode;
		createInfo.clipped = true;

		QueueFamilyIndex index = FindQueueFamilies(m_pPhysicalDevice);
		uint32_t queueFamilyIndices[] = { index.graphicsFamily, index.presentFamily };
		if (index.graphicsFamily != index.presentFamily)
		{
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		}
		else
		{
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = details.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

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
	}

	void VulkanRHI::CreateSwapChainImageView()
	{
		mSwapchainImageViews.resize(mSwapchainImages.size());
		for (size_t i = 0; i < mSwapchainImageViews.size(); i++)
		{
			CreateImageView(mSwapchainImages[i], mSwapchainImageViews[i], mSwapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	void VulkanRHI::CreateImageView(VkImage pImage, VkImageView& pImageView, VkFormat format, VkImageAspectFlags aspectMask, uint32_t mipLevels)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = pImage;
		createInfo.format = format;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = aspectMask;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(m_pDevice, &createInfo, nullptr, &pImageView) != VK_SUCCESS)
		{
			ASSERT(0);
		}
	}

	void VulkanRHI::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = mSwapchainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = FindDepthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependencyInfo = {};
		dependencyInfo.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencyInfo.dstSubpass = 0;
		dependencyInfo.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencyInfo.srcAccessMask = 0;
		dependencyInfo.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencyInfo.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderCreateInfo = {};
		renderCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderCreateInfo.pAttachments = attachments;
		renderCreateInfo.attachmentCount = 2;
		renderCreateInfo.subpassCount = 1;
		renderCreateInfo.pSubpasses = &subpass;
		renderCreateInfo.dependencyCount = 1;
		renderCreateInfo.pDependencies = &dependencyInfo;

		if (vkCreateRenderPass(m_pDevice, &renderCreateInfo, nullptr, &m_pRenderPass) != VK_SUCCESS)
		{
			LOG_ERROR("VkRenderPass create failed!");
			throw std::runtime_error("VkRenderPass create failed!");
		}
	}

	void VulkanRHI::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

		VkDescriptorSetLayoutBinding samplerBinding = {};
		samplerBinding.binding = 1;
		samplerBinding.descriptorCount = 1;
		samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding bindings[] = { uboLayoutBinding, samplerBinding };

		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = 2;
		info.pBindings = bindings;

		if (vkCreateDescriptorSetLayout(m_pDevice, &info, nullptr, &m_pDescriptorSetLayout) != VK_SUCCESS)
		{
			ASSERT(0);
		}
	}

	void VulkanRHI::CreateGraphicsPipeline()
	{
		/*std::vector<char> vertexShaderCode = ReadFile("shader/vert.spv");
		std::vector<char> fragmentShaderCode = ReadFile("shader/frag.spv");
		VkShaderModule pVertexShaderModule = CreateShaderModule(vertexShaderCode);
		VkShaderModule pFragmentShaderModule = CreateShaderModule(fragmentShaderCode);

		VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
		vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderCreateInfo.module = pVertexShaderModule;
		vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderCreateInfo.pName = "main";
		vertexShaderCreateInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
		fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShaderCreateInfo.module = pFragmentShaderModule;
		fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShaderCreateInfo.pName = "main";
		fragmentShaderCreateInfo.pSpecializationInfo = nullptr;
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };*/

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto descriptions = VertexData::GetVertexInputAttributeDescriptions();
		vertexInputStateInfo.vertexAttributeDescriptionCount = (uint32_t)descriptions.size();
		vertexInputStateInfo.pVertexAttributeDescriptions = descriptions.data();
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &VertexData::GetBindingDescription();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo = {};
		inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

		VkViewport viewPort = {};
		viewPort.x = 0.f;
		viewPort.y = 0.f;
		viewPort.width = (float)mSwapchainImageExtent.width;
		viewPort.height = (float)mSwapchainImageExtent.height;
		viewPort.minDepth = 0.f;
		viewPort.maxDepth = 1.f;

		VkRect2D rect = {};
		rect.offset = { 0, 0 };
		rect.extent = mSwapchainImageExtent;

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
		viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.pViewports = &viewPort;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pScissors = &rect;
		viewportStateCreateInfo.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {};
		rasterizationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateInfo.lineWidth = 1.f;
		rasterizationStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationStateInfo.depthClampEnable = VK_FALSE;
		rasterizationStateInfo.depthBiasEnable = VK_FALSE;
		rasterizationStateInfo.depthBiasClamp = 0.f;
		rasterizationStateInfo.depthBiasConstantFactor = 0.f;
		rasterizationStateInfo.depthBiasSlopeFactor = 0.f;

		VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {};
		multisampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateInfo.sampleShadingEnable = VK_FALSE;
		multisampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleStateInfo.minSampleShading = 0.f;
		multisampleStateInfo.pSampleMask = nullptr;
		multisampleStateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleStateInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendState = {};
		colorBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendState.blendEnable = VK_FALSE;
		/*colorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendState.alphaBlendOp = VK_BLEND_OP_ADD;
	*/
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
		colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendStateCreateInfo.attachmentCount = 1;
		colorBlendStateCreateInfo.pAttachments = &colorBlendState;
		colorBlendStateCreateInfo.blendConstants[0] = 0.f;
		colorBlendStateCreateInfo.blendConstants[1] = 0.f;
		colorBlendStateCreateInfo.blendConstants[2] = 0.f;
		colorBlendStateCreateInfo.blendConstants[3] = 0.f;

		VkPipelineDepthStencilStateCreateInfo depthInfo = {};
		depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthInfo.depthTestEnable = VK_TRUE;
		depthInfo.depthWriteEnable = VK_TRUE;
		depthInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthInfo.depthBoundsTestEnable = VK_FALSE;
		depthInfo.stencilTestEnable = VK_FALSE;

		VkDynamicState dynamicState[] =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
		dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.dynamicStateCount = 2;
		dynamicStateCreateInfo.pDynamicStates = dynamicState;

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_pDescriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_pDevice, &pipelineLayoutCreateInfo, nullptr, &m_pPipelineLayout) != VK_SUCCESS)
		{
			LOG_ERROR("VkPipelineLayout create failed!");
			throw std::runtime_error("VkPipelineLayout create failed!");
			return;
		}

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.stageCount = 2;
		graphicsPipelineCreateInfo.pStages = shaderStages;
		graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
		graphicsPipelineCreateInfo.pDepthStencilState = &depthInfo;
		//graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
		graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateInfo;
		graphicsPipelineCreateInfo.pTessellationState = nullptr;
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateInfo;
		graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		graphicsPipelineCreateInfo.renderPass = m_pRenderPass;
		graphicsPipelineCreateInfo.subpass = 0;
		graphicsPipelineCreateInfo.layout = m_pPipelineLayout;
		graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_pDevice, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &m_pPipeline) != VK_SUCCESS)
		{
			LOG_ERROR("VkPipeline create failed!");
			throw std::runtime_error("VkPipeline create failed!");
			return;
		}

		vkDestroyShaderModule(m_pDevice, pVertexShaderModule, nullptr);
		vkDestroyShaderModule(m_pDevice, pFragmentShaderModule, nullptr);
	}

	bool VulkanRHI::CheckValidationLayerSupport() const
	{
		uint32_t count = 0;
		vkEnumerateInstanceLayerProperties(&count, nullptr);
		std::vector<VkLayerProperties> Layers(count);
		vkEnumerateInstanceLayerProperties(&count, Layers.data());

		for (const char* layerName : validationLayers)
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

	std::vector<const char*> VulkanRHI::GetRequireExtenstion() const
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

	bool VulkanRHI::IsDeviceSuitable(VkPhysicalDevice pDevice) const
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

	FakeReal::QueueFamilyIndex VulkanRHI::FindQueueFamilies(VkPhysicalDevice pDevice) const
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

	const std::vector<const char*> deviceExtenstion =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	bool VulkanRHI::CheckDeviceExtenstionSupport(VkPhysicalDevice pDevice) const
	{
		uint32_t extenstionCount = 0;
		vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extenstionCount, nullptr);
		std::vector<VkExtensionProperties> ExtenstionProps(extenstionCount);
		vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &extenstionCount, ExtenstionProps.data());
		std::set<std::string> RequireExtenstion(deviceExtenstion.begin(), deviceExtenstion.end());

		for (const auto& extenstion : ExtenstionProps)
		{
			RequireExtenstion.erase(extenstion.extensionName);
		}
		return RequireExtenstion.empty();
	}

	FakeReal::SwapChainSupportDetails VulkanRHI::QuerySwapChainSupport(VkPhysicalDevice pDevice) const
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
		//一些窗口系统会使用一个特殊值，扵扩扮扴戳戲 扴变量类型的构大值，表示允许我们自己选择对于窗口柣合适的交换范围，
	//但我们选择的交换范围韴要在minImageExtent与maxImageExtent的范围内
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