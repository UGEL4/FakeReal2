#include "cascade_shadow_pass.hpp"
#include "model_entity.hpp"

void CascadeShadowPass::Draw(GPUCommandBufferID cmd, const EntityModel* modelEntity, const Camera* cam, const FakeReal::math::Vector3& lightPos, uint32_t frameIndex,  const Culler& culler)
{
    // reorganize mesh
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

    auto roundUp = [](uint32_t value, uint32_t aligment) {
        uint32_t tmp = value + (aligment - 1);
        return tmp - tmp % aligment;
    };
    CalculateDirectionalLightCamera2(*cam, culler, lightPos, modelEntity);

    uint32_t perframe_dynamic_offset                                                    = roundUp(global::g_global_reader_resource.storage._global_upload_ringbuffers_end[frameIndex], global::g_global_reader_resource.storage.minAlignment);
    global::g_global_reader_resource.storage._global_upload_ringbuffers_end[frameIndex] = perframe_dynamic_offset + sizeof(global::MeshDirectionalLightShadowPerFrameStorageBufferObject);
    assert(global::g_global_reader_resource.storage._global_upload_ringbuffers_end[frameIndex] <=
           (global::g_global_reader_resource.storage._global_upload_ringbuffers_begin[frameIndex] + global::g_global_reader_resource.storage._global_upload_ringbuffers_size[frameIndex]));
    global::MeshDirectionalLightShadowPerFrameStorageBufferObject& perframeStorageBufferObject =
    (*reinterpret_cast<global::MeshDirectionalLightShadowPerFrameStorageBufferObject*>(reinterpret_cast<uintptr_t>(global::g_global_reader_resource.storage.buffer->cpu_mapped_address) + perframe_dynamic_offset));

    glm::mat4 LightSpaceMatrix[sCascadeCount];
    for (uint32_t i = 0; i < sCascadeCount; i++)
    {
        perframeStorageBufferObject.lightSpaceMatrix[i] = cascades[i].viewProjMatrix;
    }

    GPUTextureBarrier tex_barriers[1] = {};
    {
        tex_barriers[0].texture   = mDepthTexture;
        tex_barriers[0].src_state = GPU_RESOURCE_STATE_UNDEFINED;
        tex_barriers[0].dst_state = GPU_RESOURCE_STATE_DEPTH_WRITE;
    }
    GPUResourceBarrierDescriptor draw_barrier{};
    {
        draw_barrier.texture_barriers       = tex_barriers;
        draw_barrier.texture_barriers_count = sizeof(tex_barriers) / sizeof(GPUTextureBarrier);
    }
    GPUCmdResourceBarrier(cmd, &draw_barrier);

    GPUColorAttachment screenAttachment{};
    {
        screenAttachment.view         = mTextureView;
        screenAttachment.load_action  = GPU_LOAD_ACTION_CLEAR;
        screenAttachment.store_action = GPU_STORE_ACTION_STORE;
        screenAttachment.clear_color  = { { 0.f, 0.f, 0.f, 0.f } };
    }
    GPUDepthStencilAttachment ds_attachment{};
    {
        ds_attachment.view               = mDepthTextureView;
        ds_attachment.depth_load_action  = GPU_LOAD_ACTION_CLEAR;
        ds_attachment.depth_store_action = GPU_STORE_ACTION_STORE;
        ds_attachment.clear_depth        = 1.0f;
        ds_attachment.write_depth        = true;
    }
    GPURenderPassDescriptor render_pass_desc{};
    {
        render_pass_desc.sample_count        = GPU_SAMPLE_COUNT_1;
        render_pass_desc.color_attachments   = &screenAttachment;
        render_pass_desc.render_target_count = 1;
        render_pass_desc.depth_stencil       = &ds_attachment;
    }
    GPURenderPassEncoderID encoder = GPUCmdBeginRenderPass(cmd, &render_pass_desc);
    {
        GPURenderEncoderSetViewport(encoder, 0.f, 0.f, (float)mDepthTexture->width, (float)mDepthTexture->height, 0.f, 1.f);
        GPURenderEncoderSetScissor(encoder, 0, 0, mDepthTexture->width, mDepthTexture->height);
        // draw call
        GPURenderEncoderBindPipeline(encoder, mPipeline);

        for (auto& pair1 : drawCallBatch)
        {
            auto& meshInstanced = pair1.second;
            for (auto& pair2 : meshInstanced)
            {
                auto& meshf      = pair2.first;
                auto& mesh_nodes = pair2.second;

                global::GlobalGPUMeshRes* mesh = nullptr;
                global::GetGpuMeshRes(meshf, mesh);
                uint32_t totalInstanceCount = mesh_nodes.size();
                if (totalInstanceCount > 0)
                {
                    uint32_t strides = sizeof(global::GlobalMeshRes::Vertex);
                    GPURenderEncoderBindVertexBuffers(encoder, 1, &mesh->vertexBuffer, &strides, nullptr);
                    GPURenderEncoderBindIndexBuffer(encoder, mesh->indexBuffer, 0, sizeof(uint32_t));

                    uint32_t drawcallMaxInctanceCount = sizeof(global::MeshDirectionalLightShadowPerdrawcallStorageBufferObject::meshInstances) / sizeof(global::MeshDirectionalLightShadowPerdrawcallStorageBufferObject::meshInstances[0]);
                    uint32_t drawCount                = roundUp(totalInstanceCount, drawcallMaxInctanceCount) / drawcallMaxInctanceCount;
                    for (uint32_t drawcallIndex = 0; drawcallIndex < drawCount; drawcallIndex++)
                    {
                        uint32_t currInstanceCount =
                        ((totalInstanceCount - drawcallMaxInctanceCount * drawcallIndex) < drawcallMaxInctanceCount) ?
                        (totalInstanceCount - drawcallMaxInctanceCount * drawcallIndex) :
                        drawcallMaxInctanceCount;

                        uint32_t perdrawcall_dynamic_offset                                                 = roundUp(global::g_global_reader_resource.storage._global_upload_ringbuffers_end[frameIndex], global::g_global_reader_resource.storage.minAlignment);
                        global::g_global_reader_resource.storage._global_upload_ringbuffers_end[frameIndex] = perdrawcall_dynamic_offset + sizeof(global::MeshDirectionalLightShadowPerdrawcallStorageBufferObject);
                        assert(global::g_global_reader_resource.storage._global_upload_ringbuffers_end[frameIndex] <=
                               (global::g_global_reader_resource.storage._global_upload_ringbuffers_begin[frameIndex] + global::g_global_reader_resource.storage._global_upload_ringbuffers_size[frameIndex]));
                        global::MeshDirectionalLightShadowPerdrawcallStorageBufferObject& perdrawcallStorageBufferObject =
                        (*reinterpret_cast<global::MeshDirectionalLightShadowPerdrawcallStorageBufferObject*>(reinterpret_cast<uintptr_t>(global::g_global_reader_resource.storage.buffer->cpu_mapped_address) + perdrawcall_dynamic_offset));

                        for (uint32_t i = 0; i < currInstanceCount; i++)
                        {
                            perdrawcallStorageBufferObject.meshInstances[i].model = mesh_nodes[drawcallMaxInctanceCount * drawcallIndex + i].modelMatrix;
                        }

                        // bind perdrawcall set
                        uint32_t dynamicOffsets[2] = { perframe_dynamic_offset, perdrawcall_dynamic_offset };
                        GPURenderEncoderBindDescriptorSet(encoder, mSet, 2, dynamicOffsets);

                        // draw indexed
                        GPURenderEncoderDrawIndexedInstanced(encoder, mesh->indexCount, currInstanceCount, 0, 0, 0);
                    }
                }
            }
        }

        /* GPURenderEncoderBindDescriptorSet(encoder, mSet);
        GPURenderEncoderBindVertexBuffers(encoder, 1, &sceneInfo.vertexBuffer, &sceneInfo.strides, nullptr);
        GPURenderEncoderBindIndexBuffer(encoder, sceneInfo.indexBuffer, 0, sizeof(uint32_t));
        struct
        {
            glm::mat4 model;
            float offsets[8];
        } push;
        push.model = sceneInfo.modelMatrix;
        for (uint32_t i = 0; i < 8; i++)
        {
            push.offsets[i] = 50.f * i;
        }
        for (auto& nodePair : drawNodesInfo)
        {
            for (size_t i = 0; i < nodePair.second.size() && i < 1; i++)
            {
                // per mesh
                auto& mesh          = nodePair.second[i];
                uint32_t indexCount = mesh->indexCount;
                GPURenderEncoderPushConstant(encoder, mRS, &push);
                GPURenderEncoderDrawIndexedInstanced(encoder, indexCount, 1, mesh->indexOffset, mesh->vertexOffset, 0);
            }
        } */
    }
    GPUCmdEndRenderPass(cmd, encoder);

    GPUTextureBarrier tex_barrier1{};
    {
        tex_barrier1.texture   = mDepthTexture;
        tex_barrier1.src_state = GPU_RESOURCE_STATE_DEPTH_WRITE;
        tex_barrier1.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
    }
    GPUResourceBarrierDescriptor barrier{};
    {
        barrier.texture_barriers_count = 1;
        barrier.texture_barriers       = &tex_barrier1;
    }
    GPUCmdResourceBarrier(cmd, &barrier);
}

void CascadeShadowPass::CalculateDirectionalLightCamera2(const Camera& cam, const Culler& culler, const glm::vec3& lightDir, const EntityModel* modelEntity)
{
        // CreateEuler(glm::radians(0.f), glm::radians(45.f), glm::radians((0.f)));
        //  Calculate split depths based on view camera frustum
        //  Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
        float cascadeSplits[sCascadeCount];
        float lambda    = 0.95f;
        float n         = cam.getNearClip();
        float f         = cam.getFarClip();
        float clipRange = f - n;
        for (uint32_t i = 0; i < sCascadeCount; i++)
        {
            float p       = (i + 1) / (float)sCascadeCount;
            float log     = n * glm::pow(f / n, p);
            float uniform = n + clipRange * p;
            float d       = lambda * log + (1.f - lambda) * uniform; // l * log + un - l * un : lambda * (log - uniform) + uniform;

            cascadeSplits[i] = (d - n) / clipRange;
        }

        BoundingBox casterAABB1;
        /* {
            // just one entity for now;
            casterAABB.Merge(BoundingBox::BoundingBoxTransform(entityBoundingBox, entityModel));
        } */
        std::vector<BoundingBox> aabbArray1;
        culler.GetAllVisibleAABB(aabbArray1);
        for (auto& aabb : aabbArray1)
        {
            casterAABB1.Merge(aabb);
        }

        glm::mat4 vp    = cam.matrices.perspective * cam.matrices.view;
        glm::mat4 invVP = glm::inverse(vp);
        // Calculate orthographic projection matrix for each cascade
        size_t constexpr CORNER_COUNT = 8;
        float lastSplitDist           = 0.0;
        std::array<math::Vector3, 8> frustumPointsNDCSpace = cam.GetFrustumPoints();
        for (uint32_t i = 0; i < sCascadeCount; i++)
        {
            float splitDist                    = cascadeSplits[i];
            /* glm::vec3 frustumPointsNDCSpace[8] = {
                glm::vec3(-1.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, 1.0f, 0.0f),
                glm::vec3(-1.0f, 1.0f, 0.0f),
                glm::vec3(-1.0f, -1.0f, 1.0f),
                glm::vec3(1.0f, -1.0f, 1.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                glm::vec3(-1.0f, 1.0f, 1.0f),
            };
            for (size_t j = 0; j < CORNER_COUNT; ++j)
            {
                glm::vec4 frustumPointWith_w = invVP * glm::vec4(frustumPointsNDCSpace[j], 1.0);
                frustumPointsNDCSpace[j]     = frustumPointWith_w / frustumPointWith_w.w;
            } */

            //std::array<math::Vector3, 8> points = cam.GetFrustumPoints();

            Camera shadowCamera;
            shadowCamera.type = Camera::CameraType::firstperson;
            shadowCamera.setPerspective(cam.getFov(), cam.getAspect(), i == 0 ? n : clipRange * lastSplitDist, clipRange * splitDist);
            shadowCamera.setRotation(cam.rotation);
            shadowCamera.setPosition(cam.position);
            Culler shadowCuller;
            shadowCuller.ClearVisibleSet();
            shadowCuller.ClearAllPanel();
            shadowCuller.PushCameraPlane(shadowCamera);
            /* math::Matrix4X4 trans = modelEntity->mTransformComp.GetMatrix();
            for (auto& comp : modelEntity->mMeshComp.rawMeshes)
            {
                auto iter = global::g_cache_mesh_bounding_box.find(comp.meshFile);
                TransformComponent meshTransComp;
                meshTransComp.transform = comp.transform;
                BoundingBox aabb        = BoundingBox::BoundingBoxTransform(iter->second, trans * meshTransComp.GetMatrix());
                if (shadowCuller.IsVisible(aabb))
                {
                    shadowCuller.AddVisibleAABB(aabb);
                }
            } */
            std::vector<BoundingBox> oldArray;
            culler.GetAllVisibleAABB(oldArray);
            for (auto& aabb : oldArray)
            {
                if (shadowCuller.IsVisible(aabb))
                {
                    shadowCuller.AddVisibleAABB(aabb);
                }
            }
            BoundingBox casterAABB;
            std::vector<BoundingBox> aabbArray;
            shadowCuller.GetAllVisibleAABB(aabbArray);
            if (aabbArray.size() == 0)
            {
                cascades[i].splitDepth     = (n + splitDist * clipRange) * -1.0f;
                cascades[i].viewProjMatrix = math::Matrix4X4(1.f);
                lastSplitDist              = cascadeSplits[i];
                continue;
            }
            for (auto& aabb : aabbArray)
            {
                casterAABB.Merge(aabb);
            }

            BoundingBox receiverAABB;
            for (uint32_t j = 0; j < 4; j++)
            {
                glm::vec3 dist               = frustumPointsNDCSpace[j + 4] - frustumPointsNDCSpace[j];
                /* frustumPointsNDCSpace[j + 4] = frustumPointsNDCSpace[j] + (dist * splitDist);
                frustumPointsNDCSpace[j]     = frustumPointsNDCSpace[j] + (dist * lastSplitDist);
                receiverAABB.Update(frustumPointsNDCSpace[j]);
                receiverAABB.Update(frustumPointsNDCSpace[j + 4]); */

                math::Vector3 pointF = frustumPointsNDCSpace[j] + (dist * splitDist);
                math::Vector3 pointN = frustumPointsNDCSpace[j] + (dist * lastSplitDist);
                receiverAABB.Update(pointN);
                receiverAABB.Update(pointF);
            }

            glm::vec3 lightDir1 = glm::normalize(-lightDir);
            glm::mat4 lightRot  = LookDirRH(glm::vec3(0.f), lightDir1);

            BoundingBox newCasterAABB  = BoundingBox::BoundingBoxTransform(casterAABB, lightRot);
            BoundingBox newReceiveAABB = BoundingBox::BoundingBoxTransform(receiverAABB, lightRot);

            BoundingBox minAABB = newReceiveAABB.GetMin(newCasterAABB);
            glm::vec3 minP(minAABB.min.x, minAABB.min.y, newCasterAABB.min.z);
            glm::vec3 maxP(minAABB.max.x, minAABB.max.y, newCasterAABB.max.z);
            minAABB.min = minP;
            minAABB.max = maxP;

            glm::vec3 center = minAABB.GetCenter();

            glm::vec3 front(0.f, 0.f, -1);
            glm::vec3 rayOri(center);
            glm::vec3 rayDir(front * (-1.f)); // front vec3(0.f, 0.f, -1.f);

            float tNear = 0.f, tFar = 0.f;
            if (minAABB.RelationWithRay(rayOri, glm::normalize(rayDir), tNear, tFar) != 1)
            {
                cascades[i].splitDepth     = (cam.getNearClip() + splitDist * clipRange) * -1.0f;
                cascades[i].viewProjMatrix = glm::mat4(1.f);

                lastSplitDist = cascadeSplits[i];
                continue;
            }

            glm::vec3 lightPT    = center - front * tNear * 10.f;
            glm::vec3 newLigthPT = glm::inverse(lightRot) * glm::vec4(lightPT, 1.f);

            glm::mat4 lightViewMT = LookDirRH(newLigthPT, lightDir1);

            minAABB.max = minAABB.max - lightPT;
            minAABB.min = minAABB.min - lightPT;

            glm::mat4 ligthOrthoMT = glm::ortho(minAABB.min.x, minAABB.max.x, minAABB.min.y, minAABB.max.y, -minAABB.max.z, -minAABB.min.z);

            // Store split distance and matrix in cascade
            cascades[i].splitDepth     = (cam.getNearClip() + splitDist * clipRange) * -1.0f;
            cascades[i].viewProjMatrix = ligthOrthoMT * lightViewMT;

            lastSplitDist = cascadeSplits[i];
        }
}