#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Gpu/GpuApi.h"
#include "utils.hpp"
#include "model.hpp"
#include  "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <array>
#include "Camera.hpp"

#define SHADOW_MAP_CASCADE_COUNT 4u

class CascadeShadowPass
{
public:
    GPUBufferID mUBO;
    GPUDescriptorSetID mSet;
    GPURenderPipelineID mPipeline;
    GPURootSignatureID mRS;
    GPUTextureID mDepthTexture;
    GPUTextureViewID mDepthTextureView;
    GPUTextureID mTexture;
    GPUTextureViewID mTextureView;
    GPUDeviceID mRefDevice;
    GPUQueueID mRefGfxQueue;
    glm::mat4 mLightSpaceMatrix;

    GPURenderPipelineID mDebugPipeline;
    GPUDescriptorSetID mDebugSet;

    GPUSamplerID mSampler;
public:
    CascadeShadowPass(GPUDeviceID device, GPUQueueID gfxQueue)
    : mRefDevice(device), mRefGfxQueue(gfxQueue)
    {

    };
    ~CascadeShadowPass()
    {
        if (mUBO) GPUFreeBuffer(mUBO);
        if (mSet) GPUFreeDescriptorSet(mSet);
        if (mDepthTextureView) GPUFreeTextureView(mDepthTextureView);
        if (mDepthTexture) GPUFreeTexture(mDepthTexture);
        if (mTextureView) GPUFreeTextureView(mTextureView);
        if (mTexture) GPUFreeTexture(mTexture);
        if (mRS) GPUFreeRootSignature(mRS);
        if (mPipeline) GPUFreeRenderPipeline(mPipeline);

        if (mSampler) GPUFreeSampler(mSampler);

        if (mDebugSet) GPUFreeDescriptorSet(mDebugSet);
        if (mDebugPipeline) GPUFreeRenderPipeline(mDebugPipeline);
    }

    void InitRenderObjects()
    {
        const char8_t* samplerName = u8"texSamp";

        uint32_t* shaderCode;
        uint32_t size = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/shadow_map.vert", &shaderCode, &size);
        GPUShaderLibraryDescriptor shaderDesc = {
            .pName    = u8"",
            .code     = shaderCode,
            .codeSize = size,
            .stage    = GPU_SHADER_STAGE_VERT
        };
        GPUShaderLibraryID vsShader = GPUCreateShaderLibrary(mRefDevice, &shaderDesc);
        free(shaderCode);
        
        shaderCode = nullptr;
        size       = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/shadow_map.frag", &shaderCode, &size);
        shaderDesc = {
            .pName    = u8"",
            .code     = shaderCode,
            .codeSize = size,
            .stage    = GPU_SHADER_STAGE_FRAG
        };
        GPUShaderLibraryID psShader = GPUCreateShaderLibrary(mRefDevice, &shaderDesc);
        free(shaderCode);
        
        shaderCode = nullptr;
        size       = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/debug_shadow_map.vert", &shaderCode, &size);
        shaderDesc = {
            .pName    = u8"",
            .code     = shaderCode,
            .codeSize = size,
            .stage    = GPU_SHADER_STAGE_VERT
        };
        GPUShaderLibraryID vsDShader = GPUCreateShaderLibrary(mRefDevice, &shaderDesc);
        free(shaderCode);
        
        shaderCode = nullptr;
        size       = 0;
        ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/debug_shadow_map.frag", &shaderCode, &size);
        shaderDesc = {
            .pName    = u8"",
            .code     = shaderCode,
            .codeSize = size,
            .stage    = GPU_SHADER_STAGE_FRAG
        };
        GPUShaderLibraryID psDShader = GPUCreateShaderLibrary(mRefDevice, &shaderDesc);
        free(shaderCode);
        shaderCode = nullptr;
        
        CGPUShaderEntryDescriptor entry_desc[4] = {};
        entry_desc[0].pLibrary                  = vsShader;
        entry_desc[0].entry                     = u8"main";
        entry_desc[0].stage                     = GPU_SHADER_STAGE_VERT;
        entry_desc[1].pLibrary                  = psShader;
        entry_desc[1].entry                     = u8"main";
        entry_desc[1].stage                     = GPU_SHADER_STAGE_FRAG;
        entry_desc[2].pLibrary                  = vsDShader;
        entry_desc[2].entry                     = u8"main";
        entry_desc[2].stage                     = GPU_SHADER_STAGE_VERT;
        entry_desc[3].pLibrary                  = psDShader;
        entry_desc[3].entry                     = u8"main";
        entry_desc[3].stage                     = GPU_SHADER_STAGE_FRAG;

        GPURootSignatureDescriptor rs_desc = {
            .shaders      = entry_desc,
            .shader_count = 4,
        };
        mRS = GPUCreateRootSignature(mRefDevice, &rs_desc);

        // vertex layout
        GPUVertexLayout vertexLayout{};
        vertexLayout.attributeCount = 5;
        vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[3]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 8, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        vertexLayout.attributes[4]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 11, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
        // renderpipeline
        GPURasterizerStateDescriptor rasterizerState = {
            .cullMode             = GPU_CULL_MODE_NONE,
            .fillMode             = GPU_FILL_MODE_SOLID,
            .frontFace            = GPU_FRONT_FACE_CCW,
            .depthBias            = 0,
            .slopeScaledDepthBias = 1.75f,
            .enableMultiSample    = false,
            .enableScissor        = false,
            .enableDepthClamp     = false
        };
        GPUDepthStateDesc depthDesc = {
            .depthTest  = true,
            .depthWrite = true,
            .depthFunc  = GPU_CMP_LEQUAL
        };
        EGPUFormat format               = GPU_FORMAT_R8G8B8A8_UNORM;
        GPURenderPipelineDescriptor pipelineDesc = {
            .pRootSignature     = mRS,
            .pVertexShader      = &entry_desc[0],
            .pFragmentShader    = &entry_desc[1],
            .pVertexLayout      = &vertexLayout,
            .pDepthState        = &depthDesc,
            .pRasterizerState   = &rasterizerState,
            .primitiveTopology  = GPU_PRIM_TOPO_TRI_LIST,
            .pColorFormats      = const_cast<EGPUFormat*>(&format),
            .renderTargetCount  = 1,
            .depthStencilFormat = GPU_FORMAT_D32_SFLOAT
        };
        mPipeline = GPUCreateRenderPipeline(mRefDevice, &pipelineDesc);
        GPUFreeShaderLibrary(vsShader);
        GPUFreeShaderLibrary(psShader);

        GPUVertexLayout emptyLayout {};
        rasterizerState = {
            .cullMode             = GPU_CULL_MODE_NONE,
            .fillMode             = GPU_FILL_MODE_SOLID,
            .frontFace            = GPU_FRONT_FACE_CCW,
            .depthBias            = 0,
            .slopeScaledDepthBias = 0.f,
            .enableMultiSample    = false,
            .enableScissor        = false,
            .enableDepthClamp     = false
        };

        format = GPU_FORMAT_B8G8R8A8_UNORM;
        pipelineDesc.pRasterizerState = &rasterizerState;
        pipelineDesc.pVertexShader   = &entry_desc[2];
        pipelineDesc.pFragmentShader = &entry_desc[3];
        pipelineDesc.pVertexLayout = &emptyLayout;
        pipelineDesc.pColorFormats      = const_cast<EGPUFormat*>(&format);
        mDebugPipeline = GPUCreateRenderPipeline(mRefDevice, &pipelineDesc);
        GPUFreeShaderLibrary(vsDShader);
        GPUFreeShaderLibrary(psDShader);

        static constexpr uint32_t DEPTH_W = 4096, DEPTH_H = 4096;
        format               = GPU_FORMAT_R8G8B8A8_UNORM;
        GPUTextureDescriptor desc = {
            .flags       = GPU_TCF_OWN_MEMORY_BIT,
            .width       = DEPTH_W,
            .height      = DEPTH_H,
            .depth       = 1,
            .array_size  = 1,
            .format      = format,
            .owner_queue = mRefGfxQueue,
            .start_state = GPU_RESOURCE_STATE_RENDER_TARGET,
            .descriptors = GPU_RESOURCE_TYPE_TEXTURE | GPU_RESOURCE_TYPE_RENDER_TARGET
        };
        mTexture = GPUCreateTexture(mRefDevice, &desc);
        GPUTextureViewDescriptor tex_view_desc = {
            .pTexture        = mTexture,
            .format          = format,
            .usages          = EGPUTexutreViewUsage::GPU_TVU_RTV_DSV | GPU_TVU_SRV,
            .aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR,
            .baseMipLevel    = 0,
            .mipLevelCount   = 1,
            .baseArrayLayer  = 0,
            .arrayLayerCount = 1
        };
        mTextureView = GPUCreateTextureView(mRefDevice, &tex_view_desc);

        GPUTextureDescriptor depth_tex_desc{};
        {
            depth_tex_desc.flags       = GPU_TCF_OWN_MEMORY_BIT;
            depth_tex_desc.width       = DEPTH_W;
            depth_tex_desc.height      = DEPTH_H;
            depth_tex_desc.depth       = 1;
            depth_tex_desc.array_size  = 1;
            depth_tex_desc.format      = GPU_FORMAT_D32_SFLOAT;
            depth_tex_desc.owner_queue = mRefGfxQueue;
            depth_tex_desc.start_state = GPU_RESOURCE_STATE_DEPTH_WRITE;
            depth_tex_desc.descriptors = GPU_RESOURCE_TYPE_TEXTURE | GPU_RESOURCE_TYPE_DEPTH_STENCIL;
        }
        mDepthTexture = GPUCreateTexture(mRefDevice, &depth_tex_desc);
        GPUTextureViewDescriptor depth_tex_view_desc{};
        {
            depth_tex_view_desc.pTexture        = mDepthTexture;
            depth_tex_view_desc.format          = GPU_FORMAT_D32_SFLOAT;
            depth_tex_view_desc.usages          = EGPUTexutreViewUsage::GPU_TVU_RTV_DSV | GPU_TVU_SRV;
            depth_tex_view_desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_DEPTH;
            depth_tex_view_desc.baseMipLevel    = 0;
            depth_tex_view_desc.mipLevelCount   = 1;
            depth_tex_view_desc.baseArrayLayer  = 0;
            depth_tex_view_desc.arrayLayerCount = 1;
        }
        mDepthTextureView = GPUCreateTextureView(mRefDevice, &depth_tex_view_desc);

        GPUDescriptorSetDescriptor setDesc = {
            .root_signature = mRS,
            .set_index      = 0
        };
        mSet = GPUCreateDescriptorSet(mRefDevice, &setDesc);

        setDesc = {
            .root_signature = mRS,
            .set_index      = 1
        };
        mDebugSet = GPUCreateDescriptorSet(mRefDevice, &setDesc);

        GPUBufferDescriptor uboDesc = {
            .size             = sizeof(glm::mat4),
            .descriptors      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER,
            .memory_usage     = GPU_MEM_USAGE_CPU_TO_GPU,
            .flags            = GPU_BCF_PERSISTENT_MAP_BIT,
            .prefer_on_device = true
        };
        mUBO = GPUCreateBuffer(mRefDevice, &uboDesc);

        GPUDescriptorData dataDesc[2] = {};
        dataDesc[0].binding           = 0;
        dataDesc[0].binding_type      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
        dataDesc[0].buffers           = &mUBO;
        dataDesc[0].count             = 1;
        GPUUpdateDescriptorSet(mSet, dataDesc, 1);

        GPUSamplerDescriptor sampler_desc = {
            .min_filter   = GPU_FILTER_TYPE_LINEAR,
            .mag_filter   = GPU_FILTER_TYPE_LINEAR,
            .mipmap_mode  = GPU_MIPMAP_MODE_LINEAR,
            .address_u    = GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
            .address_v    = GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
            .address_w    = GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
            .compare_func = GPU_CMP_NEVER,
        };
        mSampler = GPUCreateSampler(mRefDevice, &sampler_desc);

        dataDesc[0].binding      = 0;
        dataDesc[0].binding_type = GPU_RESOURCE_TYPE_TEXTURE;
        dataDesc[0].textures     = &mDepthTextureView;
        dataDesc[0].count        = 1;
        dataDesc[1].binding      = 1;
        dataDesc[1].binding_type = GPU_RESOURCE_TYPE_SAMPLER;
        dataDesc[1].samplers     = &mSampler;
        dataDesc[1].count        = 1;
        GPUUpdateDescriptorSet(mDebugSet, dataDesc, 2);
    }

    struct ShadowDrawSceneInfo
    {
        GPUBufferID vertexBuffer;
        GPUBufferID indexBuffer;
        Mesh* mesh;
        std::unordered_map<uint32_t, PBRMaterial*>* materials;
        uint32_t strides;
        glm::mat4 modelMatrix;
    };

    void Draw(const ShadowDrawSceneInfo& sceneInfo, GPUCommandBufferID cmd, const glm::mat4& view, const glm::mat4& proj, const glm::vec4& viewPos, const glm::vec3 lightPos, const BoundingBox& entityBoundingBox)
    {
        glm::vec3 lightDir = glm::normalize(lightPos);
        mLightSpaceMatrix  = CalculateDirectionalLightCamera(view, proj, entityBoundingBox, sceneInfo.modelMatrix, lightDir);
        memcpy(mUBO->cpu_mapped_address, &mLightSpaceMatrix, sizeof(glm::mat4));

        // reorganize mesh
        std::unordered_map<PBRMaterial*, std::vector<SubMesh*>> drawNodesInfo;
        for (size_t i = 0; i < sceneInfo.mesh->subMeshes.size(); i++)
        {
            const auto itr = sceneInfo.materials->find(sceneInfo.mesh->subMeshes[i].materialIndex);
            if (itr != sceneInfo.materials->end())
            {
                auto cur_pair = drawNodesInfo.find(itr->second);
                if (cur_pair != drawNodesInfo.end())
                {
                    cur_pair->second.push_back(&sceneInfo.mesh->subMeshes[i]);
                }
                else
                {
                    auto& nodes = drawNodesInfo[itr->second];
                    nodes.push_back(&sceneInfo.mesh->subMeshes[i]);
                }
            }
        }

        GPUTextureBarrier tex_barriers[1] = {};
        /* tex_barriers[0].texture   = mTexture;
        tex_barriers[0].src_state = GPU_RESOURCE_STATE_UNDEFINED;
        tex_barriers[0].dst_state = GPU_RESOURCE_STATE_RENDER_TARGET; */
        tex_barriers[0].texture   = mDepthTexture;
        tex_barriers[0].src_state = GPU_RESOURCE_STATE_UNDEFINED;
        tex_barriers[0].dst_state = GPU_RESOURCE_STATE_DEPTH_WRITE;
        GPUResourceBarrierDescriptor draw_barrier{};
        draw_barrier.texture_barriers       = tex_barriers;
        draw_barrier.texture_barriers_count = sizeof(tex_barriers) / sizeof(GPUTextureBarrier);
        GPUCmdResourceBarrier(cmd, &draw_barrier);

        GPUColorAttachment screenAttachment{};
        screenAttachment.view         = mTextureView;
        screenAttachment.load_action  = GPU_LOAD_ACTION_CLEAR;
        screenAttachment.store_action = GPU_STORE_ACTION_STORE;
        screenAttachment.clear_color  = { { 0.f, 0.f, 0.f, 0.f } };
        GPUDepthStencilAttachment ds_attachment{};
        ds_attachment.view               = mDepthTextureView;
        ds_attachment.depth_load_action  = GPU_LOAD_ACTION_CLEAR;
        ds_attachment.depth_store_action = GPU_STORE_ACTION_STORE;
        ds_attachment.clear_depth        = 1.0f;
        ds_attachment.write_depth        = true;
        GPURenderPassDescriptor render_pass_desc{};
        render_pass_desc.sample_count        = GPU_SAMPLE_COUNT_1;
        render_pass_desc.color_attachments   = &screenAttachment;
        render_pass_desc.render_target_count = 1;
        render_pass_desc.depth_stencil       = &ds_attachment;
        GPURenderPassEncoderID encoder       = GPUCmdBeginRenderPass(cmd, &render_pass_desc);
        {
            GPURenderEncoderSetViewport(encoder, 0.f, 0.f,
                                        (float)mDepthTexture->width,
                                        (float)mDepthTexture->height,
                                        0.f, 1.f);
            GPURenderEncoderSetScissor(encoder, 0, 0, mDepthTexture->width,
                                       mDepthTexture->height);
            // draw call
            GPURenderEncoderBindPipeline(encoder, mPipeline);
            GPURenderEncoderBindDescriptorSet(encoder, mSet);
            GPURenderEncoderBindVertexBuffers(encoder, 1, &sceneInfo.vertexBuffer, &sceneInfo.strides, nullptr);
            GPURenderEncoderBindIndexBuffer(encoder, sceneInfo.indexBuffer, 0, sizeof(uint32_t));
            //glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f, 0.02f, 0.02f));
            //glm::mat4 model = glm::mat4(1.0f);
            //model = proj * view * model;
            for (auto& nodePair : drawNodesInfo)
            {
                for (size_t i = 0; i < nodePair.second.size() && i < 1; i++)
                {
                    // per mesh
                    auto& mesh          = nodePair.second[i];
                    uint32_t indexCount = mesh->indexCount;
                    GPURenderEncoderPushConstant(encoder, mRS, const_cast<glm::mat4*>(&sceneInfo.modelMatrix));
                    GPURenderEncoderDrawIndexedInstanced(encoder, indexCount, 1, mesh->indexOffset, mesh->vertexOffset, 0);
                }
            }
        }
        GPUCmdEndRenderPass(cmd, encoder);

        GPUTextureBarrier tex_barrier1{};
        tex_barrier1.texture   = mDepthTexture;
        tex_barrier1.src_state = GPU_RESOURCE_STATE_DEPTH_WRITE;
        tex_barrier1.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
        /* tex_barrier1.texture   = mTexture;
        tex_barrier1.src_state = GPU_RESOURCE_STATE_RENDER_TARGET;
        tex_barrier1.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE; */
        GPUResourceBarrierDescriptor barrier{};
        barrier.texture_barriers_count = 1;
        barrier.texture_barriers       = &tex_barrier1;
        GPUCmdResourceBarrier(cmd, &barrier);
    }

    void DebugShadow(GPURenderPassEncoderID encoder)
    {
        GPURenderEncoderBindPipeline(encoder, mDebugPipeline);
        GPURenderEncoderBindDescriptorSet(encoder, mDebugSet);
        GPURenderEncoderDraw(encoder, 3, 0);
    }

    struct Cascade
    {
        float splitDepth;
        glm::mat4 viewProjMatrix;
    };

    std::array<Cascade, SHADOW_MAP_CASCADE_COUNT> cascades;
    glm::mat4 CalculateDirectionalLightCamera(const Camera& cam, const glm::mat4& view, const glm::mat4& proj, const BoundingBox& entityBoundingBox, const glm::mat4& entityModel, const glm::vec3& lightDir)
    {
        // Calculate split depths based on view camera frustum
        // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
        float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];
        float lambda = 0.95f;
        float n = cam.getNearClip();
        float f = cam.getFarClip();

        for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
        {
            float p       = (i + 1) / (float)SHADOW_MAP_CASCADE_COUNT;
            float log     = n * glm::pow(f / n, p);
            float uniform = n + (f - n) * p;
            float d       = lambda * log + (1.f - lambda) * uniform; // l * log + un - l * un : lambda * (log - uniform) + uniform;

            cascadeSplits[i] = d;
        }

        BoundingBox sceneBoundingBox;
        {
            //just one entity for now;
            BoundingBox worldBoundingBox = BoundingBox::BoundingBoxTransform(entityBoundingBox, entityModel);
            sceneBoundingBox.Merge(worldBoundingBox);
        }

        glm::mat4 vp    = cam.matrices.perspective * cam.matrices.view;
        glm::mat4 invVP = glm::inverse(vp);
        // Calculate orthographic projection matrix for each cascade
        size_t constexpr CORNER_COUNT = 8;
        float lastSplitDist = 0.0;
        for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
        {
            float splitDist = cascadeSplits[i];
            BoundingBox frustumBoundingBox;
            glm::vec3 frustumPointsNDCSpace[8] = {
                glm::vec3(-1.0f, -1.0f, 1.0f),
                glm::vec3(1.0f, -1.0f, 1.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                glm::vec3(-1.0f, 1.0f, 1.0f),
                glm::vec3(-1.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, 1.0f, 0.0f),
                glm::vec3(-1.0f, 1.0f, 0.0f),
            };
            for (size_t i = 0; i < CORNER_COUNT; ++i)
            {
                glm::vec4 frustumPointWith_w = invVP * glm::vec4(frustumPointsNDCSpace[i], 1.0);
                frustumPointsNDCSpace[i]     = frustumPointWith_w / frustumPointWith_w.w;

                frustumBoundingBox.Update(frustumPointsNDCSpace[i]);
            }
            for (uint32_t i = 0; i < 4; i++)
            {
                glm::vec3 dist               = frustumPointsNDCSpace[i + 4] - frustumPointsNDCSpace[i];
                frustumPointsNDCSpace[i + 4] = frustumPointsNDCSpace[i] + (dist * splitDist);
                frustumPointsNDCSpace[i]     = frustumPointsNDCSpace[i] + (dist * lastSplitDist);
            }

            // Get frustum center
            glm::vec3 frustumCenter = glm::vec3(0.0f);
            for (uint32_t i = 0; i < CORNER_COUNT; i++)
            {
                frustumCenter += frustumPointsNDCSpace[i];
            }
            frustumCenter /= (float)CORNER_COUNT;
        }

        glm::mat4 lightView;
        glm::mat4 lightProj;
        {
            glm::vec3 boxCenter = glm::vec3(
                (frustumBoundingBox.max.x + frustumBoundingBox.min.x) * 0.5f,
                (frustumBoundingBox.max.y + frustumBoundingBox.min.y) * 0.5f,
                (frustumBoundingBox.max.z + frustumBoundingBox.min.z) * 0.5f
            );
            glm::vec3 boxExtents = glm::vec3(
                (frustumBoundingBox.max.x - frustumBoundingBox.min.x) * 0.5f,
                (frustumBoundingBox.max.y - frustumBoundingBox.min.y) * 0.5f,
                (frustumBoundingBox.max.z - frustumBoundingBox.min.z) * 0.5f
            );

            glm::vec3 eye = boxCenter + lightDir * std::hypot(boxExtents.x, boxExtents.y, boxExtents.z);
            lightView     = glm::lookAt(eye, boxCenter, glm::vec3(0.0f, 1.0f, 0.0f));

            BoundingBox frustumBoundingBoxLightView = BoundingBox::BoundingBoxTransform(frustumBoundingBox, lightView);
            BoundingBox sceneBoundingBoxLightView   = BoundingBox::BoundingBoxTransform(sceneBoundingBox, lightView);

            lightProj = glm::ortho(
            std::max(frustumBoundingBoxLightView.min.x, sceneBoundingBoxLightView.min.x),
            std::min(frustumBoundingBoxLightView.max.x, sceneBoundingBoxLightView.max.x),
            std::max(frustumBoundingBoxLightView.min.y, sceneBoundingBoxLightView.min.y),
            std::min(frustumBoundingBoxLightView.max.y, sceneBoundingBoxLightView.max.y),
            -sceneBoundingBoxLightView.max.z, // the objects which are nearer than the frustum bounding box may caster shadow as well
            -std::max(frustumBoundingBoxLightView.min.z, sceneBoundingBoxLightView.min.z));
        }

        glm::mat4 lightVP = lightProj * lightView;
        return lightVP;
    }
    
    void Dummy()
    {

    }
    Matrix Light::CalculateCropMatrix(ObjectList casters, ObjectList receivers, Frustum frustum)
    {
        // Bounding boxes
        BoundingBox receiverBB, casterBB, splitBB;
        Matrix lightViewProjMatrix = viewMatrix * projMatrix;
        // Merge all bounding boxes of casters into a bigger "casterBB".
        for (int i = 0; i < casters.size(); i++)
        {
            BoundingBox bb = CreateAABB(casters[i]->AABB, lightViewProjMatrix);
            casterBB.Union(bb);
        }
        // Merge all bounding boxes of receivers into a bigger "receiverBB".
        for (int i = 0; i < receivers.size(); i++)
        {
            bb = CreateAABB(receivers[i]->AABB, lightViewProjMatrix);
            receiverBB.Union(bb);
        }
        // Find the bounding box of the current split
        // in the light's clip space.
        splitBB = CreateAABB(splitFrustum.AABB, lightViewProjMatrix);
        // Scene-dependent bounding volume
        BoundingBox cropBB;
        cropBB.min.x = Max(Max(casterBB.min.x, receiverBB.min.x), splitBB.min.x);
        cropBB.max.x = Min(Min(casterBB.max.x, receiverBB.max.x), splitBB.max.x);
        cropBB.min.y = Max(Max(casterBB.min.y, receiverBB.min.y), splitBB.min.y);
        cropBB.max.y = Min(Min(casterBB.max.y, receiverBB.max.y), splitBB.max.y);
        cropBB.min.z = Min(casterBB.min.z, splitBB.min.z);
        cropBB.max.z = Min(receiverBB.max.z, splitBB.max.z);
        // Create the crop matrix.
        float scaleX, scaleY, scaleZ;
        float offsetX, offsetY, offsetZ;
        scaleX  = 2.0f / (cropBB.max.x - cropBB.min.x);
        scaleY  = 2.0f / (cropBB.max.y - cropBB.min.y);
        offsetX = -0.5f * (cropBB.max.x + cropBB.min.x) * scaleX;
        offsetY = -0.5f * (cropBB.max.y + cropBB.min.y) * scaleY;
        scaleZ  = 1.0f / (cropBB.max.z - cropBB.min.z);
        offsetZ = -cropBB.min.z * scaleZ;
        return Matrix(scaleX, 0.0f, 0.0f, 0.0f, 0.0f, scaleY, 0.0f, 0.0f, 0.0f, 0.0f, scaleZ, 0.0f, offsetX, offsetY, offsetZ, 1.0f);
    }

    Matrix Light::CalculateCropMatrix(Frustum splitFrustum)
    {
        Matrix lightViewProjMatrix = viewMatrix * projMatrix;
        // Find boundaries in light's clip space
        BoundingBox cropBB = CreateAABB(splitFrustum.AABB, lightViewProjMatrix);
        // Use default near-plane value
        cropBB.min.z = 0.0f;
        // Create the crop matrix
        float scaleX, scaleY, scaleZ;
        float offsetX, offsetY, offsetZ;
        scaleX  = 2.0f / (cropBB.max.x - cropBB.min.x);
        scaleY  = 2.0f / (cropBB.max.y - cropBB.min.y);
        offsetX = -0.5f * (cropBB.max.x + cropBB.min.x) * scaleX;
        offsetY = -0.5f * (cropBB.max.y + cropBB.min.y) * scaleY;
        scaleZ  = 1.0f / (cropBB.max.z - cropBB.min.z);
        offsetZ = -cropBB.min.z * scaleZ;
        return Matrix(scaleX, 0.0f, 0.0f, 0.0f, 0.0f, scaleY, 0.0f, 0.0f, 0.0f, 0.0f, scaleZ, 0.0f, offsetX, offsetY, offsetZ, 1.0f);
    }
};