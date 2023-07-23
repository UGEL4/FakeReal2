#pragma once

#include "Config.h"
#include "DependencyGraph.h"
#include "Gpu/GpuApi.h"
#include "Gpu/GpuBindTable.h"
#include <string>
#include <functional>
#include <span>

namespace FakeReal
{
    namespace render_graph
    {
        typedef uint64_t _handle_t;
        enum class EObjectType : uint8_t
        {
            Pass,
            Texture,
            Buffer,
            Count
        };

        enum class EPassType : uint8_t
        {
            None,
            Render,
            Compute,
            Copy,
            Present,
            Count
        };

        enum class ERelationshipType : uint8_t
        {
            TextureRead,      // SRV
            TextureWrite,     // RTV/DSV
            TextureReadWrite, // UAV
            PipelineBuffer,   // VB/IB...
            BufferRead,       // CBV
            BufferReadWrite,  // UAV
            Count
        };

        template <EObjectType type>
        struct RENDER_GRAPH_API ObjectHandle
        {
            ObjectHandle(_handle_t handle)
                : mHandle(handle)
            {
            }

            inline operator _handle_t() const
            {
                return mHandle;
            }

        private:
            _handle_t mHandle;
        };

        using PassHandle = ObjectHandle<EObjectType::Pass>;

        template <>
        struct RENDER_GRAPH_API ObjectHandle<EObjectType::Texture>
        {
            struct RENDER_GRAPH_API ShaderWriteHandle
            {
                friend struct ObjectHandle<EObjectType::Texture>;
                friend class RenderGraph;
                friend class TextureWriteEdge;
                ShaderWriteHandle(const _handle_t _this)
                    : mThis(_this)
                {
                }
                inline operator ObjectHandle<EObjectType::Texture>() const
                {
                    return ObjectHandle<EObjectType::Texture>(mThis);
                }
                ShaderWriteHandle WriteMip(uint32_t level) const
                {
                    ShaderWriteHandle handle = *this;
                    handle.mMipBase          = level;
                    return handle;
                }
                ShaderWriteHandle WriteArray(uint32_t base, uint32_t count) const
                {
                    ShaderWriteHandle handle = *this;
                    handle.mArrayBase        = base;
                    handle.mArrayCount       = count;
                    return handle;
                }

            private:
                _handle_t mThis;
                uint32_t mMipBase    = 0;
                uint32_t mArrayBase  = 0;
                uint32_t mArrayCount = 1;
            };
            inline operator ShaderWriteHandle() const
            {
                return ShaderWriteHandle(mHandle);
            }

            struct RENDER_GRAPH_API DepthStencilHandle : public ShaderWriteHandle
            {
                friend struct ObjectHandle<EObjectType::Texture>;
                friend class RenderGraph;
                friend class TextureWriteEdge;
                DepthStencilHandle(const _handle_t _this)
                    : ShaderWriteHandle(_this)
                {
                }
                DepthStencilHandle ClearDepth(float depth) const
                {
                    DepthStencilHandle handle = *this;
                    handle.mClearDepth        = depth;
                    return handle;
                }

            private:
                float mClearDepth = 0.f;
            };
            inline operator DepthStencilHandle() const
            {
                return DepthStencilHandle(mHandle);
            }

            struct RENDER_GRAPH_API ShaderReadHandle
            {
                friend struct ObjectHandle<EObjectType::Texture>;
                friend class RenderGraph;
                friend class TextureReadEdge;
                ShaderReadHandle(const _handle_t _this, uint32_t mipBase = 0, uint32_t mipCount = 1, uint32_t arrayBase = 0, uint32_t arrayCount = 1)
                    : mThis(_this)
                    , mMipBase(mipBase)
                    , mMipCount(mipCount)
                    , mArrayBase(arrayBase)
                    , mArrayCount(arrayCount)
                {
                }
                inline operator ObjectHandle<EObjectType::Texture>() const
                {
                    return ObjectHandle<EObjectType::Texture>(mThis);
                }
                ShaderReadHandle ReadMip(uint32_t base, uint32_t count) const
                {
                    ShaderReadHandle handle = *this;
                    handle.mMipBase         = base;
                    handle.mMipCount        = count;
                    return handle;
                }

                ShaderReadHandle ReadArray(uint32_t base, uint32_t count) const
                {
                    ShaderReadHandle handle = *this;
                    handle.mArrayBase       = base;
                    handle.mArrayCount      = count;
                    return handle;
                }

                ShaderReadHandle ReadDimension(EGPUTextureDimension dim) const
                {
                    ShaderReadHandle handle = *this;
                    handle.mDim             = dim;
                    return handle;
                }

            private:
                _handle_t mThis;
                uint32_t mMipBase         = 0;
                uint32_t mMipCount        = 1;
                uint32_t mArrayBase       = 0;
                uint32_t mArrayCount      = 1;
                EGPUTextureDimension mDim = GPU_TEX_DIMENSION_2D;
            };
            inline operator ShaderReadHandle() const
            {
                return ShaderReadHandle(mHandle);
            }

            struct RENDER_GRAPH_API SubresourceHandle
            {
                friend struct ObjectHandle<EObjectType::Texture>;
                friend class RenderGraph;
                friend class RenderGraphBackend;
                inline operator ObjectHandle<EObjectType::Texture>() const
                {
                    return ObjectHandle<EObjectType::Texture>(mThis);
                }

                SubresourceHandle(const _handle_t _this)
                    : mThis(_this)
                {
                }

            protected:
                _handle_t mThis;
                uint32_t mip_level            = 0;
                uint32_t array_base           = 0;
                uint32_t array_count          = 1;
                GPUTextureViewAspects aspects = GPU_TVA_COLOR;
            };
            inline operator SubresourceHandle() const
            {
                return SubresourceHandle(mHandle);
            }

            inline operator _handle_t() const
            {
                return mHandle;
            }
            ObjectHandle()
            {
            }

            friend class TextureNode;
            friend class RenderGraph;
            friend class TextureRenderEdge;
            friend class TextureReadEdge;

        protected:
            ObjectHandle(_handle_t handle)
                : mHandle(handle)
            {
            }

        private:
            _handle_t mHandle;
        };
        using TextureHandle            = ObjectHandle<EObjectType::Texture>;
        using TextureRTVHandle         = TextureHandle::ShaderWriteHandle;
        using TextureSRVHandle         = TextureHandle::ShaderReadHandle;
        using TextureDSVHandle         = TextureHandle::DepthStencilHandle;
        using TextureSubresourceHandle = TextureHandle::SubresourceHandle;

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        template <>
        struct RENDER_GRAPH_API ObjectHandle<EObjectType::Buffer>
        {
            struct RENDER_GRAPH_API ShaderReadHandle
            {
                friend struct ObjectHandle<EObjectType::Buffer>;
                friend class RenderGraph;
                friend class BufferReadEdge;
                ShaderReadHandle(const _handle_t _this)
                    : mThis(_this)
                {
                }
                inline operator ObjectHandle<EObjectType::Buffer>() const
                {
                    return ObjectHandle<EObjectType::Buffer>(mThis);
                }

            private:
                _handle_t mThis;
            };
            inline operator ShaderReadHandle() const
            {
                return ShaderReadHandle(mHandle);
            }

            struct RENDER_GRAPH_API ShaderReadWriteHandle
            {
                friend struct ObjectHandle<EObjectType::Buffer>;
                friend class RenderGraph;
                friend class BufferReadEdge;
                ShaderReadWriteHandle(const _handle_t _this)
                    : mThis(_this)
                {
                }
                inline operator ObjectHandle<EObjectType::Buffer>() const
                {
                    return ObjectHandle<EObjectType::Buffer>(mThis);
                }

            private:
                _handle_t mThis;
            };
            inline operator ShaderReadWriteHandle() const
            {
                return ShaderReadWriteHandle(mHandle);
            }

            struct RENDER_GRAPH_API BufferRangeHandle
            {
                friend struct ObjectHandle<EObjectType::Buffer>;
                friend class RenderGraph;
                friend class RenderGraphBackend;
                friend class BufferReadEdge;

                BufferRangeHandle(const _handle_t _this, uint64_t from, uint64_t to)
                    : mThis(_this)
                    , mFrom(from)
                    , mTo(to)
                {
                }
                inline operator ObjectHandle<EObjectType::Buffer>() const
                {
                    return ObjectHandle<EObjectType::Buffer>(mThis);
                }

            private:
                _handle_t mThis;
                uint64_t mFrom;
                uint64_t mTo;
            };
            inline BufferRangeHandle BufferRange(uint64_t from, uint64_t to) const
            {
                return BufferRangeHandle(mHandle, from, to);
            }

            inline operator _handle_t() const
            {
                return mHandle;
            }
            ObjectHandle()
            {
            }

            friend class BufferNode;
            friend class RenderGraph;
            friend class BufferReadEdge;
            friend class BufferReadWriteEdge;

        protected:
            ObjectHandle(_handle_t handle)
                : mHandle(handle)
            {
            }

        private:
            _handle_t mHandle = UINT64_MAX;
        };

        using BufferHandle      = ObjectHandle<EObjectType::Buffer>;
        using BufferCBVHandle   = BufferHandle::ShaderReadHandle;
        using BufferUAVHandle   = BufferHandle::ShaderReadWriteHandle;
        using BufferRangeHandle = BufferHandle::BufferRangeHandle;

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        struct RenderGraphNode : public DependencyGraphNode
        {
            RenderGraphNode(EObjectType type)
                : type(type)
            {
            }
            void SetName(const char* name)
            {
                mName = name;
            }
            const char* GetName() const
            {
                return mName.c_str();
            }
            const EObjectType type;
            const uint32_t pooled_size = 0;

        protected:
            std::string mName = "";
        };

        struct RenderGraphEdge : public DependencyGraphEdge
        {
            RenderGraphEdge(ERelationshipType type)
                : type(type)
            {
            }

            const ERelationshipType type;
            const uint32_t pooled_size = 0;
        };

        class PassNode;
        class RenderGraphBackend;
        class RenderGraph;
        struct RENDER_GRAPH_API PassContext
        {
            friend class RenderGraphBackend;
            PassNode* m_pPassNode;
            RenderGraphBackend* m_pGraph;
            GPUCommandBufferID m_pCmd;
            std::span<std::pair<BufferHandle, GPUBufferID>> mResolvedBuffers;
            std::span<std::pair<TextureHandle, GPUTextureID>> mResolvedTextures;

            GPUBufferID Resolve(BufferHandle buffer_handle) const
            {
                for (auto iter : mResolvedBuffers)
                {
                    if (iter.first == buffer_handle) return iter.second;
                }
                return nullptr;
            }

            GPUTextureID Resolve(TextureHandle tex_handle) const
            {
                for (auto iter : mResolvedTextures)
                {
                    if (iter.first == tex_handle) return iter.second;
                }
                return nullptr;
            }
        };

        struct RENDER_GRAPH_API RenderPassContext : public PassContext
        {
            GPURenderPassEncoderID m_pEncoder;
            const GPUBindTable* m_pBindTable;
        };

        struct RENDER_GRAPH_API CopyPassContext : public PassContext
        {
        };

        using RenderPassExecuteFunction = std::function<void(RenderGraph&, RenderPassContext&)>;
        using CopyPassExecuteFunction   = std::function<void(RenderGraph&, CopyPassContext&)>;
    } // namespace render_graph
} // namespace FakeReal