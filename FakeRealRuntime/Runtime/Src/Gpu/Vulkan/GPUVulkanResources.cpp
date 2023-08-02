#include "Gpu/Backend/Vulkan/GPUVulkan.h"
#include "Gpu/Backend/Vulkan/VulkanUtils.h"

GPUTextureViewID GPUCreateTextureView_Vulkan(GPUDeviceID pDevice, const GPUTextureViewDescriptor* pDesc)
{
    GPUDevice_Vulkan* pVkDevice           = (GPUDevice_Vulkan*)pDevice;
    GPUTextureView_Vulkan* pVkTextureView = (GPUTextureView_Vulkan*)_aligned_malloc(sizeof(GPUTextureView_Vulkan), alignof(GPUTextureView_Vulkan));
    memset(pVkTextureView, 0, sizeof(GPUTextureView_Vulkan));

    GPUTexture_Vulkan* pVkTexture = (GPUTexture_Vulkan*)pDesc->pTexture;

    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    VkImageType imageType    = pDesc->pTexture->isCube ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
    switch (imageType)
    {
        case VK_IMAGE_TYPE_1D:
            viewType = pDesc->arrayLayerCount > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
            break;
        case VK_IMAGE_TYPE_2D:
            viewType = VK_IMAGE_VIEW_TYPE_2D;
            break;
        case VK_IMAGE_TYPE_3D: {
            if (pDesc->arrayLayerCount > 1)
            {
                assert(0);
            }
            viewType = VK_IMAGE_VIEW_TYPE_3D;
        }
        break;
        default:
            assert(0);
            break;
    }
    assert(viewType != VK_IMAGE_VIEW_TYPE_MAX_ENUM);

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_NONE;
    if (pDesc->aspectMask & EGPUTextureViewAspect::GPU_TVA_COLOR)
    {
        aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (pDesc->aspectMask & EGPUTextureViewAspect::GPU_TVA_DEPTH)
    {
        aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (pDesc->aspectMask & EGPUTextureViewAspect::GPU_TVA_STENCIL)
    {
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageViewCreateInfo createInfo{};
    createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image                           = pVkTexture->pVkImage;
    createInfo.viewType                        = viewType;
    createInfo.format                          = VulkanUtil_GPUFormatToVulkanFormat(pDesc->format);
    createInfo.components.r                    = VK_COMPONENT_SWIZZLE_R;
    createInfo.components.g                    = VK_COMPONENT_SWIZZLE_G;
    createInfo.components.b                    = VK_COMPONENT_SWIZZLE_B;
    createInfo.components.a                    = VK_COMPONENT_SWIZZLE_A;
    createInfo.subresourceRange.aspectMask     = aspectMask;
    createInfo.subresourceRange.baseMipLevel   = pDesc->baseMipLevel;
    createInfo.subresourceRange.levelCount     = pDesc->mipLevelCount;
    createInfo.subresourceRange.baseArrayLayer = pDesc->baseArrayLayer;
    createInfo.subresourceRange.layerCount     = pDesc->arrayLayerCount;

    pVkTextureView->pVkSRVDescriptor = VK_NULL_HANDLE;
    if (pDesc->usages & EGPUTexutreViewUsage::GPU_TVU_SRV)
    {
        if (pVkDevice->mVkDeviceTable.vkCreateImageView(pVkDevice->pDevice,
                                                        &createInfo,
                                                        GLOBAL_VkAllocationCallbacks,
                                                        &pVkTextureView->pVkSRVDescriptor) != VK_SUCCESS)
        {
            assert(0);
        }
    }
    pVkTextureView->pVkUAVDescriptor = VK_NULL_HANDLE;
    if (pDesc->usages & EGPUTexutreViewUsage::GPU_TVU_UAV)
    {
        VkImageViewCreateInfo tmp = createInfo;
        // #NOTE : We dont support imageCube, imageCubeArray for consistency with other APIs
        // All cubemaps will be used as image2DArray for Image Load / Store ops
        if (tmp.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY || tmp.viewType == VK_IMAGE_VIEW_TYPE_CUBE)
        {
            tmp.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }
        tmp.subresourceRange.baseMipLevel = pDesc->baseMipLevel;
        if (pVkDevice->mVkDeviceTable.vkCreateImageView(pVkDevice->pDevice,
                                                        &tmp,
                                                        GLOBAL_VkAllocationCallbacks,
                                                        &pVkTextureView->pVkUAVDescriptor) != VK_SUCCESS)
        {
            assert(0);
        }
    }
    pVkTextureView->pVkRTVDSVDescriptor = VK_NULL_HANDLE;
    if (pDesc->usages & EGPUTexutreViewUsage::GPU_TVU_RTV_DSV)
    {
        if (pVkDevice->mVkDeviceTable.vkCreateImageView(pVkDevice->pDevice,
                                                        &createInfo,
                                                        GLOBAL_VkAllocationCallbacks,
                                                        &pVkTextureView->pVkRTVDSVDescriptor) != VK_SUCCESS)
        {
            assert(0);
        }
    }

    return &pVkTextureView->super;
}

void GPUFreeTextureView_Vulkan(GPUTextureViewID pTextureView)
{
    GPUDevice_Vulkan* pVkDevice  = (GPUDevice_Vulkan*)pTextureView->pDevice;
    GPUTextureView_Vulkan* pView = (GPUTextureView_Vulkan*)pTextureView;
    if (pView->pVkRTVDSVDescriptor != VK_NULL_HANDLE)
    {
        pVkDevice->mVkDeviceTable.vkDestroyImageView(pVkDevice->pDevice, pView->pVkRTVDSVDescriptor, GLOBAL_VkAllocationCallbacks);
    }
    if (pView->pVkSRVDescriptor != VK_NULL_HANDLE)
    {
        pVkDevice->mVkDeviceTable.vkDestroyImageView(pVkDevice->pDevice, pView->pVkSRVDescriptor, GLOBAL_VkAllocationCallbacks);
    }
    if (pView->pVkUAVDescriptor != VK_NULL_HANDLE)
    {
        pVkDevice->mVkDeviceTable.vkDestroyImageView(pVkDevice->pDevice, pView->pVkUAVDescriptor, GLOBAL_VkAllocationCallbacks);
    }

    _aligned_free(pView);
}

GPUTextureID GPUCreateTexture_Vulkan(GPUDeviceID device, const GPUTextureDescriptor* desc)
{
    if (desc->sample_count > GPU_SAMPLE_COUNT_1 && desc->mip_levels > 1)
    {
        //cgpu_error("Multi-Sampled textures cannot have mip maps");
        assert(false);
        return nullptr;
    }
    // Alloc aligned memory
    size_t totalSize     = sizeof(GPUTexture_Vulkan);
    uint64_t unique_id   = UINT64_MAX;
    GPUQueue_Vulkan* Q   = (GPUQueue_Vulkan*)desc->owner_queue;
    GPUDevice_Vulkan* D  = (GPUDevice_Vulkan*)device;
    GPUAdapter_Vulkan* A = (GPUAdapter_Vulkan*)device->pAdapter;

    bool owns_image      = false;
    bool is_dedicated    = false;
    bool can_alias_alloc = false;
    bool is_imported     = false;
    VkImageType mImageType;
    VkImage pVkImage                        = VK_NULL_HANDLE;
    VkDeviceMemory pVkDeviceMemory          = VK_NULL_HANDLE;
    uint32_t aspect_mask                    = 0;
    VmaAllocation vmaAllocation             = VK_NULL_HANDLE;
    const bool is_depth_stencil             = FormatUtil_IsDepthStencilFormat(desc->format);
    const GPUFormatSupport* format_support = &A->adapterDetail.format_supports[desc->format];
    /*if (desc->native_handle && !(desc->flags & CGPU_INNER_TCF_IMPORT_SHARED_HANDLE))
    {
        owns_image = false;
        pVkImage   = (VkImage)desc->native_handle;
    }
    else */if (!desc->is_aliasing)
    {
        owns_image = true;
    }

    // Usage flags
    VkImageUsageFlags additionalFlags = 0;
    if (desc->descriptors & GPU_RESOURCE_TYPE_RENDER_TARGET)
        additionalFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    else if (is_depth_stencil)
        additionalFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    uint32_t arraySize = desc->array_size;
    // Image type
    mImageType = VK_IMAGE_TYPE_MAX_ENUM;
    if (desc->flags & GPU_TCF_FORCE_2D)
    {
        assert(desc->depth == 1);
        mImageType = VK_IMAGE_TYPE_2D;
    }
    else if (desc->flags & GPU_TCF_FORCE_3D)
        mImageType = VK_IMAGE_TYPE_3D;
    else
    {
        if (desc->depth > 1)
            mImageType = VK_IMAGE_TYPE_3D;
        else if (desc->height > 1)
            mImageType = VK_IMAGE_TYPE_2D;
        else
            mImageType = VK_IMAGE_TYPE_1D;
    }
    GPUResourceTypes descriptors = desc->descriptors;
    bool cubemapRequired         = (GPU_RESOURCE_TYPE_TEXTURE_CUBE == (descriptors & GPU_RESOURCE_TYPE_TEXTURE_CUBE));
    bool arrayRequired           = mImageType == VK_IMAGE_TYPE_3D;
    // TODO: Support stencil format
    const bool isStencilFormat = false;
    (void)isStencilFormat;
    // TODO: Support planar format
    const bool isPlanarFormat  = false;
    const uint32_t numOfPlanes = 1;
    const bool isSinglePlane   = true;
    assert(((isSinglePlane && numOfPlanes == 1) || (!isSinglePlane && numOfPlanes > 1 && numOfPlanes <= MAX_PLANE_COUNT)));

    if (pVkImage == VK_NULL_HANDLE)
    {
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.pNext                 = NULL;
        imageCreateInfo.flags                 = 0;
        imageCreateInfo.imageType             = mImageType;
        imageCreateInfo.format                = (VkFormat)VulkanUtil_GPUFormatToVulkanFormat(desc->format);
        imageCreateInfo.extent.width          = desc->width;
        imageCreateInfo.extent.height         = desc->height;
        imageCreateInfo.extent.depth          = desc->depth;
        imageCreateInfo.mipLevels             = desc->mip_levels;
        imageCreateInfo.arrayLayers           = arraySize;
        imageCreateInfo.samples               = VulkanUtil_SampleCountToVk(desc->sample_count);
        imageCreateInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage                 = VulkanUtil_DescriptorTypesToImageUsage(descriptors);
        imageCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.queueFamilyIndexCount = 0;
        imageCreateInfo.pQueueFamilyIndices   = NULL;
        imageCreateInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;
        aspect_mask                           = VulkanUtil_DeterminAspectMask(imageCreateInfo.format, true);
        imageCreateInfo.usage |= additionalFlags;
        if (cubemapRequired)
            imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        if (arrayRequired)
            imageCreateInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;

        VkFormatProperties format_props{};
        vkGetPhysicalDeviceFormatProperties(A->pPhysicalDevice, imageCreateInfo.format, &format_props);
        //if (isPlanarFormat) // multi-planar formats must have each plane separately bound to memory, rather than having a single memory binding for the whole image
        //{
        //    cgpu_assert(format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DISJOINT_BIT);
        //    imageCreateInfo.flags |= VK_IMAGE_CREATE_DISJOINT_BIT;
        //}
        if ((VK_IMAGE_USAGE_SAMPLED_BIT & imageCreateInfo.usage) || (VK_IMAGE_USAGE_STORAGE_BIT & imageCreateInfo.usage))
        {
            // Make it easy to copy to and from textures
            imageCreateInfo.usage |= (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        }
        assert(format_support->shader_read && "GPU shader can't' read from this format");
        // Verify that GPU supports this format
        VkFormatFeatureFlags format_features = VulkanUtil_ImageUsageToFormatFeatures(imageCreateInfo.usage);
        VkFormatFeatureFlags flags           = format_props.optimalTilingFeatures & format_features;
        assert((flags != 0) && "Format is not supported for GPU local images (i.e. not host visible images)");
        VmaAllocationCreateInfo mem_reqs {};
        if (desc->is_aliasing)
        {
            // Aliasing VkImage
            VkResult res = D->mVkDeviceTable.vkCreateImage(D->pDevice, &imageCreateInfo, GLOBAL_VkAllocationCallbacks, &pVkImage);
            assert(res == VK_SUCCESS);
        }
        else
        {
            // Allocate texture memory
            if (desc->flags & GPU_TCF_OWN_MEMORY_BIT)
                mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

            mem_reqs.usage = (VmaMemoryUsage)VMA_MEMORY_USAGE_GPU_ONLY;
#ifdef USE_EXTERNAL_MEMORY_EXTENSIONS
            VkExternalMemoryImageCreateInfo externalInfo = { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_KHR, NULL };
            VkExportMemoryAllocateInfo exportMemoryInfo  = { VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR, NULL };
    #if defined(VK_USE_PLATFORM_WIN32_KHR)
            wchar_t* win32Name                                     = CGPU_NULLPTR;
            const wchar_t* nameFormat                              = L"cgpu-shared-texture-%llu";
            VkImportMemoryWin32HandleInfoKHR win32ImportInfo       = { VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, NULL };
            VkExportMemoryWin32HandleInfoKHR win32ExportMemoryInfo = { VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_KHR, NULL };
    #endif
            if (A->external_memory && (desc->flags & CGPU_INNER_TCF_IMPORT_SHARED_HANDLE))
            {
                is_imported           = true;
                imageCreateInfo.pNext = &externalInfo;
    #if defined(VK_USE_PLATFORM_WIN32_KHR)
                CGPUImportTextureDescriptor* pImportDesc = (CGPUImportTextureDescriptor*)desc->native_handle;
                externalInfo.handleTypes                 = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
                if (pImportDesc->backend == CGPU_BACKEND_D3D12)
                {
                    externalInfo.handleTypes   = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
                    win32ImportInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
                }
                if (pImportDesc->backend == CGPU_BACKEND_VULKAN)
                {
                    externalInfo.handleTypes   = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
                    win32ImportInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
                }
                // format name wstring
                unique_id       = pImportDesc->shared_handle;
                int size_needed = swprintf(CGPU_NULL, 0, nameFormat, unique_id);
                win32Name       = cgpu_calloc(1 + size_needed, sizeof(wchar_t));
                swprintf(win32Name, 1 + size_needed, nameFormat, unique_id);
                // record import info
                win32ImportInfo.handle = NULL;
                win32ImportInfo.name   = win32Name;
                // Allocate external (importable / exportable) memory as dedicated memory to avoid adding unnecessary complexity to the Vulkan Memory Allocator
                uint32_t memoryType = 0;
                VkResult findResult = vmaFindMemoryTypeIndexForImageInfo(D->pVmaAllocator, &imageCreateInfo, &mem_reqs, &memoryType);
                if (findResult != VK_SUCCESS)
                {
                    cgpu_error("Failed to find memory type for image");
                }
                // import memory
                VkResult importRes = D->mVkDeviceTable.vkCreateImage(D->pVkDevice, &imageCreateInfo, GLOBAL_VkAllocationCallbacks, &pVkImage);
                CHECK_VKRESULT(importRes);
                VkMemoryDedicatedRequirements MemoryDedicatedRequirements   = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS };
                VkMemoryRequirements2 MemoryRequirements2                   = { VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
                MemoryRequirements2.pNext                                   = &MemoryDedicatedRequirements;
                VkImageMemoryRequirementsInfo2 ImageMemoryRequirementsInfo2 = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2 };
                ImageMemoryRequirementsInfo2.image                          = pVkImage;
                // WARN: Memory access violation unless validation instance layer is enabled, otherwise success but...
                D->mVkDeviceTable.vkGetImageMemoryRequirements2(D->pVkDevice, &ImageMemoryRequirementsInfo2, &MemoryRequirements2);
                VkMemoryAllocateInfo importAllocation = {
                    .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .allocationSize  = MemoryRequirements2.memoryRequirements.size, // this is valid for import allocations
                    .memoryTypeIndex = memoryType,
                    .pNext           = &win32ImportInfo
                };
                cgpu_info("Importing external memory %ls allocation of size %llu", win32Name, importAllocation.allocationSize);
                importRes = D->mVkDeviceTable.vkAllocateMemory(D->pVkDevice, &importAllocation, GLOBAL_VkAllocationCallbacks, &pVkDeviceMemory);
                CHECK_VKRESULT(importRes);
                // bind memory
                importRes = D->mVkDeviceTable.vkBindImageMemory(D->pVkDevice, pVkImage, pVkDeviceMemory, 0);
                CHECK_VKRESULT(importRes);
                if (importRes == VK_SUCCESS)
                {
                    cgpu_trace("Imported image %p with allocation %p", pVkImage, pVkDeviceMemory);
                }
    #endif
            }
            else if (A->external_memory && desc->flags & CGPU_TCF_EXPORT_BIT)
            {
                imageCreateInfo.pNext = &externalInfo;
    #if defined(VK_USE_PLATFORM_WIN32_KHR)
                const VkExternalMemoryHandleTypeFlags exportFlags =
                VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT |
                VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT;
                externalInfo.handleTypes = exportFlags;
                // format name wstring
                uint64_t pid       = (uint64_t)GetCurrentProcessId();
                uint64_t shared_id = D->next_shared_id++;
                unique_id          = (pid << 32) | shared_id;
                int size_needed    = swprintf(CGPU_NULL, 0, nameFormat, unique_id);
                win32Name          = cgpu_calloc(1 + size_needed, sizeof(wchar_t));
                swprintf(win32Name, 1 + size_needed, nameFormat, unique_id);
                // record export info
                win32ExportMemoryInfo.dwAccess    = GENERIC_ALL;
                win32ExportMemoryInfo.name        = win32Name;
                win32ExportMemoryInfo.pAttributes = CGPU_NULLPTR;
                exportMemoryInfo.pNext            = &win32ExportMemoryInfo;
                exportMemoryInfo.handleTypes      = exportFlags;
                cgpu_trace("Exporting texture with name %ls size %dx%dx%d", win32Name, desc->width, desc->height, desc->depth);
    #else
                exportMemoryInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    #endif
                // mem_reqs.pUserData = &exportMemoryInfo;
                uint32_t memoryType = 0;
                VkResult findResult = vmaFindMemoryTypeIndexForImageInfo(D->pVmaAllocator, &imageCreateInfo, &mem_reqs, &memoryType);
                if (findResult != VK_SUCCESS)
                {
                    cgpu_error("Failed to find memory type for image");
                }
                if (D->pExternalMemoryVmaPools[memoryType] == CGPU_NULLPTR)
                {
                    D->pExternalMemoryVmaPoolNexts[memoryType] = cgpu_calloc(1, sizeof(VkExportMemoryAllocateInfoKHR));
                    VkExportMemoryAllocateInfoKHR* Next        = (VkExportMemoryAllocateInfoKHR*)D->pExternalMemoryVmaPoolNexts[memoryType];
                    Next->sType                                = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
                    VmaPoolCreateInfo poolCreateInfo           = {
                                  .memoryTypeIndex     = memoryType,
                                  .blockSize           = 0,
                                  .maxBlockCount       = 1024,
                                  .pMemoryAllocateNext = D->pExternalMemoryVmaPoolNexts[memoryType]
                    };
                    if (vmaCreatePool(D->pVmaAllocator, &poolCreateInfo, &D->pExternalMemoryVmaPools[memoryType]) != VK_SUCCESS)
                    {
                        cgpu_assert(0 && "Failed to create VMA Pool");
                    }
                }
                memcpy(D->pExternalMemoryVmaPoolNexts[memoryType], &exportMemoryInfo, sizeof(VkExportMemoryAllocateInfoKHR));
                mem_reqs.pool = D->pExternalMemoryVmaPools[memoryType];
                // Allocate external (importable / exportable) memory as dedicated memory to avoid adding unnecessary complexity to the Vulkan Memory Allocator
                mem_reqs.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            }
#endif
            // Do allocation if is not imported
            if (!is_imported)
            {
                VmaAllocationInfo alloc_info = { 0 };
                if (isSinglePlane)
                {
                    if (!desc->is_dedicated && !is_imported && !(desc->flags & GPU_TCF_EXPORT_BIT))
                    {
                        mem_reqs.flags |= VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
                    }
                    VkResult res = vmaCreateImage(D->pVmaAllocator,
                                                  &imageCreateInfo, &mem_reqs, &pVkImage,
                                                  &vmaAllocation, &alloc_info);
                    assert(res == VK_SUCCESS);
                }
                else // Multi-planar formats
                {
                    // TODO: Planar formats
                }
            }
//#ifdef VK_USE_PLATFORM_WIN32_KHR
//            cgpu_free(win32Name);
//#endif
            is_dedicated    = mem_reqs.flags & VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
            can_alias_alloc = mem_reqs.flags & VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
        }
    }
    GPUTexture_Vulkan* T = (GPUTexture_Vulkan*)_aligned_malloc(totalSize, _alignof(GPUTexture_Vulkan));
    memset(T, 0, totalSize);
    assert(T);
    T->super.ownsImage   = owns_image;
    T->super.aspectMask  = aspect_mask;
    T->super.isDedicated = is_dedicated;
    T->super.isAliasing  = desc->is_aliasing;
    T->super.canAlias    = can_alias_alloc || desc->is_aliasing;
    T->pVkImage          = pVkImage;
    if (pVkDeviceMemory) T->pVkDeviceMemory = pVkDeviceMemory;
    if (vmaAllocation) T->pVkAllocation = vmaAllocation;
    T->super.sampleCount       = desc->sample_count;
    T->super.width             = desc->width;
    T->super.height            = desc->height;
    T->super.depth             = desc->depth;
    T->super.mipLevels         = desc->mip_levels;
    T->super.isCube            = cubemapRequired;
    T->super.arraySizeMinusOne = arraySize - 1;
    T->super.format            = desc->format;
    T->super.isImported        = is_imported;
    T->super.uniqueId          = (unique_id == UINT64_MAX) ? D->spuer.nextTextureId++ : unique_id;
    // Set Texture Name
    //VkUtil_OptionalSetObjectName(D, (uint64_t)T->pVkImage, VK_OBJECT_TYPE_IMAGE, desc->name);
    // Start state
    if (Q && T->pVkImage != VK_NULL_HANDLE && T->pVkAllocation != VK_NULL_HANDLE)
    {
        GPUResetCommandPool(Q->pInnerCmdPool);
        GPUCmdBegin(Q->pInnerCmdBuffer);
        GPUTextureBarrier init_barrier = {
            .texture   = &T->super,
            .src_state = GPU_RESOURCE_STATE_UNDEFINED,
            .dst_state = desc->start_state
        };
        GPUResourceBarrierDescriptor init_barrier_d = {
            .texture_barriers       = &init_barrier,
            .texture_barriers_count = 1
        };
        GPUCmdResourceBarrier(Q->pInnerCmdBuffer, &init_barrier_d);
        GPUCmdEnd(Q->pInnerCmdBuffer);
        GPUQueueSubmitDescriptor barrier_submit = {
            .cmds         = &Q->pInnerCmdBuffer,
            .signal_fence = Q->pInnerFence,
            .cmds_count   = 1
        };
        GPUSubmitQueue(&Q->super, &barrier_submit);
        GPUWaitFences(&Q->pInnerFence, 1);
    }
    return &T->super;
}

void GPUFreeTexture_Vulkan(GPUTextureID texture)
{
    GPUDevice_Vulkan* D  = (GPUDevice_Vulkan*)texture->pDevice;
    GPUTexture_Vulkan* T = (GPUTexture_Vulkan*)texture;
    if (T->pVkImage != VK_NULL_HANDLE)
    {
        if (T->super.isImported)
        {
            D->mVkDeviceTable.vkDestroyImage(D->pDevice, T->pVkImage, GLOBAL_VkAllocationCallbacks);
            D->mVkDeviceTable.vkFreeMemory(D->pDevice, T->pVkDeviceMemory, GLOBAL_VkAllocationCallbacks);
        }
        else if (T->super.ownsImage)
        {
            const EGPUFormat fmt = (EGPUFormat)texture->format;
            (void)fmt;
            // TODO: Support planar formats
            const bool isSinglePlane = true;
            if (isSinglePlane)
            {
                /*cgpu_trace("Freeing texture allocation %p \n\t size: %dx%dx%d owns_image: %d imported: %d",
                           T->pVkImage, texture->width, texture->height, texture->depth, T->super.owns_image, T->super.is_imported);*/

                vmaDestroyImage(D->pVmaAllocator, T->pVkImage, T->pVkAllocation);
            }
            else
            {
                D->mVkDeviceTable.vkDestroyImage(D->pDevice, T->pVkImage, GLOBAL_VkAllocationCallbacks);
                D->mVkDeviceTable.vkFreeMemory(D->pDevice, T->pVkDeviceMemory, GLOBAL_VkAllocationCallbacks);
            }
        }
        else
        {
            /*cgpu_trace("Freeing texture %p \n\t size: %dx%dx%d owns_image: %d imported: %d",
                       T->pVkImage, texture->width, texture->height, texture->depth, T->super.owns_image, T->super.is_imported);*/

            D->mVkDeviceTable.vkDestroyImage(D->pDevice, T->pVkImage, GLOBAL_VkAllocationCallbacks);
        }
    }
    _aligned_free(T);
}

GPUShaderLibraryID GPUCreateShaderLibrary_Vulkan(GPUDeviceID pDevice, const GPUShaderLibraryDescriptor* pDesc)
{
    GPUDevice_Vulkan* pVkDevice = (GPUDevice_Vulkan*)pDevice;

    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = pDesc->codeSize;
    info.pCode    = pDesc->code;

    GPUShaderLibrary_Vulkan* pShader = (GPUShaderLibrary_Vulkan*)calloc(1, sizeof(GPUShaderLibrary_Vulkan));
    if (!pDesc->reflectionOnly)
    {
        if (pVkDevice->mVkDeviceTable.vkCreateShaderModule(pVkDevice->pDevice, &info, GLOBAL_VkAllocationCallbacks, &pShader->pShader) != VK_SUCCESS)
        {
            assert(0);
        }
    }
    VulkanUtil_InitializeShaderReflection(pDevice, pShader, pDesc);
    return &pShader->super;
}

void GPUFreeShaderLibrary_Vulkan(GPUShaderLibraryID pShader)
{
    GPUShaderLibrary_Vulkan* pVkShader = (GPUShaderLibrary_Vulkan*)pShader;
    GPUDevice_Vulkan* pVkDevice        = (GPUDevice_Vulkan*)pShader->pDevice;
    VulkanUtil_FreeShaderReflection(pVkShader);
    pVkDevice->mVkDeviceTable.vkDestroyShaderModule(pVkDevice->pDevice, pVkShader->pShader, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(pVkShader);
}