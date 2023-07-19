#pragma once

#include <cstdint>
#include "Gpu/Backend/Vulkan/GPUVulkan.h"
#include "Gpu/shader-reflections/spirv/spirv_reflect.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VulkanUtil_DescriptorPool
{
    struct GPUDevice_Vulkan* Device;
    VkDescriptorPool pVkDescPool;
    VkDescriptorPoolCreateFlags mFlags;
} VulkanUtil_DescriptorPool;

void VulkanUtil_SelectValidationLayers(struct GPUInstance_Vulkan* pInstance, const char** instanceLayers, uint32_t layersCount);
void VulkanUtil_SelectInstanceExtensions(struct GPUInstance_Vulkan* pInstance, const char** instanceExtensions, uint32_t extensionCount);
void VulkanUtil_QueryAllAdapters(struct GPUInstance_Vulkan* pInstance, const char** ppExtensions, uint32_t extensionsCount);
void VulkanUtil_SelectPhysicalDeviceExtensions(GPUAdapter_Vulkan* pAdapter, const char** ppExtensions, uint32_t extensionsCount);
void VulkanUtil_SelectQueueFamilyIndex(GPUAdapter_Vulkan* pAdapter);
void VulkanUtil_EnumFormatSupport(GPUAdapter_Vulkan* pAdapter);
void VulkanUtil_RecordAdaptorDetail(GPUAdapter_Vulkan* pAdapter);
void VulkanUtil_EnableValidationLayers(struct GPUInstance_Vulkan* pInstance, const struct VkDebugUtilsMessengerCreateInfoEXT* pMessengerCreateInfo);
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanUtil_DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                             void* pUserData);

VkFormat VulkanUtil_GPUFormatToVulkanFormat(EGPUFormat format);
EGPUFormat VulkanUtil_VulkanFormatToGPUFormat(VkFormat format);
VkPrimitiveTopology VulkanUtil_PrimitiveTopologyToVk(EGPUPrimitiveTopology topology);
VkImageAspectFlags VulkanUtil_DeterminAspectMask(VkFormat format, bool includeStencilBit);
VkSampleCountFlagBits VulkanUtil_SampleCountToVk(EGPUSampleCount sampleCount);
VkCompareOp VulkanUtil_CompareOpToVk(EGPUCompareMode compareMode);
VkStencilOp VulkanUtil_StencilOpToVk(EGPUStencilOp op);
VkBufferUsageFlags VulkanUtil_DescriptorTypesToImageUsage(GPUResourceTypes descriptors);
VkBufferUsageFlags VulkanUtil_DescriptorTypesToBufferUsage(GPUResourceTypes descriptors, bool texel);
VkFormatFeatureFlags VulkanUtil_ImageUsageToFormatFeatures(VkImageUsageFlags usage);

void VulkanUtil_InitializeShaderReflection(GPUDeviceID device, GPUShaderLibrary_Vulkan* S, const struct GPUShaderLibraryDescriptor* desc);
void VulkanUtil_FreeShaderReflection(GPUShaderLibrary_Vulkan* S);

VkShaderStageFlags VulkanUtil_TranslateShaderUsages(GPUShaderStages shader_stages);
VkDescriptorType VulkanUtil_TranslateResourceType(EGPUResourceType type);
VkAccessFlags VulkanUtil_ResourceStateToVkAccessFlags(EGPUResourceState state);

void VulkanUtil_ConsumeDescriptorSets(VulkanUtil_DescriptorPool* pool, const VkDescriptorSetLayout* pLayouts, VkDescriptorSet* pSets, uint32_t setsNum);
void VulkanUtil_ReturnDescriptorSets(struct VulkanUtil_DescriptorPool* pPool, VkDescriptorSet* pSets, uint32_t setsNum);

uint32_t VulkanUtil_BitSizeOfBlock(EGPUFormat format);

VkImageLayout VulkanUtil_ResourceStateToImageLayout(EGPUResourceState usage);
VkPipelineStageFlags VulkanUtil_DeterminePipelineStageFlags(GPUAdapter_Vulkan* A, VkAccessFlags accessFlags, EGPUQueueType queue_type);

VkFilter VulkanUtil_TranslateFilterType(EGPUFilterType type);
VkSamplerMipmapMode VulkanUtil_TranslateMipMapMode(EGPUMipMapMode mode);
VkSamplerAddressMode VulkanUtil_TranslateAddressMode(EGPUAddressMode mode);

#define GPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE (VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT + 1)
static const VkDescriptorPoolSize gDescriptorPoolSizes[GPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE] = {
    { VK_DESCRIPTOR_TYPE_SAMPLER, 1024 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8192 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024 },
    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8192 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 },
    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 },
};

// Shader Reflection
static const EGPUResourceType RTLut[] = {
    GPU_RESOURCE_TYPE_SAMPLER,                // SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER
    GPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, // SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    GPU_RESOURCE_TYPE_TEXTURE,                // SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    GPU_RESOURCE_TYPE_RW_TEXTURE,             // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE
    GPU_RESOURCE_TYPE_TEXEL_BUFFER,           // SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    GPU_RESOURCE_TYPE_RW_TEXEL_BUFFER,        // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    GPU_RESOURCE_TYPE_UNIFORM_BUFFER,         // SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    GPU_RESOURCE_TYPE_RW_BUFFER,              // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER
    GPU_RESOURCE_TYPE_UNIFORM_BUFFER,         // SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    GPU_RESOURCE_TYPE_RW_BUFFER,              // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
    GPU_RESOURCE_TYPE_INPUT_ATTACHMENT,       // SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
    GPU_RESOURCE_TYPE_RAY_TRACING             // SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
};

static EGPUTextureDimension DIMLut[SpvDimSubpassData + 1] = {
    GPU_TEX_DIMENSION_1D,        // SpvDim1D
    GPU_TEX_DIMENSION_2D,        // SpvDim2D
    GPU_TEX_DIMENSION_3D,        // SpvDim3D
    GPU_TEX_DIMENSION_CUBE,      // SpvDimCube
    GPU_TEX_DIMENSION_UNDEFINED, // SpvDimRect
    GPU_TEX_DIMENSION_UNDEFINED, // SpvDimBuffer
    GPU_TEX_DIMENSION_UNDEFINED  // SpvDimSubpassData
};
static EGPUTextureDimension ArrDIMLut[SpvDimSubpassData + 1] = {
    GPU_TEX_DIMENSION_1D_ARRAY,   // SpvDim1D
    GPU_TEX_DIMENSION_2D_ARRAY,   // SpvDim2D
    GPU_TEX_DIMENSION_UNDEFINED,  // SpvDim3D
    GPU_TEX_DIMENSION_CUBE_ARRAY, // SpvDimCube
    GPU_TEX_DIMENSION_UNDEFINED,  // SpvDimRect
    GPU_TEX_DIMENSION_UNDEFINED,  // SpvDimBuffer
    GPU_TEX_DIMENSION_UNDEFINED   // SpvDimSubpassData
};

static const VkPipelineBindPoint gPipelineBindPoint[GPU_PIPELINE_TYPE_COUNT] = {
    VK_PIPELINE_BIND_POINT_MAX_ENUM,
    VK_PIPELINE_BIND_POINT_COMPUTE,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
#ifdef ENABLE_RAYTRACING
    VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
#endif
};

static const VkCullModeFlagBits gVkCullModeTranslator[GPU_CULL_MODE_COUNT] = {
    VK_CULL_MODE_NONE,
    VK_CULL_MODE_BACK_BIT,
    VK_CULL_MODE_FRONT_BIT
};

static const VkPolygonMode gVkFillModeTranslator[GPU_FILL_MODE_COUNT] = {
    VK_POLYGON_MODE_FILL,
    VK_POLYGON_MODE_LINE
};

static const VkFrontFace gVkFrontFaceTranslator[] = {
    VK_FRONT_FACE_COUNTER_CLOCKWISE,
    VK_FRONT_FACE_CLOCKWISE
};

static const VkBlendFactor gVkBlendConstantTranslator[GPU_BLEND_CONST_COUNT] = {
    VK_BLEND_FACTOR_ZERO,
    VK_BLEND_FACTOR_ONE,
    VK_BLEND_FACTOR_SRC_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    VK_BLEND_FACTOR_DST_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    VK_BLEND_FACTOR_SRC_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    VK_BLEND_FACTOR_DST_ALPHA,
    VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
    VK_BLEND_FACTOR_CONSTANT_COLOR,
    VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
};

static const VkBlendOp gVkBlendOpTranslator[GPU_BLEND_MODE_COUNT] = {
    VK_BLEND_OP_ADD,
    VK_BLEND_OP_SUBTRACT,
    VK_BLEND_OP_REVERSE_SUBTRACT,
    VK_BLEND_OP_MIN,
    VK_BLEND_OP_MAX,
};

static const VkAttachmentLoadOp gVkAttachmentLoadOpTranslator[GPU_LOAD_ACTION_COUNT] = {
    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    VK_ATTACHMENT_LOAD_OP_LOAD,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
};

static const VkAttachmentStoreOp gVkAttachmentStoreOpTranslator[GPU_STORE_ACTION_COUNT] = {
    VK_ATTACHMENT_STORE_OP_STORE,
    VK_ATTACHMENT_STORE_OP_DONT_CARE
};

static const VkCompareOp gVkCompareOpTranslator[GPU_CMP_COUNT] = {
    VK_COMPARE_OP_NEVER,
    VK_COMPARE_OP_LESS,
    VK_COMPARE_OP_EQUAL,
    VK_COMPARE_OP_LESS_OR_EQUAL,
    VK_COMPARE_OP_GREATER,
    VK_COMPARE_OP_NOT_EQUAL,
    VK_COMPARE_OP_GREATER_OR_EQUAL,
    VK_COMPARE_OP_ALWAYS,
};

#ifdef __cplusplus
}
#endif