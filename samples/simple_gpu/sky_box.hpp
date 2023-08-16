#pragma once
#include "model.hpp"
#include "utils.hpp"
#include "glm/gtc/matrix_transform.hpp"

class IBLCubeMapTextureData : public HDRIBLCubeMapTextureData
{
public:

    IBLCubeMapTextureData() = default;
    ~IBLCubeMapTextureData()
    {
    }

    virtual void Load(std::array<std::string, 6>& files, GPUDeviceID device, GPUQueueID gfxQueue, EGPUFormat format, bool flip, uint32_t desiredChannels = 4) override
    {
        stbi_set_flip_vertically_on_load(flip);
        int width, height, comp;
        std::array<void*, 6> pixels = {nullptr};
        for (uint32_t i = 0; i < 6; i++)
        {
            pixels[i] = stbi_load(files[i].c_str(), &width, &height, &comp, desiredChannels);
            if (!pixels[i])
            {
                assert(0 && ("Load image failed : " + files[i]).c_str());
                return;
            }
        }

        uint32_t bytes = 0;
        switch (format)
        {
            case GPU_FORMAT_R8G8B8A8_SRGB:
            case GPU_FORMAT_R8G8B8A8_UNORM:
            {
                bytes = width * height * 4;
            }
            break;
            default:
                assert(0);
                break;
        }

        uint32_t totalBytes = bytes * 6;

        GPUTextureDescriptor desc{};
        desc.flags       = GPU_TCF_OWN_MEMORY_BIT;
        desc.width       = width;
        desc.height      = height;
        desc.depth       = 1;
        desc.array_size  = 6;
        desc.format      = format;
        desc.owner_queue = gfxQueue;
        desc.start_state = GPU_RESOURCE_STATE_COPY_DEST;
        desc.descriptors = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
        mTexture         = GPUCreateTexture(device, &desc);
        GPUTextureViewDescriptor tex_view_desc{};
        tex_view_desc.pTexture        = mTexture;
        tex_view_desc.format          = (EGPUFormat)mTexture->format;
        tex_view_desc.usages          = EGPUTexutreViewUsage::GPU_TVU_SRV;
        tex_view_desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR;
        tex_view_desc.baseMipLevel    = 0;
        tex_view_desc.mipLevelCount   = 1;
        tex_view_desc.baseArrayLayer  = 0;
        tex_view_desc.arrayLayerCount = 6;
        mTextureView                  = GPUCreateTextureView(device, &tex_view_desc);

        GPUBufferDescriptor upload_buffer{};
        upload_buffer.size         = totalBytes;
        upload_buffer.flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT;
        upload_buffer.descriptors  = GPU_RESOURCE_TYPE_NONE;
        upload_buffer.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
        GPUBufferID uploadBuffer   = GPUCreateBuffer(device, &upload_buffer);
        for (uint32_t i = 0; i < 6; i++)
        {
            memcpy(((uint8_t*)(uploadBuffer->cpu_mapped_address) + bytes * i), pixels[i], bytes);
        }
        GPUCommandPoolID pool = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmdDesc{};
        cmdDesc.isSecondary    = false;
        GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmdDesc);
        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUBufferToTextureTransfer trans_texture_buffer_desc{};
            trans_texture_buffer_desc.dst                              = mTexture;
            trans_texture_buffer_desc.dst_subresource.mip_level        = 0;
            trans_texture_buffer_desc.dst_subresource.base_array_layer = 0;
            trans_texture_buffer_desc.dst_subresource.layer_count      = 6;
            trans_texture_buffer_desc.src                              = uploadBuffer;
            trans_texture_buffer_desc.src_offset                       = 0;
            GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);
            GPUTextureBarrier barrier{};
            barrier.texture = mTexture;
            barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
            GPUResourceBarrierDescriptor rs_barrer{};
            rs_barrer.texture_barriers      = &barrier;
            rs_barrer.texture_barriers_count = 1;
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        GPUQueueSubmitDescriptor texture_cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &texture_cpy_submit);
        GPUWaitQueueIdle(gfxQueue);

        GPUFreeBuffer(uploadBuffer);
        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);

        for (uint32_t i = 0; i < 6; i++)
        {
            if (pixels[i] != nullptr)
            {
                stbi_image_free(pixels[i]);
            }
        }
    }
};

class SkyBox
{
public:
    SkyBox(HDRIBLCubeMapTextureData* texture)
    {
        mVertices = Sphere::GenCubeIdentityVertices();
        mIndices  = Sphere::GenCubeIdentityIndices();
        mTextureData = texture;
    }

    ~SkyBox()
    {
        mVertices.clear();
        mIndices.clear();
        if (mTextureData) delete mTextureData;
        if (mIrradianceMap) delete mIrradianceMap;
        if (mPrefilteredMap) delete mPrefilteredMap;
        if (mBRDFLut) delete mBRDFLut;

        if (mVertexBuffer) GPUFreeBuffer(mVertexBuffer);
        if (mIndexBuffer) GPUFreeBuffer(mIndexBuffer);
        if (mUniformBuffer) GPUFreeBuffer(mUniformBuffer);
        if (mSet) GPUFreeDescriptorSet(mSet);
        if (mRS) GPUFreeRootSignature(mRS);
        if (mPipeline) GPUFreeRenderPipeline(mPipeline);
    }

    void Load(std::array<std::string, 6>& files, GPUDeviceID device, GPUQueueID gfxQueue, EGPUFormat format, bool flip, uint32_t desiredChannels = 4)
    {
        mTextureData->Load(files, device, gfxQueue, format, flip, desiredChannels);
    }

    void Draw(GPURenderPassEncoderID encoder, const glm::mat4& view, const glm::mat4& proj, const glm::vec4& viewPos)
    {
        UniformBuffer ubo = {
            .model   = glm::mat4(glm::mat3(view)),
            .proj    = proj,
            .viewPos = viewPos
        };
        GPUBufferRange rang{};
        rang.offset = 0;
        rang.size   = sizeof(UniformBuffer);
        GPUMapBuffer(mUniformBuffer, &rang);
        memcpy(mUniformBuffer->cpu_mapped_address, &ubo, rang.size);
        GPUUnmapBuffer(mUniformBuffer);

        GPURenderEncoderBindPipeline(encoder, mPipeline);
        GPURenderEncoderBindDescriptorSet(encoder, mSet);
        struct
        {
            float roughness;
        }pushData;
        pushData.roughness = (float)0;
        GPURenderEncoderPushConstant(encoder, mSet->root_signature, &pushData);
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
        glm::vec4 viewPos;
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
            .depthTest  = true,
            .depthWrite = true,
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

    void GenIBLImageFromHDR(const std::string& file, GPUDeviceID device, GPUQueueID gfxQueue, EGPUFormat format, bool flip, uint32_t desiredChannels = 4)
    {
        stbi_set_flip_vertically_on_load(flip);
        int width, height, comp;
        void* pixel = stbi_loadf(file.c_str(), &width, &height, &comp, desiredChannels);
        if (!pixel)
        {
            assert(0 && "HDR texture load field!");
            return;
        }

        uint32_t bytes = 0;
        switch (format)
        {
            case GPU_FORMAT_R32G32B32_SFLOAT:
            {
                bytes = width * height * 4 * 3;
            }
            break;
            case GPU_FORMAT_R32G32B32A32_SFLOAT:
                bytes = width * height * 4 * 4;
                break;
            default:
                assert(0);
                break;
        }

        uint32_t cube_width = 512;
        GPUTextureDescriptor desc = {};
        desc.flags       = GPU_TCF_OWN_MEMORY_BIT;
        desc.width       = cube_width;
        desc.height      = cube_width;
        desc.depth       = 1;
        desc.array_size  = 6;
        desc.format      = format;
        desc.owner_queue = gfxQueue;
        desc.start_state = GPU_RESOURCE_STATE_COPY_DEST;
        desc.descriptors = GPU_RESOURCE_TYPE_TEXTURE_CUBE;
        mTextureData->mTexture = GPUCreateTexture(device, &desc);
        GPUTextureViewDescriptor tex_view_desc = {};
        tex_view_desc.pTexture        = mTextureData->mTexture;
        tex_view_desc.format          = (EGPUFormat)mTextureData->mTexture->format;
        tex_view_desc.usages          = EGPUTexutreViewUsage::GPU_TVU_SRV;
        tex_view_desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR;
        tex_view_desc.baseMipLevel    = 0;
        tex_view_desc.mipLevelCount   = 1;
        tex_view_desc.baseArrayLayer  = 0;
        tex_view_desc.arrayLayerCount = 6;
        mTextureData->mTextureView    = GPUCreateTextureView(device, &tex_view_desc);

        //temp resource
        desc = {};
        desc.flags               = GPU_TCF_OWN_MEMORY_BIT;
        desc.width               = width;
        desc.height              = height;
        desc.depth               = 1;
        desc.array_size          = 1;
        desc.format              = format;
        desc.owner_queue         = gfxQueue;
        desc.start_state         = GPU_RESOURCE_STATE_COPY_DEST;
        desc.descriptors         = GPU_RESOURCE_TYPE_TEXTURE;
        GPUTextureID tempTexture = GPUCreateTexture(device, &desc);
        tex_view_desc = {};
        tex_view_desc.pTexture           = tempTexture;
        tex_view_desc.format             = format;
        tex_view_desc.usages             = EGPUTexutreViewUsage::GPU_TVU_SRV;
        tex_view_desc.aspectMask         = EGPUTextureViewAspect::GPU_TVA_COLOR;
        tex_view_desc.baseMipLevel       = 0;
        tex_view_desc.mipLevelCount      = 1;
        tex_view_desc.baseArrayLayer     = 0;
        tex_view_desc.arrayLayerCount    = 1;
        GPUTextureViewID tempTextureView = GPUCreateTextureView(device, &tex_view_desc);

        GPUBufferDescriptor upload_buffer{};
        upload_buffer.size         = bytes;
        upload_buffer.flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT;
        upload_buffer.descriptors  = GPU_RESOURCE_TYPE_NONE;
        upload_buffer.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
        GPUBufferID uploadBuffer   = GPUCreateBuffer(device, &upload_buffer);
        memcpy(uploadBuffer->cpu_mapped_address, pixel, bytes);
        GPUCommandPoolID pool = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmdDesc{};
        cmdDesc.isSecondary    = false;
        GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmdDesc);
        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUBufferToTextureTransfer trans_texture_buffer_desc{};
            trans_texture_buffer_desc.dst                              = tempTexture;
            trans_texture_buffer_desc.dst_subresource.mip_level        = 0;
            trans_texture_buffer_desc.dst_subresource.base_array_layer = 0;
            trans_texture_buffer_desc.dst_subresource.layer_count      = 1;
            trans_texture_buffer_desc.src                              = uploadBuffer;
            trans_texture_buffer_desc.src_offset                       = 0;
            GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);
            GPUTextureBarrier barrier{};
            barrier.texture   = tempTexture;
            barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
            barrier.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
            GPUResourceBarrierDescriptor rs_barrer{};
            rs_barrer.texture_barriers       = &barrier;
            rs_barrer.texture_barriers_count = 1;
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        GPUQueueSubmitDescriptor texture_cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &texture_cpy_submit);
        GPUWaitQueueIdle(gfxQueue);
        GPUFreeBuffer(uploadBuffer);

        stbi_image_free(pixel);

        struct
        {
            GPUTextureID texture;
            GPUTextureViewID texView;
        } offScreen;
        desc = {};
        desc.flags               = GPU_TCF_OWN_MEMORY_BIT;
        desc.width               = cube_width;
        desc.height              = cube_width;
        desc.depth               = 1;
        desc.array_size          = 1;
        desc.format              = format;
        desc.owner_queue         = gfxQueue;
        desc.start_state         = GPU_RESOURCE_STATE_RENDER_TARGET;
        desc.descriptors         = GPU_RESOURCE_TYPE_TEXTURE | GPU_RESOURCE_TYPE_RENDER_TARGET;
        offScreen.texture = GPUCreateTexture(device, &desc);
        tex_view_desc = {};
        tex_view_desc.pTexture           = offScreen.texture;
        tex_view_desc.format             = format;
        tex_view_desc.usages             = EGPUTexutreViewUsage::GPU_TVU_RTV_DSV;
        tex_view_desc.aspectMask         = EGPUTextureViewAspect::GPU_TVA_COLOR;
        tex_view_desc.baseMipLevel       = 0;
        tex_view_desc.mipLevelCount      = 1;
        tex_view_desc.baseArrayLayer     = 0;
        tex_view_desc.arrayLayerCount    = 1;
        offScreen.texView = GPUCreateTextureView(device, &tex_view_desc);

        // shader
        uint32_t* vShaderCode;
        uint32_t vSize = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/gen_cube.vert", &vShaderCode, &vSize);
        uint32_t* fShaderCode;
        uint32_t fSize = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/gen_cube.frag", &fShaderCode, &fSize);
        GPUShaderLibraryDescriptor vShaderDesc = {
            .pName    = u8"gen_cube_vertex_shader",
            .code     = vShaderCode,
            .codeSize = vSize,
            .stage    = GPU_SHADER_STAGE_VERT
        };
        GPUShaderLibraryDescriptor fShaderDesc = {
            .pName    = u8"gen_cube_fragment_shader",
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

        GPUSamplerDescriptor sampler_desc = {
            .min_filter   = GPU_FILTER_TYPE_LINEAR,
            .mag_filter   = GPU_FILTER_TYPE_LINEAR,
            .mipmap_mode  = GPU_MIPMAP_MODE_LINEAR,
            .address_u    = GPU_ADDRESS_MODE_REPEAT,
            .address_v    = GPU_ADDRESS_MODE_REPEAT,
            .address_w    = GPU_ADDRESS_MODE_REPEAT,
            .compare_func = GPU_CMP_NEVER
        };
        GPUSamplerID staticSampler  = GPUCreateSampler(device, &sampler_desc);
        const char8_t* sampler_name = u8"texSamp";

        // create root signature
        GPURootSignatureDescriptor rootRSDesc = {
            .shaders              = shaderEntries,
            .shader_count         = 2,
            .static_samplers      = &staticSampler,
            .static_sampler_names = &sampler_name,
            .static_sampler_count = 1
        };
        GPURootSignatureID rs = GPUCreateRootSignature(device, &rootRSDesc);

        // create descriptorset
        GPUDescriptorSetDescriptor set_desc{};
        set_desc.root_signature = rs;
        set_desc.set_index      = 0;
        GPUDescriptorSetID set  = GPUCreateDescriptorSet(device, &set_desc);

        // vertex layout
        GPUVertexLayout vertexLayout{};
        vertexLayout.attributeCount = 3;
        vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
        // renderpipeline
        GPURasterizerStateDescriptor rasterizerState = {
            .cullMode             = GPU_CULL_MODE_BACK,
            .fillMode             = GPU_FILL_MODE_SOLID,
            .frontFace            = GPU_FRONT_FACE_CW,
            .depthBias            = 0,
            .slopeScaledDepthBias = 0.f,
            .enableMultiSample    = false,
            .enableScissor        = false,
            .enableDepthClamp     = false
        };
        GPURenderPipelineDescriptor pipelineDesc{};
        {
            pipelineDesc.pRootSignature    = rs;
            pipelineDesc.pVertexShader     = &shaderEntries[0];
            pipelineDesc.pFragmentShader   = &shaderEntries[1];
            pipelineDesc.pVertexLayout     = &vertexLayout;
            pipelineDesc.primitiveTopology = GPU_PRIM_TOPO_TRI_LIST;
            pipelineDesc.pRasterizerState  = &rasterizerState;
            pipelineDesc.pColorFormats     = const_cast<EGPUFormat*>(&format);
            pipelineDesc.renderTargetCount = 1;
        }
        GPURenderPipelineID tempPipeline = GPUCreateRenderPipeline(device, &pipelineDesc);
        GPUFreeShaderLibrary(pVShader);
        GPUFreeShaderLibrary(pFShader);

        GPUDescriptorData desc_data[1] = {};
        {
            desc_data[0].name         = u8"tex"; // shader texture2D`s name
            desc_data[0].binding      = 0;
            desc_data[0].binding_type = GPU_RESOURCE_TYPE_TEXTURE;
            desc_data[0].count        = 1;
            desc_data[0].textures     = &tempTextureView;
        }
        GPUUpdateDescriptorSet(set, desc_data, 1);

        // render
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
        glm::mat4 captureViews[]    = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
        };

        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUColorAttachment screenAttachment{};
            screenAttachment.view         = offScreen.texView;
            screenAttachment.load_action  = GPU_LOAD_ACTION_CLEAR;
            screenAttachment.store_action = GPU_STORE_ACTION_STORE;
            screenAttachment.clear_color  = { { 0.f, 0.f, 0.f, 0.f } };
            GPURenderPassDescriptor render_pass_desc{};
            render_pass_desc.sample_count        = GPU_SAMPLE_COUNT_1;
            render_pass_desc.color_attachments   = &screenAttachment;
            render_pass_desc.render_target_count = 1;
            for (uint32_t i = 0; i < 6; i++)
            {
                GPURenderPassEncoderID encoder = GPUCmdBeginRenderPass(cmd, &render_pass_desc);
                {
                    GPURenderEncoderSetViewport(encoder, 0.f, (float)0,
                                                (float)offScreen.texture->width,
                                                (float)offScreen.texture->height,
                                                0.f, 1.f);
                    GPURenderEncoderSetScissor(encoder, 0, 0, offScreen.texture->width, offScreen.texture->height);

                    GPURenderEncoderBindPipeline(encoder, tempPipeline);
                    GPURenderEncoderBindDescriptorSet(encoder, set);

                    struct PushData
                    {
                        glm::mat4 view;
                        glm::mat4 proj;
                    } push_constant;
                    push_constant.view = captureViews[i];
                    push_constant.proj = captureProjection;
                    GPURenderEncoderPushConstant(encoder, rs, &push_constant);
                    uint32_t strides = sizeof(Vertex);
                    GPURenderEncoderBindVertexBuffers(encoder, 1, &mVertexBuffer, &strides, nullptr);
                    GPURenderEncoderBindIndexBuffer(encoder, mIndexBuffer, 0, sizeof(uint32_t));
                    GPURenderEncoderDrawIndexed(encoder, mIndices.size(), 0, 0);
                }
                GPUCmdEndRenderPass(cmd, encoder);

                GPUTextureBarrier tex_barrier1{};
                tex_barrier1.texture   = offScreen.texture;
                tex_barrier1.src_state = GPU_RESOURCE_STATE_RENDER_TARGET;
                tex_barrier1.dst_state = GPU_RESOURCE_STATE_COPY_SOURCE;
                GPUResourceBarrierDescriptor barrier{};
                barrier.texture_barriers_count = 1;
                barrier.texture_barriers       = &tex_barrier1;
                GPUCmdResourceBarrier(cmd, &barrier);

                GPUTextureSubresource src_sub_rang = {
                    .aspects          = GPU_TVA_COLOR,
                    .mip_level        = 0,
                    .base_array_layer = 0,
                    .layer_count      = 1
                };
                GPUTextureSubresource dst_sub_rang = {
                    .aspects          = GPU_TVA_COLOR,
                    .mip_level        = 0,
                    .base_array_layer = i,
                    .layer_count      = 1
                };
                GPUTextureToTextureTransfer trans = {
                    .src             = offScreen.texture,
                    .dst             = mTextureData->mTexture,
                    .src_subresource = src_sub_rang,
                    .dst_subresource = dst_sub_rang
                };
                GPUCmdTransferTextureToTexture(cmd, &trans);

                GPUTextureBarrier tex_barrier2{};
                tex_barrier2.texture   = offScreen.texture;
                tex_barrier2.src_state = GPU_RESOURCE_STATE_COPY_SOURCE;
                tex_barrier2.dst_state = GPU_RESOURCE_STATE_RENDER_TARGET;
                GPUResourceBarrierDescriptor barrier2{};
                barrier2.texture_barriers_count = 1;
                barrier2.texture_barriers       = &tex_barrier2;
                GPUCmdResourceBarrier(cmd, &barrier2);
            }
            GPUTextureBarrier final_barrier{};
            final_barrier.texture   = mTextureData->mTexture;
            final_barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
            final_barrier.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
            GPUResourceBarrierDescriptor rs_barrer{};
            rs_barrer.texture_barriers       = &final_barrier;
            rs_barrer.texture_barriers_count = 1;
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        // submit
        GPUQueueSubmitDescriptor submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &submit);
        GPUWaitQueueIdle(gfxQueue);
        // render

        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);
        GPUFreeDescriptorSet(set);
        GPUFreeTextureView(offScreen.texView);
        GPUFreeTexture(offScreen.texture);
        GPUFreeTextureView(tempTextureView);
        GPUFreeTexture(tempTexture);
        GPUFreeSampler(staticSampler);
        GPUFreeRootSignature(rs);
        GPUFreeRenderPipeline(tempPipeline);
    }

    void GenIrradianceCubeMap(GPUDeviceID device, GPUQueueID gfxQueue)
    {
        mIrradianceMap      = new HDRIBLCubeMapTextureData;
        EGPUFormat format   = GPU_FORMAT_R32G32B32A32_SFLOAT;
        uint32_t dims       = 64;
        uint32_t mipLevels  = static_cast<uint32_t>(std::floor(std::log2(dims))) + 1;

        GPUTextureDescriptor desc = {
            .flags       = GPU_TCF_OWN_MEMORY_BIT,
            .width       = dims,
            .height      = dims,
            .depth       = 1,
            .array_size  = 6,
            .format      = format,
            .mip_levels  = mipLevels,
            .owner_queue = gfxQueue,
            .start_state = GPU_RESOURCE_STATE_COPY_DEST,
            .descriptors = GPU_RESOURCE_TYPE_TEXTURE_CUBE
        };
        mIrradianceMap->mTexture = GPUCreateTexture(device, &desc);
        GPUTextureViewDescriptor tex_view_desc = {
            .pTexture        = mIrradianceMap->mTexture,
            .format          = format,
            .usages          = EGPUTexutreViewUsage::GPU_TVU_SRV,
            .aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR,
            .baseMipLevel    = 0,
            .mipLevelCount   = mipLevels,
            .baseArrayLayer  = 0,
            .arrayLayerCount = 6
        };
        mIrradianceMap->mTextureView = GPUCreateTextureView(device, &tex_view_desc);

        struct
        {
            GPUTextureID texture;
            GPUTextureViewID texView;
        } offScreen;
        desc = {
            .flags       = GPU_TCF_OWN_MEMORY_BIT,
            .width       = dims,
            .height      = dims,
            .depth       = 1,
            .array_size  = 1,
            .format      = format,
            .owner_queue = gfxQueue,
            .start_state = GPU_RESOURCE_STATE_RENDER_TARGET,
            .descriptors = GPU_RESOURCE_TYPE_TEXTURE | GPU_RESOURCE_TYPE_RENDER_TARGET
        };
        offScreen.texture = GPUCreateTexture(device, &desc);
        tex_view_desc = {
            .pTexture        = offScreen.texture,
            .format          = format,
            .usages          = EGPUTexutreViewUsage::GPU_TVU_RTV_DSV,
            .aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR,
            .baseMipLevel    = 0,
            .mipLevelCount   = 1,
            .baseArrayLayer  = 0,
            .arrayLayerCount = 1
        };
        offScreen.texView = GPUCreateTextureView(device, &tex_view_desc);

        // shader
        uint32_t* vShaderCode;
        uint32_t vSize = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/gen_irradiance_cube.vert", &vShaderCode, &vSize);
        uint32_t* fShaderCode;
        uint32_t fSize = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/gen_irradiance_cube.frag", &fShaderCode, &fSize);
        GPUShaderLibraryDescriptor vShaderDesc = {
            .pName    = u8"gen_irradiance_cube_vertex_shader",
            .code     = vShaderCode,
            .codeSize = vSize,
            .stage    = GPU_SHADER_STAGE_VERT
        };
        GPUShaderLibraryDescriptor fShaderDesc = {
            .pName    = u8"gen_irradiance_cube_fragment_shader",
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

        GPUSamplerDescriptor sampler_desc = {
            .min_filter     = GPU_FILTER_TYPE_LINEAR,
            .mag_filter     = GPU_FILTER_TYPE_LINEAR,
            .mipmap_mode    = GPU_MIPMAP_MODE_LINEAR,
            .address_u      = GPU_ADDRESS_MODE_REPEAT,
            .address_v      = GPU_ADDRESS_MODE_REPEAT,
            .address_w      = GPU_ADDRESS_MODE_REPEAT,
            //.max_anisotropy = 1.0f,
            .compare_func   = GPU_CMP_NEVER
        };
        GPUSamplerID staticSampler  = GPUCreateSampler(device, &sampler_desc);
        const char8_t* sampler_name = u8"texSamp";

        // create root signature
        GPURootSignatureDescriptor rootRSDesc = {
            .shaders              = shaderEntries,
            .shader_count         = 2,
            .static_samplers      = &staticSampler,
            .static_sampler_names = &sampler_name,
            .static_sampler_count = 1
        };
        GPURootSignatureID rs = GPUCreateRootSignature(device, &rootRSDesc);

        // create descriptorset
        GPUDescriptorSetDescriptor set_desc = {
            .root_signature = rs,
            .set_index      = 0
        };
        GPUDescriptorSetID set = GPUCreateDescriptorSet(device, &set_desc);

        // vertex layout
        GPUVertexLayout vertexLayout{};
        vertexLayout.attributeCount = 3;
        vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
        // renderpipeline
        GPURasterizerStateDescriptor rasterizerState = {
            .cullMode             = GPU_CULL_MODE_BACK,
            .fillMode             = GPU_FILL_MODE_SOLID,
            .frontFace            = GPU_FRONT_FACE_CW,
            .depthBias            = 0,
            .slopeScaledDepthBias = 0.f,
            .enableMultiSample    = false,
            .enableScissor        = false,
            .enableDepthClamp     = false
        };
        GPURenderPipelineDescriptor pipelineDesc = {
            .pRootSignature    = rs,
            .pVertexShader     = &shaderEntries[0],
            .pFragmentShader   = &shaderEntries[1],
            .pVertexLayout     = &vertexLayout,
            .pRasterizerState  = &rasterizerState,
            .primitiveTopology = GPU_PRIM_TOPO_TRI_LIST,
            .pColorFormats     = const_cast<EGPUFormat*>(&format),
            .renderTargetCount = 1
        };
        GPURenderPipelineID tempPipeline = GPUCreateRenderPipeline(device, &pipelineDesc);
        GPUFreeShaderLibrary(pVShader);
        GPUFreeShaderLibrary(pFShader);

        GPUDescriptorData desc_data = {};
        {
            desc_data.name         = u8"tex";
            desc_data.binding      = 0;
            desc_data.binding_type = GPU_RESOURCE_TYPE_TEXTURE;
            desc_data.count        = 1;
            desc_data.textures     = &mTextureData->mTextureView;
        }
        GPUUpdateDescriptorSet(set, &desc_data, 1);

        // render
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
        glm::mat4 captureViews[]    = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
        };

        GPUCommandPoolID pool              = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmdDesc = { .isSecondary = false };
        GPUCommandBufferID cmd             = GPUCreateCommandBuffer(pool, &cmdDesc);
        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUColorAttachment screenAttachment{};
            screenAttachment.view         = offScreen.texView;
            screenAttachment.load_action  = GPU_LOAD_ACTION_CLEAR;
            screenAttachment.store_action = GPU_STORE_ACTION_STORE;
            screenAttachment.clear_color  = { { 0.f, 0.f, 0.f, 0.f } };
            GPURenderPassDescriptor render_pass_desc{};
            render_pass_desc.sample_count        = GPU_SAMPLE_COUNT_1;
            render_pass_desc.color_attachments   = &screenAttachment;
            render_pass_desc.render_target_count = 1;
            for (uint32_t m = 0; m < mipLevels; m++)
            {
                for (uint32_t i = 0; i < 6; i++)
                {
                    float viewPortW = static_cast<float>(dims * std::pow(0.5f, m));
                    float viewPortH = static_cast<float>(dims * std::pow(0.5f, m));
                    GPURenderPassEncoderID encoder = GPUCmdBeginRenderPass(cmd, &render_pass_desc);
                    {
                        GPURenderEncoderSetViewport(encoder, 0.f, 0.f, viewPortW, viewPortH, 0.f, 1.f);
                        GPURenderEncoderSetScissor(encoder, 0, 0, dims, dims);

                        GPURenderEncoderBindPipeline(encoder, tempPipeline);
                        GPURenderEncoderBindDescriptorSet(encoder, set);

                        struct PushData
                        {
                            glm::mat4 view;
                            glm::mat4 proj;
                        } push_constant;
                        push_constant.view = captureViews[i];
                        push_constant.proj = captureProjection;
                        GPURenderEncoderPushConstant(encoder, rs, &push_constant);
                        uint32_t strides = sizeof(Vertex);
                        GPURenderEncoderBindVertexBuffers(encoder, 1, &mVertexBuffer, &strides, nullptr);
                        GPURenderEncoderBindIndexBuffer(encoder, mIndexBuffer, 0, sizeof(uint32_t));
                        GPURenderEncoderDrawIndexed(encoder, mIndices.size(), 0, 0);
                    }
                    GPUCmdEndRenderPass(cmd, encoder);

                    GPUTextureBarrier tex_barrier1 = {
                        .texture   = offScreen.texture,
                        .src_state = GPU_RESOURCE_STATE_RENDER_TARGET,
                        .dst_state = GPU_RESOURCE_STATE_COPY_SOURCE
                    };
                    GPUResourceBarrierDescriptor barrier = {
                        .texture_barriers       = &tex_barrier1,
                        .texture_barriers_count = 1
                    };
                    GPUCmdResourceBarrier(cmd, &barrier);

                    GPUTextureSubresource src_sub_rang = {
                        .aspects          = GPU_TVA_COLOR,
                        .mip_level        = 0,
                        .base_array_layer = 0,
                        .layer_count      = 1
                    };
                    GPUTextureSubresource dst_sub_rang = {
                        .aspects          = GPU_TVA_COLOR,
                        .mip_level        = m,
                        .base_array_layer = i,
                        .layer_count      = 1
                    };
                    GPUTextureToTextureTransfer trans = {
                        .src             = offScreen.texture,
                        .dst             = mIrradianceMap->mTexture,
                        .src_subresource = src_sub_rang,
                        .dst_subresource = dst_sub_rang
                    };
                    GPUCmdTransferTextureToTexture(cmd, &trans);

                    GPUTextureBarrier tex_barrier2 = {
                        .texture   = offScreen.texture,
                        .src_state = GPU_RESOURCE_STATE_COPY_SOURCE,
                        .dst_state = GPU_RESOURCE_STATE_RENDER_TARGET
                    };
                    GPUResourceBarrierDescriptor barrier2 = {
                        .texture_barriers       = &tex_barrier2,
                        .texture_barriers_count = 1
                    };
                    GPUCmdResourceBarrier(cmd, &barrier2);
                }
            }
            GPUTextureBarrier final_barrier = {
                .texture   = mIrradianceMap->mTexture,
                .src_state = GPU_RESOURCE_STATE_COPY_DEST,
                .dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE
            };
            GPUResourceBarrierDescriptor rs_barrer = {
                .texture_barriers       = &final_barrier,
                .texture_barriers_count = 1
            };
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        // submit
        GPUQueueSubmitDescriptor submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &submit);
        GPUWaitQueueIdle(gfxQueue);
        // render

        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);
        GPUFreeDescriptorSet(set);
        GPUFreeTextureView(offScreen.texView);
        GPUFreeTexture(offScreen.texture);
        GPUFreeSampler(staticSampler);
        GPUFreeRootSignature(rs);
        GPUFreeRenderPipeline(tempPipeline);
    }

    // Prefilter environment cubemap
    // See https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
    void GenPrefilteredCubeMap(GPUDeviceID device, GPUQueueID gfxQueue, uint32_t numSamples)
    {
        mPrefilteredMap      = new HDRIBLCubeMapTextureData;
        EGPUFormat format   = GPU_FORMAT_R16G16B16A16_SFLOAT;
        uint32_t dims       = 512;
        uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(dims))) + 1;

        GPUTextureDescriptor desc = {
            .flags       = GPU_TCF_OWN_MEMORY_BIT,
            .width       = dims,
            .height      = dims,
            .depth       = 1,
            .array_size  = 6,
            .format      = format,
            .mip_levels  = mipLevels,
            .owner_queue = gfxQueue,
            .start_state = GPU_RESOURCE_STATE_COPY_DEST,
            .descriptors = GPU_RESOURCE_TYPE_TEXTURE_CUBE
        };
        mPrefilteredMap->mTexture = GPUCreateTexture(device, &desc);
        GPUTextureViewDescriptor tex_view_desc = {
            .pTexture        = mPrefilteredMap->mTexture,
            .format          = format,
            .usages          = EGPUTexutreViewUsage::GPU_TVU_SRV,
            .aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR,
            .baseMipLevel    = 0,
            .mipLevelCount   = mipLevels,
            .baseArrayLayer  = 0,
            .arrayLayerCount = 6
        };
        mPrefilteredMap->mTextureView = GPUCreateTextureView(device, &tex_view_desc);

        struct
        {
            GPUTextureID texture;
            GPUTextureViewID texView;
        } offScreen;
        desc = {
            .flags       = GPU_TCF_OWN_MEMORY_BIT,
            .width       = dims,
            .height      = dims,
            .depth       = 1,
            .array_size  = 1,
            .format      = format,
            .owner_queue = gfxQueue,
            .start_state = GPU_RESOURCE_STATE_RENDER_TARGET,
            .descriptors = GPU_RESOURCE_TYPE_TEXTURE | GPU_RESOURCE_TYPE_RENDER_TARGET
        };
        offScreen.texture = GPUCreateTexture(device, &desc);
        tex_view_desc = {
            .pTexture        = offScreen.texture,
            .format          = format,
            .usages          = EGPUTexutreViewUsage::GPU_TVU_RTV_DSV,
            .aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR,
            .baseMipLevel    = 0,
            .mipLevelCount   = 1,
            .baseArrayLayer  = 0,
            .arrayLayerCount = 1
        };
        offScreen.texView = GPUCreateTextureView(device, &tex_view_desc);

        // shader
        uint32_t* vShaderCode;
        uint32_t vSize = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/gen_prefiltered_cube.vert", &vShaderCode, &vSize);
        uint32_t* fShaderCode;
        uint32_t fSize = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/gen_prefiltered_cube.frag", &fShaderCode, &fSize);
        GPUShaderLibraryDescriptor vShaderDesc = {
            .pName    = u8"gen_irradiance_cube_vertex_shader",
            .code     = vShaderCode,
            .codeSize = vSize,
            .stage    = GPU_SHADER_STAGE_VERT
        };
        GPUShaderLibraryDescriptor fShaderDesc = {
            .pName    = u8"gen_prefiltered_cube_fragment_shader",
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

        GPUSamplerDescriptor sampler_desc = {
            .min_filter     = GPU_FILTER_TYPE_LINEAR,
            .mag_filter     = GPU_FILTER_TYPE_LINEAR,
            .mipmap_mode    = GPU_MIPMAP_MODE_LINEAR,
            .address_u      = GPU_ADDRESS_MODE_REPEAT,
            .address_v      = GPU_ADDRESS_MODE_REPEAT,
            .address_w      = GPU_ADDRESS_MODE_REPEAT,
            .compare_func   = GPU_CMP_NEVER
        };
        GPUSamplerID staticSampler  = GPUCreateSampler(device, &sampler_desc);
        const char8_t* sampler_name = u8"texSamp";

        // create root signature
        GPURootSignatureDescriptor rootRSDesc = {
            .shaders              = shaderEntries,
            .shader_count         = 2,
            .static_samplers      = &staticSampler,
            .static_sampler_names = &sampler_name,
            .static_sampler_count = 1
        };
        GPURootSignatureID rs = GPUCreateRootSignature(device, &rootRSDesc);

        // create descriptorset
        GPUDescriptorSetDescriptor set_desc = {
            .root_signature = rs,
            .set_index      = 0
        };
        GPUDescriptorSetID set = GPUCreateDescriptorSet(device, &set_desc);

        // vertex layout
        GPUVertexLayout vertexLayout{};
        vertexLayout.attributeCount = 3;
        vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
        // renderpipeline
        GPURasterizerStateDescriptor rasterizerState = {
            .cullMode             = GPU_CULL_MODE_BACK,
            .fillMode             = GPU_FILL_MODE_SOLID,
            .frontFace            = GPU_FRONT_FACE_CW,
            .depthBias            = 0,
            .slopeScaledDepthBias = 0.f,
            .enableMultiSample    = false,
            .enableScissor        = false,
            .enableDepthClamp     = false
        };
        GPURenderPipelineDescriptor pipelineDesc = {
            .pRootSignature    = rs,
            .pVertexShader     = &shaderEntries[0],
            .pFragmentShader   = &shaderEntries[1],
            .pVertexLayout     = &vertexLayout,
            .pRasterizerState  = &rasterizerState,
            .primitiveTopology = GPU_PRIM_TOPO_TRI_LIST,
            .pColorFormats     = const_cast<EGPUFormat*>(&format),
            .renderTargetCount = 1
        };
        GPURenderPipelineID tempPipeline = GPUCreateRenderPipeline(device, &pipelineDesc);
        GPUFreeShaderLibrary(pVShader);
        GPUFreeShaderLibrary(pFShader);

        GPUDescriptorData desc_data = {};
        {
            desc_data.name         = u8"tex";
            desc_data.binding      = 0;
            desc_data.binding_type = GPU_RESOURCE_TYPE_TEXTURE;
            desc_data.count        = 1;
            desc_data.textures     = &mTextureData->mTextureView;
        }
        GPUUpdateDescriptorSet(set, &desc_data, 1);

        // render
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.0f);
        glm::mat4 captureViews[]    = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
        };

        GPUCommandPoolID pool              = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmdDesc = { .isSecondary = false };
        GPUCommandBufferID cmd             = GPUCreateCommandBuffer(pool, &cmdDesc);
        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUColorAttachment screenAttachment{};
            screenAttachment.view         = offScreen.texView;
            screenAttachment.load_action  = GPU_LOAD_ACTION_CLEAR;
            screenAttachment.store_action = GPU_STORE_ACTION_STORE;
            screenAttachment.clear_color  = { { 0.f, 0.f, 0.f, 0.f } };
            GPURenderPassDescriptor render_pass_desc{};
            render_pass_desc.sample_count        = GPU_SAMPLE_COUNT_1;
            render_pass_desc.color_attachments   = &screenAttachment;
            render_pass_desc.render_target_count = 1;
            struct PushData
            {
                glm::mat4 view;
                glm::mat4 proj;
                float roughness;
                uint32_t numSamples;
            } push_constant;
            push_constant.numSamples = numSamples;
            for (uint32_t m = 0; m < mipLevels; m++)
            {
                push_constant.roughness = (float)m / (mipLevels - 1);
                for (uint32_t i = 0; i < 6; i++)
                {
                    float viewPortW = static_cast<float>(dims * std::pow(0.5f, m));
                    float viewPortH = static_cast<float>(dims * std::pow(0.5f, m));
                    GPURenderPassEncoderID encoder = GPUCmdBeginRenderPass(cmd, &render_pass_desc);
                    {
                        GPURenderEncoderSetViewport(encoder, 0.f, 0.f, viewPortW, viewPortH, 0.f, 1.f);
                        GPURenderEncoderSetScissor(encoder, 0, 0, dims, dims);

                        GPURenderEncoderBindPipeline(encoder, tempPipeline);
                        GPURenderEncoderBindDescriptorSet(encoder, set);

                        push_constant.view = captureViews[i];
                        push_constant.proj = captureProjection;
                        GPURenderEncoderPushConstant(encoder, rs, &push_constant);
                        uint32_t strides = sizeof(Vertex);
                        GPURenderEncoderBindVertexBuffers(encoder, 1, &mVertexBuffer, &strides, nullptr);
                        GPURenderEncoderBindIndexBuffer(encoder, mIndexBuffer, 0, sizeof(uint32_t));
                        GPURenderEncoderDrawIndexed(encoder, mIndices.size(), 0, 0);
                    }
                    GPUCmdEndRenderPass(cmd, encoder);

                    GPUTextureBarrier tex_barrier1 = {
                        .texture   = offScreen.texture,
                        .src_state = GPU_RESOURCE_STATE_RENDER_TARGET,
                        .dst_state = GPU_RESOURCE_STATE_COPY_SOURCE
                    };
                    GPUResourceBarrierDescriptor barrier = {
                        .texture_barriers       = &tex_barrier1,
                        .texture_barriers_count = 1
                    };
                    GPUCmdResourceBarrier(cmd, &barrier);

                    GPUTextureSubresource src_sub_rang = {
                        .aspects          = GPU_TVA_COLOR,
                        .mip_level        = 0,
                        .base_array_layer = 0,
                        .layer_count      = 1
                    };
                    GPUTextureSubresource dst_sub_rang = {
                        .aspects          = GPU_TVA_COLOR,
                        .mip_level        = m,
                        .base_array_layer = i,
                        .layer_count      = 1
                    };
                    GPUTextureToTextureTransfer trans = {
                        .src             = offScreen.texture,
                        .dst             = mPrefilteredMap->mTexture,
                        .src_subresource = src_sub_rang,
                        .dst_subresource = dst_sub_rang
                    };
                    GPUCmdTransferTextureToTexture(cmd, &trans);

                    GPUTextureBarrier tex_barrier2 = {
                        .texture   = offScreen.texture,
                        .src_state = GPU_RESOURCE_STATE_COPY_SOURCE,
                        .dst_state = GPU_RESOURCE_STATE_RENDER_TARGET
                    };
                    GPUResourceBarrierDescriptor barrier2 = {
                        .texture_barriers       = &tex_barrier2,
                        .texture_barriers_count = 1
                    };
                    GPUCmdResourceBarrier(cmd, &barrier2);
                }
            }
            GPUTextureBarrier final_barrier = {
                .texture   = mPrefilteredMap->mTexture,
                .src_state = GPU_RESOURCE_STATE_COPY_DEST,
                .dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE
            };
            GPUResourceBarrierDescriptor rs_barrer = {
                .texture_barriers       = &final_barrier,
                .texture_barriers_count = 1
            };
            GPUCmdResourceBarrier(cmd, &rs_barrer);
        }
        GPUCmdEnd(cmd);
        // submit
        GPUQueueSubmitDescriptor submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &submit);
        GPUWaitQueueIdle(gfxQueue);
        // render

        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);
        GPUFreeDescriptorSet(set);
        GPUFreeTextureView(offScreen.texView);
        GPUFreeTexture(offScreen.texture);
        GPUFreeSampler(staticSampler);
        GPUFreeRootSignature(rs);
        GPUFreeRenderPipeline(tempPipeline);
    }

    void GenBRDFLut(GPUDeviceID device, GPUQueueID gfxQueue, uint32_t numSamples)
    {
        mBRDFLut          = new HDRIBLCubeMapTextureData;
        EGPUFormat format = GPU_FORMAT_R16G16_SFLOAT;
        uint32_t dims     = 512;

        GPUTextureDescriptor desc = {
            .flags       = GPU_TCF_OWN_MEMORY_BIT,
            .width       = dims,
            .height      = dims,
            .depth       = 1,
            .array_size  = 1,
            .format      = format,
            .mip_levels  = 1,
            .owner_queue = gfxQueue,
            .start_state = GPU_RESOURCE_STATE_RENDER_TARGET,
            .descriptors = GPU_RESOURCE_TYPE_TEXTURE | GPU_RESOURCE_TYPE_RENDER_TARGET
        };
        mBRDFLut->mTexture = GPUCreateTexture(device, &desc);
        GPUTextureViewDescriptor tex_view_desc = {
            .pTexture        = mBRDFLut->mTexture,
            .format          = format,
            .usages          = EGPUTexutreViewUsage::GPU_TVU_SRV | GPU_TVU_RTV_DSV,
            .aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR,
            .baseMipLevel    = 0,
            .mipLevelCount   = 1,
            .baseArrayLayer  = 0,
            .arrayLayerCount = 1
        };
        mBRDFLut->mTextureView = GPUCreateTextureView(device, &tex_view_desc);

        // shader
        uint32_t* vShaderCode;
        uint32_t vSize = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/gen_brdf_lut.vert", &vShaderCode, &vSize);
        uint32_t* fShaderCode;
        uint32_t fSize = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/gen_brdf_lut.frag", &fShaderCode, &fSize);
        GPUShaderLibraryDescriptor vShaderDesc = {
            .pName    = u8"gen_brdf_lut_vertex_shader",
            .code     = vShaderCode,
            .codeSize = vSize,
            .stage    = GPU_SHADER_STAGE_VERT
        };
        GPUShaderLibraryDescriptor fShaderDesc = {
            .pName    = u8"gen_brdf_lut_fragment_shader",
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
            .shader_count         = 2
        };
        GPURootSignatureID rs = GPUCreateRootSignature(device, &rootRSDesc);

        // vertex layout
        GPUVertexLayout vertexLayout{};
        // renderpipeline
        GPURasterizerStateDescriptor rasterizerState = {
            .cullMode             = GPU_CULL_MODE_NONE,
            .fillMode             = GPU_FILL_MODE_SOLID,
            .frontFace            = GPU_FRONT_FACE_CW,
            .depthBias            = 0,
            .slopeScaledDepthBias = 0.f,
            .enableMultiSample    = false,
            .enableScissor        = false,
            .enableDepthClamp     = false
        };
        GPURenderPipelineDescriptor pipelineDesc = {
            .pRootSignature    = rs,
            .pVertexShader     = &shaderEntries[0],
            .pFragmentShader   = &shaderEntries[1],
            .pVertexLayout     = &vertexLayout,
            .pRasterizerState  = &rasterizerState,
            .primitiveTopology = GPU_PRIM_TOPO_TRI_LIST,
            .pColorFormats     = const_cast<EGPUFormat*>(&format),
            .renderTargetCount = 1
        };
        GPURenderPipelineID tempPipeline = GPUCreateRenderPipeline(device, &pipelineDesc);
        GPUFreeShaderLibrary(pVShader);
        GPUFreeShaderLibrary(pFShader);

        // render
        GPUCommandPoolID pool              = GPUCreateCommandPool(gfxQueue);
        GPUCommandBufferDescriptor cmdDesc = { .isSecondary = false };
        GPUCommandBufferID cmd             = GPUCreateCommandBuffer(pool, &cmdDesc);
        GPUResetCommandPool(pool);
        GPUCmdBegin(cmd);
        {
            GPUColorAttachment screenAttachment{};
            screenAttachment.view         = mBRDFLut->mTextureView;
            screenAttachment.load_action  = GPU_LOAD_ACTION_CLEAR;
            screenAttachment.store_action = GPU_STORE_ACTION_STORE;
            screenAttachment.clear_color  = { { 0.f, 0.f, 0.f, 0.f } };
            GPURenderPassDescriptor render_pass_desc{};
            render_pass_desc.sample_count        = GPU_SAMPLE_COUNT_1;
            render_pass_desc.color_attachments   = &screenAttachment;
            render_pass_desc.render_target_count = 1;
            struct PushData
            {
                uint32_t numSamples;
            } push_constant;
            push_constant.numSamples = numSamples;
            GPURenderPassEncoderID encoder = GPUCmdBeginRenderPass(cmd, &render_pass_desc);
            {
                GPURenderEncoderSetViewport(encoder, 0.f, 0.f, dims, dims, 0.f, 1.f);
                GPURenderEncoderSetScissor(encoder, 0, 0, dims, dims);
                GPURenderEncoderBindPipeline(encoder, tempPipeline);
                GPURenderEncoderPushConstant(encoder, rs, &push_constant);
                GPURenderEncoderDraw(encoder, 3, 0);
            }
            GPUCmdEndRenderPass(cmd, encoder);

            GPUTextureBarrier tex_barrier1 = {
                .texture   = mBRDFLut->mTexture,
                .src_state = GPU_RESOURCE_STATE_RENDER_TARGET,
                .dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE
            };
            GPUResourceBarrierDescriptor barrier = {
                .texture_barriers       = &tex_barrier1,
                .texture_barriers_count = 1
            };
            GPUCmdResourceBarrier(cmd, &barrier);
        }
        GPUCmdEnd(cmd);
        // submit
        GPUQueueSubmitDescriptor submit = { .cmds = &cmd, .cmds_count = 1 };
        GPUSubmitQueue(gfxQueue, &submit);
        GPUWaitQueueIdle(gfxQueue);
        // render

        GPUFreeCommandBuffer(cmd);
        GPUFreeCommandPool(pool);
        GPUFreeRootSignature(rs);
        GPUFreeRenderPipeline(tempPipeline);
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
    HDRIBLCubeMapTextureData* mIrradianceMap{ nullptr };
    HDRIBLCubeMapTextureData* mPrefilteredMap{ nullptr };
    HDRIBLCubeMapTextureData* mBRDFLut{ nullptr };
    GPUBufferID mVertexBuffer{ nullptr };
    GPUBufferID mIndexBuffer{ nullptr };
    GPURenderPipelineID mPipeline{ nullptr };
    GPURootSignatureID mRS{ nullptr };
    GPUDescriptorSetID mSet{ nullptr };
    GPUBufferID mUniformBuffer{ nullptr };
};
