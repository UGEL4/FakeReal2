#pragma once
#include <functional>
#include "DependencyGraph.h"
#include "BaseTypes.h"
#include <vector>

namespace FakeReal
{
    namespace render_graph
    {
        class PassNode;
        class RenderPassNode;
        class ResourceNode;
        class TextureEdge;
        class PresentPassNode;
        class BufferNode;
        class CopyPassNode;

        class RENDER_GRAPH_API RenderGraph
        {
        public:
            class RENDER_GRAPH_API RenderGraphBuilder
            {
            public:
                friend class RenderGraphBackend;
                RenderGraphBuilder& WithDevice(GPUDeviceID device);
                RenderGraphBuilder& WithGFXQueue(GPUQueueID queue);

            private:
                GPUDeviceID m_pDevice;
                GPUQueueID m_pQueue;
            };
            using RenderGraphSetupFunc = std::function<void(RenderGraphBuilder&)>;
            static RenderGraph* Create(const RenderGraphSetupFunc& setup);
            static void Destroy(RenderGraph* graph);

            void Compile();
            virtual uint64_t Execute();
            virtual void Initialize();
            virtual void Finalize();

            EGPUResourceState GetLastestState(const TextureNode* texture, const PassNode* pending_pass);
            uint32_t ForeachWriterPass(const TextureHandle handle, const std::function<void(TextureNode* texture, PassNode* pass, RenderGraphEdge* edge)>&);
            uint32_t ForeachReaderPass(const TextureHandle handle, const std::function<void(TextureNode* texture, PassNode* pass, RenderGraphEdge* edge)>&);

            class RENDER_GRAPH_API RenderPassBuilder
            {
            public:
                friend class RenderGraph;

            protected:
                RenderPassBuilder(RenderGraph& graph, RenderPassNode& node);

            public:
                RenderPassBuilder& SetName(const char* name);
                RenderPassBuilder& Write(uint32_t mrtIndex, TextureRTVHandle handle, EGPULoadAction load, EGPUStoreAction store);
                RenderPassBuilder& Read(const char* name, TextureSRVHandle handle);
                RenderPassBuilder& SetRootSignature(GPURootSignatureID rs);
                RenderPassBuilder& SetPipeline(GPURenderPipelineID pipeline);
                RenderPassBuilder& SetDepthStencil(TextureDSVHandle handle, EGPULoadAction depthLoad, EGPUStoreAction depthStore, EGPULoadAction stencilLoad, EGPUStoreAction stencilStore);
                // pipeline
            private:
                RenderGraph& mGraph;
                RenderPassNode& mPassNode;
            };
            using RenderPassSetupFunc = std::function<void(RenderGraph&, RenderPassBuilder&)>;
            PassHandle AddRenderPass(const RenderPassSetupFunc& setup, const RenderPassExecuteFunction& execute);

            class RENDER_GRAPH_API TextureBuilder
            {
            public:
                friend class RenderGraph;

            protected:
                TextureBuilder(RenderGraph& graph, TextureNode& textureNode);

            public:
                TextureBuilder& Import(GPUTextureID texture, EGPUResourceState initedState);
                TextureBuilder& SetName(const char* name);
                TextureBuilder& AllowRenderTarget();
                // todo: all setters
            private:
                RenderGraph& mGraph;
                TextureNode& mTextureNode;
            };
            using TextureSetupFunc = std::function<void(RenderGraph&, TextureBuilder&)>;
            TextureHandle CreateTexture(const TextureSetupFunc& setup);

            class RENDER_GRAPH_API PresentPassBuilder
            {
            public:
                friend class RenderGraph;
                PresentPassBuilder& SetName(const char* name);
                PresentPassBuilder& Swapchain(GPUSwapchainID swapchain, uint32_t index);
                PresentPassBuilder& Texture(TextureHandle handle, bool isBackBuffer = true);

            protected:
                PresentPassBuilder(RenderGraph& graph, PresentPassNode& node);

            private:
                RenderGraph& mGraph;
                PresentPassNode& mPassNode;
            };
            using PresentPassSetupFunc = std::function<void(RenderGraph&, PresentPassBuilder&)>;
            PassHandle AddPresentPass(const PresentPassSetupFunc& setup);

            class RENDER_GRAPH_API BufferBuilder
            {
            public:
                friend class RenderGraph;
                BufferBuilder& SetName(const char* name);
                BufferBuilder& Import(GPUBufferID buffer, EGPUResourceState initState);
                BufferBuilder& OwnsMemory();
                // BufferBuilder& structured(uint64_t first_element, uint64_t element_count, uint64_t element_stride) SKR_NOEXCEPT;
                BufferBuilder& Size(uint64_t size);
                BufferBuilder& WithFlags(GPUBufferCreationFlags flags);
                BufferBuilder& MemoryUsage(EGPUMemoryUsage mem_usage);
                BufferBuilder& AllowShaderReadWrite();
                BufferBuilder& AllowShaderRead();
                BufferBuilder& AsUploadBuffer();
                BufferBuilder& AsVertexBuffer();
                BufferBuilder& AsIndexBuffer();
                BufferBuilder& AsUniformBuffer();
                BufferBuilder& PreferOnDevice();
                BufferBuilder& PreferOnHost();

            protected:
                BufferBuilder(RenderGraph& graph, BufferNode& node);
                RenderGraph& mGraph;
                BufferNode& mNode;
            };
            using BufferSetupFunc = std::function<void(RenderGraph&, BufferBuilder&)>;
            BufferHandle CreateBuffer(const BufferSetupFunc& setup);
            EGPUResourceState GetLastestState(const BufferNode* buffer, const PassNode* pending_pass);
            // BufferHandle GetBufferHandle(const char* name);

            uint32_t ForeachWriterPass(const BufferHandle handle, const std::function<void(BufferNode*, PassNode*, RenderGraphEdge*)>&);
            uint32_t ForeachReaderPass(const BufferHandle handle, const std::function<void(BufferNode*, PassNode*, RenderGraphEdge*)>&);

            class RENDER_GRAPH_API CopyPassBuilder
            {
            public:
                friend class RenderGraph;

            protected:
                CopyPassBuilder(RenderGraph& graph, CopyPassNode& node);

            public:
                CopyPassBuilder& SetName(const char* name);
                CopyPassBuilder& CanBeLone();
                CopyPassBuilder& TextureToTexture(TextureSubresourceHandle src, TextureSubresourceHandle dst, EGPUResourceState dstState = GPU_RESOURCE_STATE_COPY_DEST);
                CopyPassBuilder& BufferToBuffer(BufferRangeHandle src, BufferRangeHandle dst, EGPUResourceState dstState = GPU_RESOURCE_STATE_COPY_DEST);
                CopyPassBuilder& BufferToTexture(BufferRangeHandle src, TextureSubresourceHandle dst, EGPUResourceState dstState = GPU_RESOURCE_STATE_COPY_DEST);
                CopyPassBuilder& FromBuffer(BufferRangeHandle src);

            private:
                RenderGraph& mGraph;
                CopyPassNode& mPassNode;
            };
            using CopyPassSetupFunc = std::function<void(RenderGraph&, CopyPassBuilder&)>;
            PassHandle AddCopyPass(const CopyPassSetupFunc& setup, const CopyPassExecuteFunction& execute);

            BufferNode* Resolve(BufferHandle hdl);
            TextureNode* Resolve(TextureHandle hdl);
            PassNode* Resolve(PassHandle hdl);

            RenderGraph();
            virtual ~RenderGraph() = default;

        protected:
            DependencyGraph* m_pGraph                = nullptr;
            struct NodeAndEdgeFactory* m_pNAEFactory = nullptr;
            std::vector<PassNode*> mPasses;
            std::vector<ResourceNode*> mResources;
            std::vector<PassNode*> mCulledPasses;
            std::vector<ResourceNode*> mCulledResources;
            uint32_t mFrameIndex = 0;
        };

        using RenderGraphBuilder = RenderGraph::RenderGraphBuilder;
        using RenderPassBuilder  = RenderGraph::RenderPassBuilder;
        using TextureBuilder     = RenderGraph::TextureBuilder;
        using PresentPassBuilder = RenderGraph::PresentPassBuilder;
        using BufferBuilder      = RenderGraph::BufferBuilder;
        using CopyPassBuilder    = RenderGraph::CopyPassBuilder;
    } // namespace render_graph
} // namespace FakeReal