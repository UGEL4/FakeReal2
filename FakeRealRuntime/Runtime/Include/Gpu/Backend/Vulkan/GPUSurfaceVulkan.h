#pragma once

#include "Gpu/Backend/vulkan/GPUVulkan.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN64)
	GPUSurfaceID CreateSurfaceFromHWND_Vulkan(GPUInstanceID pInstance, HWND window);
#endif
    void FreeSruface_Vulkan(GPUInstanceID pInstance, GPUSurfaceID pSurface);

#ifdef __cplusplus
}
#endif