#pragma once
#include "Gpu/GpuApi.h"

class MainCameraPass
{
public:
    MainCameraPass();
    ~MainCameraPass();

    void Initialize(GPUDeviceID device, GPUQueueID gfxQueue);

    GPUDeviceID mDevice;
    GPUQueueID mGfxQueue;
    GPURenderPipelineID mPbrPipeline;
    GPURootSignatureID mRootSignature;
};