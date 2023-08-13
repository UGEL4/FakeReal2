#pragma once
#include "model.hpp"
#include "utils.hpp"

class SkyBox
{
public:
    SkyBox()
    {
        mVertices = Sphere::GenCubeIdentityVertices();
        mIndices  = Sphere::GenCubeIndices();
        mTextureData = new HDRIBLCubeMapTextureData;
    }

    ~SkyBox()
    {
        mVertices.clear();
        mIndices.clear();
        delete mTextureData;

        if (mVertexBuffer) GPUFreeBuffer(mVertexBuffer);
        if (mIndexBuffer) GPUFreeBuffer(mIndexBuffer);
        if (mUniformBuffer) GPUFreeBuffer(mUniformBuffer);
        if (mSet) GPUFreeDescriptorSet(mSet);
        if (mRS) GPUFreeRootSignature(mRS);
        if (mPipeline) GPUFreeRenderPipeline(mPipeline);
    }

    void Load(std::array<std::string, 6>& files, GPUDeviceID device, GPUQueueID gfxQueue, EGPUFormat format, uint32_t desiredChannels = 4)
    {
        mTextureData->Load(files, device, gfxQueue, format, desiredChannels);
    }

    void Draw(GPURenderPassEncoderID encoder, const glm::mat4& view, const glm::mat4& proj)
    {
        UniformBuffer ubo = {
            .model = glm::mat4(glm::mat3(view)),
            .proj  = proj
        };
        GPUBufferRange rang{};
        rang.offset = 0;
        rang.size   = sizeof(UniformBuffer);
        GPUMapBuffer(mUniformBuffer, &rang);
        memcpy(mUniformBuffer->cpu_mapped_address, &ubo, rang.size);
        GPUUnmapBuffer(mUniformBuffer);

        GPURenderEncoderBindPipeline(encoder, mPipeline);
        GPURenderEncoderBindDescriptorSet(encoder, mSet);
        uint32_t strides = sizeof(Vertex);
        GPURenderEncoderBindVertexBuffers(encoder, 1, &mVertexBuffer, &strides, nullptr);
        GPURenderEncoderBindIndexBuffer(encoder, mIndexBuffer, 0, sizeof(uint32_t));
        GPURenderEncoderDrawIndexed(encoder, mIndices.size(), 0, 0);
    }

public:
    void InitVertexAndIndexResource(GPUDeviceID device, GPUQueueID gfxQueue)
    {
        GPUBufferDescriptor v_desc = {
            .size         = sizeof(Vertex) * (uint32_t)mVertices.size(),
            .descriptors  = GPU_RESOURCE_TYPE_VERTEX_BUFFER,
            .memory_usage = GPU_MEM_USAGE_GPU_ONLY,
            .flags        = GPU_BCF_OWN_MEMORY_BIT
        };
        mVertexBuffer = GPUCreateBuffer(device, &v_desc);

        GPUBufferDescriptor i_desc = {
            .size         = sizeof(uint32_t) * (uint32_t)mIndices.size(),
            .descriptors  = GPU_RESOURCE_TYPE_INDEX_BUFFER,
            .memory_usage = GPU_MEM_USAGE_GPU_ONLY,
            .flags        = GPU_BCF_OWN_MEMORY_BIT
        };
        mIndexBuffer = GPUCreateBuffer(device, &i_desc);

        uint32_t uploadBufferSize = v_desc.size > i_desc.size ? v_desc.size : i_desc.size;
        GPUBufferDescriptor upload_buffer = {
            .size         = uploadBufferSize,
            .descriptors  = GPU_RESOURCE_TYPE_NONE,
            .memory_usage = GPU_MEM_USAGE_CPU_ONLY,
            .flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT,
        };
        GPUBufferID uploadBuffer = GPUCreateBuffer(device, &upload_buffer);

        GPUCommandPoolID pool = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmd_desc = {
            .isSecondary = false
        };
        GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmd_desc);

        memcpy(uploadBuffer->cpu_mapped_address, mVertices.data(), v_desc.size);

        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUBufferToBufferTransfer trans_desc = {
                .dst        = mVertexBuffer,
                .dst_offset = 0,
                .src        = uploadBuffer,
                .src_offset = 0,
                .size       = v_desc.size
            };
            GPUCmdTransferBufferToBuffer(cmd, &trans_desc);
            GPUBufferBarrier barrier = {
                .buffer    = mVertexBuffer,
                .src_state = GPU_RESOURCE_STATE_COPY_DEST,
                .dst_state = GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
            };
            GPUResourceBarrierDescriptor rs_barrer = {
                .buffer_barriers       = &barrier,
                .buffer_barriers_count = 1
            };
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        GPUQueueSubmitDescriptor cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &cpy_submit);
        GPUWaitQueueIdle(gfxQueue);

        memcpy(uploadBuffer->cpu_mapped_address, mIndices.data(), i_desc.size);
        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUBufferToBufferTransfer trans_desc = {
                .dst        = mIndexBuffer,
                .dst_offset = 0,
                .src        = uploadBuffer,
                .src_offset = 0,
                .size       = i_desc.size
            };
            GPUCmdTransferBufferToBuffer(cmd, &trans_desc);
            GPUBufferBarrier barrier = {
                .buffer    = mIndexBuffer,
                .src_state = GPU_RESOURCE_STATE_COPY_DEST,
                .dst_state = GPU_RESOURCE_STATE_INDEX_BUFFER
            };
            GPUResourceBarrierDescriptor rs_barrer = {
                .buffer_barriers       = &barrier,
                .buffer_barriers_count = 1
            };
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        GPUSubmitQueue(gfxQueue, &cpy_submit);
        GPUWaitQueueIdle(gfxQueue);

        GPUFreeBuffer(uploadBuffer);
        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);
    }

    struct UniformBuffer
    {
        glm::mat4 model;
        glm::mat4 proj;
    };

    void CreateRenderPipeline(GPUDeviceID device, GPUSamplerID staticSampler, const char8_t* samplerName, EGPUFormat colorFormat)
    {
        // shader
        uint32_t* vShaderCode;
        uint32_t vSize = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/skybox.vert", &vShaderCode, &vSize);
        uint32_t* fShaderCode;
        uint32_t fSize = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/skybox.frag", &fShaderCode, &fSize);
        GPUShaderLibraryDescriptor vShaderDesc = {
            .pName    = u8"skybox_vertex_shader",
            .code     = vShaderCode,
            .codeSize = vSize,
            .stage    = GPU_SHADER_STAGE_VERT
        };
        GPUShaderLibraryDescriptor fShaderDesc = {
            .pName    = u8"skybox_fragment_shader",
            .code     = fShaderCode,
            .codeSize = fSize,
            .stage    = GPU_SHADER_STAGE_FRAG
        };
        GPUShaderLibraryID pVShader = GPUCreateShaderLibrary(device, &vShaderDesc);
        GPUShaderLibraryID pFShader = GPUCreateShaderLibrary(device, &fShaderDesc);
        free(vShaderCode);
        free(fShaderCode);

        GPUShaderEntryDescriptor shaderEntries[2] = { 0 };
        {
            shaderEntries[0].stage    = GPU_SHADER_STAGE_VERT;
            shaderEntries[0].entry    = u8"main";
            shaderEntries[0].pLibrary = pVShader;
            shaderEntries[1].stage    = GPU_SHADER_STAGE_FRAG;
            shaderEntries[1].entry    = u8"main";
            shaderEntries[1].pLibrary = pFShader;
        }

        // create root signature
        GPURootSignatureDescriptor rootRSDesc = {
            .shaders              = shaderEntries,
            .shader_count         = 2,
            .static_samplers      = &staticSampler,
            .static_sampler_names = &samplerName,
            .static_sampler_count = 1
        };
        mRS = GPUCreateRootSignature(device, &rootRSDesc);

        // create descriptorset
        GPUDescriptorSetDescriptor set_desc{};
        set_desc.root_signature = mRS;
        set_desc.set_index      = 0;
        mSet                    = GPUCreateDescriptorSet(device, &set_desc);

        // vertex layout
        GPUVertexLayout vertexLayout{};
        vertexLayout.attributeCount = 3;
        vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
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
        GPUDepthStateDesc depthDesc = {
            .depthTest  = false,
            .depthWrite = false,
            .depthFunc  = GPU_CMP_LEQUAL
        };
        GPURenderPipelineDescriptor pipelineDesc{};
        {
            pipelineDesc.pRootSignature     = mRS;
            pipelineDesc.pVertexShader      = &shaderEntries[0];
            pipelineDesc.pFragmentShader    = &shaderEntries[1];
            pipelineDesc.pVertexLayout      = &vertexLayout;
            pipelineDesc.primitiveTopology  = GPU_PRIM_TOPO_TRI_LIST;
            pipelineDesc.pDepthState        = &depthDesc;
            pipelineDesc.pRasterizerState   = &rasterizerState;
            pipelineDesc.pColorFormats      = const_cast<EGPUFormat*>(&colorFormat);
            pipelineDesc.renderTargetCount  = 1;
            pipelineDesc.depthStencilFormat = GPU_FORMAT_D32_SFLOAT;
        }
        mPipeline = GPUCreateRenderPipeline(device, &pipelineDesc);
        GPUFreeShaderLibrary(pVShader);
        GPUFreeShaderLibrary(pFShader);

        //update descriptorset
        GPUBufferDescriptor uniform_buffer{};
        uniform_buffer.size             = sizeof(UniformBuffer);
        uniform_buffer.flags            = GPU_BCF_NONE;
        uniform_buffer.descriptors      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
        uniform_buffer.memory_usage     = GPU_MEM_USAGE_CPU_TO_GPU;
        uniform_buffer.prefer_on_device = true;
        mUniformBuffer                  = GPUCreateBuffer(device, &uniform_buffer);

        UpdateDescriptorSet();
    }

private:
    void UpdateDescriptorSet()
    {
        GPUDescriptorData desc_data[2] = {};
        desc_data[0].name              = u8"tex"; // shader texture2D`s name
        desc_data[0].binding           = 1;
        desc_data[0].binding_type      = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
        desc_data[0].count             = 1;
        desc_data[0].textures          = &mTextureData->mTextureView;
        desc_data[1].name              = u8"ubo";
        desc_data[1].binding           = 0;
        desc_data[1].binding_type      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
        desc_data[1].count             = 1;
        desc_data[1].buffers           = &mUniformBuffer;
        GPUUpdateDescriptorSet(mSet, desc_data, 2);
    }

public:
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;
    HDRIBLCubeMapTextureData* mTextureData{ nullptr };
    GPUBufferID mVertexBuffer{ nullptr };
    GPUBufferID mIndexBuffer{ nullptr };
    GPURenderPipelineID mPipeline{ nullptr };
    GPURootSignatureID mRS{ nullptr };
    GPUDescriptorSetID mSet{ nullptr };
    GPUBufferID mUniformBuffer{ nullptr };
};
