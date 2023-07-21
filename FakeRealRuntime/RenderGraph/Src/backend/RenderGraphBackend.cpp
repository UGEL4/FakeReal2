#include "render_graph/include/backend/RenderGraphBackend.h"
#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceEdge.hpp"
#include "render_graph/include/frontend/ResourceNode.hpp"
#include "render_graph/include/frontend/NodeAndEdgeFactory.hpp"
#include "Utils.h"
#include <iostream>
#include <stdint.h>
#include <vector>
#include <assert.h>

//////////////////RenderGraphFrameExecutor////////////////////////
void RenderGraphFrameExecutor::Initialize(GPUDeviceID gfxDevice, GPUQueueID gfxQueue)
{
    m_pCommandPool                  = GPUCreateCommandPool(gfxQueue);
    GPUCommandBufferDescriptor desc = {};
    desc.isSecondary                = false;
    m_pCmd                          = GPUCreateCommandBuffer(m_pCommandPool, &desc);
    m_pFence                        = GPUCreateFence(gfxDevice);
}

void RenderGraphFrameExecutor::Finalize()
{
    if (m_pCmd) GPUFreeCommandBuffer(m_pCmd);
    if (m_pCommandPool) GPUFreeCommandPool(m_pCommandPool);
    if (m_pFence) GPUFreeFence(m_pFence);
    m_pCommandPool = nullptr;
    m_pCmd         = nullptr;
    m_pFence       = nullptr;
    for (auto iter : mBindTablePools)
    {
        if (iter.second)
        {
            iter.second->Destroy();
            iter.second->~BindTablePool();
            free(iter.second);
        }
    }
    mBindTablePools.clear();
}

void RenderGraphFrameExecutor::ResetOnStart()
{
    for (auto iter : mBindTablePools)
    {
        if (iter.second) iter.second->Reset();
    }
    GPUResetCommandPool(m_pCommandPool);
}

void RenderGraphFrameExecutor::Commit(GPUQueueID gfxQueue, uint64_t frameIndex)
{
    // submit
    GPUQueueSubmitDescriptor submitDesc{};
    submitDesc.cmds         = &m_pCmd;
    submitDesc.cmds_count   = 1;
    submitDesc.signal_fence = m_pFence;
    GPUSubmitQueue(gfxQueue, &submitDesc);
    mExecFrame = frameIndex;
}
//////////////////RenderGraphFrameExecutor////////////////////////

//////////////////RenderGraphBackend////////////////////////
RenderGraphBackend::RenderGraphBackend(const RenderGraphBuilder& builder)
: m_pDevice(builder.m_pDevice), m_pQueue(builder.m_pQueue)
{

}

uint64_t RenderGraphBackend::Execute()
{
    std::cout << "RenderGraphBackend::Execute" << std::endl;

    uint32_t frameIndex = mFrameIndex % RG_MAX_FRAME_IN_FLIGHT;
    auto& executor = mExecutors[frameIndex];
    GPUWaitFences(&executor.m_pFence, 1);

    executor.ResetOnStart();
    GPUCmdBegin(executor.m_pCmd);
    {
        for (auto& pass : mPasses)
        {
            if (pass->mPassType == EPassType::Render)
            {
                ExecuteRenderPass(static_cast<RenderPassNode*>(pass), executor);
            }
            if (pass->mPassType == EPassType::Present)
            {
                ExectuePresentPass(static_cast<PresentPassNode*>(pass), executor);
            }
            if (pass->mPassType == EPassType::Copy)
            {
                ExectueCopyPass(static_cast<CopyPassNode*>(pass), executor);
            }
        }
    }
    GPUCmdEnd(executor.m_pCmd);
    {
        //submit
        executor.Commit(m_pQueue, frameIndex);
    }

    //clear
    {
        for (auto pass : mCulledPasses)
        {
            m_pNAEFactory->Dealloc(pass);
        }
        mCulledPasses.clear();

        for (auto res : mCulledPasses)
        {
            m_pNAEFactory->Dealloc(res);
        }
        mCulledResources.clear();

        //pass
        for (auto pass : mPasses)
        {
            if (pass)
            {
                //texture
                pass->ForEachTextures([this](TextureNode* texture, TextureEdge* edge)
                {
                    m_pNAEFactory->Dealloc(edge);
                });

                //buffer
                pass->ForeachBuffer([this](BufferNode* buffer, BufferEdge* dege)
                {
                    m_pNAEFactory->Dealloc(dege);
                });
                m_pNAEFactory->Dealloc(pass);
            }
        }
        mPasses.clear();

        //resources
        for (auto res : mResources)
        {
            if (res)
            {
                m_pNAEFactory->Dealloc(res);
            }
        }
        mResources.clear();

        m_pGraph->Clear();
    }

    return mFrameIndex++;
}

void RenderGraphBackend::Initialize()
{
    RenderGraph::Initialize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        mExecutors[i].Initialize(m_pDevice, m_pQueue);
    }
    mTexturePool.Initialize(m_pDevice);
    mTextureViewPool.Initialize(m_pDevice);
    mBufferPool.Initialize(m_pDevice);
}

void RenderGraphBackend::Finalize()
{
    RenderGraph::Finalize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FLIGHT; i++)
    {
        mExecutors[i].Finalize();
    }
    mTextureViewPool.Finalize();
    mTexturePool.Finalize();
    mBufferPool.Finalize();
}

void RenderGraphBackend::ExecuteRenderPass(RenderPassNode* pass,  RenderGraphFrameExecutor& executor)
{
    // resource de-virtualize
    std::vector<GPUTextureBarrier> tex_barriers;
    std::vector<std::pair<TextureHandle, GPUTextureID>> resolved_textures;
    std::vector<GPUBufferBarrier> buffer_barriers;
    std::vector<std::pair<BufferHandle, GPUBufferID>> resolved_buffers;
    CalculateResourceBarriers(executor, pass, tex_barriers, resolved_textures, buffer_barriers, resolved_buffers);
    //alloca & update descriptorset
    RenderPassContext passContext {};
    passContext.m_pPassNode       = pass;
    passContext.m_pCmd            = executor.m_pCmd;
    passContext.m_pGraph          = this;
    passContext.mResolvedBuffers  = resolved_buffers;
    passContext.mResolvedTextures = resolved_textures;
    passContext.m_pBindTable      = AllocateAndUpdatePassBindTable(executor, pass, pass->m_pRootSignature);
    //call gpu aip
    GPUResourceBarrierDescriptor barrier_desc{};
    if (!tex_barriers.empty())
    {
        barrier_desc.texture_barriers = tex_barriers.data();
        barrier_desc.texture_barriers_count = (uint32_t)tex_barriers.size();
    }
    GPUCmdResourceBarrier(executor.m_pCmd, &barrier_desc);

    {
        std::vector<GPUColorAttachment> attachments = {};
        GPUDepthStencilAttachment ds_attachment     = {};
        auto writeEdges                             = pass->GetTextureWriteEdges();
        EGPUSampleCount passSampleCount             = GPU_SAMPLE_COUNT_1;
        for (auto edge : writeEdges)
        {
            auto targetTexture = edge->GetTextureNode();
            bool is_depth_stencil = Utils::FormatUtil_IsDepthStencilFormat((EGPUFormat)(Resolve(executor, *targetTexture)->format));
            bool is_depth_only = Utils::FormatUtil_IsDepthOnlyFormat((EGPUFormat)(Resolve(executor, *targetTexture)->format));
            if (edge->mRequestedState == GPU_RESOURCE_STATE_DEPTH_WRITE && is_depth_stencil)
            {
                GPUTextureViewDescriptor desc {};
                desc.pTexture        = Resolve(executor, *targetTexture);
                desc.format          = (EGPUFormat)desc.pTexture->format;
                desc.usages          = GPU_TVU_RTV_DSV;
                desc.aspectMask      = is_depth_only ? GPU_TVA_DEPTH : GPU_TVA_DEPTH | GPU_TVA_STENCIL;
                desc.baseMipLevel    = edge->GetMipBase();
                desc.mipLevelCount   = 1;
                desc.baseArrayLayer  = edge->GetArrayBase();
                desc.arrayLayerCount = edge->GetArrayCount();
                desc.dims            = GPU_TEX_DIMENSION_2D;

                ds_attachment.view                 = mTextureViewPool.Allocate(desc, mFrameIndex);
                ds_attachment.depth_load_action    = pass->mDepthLoadAction;
                ds_attachment.depth_store_action   = pass->mDepthStoreAction;
                ds_attachment.clear_depth          = pass->mClearDepth;
                ds_attachment.write_depth          = 1;
                ds_attachment.stencil_load_action  = pass->mStencilLoadAction;
                ds_attachment.stencil_store_action = pass->mStencilStoreAction;
            }
            else
            {
                GPUTextureViewDescriptor desc {};
                desc.pTexture        = Resolve(executor, *targetTexture);
                desc.format          = (EGPUFormat)desc.pTexture->format;
                desc.usages          = GPU_TVU_RTV_DSV;
                desc.aspectMask      = GPU_TVA_COLOR;
                desc.baseMipLevel    = edge->GetMipBase();
                desc.mipLevelCount   = 1;
                desc.baseArrayLayer  = edge->GetArrayBase();
                desc.arrayLayerCount = edge->GetArrayCount();
                desc.dims            = GPU_TEX_DIMENSION_2D;

                GPUColorAttachment colorAttachment{};
                colorAttachment.view         = mTextureViewPool.Allocate(desc, mFrameIndex);
                colorAttachment.load_action  = pass->mLoadActions[edge->mMRTIndex];
                colorAttachment.store_action = pass->mStoreActions[edge->mMRTIndex];
                colorAttachment.clear_color  = { { 0.f, 0.f, 0.f, 0.f } };//todo : write edge store clear_color

                attachments.emplace_back(colorAttachment);
            }
        }
        GPURenderPassDescriptor render_pass_desc{};
        render_pass_desc.name                = pass->GetName();
        render_pass_desc.sample_count        = passSampleCount;
        render_pass_desc.color_attachments   = attachments.data();
        render_pass_desc.render_target_count = (uint32_t)attachments.size();
        render_pass_desc.depth_stencil       = &ds_attachment;
        passContext.m_pEncoder = GPUCmdBeginRenderPass(executor.m_pCmd, &render_pass_desc);
        {
            if (pass->m_pPipeline) GPURenderEncoderBindPipeline(passContext.m_pEncoder, pass->m_pPipeline);
            if (passContext.m_pEncoder) GPURenderEncoderBindBindTable(passContext.m_pEncoder, passContext.m_pBindTable);
            pass->mExecuteFunc(*this, passContext);
        }
        GPUCmdEndRenderPass(executor.m_pCmd, passContext.m_pEncoder);
    }
    //deallace resource
    DeallocaResources(pass);
}

void RenderGraphBackend::ExectuePresentPass(PresentPassNode* pass, RenderGraphFrameExecutor& executor)
{
    auto edges = pass->GetTextureReadEdges();
    auto&& edge = edges[0];
    GPUTextureBarrier present_barrier{};
    present_barrier.texture   = pass->mDesc.swapchain->ppBackBuffers[pass->mDesc.index];
    present_barrier.src_state = GetLastestState(edge->GetTextureNode(), pass);
    present_barrier.dst_state = GPU_RESOURCE_STATE_PRESENT;
    GPUResourceBarrierDescriptor barrier_desc{};
    barrier_desc.texture_barriers_count = 1;
    barrier_desc.texture_barriers       = &present_barrier;
    GPUCmdResourceBarrier(executor.m_pCmd, &barrier_desc);
}

void RenderGraphBackend::ExectueCopyPass(CopyPassNode* pass, RenderGraphFrameExecutor& executor)
{
    std::vector<GPUTextureBarrier> tex_barriers;
    std::vector<std::pair<TextureHandle, GPUTextureID>> resolved_textures;
    std::vector<GPUBufferBarrier> buffer_barriers;
    std::vector<std::pair<BufferHandle, GPUBufferID>> resolved_buffers;
    CalculateResourceBarriers(executor, pass, tex_barriers, resolved_textures, buffer_barriers, resolved_buffers);
    // late barriers
    std::vector<GPUTextureBarrier> late_tex_barriers = {};
    std::vector<GPUBufferBarrier> late_buf_barriers = {};
    // call cgpu apis
    GPUResourceBarrierDescriptor barriers = {};
    GPUResourceBarrierDescriptor late_barriers = {};
    if (!tex_barriers.empty())
    {
        barriers.texture_barriers = tex_barriers.data();
        barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
    }
    if (!buffer_barriers.empty())
    {
        barriers.buffer_barriers = buffer_barriers.data();
        barriers.buffer_barriers_count = (uint32_t)buffer_barriers.size();
    }
    {
        CopyPassContext passContext   = {};
        passContext.m_pCmd            = executor.m_pCmd;
        passContext.mResolvedBuffers  = resolved_buffers;
        passContext.mResolvedTextures = resolved_textures;
        pass->mExecuteFunc(*this, passContext);
        for (auto [buffer_handle, state] : pass->mBufferBarriers)
        {
            auto buffer       = passContext.Resolve(buffer_handle);
            auto& barrier     = late_buf_barriers.emplace_back();
            barrier.buffer    = buffer;
            barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = state;
        }
        for (auto [texture_handle, state] : pass->mTextureBarriers)
        {
            auto texture      = passContext.Resolve(texture_handle);
            auto& barrier     = late_tex_barriers.emplace_back();
            barrier.texture   = texture;
            barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = state;
        }
        if (!late_tex_barriers.empty())
        {
            late_barriers.texture_barriers       = late_tex_barriers.data();
            late_barriers.texture_barriers_count = (uint32_t)late_tex_barriers.size();
        }
        if (!late_buf_barriers.empty())
        {
            late_barriers.buffer_barriers       = late_buf_barriers.data();
            late_barriers.buffer_barriers_count = (uint32_t)late_buf_barriers.size();
        }
    }
    GPUCmdResourceBarrier(executor.m_pCmd, &barriers);
    for (uint32_t i = 0; i < pass->mT2Ts.size(); i++)
    {
        auto src_node = RenderGraph::Resolve(pass->mT2Ts[i].first);
        auto dst_node = RenderGraph::Resolve(pass->mT2Ts[i].second);
        GPUTextureToTextureTransfer t2t      = {};
        t2t.src                              = Resolve(executor, *src_node);
        t2t.src_subresource.aspects          = pass->mT2Ts[i].first.aspects;
        t2t.src_subresource.mip_level        = pass->mT2Ts[i].first.mip_level;
        t2t.src_subresource.base_array_layer = pass->mT2Ts[i].first.array_base;
        t2t.src_subresource.layer_count      = pass->mT2Ts[i].first.array_count;
        t2t.dst                              = Resolve(executor, *dst_node);
        t2t.dst_subresource.aspects          = pass->mT2Ts[i].second.aspects;
        t2t.dst_subresource.mip_level        = pass->mT2Ts[i].second.mip_level;
        t2t.dst_subresource.base_array_layer = pass->mT2Ts[i].second.array_base;
        t2t.dst_subresource.layer_count      = pass->mT2Ts[i].second.array_count;
        GPUCmdTransferTextureToTexture(executor.m_pCmd, &t2t);
    }
    for (uint32_t i = 0; i < pass->mB2Bs.size(); i++)
    {
        auto src_node = RenderGraph::Resolve(pass->mB2Bs[i].first);
        auto dst_node = RenderGraph::Resolve(pass->mB2Bs[i].second);
        GPUBufferToBufferTransfer b2b = {};
        b2b.src                       = Resolve(executor, *src_node);
        b2b.src_offset                = pass->mB2Bs[i].first.mFrom;
        b2b.dst                       = Resolve(executor, *dst_node);
        b2b.dst_offset                = pass->mB2Bs[i].second.mFrom;
        b2b.size                      = pass->mB2Bs[i].first.mTo - b2b.src_offset;
        GPUCmdTransferBufferToBuffer(executor.m_pCmd, &b2b);
    }
    for (uint32_t i = 0; i < pass->mB2Ts.size(); i++)
    {
        auto src_node                        = RenderGraph::Resolve(pass->mB2Ts[i].first);
        auto dst_node                        = RenderGraph::Resolve(pass->mB2Ts[i].second);
        GPUBufferToTextureTransfer b2t       = {};
        b2t.src                              = Resolve(executor, *src_node);
        b2t.src_offset                       = pass->mB2Ts[i].first.mFrom;
        b2t.dst                              = Resolve(executor, *dst_node);
        b2t.dst_subresource.mip_level        = pass->mB2Ts[i].second.mip_level;
        b2t.dst_subresource.base_array_layer = pass->mB2Ts[i].second.array_base;
        b2t.dst_subresource.layer_count      = pass->mB2Ts[i].second.array_count;
        GPUCmdTransferBufferToTexture(executor.m_pCmd, &b2t);
    }
    GPUCmdResourceBarrier(executor.m_pCmd, &late_barriers);
    DeallocaResources(pass);
}

void RenderGraphBackend::CalculateResourceBarriers(RenderGraphFrameExecutor& executor, PassNode* pass,
        std::vector<GPUTextureBarrier>& tex_barriers, std::vector<std::pair<TextureHandle, GPUTextureID>>& resolved_textures,
        std::vector<GPUBufferBarrier>& buffer_barriers, std::vector<std::pair<BufferHandle, GPUBufferID>>& resolved_buffers)
{
    tex_barriers.reserve(pass->GetTextureCount());
    resolved_textures.reserve(pass->GetTextureCount());
    buffer_barriers.reserve(pass->GetBuffersCount());
    resolved_buffers.reserve(pass->GetBuffersCount());
    //遍历pass的每一个texture资源
    pass->ForEachTextures([&](TextureNode* tex, TextureEdge* edge)
    {
        //分配texture资源
        auto resolved_texture = Resolve(executor, *tex);
        resolved_textures.emplace_back(tex->GetHandle(), resolved_texture);
        auto curr_state = GetLastestState(tex, pass);
        if (curr_state == edge->mRequestedState) return;
        //分配barrier
        GPUTextureBarrier barrier{};
        barrier.texture   = resolved_texture;
        barrier.src_state = curr_state;
        barrier.dst_state = edge->mRequestedState;
        tex_barriers.emplace_back(barrier);
    });

    //遍历pass的每一个buffer资源
    pass->ForeachBuffer([&](BufferNode* bufferNode, BufferEdge* bufferEdge)
    {
        auto resolved_buffer = Resolve(executor, *bufferNode);
        resolved_buffers.emplace_back(bufferNode->GetHandle(), resolved_buffer);
        auto curr_state = GetLastestState(bufferNode, pass);
        if (curr_state == bufferEdge->mRequestedState) return;
        //分配barrier
        GPUBufferBarrier barrier{};
        barrier.buffer = resolved_buffer;
        barrier.src_state = curr_state;
        barrier.dst_state = bufferEdge->mRequestedState;
        buffer_barriers.emplace_back(barrier);
    });
}

GPUTextureID RenderGraphBackend::Resolve(RenderGraphFrameExecutor& executor, const TextureNode& texture)
{
    if (!texture.m_pFrameTexture)
    {
        //allocate texture
        auto allocated          = mTexturePool.Allocate(texture.mDesc, { mFrameIndex, 0 });
        texture.m_pFrameTexture = allocated.first;
        texture.mInitState      = allocated.second;
    }
    return texture.m_pFrameTexture;
}

GPUBufferID RenderGraphBackend::Resolve(RenderGraphFrameExecutor& executor, const BufferNode& buffer)
{
    if (!buffer.m_pBuffer)
    {
        //uint64_t latest_frame   = (node.tags & kRenderGraphDynamicResourceTag) ? get_latest_finished_frame() : UINT64_MAX
        uint64_t latest_frame = UINT64_MAX;
        auto allocated        = mBufferPool.Allocate(buffer.mDesc, { mFrameIndex, 0 }, latest_frame);
        buffer.m_pBuffer      = allocated.first;
        buffer.mInitState     = allocated.second;
    }
    return buffer.m_pBuffer;
}

GPUBindTableID RenderGraphBackend::AllocateAndUpdatePassBindTable(RenderGraphFrameExecutor& executor, PassNode* pass, GPURootSignatureID root_sig)
{
    if (root_sig == nullptr) return nullptr;
    auto texReadEdges = pass->GetTextureReadEdges();
    // Allocate or get descriptor set heap
    auto iter = executor.mBindTablePools.find(root_sig);
    if (iter == executor.mBindTablePools.end())
    {
        void* ptr           = calloc(1, sizeof(BindTablePool));
        BindTablePool* pool = new (ptr) BindTablePool(root_sig);
        executor.mBindTablePools.emplace(root_sig, pool);
    }
    // Bind resources
    std::string bind_table_keys = "";
    std::vector<GPUDescriptorData> desc_set_updates;
    std::vector<const char*> bindTableValueNames = {};
    std::vector<GPUTextureViewID> SRVs(texReadEdges.size());
    // SRV
    for (uint32_t i = 0; i < texReadEdges.size(); i++)
    {
        auto& readEdge = texReadEdges[i];
        assert(!readEdge->mName.empty());
        const auto& res = *FindShaderResource(readEdge->mNameHash, root_sig);
        bind_table_keys += readEdge->mName.empty() ? (const char*)res.name : readEdge->mName;
        bind_table_keys += ';';
        bindTableValueNames.emplace_back((const char*)res.name);

        auto texture_readed                = readEdge->GetTextureNode();
        GPUDescriptorData update           = {};
        update.count                       = 1;
        update.name                        = res.name;
        update.binding_type                = GPU_RESOURCE_TYPE_TEXTURE;
        update.binding                     = res.binding;
        GPUTextureViewDescriptor view_desc = {};
        view_desc.pTexture                 = Resolve(executor, *texture_readed);
        view_desc.baseArrayLayer           = readEdge->GetArrayBase();
        view_desc.arrayLayerCount          = readEdge->GetArrayCount();
        view_desc.baseMipLevel             = readEdge->GetMipBase();
        view_desc.mipLevelCount            = readEdge->GetMipCount();
        view_desc.format                   = (EGPUFormat)view_desc.pTexture->format;
        bool is_depth_stencil              = Utils::FormatUtil_IsDepthStencilFormat(view_desc.format);
        bool is_depth_only                 = Utils::FormatUtil_IsDepthOnlyFormat(view_desc.format);
        view_desc.aspectMask =
        is_depth_stencil ?
        is_depth_only ? GPU_TVA_DEPTH : GPU_TVA_DEPTH | GPU_TVA_STENCIL :
        GPU_TVA_COLOR;
        view_desc.usages = GPU_TVU_SRV;
        view_desc.dims   = readEdge->GetDimension();
        SRVs[i]          = mTextureViewPool.Allocate(view_desc, mFrameIndex);
        update.textures  = &SRVs[i];
        desc_set_updates.emplace_back(update);
    }

    GPUBindTableID table = executor.mBindTablePools[root_sig]->Pop(bind_table_keys, bindTableValueNames.data(), (uint32_t)bindTableValueNames.size());
    GPUBindTableUpdate(table, desc_set_updates.data(), (uint32_t)desc_set_updates.size());
    return table;
}

const GPUShaderResource* RenderGraphBackend::FindShaderResource(uint64_t nameHash, GPURootSignatureID rs, EGPUResourceType* type) const
{
    for (uint32_t i = 0; i < rs->table_count; i++)
        {
            for (uint32_t j = 0; j < rs->tables[i].resources_count; j++)
            {
                const auto& resource = rs->tables[i].resources[j];
                if (resource.name_hash == nameHash)
                //if (strcmp((const char*)resource.name, name.c_str()) == 0)
                {
                    if (type) *type = resource.type;
                    return &rs->tables[i].resources[j];
                }
            }
        }
        return nullptr;
}

void RenderGraphBackend::DeallocaResources(PassNode* pass)
{
    pass->ForEachTextures([this, pass](TextureNode* texture, TextureEdge* edge)
    {
        if (texture->mImported) return;
        bool isLastUser = true;
        texture->ForeachNeighbors([&](DependencyGraphNode* node)
        {
            RenderGraphNode* renderNode = (RenderGraphNode*)node;
            if (renderNode->type == EObjectType::Pass)
            {
                isLastUser = isLastUser && pass->mOrder >= ((PassNode*)renderNode)->mOrder;
            }
        });
        if (isLastUser)
        {
            mTexturePool.Deallocate(texture->mDesc, texture->m_pFrameTexture, edge->mRequestedState, {mFrameIndex, 0});
        }
    });

    // for each buffer
    pass->ForeachBuffer([this, pass](BufferNode* bufferNode, BufferEdge* buffreEdge)
    {
        if (bufferNode->mImported) return;
        bool isLastUser = true;
        bufferNode->ForeachNeighbors([&](DependencyGraphNode* node)
        {
            RenderGraphNode* renderNode = (RenderGraphNode*)node;
            if (renderNode->type == EObjectType::Pass)
            {
                isLastUser = isLastUser && pass->mOrder >= ((PassNode*)renderNode)->mOrder;
            }
        });
        if (isLastUser)
        {
            mBufferPool.Deallocate(bufferNode->mDesc, bufferNode->m_pBuffer, buffreEdge->mRequestedState, {mFrameIndex, 0});
        }
    });
}

uint64_t RenderGraphBackend::GetLatestFinishedFrame()
{
    if (mFrameIndex < RG_MAX_FRAME_IN_FLIGHT) return 0;

    uint64_t result = mFrameIndex - RG_MAX_FRAME_IN_FLIGHT;
    for (auto&& executor : mExecutors)
    {
        if (!executor.m_pFence) continue;
        if (GPUQueryFenceStatus(executor.m_pFence) == GPU_FENCE_STATUS_COMPLETE)
        {
            result = std::max(result, executor.mExecFrame);
        }
    }
    return result;
}
//////////////////RenderGraphBackend////////////////////////