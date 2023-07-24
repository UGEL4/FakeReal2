#include "render_effect.h"
#include "RenderSystem.h"
#include "RenderDevice.h"
#include <filesystem>

inline static void ReadBytes(const char8_t* file_name, uint32_t** bytes, uint32_t* length)
{
    FILE* f = fopen((const char*)file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (uint32_t*)malloc(*length);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void ReadShaderBytes(const char8_t* virtual_path, uint32_t** bytes, uint32_t* length)
{
    std::filesystem::path cur_path = std::filesystem::current_path();
    std::u8string shader_path      = cur_path.u8string();
    shader_path.append(u8"./../shaders/");
    char8_t shader_file[256];
    strcpy((char*)shader_file, (const char*)shader_path.c_str());
    strcat((char*)shader_file, (const char*)virtual_path);
    strcat((char*)shader_file, (const char*)(u8".spv"));
    ReadBytes(shader_file, bytes, length);
}

RenderEffectForward::RenderEffectForward()
{}

RenderEffectForward::~RenderEffectForward()
{}

void RenderEffectForward::OnRegister(RenderSystem* pRender)
{
    CreatePipeline(pRender);
    PrepareResources(pRender);
}

void RenderEffectForward::OnUnregister(RenderSystem* pRender)
{
    FreeResources();
    GPUFreeRenderPipeline(mPipeline);
}

void RenderEffectForward::ProduceDraw(const PrimitiveDrawContext* context)
{}

void RenderEffectForward::PrepareResources(RenderSystem* pRender)
{
    struct Vertex {
        float x;
        float y;
        float z;
        float r;
        float g;
        float b;
        float u;
        float v;
    };
    Vertex vertices[] = {
        { -0.5f, 0.5f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f },
        { -0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f },
        { 0.5f, -0.5f, 0.f, 1.f, 0.f, 1.f, 1.f, 1.f },
        { 0.5f, 0.5f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f }
    };

    uint16_t indices[] = {
        0, 1, 2, 0, 2, 3
    };

    const auto renderDevice = pRender->GetRenderDevice();
    const auto device = renderDevice->GetGpuDevice();
    const auto queue = renderDevice->GetGFXQueue();

    GPUBufferDescriptor vb_desc{};
    vb_desc.size         = sizeof(vertices);
    vb_desc.descriptors  = GPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vb_desc.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
    vb_desc.flags        = GPU_BCF_NONE;
    mVertexBuffer        = GPUCreateBuffer(device, &vb_desc);

    GPUBufferDescriptor ib_desc{};
    ib_desc.size         = sizeof(indices);
    ib_desc.descriptors  = GPU_RESOURCE_TYPE_INDEX_BUFFER;
    ib_desc.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
    ib_desc.flags        = GPU_BCF_NONE;
    mIndexBuffer         = GPUCreateBuffer(device, &ib_desc);

    GPUBufferDescriptor uploadBufferDesc{};
    uploadBufferDesc.size         = sizeof(vertices) + sizeof(indices);
    uploadBufferDesc.descriptors  = GPU_RESOURCE_TYPE_NONE;
    uploadBufferDesc.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
    uploadBufferDesc.flags        = GPU_BCF_PERSISTENT_MAP_BIT;
    GPUBufferID uploadBuffer      = GPUCreateBuffer(device, &uploadBufferDesc);

    GPUCommandPoolID pool = GPUCreateCommandPool(queue);
    GPUCommandBufferDescriptor cmd_desc{};
    cmd_desc.isSecondary   = false;
    GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmd_desc);

    memcpy(uploadBuffer->cpu_mapped_address, vertices, sizeof(vertices));
    GPUCmdBegin(cmd);
    {
        GPUBufferToBufferTransfer tr{};
        tr.dst        = mVertexBuffer;
        tr.dst_offset = 0;
        tr.src        = uploadBuffer;
        tr.src_offset = 0;
        tr.size       = sizeof(vertices);
        GPUCmdTransferBufferToBuffer(cmd, &tr);

        memcpy((char8_t*)uploadBuffer->cpu_mapped_address + sizeof(vertices), indices, sizeof(indices));
        GPUBufferToBufferTransfer tr1{};
        tr1.dst        = mIndexBuffer;
        tr1.dst_offset = 0;
        tr1.src        = uploadBuffer;
        tr1.src_offset = sizeof(vertices);
        tr1.size       = sizeof(indices);
        GPUCmdTransferBufferToBuffer(cmd, &tr1);

        GPUBufferBarrier barriers[2];
        barriers[0].buffer    = mVertexBuffer;
        barriers[0].src_state = GPU_RESOURCE_STATE_COPY_DEST;
        barriers[0].dst_state = GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        barriers[1].buffer    = mIndexBuffer;
        barriers[1].src_state = GPU_RESOURCE_STATE_COPY_DEST;
        barriers[1].dst_state = GPU_RESOURCE_STATE_INDEX_BUFFER;
        GPUResourceBarrierDescriptor barrierDesc{};
        barrierDesc.buffer_barriers       = barriers;
        barrierDesc.buffer_barriers_count = 2;
        GPUCmdResourceBarrier(cmd, &barrierDesc);
    }
    GPUCmdEnd(cmd);
    GPUQueueSubmitDescriptor submitDesc {};
    submitDesc.cmds       = &cmd;
    submitDesc.cmds_count = 1;
    GPUSubmitQueue(queue, &submitDesc);
    GPUWaitQueueIdle(queue);
    GPUFreeBuffer(uploadBuffer);
    GPUFreeCommandBuffer(cmd);
    GPUFreeCommandPool(pool);
}

void RenderEffectForward::FreeResources()
{
    GPUFreeBuffer(mVertexBuffer);
    GPUFreeBuffer(mIndexBuffer);
}

void RenderEffectForward::CreatePipeline(RenderSystem* pRender)
{
    const auto renderDevice = pRender->GetRenderDevice();
    const auto device       = renderDevice->GetGpuDevice();

    uint32_t* vShaderCode;
    uint32_t vSize = 0;
    ReadShaderBytes(u8"vertex_shader.vert", &vShaderCode, &vSize);
    uint32_t* fShaderCode;
    uint32_t fSize = 0;
    ReadShaderBytes(u8"fragment_shader.frag", &fShaderCode, &fSize);
    GPUShaderLibraryDescriptor vShaderDesc{};
    vShaderDesc.pName    = u8"vertex_shader";
    vShaderDesc.code     = vShaderCode;
    vShaderDesc.codeSize = vSize;
    vShaderDesc.stage    = GPU_SHADER_STAGE_VERT;
    GPUShaderLibraryDescriptor fShaderDesc{};
    fShaderDesc.pName    = u8"fragment_shader";
    fShaderDesc.code     = fShaderCode;
    fShaderDesc.codeSize = fSize;
    fShaderDesc.stage    = GPU_SHADER_STAGE_FRAG;
    GPUShaderLibraryID pVShader = GPUCreateShaderLibrary(device, &vShaderDesc);
    GPUShaderLibraryID pFShader = GPUCreateShaderLibrary(device, &fShaderDesc);
    free(vShaderCode);
    free(fShaderCode);
    GPUShaderEntryDescriptor shaderEntries[2] = {0};
    shaderEntries[0].stage                    = GPU_SHADER_STAGE_VERT;
    shaderEntries[0].entry                    = u8"main";
    shaderEntries[0].pLibrary                 = pVShader;
    shaderEntries[1].stage                    = GPU_SHADER_STAGE_FRAG;
    shaderEntries[1].entry                    = u8"main";
    shaderEntries[1].pLibrary                 = pFShader;

    //create root signature
    GPURootSignatureDescriptor rootRSDesc     = {};
    rootRSDesc.shaders                        = shaderEntries;
    rootRSDesc.shader_count                   = 2;
    /* rootRSDesc.static_sampler_names           = &sampler_name;
    rootRSDesc.static_sampler_count           = 1;
    rootRSDesc.static_samplers                = &texture_sampler; */
    GPURootSignatureID pRS                    = GPUCreateRootSignature(device, &rootRSDesc);

    //vertex layout
    GPUVertexLayout vertexLayout{};
    vertexLayout.attributeCount = 3;
    vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
    // renderpipeline
    auto fmt = GPU_FORMAT_B8G8R8A8_UNORM;
    GPURenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.pRootSignature    = pRS;
    pipelineDesc.pVertexShader     = &shaderEntries[0];
    pipelineDesc.pFragmentShader   = &shaderEntries[1];
    pipelineDesc.pVertexLayout     = &vertexLayout;
    pipelineDesc.primitiveTopology = GPU_PRIM_TOPO_TRI_LIST;
    pipelineDesc.pColorFormats     = &fmt;
    pipelineDesc.renderTargetCount = 1;
    mPipeline = GPUCreateRenderPipeline(device, &pipelineDesc);
    GPUFreeShaderLibrary(pVShader);
    GPUFreeShaderLibrary(pFShader);
}