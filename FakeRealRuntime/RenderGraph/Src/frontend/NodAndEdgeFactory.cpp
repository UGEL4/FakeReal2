#include "Frontend/NodeAndEdgeFactory.h"
#include "Utils/concurrent_queue.h"
#include <unordered_map>
#include "Platform/Memory.h"

namespace FakeReal
{
    namespace render_graph
    {
        struct NodeAndEdgeFactoryImp : public NodeAndEdgeFactory
        {
            NodeAndEdgeFactoryImp()
            {
            }
            ~NodeAndEdgeFactoryImp()
            {
                for (auto pool : pools)
                {
                    if (pool.second)
                    {
                        /* pool.second->~factory_pool_t();
                        free(pool.second); */
                        Delete(pool.second);
                    }
                }
            }

            struct factory_pool_t
            {
                size_t blockSize;
                moodycamel::ConcurrentQueue<void*> blocks;

                factory_pool_t(size_t blockSize, size_t blockCount)
                    : blockSize(blockSize)
                    , blocks(blockCount)
                {
                }
                ~factory_pool_t()
                {
                    void* block;
                    while (blocks.try_dequeue(block))
                        free(block);
                }
                void* Allocate()
                {
                    void* block;
                    if (blocks.try_dequeue(block))
                        return block;
                    {
                        return calloc(1, blockSize);
                    }
                }
                void Free(void* block)
                {
                    if (blocks.try_enqueue(block))
                        return;
                    free(block);
                }
            };

            bool InternalFreeMemory(void* memory, size_t size) override
            {
                auto pool = pools.find(size);
                assert(pool != pools.end());
                pool->second->Free(memory);
                return true;
            }

            void* InternalAllocateMemory(size_t size) override
            {
                factory_pool_t* p;
                auto pool = pools.find(size);
                if (pool == pools.end())
                {
                    //void* ptr = calloc(1, sizeof(factory_pool_t));
                    //p         = new (ptr) factory_pool_t(size, 2048);
                    p = New<factory_pool_t>(size, 2048);
                    pools.emplace(size, p);
                }
                else
                {
                    p = pool->second;
                }
                return p->Allocate();
            }
            std::unordered_map<size_t, factory_pool_t*> pools;
        };

        ///////////////////////
        NodeAndEdgeFactory* NodeAndEdgeFactory::Create()
        {
            //void* ptr             = calloc(1, sizeof(NodeAndEdgeFactoryImp));
            //NodeAndEdgeFactory* f = new (ptr) NodeAndEdgeFactoryImp();
            NodeAndEdgeFactory* f = New<NodeAndEdgeFactoryImp>();
            return f;
        }
        void NodeAndEdgeFactory::Destroy(NodeAndEdgeFactory* factory)
        {
            if (factory)
            {
                /* factory->~NodeAndEdgeFactory();
                free(factory); */
                Delete(factory);
            }
        }
    } // namespace render_graph
} // namespace FakeReal