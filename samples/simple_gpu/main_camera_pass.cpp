#include "main_camera_pass.hpp"

MainCameraPass::MainCameraPass()
{

}

MainCameraPass::~MainCameraPass()
{
    if (mPbrPipeline) GPUFreeRenderPipeline(mPbrPipeline); mPbrPipeline = nullptr;
    if (mRootSignature) GPUFreeRootSignature(mRootSignature); mRootSignature = nullptr;
}

void MainCameraPass::Initialize(GPUDeviceID device, GPUQueueID gfxQueue)
{
    mDevice   = device;
    mGfxQueue = gfxQueue;
}