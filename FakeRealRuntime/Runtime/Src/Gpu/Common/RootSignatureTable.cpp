#include "Gpu/GpuApi.h"
#include "CommonUtils.h"
#include <set>

void InitRSParamTables(GPURootSignature* RS, const struct GPURootSignatureDescriptor* desc)
{
    GPUShaderReflection* entry_reflections[32] = { 0 };
    // Pick shader reflection data
    for (uint32_t i = 0; i < desc->shader_count; i++)
    {
        const GPUShaderEntryDescriptor* shader_desc = &desc->shaders[i];
        // Find shader reflection
        for (uint32_t j = 0; j < shader_desc->pLibrary->entrys_count; j++)
        {
            GPUShaderReflection* temp_entry_reflcetion = &shader_desc->pLibrary->entry_reflections[j];
            if (temp_entry_reflcetion->entry_name)
            {
                if (strcmp((const char*)shader_desc->entry, (const char*)temp_entry_reflcetion->entry_name) == 0)
                {
                    entry_reflections[i] = temp_entry_reflcetion;
                    break;
                }
            }
        }
        if (entry_reflections[i] == NULL)
        {
            entry_reflections[i] = &shader_desc->pLibrary->entry_reflections[0];
        }
    }
    // Collect all resources
    RS->pipeline_type = GPU_PIPELINE_TYPE_NONE;
    std::vector<GPUShaderResource> all_resources;
    std::vector<GPUShaderResource> all_push_constants;
    std::vector<GPUShaderResource> all_static_samplers;
    for (uint32_t i = 0; i < desc->shader_count; i++)
    {
        GPUShaderReflection* reflection = entry_reflections[i];
        for (uint32_t j = 0; j < reflection->shader_resources_count; j++)
        {
            GPUShaderResource& resource = reflection->shader_resources[j];
            if (ShaderResourceIsRootConst(&resource, desc))
            {
                bool coincided = false;
                for (auto&& root_const : all_push_constants)
                {
                    if (root_const.name_hash == resource.name_hash &&
                        root_const.set == resource.set &&
                        root_const.binding == resource.binding &&
                        root_const.size == resource.size)
                    {
                        root_const.stages |= resource.stages;
                        coincided = true;
                    }
                }
                if (!coincided)
                    all_push_constants.emplace_back(resource);
            }
            else if (ShaderResourceIsStaticSampler(&resource, desc))
            {
                bool coincided = false;
                for (auto&& static_sampler : all_static_samplers)
                {
                    if (static_sampler.name_hash == resource.name_hash &&
                    //if (strcmp((const char*)static_sampler.name, (const char*)resource.name) == 0 &&
                        static_sampler.set == resource.set &&
                        static_sampler.binding == resource.binding)
                    {
                        static_sampler.stages |= resource.stages;
                        coincided = true;
                    }
                }
                if (!coincided)
                    all_static_samplers.emplace_back(resource);
            }
            else
            {
                all_resources.emplace_back(resource);
            }
        }
        // Pipeline Type
        if (reflection->stage & GPU_SHADER_STAGE_COMPUTE)
            RS->pipeline_type = GPU_PIPELINE_TYPE_COMPUTE;
        else if (reflection->stage & GPU_SHADER_STAGE_RAYTRACING)
            RS->pipeline_type = GPU_PIPELINE_TYPE_RAYTRACING;
        else
            RS->pipeline_type = GPU_PIPELINE_TYPE_GRAPHICS;
    }
    // Merge
    std::set<uint32_t> valid_sets;
    std::vector<GPUShaderResource> RST_resources;
    RST_resources.reserve(all_resources.size());
    for (auto&& shader_resource : all_resources)
    {
        bool coincided = false;
        for (auto&& RST_resource : RST_resources)
        {
            if (RST_resource.set == shader_resource.set &&
                RST_resource.binding == shader_resource.binding &&
                RST_resource.type == shader_resource.type)
            {
                RST_resource.stages |= shader_resource.stages;
                coincided = true;
            }
        }
        if (!coincided)
        {
            valid_sets.insert(shader_resource.set);
            RST_resources.emplace_back(shader_resource);
        }
    }
    std::stable_sort(RST_resources.begin(), RST_resources.end(),
                       [](const GPUShaderResource& lhs, const GPUShaderResource& rhs) {
                           if (lhs.set != rhs.set)
                               return lhs.set < rhs.set;
                           else
                               return lhs.binding < rhs.binding;
                       });
    // Slice
    RS->table_count      = (uint32_t)valid_sets.size();
    RS->tables           = (GPUParameterTable*)malloc(RS->table_count * sizeof(GPUParameterTable));
    if (RS->table_count == 0) RS->tables = nullptr;
    uint32_t table_index = 0;
    for (auto set_index : valid_sets)
    {
        GPUParameterTable& table = RS->tables[table_index];
        table.set_index           = set_index;
        table.resources_count     = 0;
        for (auto&& RST_resource : RST_resources)
        {
            if (RST_resource.set == set_index)
                table.resources_count++;
        }
        table.resources = (GPUShaderResource*)malloc(
        table.resources_count * sizeof(GPUShaderResource));
        uint32_t slot_index = 0;
        for (auto&& RST_resource : RST_resources)
        {
            if (RST_resource.set == set_index)
            {
                table.resources[slot_index] = RST_resource;
                slot_index++;
            }
        }
        table_index++;
    }
    // push constants
    RS->push_constant_count = (uint32_t)all_push_constants.size();
    RS->push_constants      = (GPUShaderResource*)malloc(
    RS->push_constant_count * sizeof(GPUShaderResource));
    for (uint32_t i = 0; i < all_push_constants.size(); i++)
    {
        RS->push_constants[i] = all_push_constants[i];
    }
    // static samplers
    std::stable_sort(all_static_samplers.begin(), all_static_samplers.end(),
                       [](const GPUShaderResource& lhs, const GPUShaderResource& rhs) {
                           if (lhs.set != rhs.set)
                               return lhs.set < rhs.set;
                           else
                               return lhs.binding < rhs.binding;
                       });
    RS->static_sampler_count = (uint32_t)all_static_samplers.size();
    RS->static_samplers      = (GPUShaderResource*)malloc(
    RS->static_sampler_count * sizeof(GPUShaderResource));
    for (uint32_t i = 0; i < all_static_samplers.size(); i++)
    {
        RS->static_samplers[i] = all_static_samplers[i];
    }
    // copy names
    for (uint32_t i = 0; i < RS->push_constant_count; i++)
    {
        GPUShaderResource* dst = RS->push_constants + i;
        dst->name               = DuplicateString(dst->name);
    }
    for (uint32_t i = 0; i < RS->static_sampler_count; i++)
    {
        GPUShaderResource* dst = RS->static_samplers + i;
        dst->name               = DuplicateString(dst->name);
    }
    for (uint32_t i = 0; i < RS->table_count; i++)
    {
        GPUParameterTable* set_to_record = &RS->tables[i];
        for (uint32_t j = 0; j < set_to_record->resources_count; j++)
        {
            GPUShaderResource* dst = &set_to_record->resources[j];
            dst->name               = DuplicateString(dst->name);
        }
    }
}

void FreeRSParamTables(GPURootSignature* RS)
{
    if (RS->tables != NULL)
    {
        for (uint32_t i_set = 0; i_set < RS->table_count; i_set++)
        {
            GPUParameterTable* param_table = &RS->tables[i_set];
            if (param_table->resources != NULL)
            {
                for (uint32_t i_binding = 0; i_binding < param_table->resources_count; i_binding++)
                {
                    GPUShaderResource* binding_to_free = &param_table->resources[i_binding];
                    if (binding_to_free->name != NULL)
                    {
                        free((char8_t*)binding_to_free->name);
                    }
                }
                free(param_table->resources);
            }
        }
        free(RS->tables);
    }
    if (RS->push_constants != NULL)
    {
        for (uint32_t i = 0; i < RS->push_constant_count; i++)
        {
            GPUShaderResource* binding_to_free = RS->push_constants + i;
            if (binding_to_free->name != NULL)
            {
                free((char8_t*)binding_to_free->name);
            }
        }
        free(RS->push_constants);
    }
    if (RS->static_samplers != NULL)
    {
        for (uint32_t i = 0; i < RS->static_sampler_count; i++)
        {
            GPUShaderResource* binding_to_free = RS->static_samplers + i;
            if (binding_to_free->name != NULL)
            {
                free((char8_t*)binding_to_free->name);
            }
        }
        free(RS->static_samplers);
    }
}

bool ShaderResourceIsRootConst(const struct GPUShaderResource* resource, const struct GPURootSignatureDescriptor* desc)
{
    if (resource->type == GPU_RESOURCE_TYPE_PUSH_CONSTANT) return true;
    for (uint32_t i = 0; i < desc->push_constant_count; i++)
    {
        if (strcmp((const char*)resource->name, (const char*)desc->push_constant_names[i]) == 0)
            return true;
    }
    return false;
}

bool ShaderResourceIsStaticSampler(const struct GPUShaderResource* resource, const struct GPURootSignatureDescriptor* desc)
{
    for (uint32_t i = 0; i < desc->static_sampler_count; i++)
    {
        if (strcmp((const char*)resource->name, (const char*)desc->static_sampler_names[i]) == 0)
        {
            return resource->type == GPU_RESOURCE_TYPE_SAMPLER;
        }
    }
    return false;
}

