#pragma once

#include <unordered_map>
#include <string>
#include "Gpu/GpuBindTable.h"

namespace FakeReal
{
    namespace render_graph
    {
        struct BindTablePool
        {
            BindTablePool(GPURootSignatureID rs)
                : m_pRootSignature(rs)
            {
            }
            GPUBindTableID Pop(const std::string& name, const char** ppNames, uint32_t namesCount);
            void Expand(const std::string& name, const char** ppNames, uint32_t namesCount, uint32_t exCount = 1);
            void Reset();
            void Destroy();

            struct TablesBlock
            {
                uint64_t mCursor = 0;
                std::vector<GPUBindTableID> mTables;
            };

            GPURootSignatureID m_pRootSignature;
            std::unordered_map<std::string, TablesBlock> mPool;
        };
    } // namespace render_graph
} // namespace FakeReal