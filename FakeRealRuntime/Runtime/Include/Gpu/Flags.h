#pragma once
#include <stdint.h>
#include "GpuConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum EGPUBackend
{
    GPUBackend_Vulkan = 0,
    GPUBackend_D3D12 = 1,
    GPUBackend_Count,
    GPUBackend_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUBackend;

typedef enum EGPUQueueType
{
    GPU_QUEUE_TYPE_GRAPHICS = 0,
    GPU_QUEUE_TYPE_COMPUTE  = 1,
    GPU_QUEUE_TYPE_TRANSFER = 2,
    GPU_QUEUE_TYPE_COUNT,
    GPU_QUEUE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUQueueType;

typedef enum EGPUFormat
{
    GPU_FORMAT_UNDEFINED,
    GPU_FORMAT_B8G8R8A8_UNORM,
    GPU_FORMAT_B8G8R8A8_SRGB,
    GPU_FORMAT_R8G8B8A8_UNORM,
    GPU_FORMAT_R8G8B8A8_SRGB,
    GPU_FORMAT_R16_UINT,
    GPU_FORMAT_R32_UINT,
    GPU_FORMAT_R32_SFLOAT,
    GPU_FORMAT_R32G32_SFLOAT,
    GPU_FORMAT_R32G32B32_SFLOAT,
    GPU_FORMAT_R32G32B32A32_SFLOAT,
    GPU_FORMAT_D16_UNORM_S8_UINT,
    GPU_FORMAT_D24_UNORM_S8_UINT,
    GPU_FORMAT_D32_SFLOAT_S8_UINT,
    GPU_FORMAT_D16_UNORM,
    GPU_FORMAT_D32_SFLOAT,
    CGPU_FORMAT_R16G16B16_SFLOAT,
    GPU_FORMAT_COUNT
} EGPUFormat;

typedef enum EGPUSampleCount
{
    GPU_SAMPLE_COUNT_1 = 1,
    GPU_SAMPLE_COUNT_2 = 2,
    GPU_SAMPLE_COUNT_4 = 4,
    GPU_SAMPLE_COUNT_8 = 8,
    GPU_SAMPLE_COUNT_16 = 16,
    GPU_SAMPLE_COUNT_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUSampleCount;

typedef enum EGPUTexutreViewUsage
{
    GPU_TVU_SRV = 0x01,
    GPU_TVU_RTV_DSV = 0x02,
    GPU_TVU_UAV = 0x04,
    GPU_TVU_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUTexutreViewUsage;

typedef enum EGPUTextureViewAspect
{
    GPU_TVA_COLOR = 0x01,
    GPU_TVA_DEPTH = 0x02,
    GPU_TVA_STENCIL = 0x04,
    GPU_TVA_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUTextureViewAspect;
typedef uint32_t GPUTextureViewAspects;

// Same Value As Vulkan Enumeration Bits.
typedef enum EGPUShaderStage
{
    GPU_SHADER_STAGE_NONE = 0,
    GPU_SHADER_STAGE_VERT = 0X00000001,
    GPU_SHADER_STAGE_TESC = 0X00000002,
    GPU_SHADER_STAGE_TESE = 0X00000004,
    GPU_SHADER_STAGE_GEOM = 0X00000008,
    GPU_SHADER_STAGE_FRAG = 0X00000010,
    GPU_SHADER_STAGE_COMPUTE = 0X00000020,
    GPU_SHADER_STAGE_RAYTRACING = 0X00000040,
    GPU_SHADER_STAGE_ALL_GRAPHICS =
        (uint32_t)GPU_SHADER_STAGE_VERT | (uint32_t)GPU_SHADER_STAGE_TESC |
        (uint32_t)GPU_SHADER_STAGE_TESE | (uint32_t)GPU_SHADER_STAGE_GEOM |
        (uint32_t)GPU_SHADER_STAGE_FRAG,
    GPU_SHADER_STAGE_HULL = GPU_SHADER_STAGE_TESC,
    GPU_SHADER_STAGE_DOMAIN = GPU_SHADER_STAGE_TESE,
    GPU_SHADER_STAGE_COUNT = 6,
    GPU_SHADER_STAGE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUShaderStage;
typedef uint32_t GPUShaderStages;

typedef enum EGPUVertexInputRate
{
    GPU_INPUT_RATE_VERTEX = 0,
    GPU_INPUT_RATE_INSTANCE = 1,
    GPU_INPUT_RATE_COUNT,
    GPU_INPUT_RATE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUVertexInputRate;

typedef enum EGPUPrimitiveTopology
{
    GPU_PRIM_TOPO_POINT_LIST = 0,
    GPU_PRIM_TOPO_LINE_LIST,
    GPU_PRIM_TOPO_LINE_STRIP,
    GPU_PRIM_TOPO_TRI_LIST,
    GPU_PRIM_TOPO_TRI_STRIP,
    GPU_PRIM_TOPO_PATCH_LIST,
    GPU_PRIM_TOPO_COUNT,
    GPU_PRIM_TOPO_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUPrimitiveTopology;

typedef enum EGPUCompareMode
{
    GPU_CMP_NEVER,
    GPU_CMP_LESS,
    GPU_CMP_EQUAL,
    GPU_CMP_LEQUAL,
    GPU_CMP_GREATER,
    GPU_CMP_NOTEQUAL,
    GPU_CMP_GEQUAL,
    GPU_CMP_ALWAYS,
    GPU_CMP_COUNT,
    GPU_CMP_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUCompareMode;

typedef enum EGPUStencilOp
{
    GPU_STENCIL_OP_KEEP,
    GPU_STENCIL_OP_SET_ZERO,
    GPU_STENCIL_OP_REPLACE,
    GPU_STENCIL_OP_INVERT,
    GPU_STENCIL_OP_INCR,
    GPU_STENCIL_OP_DECR,
    GPU_STENCIL_OP_INCR_SAT,
    GPU_STENCIL_OP_DECR_SAT,
    GPU_STENCIL_OP_COUNT,
    GPU_STENCIL_OP_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUStencilOp;

typedef enum EGPUCullMode
{
    GPU_CULL_MODE_NONE = 0,
    GPU_CULL_MODE_BACK,
    GPU_CULL_MODE_FRONT,
    GPU_CULL_MODE_BOTH,
    GPU_CULL_MODE_COUNT,
    GPU_CULL_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUCullMode;

typedef enum EGPUFrontFace
{
    GPU_FRONT_FACE_CCW = 0,
    GPU_FRONT_FACE_CW,
    GPU_FRONT_FACE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUFrontFace;

typedef enum EGPUFillMode
{
    GPU_FILL_MODE_SOLID,
    GPU_FILL_MODE_WIREFRAME,
    GPU_FILL_MODE_COUNT,
    GPU_FILL_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUFillMode;

typedef enum EGPUBlendConstant
{
    GPU_BLEND_CONST_ZERO = 0,
    GPU_BLEND_CONST_ONE,
    GPU_BLEND_CONST_SRC_COLOR,
    GPU_BLEND_CONST_ONE_MINUS_SRC_COLOR,
    GPU_BLEND_CONST_DST_COLOR,
    GPU_BLEND_CONST_ONE_MINUS_DST_COLOR,
    GPU_BLEND_CONST_SRC_ALPHA,
    GPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA,
    GPU_BLEND_CONST_DST_ALPHA,
    GPU_BLEND_CONST_ONE_MINUS_DST_ALPHA,
    GPU_BLEND_CONST_SRC_ALPHA_SATURATE,
    GPU_BLEND_CONST_BLEND_FACTOR,
    GPU_BLEND_CONST_ONE_MINUS_BLEND_FACTOR,
    GPU_BLEND_CONST_COUNT,
    GPU_BLEND_CONST_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUBlendConstant;

typedef enum EGPUBlendMode
{
    GPU_BLEND_MODE_ADD,
    GPU_BLEND_MODE_SUBTRACT,
    GPU_BLEND_MODE_REVERSE_SUBTRACT,
    GPU_BLEND_MODE_MIN,
    GPU_BLEND_MODE_MAX,
    GPU_BLEND_MODE_COUNT,
    GPU_BLEND_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUBlendMode;

typedef enum EGPULoadAction
{
    GPU_LOAD_ACTION_DONTCARE,
    GPU_LOAD_ACTION_LOAD,
    GPU_LOAD_ACTION_CLEAR,
    GPU_LOAD_ACTION_COUNT,
    GPU_LOAD_ACTION_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPULoadAction;

typedef enum EGPUStoreAction
{
    GPU_STORE_ACTION_STORE,
    GPU_STORE_ACTION_DISCARD,
    GPU_STORE_ACTION_COUNT,
    GPU_STORE_ACTION_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUStoreAction;

typedef enum EGPUPipelineType
{
    GPU_PIPELINE_TYPE_NONE = 0,
    GPU_PIPELINE_TYPE_COMPUTE,
    GPU_PIPELINE_TYPE_GRAPHICS,
    GPU_PIPELINE_TYPE_RAYTRACING,
    GPU_PIPELINE_TYPE_COUNT,
    GPU_PIPELINE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUPipelineType;

typedef enum EGPUFenceStatus
{
    GPU_FENCE_STATUS_COMPLETE = 0,
    GPU_FENCE_STATUS_INCOMPLETE,
    GPU_FENCE_STATUS_NOTSUBMITTED,
    GPU_FENCE_STATUS_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUFenceStatus;

typedef enum EGPUMemoryUsage
{
    /// No intended memory usage specified.
    GPU_MEM_USAGE_UNKNOWN = 0,
    /// Memory will be used on device only, no need to be mapped on host.
    GPU_MEM_USAGE_GPU_ONLY = 1,
    /// Memory will be mapped on host. Could be used for transfer to device.
    GPU_MEM_USAGE_CPU_ONLY = 2,
    /// Memory will be used for frequent (dynamic) updates from host and reads on
    /// device. Memory location (heap) is unsure.
    GPU_MEM_USAGE_CPU_TO_GPU = 3,
    /// Memory will be used for writing on device and readback on host.
    /// Memory location (heap) is unsure.
    GPU_MEM_USAGE_GPU_TO_CPU = 4,
    GPU_MEM_USAGE_COUNT,
    GPU_MEM_USAGE_MAX_ENUM = 0x7FFFFFFF
} EGPUMemoryUsage;

typedef enum EGPUBufferCreationFlag
{
    /// Default flag (Buffer will use aliased memory, buffer will not be cpu
    /// accessible until mapBuffer is called)
    GPU_BCF_NONE = 0,
    /// Buffer will allocate its own memory (COMMITTED resource)
    GPU_BCF_OWN_MEMORY_BIT = 0x02,
    /// Buffer will be persistently mapped
    GPU_BCF_PERSISTENT_MAP_BIT = 0x04,
    /// Use ESRAM to store this buffer
    GPU_BCF_ESRAM = 0x08,
    /// Flag to specify not to allocate descriptors for the resource
    GPU_BCF_NO_DESCRIPTOR_VIEW_CREATION = 0x10,
    /// Flag to specify to create GPUOnly buffer as Host visible
    GPU_BCF_HOST_VISIBLE = 0x20,
    GPU_BCF_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUBufferCreationFlag;
typedef uint32_t GPUBufferCreationFlags;

typedef enum EGPUFilterType
{
    GPU_FILTER_TYPE_NEAREST = 0,
    GPU_FILTER_TYPE_LINEAR,
    GPU_FILTER_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} CGPUFilterType;

typedef enum EGPUAddressMode
{
    GPU_ADDRESS_MODE_MIRROR,
    GPU_ADDRESS_MODE_REPEAT,
    GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
    GPU_ADDRESS_MODE_CLAMP_TO_BORDER,
    GPU_ADDRESS_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUAddressMode;

typedef enum EGPUMipMapMode
{
    GPU_MIPMAP_MODE_NEAREST = 0,
    GPU_MIPMAP_MODE_LINEAR,
    GPU_MIPMAP_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUMipMapMode;

typedef enum EGPUResourceState
{
    GPU_RESOURCE_STATE_UNDEFINED = 0,
    GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
    GPU_RESOURCE_STATE_INDEX_BUFFER = 0x2,
    GPU_RESOURCE_STATE_RENDER_TARGET = 0x4,
    GPU_RESOURCE_STATE_UNORDERED_ACCESS = 0x8,
    GPU_RESOURCE_STATE_DEPTH_WRITE = 0x10,
    GPU_RESOURCE_STATE_DEPTH_READ = 0x20,
    GPU_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE = 0x40,
    GPU_RESOURCE_STATE_PIXEL_SHADER_RESOURCE = 0x80,
    GPU_RESOURCE_STATE_SHADER_RESOURCE = 0x40 | 0x80,
    GPU_RESOURCE_STATE_STREAM_OUT = 0x100,
    GPU_RESOURCE_STATE_INDIRECT_ARGUMENT = 0x200,
    GPU_RESOURCE_STATE_COPY_DEST = 0x400,
    GPU_RESOURCE_STATE_COPY_SOURCE = 0x800,
    GPU_RESOURCE_STATE_GENERIC_READ =
        (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
    GPU_RESOURCE_STATE_PRESENT = 0x1000,
    GPU_RESOURCE_STATE_COMMON = 0x2000,
    GPU_RESOURCE_STATE_ACCELERATION_STRUCTURE = 0x4000,
    GPU_RESOURCE_STATE_SHADING_RATE_SOURCE = 0x8000,
    GPU_RESOURCE_STATE_RESOLVE_DEST = 0x10000,
    GPU_RESOURCE_STATE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUResourceState;
typedef uint32_t GPUResourceStates;

typedef enum EGPUResourceType
{
    GPU_RESOURCE_TYPE_NONE = 0,
    GPU_RESOURCE_TYPE_SAMPLER = 0x00000001,
    // SRV Read only texture
    GPU_RESOURCE_TYPE_TEXTURE = (GPU_RESOURCE_TYPE_SAMPLER << 1),
    /// RTV Texture
    GPU_RESOURCE_TYPE_RENDER_TARGET = (GPU_RESOURCE_TYPE_TEXTURE << 1),
    /// DSV Texture
    GPU_RESOURCE_TYPE_DEPTH_STENCIL = (GPU_RESOURCE_TYPE_RENDER_TARGET << 1),
    /// UAV Texture
    GPU_RESOURCE_TYPE_RW_TEXTURE = (GPU_RESOURCE_TYPE_DEPTH_STENCIL << 1),
    // SRV Read only buffer
    GPU_RESOURCE_TYPE_BUFFER = (GPU_RESOURCE_TYPE_RW_TEXTURE << 1),
    GPU_RESOURCE_TYPE_BUFFER_RAW =
        (GPU_RESOURCE_TYPE_BUFFER | (GPU_RESOURCE_TYPE_BUFFER << 1)),
    /// UAV Buffer
    GPU_RESOURCE_TYPE_RW_BUFFER = (GPU_RESOURCE_TYPE_BUFFER << 2),
    GPU_RESOURCE_TYPE_RW_BUFFER_RAW =
        (GPU_RESOURCE_TYPE_RW_BUFFER | (GPU_RESOURCE_TYPE_RW_BUFFER << 1)),
    /// CBV Uniform buffer
    GPU_RESOURCE_TYPE_UNIFORM_BUFFER = (GPU_RESOURCE_TYPE_RW_BUFFER << 2),
    /// Push constant / Root constant
    GPU_RESOURCE_TYPE_PUSH_CONSTANT = (GPU_RESOURCE_TYPE_UNIFORM_BUFFER << 1),
    /// IA
    GPU_RESOURCE_TYPE_VERTEX_BUFFER = (GPU_RESOURCE_TYPE_PUSH_CONSTANT << 1),
    GPU_RESOURCE_TYPE_INDEX_BUFFER = (GPU_RESOURCE_TYPE_VERTEX_BUFFER << 1),
    GPU_RESOURCE_TYPE_INDIRECT_BUFFER = (GPU_RESOURCE_TYPE_INDEX_BUFFER << 1),
    /// Cubemap SRV
    GPU_RESOURCE_TYPE_TEXTURE_CUBE =
        (GPU_RESOURCE_TYPE_TEXTURE | (GPU_RESOURCE_TYPE_INDIRECT_BUFFER << 1)),
    /// RTV / DSV per mip slice
    GPU_RESOURCE_TYPE_RENDER_TARGET_MIP_SLICES =
        (GPU_RESOURCE_TYPE_INDIRECT_BUFFER << 2),
    /// RTV / DSV per array slice
    GPU_RESOURCE_TYPE_RENDER_TARGET_ARRAY_SLICES =
        (GPU_RESOURCE_TYPE_RENDER_TARGET_MIP_SLICES << 1),
    /// RTV / DSV per depth slice
    GPU_RESOURCE_TYPE_RENDER_TARGET_DEPTH_SLICES =
        (GPU_RESOURCE_TYPE_RENDER_TARGET_ARRAY_SLICES << 1),
    GPU_RESOURCE_TYPE_RAY_TRACING =
        (GPU_RESOURCE_TYPE_RENDER_TARGET_DEPTH_SLICES << 1),
#if defined(GPU_USE_VULKAN)
    /// Subpass input (descriptor type only available in Vulkan)
    GPU_RESOURCE_TYPE_INPUT_ATTACHMENT = (GPU_RESOURCE_TYPE_RAY_TRACING << 1),
    GPU_RESOURCE_TYPE_TEXEL_BUFFER = (GPU_RESOURCE_TYPE_INPUT_ATTACHMENT << 1),
    GPU_RESOURCE_TYPE_RW_TEXEL_BUFFER = (GPU_RESOURCE_TYPE_TEXEL_BUFFER << 1),
    GPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER =
      (GPU_RESOURCE_TYPE_RW_TEXEL_BUFFER << 1),
#endif
    GPU_RESOURCE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUResourceType;
typedef uint32_t GPUResourceTypes;

typedef enum EGPUTextureDimension
{
    GPU_TEX_DIMENSION_1D,
    GPU_TEX_DIMENSION_2D,
    GPU_TEX_DIMENSION_2DMS,
    GPU_TEX_DIMENSION_3D,
    GPU_TEX_DIMENSION_CUBE,
    GPU_TEX_DIMENSION_1D_ARRAY,
    GPU_TEX_DIMENSION_2D_ARRAY,
    GPU_TEX_DIMENSION_2DMS_ARRAY,
    GPU_TEX_DIMENSION_CUBE_ARRAY,
    GPU_TEX_DIMENSION_COUNT,
    GPU_TEX_DIMENSION_UNDEFINED,
    GPU_TEX_DIMENSION_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUTextureDimension;

typedef enum EGPUTextureCreationFlag
{
    /// Default flag (Texture will use default allocation strategy decided by the
    /// api specific allocator)
    GPU_TCF_NONE = 0,
    /// Texture will allocate its own memory (COMMITTED resource)
    /// Note that this flag is not restricted Commited/Dedicated Allocation
    /// Actually VMA/D3D12MA allocate dedicated memories with ALLOW_ALIAS flag
    /// with specific loacl heaps If the texture needs to be restricted
    /// Committed/Dedicated(thus you want to keep its priority high) Toogle
    /// is_dedicated flag in GPUTextureDescriptor
    GPU_TCF_OWN_MEMORY_BIT = 0x01,
    /// Texture will be allocated in memory which can be shared among multiple
    /// processes
    GPU_TCF_EXPORT_BIT = 0x02,
    /// Texture will be allocated in memory which can be shared among multiple
    /// gpus
    GPU_TCF_EXPORT_ADAPTER_BIT = 0x04,
    /// Use on-tile memory to store this texture
    GPU_TCF_ON_TILE = 0x08,
    /// Prevent compression meta data from generating (XBox)
    GPU_TCF_NO_COMPRESSION = 0x10,
    /// Force 2D instead of automatically determining dimension based on width,
    /// height, depth
    GPU_TCF_FORCE_2D = 0x20,
    /// Force 3D instead of automatically determining dimension based on width,
    /// height, depth
    GPU_TCF_FORCE_3D = 0x40,
    /// Display target
    GPU_TCF_ALLOW_DISPLAY_TARGET = 0x80,
    /// Create a normal map texture
    GPU_TCF_NORMAL_MAP = 0x100,
    /// Fragment mask
    GPU_TCF_FRAG_MASK = 0x200,
    ///
    GPU_TCF_USABLE_MAX   = 0x40000,
    GPU_TCF_MAX_ENUM_BIT = 0x7FFFFFFF
} EGPUTextureCreationFlag;
typedef uint32_t GPUTextureCreationFlags;

typedef union GPUClearValue
{
    struct
    {
        float r;
        float g;
        float b;
        float a;
    };
    struct
    {
        float depth;
        uint32_t stencil;
    };
} GPUClearValue;

static inline bool FormatUtil_IsDepthStencilFormat(EGPUFormat const fmt)
{
    switch (fmt)
    {
        case GPU_FORMAT_D24_UNORM_S8_UINT:
        case GPU_FORMAT_D32_SFLOAT_S8_UINT:
        case GPU_FORMAT_D16_UNORM_S8_UINT:
        case GPU_FORMAT_D16_UNORM:
        case GPU_FORMAT_D32_SFLOAT:
            return true;
        default:
            return false;
    }
    return false;
}

static inline bool FormatUtil_IsDepthOnlyFormat(EGPUFormat const fmt)
{
    switch (fmt)
    {
        case GPU_FORMAT_D16_UNORM:
        case GPU_FORMAT_D32_SFLOAT:
            return true;
        default:
            return false;
    }
    return false;
}

#ifdef __cplusplus
}
#endif