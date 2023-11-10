#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <vector>
#include <array>
#include "stb_image.h"
#include "Gpu/GpuApi.h"
#include <array>
#include "Math/Matrix.h"
#include "ECS/Entity.h"
#include "ECS/Component.h"
#include "boundingbox.hpp"

using namespace FakeReal;

class EntityModel
{
public:
    EntityModel(const std::string_view file, GPUDeviceID device, GPUQueueID gfxQueue);
    ~EntityModel();

    void UploadRenderResource();

public:

    std::string mMeshFile;
    GPUDeviceID mDevice;
    GPUQueueID mGfxQueue;
    GPURootSignatureID mRootSignature;
    GPUSamplerID mSampler{nullptr};

    ///
    FakeReal::MeshComponent mMeshComp;
    FakeReal::TransformComponent mTransformComp;
    BoundingBox mAABB;
};