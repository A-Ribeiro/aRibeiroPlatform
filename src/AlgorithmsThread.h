#ifndef __algorithms__Thread__h__
#define __algorithms__Thread__h__

#include <aRibeiroCore/common.h>
#include <aRibeiroPlatform/ObjectBuffer.h>
#include <aRibeiroPlatform/ObjectQueue.h>
#include <aRibeiroPlatform/PlatformThread.h>
#include <aRibeiroPlatform/PlatformSemaphore.h>

#include <aRibeiroCore/Algorithms.h>
#include <aRibeiroPlatform/ThreadPool.h>

namespace aRibeiro {
    namespace Sorting {

        enum DynamicSortGather {
            DynamicSortGather_none,
            DynamicSortGather_bucket,
            DynamicSortGather_counting,
            DynamicSortGather_merge
        };

        enum DynamicSortAlgorithm {
            DynamicSortAlgorithm_none,
            DynamicSortAlgorithm_std,
            DynamicSortAlgorithm_radix_counting
        };

        enum DynamicSortJob_type {
            DynamicSortJob_SortAndCopy,
            DynamicSortJob_OnlySort,
            DynamicSortJob_Merge
        };

        enum DynamicSortData_type {
            DynamicSortData_Int32,
            DynamicSortData_UInt32,
            DynamicSortData_IndexInt32,
            DynamicSortData_IndexUInt32
        };

        struct DynamicSortJob {

            DynamicSortJob_type type;
            DynamicSortAlgorithm algorithm;

            DynamicSortData_type input_data_type;

            union {
                struct {
                    struct {
                        union {
                            int32_t* _array_int32;
                            uint32_t* _array_uint32;
                            IndexInt32* _array_IndexInt32;
                            IndexUInt32* _array_IndexUInt32;
                        };
                        uint32_t _size;
                        union {
                            int32_t* _tmp_array_int32;
                            uint32_t* _tmp_array_uint32;
                            IndexInt32* _tmp_array_IndexInt32;
                            IndexUInt32* _tmp_array_IndexUInt32;
                        };
                    } sort;

                    struct {
                        void* _Dst;
                        const void* _Src;
                        size_t _Size;
                    } copy;
                } sort_copy;

                struct {
                    union {
                        int32_t* in_int32;
                        uint32_t* in_uint32;
                        IndexInt32* in_IndexInt32;
                        IndexUInt32* in_IndexUInt32;
                    };
                    union {
                        int32_t* out_int32;
                        uint32_t* out_uint32;
                        IndexInt32* out_IndexInt32;
                        IndexUInt32* out_IndexUInt32;
                    };
                    int i;
                    int element_count;
                    int size;
                } merge;
            };

        };

        class DynamicSort {
            //std::vector<PlatformThread*> threads;
            ThreadPool* threadPool;
            ObjectQueue <DynamicSortJob> queue;
            PlatformSemaphore semaphore;
            ObjectBuffer auxBuffer;
            PlatformMutex mutex;
            uint32_t useMultithreadStartingAtCount;
            
            void merge_job_int32(const int32_t* in, int32_t* out, int i, int element_count, int size);
            void merge_job_uint32(const uint32_t* in, uint32_t* out, int i, int element_count, int size);

            void merge_job_IndexInt32(const IndexInt32* in, IndexInt32* out, int i, int element_count, int size);
            void merge_job_IndexUInt32(const IndexUInt32* in, IndexUInt32* out, int i, int element_count, int size);

            void task_run();

            void bucket_int32_t(int32_t* A, uint32_t size, DynamicSortAlgorithm algorithm);
            void counting_int32_t(int32_t* A, uint32_t size, DynamicSortAlgorithm algorithm);
            void merge_int32_t(int32_t* A, uint32_t size, DynamicSortAlgorithm algorithm);
            
            void bucket_uint32_t(uint32_t* A, uint32_t size, DynamicSortAlgorithm algorithm);
            void counting_uint32_t(uint32_t* A, uint32_t size, DynamicSortAlgorithm algorithm);
            void merge_uint32_t(uint32_t* A, uint32_t size, DynamicSortAlgorithm algorithm);


            void bucket_IndexInt32(IndexInt32* A, uint32_t size, DynamicSortAlgorithm algorithm);
            void counting_IndexInt32(IndexInt32* A, uint32_t size, DynamicSortAlgorithm algorithm);
            void merge_IndexInt32(IndexInt32* A, uint32_t size, DynamicSortAlgorithm algorithm);

            void bucket_IndexUInt32(IndexUInt32* A, uint32_t size, DynamicSortAlgorithm algorithm);
            void counting_IndexUInt32(IndexUInt32* A, uint32_t size, DynamicSortAlgorithm algorithm);
            void merge_IndexUInt32(IndexUInt32* A, uint32_t size, DynamicSortAlgorithm algorithm);

        public:
            
            DynamicSort(ThreadPool* _threadPool, uint32_t useMultithreadStartingAtCount = 64*1024);//64k
            ~DynamicSort();

            void sort_int32_t(int32_t* A, uint32_t size, DynamicSortGather gather = DynamicSortGather_counting, DynamicSortAlgorithm algorithm = DynamicSortAlgorithm_radix_counting);
            void sort_uint32_t(uint32_t* A, uint32_t size, DynamicSortGather gather = DynamicSortGather_counting, DynamicSortAlgorithm algorithm = DynamicSortAlgorithm_radix_counting);

            void sort_IndexInt32(IndexInt32* A, uint32_t size, DynamicSortGather gather = DynamicSortGather_counting, DynamicSortAlgorithm algorithm = DynamicSortAlgorithm_radix_counting);
            void sort_IndexUInt32(IndexUInt32* A, uint32_t size, DynamicSortGather gather = DynamicSortGather_counting, DynamicSortAlgorithm algorithm = DynamicSortAlgorithm_radix_counting);


        };
    }
}

#endif