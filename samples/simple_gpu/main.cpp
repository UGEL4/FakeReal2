#include "Gpu/GpuApi.h"
#include <memory>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "texture.h"
#include "Frontend/RenderGraph.h"
#include "model.hpp"
#include "Utils/Log/LogSystem.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "sky_box.hpp"

static int WIDTH = 1080;
static int HEIGHT = 1080;
#define FLIGHT_FRAMES 3

GPUTextureID depthTexture = nullptr;
GPUTextureViewID depthTextureView = nullptr;
GPUSamplerID staticSampler = nullptr;
GPUInstanceID instance = nullptr;
GPUDeviceID device = nullptr;
GPUAdapterID adapter = nullptr;
GPUSurfaceID surface = nullptr;
GPUQueueID graphicQueue = nullptr;
GPUFenceID presenFences[FLIGHT_FRAMES];
GPUSwapchainID swapchain = nullptr;
GPUTextureViewID ppSwapchainImage[FLIGHT_FRAMES];
GPUCommandPoolID pools[FLIGHT_FRAMES];
GPUCommandBufferID cmds[FLIGHT_FRAMES];
GPUSemaphoreID presentSemaphore = nullptr;
const char8_t* sampler_name  = u8"texSamp";

GPURootSignatureID RS = nullptr;
GPUDescriptorSetID set = nullptr;
GPURenderPipelineID pipeline = nullptr;
GPURenderPipelineID debugNormalPipeline = nullptr;
GPUTextureID texture = nullptr;
GPUTextureViewID textureView = nullptr;
GPUBufferID vertexBuffer  = nullptr;
GPUBufferID indexBuffer  = nullptr;
GPUBufferID uniformBuffer = nullptr;
GPUBufferID lightUniformBuffer = nullptr;
uint32_t normalMeshindexCount = 0;

GPURootSignatureID modelRS = nullptr;
GPUDescriptorSetID modelSet = nullptr;
GPUDescriptorSetID modelSet2 = nullptr;
GPUBufferID modelVertexBuffer = nullptr;
GPUBufferID modelIndexBuffer  = nullptr;
GPURenderPipelineID modelRenderPipeline = nullptr;
GPUBufferID modelUniformBuffer = nullptr;
GPUBufferID modelLightUniformBuffer = nullptr;
Model* pModel = nullptr;
std::vector<TextureData> modelTextures;


/* inline static void ReadBytes(const char8_t* file_name, uint32_t** bytes, uint32_t* length)
{
    FILE* f = fopen((const char*)file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (uint32_t*)malloc(*length);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void ReadShaderBytes(const char8_t* virtual_path, uint32_t** bytes, uint32_t* length, EGPUBackend backend)
{
    std::filesystem::path cur_path = std::filesystem::current_path();
    std::u8string shader_path      = cur_path.u8string();
    shader_path.append(u8"./../shaders/");
    char8_t shader_file[256];
    strcpy((char*)shader_file, (const char*)shader_path.c_str());
    strcat((char*)shader_file, (const char*)virtual_path);
    switch (backend)
    {
        case EGPUBackend::GPUBackend_Vulkan:
            strcat((char*)shader_file, (const char*)(u8".spv"));
            break;
        default:
            break;
    }
    ReadBytes(shader_file, bytes, length);
} */

LRESULT CALLBACK WindowProcedure(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_DESTROY:
            std::cout << "\ndestroying window\n";
            PostQuitMessage(0);
            return 0L;
        case WM_LBUTTONDOWN:
            std::cout << "\nmouse left button down at (" << LOWORD(lp) << ',' << HIWORD(lp) << ")\n";
        default:
            return DefWindowProc(window, msg, wp, lp);
    }
}

HWND CreateWin32Window()
{
    // Register the window class.
    auto myclass        = L"myclass";
    WNDCLASSEX wndclass = {
        sizeof(WNDCLASSEX), CS_DBLCLKS,
        WindowProcedure,
        0, 0, GetModuleHandle(0), LoadIcon(0, IDI_APPLICATION),
        LoadCursor(0, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1),
        0, myclass, LoadIcon(0, IDI_APPLICATION)
    };
    static bool bRegistered = RegisterClassEx(&wndclass);
    if (bRegistered)
    {
        HWND window = CreateWindowEx(0, myclass, TEXT("title"),
                                     WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                     WIDTH, HEIGHT, 0, 0, GetModuleHandle(0), 0);
        if (window)
        {
            ShowWindow(window, SW_SHOWDEFAULT);
        }
        return window;
    }
    return nullptr;
}

GPUTextureID CreateTexture(GPUDeviceID device, GPUQueueID queue)
{
    GPUTextureDescriptor desc{};
    desc.flags = GPU_TCF_OWN_MEMORY_BIT;
    desc.width = TEXTURE_WIDTH;
    desc.height = TEXTURE_HEIGHT;
    desc.depth  = 1;
    desc.array_size = 1;
    desc.format     = GPU_FORMAT_R8G8BA8_UNORM;
    desc.owner_queue = queue;
    desc.start_state = GPU_RESOURCE_STATE_COPY_DEST;
    desc.descriptors = GPU_RESOURCE_TYPE_TEXTURE;
    return GPUCreateTexture(device, &desc);
}

GPUTextureViewID CreateTextureView(GPUTextureID texture)
{
    GPUTextureViewDescriptor desc{};
    desc.pTexture        = texture;
    desc.format          = (EGPUFormat)texture->format;
    desc.usages          = EGPUTexutreViewUsage::GPU_TVU_SRV;
    desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR;
    desc.baseMipLevel    = 0;
    desc.mipLevelCount   = 1;
    desc.baseArrayLayer  = 0;
    desc.arrayLayerCount = 1;
    return GPUCreateTextureView(texture->pDevice, &desc);
}

GPUSamplerID CreateTextureSampler(GPUDeviceID device)
{
    GPUSamplerDescriptor desc{};
    desc.min_filter = GPU_FILTER_TYPE_LINEAR;
    desc.mag_filter = GPU_FILTER_TYPE_LINEAR;
    desc.mipmap_mode = GPU_MIPMAP_MODE_LINEAR;
    desc.address_u   = GPU_ADDRESS_MODE_REPEAT;
    desc.address_v  = GPU_ADDRESS_MODE_REPEAT;
    desc.address_w   = GPU_ADDRESS_MODE_REPEAT;
    desc.compare_func = GPU_CMP_NEVER;
    return GPUCreateSampler(device, &desc);
}

void CreateNormalRendeObjects()
{
    //shader
    uint32_t* vShaderCode;
    uint32_t vSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/vertex_shader.vert", &vShaderCode, &vSize);
    uint32_t* fShaderCode;
    uint32_t fSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/fragment_shader.frag", &fShaderCode, &fSize);
    GPUShaderLibraryDescriptor vShaderDesc{};
    {
        vShaderDesc.pName    = u8"vertex_shader";
        vShaderDesc.code     = vShaderCode;
        vShaderDesc.codeSize = vSize;
        vShaderDesc.stage    = GPU_SHADER_STAGE_VERT;
    }
    GPUShaderLibraryDescriptor fShaderDesc{};
    {
        fShaderDesc.pName    = u8"fragment_shader";
        fShaderDesc.code     = fShaderCode;
        fShaderDesc.codeSize = fSize;
        fShaderDesc.stage    = GPU_SHADER_STAGE_FRAG;
    }
    GPUShaderLibraryID pVShader = GPUCreateShaderLibrary(device, &vShaderDesc);
    GPUShaderLibraryID pFShader = GPUCreateShaderLibrary(device, &fShaderDesc);
    free(vShaderCode);
    free(fShaderCode);

    uint32_t* normal_vShaderCode;
    uint32_t normal_vSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/normal_geom.vert", &normal_vShaderCode, &normal_vSize);
    uint32_t* normal_fShaderCode;
    uint32_t normal_fSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/normal_geom.frag", &normal_fShaderCode, &normal_fSize);
    uint32_t* normal_gShaderCode;
    uint32_t normal_gSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/normal_geom.geom", &normal_gShaderCode, &normal_gSize);
    GPUShaderLibraryDescriptor normal_vShaderDesc{};
    normal_vShaderDesc.pName    = u8"normal_vertex_shader";
    normal_vShaderDesc.code     = normal_vShaderCode;
    normal_vShaderDesc.codeSize = normal_vSize;
    normal_vShaderDesc.stage    = GPU_SHADER_STAGE_VERT;
    GPUShaderLibraryDescriptor normal_fShaderDesc{};
    normal_fShaderDesc.pName    = u8"normal_fragment_shader";
    normal_fShaderDesc.code     = normal_fShaderCode;
    normal_fShaderDesc.codeSize = normal_fSize;
    normal_fShaderDesc.stage    = GPU_SHADER_STAGE_FRAG;
    GPUShaderLibraryDescriptor normal_gShaderDesc{};
    normal_gShaderDesc.pName    = u8"normal_geometry_shader";
    normal_gShaderDesc.code     = normal_gShaderCode;
    normal_gShaderDesc.codeSize = normal_gSize;
    normal_gShaderDesc.stage    = GPU_SHADER_STAGE_GEOM;
    GPUShaderLibraryID pNVShader = GPUCreateShaderLibrary(device, &normal_vShaderDesc);
    GPUShaderLibraryID pNFShader = GPUCreateShaderLibrary(device, &normal_fShaderDesc);
    GPUShaderLibraryID pNGShader = GPUCreateShaderLibrary(device, &normal_gShaderDesc);
    free(normal_vShaderCode);
    free(normal_fShaderCode);
    free(normal_gShaderCode);

    GPUShaderEntryDescriptor shaderEntries[5] = {0};
    {
        shaderEntries[0].stage    = GPU_SHADER_STAGE_VERT;
        shaderEntries[0].entry    = u8"main";
        shaderEntries[0].pLibrary = pVShader;
        shaderEntries[1].stage    = GPU_SHADER_STAGE_FRAG;
        shaderEntries[1].entry    = u8"main";
        shaderEntries[1].pLibrary = pFShader;
        shaderEntries[2].stage    = GPU_SHADER_STAGE_VERT;
        shaderEntries[2].entry    = u8"main";
        shaderEntries[2].pLibrary = pNVShader;
        shaderEntries[3].stage    = GPU_SHADER_STAGE_FRAG;
        shaderEntries[3].entry    = u8"main";
        shaderEntries[3].pLibrary = pNFShader;
        shaderEntries[4].stage    = GPU_SHADER_STAGE_GEOM;
        shaderEntries[4].entry    = u8"main";
        shaderEntries[4].pLibrary = pNGShader;
    }

    //create root signature
    GPURootSignatureDescriptor rootRSDesc = {};
    rootRSDesc.shaders                    = shaderEntries;
    rootRSDesc.shader_count               = 5;
    rootRSDesc.static_sampler_names       = &sampler_name;
    rootRSDesc.static_sampler_count       = 1;
    rootRSDesc.static_samplers            = &staticSampler;
    RS                                    = GPUCreateRootSignature(device, &rootRSDesc);

    //create descriptorset
    GPUDescriptorSetDescriptor set_desc{};
    set_desc.root_signature = RS;
    set_desc.set_index      = 0;
    set                     = GPUCreateDescriptorSet(device, &set_desc);

    //vertex layout
    GPUVertexLayout vertexLayout{};
    vertexLayout.attributeCount = 3;
    vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
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
    GPUDepthStateDesc depthDesc{};
    depthDesc.depthTest  = true;
    depthDesc.depthWrite = true;
    depthDesc.depthFunc  = GPU_CMP_LEQUAL;
    GPURenderPipelineDescriptor pipelineDesc{};
    {
        pipelineDesc.pRootSignature    = RS;
        pipelineDesc.pVertexShader     = &shaderEntries[0];
        pipelineDesc.pFragmentShader   = &shaderEntries[1];
        pipelineDesc.pVertexLayout     = &vertexLayout;
        pipelineDesc.primitiveTopology = GPU_PRIM_TOPO_TRI_LIST;
        pipelineDesc.pDepthState       = &depthDesc;
        pipelineDesc.pRasterizerState  = &rasterizerState;
        pipelineDesc.pColorFormats     = const_cast<EGPUFormat*>(&ppSwapchainImage[0]->desc.format);
        pipelineDesc.renderTargetCount = 1;
        pipelineDesc.depthStencilFormat = GPU_FORMAT_D32_SFLOAT;
    }
    pipeline   = GPUCreateRenderPipeline(device, &pipelineDesc);
    GPUFreeShaderLibrary(pVShader);
    GPUFreeShaderLibrary(pFShader);

    //normal pipeline
    GPURenderPipelineDescriptor debug_normal_pipelineDesc = pipelineDesc;
    debug_normal_pipelineDesc.pVertexShader               = &shaderEntries[2];
    debug_normal_pipelineDesc.pFragmentShader             = &shaderEntries[3];
    debug_normal_pipelineDesc.pGeometryShader             = &shaderEntries[4];
    debugNormalPipeline                                   = GPUCreateRenderPipeline(device, &debug_normal_pipelineDesc);
    GPUFreeShaderLibrary(pNVShader);
    GPUFreeShaderLibrary(pNFShader);
    GPUFreeShaderLibrary(pNGShader);
    // end create renderpipeline

    texture     = CreateTexture(device, graphicQueue);
    textureView = CreateTextureView(texture);

    //vertex buffer
    //std::vector<Vertex> vertices = Sphere::GenCubeVertices();
    std::vector<Vertex> vertices = Sphere::GenSphereVertices();
    //std::vector<uint32_t> indices = Sphere::GenCubeIndices();
    std::vector<uint32_t> indices = Sphere::GenSphereIndices();
    normalMeshindexCount = indices.size();
    // start upload resources
    uint32_t uploadBufferSize = sizeof(TEXTURE_DATA);
    if (uploadBufferSize < (sizeof(Vertex) * vertices.size()))
    {
        uploadBufferSize = sizeof(Vertex) * vertices.size();
    }
    if (uploadBufferSize < (sizeof(uint32_t) * indices.size()))
    {
        uploadBufferSize = sizeof(uint32_t) * indices.size();
    }
    GPUBufferDescriptor upload_buffer{};
    upload_buffer.size         = uploadBufferSize;
    upload_buffer.flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer.descriptors  = GPU_RESOURCE_TYPE_NONE;
    upload_buffer.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
    GPUBufferID uploadBuffer   = GPUCreateBuffer(device, &upload_buffer);
    //copy texture
    memcpy(uploadBuffer->cpu_mapped_address, TEXTURE_DATA, sizeof(TEXTURE_DATA));
    GPUResetCommandPool(pools[0]);
    GPUCmdBegin(cmds[0]);
    {
        GPUBufferToTextureTransfer trans_texture_buffer_desc{};
        trans_texture_buffer_desc.dst                              = texture;
        trans_texture_buffer_desc.dst_subresource.mip_level        = 0;
        trans_texture_buffer_desc.dst_subresource.base_array_layer = 0;
        trans_texture_buffer_desc.dst_subresource.layer_count      = 1;
        trans_texture_buffer_desc.src                              = uploadBuffer;
        trans_texture_buffer_desc.src_offset                       = 0;
        GPUCmdTransferBufferToTexture(cmds[0], &trans_texture_buffer_desc);
        GPUTextureBarrier barrier{};
        barrier.texture   = texture;
        barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
        barrier.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
        GPUResourceBarrierDescriptor rs_barrer{};
        rs_barrer.texture_barriers      = &barrier;
        rs_barrer.texture_barriers_count = 1;
        GPUCmdResourceBarrier(cmds[0], &rs_barrer);
    }
    GPUCmdEnd(cmds[0]);
    GPUQueueSubmitDescriptor texture_cpy_submit = { .cmds = &cmds[0], .cmds_count = 1 };
    GPUSubmitQueue(graphicQueue, &texture_cpy_submit);
    GPUWaitQueueIdle(graphicQueue);

    GPUBufferDescriptor vertex_desc{};
    vertex_desc.size         = sizeof(Vertex) * vertices.size();
    vertex_desc.flags        = GPU_BCF_OWN_MEMORY_BIT;
    vertex_desc.descriptors  = GPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vertex_desc.memory_usage = GPU_MEM_USAGE_GPU_ONLY;
    vertexBuffer             = GPUCreateBuffer(device, &vertex_desc);
    //COPY
    memcpy(uploadBuffer->cpu_mapped_address, vertices.data(), vertex_desc.size);
    GPUResetCommandPool(pools[0]);
    GPUCmdBegin(cmds[0]);
    {
        GPUBufferToBufferTransfer trans_verticex_buffer_desc{};
        trans_verticex_buffer_desc.size       = vertex_desc.size;
        trans_verticex_buffer_desc.src        = uploadBuffer;
        trans_verticex_buffer_desc.src_offset = 0;
        trans_verticex_buffer_desc.dst        = vertexBuffer;
        trans_verticex_buffer_desc.dst_offset = 0;
        GPUCmdTransferBufferToBuffer(cmds[0], &trans_verticex_buffer_desc);
        GPUBufferBarrier vertex_barrier{};
        vertex_barrier.buffer    = vertexBuffer;
        vertex_barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
        vertex_barrier.dst_state = GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        GPUResourceBarrierDescriptor vertex_rs_barrer{};
        vertex_rs_barrer.buffer_barriers       = &vertex_barrier;
        vertex_rs_barrer.buffer_barriers_count = 1;
        GPUCmdResourceBarrier(cmds[0], &vertex_rs_barrer);
    }
    GPUCmdEnd(cmds[0]);
    GPUQueueSubmitDescriptor cpy_submit = { .cmds = &cmds[0], .cmds_count = 1 };
    GPUSubmitQueue(graphicQueue, &cpy_submit);
    GPUWaitQueueIdle(graphicQueue);

    //index buffer
    GPUBufferDescriptor index_desc{};
    index_desc.size         = sizeof(uint32_t) * indices.size();
    index_desc.flags        = GPU_BCF_OWN_MEMORY_BIT;
    index_desc.descriptors  = GPU_RESOURCE_TYPE_INDEX_BUFFER;
    index_desc.memory_usage = GPU_MEM_USAGE_GPU_ONLY;
    indexBuffer = GPUCreateBuffer(device, &index_desc);
    //copy
    memcpy(uploadBuffer->cpu_mapped_address, indices.data(), index_desc.size);
    GPUResetCommandPool(pools[0]);
    GPUCmdBegin(cmds[0]);
    {
        GPUBufferToBufferTransfer trans_index_buffer_desc{};
        trans_index_buffer_desc.size       = index_desc.size;
        trans_index_buffer_desc.src        = uploadBuffer;
        trans_index_buffer_desc.src_offset = 0;
        trans_index_buffer_desc.dst        = indexBuffer;
        trans_index_buffer_desc.dst_offset = 0;
        GPUCmdTransferBufferToBuffer(cmds[0], &trans_index_buffer_desc);
        GPUBufferBarrier index_barrier{};
        index_barrier.buffer    = indexBuffer;
        index_barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
        index_barrier.dst_state = GPU_RESOURCE_STATE_INDEX_BUFFER;
        GPUResourceBarrierDescriptor index_rs_barrer{};
        index_rs_barrer.buffer_barriers       = &index_barrier;
        index_rs_barrer.buffer_barriers_count = 1;
        GPUCmdResourceBarrier(cmds[0], &index_rs_barrer);
    }
    GPUCmdEnd(cmds[0]);
    GPUQueueSubmitDescriptor index_cpy_submit = { .cmds = &cmds[0], .cmds_count = 1 };
    GPUSubmitQueue(graphicQueue, &index_cpy_submit);
    GPUWaitQueueIdle(graphicQueue);
    GPUFreeBuffer(uploadBuffer);
    // end upload resources

    //uniform
    GPUBufferDescriptor uniform_buffer{};
    uniform_buffer.size             = sizeof(UniformBuffer);
    uniform_buffer.flags            = GPU_BCF_NONE;
    uniform_buffer.descriptors      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    uniform_buffer.memory_usage     = GPU_MEM_USAGE_CPU_TO_GPU;
    uniform_buffer.prefer_on_device = true;
    uniformBuffer                   = GPUCreateBuffer(device, &uniform_buffer);

    //light
    GPUBufferDescriptor light_param_uniform_buffer{};
    light_param_uniform_buffer.size             = sizeof(LightParam);
    light_param_uniform_buffer.flags            = GPU_BCF_NONE;
    light_param_uniform_buffer.descriptors      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    light_param_uniform_buffer.memory_usage     = GPU_MEM_USAGE_CPU_TO_GPU;
    light_param_uniform_buffer.prefer_on_device = true;
    lightUniformBuffer                          = GPUCreateBuffer(device, &light_param_uniform_buffer);

    // update descriptorset
    {
        GPUDescriptorData desc_data[3] = {};
        desc_data[0].name              = u8"tex"; // shader texture2D`s name
        desc_data[0].binding           = 2;
        desc_data[0].binding_type      = GPU_RESOURCE_TYPE_TEXTURE;
        desc_data[0].count             = 1;
        desc_data[0].textures          = &textureView;
        desc_data[1].name              = u8"ubo";
        desc_data[1].binding           = 0;
        desc_data[1].binding_type      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
        desc_data[1].count             = 1;
        desc_data[1].buffers           = &uniformBuffer;
        desc_data[2].name              = u8"Lights";
        desc_data[2].binding           = 0;
        desc_data[2].binding_type      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
        desc_data[2].count             = 1;
        desc_data[2].buffers           = &lightUniformBuffer;
        GPUUpdateDescriptorSet(set, desc_data, 3);
    }
}

void DrawNormalObject(GPURenderPassEncoderID encoder, const LightParam& light, const glm::vec4& viewPos, const glm::mat4& view, const glm::mat4 proj)
{
    GPUBufferRange rang{};
    rang.offset = 0;
    rang.size   = sizeof(LightParam);
    GPUMapBuffer(lightUniformBuffer, &rang);
    memcpy(lightUniformBuffer->cpu_mapped_address, &light, rang.size);
    GPUUnmapBuffer(lightUniformBuffer);

    glm::mat4 m = glm::translate(glm::mat4(1.f), { 1.f, 0.f, -2.f });
    UniformBuffer ubo{};
    ubo.model   = glm::rotate(m, glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));
    ubo.view    = view;
    ubo.proj    = proj;
    ubo.viewPos = viewPos;
    rang.offset = 0;
    rang.size   = sizeof(UniformBuffer);
    GPUMapBuffer(uniformBuffer, &rang);
    memcpy(uniformBuffer->cpu_mapped_address, &ubo, rang.size);
    GPUUnmapBuffer(uniformBuffer);

    constexpr const static bool visualizeNormal = false;

    GPURenderEncoderBindPipeline(encoder, pipeline);
    // bind vertexbuffer
    uint32_t stride = sizeof(Vertex);
    GPURenderEncoderBindVertexBuffers(encoder, 1, &vertexBuffer, &stride, nullptr);
    uint32_t indexStride = sizeof(uint32_t);
    GPURenderEncoderBindIndexBuffer(encoder, indexBuffer, 0, indexStride);
    // bind descriptor ste
    GPURenderEncoderBindDescriptorSet(encoder, set);
    /* glm::vec4 pos = glm::vec4(0.f, 0.f, 0.f, 1.f);
    PushConstant push_constant{};
    push_constant.objOffsetPos = pos;
    push_constant.ao           = 1.f;
    push_constant.metallic     = 0.3f;
    push_constant.roughness    = 0.05f;
    GPURenderEncoderPushConstant(encoder, RS, &push_constant);
    GPURenderEncoderDrawIndexed(encoder, normalMeshindexCount, 0, 0); */

    for (uint32_t y = 0; y < 7; y++)
    {
        for (uint32_t x = 0; x < 7; x++)
        {
            glm::vec4 pos = glm::vec4(float(x - (7 / 2.0f)) * 2.5f, float(y - (7 / 2.0f)) * 2.5f, 0.f, 1.f);
            PushConstant push_constant{};
            push_constant.objOffsetPos = pos;
            push_constant.ao           = 1.f;
            push_constant.metallic     = glm::clamp((float)y / (float)(7), 0.1f, 1.0f);
            push_constant.roughness    = glm::clamp((float)x / (float)(7), 0.05f, 1.0f);
            GPURenderEncoderPushConstant(encoder, RS, &push_constant);
            GPURenderEncoderDrawIndexed(encoder, normalMeshindexCount, 0, 0);
        }
    }

    if constexpr (visualizeNormal)
    {
        GPURenderEncoderBindPipeline(encoder, debugNormalPipeline);
        for (uint32_t y = 0; y < 7; y++)
        {
            for (uint32_t x = 0; x < 7; x++)
            {
                glm::vec4 pos = glm::vec4(float(x - (7 / 2.0f)) * 2.5f, float(y - (7 / 2.0f)) * 2.5f, 0.f, 1.f);
                PushConstant push_constant{};
                push_constant.objOffsetPos = pos;
                push_constant.ao           = 1.f;
                push_constant.metallic     = glm::clamp((float)y / (float)(7), 0.1f, 1.0f);
                push_constant.roughness    = glm::clamp((float)x / (float)(7), 0.05f, 1.0f);
                GPURenderEncoderPushConstant(encoder, RS, &push_constant);
                GPURenderEncoderDrawIndexed(encoder, normalMeshindexCount, 0, 0);
            }
        }
        /* GPURenderEncoderBindPipeline(encoder, debugNormalPipeline);
        GPURenderEncoderDrawIndexed(encoder, normalMeshindexCount, 0, 0); */
    }
}

void FreeNormalRenderObjects()
{
    GPUFreeDescriptorSet(set);
    GPUFreeTextureView(textureView);
    GPUFreeTexture(texture);
    GPUFreeBuffer(lightUniformBuffer);
    GPUFreeBuffer(uniformBuffer);
    GPUFreeBuffer(vertexBuffer);
    GPUFreeBuffer(indexBuffer);
    GPUFreeRenderPipeline(pipeline);
    GPUFreeRenderPipeline(debugNormalPipeline);
    GPUFreeRootSignature(RS);
}

void CreateModelRenderObjects()
{
    uint32_t* vShaderCode;
    uint32_t vSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/pbr_model.vert", &vShaderCode, &vSize);
    uint32_t* fShaderCode;
    uint32_t fSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/pbr_model.frag", &fShaderCode, &fSize);
    GPUShaderLibraryDescriptor modelShaderDesc[2] = {};
    {
        modelShaderDesc[0].pName    = u8"bpr_model_vertex_shader";
        modelShaderDesc[0].code     = vShaderCode;
        modelShaderDesc[0].codeSize = vSize;
        modelShaderDesc[0].stage    = GPU_SHADER_STAGE_VERT;
        modelShaderDesc[1].pName    = u8"bpr_model_fragment_shader";
        modelShaderDesc[1].code     = fShaderCode;
        modelShaderDesc[1].codeSize = fSize;
        modelShaderDesc[1].stage    = GPU_SHADER_STAGE_FRAG;
    }
    GPUShaderLibraryID pModelVShader = GPUCreateShaderLibrary(device, &modelShaderDesc[0]);
    GPUShaderLibraryID pModelFShader = GPUCreateShaderLibrary(device, &modelShaderDesc[1]);
    free(vShaderCode);
    free(fShaderCode);
    GPUShaderEntryDescriptor modelShaderEntries[2] = { 0 };
    {
        modelShaderEntries[0].stage    = GPU_SHADER_STAGE_VERT;
        modelShaderEntries[0].entry    = u8"main";
        modelShaderEntries[0].pLibrary = pModelVShader;
        modelShaderEntries[1].stage    = GPU_SHADER_STAGE_FRAG;
        modelShaderEntries[1].entry    = u8"main";
        modelShaderEntries[1].pLibrary = pModelFShader;
    }
    //
    GPURootSignatureDescriptor modelRSDesc{};
    {
        modelRSDesc.shaders              = modelShaderEntries;
        modelRSDesc.shader_count         = 2;
        modelRSDesc.static_sampler_names = &sampler_name;
        modelRSDesc.static_sampler_count = 1;
        modelRSDesc.static_samplers      = &staticSampler;
    }
    modelRS = GPUCreateRootSignature(device, &modelRSDesc);
    //
    GPUDescriptorSetDescriptor model_set_desc{};
    model_set_desc.root_signature = modelRS;
    model_set_desc.set_index      = 0;
    modelSet                      = GPUCreateDescriptorSet(device, &model_set_desc);
    GPUDescriptorSetDescriptor set_2_desc{};
    set_2_desc.root_signature = modelRS;
    set_2_desc.set_index      = 2;
    modelSet2                 = GPUCreateDescriptorSet(device, &set_2_desc);

    // start create renderpipeline
    //vertex layout
    GPUVertexLayout vertexLayout{};
    vertexLayout.attributeCount = 3;
    vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
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
    GPUDepthStateDesc depthDesc{};
    depthDesc.depthTest  = true;
    depthDesc.depthWrite = true;
    depthDesc.depthFunc  = GPU_CMP_LEQUAL;
    GPURenderPipelineDescriptor pipelineDesc{};
    {
        pipelineDesc.pRootSignature     = modelRS;
        pipelineDesc.pVertexShader      = &modelShaderEntries[0];
        pipelineDesc.pFragmentShader    = &modelShaderEntries[1];
        pipelineDesc.pVertexLayout      = &vertexLayout;
        pipelineDesc.primitiveTopology  = GPU_PRIM_TOPO_TRI_LIST;
        pipelineDesc.pDepthState        = &depthDesc;
        pipelineDesc.pRasterizerState   = &rasterizerState;
        pipelineDesc.pColorFormats      = const_cast<EGPUFormat*>(&ppSwapchainImage[0]->desc.format);
        pipelineDesc.renderTargetCount  = 1;
        pipelineDesc.depthStencilFormat = GPU_FORMAT_D32_SFLOAT;
    }
    modelRenderPipeline = GPUCreateRenderPipeline(device, &pipelineDesc);
    GPUFreeShaderLibrary(pModelVShader);
    GPUFreeShaderLibrary(pModelFShader);
    // end create renderpipeline

    pModel = new Model("C:\\Dev\\nanosuit\\out\\nanosuit.json");
    //Model model("D:\\c++\\nanosuit\\out\\nanosuit.json");
    modelTextures.reserve(pModel->mMesh.subMeshes.size());
    {
        for (size_t i = 0; i < pModel->mMesh.subMeshes.size(); i++)
        {
            if (pModel->mMesh.subMeshes[i].diffuse_tex_url != "")
            {
                auto& res = modelTextures.emplace_back();
                res.LoadTexture("C:\\Dev\\nanosuit\\" +pModel->mMesh.subMeshes[i].diffuse_tex_url, device, graphicQueue);
                //res.LoadTexture("D:\\c++\\nanosuit\\" + model.mMesh.subMeshes[i].diffuse_tex_url, device, pGraphicQueue);
                res.SetDescriptorSet(modelRS);
            }
        }
    }
    // start upload resources
    uint32_t uploadBufferSize = pModel->GetMeshDataVerticesByteSize();
    if (uploadBufferSize < pModel->GetMeshDataIndicesByteSize())
    {
        uploadBufferSize = pModel->GetMeshDataIndicesByteSize();
    }
    GPUBufferDescriptor upload_buffer{};
    upload_buffer.size         = uploadBufferSize;
    upload_buffer.flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer.descriptors  = GPU_RESOURCE_TYPE_NONE;
    upload_buffer.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
    GPUBufferID uploadBuffer   = GPUCreateBuffer(device, &upload_buffer);
    //copy
    GPUBufferDescriptor vertex_desc{};
    vertex_desc.size         = pModel->GetMeshDataVerticesByteSize();
    vertex_desc.flags        = GPU_BCF_OWN_MEMORY_BIT;
    vertex_desc.descriptors  = GPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vertex_desc.memory_usage = GPU_MEM_USAGE_GPU_ONLY;
    modelVertexBuffer        = GPUCreateBuffer(device, &vertex_desc);
    //COPY
    memcpy(uploadBuffer->cpu_mapped_address, pModel->GetVertexBufferData().data(), vertex_desc.size);
    GPUResetCommandPool(pools[0]);
    GPUCmdBegin(cmds[0]);
    {
        GPUBufferToBufferTransfer trans_verticex_buffer_desc{};
        trans_verticex_buffer_desc.size       = vertex_desc.size;
        trans_verticex_buffer_desc.src        = uploadBuffer;
        trans_verticex_buffer_desc.src_offset = 0;
        trans_verticex_buffer_desc.dst        = modelVertexBuffer;
        trans_verticex_buffer_desc.dst_offset = 0;
        GPUCmdTransferBufferToBuffer(cmds[0], &trans_verticex_buffer_desc);
        GPUBufferBarrier vertex_barrier{};
        vertex_barrier.buffer    = modelVertexBuffer;
        vertex_barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
        vertex_barrier.dst_state = GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        GPUResourceBarrierDescriptor vertex_rs_barrer{};
        vertex_rs_barrer.buffer_barriers       = &vertex_barrier;
        vertex_rs_barrer.buffer_barriers_count = 1;
        GPUCmdResourceBarrier(cmds[0], &vertex_rs_barrer);
    }
    GPUCmdEnd(cmds[0]);
    GPUQueueSubmitDescriptor cpy_submit = { .cmds = &cmds[0], .cmds_count = 1 };
    GPUSubmitQueue(graphicQueue, &cpy_submit);
    GPUWaitQueueIdle(graphicQueue);

    //index buffer
    GPUBufferDescriptor index_desc{};
    index_desc.size         = pModel->GetMeshDataIndicesByteSize();
    index_desc.flags        = GPU_BCF_OWN_MEMORY_BIT;
    index_desc.descriptors  = GPU_RESOURCE_TYPE_INDEX_BUFFER;
    index_desc.memory_usage = GPU_MEM_USAGE_GPU_ONLY;
    modelIndexBuffer        = GPUCreateBuffer(device, &index_desc);
    //copy
    memcpy(uploadBuffer->cpu_mapped_address, pModel->GetIndexBufferData().data(), index_desc.size);
    GPUResetCommandPool(pools[0]);
    GPUCmdBegin(cmds[0]);
    {
        GPUBufferToBufferTransfer trans_index_buffer_desc{};
        trans_index_buffer_desc.size       = index_desc.size;
        trans_index_buffer_desc.src        = uploadBuffer;
        trans_index_buffer_desc.src_offset = 0;
        trans_index_buffer_desc.dst        = modelIndexBuffer;
        trans_index_buffer_desc.dst_offset = 0;
        GPUCmdTransferBufferToBuffer(cmds[0], &trans_index_buffer_desc);
        GPUBufferBarrier index_barrier{};
        index_barrier.buffer    = modelIndexBuffer;
        index_barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
        index_barrier.dst_state = GPU_RESOURCE_STATE_INDEX_BUFFER;
        GPUResourceBarrierDescriptor index_rs_barrer{};
        index_rs_barrer.buffer_barriers        = &index_barrier;
        index_rs_barrer.buffer_barriers_count = 1;
        GPUCmdResourceBarrier(cmds[0], &index_rs_barrer);
    }
    GPUCmdEnd(cmds[0]);
    GPUQueueSubmitDescriptor index_cpy_submit = { .cmds = &cmds[0], .cmds_count = 1 };
    GPUSubmitQueue(graphicQueue, &index_cpy_submit);
    GPUWaitQueueIdle(graphicQueue);
    GPUFreeBuffer(uploadBuffer);
    // end upload resources

    //uniform
    GPUBufferDescriptor uniform_buffer{};
    uniform_buffer.size             = sizeof(UniformBuffer);
    uniform_buffer.flags            = GPU_BCF_NONE;
    uniform_buffer.descriptors      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    uniform_buffer.memory_usage     = GPU_MEM_USAGE_CPU_TO_GPU;
    uniform_buffer.prefer_on_device = true;
    modelUniformBuffer              = GPUCreateBuffer(device, &uniform_buffer);
    //light
    GPUBufferDescriptor light_param_uniform_buffer{};
    light_param_uniform_buffer.size             = sizeof(LightParam);
    light_param_uniform_buffer.flags            = GPU_BCF_NONE;
    light_param_uniform_buffer.descriptors      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    light_param_uniform_buffer.memory_usage     = GPU_MEM_USAGE_CPU_TO_GPU;
    light_param_uniform_buffer.prefer_on_device = true;
    modelLightUniformBuffer                     = GPUCreateBuffer(device, &light_param_uniform_buffer);

    //update descriptorset
    GPUDescriptorData desc_data[2] = {};
    desc_data[0].name              = u8"ubo";
    desc_data[0].binding           = 0;
    desc_data[0].binding_type      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    desc_data[0].count             = 1;
    desc_data[0].buffers           = &modelUniformBuffer;
    desc_data[1].name              = u8"Lights";
    desc_data[1].binding           = 0;
    desc_data[1].binding_type      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    desc_data[1].count             = 1;
    desc_data[1].buffers           = &modelLightUniformBuffer;
    GPUUpdateDescriptorSet(modelSet, desc_data, 1);
    GPUUpdateDescriptorSet(modelSet2, desc_data + 1, 1);
    for (size_t i = 0; i < modelTextures.size(); i++)
    {
        GPUDescriptorData desc_data = {};
        desc_data.name              = u8"tex";// shader texture2D`s name
        desc_data.binding           = 0;
        desc_data.binding_type      = GPU_RESOURCE_TYPE_TEXTURE;
        desc_data.count             = 1;
        desc_data.textures          = &modelTextures[i].mTextureView;
        GPUUpdateDescriptorSet(modelTextures[i].mSet, &desc_data, 1);
    }
}

void DrawModel(GPURenderPassEncoderID encoder, const LightParam& light, const glm::vec4& viewPos, const glm::mat4& view, const glm::mat4 proj)
{
    GPUBufferRange rang{};
    rang.offset = 0;
    rang.size   = sizeof(LightParam);
    GPUMapBuffer(modelLightUniformBuffer, &rang);
    memcpy(modelLightUniformBuffer->cpu_mapped_address, &light, rang.size);
    GPUUnmapBuffer(modelLightUniformBuffer);

    UniformBuffer ubo{};
    glm::mat4 m = glm::translate(glm::mat4(1.f), { 0.f, -10.f, 0.f });
    ubo.model   = glm::rotate(m, glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));
    ubo.view    = view;
    ubo.proj    = proj;
    ubo.viewPos = viewPos;

    rang.offset = 0;
    rang.size   = sizeof(UniformBuffer);
    GPUMapBuffer(modelUniformBuffer, &rang);
    memcpy(modelUniformBuffer->cpu_mapped_address, &ubo, rang.size);
    GPUUnmapBuffer(modelUniformBuffer);

    //draw call
    {
        GPURenderEncoderBindPipeline(encoder, modelRenderPipeline);
        // bind vertexbuffer
        uint32_t stride = sizeof(Vertex);
        GPURenderEncoderBindVertexBuffers(encoder, 1, &modelVertexBuffer, &stride, nullptr);
        uint32_t indexStride = sizeof(uint32_t);
        GPURenderEncoderBindIndexBuffer(encoder, modelIndexBuffer, 0, indexStride);
        // bind descriptor ste
        GPURenderEncoderBindDescriptorSet(encoder, modelSet2); // lighting
        GPURenderEncoderBindDescriptorSet(encoder, modelSet);
        glm::vec4 pos = glm::vec4(0.f, 0.f, 0.f, 1.f);
        PushConstant push_constant{};
        push_constant.objOffsetPos = pos;
        push_constant.ao           = 1.f;
        push_constant.metallic     = 0.1f;
        push_constant.roughness    = 0.5f;
        GPURenderEncoderPushConstant(encoder, modelRS, &push_constant);
        for (uint32_t i = 0; i < pModel->mMesh.subMeshes.size(); i++)
        {
            GPURenderEncoderBindDescriptorSet(encoder, modelTextures[i].mSet);
            uint32_t indexCount = pModel->mMesh.subMeshes[i].indexCount;
            GPURenderEncoderDrawIndexedInstanced(encoder, indexCount, 1, pModel->mMesh.subMeshes[i].indexOffset, pModel->mMesh.subMeshes[i].vertexOffset, 0);
        }
    }
}

void FreeModelRendderObjects()
{
    modelTextures.clear();
    delete pModel;
    pModel = nullptr;
    GPUFreeDescriptorSet(modelSet2);
    GPUFreeDescriptorSet(modelSet);
    GPUFreeBuffer(modelUniformBuffer);
    GPUFreeBuffer(modelLightUniformBuffer);
    GPUFreeBuffer(modelIndexBuffer);
    GPUFreeBuffer(modelVertexBuffer);
    GPUFreeRenderPipeline(modelRenderPipeline);
    GPUFreeRootSignature(modelRS);
}

void NormalRenderSimple();
void RenderGraphSimple();
int main(int argc, char** argv)
{
    FakeReal::LogSystem::Initialize();
    NormalRenderSimple();
   // RenderGraphSimple();
   //Model model("C:\\Dev\\nanosuit\\out\\nanosuit.json");
    return 0;
}

void NormalRenderSimple()
{
    //create instance
    GPUInstanceDescriptor desc{
        .pChained         = nullptr,
        .backend          = EGPUBackend::GPUBackend_Vulkan,
        .enableDebugLayer = true,
        .enableValidation = true
    };
    instance = GPUCreateInstance(&desc);

    //enumerate adapters
    uint32_t adapterCount   = 0;
    GPUEnumerateAdapters(instance, NULL, &adapterCount);
    DECLARE_ZERO_VAL(GPUAdapterID, adapters, adapterCount);
    GPUEnumerateAdapters(instance, adapters, &adapterCount);
    adapter = adapters[0];

    //create
    auto window     = CreateWin32Window();
    surface = GPUCreateSurfaceFromNativeView(instance, window);

    //create device
    GPUQueueGroupDescriptor G = {
        .queueType  = EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS,
        .queueCount = 1
    };
    GPUDeviceDescriptor deviceDesc = {
        .pQueueGroup          = &G,
        .queueGroupCount      = 1,
        .disablePipelineCache = false
    };
    device = GPUCreateDevice(adapter, &deviceDesc);

    //greate graphic queue
    graphicQueue = GPUGetQueue(device, EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS, 0);

    //create present fence
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        presenFences[i] = GPUCreateFence(device);
    }

    //create swapchain
    GPUSwapchainDescriptor swapchainDesc{};
    {
        swapchainDesc.ppPresentQueues    = &graphicQueue;
        swapchainDesc.presentQueuesCount = 1;
        swapchainDesc.pSurface           = surface;
        swapchainDesc.format             = EGPUFormat::GPU_FORMAT_B8G8R8A8_UNORM;
        swapchainDesc.width              = WIDTH;
        swapchainDesc.height             = HEIGHT;
        swapchainDesc.imageCount         = FLIGHT_FRAMES;
        swapchainDesc.enableVSync        = true;
    }
    swapchain = GPUCreateSwapchain(device, &swapchainDesc);
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        GPUTextureViewDescriptor desc{};
        desc.pTexture        = swapchain->ppBackBuffers[i];
        desc.format          = (EGPUFormat)desc.pTexture->format;
        desc.usages          = EGPUTexutreViewUsage::GPU_TVU_RTV_DSV;
        desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR;
        desc.baseMipLevel    = 0;
        desc.mipLevelCount   = 1;
        desc.baseArrayLayer  = 0;
        desc.arrayLayerCount = 1;

        ppSwapchainImage[i] = GPUCreateTextureView(device, &desc);
    }

    //render resources
    staticSampler = CreateTextureSampler(device);
    GPUTextureDescriptor depth_tex_desc{};
    {
        depth_tex_desc.flags       = GPU_TCF_OWN_MEMORY_BIT;
        depth_tex_desc.width       = swapchain->ppBackBuffers[0]->width;
        depth_tex_desc.height      = swapchain->ppBackBuffers[0]->height;
        depth_tex_desc.depth       = 1;
        depth_tex_desc.array_size  = 1;
        depth_tex_desc.format      = GPU_FORMAT_D32_SFLOAT;
        depth_tex_desc.owner_queue = graphicQueue;
        depth_tex_desc.start_state = GPU_RESOURCE_STATE_DEPTH_WRITE;
        depth_tex_desc.descriptors = GPU_RESOURCE_TYPE_DEPTH_STENCIL;
    }
    depthTexture = GPUCreateTexture(device, &depth_tex_desc);
    GPUTextureViewDescriptor depth_tex_view_desc{};
    {
        depth_tex_view_desc.pTexture        = depthTexture;
        depth_tex_view_desc.format          = GPU_FORMAT_D32_SFLOAT;
        depth_tex_view_desc.usages          = EGPUTexutreViewUsage::GPU_TVU_RTV_DSV;
        depth_tex_view_desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_DEPTH;
        depth_tex_view_desc.baseMipLevel    = 0;
        depth_tex_view_desc.mipLevelCount   = 1;
        depth_tex_view_desc.baseArrayLayer  = 0;
        depth_tex_view_desc.arrayLayerCount = 1;
    }
    depthTextureView = GPUCreateTextureView(device, &depth_tex_view_desc);

    //create command objs
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        pools[i] = GPUCreateCommandPool(graphicQueue);
        GPUCommandBufferDescriptor cmdDesc{};
        cmdDesc.isSecondary = false;
        cmds[i] = GPUCreateCommandBuffer(pools[i], &cmdDesc);
    }

    //create semaphore
    presentSemaphore = GPUCreateSemaphore(device);

    ////model
    //CreateModelRenderObjects();
    ////model

    ///normal
    CreateNormalRendeObjects();
    ///normal

    ///skybox
    SkyBox skyBox;
    {
        std::array<std::string, 6> textures =
        {
            "../../../../asset/textures/sky/skybox_irradiance_X+.hdr",
            "../../../../asset/textures/sky/skybox_irradiance_X-.hdr",
            "../../../../asset/textures/sky/skybox_irradiance_Z+.hdr",
            "../../../../asset/textures/sky/skybox_irradiance_Z-.hdr",
            "../../../../asset/textures/sky/skybox_irradiance_Y+.hdr",
            "../../../../asset/textures/sky/skybox_irradiance_Y-.hdr"
        };
        skyBox.Load(textures, device, graphicQueue, GPU_FORMAT_R32G32B32A32_SFLOAT);
        skyBox.InitVertexAndIndexResource(device, graphicQueue);
        skyBox.CreateRenderPipeline(device, staticSampler, sampler_name, (EGPUFormat)swapchain->ppBackBuffers[0]->format);
    }
    ///skybox

    //light
    LightParam lightInfo    = {};
    const float p           = 15.0f;
    lightInfo.lightPos[0]   = glm::vec4(-p, p * 0.5f, 5.f, 1.f);
    lightInfo.lightPos[1]   = glm::vec4(-p, -p * 0.5f, 5.f, 1.f);
    lightInfo.lightPos[2]   = glm::vec4(p, p * 0.5f, 5.f, 1.f);
    lightInfo.lightPos[3]   = glm::vec4(p, -p * 0.5f, 5.f, 1.f);
    lightInfo.lightColor[0] = glm::vec4(1000.f, 1000.f, 1000.f, 1.f);
    lightInfo.lightColor[1] = glm::vec4(1000.f, 1000.f, 1000.f, 1.f);
    lightInfo.lightColor[2] = glm::vec4(1000.f, 1000.f, 1000.f, 1.f);
    lightInfo.lightColor[3] = glm::vec4(1000.f, 1000.f, 1000.f, 1.f);

    glm::vec4 viewPos = glm::vec4(0.f, 0.f, 6.f, 1.f);
    glm::mat4 view    = glm::lookAt(glm::vec3(viewPos.x, viewPos.y, viewPos.z), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 proj    = glm::perspective(glm::radians(90.f), (float)WIDTH / HEIGHT, 0.1f, 1000.f);

    //render loop begin
    uint32_t backbufferIndex = 0;
    bool exit                = false;
    MSG msg{};
    while (!exit)
    {
        if (PeekMessage(&msg, nullptr, 0, 0,  PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                exit = true;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {

            GPUAcquireNextDescriptor acq_desc{};
            acq_desc.signal_semaphore        = presentSemaphore;
            backbufferIndex                  = GPUAcquireNextImage(swapchain, &acq_desc);
            GPUTextureID backbuffer          = swapchain->ppBackBuffers[backbufferIndex];
            GPUTextureViewID backbuffer_view = ppSwapchainImage[backbufferIndex];

            GPUWaitFences(presenFences + backbufferIndex, 1);
            
            GPUCommandPoolID pool  = pools[backbufferIndex];
            GPUCommandBufferID cmd = cmds[backbufferIndex];

            GPUResetCommandPool(pool);
            GPUCmdBegin(cmd);
            {
                GPUTextureBarrier tex_barrier{};
                tex_barrier.texture   = backbuffer;
                tex_barrier.src_state = GPU_RESOURCE_STATE_UNDEFINED;
                tex_barrier.dst_state = GPU_RESOURCE_STATE_RENDER_TARGET;
                GPUResourceBarrierDescriptor draw_barrier{};
                draw_barrier.texture_barriers       = &tex_barrier;
                draw_barrier.texture_barriers_count = 1;
                GPUCmdResourceBarrier(cmd, &draw_barrier);

                GPUColorAttachment screenAttachment{};
                screenAttachment.view         = backbuffer_view;
                screenAttachment.load_action  = GPU_LOAD_ACTION_CLEAR;
                screenAttachment.store_action = GPU_STORE_ACTION_STORE;
                screenAttachment.clear_color  = { { 0.f, 0.f, 0.f, 0.f } };
                GPUDepthStencilAttachment ds_attachment{};
                ds_attachment.view = depthTextureView;
                ds_attachment.depth_load_action = GPU_LOAD_ACTION_CLEAR;
                ds_attachment.depth_store_action = GPU_STORE_ACTION_STORE;
                ds_attachment.clear_depth = 1.0f;
                ds_attachment.write_depth = true;
                GPURenderPassDescriptor render_pass_desc{};
                render_pass_desc.sample_count        = GPU_SAMPLE_COUNT_1;
                render_pass_desc.color_attachments   = &screenAttachment;
                render_pass_desc.render_target_count = 1;
                render_pass_desc.depth_stencil       = &ds_attachment;
                GPURenderPassEncoderID encoder       = GPUCmdBeginRenderPass(cmd, &render_pass_desc);
                {
                    GPURenderEncoderSetViewport(encoder, 0.f, (float)backbuffer->height,
                                                (float)backbuffer->width,
                                                -(float)backbuffer->height, 
                                                0.f, 1.f);
                    GPURenderEncoderSetScissor(encoder, 0, 0, backbuffer->width,
                                               backbuffer->height);

                    //skyybox
                    skyBox.Draw(encoder, view, proj);
                    //DrawModel(encoder, lightInfo, viewPos, view, proj);
                    DrawNormalObject(encoder, lightInfo, viewPos, view, proj);
                }
                GPUCmdEndRenderPass(cmd, encoder);

                GPUTextureBarrier tex_barrier1{};
                tex_barrier1.texture   = backbuffer;
                tex_barrier1.src_state = GPU_RESOURCE_STATE_RENDER_TARGET;
                tex_barrier1.dst_state = GPU_RESOURCE_STATE_PRESENT;
                GPUResourceBarrierDescriptor present_barrier{};
                present_barrier.texture_barriers_count = 1;
                present_barrier.texture_barriers       = &tex_barrier1;
                GPUCmdResourceBarrier(cmd, &present_barrier);
            }
            GPUCmdEnd(cmd);

            // submit
            GPUQueueSubmitDescriptor submitDesc{};
            submitDesc.cmds         = &cmd;
            submitDesc.cmds_count   = 1;
            submitDesc.signal_fence = presenFences[backbufferIndex];
            GPUSubmitQueue(graphicQueue, &submitDesc);
            // present
            //GPUWaitQueueIdle(pGraphicQueue);
            GPUQueuePresentDescriptor presentDesc{};
            presentDesc.swapchain            = swapchain;
            presentDesc.index                = backbufferIndex;
            presentDesc.wait_semaphores      = &presentSemaphore;
            presentDesc.wait_semaphore_count = 1;
            GPUQueuePresent(graphicQueue, &presentDesc);
        }
    }
    //render loop end

    GPUWaitQueueIdle(graphicQueue);
    GPUWaitFences(presenFences, FLIGHT_FRAMES);
    //GPUFreeFence(presenFence);
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        GPUFreeCommandBuffer(cmds[i]);
        GPUFreeCommandPool(pools[i]);
        GPUFreeFence(presenFences[i]);
    }
    GPUFreeSemaphore(presentSemaphore);
    for (uint32_t i = 0; i < swapchain->backBuffersCount; i++)
    {
        GPUFreeTextureView(ppSwapchainImage[i]);
    }
    GPUFreeSampler(staticSampler);
    
    ////////////model
    //FreeModelRendderObjects();
    ////////////model
    ///normal
    FreeNormalRenderObjects();
    ///normal
    ///skybox
    skyBox.~SkyBox();
    ///skybox

    GPUFreeTextureView(depthTextureView);
    GPUFreeTexture(depthTexture);
    GPUFreeSwapchain(swapchain);
    GPUFreeQueue(graphicQueue);
    GPUFreeDevice(device);
    GPUFreeSurface(instance, surface);
    GPUFreeInstance(instance);
    adapter = nullptr;

    DestroyWindow(window);
}

void RenderGraphSimple()
{
    //create instance
    GPUInstanceDescriptor desc{
        .pChained         = nullptr,
        .backend          = EGPUBackend::GPUBackend_Vulkan,
        .enableDebugLayer = true,
        .enableValidation = true
    };
    GPUInstanceID pInstance = GPUCreateInstance(&desc);

    //enumerate adapters
    uint32_t adapterCount   = 0;
    GPUEnumerateAdapters(pInstance, NULL, &adapterCount);
    DECLARE_ZERO_VAL(GPUAdapterID, adapters, adapterCount);
    GPUEnumerateAdapters(pInstance, adapters, &adapterCount);

    //create
    auto window     = CreateWin32Window();
    GPUSurfaceID pSurface = GPUCreateSurfaceFromNativeView(pInstance, window);

    //create device
    GPUQueueGroupDescriptor G = {
        .queueType  = EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS,
        .queueCount = 1
    };
    GPUDeviceDescriptor deviceDesc = {
        .pQueueGroup          = &G,
        .queueGroupCount      = 1,
        .disablePipelineCache = false
    };
    GPUDeviceID device = GPUCreateDevice(adapters[0], &deviceDesc);

    //greate graphic queue
    GPUQueueID pGraphicQueue = GPUGetQueue(device, EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS, 0);

    //create present fence
    GPUFenceID presenFence = GPUCreateFence(device);

    //create swapchain
    GPUSwapchainDescriptor swapchainDesc{};
    swapchainDesc.ppPresentQueues    = &pGraphicQueue;
    swapchainDesc.presentQueuesCount = 1;
    swapchainDesc.pSurface           = pSurface;
    swapchainDesc.format             = EGPUFormat::GPU_FORMAT_B8G8R8A8_UNORM;
    swapchainDesc.width              = WIDTH;
    swapchainDesc.height             = HEIGHT;
    swapchainDesc.imageCount         = 3;
    swapchainDesc.enableVSync        = true;
    GPUSwapchainID pSwapchain        = GPUCreateSwapchain(device, &swapchainDesc);

    //render resources
    GPUSamplerID texture_sampler              = CreateTextureSampler(device);
    GPUTextureID texture                      = CreateTexture(device, pGraphicQueue);
    const char8_t* sampler_name               = u8"texSamp";
    /*GPUTextureViewID textureView              = CreateTextureView(texture);
    GPUCommandPoolID copy_pool = GPUCreateCommandPool(pGraphicQueue);
    GPUCommandBufferDescriptor copy_cmd_desc{};
    copy_cmd_desc.isSecondary = false;
    GPUCommandBufferID copy_cmd = GPUCreateCommandBuffer(copy_pool, &copy_cmd_desc);
    // start upload resources
    GPUBufferDescriptor upload_buffer{};
    upload_buffer.size         = sizeof(TEXTURE_DATA);
    upload_buffer.flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer.descriptors  = GPU_RESOURCE_TYPE_NONE;
    upload_buffer.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
    GPUBufferID uploadBuffer   = GPUCreateBuffer(device, &upload_buffer);
    //copy texture
    memcpy(uploadBuffer->cpu_mapped_address, TEXTURE_DATA, sizeof(TEXTURE_DATA));
    GPUResetCommandPool(copy_pool);
    GPUCmdBegin(copy_cmd);
    {
        GPUBufferToTextureTransfer trans_texture_buffer_desc{};
        trans_texture_buffer_desc.dst                              = texture;
        trans_texture_buffer_desc.dst_subresource.mip_level        = 0;
        trans_texture_buffer_desc.dst_subresource.base_array_layer = 0;
        trans_texture_buffer_desc.dst_subresource.layer_count      = 1;
        trans_texture_buffer_desc.src                              = uploadBuffer;
        trans_texture_buffer_desc.src_offset                       = 0;
        GPUCmdTransferBufferToTexture(copy_cmd, &trans_texture_buffer_desc);
        GPUTextureBarrier barrier{};
        barrier.texture = texture;
        barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
        barrier.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
        GPUResourceBarrierDescriptor rs_barrer{};
        rs_barrer.texture_barriers      = &barrier;
        rs_barrer.texture_barriers_count = 1;
        GPUCmdResourceBarrier(copy_cmd, &rs_barrer);
    }
    GPUCmdEnd(copy_cmd);
    GPUQueueSubmitDescriptor texture_cpy_submit = { .cmds = &copy_cmd, .cmds_count = 1 };
    GPUSubmitQueue(pGraphicQueue, &texture_cpy_submit);
    GPUWaitQueueIdle(pGraphicQueue);
    GPUFreeBuffer(uploadBuffer);*/
    // end upload resources

    // start create renderpipeline
    //shader
    uint32_t* vShaderCode;
    uint32_t vSize = 0;
    ReadShaderBytes(u8"traingle_vertex_shader.vert", &vShaderCode, &vSize);
    uint32_t* fShaderCode;
    uint32_t fSize = 0;
    ReadShaderBytes(u8"traingle_fragment_shader.frag", &fShaderCode, &fSize);
    GPUShaderLibraryDescriptor vShaderDesc{};
    vShaderDesc.pName    = u8"vertex_shader";
    vShaderDesc.code = vShaderCode;
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
    GPURootSignatureDescriptor rootRSDesc = {};
    rootRSDesc.shaders                    = shaderEntries;
    rootRSDesc.shader_count               = 2;
    rootRSDesc.static_sampler_names       = &sampler_name;
    rootRSDesc.static_sampler_count       = 1;
    rootRSDesc.static_samplers            = &texture_sampler;
    GPURootSignatureID pRS                = GPUCreateRootSignature(device, &rootRSDesc);

    //vertex layout
    GPUVertexLayout vertexLayout{};
    // renderpipeline
    GPURenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.pRootSignature    = pRS;
    pipelineDesc.pVertexShader     = &shaderEntries[0];
    pipelineDesc.pFragmentShader   = &shaderEntries[1];
    pipelineDesc.pVertexLayout     = &vertexLayout;
    pipelineDesc.primitiveTopology = GPU_PRIM_TOPO_TRI_LIST;
    EGPUFormat f                   = (EGPUFormat)pSwapchain->ppBackBuffers[0]->format;
    pipelineDesc.pColorFormats     = &f;
    pipelineDesc.renderTargetCount = 1;
    GPURenderPipelineID pipeline   = GPUCreateRenderPipeline(device, &pipelineDesc);
    GPUFreeShaderLibrary(pVShader);
    GPUFreeShaderLibrary(pFShader);
    // end create renderpipeline

    /* RenderGraph* pGraph = RenderGraph::Create([=](RenderGraphBuilder& builder)
    {
        builder.WithDevice(device).WithGFXQueue(pGraphicQueue);
    });
    //render loop begin
    uint32_t backbufferIndex = 0;
    bool exit = false;
    MSG msg{};
    while (!exit)
    {
        if (PeekMessage(&msg, nullptr, 0, 0,  PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                exit = true;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            
            GPUWaitFences(&presenFence, 1);
            GPUAcquireNextDescriptor acq_desc{};
            acq_desc.fence          = presenFence;
            backbufferIndex         = GPUAcquireNextImage(pSwapchain, &acq_desc);
            GPUTextureID backbuffer = pSwapchain->ppBackBuffers[backbufferIndex];
            auto backbufferHandle = pGraph->CreateTexture([=](RenderGraph& g, TextureBuilder& builder)
            {
                builder.SetName("backbuffer")
                .Import(backbuffer, GPU_RESOURCE_STATE_UNDEFINED)
                .AllowRenderTarget();
            });

            auto uploadBufferHandle = pGraph->CreateBuffer([=](RenderGraph& g, BufferBuilder& builder)
            {
                builder.SetName("texture")
                .Size(sizeof(TEXTURE_DATA))
                .AsUploadBuffer();
            });

            auto colorSampleTexHandle = pGraph->CreateTexture([=](RenderGraph& g, TextureBuilder& builder)
            {
                builder.SetName("colorSampleTex")
                .Import(texture, GPU_RESOURCE_STATE_SHADER_RESOURCE);
            });

            

            pGraph->AddCopyPass([=](RenderGraph& g, CopyPassBuilder& builder)
            {
                builder.SetName("copy_texture")
                .CanBeLone()
                .BufferToTexture(uploadBufferHandle.BufferRange(0, 0), colorSampleTexHandle, GPU_RESOURCE_STATE_SHADER_RESOURCE);
            },
            [=](RenderGraph& graph, CopyPassContext& context)
            {
                auto uploadBuffer = context.Resolve(uploadBufferHandle);
                memcpy(uploadBuffer->cpu_mapped_address, TEXTURE_DATA, sizeof(TEXTURE_DATA));
            });

            pGraph->AddRenderPass(
                [=](RenderGraph& g, RenderPassBuilder& builder)
                {
                    builder.SetPipeline(pipeline)
                    .SetName("render pass")
                    .Read("tex", colorSampleTexHandle)
                    .Write(0, backbufferHandle, EGPULoadAction::GPU_LOAD_ACTION_CLEAR, EGPUStoreAction::GPU_STORE_ACTION_STORE);
                },
                [=](RenderGraph& g, RenderPassContext& context)
                {
                    GPURenderEncoderSetViewport(context.m_pEncoder, 0.f, 0, (float)backbuffer->width,
                                                (float)backbuffer->height, 0.f, 1.f);
                    GPURenderEncoderSetScissor(context.m_pEncoder, 0, 0, backbuffer->width,
                                               backbuffer->height);
                    GPURenderEncoderDraw(context.m_pEncoder, 3, 0);
                }
            );

            pGraph->AddPresentPass(
                [=](RenderGraph& g, PresentPassBuilder& builder)
                {
                    builder.SetName("present pass")
                    .Swapchain(pSwapchain, backbufferIndex)
                    .Texture(backbufferHandle, true);
                }
            );

            pGraph->Compile();
            uint64_t frame_idx = pGraph->Execute();
            (void)frame_idx;

            // present
            GPUWaitQueueIdle(pGraphicQueue);
            GPUQueuePresentDescriptor presentDesc{};
            presentDesc.swapchain            = pSwapchain;
            presentDesc.index                = backbufferIndex;
            GPUQueuePresent(pGraphicQueue, &presentDesc);
        }
    }
    //render loop end
    RenderGraph::Destroy(pGraph);

    GPUWaitQueueIdle(pGraphicQueue);
    GPUWaitFences(&presenFence, 1);
    GPUFreeFence(presenFence);
    GPUFreeRenderPipeline(pipeline);
    GPUFreeRootSignature(pRS);
    GPUFreeSwapchain(pSwapchain);
    GPUFreeQueue(pGraphicQueue);
    GPUFreeDevice(device);
    GPUFreeSurface(pInstance, pSurface);
    GPUFreeInstance(pInstance); */

    DestroyWindow(window);
}