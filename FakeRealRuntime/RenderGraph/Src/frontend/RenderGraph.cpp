#include "Frontend/RenderGraph.h"
#include "Backend/RenderGraphBackend.h"
#include "Frontend/PassNode.h"
#include "Frontend/ResourceNode.h"
#include "Frontend/ResourceEdge.h"
#include "Frontend/NodeAndEdgeFactory.h"
#include "Platform/Memory.h"

namespace FakeReal
{
    namespace render_graph
    {
        ///////////RenderGraphBuilder//////////////////
        RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::WithDevice(GPUDeviceID device)
        {
            m_pDevice = device;
            return *this;
        }

        RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::WithGFXQueue(GPUQueueID queue)
        {
            m_pQueue = queue;
            return *this;
        }
        ///////////RenderGraphBuilder//////////////////

        RenderGraph* RenderGraph::Create(const RenderGraphSetupFunc& setup)
        {
            RenderGraph* graph = nullptr;

            RenderGraphBuilder builder = {};
            setup(builder);

            graph = New<RenderGraphBackend>(builder);
            graph->Initialize();

            return graph;
        }

        void RenderGraph::Destroy(RenderGraph* graph)
        {
            if (graph)
            {
                graph->Finalize();
                Delete(graph);
            }
        }

        void RenderGraph::Compile()
        {
            //std::cout << "RenderGraph::Compile()" << std::endl;
            mPasses.erase(
            std::remove_if(mPasses.begin(), mPasses.end(), [this](PassNode* pass) {
                bool lone   = !(pass->InComingEdges() + pass->OutGoingEdges());
                bool culled = lone && !pass->mCanBeLone;
                if (culled) mCulledPasses.emplace_back(pass);
                return culled;
            }),
            mPasses.end());

            mResources.erase(
            std::remove_if(mResources.begin(), mResources.end(), [this](ResourceNode* node) {
                bool lone = !(node->InComingEdges() + node->OutGoingEdges());
                if (lone) mCulledResources.emplace_back(node);
                return lone;
            }),
            mResources.end());
        }

        uint64_t RenderGraph::Execute()
        {
            //std::cout << "RenderGraph::Execute" << std::endl;
            m_pGraph->Clear();
            return mFrameIndex++;
        }

        void RenderGraph::Initialize()
        {
            m_pGraph      = DependencyGraph::Create();
            m_pNAEFactory = NodeAndEdgeFactory::Create();
        }

        void RenderGraph::Finalize()
        {
            NodeAndEdgeFactory::Destroy(m_pNAEFactory);
            DependencyGraph::Destroy(m_pGraph);
        }

        EGPUResourceState RenderGraph::GetLastestState(const TextureNode* texture, const PassNode* pending_pass)
        {
            if (mPasses[0] == pending_pass) return texture->mInitState;

            PassNode* pass_iter      = nullptr;
            EGPUResourceState result = texture->mInitState;

            // each write pass
            ForeachWriterPass(texture->GetHandle(), [&](TextureNode* tex, PassNode* pass, RenderGraphEdge* edge) {
                if (edge->type == ERelationshipType::TextureWrite)
                {
                    auto writeEdge = static_cast<TextureWriteEdge*>(edge);
                    if (pass->After(pass_iter) && pass->Before(pending_pass))
                    {
                        pass_iter = pass;
                        result    = writeEdge->mRequestedState;
                    }
                }
                else if (edge->type == ERelationshipType::TextureReadWrite)
                {
                    // TextureReadWrite edge
                }
            });

            // each read pass
            ForeachReaderPass(texture->GetHandle(), [&](TextureNode* tex, PassNode* pass, RenderGraphEdge* edge) {
                if (edge->type == ERelationshipType::TextureRead)
                {
                    auto readEdge = static_cast<TextureReadEdge*>(edge);
                    if (pass->After(pass_iter) && pass->Before(pending_pass))
                    {
                        pass_iter = pass;
                        result    = readEdge->mRequestedState;
                    }
                }
            });

            return result;
        }

        uint32_t RenderGraph::ForeachWriterPass(const TextureHandle handle, const std::function<void(TextureNode* texture, PassNode* pass, RenderGraphEdge* edge)>& func)
        {
            return m_pGraph->ForeachIncomingEdges(handle, [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
                PassNode* node            = static_cast<PassNode*>(from);
                TextureNode* tex          = static_cast<TextureNode*>(to);
                RenderGraphEdge* tex_edge = static_cast<RenderGraphEdge*>(e);
                func(tex, node, tex_edge);
            });
        }

        uint32_t RenderGraph::ForeachReaderPass(const TextureHandle handle, const std::function<void(TextureNode* texture, PassNode* pass, RenderGraphEdge* edge)>& func)
        {
            return m_pGraph->ForeachOutgoingEdges(handle, [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) {
                PassNode* node            = static_cast<PassNode*>(to);
                TextureNode* tex          = static_cast<TextureNode*>(from);
                RenderGraphEdge* tex_edge = static_cast<RenderGraphEdge*>(e);
                func(tex, node, tex_edge);
            });
        }

        ///////////RenderPassBuilder//////////////////
        RenderGraph::RenderPassBuilder::RenderPassBuilder(RenderGraph& graph, RenderPassNode& node)
            : mGraph(graph)
            , mPassNode(node)
        {
        }

        RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::SetName(const char* name)
        {
            if (name) mPassNode.SetName(name);
            return *this;
        }

        RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::Write(uint32_t mrtIndex, TextureRTVHandle handle, EGPULoadAction load, EGPUStoreAction store)
        {
            // 链接pass_node 和 resource_node
            TextureWriteEdge* edge = mGraph.m_pNAEFactory->Allocate<TextureWriteEdge>(mrtIndex, handle);
            mPassNode.mOutTextureEdges.emplace_back(edge);
            mGraph.m_pGraph->Link(&mPassNode, mGraph.m_pGraph->AccessNode(handle.mThis), edge);
            mPassNode.mLoadActions[mrtIndex]  = load;
            mPassNode.mStoreActions[mrtIndex] = store;
            return *this;
        }

        RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::Read(const char* name, TextureSRVHandle handle)
        {
            TextureReadEdge* edge = mGraph.m_pNAEFactory->Allocate<TextureReadEdge>(name, handle);
            mPassNode.mInTextureEdges.emplace_back(edge);
            mGraph.m_pGraph->Link(mGraph.m_pGraph->AccessNode(handle.mThis), &mPassNode, edge);
            return *this;
        }

        RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::SetRootSignature(GPURootSignatureID rs)
        {
            mPassNode.m_pRootSignature = rs;
            return *this;
        }

        RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::SetPipeline(GPURenderPipelineID pipeline)
        {
            mPassNode.m_pPipeline      = pipeline;
            mPassNode.m_pRootSignature = pipeline->pRootSignature;
            return *this;
        }

        RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::SetDepthStencil(TextureDSVHandle handle,
                                                                                        EGPULoadAction depthLoad, EGPUStoreAction depthStore,
                                                                                        EGPULoadAction stencilLoad, EGPUStoreAction stencilStore)
        {
            TextureWriteEdge* edge = mGraph.m_pNAEFactory->Allocate<TextureWriteEdge>(GPU_MAX_MRT_COUNT, handle, GPU_RESOURCE_STATE_DEPTH_WRITE);
            mPassNode.mOutTextureEdges.emplace_back(edge);
            mGraph.m_pGraph->Link(&mPassNode, mGraph.m_pGraph->AccessNode(handle.mThis), edge);
            mPassNode.mDepthLoadAction    = depthLoad;
            mPassNode.mDepthStoreAction   = depthStore;
            mPassNode.mStencilLoadAction  = stencilLoad;
            mPassNode.mStencilStoreAction = stencilStore;
            mPassNode.mClearDepth         = handle.mClearDepth;
            return *this;
        }

        PassHandle RenderGraph::AddRenderPass(const RenderPassSetupFunc& setup, const RenderPassExecuteFunction& execute)
        {
            uint32_t order          = (uint32_t)mPasses.size();
            RenderPassNode* newPass = m_pNAEFactory->Allocate<RenderPassNode>(order);
            mPasses.emplace_back(newPass);
            m_pGraph->Insert(newPass);
            newPass->mExecuteFunc = execute;

            RenderPassBuilder builder(*this, *newPass);
            setup(*this, builder);
            return newPass->GetHandle();
        }
        ///////////RenderPassBuilder//////////////////

        ///////////TextureBuilder//////////////////
        RenderGraph::TextureBuilder::TextureBuilder(RenderGraph& graph, TextureNode& textureNode)
            : mGraph(graph)
            , mTextureNode(textureNode)
        {
            mTextureNode.mDesc.sample_count = GPU_SAMPLE_COUNT_1;
            mTextureNode.mDesc.descriptors  = GPU_RESOURCE_TYPE_TEXTURE;
            mTextureNode.mDesc.is_dedicated = false;
        }

        RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::Import(GPUTextureID texture, EGPUResourceState initedState)
        {
            mTextureNode.m_pFrameTexture    = texture;
            mTextureNode.mInitState         = initedState;
            mTextureNode.mDesc.width        = texture->width;
            mTextureNode.mDesc.height       = texture->height;
            mTextureNode.mDesc.depth        = texture->depth;
            mTextureNode.mDesc.array_size   = texture->arraySizeMinusOne + 1;
            mTextureNode.mDesc.format       = (EGPUFormat)texture->format;
            mTextureNode.mDesc.sample_count = texture->sampleCount;
            mTextureNode.mImported          = texture;
            return *this;
        }

        RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::SetName(const char* name)
        {
            mTextureNode.SetName(name);
            return *this;
        }

        RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::AllowRenderTarget()
        {
            mTextureNode.mDesc.descriptors |= GPU_RESOURCE_TYPE_RENDER_TARGET;
            mTextureNode.mDesc.start_state = GPU_RESOURCE_STATE_UNDEFINED;
            return *this;
        }

        TextureHandle RenderGraph::CreateTexture(const TextureSetupFunc& setup)
        {
            auto newTex = m_pNAEFactory->Allocate<TextureNode>();
            mResources.emplace_back(newTex);
            m_pGraph->Insert(newTex);

            TextureBuilder builder(*this, *newTex);
            setup(*this, builder);
            return newTex->GetHandle();
        }
        ///////////TextureBuilder//////////////////

        ///////////PresentPassBuilder//////////////////
        RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::SetName(const char* name)
        {
            mPassNode.SetName(name);
            return *this;
        }

        RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::Swapchain(GPUSwapchainID swapchain, uint32_t index)
        {
            mPassNode.mDesc.swapchain = swapchain;
            mPassNode.mDesc.index     = index;
            return *this;
        }

        RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::Texture(TextureHandle handle, bool isBackBuffer)
        {
            assert(isBackBuffer);
            TextureReadEdge* edge = mGraph.m_pNAEFactory->Allocate<TextureReadEdge>("PresentSrc", handle, GPU_RESOURCE_STATE_PRESENT);
            mPassNode.mInTextureEdges.emplace_back(edge);
            mGraph.m_pGraph->Link(mGraph.m_pGraph->AccessNode(handle), &mPassNode, edge);
            return *this;
        }

        RenderGraph::PresentPassBuilder::PresentPassBuilder(RenderGraph& graph, PresentPassNode& node)
            : mGraph(graph)
            , mPassNode(node)
        {
        }

        PassHandle RenderGraph::AddPresentPass(const PresentPassSetupFunc& setup)
        {
            uint32_t size         = (uint32_t)mPasses.size();
            PresentPassNode* node = m_pNAEFactory->Allocate<PresentPassNode>(size);
            mPasses.emplace_back(node);
            m_pGraph->Insert(node);

            PresentPassBuilder builder(*this, *node);
            setup(*this, builder);
            return node->GetHandle();
        }

        ///////////PresentPassBuilder//////////////////

        ///////////BufferBuilder//////////////////
        RenderGraph::BufferBuilder::BufferBuilder(RenderGraph& graph, BufferNode& node)
            : mGraph(graph)
            , mNode(node)
        {
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::SetName(const char* name)
        {
            mNode.SetName(name);
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::Import(GPUBufferID buffer, EGPUResourceState initState)
        {
            mNode.mImported          = buffer;
            mNode.mInitState         = initState;
            mNode.m_pBuffer          = buffer;
            mNode.mDesc.size         = buffer->size;
            mNode.mDesc.descriptors  = buffer->descriptors;
            mNode.mDesc.memory_usage = (EGPUMemoryUsage)buffer->memory_usage;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::OwnsMemory()
        {
            mNode.mDesc.flags |= GPU_BCF_OWN_MEMORY_BIT;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::Size(uint64_t size)
        {
            mNode.mDesc.size = size;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::WithFlags(GPUBufferCreationFlags flags)
        {
            mNode.mDesc.flags |= flags;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::MemoryUsage(EGPUMemoryUsage mem_usage)
        {
            mNode.mDesc.memory_usage = mem_usage;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::AllowShaderReadWrite()
        {
            mNode.mDesc.descriptors |= GPU_RESOURCE_TYPE_RW_BUFFER;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::AllowShaderRead()
        {
            mNode.mDesc.descriptors |= GPU_RESOURCE_TYPE_BUFFER;
            mNode.mDesc.descriptors |= GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::AsUploadBuffer()
        {
            mNode.mDesc.flags |= GPU_BCF_PERSISTENT_MAP_BIT;
            mNode.mDesc.start_state  = GPU_RESOURCE_STATE_COPY_SOURCE;
            mNode.mDesc.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::AsVertexBuffer()
        {
            mNode.mDesc.descriptors |= GPU_RESOURCE_TYPE_VERTEX_BUFFER;
            mNode.mDesc.start_state = GPU_RESOURCE_STATE_COPY_DEST;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::AsIndexBuffer()
        {
            mNode.mDesc.descriptors |= GPU_RESOURCE_TYPE_INDEX_BUFFER;
            mNode.mDesc.start_state = GPU_RESOURCE_STATE_COPY_DEST;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::AsUniformBuffer()
        {
            mNode.mDesc.descriptors |= GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
            mNode.mDesc.start_state = GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::PreferOnDevice()
        {
            mNode.mDesc.prefer_on_device = true;
            return *this;
        }

        RenderGraph::BufferBuilder& RenderGraph::BufferBuilder::PreferOnHost()
        {
            mNode.mDesc.prefer_on_host = true;
            return *this;
        }

        BufferHandle RenderGraph::CreateBuffer(const BufferSetupFunc& setup)
        {
            auto node = m_pNAEFactory->Allocate<BufferNode>();
            mResources.emplace_back(node);
            m_pGraph->Insert(node);

            BufferBuilder builder(*this, *node);
            setup(*this, builder);
            return node->GetHandle();
        }

        EGPUResourceState RenderGraph::GetLastestState(const BufferNode* buffer, const PassNode* pending_pass)
        {
            if (mPasses[0] == pending_pass) return buffer->mInitState;

            PassNode* pass_iter      = nullptr;
            EGPUResourceState result = buffer->mInitState;

            // each write pass
            ForeachWriterPass(buffer->GetHandle(), [&](BufferNode* buffer, PassNode* pass, RenderGraphEdge* edge) {
                if (edge->type == ERelationshipType::BufferReadWrite)
                {
                    auto writeEdge = static_cast<BufferReadWriteEdge*>(edge);
                    if (pass->After(pass_iter) && pass->Before(pending_pass))
                    {
                        pass_iter = pass;
                        result    = writeEdge->mRequestedState;
                    }
                }
            });

            // each read pass
            ForeachReaderPass(buffer->GetHandle(), [&](BufferNode* tex, PassNode* pass, RenderGraphEdge* edge) {
                if (edge->type == ERelationshipType::BufferRead)
                {
                    auto readEdge = static_cast<BufferReadEdge*>(edge);
                    if (pass->After(pass_iter) && pass->Before(pending_pass))
                    {
                        pass_iter = pass;
                        result    = readEdge->mRequestedState;
                    }
                }
                else if (edge->type == ERelationshipType::PipelineBuffer)
                {
                    // PipelineBuffer edge
                    /* auto readEdge = static_cast<BufferReadEdge*>(edge);
                    if (pass->After(pass_iter) && pass->Before(pending_pass))
                    {
                        pass_iter = pass;
                        result    = readEdge->mRequestedState;
                    } */
                }
            });

            return result;
        }

        uint32_t RenderGraph::ForeachWriterPass(const BufferHandle handle, const std::function<void(BufferNode*, PassNode*, RenderGraphEdge*)>& func)
        {
            return m_pGraph->ForeachIncomingEdges(handle, [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* edge) {
                PassNode* node     = static_cast<PassNode*>(from);
                BufferNode* buffer = static_cast<BufferNode*>(to);
                RenderGraphEdge* e = static_cast<RenderGraphEdge*>(edge);
                func(buffer, node, e);
            });
        }

        uint32_t RenderGraph::ForeachReaderPass(const BufferHandle handle, const std::function<void(BufferNode*, PassNode*, RenderGraphEdge*)>& func)
        {
            return m_pGraph->ForeachOutgoingEdges(handle, [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* edge) {
                PassNode* node     = static_cast<PassNode*>(to);
                BufferNode* buffer = static_cast<BufferNode*>(from);
                RenderGraphEdge* e = static_cast<RenderGraphEdge*>(edge);
                func(buffer, node, e);
            });
        }
        ///////////BufferBuilder//////////////////

        ///////////CopyPassBuilder//////////////////
        RenderGraph::CopyPassBuilder::CopyPassBuilder(RenderGraph& graph, CopyPassNode& node)
            : mGraph(graph)
            , mPassNode(node)
        {
        }

        RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::SetName(const char* name)
        {
            mPassNode.SetName(name);
            return *this;
        }

        RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::CanBeLone()
        {
            mPassNode.mCanBeLone = true;
            return *this;
        }

        RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::TextureToTexture(TextureSubresourceHandle src, TextureSubresourceHandle dst, EGPUResourceState dstState)
        {
            auto in        = mGraph.m_pNAEFactory->Allocate<TextureReadEdge>("copy_src", src.mThis, GPU_RESOURCE_STATE_COPY_SOURCE);
            auto out       = mGraph.m_pNAEFactory->Allocate<TextureWriteEdge>(0, dst.mThis, GPU_RESOURCE_STATE_COPY_DEST);
            auto&& inEdge  = mPassNode.mInTextureEdges.emplace_back(in);
            auto&& outEdge = mPassNode.mOutTextureEdges.emplace_back(out);
            mGraph.m_pGraph->Link(mGraph.m_pGraph->AccessNode(src.mThis), &mPassNode, inEdge);
            mGraph.m_pGraph->Link(&mPassNode, mGraph.m_pGraph->AccessNode(dst.mThis), outEdge);
            mPassNode.mT2Ts.emplace_back(src, dst);
            if (dstState != GPU_RESOURCE_STATE_COPY_DEST)
            {
                mPassNode.mTextureBarriers.emplace_back(dst, dstState);
            }
            return *this;
        }

        RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::BufferToBuffer(BufferRangeHandle src, BufferRangeHandle dst, EGPUResourceState dstState)
        {
            auto in        = mGraph.m_pNAEFactory->Allocate<BufferReadEdge>("copy_src", src, GPU_RESOURCE_STATE_COPY_SOURCE);
            auto out       = mGraph.m_pNAEFactory->Allocate<BufferReadWriteEdge>(dst, GPU_RESOURCE_STATE_COPY_DEST);
            auto&& inEdge  = mPassNode.mInBufferEdges.emplace_back(in);
            auto&& outEdge = mPassNode.mOutBufferEdges.emplace_back(out);
            mGraph.m_pGraph->Link(mGraph.m_pGraph->AccessNode(src.mThis), &mPassNode, inEdge);
            mGraph.m_pGraph->Link(&mPassNode, mGraph.m_pGraph->AccessNode(dst.mThis), outEdge);
            mPassNode.mB2Bs.emplace_back(src, dst);
            if (dstState != GPU_RESOURCE_STATE_COPY_DEST)
            {
                mPassNode.mBufferBarriers.emplace_back(dst, dstState);
            }
            return *this;
        }

        RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::BufferToTexture(BufferRangeHandle src, TextureSubresourceHandle dst, EGPUResourceState dstState)
        {
            auto in        = mGraph.m_pNAEFactory->Allocate<BufferReadEdge>("copy_src", src, GPU_RESOURCE_STATE_COPY_SOURCE);
            auto out       = mGraph.m_pNAEFactory->Allocate<TextureWriteEdge>(0, dst.mThis, GPU_RESOURCE_STATE_COPY_DEST);
            auto&& inEdge  = mPassNode.mInBufferEdges.emplace_back(in);
            auto&& outEdge = mPassNode.mOutTextureEdges.emplace_back(out);
            mGraph.m_pGraph->Link(mGraph.m_pGraph->AccessNode(src.mThis), &mPassNode, inEdge);
            mGraph.m_pGraph->Link(&mPassNode, mGraph.m_pGraph->AccessNode(dst.mThis), outEdge);
            mPassNode.mB2Ts.emplace_back(src, dst);
            if (dstState != GPU_RESOURCE_STATE_COPY_DEST)
            {
                mPassNode.mTextureBarriers.emplace_back(dst, dstState);
            }
            return *this;
        }

        RenderGraph::CopyPassBuilder& RenderGraph::CopyPassBuilder::FromBuffer(BufferRangeHandle src)
        {
            auto in = mGraph.m_pNAEFactory->Allocate<BufferReadEdge>("copy_src", src, GPU_RESOURCE_STATE_COPY_SOURCE);
            mPassNode.mInBufferEdges.emplace_back(in);
            mGraph.m_pGraph->Link(mGraph.m_pGraph->AccessNode(src.mThis), &mPassNode, in);
            return *this;
        }

        PassHandle RenderGraph::AddCopyPass(const CopyPassSetupFunc& setup, const CopyPassExecuteFunction& execute)
        {
            uint32_t order        = (uint32_t)mPasses.size();
            auto newPass          = m_pNAEFactory->Allocate<CopyPassNode>(order);
            newPass->mExecuteFunc = execute;
            mPasses.emplace_back(newPass);
            m_pGraph->Insert(newPass);

            CopyPassBuilder builder(*this, *newPass);
            setup(*this, builder);
            return newPass->GetHandle();
        }
        ///////////CopyPassBuilder//////////////////

        BufferNode* RenderGraph::Resolve(BufferHandle hdl)
        {
            return static_cast<BufferNode*>(m_pGraph->NodeAt(hdl));
        }

        TextureNode* RenderGraph::Resolve(TextureHandle hdl)
        {
            return static_cast<TextureNode*>(m_pGraph->NodeAt(hdl));
        }

        PassNode* RenderGraph::Resolve(PassHandle hdl)
        {
            return static_cast<PassNode*>(m_pGraph->NodeAt(hdl));
        }

        RenderGraph::RenderGraph()
        {
        }
    } // namespace render_graph
} // namespace FakeReal