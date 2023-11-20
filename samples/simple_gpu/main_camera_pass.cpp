#include "main_camera_pass.hpp"
#include "model_entity.hpp"
#include "camera.hpp"
#include "cascade_shadow_pass.hpp"
#include "sky_box.hpp"
#include "global_resources.hpp"

MainCameraPass::MainCameraPass()
{

}

MainCameraPass::~MainCameraPass()
{
    if (mShadowMapSet) GPUFreeDescriptorSet(mShadowMapSet); mShadowMapSet = nullptr;
    if (mUBO) GPUFreeBuffer(mUBO); mUBO = nullptr;
    if (mDefaultMeshDescriptorSet) GPUFreeDescriptorSet(mDefaultMeshDescriptorSet); mDefaultMeshDescriptorSet = nullptr;
    //if (mUploadStorageBuffer.buffer) GPUFreeBuffer(mUploadStorageBuffer.buffer); mUploadStorageBuffer.buffer = nullptr;
    if (mDepthTexView) GPUFreeTextureView(mDepthTexView); mDepthTexView = nullptr;
    if (mDepthTex) GPUFreeTexture(mDepthTex); mDepthTex = nullptr;
    if (mPbrPipeline) GPUFreeRenderPipeline(mPbrPipeline); mPbrPipeline = nullptr;
    if (mRootSignature) GPUFreeRootSignature(mRootSignature); mRootSignature = nullptr;
}

void MainCameraPass::Initialize(GPUDeviceID device, GPUQueueID gfxQueue, GPUSwapchainID swapchain,
                                GPUTextureViewID* swapchainImages,
                                GPUSemaphoreID presentSemaphore,
                                GPUFenceID* presenFences,
                                GPUCommandPoolID* cmdPools,
                                GPUCommandBufferID* cmds,
                                const SkyBox* skyBox)
{
    mDevice           = device;
    mGfxQueue         = gfxQueue;
    mSwapchain        = swapchain;
    mSwapchainImages  = swapchainImages;
    mPresentSemaphore = presentSemaphore;
    mPresenFences     = presenFences;
    mCmdPools         = cmdPools;
    mCmds             = cmds;
    mSkyBoxRef        = skyBox;

    GPUTextureDescriptor depth_tex_desc{
        .flags       = GPU_TCF_OWN_MEMORY_BIT,
        .width       = swapchain->ppBackBuffers[0]->width,
        .height      = swapchain->ppBackBuffers[0]->height,
        .depth       = 1,
        .array_size  = 1,
        .format      = GPU_FORMAT_D32_SFLOAT,
        .owner_queue = mGfxQueue,
        .start_state = GPU_RESOURCE_STATE_DEPTH_WRITE,
        .descriptors = GPU_RESOURCE_TYPE_DEPTH_STENCIL
    };
    mDepthTex = GPUCreateTexture(device, &depth_tex_desc);
    GPUTextureViewDescriptor depth_tex_view_desc{
        .pTexture        = mDepthTex,
        .format          = GPU_FORMAT_D32_SFLOAT,
        .usages          = EGPUTexutreViewUsage::GPU_TVU_RTV_DSV,
        .aspectMask      = EGPUTextureViewAspect::GPU_TVA_DEPTH,
        .baseMipLevel    = 0,
        .mipLevelCount   = 1,
        .baseArrayLayer  = 0,
        .arrayLayerCount = 1
    };
    mDepthTexView = GPUCreateTextureView(device, &depth_tex_view_desc);

    SetupRenderPipeline();

    //default descriptorset
    GPUDescriptorSetDescriptor setDesc{
        .root_signature = mRootSignature,
        .set_index = 0
    };
    mDefaultMeshDescriptorSet = GPUCreateDescriptorSet(mDevice, &setDesc);
    GPUDescriptorData dataDesc[6]      = {};
    dataDesc[0].binding                = 0;
    dataDesc[0].binding_type           = GPU_RESOURCE_TYPE_RW_BUFFER_RAW;
    dataDesc[0].buffers                = &global::g_global_reader_resource.storage.buffer;
    uint64_t offsets[1]                = { 0 };
    uint64_t ranges[1]                 = { sizeof(global::MeshPerdrawcallStorageBufferObject) };
    dataDesc[0].buffers_params.offsets = offsets;
    dataDesc[0].buffers_params.sizes   = ranges;
    dataDesc[0].count                  = 1;

    dataDesc[1].binding           = 1;
    dataDesc[1].binding_type      = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
    dataDesc[1].textures          = &mSkyBoxRef->mIrradianceMap->mTextureView;
    dataDesc[1].count             = 1;

    dataDesc[2].binding           = 2;
    dataDesc[2].binding_type      = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
    dataDesc[2].textures          = &mSkyBoxRef->mPrefilteredMap->mTextureView;
    dataDesc[2].count             = 1;

    dataDesc[3].binding           = 3;
    dataDesc[3].binding_type      = GPU_RESOURCE_TYPE_TEXTURE;
    dataDesc[3].textures          = &mSkyBoxRef->mBRDFLut->mTextureView;
    dataDesc[3].count             = 1;

    dataDesc[4].binding           = 4;
    dataDesc[4].binding_type      = GPU_RESOURCE_TYPE_SAMPLER;
    dataDesc[4].samplers          = &(const_cast<SkyBox*>(mSkyBoxRef))->mSamplerRef;
    dataDesc[4].count             = 1;

    dataDesc[5].binding           = 5;
    dataDesc[5].binding_type      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    dataDesc[5].buffers           = &mUBO;
    dataDesc[5].count             = 1;
    GPUUpdateDescriptorSet(mDefaultMeshDescriptorSet, dataDesc, sizeof(dataDesc) / sizeof(dataDesc[0]));

}

void MainCameraPass::DrawForward(const EntityModel* modelEntity, const Camera* cam, const CascadeShadowPass* shadowPass)
{
    UpdateVisible(cam, modelEntity);
    //reset
    global::g_global_reader_resource.Reset(mCurrFrame);

    

    GPUAcquireNextDescriptor acq_desc{};
    acq_desc.signal_semaphore        = mPresentSemaphore;
    uint32_t backbufferIndex         = GPUAcquireNextImage(mSwapchain, &acq_desc);
    GPUTextureID backbuffer          = mSwapchain->ppBackBuffers[backbufferIndex];
    GPUTextureViewID backbuffer_view = mSwapchainImages[backbufferIndex];

    GPUWaitFences(mPresenFences + backbufferIndex, 1);

    GPUCommandPoolID pool  = mCmdPools[backbufferIndex];
    GPUCommandBufferID cmd = mCmds[backbufferIndex];

    GPUResetCommandPool(pool);
    GPUCmdBegin(cmd);
    {
        DirectionalLight directionalLight{
            .direction         = FakeReal::math::Vector3(0.f, 0.5f, 0.5f),
            .padding_direction = 0.f,
            .color             = math::Vector3(1.f, 1.f, 1.f),
            .padding_color     = 0.f
        };
        // render scene
        GPUTextureBarrier tex_barrier{
            .texture   = backbuffer,
            .src_state = GPU_RESOURCE_STATE_UNDEFINED,
            .dst_state = GPU_RESOURCE_STATE_RENDER_TARGET
        };
        GPUResourceBarrierDescriptor draw_barrier{
            .texture_barriers       = &tex_barrier,
            .texture_barriers_count = 1
        };
        GPUCmdResourceBarrier(cmd, &draw_barrier);

        // pShadowPass->Draw(sceneInfo, cmd, gCamera.matrices.view, gCamera.matrices.perspective, viewPos, directLightPos, pModel->mBoundingBox);
        const_cast<CascadeShadowPass*>(shadowPass)->Draw(cmd, modelEntity, cam, directionalLight.direction, mCurrFrame, mCuller);

        // uniform
        {
            PointLight pointLight = {
                .position  = glm::vec3(-1.0f, 0.0f, 0.0f),
                .color     = glm::vec3(0.0f, 1.0f, 0.0f),
                .constant  = 1.0f,
                .linear    = 0.045f,
                .quadratic = 0.0075f
            };
            // update uniform bffer
            math::Vector4 viewPos(-cam->position, 1.0f);
            PerframeUniformBuffer ubo = {
                .view             = cam->matrices.view,
                .proj             = cam->matrices.perspective,
                .viewPos          = viewPos,
                .directionalLight = directionalLight,
                .pointLight       = pointLight
            };
            for (uint32_t i = 0; i < CascadeShadowPass::sCascadeCount; i++)
            {
                ubo.cascadeSplits[i * 4] = shadowPass->cascades[i].splitDepth;
                ubo.lightSpaceMat[i]     = shadowPass->cascades[i].viewProjMatrix;
            }
            memcpy(mUBO->cpu_mapped_address, &ubo, sizeof(ubo));
        }

        GPUColorAttachment screenAttachment{
            .view         = backbuffer_view,
            .load_action  = GPU_LOAD_ACTION_CLEAR,
            .store_action = GPU_STORE_ACTION_STORE,
            .clear_color  = { { 0.f, 0.f, 0.f, 0.f } }
        };
        GPUDepthStencilAttachment ds_attachment{
            .view               = mDepthTexView,
            .depth_load_action  = GPU_LOAD_ACTION_CLEAR,
            .depth_store_action = GPU_STORE_ACTION_STORE,
            .clear_depth        = 1.0f,
            .write_depth        = true
        };
        GPURenderPassDescriptor render_pass_desc{
            .sample_count        = GPU_SAMPLE_COUNT_1,
            .color_attachments   = &screenAttachment,
            .depth_stencil       = &ds_attachment,
            .render_target_count = 1
        };
        GPURenderPassEncoderID encoder = GPUCmdBeginRenderPass(cmd, &render_pass_desc);
        {
            GPURenderEncoderSetViewport(encoder, 0.f, (float)backbuffer->height,
                                        (float)backbuffer->width,
                                        -(float)backbuffer->height,
                                        0.f, 1.f);
            GPURenderEncoderSetScissor(encoder, 0, 0, backbuffer->width,
                                       backbuffer->height);
            
            DrawMeshLighting(encoder, modelEntity);

            // skyybox
            //const_cast<SkyBox*>(mSkyBoxRef)->Draw(encoder, cam->matrices.view, cam->matrices.perspective, viewPos);

            // pShadowPass->DebugShadow(encoder);
        }
        GPUCmdEndRenderPass(cmd, encoder);

        GPUTextureBarrier tex_barrier1{
            .texture   = backbuffer,
            .src_state = GPU_RESOURCE_STATE_RENDER_TARGET,
            .dst_state = GPU_RESOURCE_STATE_PRESENT
        };
        GPUResourceBarrierDescriptor present_barrier{
            .texture_barriers       = &tex_barrier1,
            .texture_barriers_count = 1
        };
        GPUCmdResourceBarrier(cmd, &present_barrier);
    }
    GPUCmdEnd(cmd);

    // submit
    GPUQueueSubmitDescriptor submitDesc{
        .cmds         = &cmd,
        .signal_fence = mPresenFences[backbufferIndex],
        .cmds_count   = 1
    };
    GPUSubmitQueue(mGfxQueue, &submitDesc);
    // present
    // GPUWaitQueueIdle(pGraphicQueue);
    GPUQueuePresentDescriptor presentDesc{
        .swapchain            = mSwapchain,
        .wait_semaphores      = &mPresentSemaphore,
        .wait_semaphore_count = 1,
        .index                = uint8_t(backbufferIndex)
    };
    GPUQueuePresent(mGfxQueue, &presentDesc);

    mCurrFrame = (mCurrFrame + 1) % 3;
}

void MainCameraPass::DrawMeshLighting(GPURenderPassEncoderID encoder, const EntityModel* modelEntity)
{
    struct MeshNode
    {
        FakeReal::math::Matrix4X4 modelMatrix;
    };
    //std::unordered_map<global::GlobalGPUMaterialRes*, std::unordered_map<global::GlobalGPUMeshRes*, std::vector<MeshNode>>> drawCallBatch;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<MeshNode>>> drawCallBatch;
    //
    for (auto& mesh : modelEntity->mMeshComp.rawMeshes)
    {
        global::GlobalGPUMaterialRes* material = nullptr;
        if (global::GetGpuMaterialRes(mesh.materialFile, material))
        {
            global::GlobalGPUMeshRes* refMesh = nullptr;
            if (!global::GetGpuMeshRes(mesh.meshFile, refMesh)) continue;

            auto& meshInstanced = drawCallBatch[mesh.materialFile];
            auto& meshNodes = meshInstanced[mesh.meshFile];

            TransformComponent transComp;
            transComp.transform = mesh.transform;
            MeshNode tmpNode;
            tmpNode.modelMatrix = modelEntity->mTransformComp.GetMatrix() * transComp.GetMatrix();

            meshNodes.emplace_back(tmpNode);
        }
    }

    auto roundUp = [](uint32_t value, uint32_t aligment)
    {
        uint32_t tmp = value + (aligment - 1);
        return tmp - tmp % aligment;
    };

    GPURenderEncoderBindPipeline(encoder, mPbrPipeline);

    GPURenderEncoderBindDescriptorSet(encoder, mShadowMapSet);
    for (auto& pair1 : drawCallBatch)
    {
        auto& material = pair1.first;
        auto& meshInstanced = pair1.second;

        //bind material descriptorset
        global::GlobalGPUMaterialRes* mat = nullptr;
        global::GetGpuMaterialRes(material, mat);
        GPURenderEncoderBindDescriptorSet(encoder, mat->set);

        for (auto& pair2 : meshInstanced)
        {
            auto& meshf = pair2.first;
            auto& mesh_nodes = pair2.second;

            global::GlobalGPUMeshRes* mesh = nullptr;
            global::GetGpuMeshRes(meshf, mesh);
            uint32_t totalInstanceCount = mesh_nodes.size();
            if (totalInstanceCount > 0)
            {
                uint32_t strides = sizeof(global::GlobalMeshRes::Vertex);
                GPURenderEncoderBindVertexBuffers(encoder, 1, &mesh->vertexBuffer, &strides, nullptr);
                GPURenderEncoderBindIndexBuffer(encoder, mesh->indexBuffer, 0, sizeof(uint32_t));

                uint32_t drawcallMaxInctanceCount = sizeof(global::MeshPerdrawcallStorageBufferObject::meshInstances) / sizeof(global::MeshPerdrawcallStorageBufferObject::meshInstances[0]);
                uint32_t drawCount = roundUp(totalInstanceCount, drawcallMaxInctanceCount) / drawcallMaxInctanceCount;
                for (uint32_t drawcallIndex = 0; drawcallIndex < drawCount; drawcallIndex++)
                {
                    uint32_t currInstanceCount =
                    ((totalInstanceCount - drawcallMaxInctanceCount * drawcallIndex) < drawcallMaxInctanceCount) ?
                    (totalInstanceCount - drawcallMaxInctanceCount * drawcallIndex) : drawcallMaxInctanceCount;

                    uint32_t perdrawcall_dynamic_offset                                                 = roundUp(global::g_global_reader_resource.storage._global_upload_ringbuffers_end[mCurrFrame], global::g_global_reader_resource.storage.minAlignment);
                    global::g_global_reader_resource.storage._global_upload_ringbuffers_end[mCurrFrame] = perdrawcall_dynamic_offset + sizeof(global::MeshPerdrawcallStorageBufferObject);
                    assert(global::g_global_reader_resource.storage._global_upload_ringbuffers_end[mCurrFrame] <=
                           (global::g_global_reader_resource.storage._global_upload_ringbuffers_begin[mCurrFrame] + global::g_global_reader_resource.storage._global_upload_ringbuffers_size[mCurrFrame]));
                    global::MeshPerdrawcallStorageBufferObject& perdrawcallStorageBufferObject =
                    (*reinterpret_cast<global::MeshPerdrawcallStorageBufferObject*>(reinterpret_cast<uintptr_t>(global::g_global_reader_resource.storage.buffer->cpu_mapped_address) + perdrawcall_dynamic_offset));

                    for (uint32_t i = 0; i < currInstanceCount; i++)
                    {
                        perdrawcallStorageBufferObject.meshInstances[i].model = mesh_nodes[drawcallMaxInctanceCount * drawcallIndex + i].modelMatrix;
                    }

                    //bind perdrawcall set
                    uint32_t dynamicOffsets[1] = {perdrawcall_dynamic_offset};
                    GPURenderEncoderBindDescriptorSet(encoder, mDefaultMeshDescriptorSet, 1, dynamicOffsets);

                    //draw indexed
                    GPURenderEncoderDrawIndexedInstanced(encoder, mesh->indexCount, currInstanceCount, 0, 0, 0);
                }
            }
        }
    }
}

void MainCameraPass::SetupRenderPipeline()
{
    /* GPUSamplerDescriptor sampleDesc = {
        .min_filter   = GPU_FILTER_TYPE_LINEAR,
        .mag_filter   = GPU_FILTER_TYPE_LINEAR,
        .mipmap_mode  = GPU_MIPMAP_MODE_LINEAR,
        .address_u    = GPU_ADDRESS_MODE_REPEAT,
        .address_v    = GPU_ADDRESS_MODE_REPEAT,
        .address_w    = GPU_ADDRESS_MODE_REPEAT,
        .compare_func = GPU_CMP_NEVER,
    };
    mSampler = GPUCreateSampler(mDevice, &sampleDesc); */
    const char8_t* samplerName = u8"texSamp";

    uint32_t* shaderCode;
    uint32_t size = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/pbr_instance.vert", &shaderCode, &size);
    GPUShaderLibraryDescriptor shaderDesc = {
        .pName    = u8"",
        .code     = shaderCode,
        .codeSize = size,
        .stage    = GPU_SHADER_STAGE_VERT
    };
    GPUShaderLibraryID vsShader = GPUCreateShaderLibrary(mDevice, &shaderDesc);
    free(shaderCode);
    shaderCode = nullptr;
    size = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/pbr_instance.frag", &shaderCode, &size);
    shaderDesc = {
        .pName    = u8"",
        .code     = shaderCode,
        .codeSize = size,
        .stage    = GPU_SHADER_STAGE_FRAG
    };
    GPUShaderLibraryID psShader = GPUCreateShaderLibrary(mDevice, &shaderDesc);
    free(shaderCode);
    shaderCode = nullptr;

    CGPUShaderEntryDescriptor entry_desc[2] = {};
    entry_desc[0].pLibrary = vsShader;
    entry_desc[0].entry    = u8"main";
    entry_desc[0].stage    = GPU_SHADER_STAGE_VERT;
    entry_desc[1].pLibrary = psShader;
    entry_desc[1].entry    = u8"main";
    entry_desc[1].stage    = GPU_SHADER_STAGE_FRAG;

    GPURootSignatureDescriptor rs_desc = {
        .shaders              = entry_desc,
        .shader_count         = 2,
    };
    mRootSignature = GPUCreateRootSignature(mDevice, &rs_desc);

    // vertex layout
    GPUVertexLayout vertexLayout {};
    vertexLayout.attributeCount = 5;
    vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[3]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 8, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[4]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 11, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    // renderpipeline
    GPURasterizerStateDescriptor rasterizerState = {
        .cullMode             = GPU_CULL_MODE_BACK,
        .fillMode             = GPU_FILL_MODE_SOLID,
        .frontFace            = GPU_FRONT_FACE_CCW,
        .depthBias            = 0,
        .slopeScaledDepthBias = 0.f,
        .enableMultiSample    = false,
        .enableScissor        = false,
        .enableDepthClamp     = false
    };
    GPUDepthStateDesc depthDesc = {
        .depthTest  = true,
        .depthWrite = true,
        .depthFunc  = GPU_CMP_LEQUAL
    };
    EGPUFormat swapchainFormat = (EGPUFormat)mSwapchain->ppBackBuffers[0]->format;
    GPURenderPipelineDescriptor pipelineDesc = {
        .pRootSignature     = mRootSignature,
        .pVertexShader      = &entry_desc[0],
        .pFragmentShader    = &entry_desc[1],
        .pVertexLayout      = &vertexLayout,
        .pDepthState        = &depthDesc,
        .pRasterizerState   = &rasterizerState,
        .primitiveTopology  = GPU_PRIM_TOPO_TRI_LIST,
        .pColorFormats      = const_cast<EGPUFormat*>(&swapchainFormat),
        .renderTargetCount  = 1,
        .depthStencilFormat = GPU_FORMAT_D32_SFLOAT
    };
    mPbrPipeline = GPUCreateRenderPipeline(mDevice, &pipelineDesc);
    GPUFreeShaderLibrary(vsShader);
    GPUFreeShaderLibrary(psShader);

    GPUDescriptorSetDescriptor setDesc = {
        .root_signature = mRootSignature,
        .set_index      = 2
    };
    mShadowMapSet = GPUCreateDescriptorSet(mDevice, &setDesc);

    GPUBufferDescriptor uboDesc = {
        .size             = sizeof(PerframeUniformBuffer),
        .descriptors      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER,
        .memory_usage     = GPU_MEM_USAGE_CPU_TO_GPU,
        .flags            = GPU_BCF_PERSISTENT_MAP_BIT,
        .prefer_on_device = true
    };
    mUBO = GPUCreateBuffer(mDevice, &uboDesc);
}

void MainCameraPass::UpdateShadowMapSet(GPUTextureViewID shadowMap, GPUSamplerID sampler)
{
    GPUDescriptorData dataDesc[2] = {};
    dataDesc[0].binding           = 0;
    dataDesc[0].binding_type      = GPU_RESOURCE_TYPE_TEXTURE;
    dataDesc[0].textures          = &shadowMap;
    dataDesc[0].count             = 1;
    dataDesc[1].binding           = 1;
    dataDesc[1].binding_type      = GPU_RESOURCE_TYPE_SAMPLER;
    dataDesc[1].samplers          = &sampler;
    dataDesc[1].count             = 1;
    GPUUpdateDescriptorSet(mShadowMapSet, dataDesc, 2);
}

void MainCameraPass::UpdateVisible(const Camera* cam, const EntityModel* modelEntity)
{
    mCuller.ClearVisibleSet();
    mCuller.ClearAllPanel();
    mCuller.PushCameraPlane(*const_cast<Camera*>(cam));
    math::Matrix4X4 trans = modelEntity->mTransformComp.GetMatrix();
    for (auto& comp : modelEntity->mMeshComp.rawMeshes)
    {
        auto iter = global::g_cache_mesh_bounding_box.find(comp.meshFile);
        TransformComponent meshTransComp;
        meshTransComp.transform = comp.transform;
        BoundingBox aabb = BoundingBox::BoundingBoxTransform(iter->second, trans * meshTransComp.GetMatrix());
        if (mCuller.IsVisible(aabb))
        {
            mCuller.AddVisibleAABB(aabb);
        }
    }
}

void MainCameraPass::SetupDebugPipeline()
{
    uint32_t* shaderCode;
    uint32_t size = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/debug_camera.vert", &shaderCode, &size);
    GPUShaderLibraryDescriptor shaderDesc = {
        .pName    = u8"",
        .code     = shaderCode,
        .codeSize = size,
        .stage    = GPU_SHADER_STAGE_VERT
    };
    GPUShaderLibraryID vsShader = GPUCreateShaderLibrary(mDevice, &shaderDesc);
    free(shaderCode);
    shaderCode = nullptr;
    size = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/debug_camera.frag", &shaderCode, &size);
    shaderDesc = {
        .pName    = u8"",
        .code     = shaderCode,
        .codeSize = size,
        .stage    = GPU_SHADER_STAGE_FRAG
    };
    GPUShaderLibraryID psShader = GPUCreateShaderLibrary(mDevice, &shaderDesc);
    free(shaderCode);
    shaderCode = nullptr;

    CGPUShaderEntryDescriptor entry_desc[2] = {};
    entry_desc[0].pLibrary = vsShader;
    entry_desc[0].entry    = u8"main";
    entry_desc[0].stage    = GPU_SHADER_STAGE_VERT;
    entry_desc[1].pLibrary = psShader;
    entry_desc[1].entry    = u8"main";
    entry_desc[1].stage    = GPU_SHADER_STAGE_FRAG;

    GPURootSignatureDescriptor rs_desc = {
        .shaders              = entry_desc,
        .shader_count         = 2,
    };
    mDebugRS = GPUCreateRootSignature(mDevice, &rs_desc);

    // vertex layout
    GPUVertexLayout vertexLayout {};
    vertexLayout.attributeCount = 1;
    vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    // renderpipeline
    GPURasterizerStateDescriptor rasterizerState = {
        .cullMode             = GPU_CULL_MODE_NONE,
        .fillMode             = GPU_FILL_MODE_SOLID,
        .frontFace            = GPU_FRONT_FACE_CCW,
        .depthBias            = 0,
        .slopeScaledDepthBias = 0.f,
        .enableMultiSample    = false,
        .enableScissor        = false,
        .enableDepthClamp     = false
    };

    EGPUFormat swapchainFormat = (EGPUFormat)mSwapchain->ppBackBuffers[0]->format;
    GPURenderPipelineDescriptor pipelineDesc = {
        .pRootSignature     = mDebugRS,
        .pVertexShader      = &entry_desc[0],
        .pFragmentShader    = &entry_desc[1],
        .pVertexLayout      = &vertexLayout,
        .pRasterizerState   = &rasterizerState,
        .primitiveTopology  = GPU_PRIM_TOPO_LINE_LIST,
        .pColorFormats      = const_cast<EGPUFormat*>(&swapchainFormat),
        .renderTargetCount  = 1,
    };
    mDebugCameraPipeline = GPUCreateRenderPipeline(mDevice, &pipelineDesc);
    GPUFreeShaderLibrary(vsShader);
    GPUFreeShaderLibrary(psShader);

    GPUDescriptorSetDescriptor setDesc = {
        .root_signature = mDebugRS,
        .set_index      = 0
    };
    mDebugCameraSet = GPUCreateDescriptorSet(mDevice, &setDesc);

    GPUBufferDescriptor uboDesc = {
        .size             = sizeof(DebugCameraUniform),
        .descriptors      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER,
        .memory_usage     = GPU_MEM_USAGE_CPU_TO_GPU,
        .flags            = GPU_BCF_PERSISTENT_MAP_BIT,
        .prefer_on_device = true
    };
    mDebugCameraUBO = GPUCreateBuffer(mDevice, &uboDesc);
}

void MainCameraPass::DrawCameraDebug()
{

}