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

static int WIDTH = 1080;
static int HEIGHT = 1080;
#define FLIGHT_FRAMES 3

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
}

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
    //GPUFenceID presenFence = GPUCreateFence(device);
    GPUFenceID presenFences[FLIGHT_FRAMES];
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        presenFences[i] = GPUCreateFence(device);
    }

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
    GPUTextureViewID ppSwapchainImage[3];
    for (uint32_t i = 0; i < 3; i++)
    {
        GPUTextureViewDescriptor desc{};
        desc.pTexture        = pSwapchain->ppBackBuffers[i];
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
    GPUSamplerID texture_sampler              = CreateTextureSampler(device);
    GPUTextureID texture                      = CreateTexture(device, pGraphicQueue);
    GPUTextureViewID textureView              = CreateTextureView(texture);
    const char8_t* sampler_name               = u8"texSamp";

    // start create renderpipeline
    //shader
    uint32_t* vShaderCode;
    uint32_t vSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/vertex_shader.vert", &vShaderCode, &vSize, EGPUBackend::GPUBackend_Vulkan);
    uint32_t* fShaderCode;
    uint32_t fSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/fragment_shader.frag", &fShaderCode, &fSize, EGPUBackend::GPUBackend_Vulkan);
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

    uint32_t* normal_vShaderCode;
    uint32_t normal_vSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/normal_geom.vert", &normal_vShaderCode, &normal_vSize, EGPUBackend::GPUBackend_Vulkan);
    uint32_t* normal_fShaderCode;
    uint32_t normal_fSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/normal_geom.frag", &normal_fShaderCode, &normal_fSize, EGPUBackend::GPUBackend_Vulkan);
    uint32_t* normal_gShaderCode;
    uint32_t normal_gSize = 0;
    ReadShaderBytes(u8"../../../../samples/simple_gpu/shader/normal_geom.geom", &normal_gShaderCode, &normal_gSize, EGPUBackend::GPUBackend_Vulkan);
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
    shaderEntries[0].stage                    = GPU_SHADER_STAGE_VERT;
    shaderEntries[0].entry                    = u8"main";
    shaderEntries[0].pLibrary                 = pVShader;
    shaderEntries[1].stage                    = GPU_SHADER_STAGE_FRAG;
    shaderEntries[1].entry                    = u8"main";
    shaderEntries[1].pLibrary                 = pFShader;
    shaderEntries[2].stage                    = GPU_SHADER_STAGE_VERT;
    shaderEntries[2].entry                    = u8"main";
    shaderEntries[2].pLibrary                 = pNVShader;
    shaderEntries[3].stage                    = GPU_SHADER_STAGE_FRAG;
    shaderEntries[3].entry                    = u8"main";
    shaderEntries[3].pLibrary                 = pNFShader;
    shaderEntries[4].stage                    = GPU_SHADER_STAGE_GEOM;
    shaderEntries[4].entry                    = u8"main";
    shaderEntries[4].pLibrary                 = pNGShader;

    //create root signature
    GPURootSignatureDescriptor rootRSDesc     = {};
    rootRSDesc.shaders                        = shaderEntries;
    rootRSDesc.shader_count                   = 5;
    rootRSDesc.static_sampler_names           = &sampler_name;
    rootRSDesc.static_sampler_count           = 1;
    rootRSDesc.static_samplers                = &texture_sampler;
    GPURootSignatureID pRS                    = GPUCreateRootSignature(device, &rootRSDesc);

    //create descriptorset
    GPUDescriptorSetDescriptor set_desc{};
    set_desc.root_signature = pRS;
    set_desc.set_index      = 0;
    GPUDescriptorSetID set  = GPUCreateDescriptorSet(device, &set_desc);
    //create normal descriptorset
    GPUDescriptorSetDescriptor normal_set_desc{};
    normal_set_desc.root_signature = pRS;
    normal_set_desc.set_index      = 1;
    GPUDescriptorSetID normal_set  = GPUCreateDescriptorSet(device, &normal_set_desc);

    //vertex layout
    GPUVertexLayout vertexLayout{};
    vertexLayout.attributeCount = 3;
    vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
    // renderpipeline
    GPURasterizerStateDescriptor rasterizerState = {
        .cullMode             = GPU_CULL_MODE_BACK,
        .fillMode             = GPU_FILL_MODE_WIREFRAME,
        .frontFace            = GPU_FRONT_FACE_CCW,
        .depthBias            = 0,
        .slopeScaledDepthBias = 0.f,
        .enableMultiSample    = false,
        .enableScissor        = false,
        .enableDepthClamp     = false
    };
    GPURenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.pRootSignature    = pRS;
    pipelineDesc.pVertexShader     = &shaderEntries[0];
    pipelineDesc.pFragmentShader   = &shaderEntries[1];
    pipelineDesc.pVertexLayout     = &vertexLayout;
    pipelineDesc.primitiveTopology = GPU_PRIM_TOPO_TRI_LIST;
    pipelineDesc.pRasterizerState  = &rasterizerState;
    pipelineDesc.pColorFormats     = const_cast<EGPUFormat*>(&ppSwapchainImage[0]->desc.format);
    pipelineDesc.renderTargetCount = 1;
    GPURenderPipelineID pipeline   = GPUCreateRenderPipeline(device, &pipelineDesc);
    GPUFreeShaderLibrary(pVShader);
    GPUFreeShaderLibrary(pFShader);

    //normal pipeline
    GPURenderPipelineDescriptor normal_pipelineDesc = pipelineDesc;
    normal_pipelineDesc.pVertexShader     = &shaderEntries[2];
    normal_pipelineDesc.pFragmentShader   = &shaderEntries[3];
    normal_pipelineDesc.pGeometryShader   = &shaderEntries[4];
    GPURenderPipelineID normal_pipeline   = GPUCreateRenderPipeline(device, &normal_pipelineDesc);
    GPUFreeShaderLibrary(pNVShader);
    GPUFreeShaderLibrary(pNFShader);
    GPUFreeShaderLibrary(pNGShader);
    // end create renderpipeline

    //create command objs
    GPUCommandPoolID pools[FLIGHT_FRAMES];
    GPUCommandBufferID cmds[FLIGHT_FRAMES];
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        pools[i] = GPUCreateCommandPool(pGraphicQueue);
        GPUCommandBufferDescriptor cmdDesc{};
        cmdDesc.isSecondary = false;
        cmds[i] = GPUCreateCommandBuffer(pools[i], &cmdDesc);
    }

    //create semaphore
    GPUSemaphoreID presentSemaphore = GPUCreateSemaphore(device);

    // start upload resources
    GPUBufferDescriptor upload_buffer{};
    upload_buffer.size         = sizeof(TEXTURE_DATA);
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
        barrier.texture = texture;
        barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
        barrier.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
        GPUResourceBarrierDescriptor rs_barrer{};
        rs_barrer.texture_barriers      = &barrier;
        rs_barrer.texture_barriers_count = 1;
        GPUCmdResourceBarrier(cmds[0], &rs_barrer);
    }
    GPUCmdEnd(cmds[0]);
    GPUQueueSubmitDescriptor texture_cpy_submit = { .cmds = &cmds[0], .cmds_count = 1 };
    GPUSubmitQueue(pGraphicQueue, &texture_cpy_submit);
    GPUWaitQueueIdle(pGraphicQueue);

    //vertex buffer
    std::vector<Vertex> vertices = Sphere::GenCubeVertices();
    //std::vector<Vertex> vertices = Sphere::GenSphereVertices();
    GPUBufferDescriptor vertex_desc{};
    vertex_desc.size         = sizeof(Vertex) * vertices.size();
    vertex_desc.flags        = GPU_BCF_OWN_MEMORY_BIT;
    vertex_desc.descriptors  = GPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vertex_desc.memory_usage = GPU_MEM_USAGE_GPU_ONLY;
    GPUBufferID vertexBuffer = GPUCreateBuffer(device, &vertex_desc);
    //COPY
    memcpy(uploadBuffer->cpu_mapped_address, &vertices[0], sizeof(Vertex) * vertices.size());
    GPUResetCommandPool(pools[0]);
    GPUCmdBegin(cmds[0]);
    {
        GPUBufferToBufferTransfer trans_verticex_buffer_desc{};
        trans_verticex_buffer_desc.size       = sizeof(Vertex) * vertices.size();
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
    GPUSubmitQueue(pGraphicQueue, &cpy_submit);
    GPUWaitQueueIdle(pGraphicQueue);

    //index buffer
    std::vector<uint32_t> indices = Sphere::GenCubeIndices();
    //std::vector<uint32_t> indices = Sphere::GenSphereIndices();
    GPUBufferDescriptor index_desc{};
    index_desc.size         = sizeof(uint32_t) * indices.size();
    index_desc.flags        = GPU_BCF_OWN_MEMORY_BIT;
    index_desc.descriptors  = GPU_RESOURCE_TYPE_INDEX_BUFFER;
    index_desc.memory_usage = GPU_MEM_USAGE_GPU_ONLY;
    GPUBufferID indexBuffer = GPUCreateBuffer(device, &index_desc);
    //copy
    memcpy(uploadBuffer->cpu_mapped_address, indices.data(), sizeof(uint32_t) * indices.size());
    GPUResetCommandPool(pools[0]);
    GPUCmdBegin(cmds[0]);
    {
        GPUBufferToBufferTransfer trans_index_buffer_desc{};
        trans_index_buffer_desc.size       = sizeof(uint32_t) * indices.size();
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
        index_rs_barrer.buffer_barriers        = &index_barrier;
        index_rs_barrer.buffer_barriers_count = 1;
        GPUCmdResourceBarrier(cmds[0], &index_rs_barrer);
    }
    GPUCmdEnd(cmds[0]);
    GPUQueueSubmitDescriptor index_cpy_submit = { .cmds = &cmds[0], .cmds_count = 1 };
    GPUSubmitQueue(pGraphicQueue, &index_cpy_submit);
    GPUWaitQueueIdle(pGraphicQueue);
    GPUFreeBuffer(uploadBuffer);
    // end upload resources

    struct UniformBuffer
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
        glm::vec4 color;
        glm::vec4 viewPos;
        glm::vec4 lightPos;
    };
    //uniform
    GPUBufferDescriptor uniform_buffer{};
    uniform_buffer.size         = sizeof(UniformBuffer);
    uniform_buffer.flags        = GPU_BCF_NONE;
    uniform_buffer.descriptors  = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    uniform_buffer.memory_usage = GPU_MEM_USAGE_CPU_TO_GPU;
    GPUBufferID uniformBuffer   = GPUCreateBuffer(device, &uniform_buffer);

    // update descriptorset
    GPUDescriptorData desc_data[2] = {};
    desc_data[0].name              = u8"tex"; // shader sampler2D`s name
    desc_data[0].binding           = 0;
    desc_data[0].binding_type      = GPU_RESOURCE_TYPE_TEXTURE;
    desc_data[0].count             = 1;
    desc_data[0].textures          = &textureView;
    /*  desc_data[1].name              = u8"texSamp";
     desc_data[1].samplers      = &texture_sampler;
     desc_data[1].count             = 1; */
    desc_data[1].name         = u8"ubo";
    desc_data[1].binding      = 1;
    desc_data[1].binding_type = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    desc_data[1].count        = 1;
    desc_data[1].buffers      = &uniformBuffer;
    GPUUpdateDescriptorSet(set, desc_data, 2);
    //GPUUpdateDescriptorSet(set_1, desc_data + 1, 1);

    static glm::mat4 m = glm::translate(glm::mat4(1.f), { 0.f, 0.f, 0.f });
    UniformBuffer ubo{};
    ubo.viewPos  = glm::vec4(1.f, 0.f, 5.f, 1.f);
    ubo.model    = glm::rotate(m, glm::radians(0.f), glm::vec3(0.f, 1.f, 0.f));
    ubo.view     = glm::lookAt(glm::vec3(ubo.viewPos.x, ubo.viewPos.y, ubo.viewPos.z), glm::vec3(0.f, 0.f, 0.5f), glm::vec3(0.f, 1.f, 0.f));
    ubo.proj     = glm::perspective(glm::radians(90.f), (float)WIDTH / HEIGHT, 0.1f, 1000.f);
    ubo.color    = glm::vec4(0.f, 1.f, 1.f, 1.f);
    ubo.lightPos = glm::vec4(0.f, 0.f, 8.f, 1.f);
    //ubo.proj[1][1] *= -1;

    struct NormalUniformBuffer
    {
        glm::mat4 model;
        glm::mat4 view;
        //glm::mat4 proj;
    };
    GPUBufferDescriptor normal_uniform_buffer{};
    normal_uniform_buffer.size         = sizeof(UniformBuffer);
    normal_uniform_buffer.flags        = GPU_BCF_NONE;
    normal_uniform_buffer.descriptors  = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    normal_uniform_buffer.memory_usage = GPU_MEM_USAGE_CPU_TO_GPU;
    GPUBufferID normal_uniformBuffer   = GPUCreateBuffer(device, &normal_uniform_buffer);

    GPUBufferDescriptor normal_geom_uniform_buffer{};
    normal_geom_uniform_buffer.size         = sizeof(glm::mat4);
    normal_geom_uniform_buffer.flags        = GPU_BCF_NONE;
    normal_geom_uniform_buffer.descriptors  = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    normal_geom_uniform_buffer.memory_usage = GPU_MEM_USAGE_CPU_TO_GPU;
    GPUBufferID normal_geom_uniformBuffer   = GPUCreateBuffer(device, &normal_geom_uniform_buffer);
    // update descriptorset
    GPUDescriptorData normal_desc_data[2] = {};
    normal_desc_data[0].name              = u8"nv_ubo";
    normal_desc_data[0].binding           = 0;
    normal_desc_data[0].binding_type      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    normal_desc_data[0].count             = 1;
    normal_desc_data[0].buffers           = &normal_uniformBuffer;
    normal_desc_data[1].name              = u8"ng_ubo";
    normal_desc_data[1].binding           = 1;
    normal_desc_data[1].binding_type      = GPU_RESOURCE_TYPE_UNIFORM_BUFFER;
    normal_desc_data[1].count             = 1;
    normal_desc_data[1].buffers           = &normal_geom_uniformBuffer;
    GPUUpdateDescriptorSet(normal_set, normal_desc_data, 2);
    NormalUniformBuffer normal_ubo{};
    normal_ubo.model = ubo.model;
    normal_ubo.view  = ubo.view;
    glm::mat4 geom_projection = ubo.proj;

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
            GPUBufferRange rang{};
            rang.offset = 0;
            rang.size   = sizeof(UniformBuffer);
            GPUMapBuffer(uniformBuffer, &rang);
            memcpy(uniformBuffer->cpu_mapped_address, &ubo, sizeof(UniformBuffer));
            GPUUnmapBuffer(uniformBuffer);

            GPUBufferRange normal_rang{};
            normal_rang.offset = 0;
            normal_rang.size   = sizeof(NormalUniformBuffer);
            GPUMapBuffer(normal_uniformBuffer, &normal_rang);
            memcpy(normal_uniformBuffer->cpu_mapped_address, &normal_ubo, sizeof(NormalUniformBuffer));
            GPUUnmapBuffer(normal_uniformBuffer);

            normal_rang.offset = 0;
            normal_rang.size   = sizeof(glm::mat4);
            GPUMapBuffer(normal_geom_uniformBuffer, &normal_rang);
            memcpy(normal_geom_uniformBuffer->cpu_mapped_address, &geom_projection[0][0], sizeof(glm::mat4));
            GPUUnmapBuffer(normal_geom_uniformBuffer);

            GPUAcquireNextDescriptor acq_desc{};
            acq_desc.signal_semaphore        = presentSemaphore;
            backbufferIndex                  = GPUAcquireNextImage(pSwapchain, &acq_desc);
            GPUTextureID backbuffer          = pSwapchain->ppBackBuffers[backbufferIndex];
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
                GPURenderPassDescriptor render_pass_desc{};
                render_pass_desc.sample_count        = GPU_SAMPLE_COUNT_1;
                render_pass_desc.color_attachments   = &screenAttachment;
                render_pass_desc.render_target_count = 1;
                GPURenderPassEncoderID encoder       = GPUCmdBeginRenderPass(cmd, &render_pass_desc);
                {
                    GPURenderEncoderSetViewport(encoder, 0.f, (float)backbuffer->height,
                                                (float)backbuffer->width,
                                                -(float)backbuffer->height, 
                                                0.f, 1.f);
                    GPURenderEncoderSetScissor(encoder, 0, 0, backbuffer->width,
                                               backbuffer->height);
                    GPURenderEncoderBindPipeline(encoder, pipeline);
                    //bind vertexbuffer
                    uint32_t stride = sizeof(Vertex);
                    GPURenderEncoderBindVertexBuffers(encoder, 1, &vertexBuffer, &stride, nullptr);
                    uint32_t indexStride = sizeof(decltype(indices)::value_type);
                    GPURenderEncoderBindIndexBuffer(encoder, indexBuffer, 0, indexStride);
                    //bind descriptor ste
                    GPURenderEncoderBindDescriptorSet(encoder, set);
                    //GPURenderEncoderDraw(encoder, 3, 0);
                    //GPURenderEncoderDrawIndexed(encoder, sizeof(indices) / sizeof(uint16_t), 0, 0);
                    uint32_t indexCount = indices.size();
                    GPURenderEncoderDrawIndexedInstanced(encoder, indexCount, 1, 0, 0, 0);

                    GPURenderEncoderBindPipeline(encoder, normal_pipeline);
                    GPURenderEncoderBindDescriptorSet(encoder, normal_set);
                    GPURenderEncoderDrawIndexedInstanced(encoder, indexCount, 1, 0, 0, 0);
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
            submitDesc.cmds       = &cmd;
            submitDesc.cmds_count = 1;
            submitDesc.signal_fence = presenFences[backbufferIndex];
            GPUSubmitQueue(pGraphicQueue, &submitDesc);
            // present
            //GPUWaitQueueIdle(pGraphicQueue);
            GPUQueuePresentDescriptor presentDesc{};
            presentDesc.swapchain            = pSwapchain;
            presentDesc.index                = backbufferIndex;
            presentDesc.wait_semaphores      = &presentSemaphore;
            presentDesc.wait_semaphore_count = 1;
            GPUQueuePresent(pGraphicQueue, &presentDesc);
        }
    }
    //render loop end

    GPUWaitQueueIdle(pGraphicQueue);
    GPUWaitFences(presenFences, FLIGHT_FRAMES);
    //GPUFreeFence(presenFence);
    for (uint32_t i = 0; i < FLIGHT_FRAMES; i++)
    {
        GPUFreeCommandBuffer(cmds[i]);
        GPUFreeCommandPool(pools[i]);
        GPUFreeFence(presenFences[i]);
    }
    GPUFreeSemaphore(presentSemaphore);
    for (uint32_t i = 0; i < pSwapchain->backBuffersCount; i++)
    {
        GPUFreeTextureView(ppSwapchainImage[i]);
    }
    GPUFreeSampler(texture_sampler);
    GPUFreeDescriptorSet(set);
    //GPUFreeDescriptorSet(set_1);
    GPUFreeTextureView(textureView);
    GPUFreeTexture(texture);
    GPUFreeBuffer(uniformBuffer);
    GPUFreeBuffer(vertexBuffer);
    GPUFreeBuffer(indexBuffer);
   /*  GPUFreeCommandBuffer(cmd);
    GPUFreeCommandPool(pool); */
    GPUFreeRenderPipeline(pipeline);
    GPUFreeRootSignature(pRS);
    GPUFreeSwapchain(pSwapchain);
    GPUFreeQueue(pGraphicQueue);
    GPUFreeDevice(device);
    GPUFreeSurface(pInstance, pSurface);
    GPUFreeInstance(pInstance);

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
    ReadShaderBytes(u8"traingle_vertex_shader.vert", &vShaderCode, &vSize, EGPUBackend::GPUBackend_Vulkan);
    uint32_t* fShaderCode;
    uint32_t fSize = 0;
    ReadShaderBytes(u8"traingle_fragment_shader.frag", &fShaderCode, &fSize, EGPUBackend::GPUBackend_Vulkan);
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