#pragma once

#include "Frontend/RenderGraph.h"
#include "Backend/TexturePool.h"
#include "Backend/TextureViewPool.h"
#include "Backend/BufferPool.h"
#include "Backend/BindTablePool.h"
#include <unordered_map>

#define RG_MAX_FRAME_IN_FLIGHT 3

namespace FakeReal
{
    namespace render_graph
    {
        class RenderPassNode;
        class RENDER_GRAPH_API RenderGraphFrameExecutor
        {
        public:
            friend class RenderGraphBackend;
            RenderGraphFrameExecutor() = default;

            void Initialize(GPUDeviceID gfxDevice, GPUQueueID gfxQueue);
            void Finalize();
            void ResetOnStart();
            void Commit(GPUQueueID gfxQueue, uint64_t frameIndex);

        private:
            GPUCommandPoolID m_pCommandPool = nullptr;
            GPUCommandBufferID m_pCmd       = nullptr;
            GPUFenceID m_pFence             = nullptr;
            uint64_t mExecFrame             = 0;
            std::unordered_map<GPURootSignatureID, BindTablePool*> mBindTablePools;
        };

        class RENDER_GRAPH_API RenderGraphBackend : public RenderGraph
        {
        public:
            RenderGraphBackend(const RenderGraphBuilder& builder);
            ~RenderGraphBackend() = default;

            virtual uint64_t Execute() override;
            virtual void Initialize() override;
            virtual void Finalize() override;

            void ExecuteRenderPass(RenderPassNode* pass, RenderGraphFrameExecutor& executor);
            void ExectuePresentPass(PresentPassNode* pass, RenderGraphFrameExecutor& executor);
            void ExectueCopyPass(CopyPassNode* pass, RenderGraphFrameExecutor& executor);

        private:
            void CalculateResourceBarriers(RenderGraphFrameExecutor& executor, PassNode* pass,
                                           std::vector<GPUTextureBarrier>& tex_barriers, std::vector<std::pair<TextureHandle, GPUTextureID>>& resolved_textures,
                                           std::vector<GPUBufferBarrier>& buffer_barriers, std::vector<std::pair<BufferHandle, GPUBufferID>>& resolved_buffers);
            GPUTextureID Resolve(RenderGraphFrameExecutor& executor, const TextureNode& texture);
            GPUBufferID Resolve(RenderGraphFrameExecutor& executor, const BufferNode& buffer);
            GPUBindTableID AllocateAndUpdatePassBindTable(RenderGraphFrameExecutor& executor, PassNode* pass, GPURootSignatureID root_sig);
            const GPUShaderResource* FindShaderResource(uint64_t nameHash, GPURootSignatureID rs, EGPUResourceType* type = nullptr) const;
            void DeallocaResources(PassNode* pass);
            uint64_t GetLatestFinishedFrame();

        private:
            GPUDeviceID m_pDevice;
            GPUQueueID m_pQueue;
            RenderGraphFrameExecutor mExecutors[RG_MAX_FRAME_IN_FLIGHT];
            TexturePool mTexturePool;
            TextureViewPool mTextureViewPool;
            BufferPool mBufferPool;
        };
    } // namespace render_graph
} // namespace FakeReal