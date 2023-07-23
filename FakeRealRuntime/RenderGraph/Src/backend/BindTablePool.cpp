#include "Backend/BindTablePool.h"

namespace FakeReal
{
    namespace render_graph
    {
        GPUBindTableID BindTablePool::Pop(const std::string& name, const char** ppNames, uint32_t namesCount)
        {
            auto iter = mPool.find(name);
            if (iter == mPool.end())
            {
                mPool.insert(std::make_pair(name, TablesBlock{}));
            }

            auto& B = mPool[name];
            if (B.mCursor >= B.mTables.size())
            {
                // expand
                Expand(name, ppNames, namesCount);
            }
            return B.mTables[B.mCursor++];
        }

        void BindTablePool::Expand(const std::string& name, const char** ppNames, uint32_t namesCount, uint32_t exCount)
        {
            auto iter = mPool.find(name);
            if (iter == mPool.end())
            {
                mPool.insert(std::make_pair(name, TablesBlock{}));
            }

            auto& B = mPool[name];
            B.mTables.reserve(B.mTables.size() + exCount);
            for (uint32_t i = 0; i < exCount; i++)
            {
                GPUBindTableDescriptor desc;
                desc.pRootSignature     = m_pRootSignature;
                desc.ppNames            = ppNames;
                desc.namesCount         = namesCount;
                GPUBindTableID newTable = GPUCreateBindTable(m_pRootSignature->device, &desc);
                B.mTables.emplace_back(newTable);
            }
        }

        void BindTablePool::Reset()
        {
            for (auto& [name, block] : mPool)
            {
                block.mCursor = 0;
            }
        }

        void BindTablePool::Destroy()
        {
            for (auto& [name, block] : mPool)
            {
                block.mCursor = 0;
                for (auto& table : block.mTables)
                {
                    if (table) GPUFreeBindTable(table);
                }
            }
        }
    } // namespace render_graph
} // namespace FakeReal