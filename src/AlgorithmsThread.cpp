#include "AlgorithmsThread.h"

#include <aRibeiroCore/Algorithms.h>

#include <algorithm>

namespace aRibeiro {
    namespace Sorting {

        static DynamicSortJob CreateSortAndCopyInt32(
            DynamicSortAlgorithm algorithm,
            int32_t* _array_int32, uint32_t _size, int32_t* _tmp_array_int32,
            void* _Dst, const void* _Src, size_t _Size) {

            DynamicSortJob result;

            result.type = DynamicSortJob_SortAndCopy;
            result.input_data_type = DynamicSortData_Int32;
            result.algorithm = algorithm;

            result.sort_copy.sort._array_int32 = _array_int32;
            result.sort_copy.sort._size = _size;
            result.sort_copy.sort._tmp_array_int32 = _tmp_array_int32;

            result.sort_copy.copy._Dst = _Dst;
            result.sort_copy.copy._Src = _Src;
            result.sort_copy.copy._Size = _Size;

            return result;
        }

        static DynamicSortJob CreateSortAndCopyUInt32(
            DynamicSortAlgorithm algorithm,
            uint32_t* _array_uint32, uint32_t _size, uint32_t* _tmp_array_uint32,
            void* _Dst, const void* _Src, size_t _Size) {

            DynamicSortJob result;

            result.type = DynamicSortJob_SortAndCopy;
            result.input_data_type = DynamicSortData_UInt32;
            result.algorithm = algorithm;

            result.sort_copy.sort._array_uint32 = _array_uint32;
            result.sort_copy.sort._size = _size;
            result.sort_copy.sort._tmp_array_uint32 = _tmp_array_uint32;

            result.sort_copy.copy._Dst = _Dst;
            result.sort_copy.copy._Src = _Src;
            result.sort_copy.copy._Size = _Size;

            return result;
        }

        static DynamicSortJob CreateOnlySortInt32(
            DynamicSortAlgorithm algorithm,
            int32_t* _array_int32, uint32_t _size, int32_t* _tmp_array_int32) {

            DynamicSortJob result;

            result.type = DynamicSortJob_OnlySort;
            result.input_data_type = DynamicSortData_Int32;
            result.algorithm = algorithm;

            result.sort_copy.sort._array_int32 = _array_int32;
            result.sort_copy.sort._size = _size;
            result.sort_copy.sort._tmp_array_int32 = _tmp_array_int32;

            return result;
        }

        static DynamicSortJob CreateOnlySortUInt32(
            DynamicSortAlgorithm algorithm,
            uint32_t* _array_uint32, uint32_t _size, uint32_t* _tmp_array_uint32) {

            DynamicSortJob result;

            result.type = DynamicSortJob_OnlySort;
            result.input_data_type = DynamicSortData_UInt32;
            result.algorithm = algorithm;

            result.sort_copy.sort._array_uint32 = _array_uint32;
            result.sort_copy.sort._size = _size;
            result.sort_copy.sort._tmp_array_uint32 = _tmp_array_uint32;

            return result;
        }


        static DynamicSortJob CreateMergeInt32(int32_t* in, int32_t* out, int i, int element_count, int size) {

            DynamicSortJob result;

            result.type = DynamicSortJob_Merge;
            result.input_data_type = DynamicSortData_Int32;

            result.merge.in_int32 = in;
            result.merge.out_int32 = out;
            result.merge.i = i;
            result.merge.element_count = element_count;
            result.merge.size = size;

            return result;
        }

        static DynamicSortJob CreateMergeUInt32(uint32_t* in, uint32_t* out, int i, int element_count, int size) {

            DynamicSortJob result;

            result.type = DynamicSortJob_Merge;
            result.input_data_type = DynamicSortData_UInt32;

            result.merge.in_uint32 = in;
            result.merge.out_uint32 = out;
            result.merge.i = i;
            result.merge.element_count = element_count;
            result.merge.size = size;

            return result;
        }


        //
        // Index data Types
        //

        static DynamicSortJob CreateSortAndCopyIndexInt32(
            DynamicSortAlgorithm algorithm,
            IndexInt32* _array_IndexInt32, uint32_t _size, IndexInt32* _tmp_array_IndexInt32,
            void* _Dst, const void* _Src, size_t _Size) {

            DynamicSortJob result;

            result.type = DynamicSortJob_SortAndCopy;
            result.input_data_type = DynamicSortData_IndexInt32;
            result.algorithm = algorithm;

            result.sort_copy.sort._array_IndexInt32 = _array_IndexInt32;
            result.sort_copy.sort._size = _size;
            result.sort_copy.sort._tmp_array_IndexInt32 = _tmp_array_IndexInt32;

            result.sort_copy.copy._Dst = _Dst;
            result.sort_copy.copy._Src = _Src;
            result.sort_copy.copy._Size = _Size;

            return result;
        }

        static DynamicSortJob CreateSortAndCopyIndexUInt32(
            DynamicSortAlgorithm algorithm,
            IndexUInt32* _array_IndexUInt32, uint32_t _size, IndexUInt32* _tmp_array_IndexUInt32,
            void* _Dst, const void* _Src, size_t _Size) {

            DynamicSortJob result;

            result.type = DynamicSortJob_SortAndCopy;
            result.input_data_type = DynamicSortData_IndexUInt32;
            result.algorithm = algorithm;

            result.sort_copy.sort._array_IndexUInt32 = _array_IndexUInt32;
            result.sort_copy.sort._size = _size;
            result.sort_copy.sort._tmp_array_IndexUInt32 = _tmp_array_IndexUInt32;

            result.sort_copy.copy._Dst = _Dst;
            result.sort_copy.copy._Src = _Src;
            result.sort_copy.copy._Size = _Size;

            return result;
        }

        static DynamicSortJob CreateOnlySortIndexInt32(
            DynamicSortAlgorithm algorithm,
            IndexInt32* _array_IndexInt32, uint32_t _size, IndexInt32* _tmp_array_IndexInt32) {

            DynamicSortJob result;

            result.type = DynamicSortJob_OnlySort;
            result.input_data_type = DynamicSortData_IndexInt32;
            result.algorithm = algorithm;

            result.sort_copy.sort._array_IndexInt32 = _array_IndexInt32;
            result.sort_copy.sort._size = _size;
            result.sort_copy.sort._tmp_array_IndexInt32 = _tmp_array_IndexInt32;

            return result;
        }

        static DynamicSortJob CreateOnlySortIndexUInt32(
            DynamicSortAlgorithm algorithm,
            IndexUInt32* _array_IndexUInt32, uint32_t _size, IndexUInt32* _tmp_array_IndexUInt32) {

            DynamicSortJob result;

            result.type = DynamicSortJob_OnlySort;
            result.input_data_type = DynamicSortData_IndexUInt32;
            result.algorithm = algorithm;

            result.sort_copy.sort._array_IndexUInt32 = _array_IndexUInt32;
            result.sort_copy.sort._size = _size;
            result.sort_copy.sort._tmp_array_IndexUInt32 = _tmp_array_IndexUInt32;

            return result;
        }


        static DynamicSortJob CreateMergeIndexInt32(IndexInt32* in, IndexInt32* out, int i, int element_count, int size) {

            DynamicSortJob result;

            result.type = DynamicSortJob_Merge;
            result.input_data_type = DynamicSortData_IndexInt32;

            result.merge.in_IndexInt32 = in;
            result.merge.out_IndexInt32 = out;
            result.merge.i = i;
            result.merge.element_count = element_count;
            result.merge.size = size;

            return result;
        }

        static DynamicSortJob CreateMergeIndexUInt32(IndexUInt32* in, IndexUInt32* out, int i, int element_count, int size) {

            DynamicSortJob result;

            result.type = DynamicSortJob_Merge;
            result.input_data_type = DynamicSortData_IndexUInt32;

            result.merge.in_IndexUInt32 = in;
            result.merge.out_IndexUInt32 = out;
            result.merge.i = i;
            result.merge.element_count = element_count;
            result.merge.size = size;

            return result;
        }


        void DynamicSort::merge_job_int32(const int32_t* in, int32_t* out, int i, int element_count, int size) {

            int write_index = i;
            int write_max = i + (element_count << 1);
            if (write_max > size)
                write_max = size;

            int a_index = i;
            int b_index = i + element_count;

            int a_max = b_index;
            int b_max = b_index + element_count;

            if (a_max > size)
                a_max = size;
            if (b_max > size)
                b_max = size;

            while (write_index < write_max &&
                a_index < a_max &&
                b_index < b_max) {

                const int32_t& _a = in[a_index];
                const int32_t& _b = in[b_index];

                if (_a > _b) {
                    out[write_index] = _b;
                    b_index++;
                }
                else {
                    out[write_index] = _a;
                    a_index++;
                }

                write_index++;
            }

            while (a_index < a_max) {
                out[write_index++] = in[a_index++];
            }
            while (b_index < b_max) {
                out[write_index++] = in[b_index++];
            }

        }

        void DynamicSort::merge_job_uint32(const uint32_t* in, uint32_t* out, int i, int element_count, int size) {

            int write_index = i;
            int write_max = i + (element_count << 1);
            if (write_max > size)
                write_max = size;

            int a_index = i;
            int b_index = i + element_count;

            int a_max = b_index;
            int b_max = b_index + element_count;

            if (a_max > size)
                a_max = size;
            if (b_max > size)
                b_max = size;

            while (write_index < write_max &&
                a_index < a_max &&
                b_index < b_max) {

                const uint32_t& _a = in[a_index];
                const uint32_t& _b = in[b_index];

                if (_a > _b) {
                    out[write_index] = _b;
                    b_index++;
                }
                else {
                    out[write_index] = _a;
                    a_index++;
                }

                write_index++;
            }

            while (a_index < a_max) {
                out[write_index++] = in[a_index++];
            }
            while (b_index < b_max) {
                out[write_index++] = in[b_index++];
            }

        }


        void DynamicSort::merge_job_IndexInt32(const IndexInt32* in, IndexInt32* out, int i, int element_count, int size) {

            int write_index = i;
            int write_max = i + (element_count << 1);
            if (write_max > size)
                write_max = size;

            int a_index = i;
            int b_index = i + element_count;

            int a_max = b_index;
            int b_max = b_index + element_count;

            if (a_max > size)
                a_max = size;
            if (b_max > size)
                b_max = size;

            while (write_index < write_max &&
                a_index < a_max &&
                b_index < b_max) {

                const IndexInt32& _a = in[a_index];
                const IndexInt32& _b = in[b_index];

                if (_a.toSort > _b.toSort) {
                    out[write_index] = _b;
                    b_index++;
                }
                else {
                    out[write_index] = _a;
                    a_index++;
                }

                write_index++;
            }

            while (a_index < a_max) {
                out[write_index++] = in[a_index++];
            }
            while (b_index < b_max) {
                out[write_index++] = in[b_index++];
            }

        }

        void DynamicSort::merge_job_IndexUInt32(const IndexUInt32* in, IndexUInt32* out, int i, int element_count, int size) {

            int write_index = i;
            int write_max = i + (element_count << 1);
            if (write_max > size)
                write_max = size;

            int a_index = i;
            int b_index = i + element_count;

            int a_max = b_index;
            int b_max = b_index + element_count;

            if (a_max > size)
                a_max = size;
            if (b_max > size)
                b_max = size;

            while (write_index < write_max &&
                a_index < a_max &&
                b_index < b_max) {

                const IndexUInt32& _a = in[a_index];
                const IndexUInt32& _b = in[b_index];

                if (_a.toSort > _b.toSort) {
                    out[write_index] = _b;
                    b_index++;
                }
                else {
                    out[write_index] = _a;
                    a_index++;
                }

                write_index++;
            }

            while (a_index < a_max) {
                out[write_index++] = in[a_index++];
            }
            while (b_index < b_max) {
                out[write_index++] = in[b_index++];
            }

        }

        void DynamicSort::task_run() {
            bool isSignaled;
            DynamicSortJob job = queue.dequeue(&isSignaled);
            if (isSignaled)
                return;

            switch (job.input_data_type) {
            case DynamicSortData_Int32:
                //
                // Int32
                //
                switch (job.type) {
                case DynamicSortJob_SortAndCopy:
                    switch (job.algorithm) {
                    case DynamicSortAlgorithm_radix_counting:
                        radix_counting_sort_signed(job.sort_copy.sort._array_int32, job.sort_copy.sort._size, job.sort_copy.sort._tmp_array_int32);
                        break;
                    case DynamicSortAlgorithm_std:
                        std::sort(job.sort_copy.sort._array_int32, job.sort_copy.sort._array_int32 + job.sort_copy.sort._size);
                        break;
                    default:
                        break;
                    }
                    memcpy(job.sort_copy.copy._Dst, job.sort_copy.copy._Src, job.sort_copy.copy._Size);
                    break;
                case DynamicSortJob_OnlySort:
                    switch (job.algorithm) {
                    case DynamicSortAlgorithm_radix_counting:
                        radix_counting_sort_signed(job.sort_copy.sort._array_int32, job.sort_copy.sort._size, job.sort_copy.sort._tmp_array_int32);
                        break;
                    case DynamicSortAlgorithm_std:
                        std::sort(job.sort_copy.sort._array_int32, job.sort_copy.sort._array_int32 + job.sort_copy.sort._size);
                        break;
                    default:
                        break;
                    }
                    break;
                case DynamicSortJob_Merge:
                    merge_job_int32(job.merge.in_int32, job.merge.out_int32, job.merge.i, job.merge.element_count, job.merge.size);
                    break;
                default:
                    break;
                }
                break;
            case DynamicSortData_UInt32:
                //
                // UInt32
                //
                switch (job.type) {
                case DynamicSortJob_SortAndCopy:
                    switch (job.algorithm) {
                    case DynamicSortAlgorithm_radix_counting:
                        radix_counting_sort_unsigned(job.sort_copy.sort._array_uint32, job.sort_copy.sort._size, job.sort_copy.sort._tmp_array_uint32);
                        break;
                    case DynamicSortAlgorithm_std:
                        std::sort(job.sort_copy.sort._array_uint32, job.sort_copy.sort._array_uint32 + job.sort_copy.sort._size);
                        break;
                    default:
                        break;
                    }
                    memcpy(job.sort_copy.copy._Dst, job.sort_copy.copy._Src, job.sort_copy.copy._Size);
                    break;
                case DynamicSortJob_OnlySort:
                    switch (job.algorithm) {
                    case DynamicSortAlgorithm_radix_counting:
                        radix_counting_sort_unsigned(job.sort_copy.sort._array_uint32, job.sort_copy.sort._size, job.sort_copy.sort._tmp_array_uint32);
                        break;
                    case DynamicSortAlgorithm_std:
                        std::sort(job.sort_copy.sort._array_uint32, job.sort_copy.sort._array_uint32 + job.sort_copy.sort._size);
                        break;
                    default:
                        break;
                    }
                    break;
                case DynamicSortJob_Merge:
                    merge_job_uint32(job.merge.in_uint32, job.merge.out_uint32, job.merge.i, job.merge.element_count, job.merge.size);
                    break;
                default:
                    break;
                }
                break;
            case DynamicSortData_IndexInt32:
                //
                // IndexInt32
                //
                switch (job.type) {
                case DynamicSortJob_SortAndCopy:
                    switch (job.algorithm) {
                    case DynamicSortAlgorithm_radix_counting:
                        radix_counting_sort_signed_index(job.sort_copy.sort._array_IndexInt32, job.sort_copy.sort._size, job.sort_copy.sort._tmp_array_IndexInt32);
                        break;
                    case DynamicSortAlgorithm_std:
                        std::sort(
                            job.sort_copy.sort._array_IndexInt32, 
                            job.sort_copy.sort._array_IndexInt32 + job.sort_copy.sort._size,
                            IndexInt32::comparator
                        );
                        //ARIBEIRO_ABORT(true, "comparator not implemented\n");
                        break;
                    default:
                        break;
                    }
                    memcpy(job.sort_copy.copy._Dst, job.sort_copy.copy._Src, job.sort_copy.copy._Size);
                    break;
                case DynamicSortJob_OnlySort:
                    switch (job.algorithm) {
                    case DynamicSortAlgorithm_radix_counting:
                        radix_counting_sort_signed_index(job.sort_copy.sort._array_IndexInt32, job.sort_copy.sort._size, job.sort_copy.sort._tmp_array_IndexInt32);
                        break;
                    case DynamicSortAlgorithm_std:
                        std::sort(
                            job.sort_copy.sort._array_IndexInt32, 
                            job.sort_copy.sort._array_IndexInt32 + job.sort_copy.sort._size,
                            IndexInt32::comparator
                        );
                        //ARIBEIRO_ABORT(true, "comparator not implemented\n");
                        break;
                    default:
                        break;
                    }
                    break;
                case DynamicSortJob_Merge:
                    merge_job_int32(job.merge.in_int32, job.merge.out_int32, job.merge.i, job.merge.element_count, job.merge.size);
                    break;
                default:
                    break;
                }
                break;
            case DynamicSortData_IndexUInt32:
                //
                // IndexUInt32
                //
                switch (job.type) {
                case DynamicSortJob_SortAndCopy:
                    switch (job.algorithm) {
                    case DynamicSortAlgorithm_radix_counting:
                        radix_counting_sort_unsigned_index(job.sort_copy.sort._array_IndexUInt32, job.sort_copy.sort._size, job.sort_copy.sort._tmp_array_IndexUInt32);
                        break;
                    case DynamicSortAlgorithm_std:
                        std::sort(
                            job.sort_copy.sort._array_IndexUInt32, 
                            job.sort_copy.sort._array_IndexUInt32 + job.sort_copy.sort._size,
                            IndexUInt32::comparator
                        );
                        //ARIBEIRO_ABORT(true, "comparator not implemented\n");
                        break;
                    default:
                        break;
                    }
                    memcpy(job.sort_copy.copy._Dst, job.sort_copy.copy._Src, job.sort_copy.copy._Size);
                    break;
                case DynamicSortJob_OnlySort:
                    switch (job.algorithm) {
                    case DynamicSortAlgorithm_radix_counting:
                        radix_counting_sort_unsigned_index(job.sort_copy.sort._array_IndexUInt32, job.sort_copy.sort._size, job.sort_copy.sort._tmp_array_IndexUInt32);
                        break;
                    case DynamicSortAlgorithm_std:
                        std::sort(
                            job.sort_copy.sort._array_IndexUInt32, 
                            job.sort_copy.sort._array_IndexUInt32 + job.sort_copy.sort._size,
                            IndexUInt32::comparator
                        );
                        //ARIBEIRO_ABORT(true, "comparator not implemented\n");
                        break;
                    default:
                        break;
                    }
                    break;
                case DynamicSortJob_Merge:
                    merge_job_uint32(job.merge.in_uint32, job.merge.out_uint32, job.merge.i, job.merge.element_count, job.merge.size);
                    break;
                default:
                        break;
                }
                break;
            default:
                break;
            }

            semaphore.release();
        }

        void DynamicSort::bucket_int32_t(int32_t* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            const int64_t bucket_count = 128;
            std::vector< int32_t >* bucket_list = new std::vector< int32_t >[bucket_count];

            int64_t min = INT32_MIN;
            int64_t max = INT32_MAX;
            int64_t delta = max - min + 1;

            for (uint32_t i = 0; i < size; i++) {
                int32_t element32 = A[i];
                int64_t element64 = (int64_t)element32;
                uint32_t index = ((element64 - min) * bucket_count) / delta;
                bucket_list[index].push_back(element32);
            }

            int a_offset[bucket_count];
            int offset = 0;
            for (int i = 0; i < bucket_count; i++) {
                a_offset[i] = offset;
                offset += (int)bucket_list[i].size();
            }

            for (int i = 0; i < bucket_count; i++) {
                std::vector< int32_t >& _bucket = bucket_list[i];
                if (_bucket.size() == 0) {
                    semaphore.release();
                    continue;
                }
                DynamicSortJob job = CreateSortAndCopyInt32(
                    //algorithm
                    algorithm,
                    //sort
                    &_bucket[0], (uint32_t)_bucket.size(), ((int32_t*)auxBuffer.data) + a_offset[i],
                    //copy
                    A + a_offset[i], &_bucket[0], sizeof(int32_t) * _bucket.size() );

                queue.enqueue(job);
            }

            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < bucket_count; i++)
                semaphore.blockingAcquire();

            delete[]bucket_list;

        }


        void DynamicSort::counting_int32_t(int32_t* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            const int64_t bucket_count = 128;
            //std::vector< T > bucket_list[bucket_count];
            counter_type counting[bucket_count];
            counter_type offset[bucket_count];
            memset(counting, 0, sizeof(counter_type) * bucket_count);

            int64_t min = INT32_MIN;
            int64_t max = INT32_MAX;
            int64_t delta = max - min + 1;

            int32_t* aux = ((int32_t*)auxBuffer.data);

            //count
            for (int i = 0; i < size; i++) {
                int32_t element32 = A[i];
                int64_t element64 = (int64_t)element32;
                uint32_t index = ((element64 - min) * bucket_count) / delta;
                counting[index]++;
            }

            //compute offset
            counter_type acc = counting[0];
            counting[0] = 0;
            for (uint32_t j = 1; j < bucket_count; j++) {
                counter_type tmp = counting[j];
                counting[j] = acc;
                acc += tmp;
            }

            memcpy(offset, counting, sizeof(counter_type) * bucket_count);

            // place elements in the output array
            for (uint32_t i = 0; i < size; i++) {
                int32_t element32 = A[i];
                int64_t element64 = (int64_t)element32;
                uint32_t index = ((element64 - min) * bucket_count) / delta;
                counter_type out_index = counting[index];
                counting[index]++;
                aux[out_index] = element32;
            }

            for (int i = 0; i < bucket_count; i++) {
                int32_t element_count = counting[i] - offset[i];
                if (element_count == 0) {
                    semaphore.release();
                    continue;
                }

                DynamicSortJob job = CreateSortAndCopyInt32(
                    //algorithm
                    algorithm,
                    //sort
                    aux + offset[i], element_count, A + offset[i],
                    //copy
                    A + offset[i], aux + offset[i], element_count * sizeof(int32_t));

                queue.enqueue(job);
            }

            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < bucket_count; i++)
                semaphore.blockingAcquire();

        }


        void DynamicSort::merge_int32_t(int32_t* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            int32_t* _aux = ((int32_t*)auxBuffer.data);

            int job_thread_size = size / PlatformThread::QueryNumberOfSystemThreads();// 1 << 16

            if (job_thread_size == 0)
                job_thread_size = 1;

            // sort blocks
            for (int i = 0; i < size; i += job_thread_size) {
                int index_start = i;
                int index_end_exclusive = i + job_thread_size;
                if (index_end_exclusive > size)
                    index_end_exclusive = size;

                DynamicSortJob job = CreateOnlySortInt32(
                    algorithm,
                    A + index_start, index_end_exclusive - index_start, _aux + index_start);
                
                queue.enqueue(job);
            }

            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < size; i += job_thread_size)
                semaphore.blockingAcquire();



            // merge down the blocks
            int32_t* in = A;
            int32_t* out = _aux;

            int element_count = job_thread_size;

            while (element_count < size) {

                //merge operation
                for (int i = 0; i < size; i += (element_count << 1)) {
                    DynamicSortJob job = CreateMergeInt32(in, out, i, element_count, size);
                    queue.enqueue(job);
                }
                // post task for processing the created queue
                for (int i = (int)queue.size() - 1; i >= 0; i--)
                    threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));
                for (int i = 0; i < size; i += (element_count << 1))
                    semaphore.blockingAcquire();

                //swap in/out
                int32_t* aux = in;
                in = out;
                out = aux;

                element_count = element_count << 1;
            }

            if (in != A)
                memcpy(A, in, sizeof(int32_t) * size);
        }

        void DynamicSort::bucket_uint32_t(uint32_t* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            const int64_t bucket_count = 128;
            std::vector< uint32_t >* bucket_list = new std::vector< uint32_t >[bucket_count];

            int64_t min = 0;
            int64_t max = UINT32_MAX;
            int64_t delta = max - min + 1;

            for (uint32_t i = 0; i < size; i++) {
                uint32_t element32 = A[i];
                int64_t element64 = (int64_t)element32;
                uint32_t index = ((element64 - min) * bucket_count) / delta;
                bucket_list[index].push_back(element32);
            }

            int a_offset[bucket_count];
            int offset = 0;
            for (int i = 0; i < bucket_count; i++) {
                a_offset[i] = offset;
                offset += (int)bucket_list[i].size();
            }

            for (int i = 0; i < bucket_count; i++) {
                std::vector< uint32_t >& _bucket = bucket_list[i];

                if (_bucket.size() == 0) {
                    semaphore.release();
                    continue;
                }

                DynamicSortJob job = CreateSortAndCopyUInt32(
                    //algorithm
                    algorithm,
                    //sort
                    &_bucket[0], (uint32_t)_bucket.size(), ((uint32_t*)auxBuffer.data) + a_offset[i],
                    //copy
                    A + a_offset[i], &_bucket[0], sizeof(uint32_t) * _bucket.size());

                queue.enqueue(job);
            }
            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));
            
            for (int i = 0; i < bucket_count; i++)
                semaphore.blockingAcquire();

            delete[]bucket_list;
        }
        void DynamicSort::counting_uint32_t(uint32_t* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            const int64_t bucket_count = 128;
            //std::vector< T > bucket_list[bucket_count];
            counter_type counting[bucket_count];
            counter_type offset[bucket_count];
            memset(counting, 0, sizeof(counter_type) * bucket_count);

            int64_t min = 0;
            int64_t max = UINT32_MAX;
            int64_t delta = max - min + 1;

            uint32_t* aux = ((uint32_t*)auxBuffer.data);

            //count
            for (int i = 0; i < size; i++) {
                uint32_t element32 = A[i];
                int64_t element64 = (int64_t)element32 - min;
                uint32_t index = (element64 * bucket_count) / delta;
                counting[index]++;
            }

            //compute offset
            counter_type acc = counting[0];
            counting[0] = 0;
            for (uint32_t j = 1; j < bucket_count; j++) {
                counter_type tmp = counting[j];
                counting[j] = acc;
                acc += tmp;
            }

            memcpy(offset, counting, sizeof(counter_type) * bucket_count);

            // place elements in the output array
            for (uint32_t i = 0; i < size; i++) {
                uint32_t element32 = A[i];
                int64_t element64 = (int64_t)element32 - min;
                uint32_t index = (element64 * bucket_count) / delta;
                counter_type out_index = counting[index];
                counting[index]++;
                aux[out_index] = element32;
            }

            for (int i = 0; i < bucket_count; i++) {
                int32_t element_count = counting[i] - offset[i];

                if (element_count == 0) {
                    semaphore.release();
                    continue;
                }

                DynamicSortJob job = CreateSortAndCopyUInt32(
                    //algorithm
                    algorithm,
                    //sort
                    aux + offset[i], element_count, A + offset[i],
                    //copy
                    A + offset[i], aux + offset[i], element_count * sizeof(uint32_t));

                queue.enqueue(job);
            }
            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < bucket_count; i++)
                semaphore.blockingAcquire();


        }
        void DynamicSort::merge_uint32_t(uint32_t* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            uint32_t* _aux = ((uint32_t*)auxBuffer.data);

            int job_thread_size = size / PlatformThread::QueryNumberOfSystemThreads();// 1 << 16
            //job_thread_size /= 4;

            if (job_thread_size == 0)
                job_thread_size = 1;

            // sort blocks
            for (int i = 0; i < size; i += job_thread_size) {
                int index_start = i;
                int index_end_exclusive = i + job_thread_size;
                if (index_end_exclusive > size)
                    index_end_exclusive = size;

                DynamicSortJob job = CreateOnlySortUInt32(
                    algorithm,
                    A + index_start, index_end_exclusive - index_start, _aux + index_start);

                queue.enqueue(job);
            }
            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < size; i += job_thread_size)
                semaphore.blockingAcquire();

            // merge down the blocks
            uint32_t* in = A;
            uint32_t* out = _aux;

            int element_count = job_thread_size;

            while (element_count < size) {

                //merge operation
                for (int i = 0; i < size; i += (element_count << 1)) {
                    DynamicSortJob job = CreateMergeUInt32(in, out, i, element_count, size);
                    queue.enqueue(job);
                }
                // post task for processing the created queue
                for (int i = (int)queue.size() - 1; i >= 0; i--)
                    threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

                for (int i = 0; i < size; i += (element_count << 1))
                    semaphore.blockingAcquire();

                //swap in/out
                uint32_t* aux = in;
                in = out;
                out = aux;

                element_count = element_count << 1;
            }

            if (in != A)
                memcpy(A, in, sizeof(uint32_t) * size);
        }

        //
        // IndexInt32 and IndexUInt32 versions
        //

        void DynamicSort::bucket_IndexInt32(IndexInt32* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            const int64_t bucket_count = 128;
            std::vector< IndexInt32 >* bucket_list = new std::vector< IndexInt32 >[bucket_count];

            int64_t min = INT32_MIN;
            int64_t max = INT32_MAX;
            int64_t delta = max - min + 1;

            for (uint32_t i = 0; i < size; i++) {
                const IndexInt32 &element32 = A[i];
                int64_t element64 = (int64_t)element32.toSort;
                uint32_t index = ((element64 - min) * bucket_count) / delta;
                bucket_list[index].push_back(element32);
            }

            int a_offset[bucket_count];
            int offset = 0;
            for (int i = 0; i < bucket_count; i++) {
                a_offset[i] = offset;
                offset += (int)bucket_list[i].size();
            }

            for (int i = 0; i < bucket_count; i++) {
                std::vector< IndexInt32 >& _bucket = bucket_list[i];
                if (_bucket.size() == 0) {
                    semaphore.release();
                    continue;
                }
                DynamicSortJob job = CreateSortAndCopyIndexInt32(
                    //algorithm
                    algorithm,
                    //sort
                    &_bucket[0], (uint32_t)_bucket.size(), ((IndexInt32*)auxBuffer.data) + a_offset[i],
                    //copy
                    A + a_offset[i], &_bucket[0], sizeof(IndexInt32) * _bucket.size());

                queue.enqueue(job);
            }
            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < bucket_count; i++)
                semaphore.blockingAcquire();

            delete[]bucket_list;

        }


        void DynamicSort::counting_IndexInt32(IndexInt32* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            const int64_t bucket_count = 128;
            //std::vector< T > bucket_list[bucket_count];
            counter_type counting[bucket_count];
            counter_type offset[bucket_count];
            memset(counting, 0, sizeof(counter_type) * bucket_count);

            int64_t min = INT32_MIN;
            int64_t max = INT32_MAX;
            int64_t delta = max - min + 1;

            IndexInt32* aux = ((IndexInt32*)auxBuffer.data);

            //count
            for (int i = 0; i < size; i++) {
                const IndexInt32 &element32 = A[i];
                int64_t element64 = (int64_t)element32.toSort;
                uint32_t index = ((element64 - min) * bucket_count) / delta;
                counting[index]++;
            }

            //compute offset
            counter_type acc = counting[0];
            counting[0] = 0;
            for (uint32_t j = 1; j < bucket_count; j++) {
                counter_type tmp = counting[j];
                counting[j] = acc;
                acc += tmp;
            }

            memcpy(offset, counting, sizeof(counter_type) * bucket_count);

            // place elements in the output array
            for (uint32_t i = 0; i < size; i++) {
                const IndexInt32 &element32 = A[i];
                int64_t element64 = (int64_t)element32.toSort;
                uint32_t index = ((element64 - min) * bucket_count) / delta;
                counter_type out_index = counting[index];
                counting[index]++;
                aux[out_index] = element32;
            }

            for (int i = 0; i < bucket_count; i++) {
                int32_t element_count = counting[i] - offset[i];
                if (element_count == 0) {
                    semaphore.release();
                    continue;
                }

                DynamicSortJob job = CreateSortAndCopyIndexInt32(
                    //algorithm
                    algorithm,
                    //sort
                    aux + offset[i], element_count, A + offset[i],
                    //copy
                    A + offset[i], aux + offset[i], element_count * sizeof(IndexInt32));

                queue.enqueue(job);
            }
            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < bucket_count; i++)
                semaphore.blockingAcquire();

        }


        void DynamicSort::merge_IndexInt32(IndexInt32* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            IndexInt32* _aux = ((IndexInt32*)auxBuffer.data);

            int job_thread_size = size / PlatformThread::QueryNumberOfSystemThreads();// 1 << 16

            if (job_thread_size == 0)
                job_thread_size = 1;

            // sort blocks
            for (int i = 0; i < size; i += job_thread_size) {
                int index_start = i;
                int index_end_exclusive = i + job_thread_size;
                if (index_end_exclusive > size)
                    index_end_exclusive = size;

                DynamicSortJob job = CreateOnlySortIndexInt32(
                    algorithm,
                    A + index_start, index_end_exclusive - index_start, _aux + index_start);

                queue.enqueue(job);
            }
            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < size; i += job_thread_size)
                semaphore.blockingAcquire();



            // merge down the blocks
            IndexInt32* in = A;
            IndexInt32* out = _aux;

            int element_count = job_thread_size;

            while (element_count < size) {

                //merge operation
                for (int i = 0; i < size; i += (element_count << 1)) {
                    DynamicSortJob job = CreateMergeIndexInt32(in, out, i, element_count, size);
                    queue.enqueue(job);
                }
                // post task for processing the created queue
                for (int i = (int)queue.size() - 1; i >= 0; i--)
                    threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));
                for (int i = 0; i < size; i += (element_count << 1))
                    semaphore.blockingAcquire();

                //swap in/out
                IndexInt32* aux = in;
                in = out;
                out = aux;

                element_count = element_count << 1;
            }

            if (in != A)
                memcpy(A, in, sizeof(IndexInt32) * size);
        }

        void DynamicSort::bucket_IndexUInt32(IndexUInt32* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            const int64_t bucket_count = 128;
            std::vector< IndexUInt32 >* bucket_list = new std::vector< IndexUInt32 >[bucket_count];

            int64_t min = 0;
            int64_t max = UINT32_MAX;
            int64_t delta = max - min + 1;

            for (uint32_t i = 0; i < size; i++) {
                const IndexUInt32 &element32 = A[i];
                int64_t element64 = (int64_t)element32.toSort;
                uint32_t index = ((element64 - min) * bucket_count) / delta;
                bucket_list[index].push_back(element32);
            }

            int a_offset[bucket_count];
            int offset = 0;
            for (int i = 0; i < bucket_count; i++) {
                a_offset[i] = offset;
                offset += (int)bucket_list[i].size();
            }

            for (int i = 0; i < bucket_count; i++) {
                std::vector< IndexUInt32 >& _bucket = bucket_list[i];

                if (_bucket.size() == 0) {
                    semaphore.release();
                    continue;
                }

                DynamicSortJob job = CreateSortAndCopyIndexUInt32(
                    //algorithm
                    algorithm,
                    //sort
                    &_bucket[0], (uint32_t)_bucket.size(), ((IndexUInt32*)auxBuffer.data) + a_offset[i],
                    //copy
                    A + a_offset[i], &_bucket[0], sizeof(IndexUInt32) * _bucket.size());

                queue.enqueue(job);
            }
            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < bucket_count; i++)
                semaphore.blockingAcquire();

            delete[]bucket_list;
        }
        void DynamicSort::counting_IndexUInt32(IndexUInt32* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            const int64_t bucket_count = 128;
            //std::vector< T > bucket_list[bucket_count];
            counter_type counting[bucket_count];
            counter_type offset[bucket_count];
            memset(counting, 0, sizeof(counter_type) * bucket_count);

            int64_t min = 0;
            int64_t max = UINT32_MAX;
            int64_t delta = max - min + 1;

            IndexUInt32* aux = ((IndexUInt32*)auxBuffer.data);

            //count
            for (int i = 0; i < size; i++) {
                const IndexUInt32 &element32 = A[i];
                int64_t element64 = (int64_t)element32.toSort - min;
                uint32_t index = (element64 * bucket_count) / delta;
                counting[index]++;
            }

            //compute offset
            counter_type acc = counting[0];
            counting[0] = 0;
            for (uint32_t j = 1; j < bucket_count; j++) {
                counter_type tmp = counting[j];
                counting[j] = acc;
                acc += tmp;
            }

            memcpy(offset, counting, sizeof(counter_type) * bucket_count);

            // place elements in the output array
            for (uint32_t i = 0; i < size; i++) {
                const IndexUInt32 &element32 = A[i];
                int64_t element64 = (int64_t)element32.toSort - min;
                uint32_t index = (element64 * bucket_count) / delta;
                counter_type out_index = counting[index];
                counting[index]++;
                aux[out_index] = element32;
            }

            for (int i = 0; i < bucket_count; i++) {
                int32_t element_count = counting[i] - offset[i];

                if (element_count == 0) {
                    semaphore.release();
                    continue;
                }

                DynamicSortJob job = CreateSortAndCopyIndexUInt32(
                    //algorithm
                    algorithm,
                    //sort
                    aux + offset[i], element_count, A + offset[i],
                    //copy
                    A + offset[i], aux + offset[i], element_count * sizeof(IndexUInt32));

                queue.enqueue(job);
            }
            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < bucket_count; i++)
                semaphore.blockingAcquire();


        }
        void DynamicSort::merge_IndexUInt32(IndexUInt32* A, uint32_t size, DynamicSortAlgorithm algorithm) {
            IndexUInt32* _aux = ((IndexUInt32*)auxBuffer.data);

            int job_thread_size = size / PlatformThread::QueryNumberOfSystemThreads();// 1 << 16
            //job_thread_size /= 4;

            if (job_thread_size == 0)
                job_thread_size = 1;

            // sort blocks
            for (int i = 0; i < size; i += job_thread_size) {
                int index_start = i;
                int index_end_exclusive = i + job_thread_size;
                if (index_end_exclusive > size)
                    index_end_exclusive = size;

                DynamicSortJob job = CreateOnlySortIndexUInt32(
                    algorithm,
                    A + index_start, index_end_exclusive - index_start, _aux + index_start);

                queue.enqueue(job);
            }
            // post task for processing the created queue
            for (int i = (int)queue.size() - 1; i >= 0; i--)
                threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

            for (int i = 0; i < size; i += job_thread_size)
                semaphore.blockingAcquire();

            // merge down the blocks
            IndexUInt32* in = A;
            IndexUInt32* out = _aux;

            int element_count = job_thread_size;

            while (element_count < size) {

                //merge operation
                for (int i = 0; i < size; i += (element_count << 1)) {
                    DynamicSortJob job = CreateMergeIndexUInt32(in, out, i, element_count, size);
                    queue.enqueue(job);
                }
                // post task for processing the created queue
                for (int i = (int)queue.size() - 1; i >= 0; i--)
                    threadPool->postTask(TaskMethod_Fnc(this, &DynamicSort::task_run));

                for (int i = 0; i < size; i += (element_count << 1))
                    semaphore.blockingAcquire();

                //swap in/out
                IndexUInt32* aux = in;
                in = out;
                out = aux;

                element_count = element_count << 1;
            }

            if (in != A)
                memcpy(A, in, sizeof(IndexUInt32) * size);
        }


        DynamicSort::DynamicSort(ThreadPool* _threadPool, uint32_t _useMultithreadStartingAtCount) :semaphore(0) {
            threadPool = _threadPool;
            useMultithreadStartingAtCount = _useMultithreadStartingAtCount;
            /*

            for (int i = 0; i < PlatformThread::QueryNumberOfSystemThreads(); i++)
                threads.push_back(new PlatformThread(this, &DynamicSort::run));
            for (int i = 0; i < threads.size(); i++)
                threads[i]->start();

            printf("[DynamicSort] Created %i threads for the sorting algorithms\n", (int)threads.size());

            */
        }

        DynamicSort::~DynamicSort() {
            /*
            for (int i = 0; i < threads.size(); i++)
                threads[i]->interrupt();
            for (int i = 0; i < threads.size(); i++)
                threads[i]->wait();
            for (int i = 0; i < threads.size(); i++)
                delete threads[i];
            threads.clear();
            */
        }

        void DynamicSort::sort_int32_t(int32_t* A, uint32_t size, DynamicSortGather gather, DynamicSortAlgorithm algorithm) {

            PlatformAutoLock _autoLock(&mutex);

            auxBuffer.setSize(size * sizeof(int32_t));
            if (size < useMultithreadStartingAtCount) {

                switch (algorithm) {
                case DynamicSortAlgorithm_radix_counting:
                    radix_counting_sort_signed(A, size, (int32_t*)auxBuffer.data);
                    break;
                case DynamicSortAlgorithm_std:
                    std::sort(A, A + size);
                    break;
                default:
                    break;
                }

            }
            else {
                switch (gather) {
                case DynamicSortGather_bucket:
                    bucket_int32_t(A, size, algorithm);
                    break;
                case DynamicSortGather_counting:
                    counting_int32_t(A, size, algorithm);
                    break;
                case DynamicSortGather_merge:
                    merge_int32_t(A, size, algorithm);
                    break;
                default:
                    break;
                }
            }
        }

        void DynamicSort::sort_uint32_t(uint32_t* A, uint32_t size, DynamicSortGather gather, DynamicSortAlgorithm algorithm) {
            PlatformAutoLock _autoLock(&mutex);

            auxBuffer.setSize(size * sizeof(uint32_t));
            if (size < useMultithreadStartingAtCount) {

                switch (algorithm) {
                case DynamicSortAlgorithm_radix_counting:
                    radix_counting_sort_unsigned(A, size, (uint32_t*)auxBuffer.data);
                    break;
                case DynamicSortAlgorithm_std:
                    std::sort(A, A + size);
                    break;
                default:
                    break;
                }

            }
            else {
                switch (gather) {
                case DynamicSortGather_bucket:
                    bucket_uint32_t(A, size, algorithm);
                    break;
                case DynamicSortGather_counting:
                    counting_uint32_t(A, size, algorithm);
                    break;
                case DynamicSortGather_merge:
                    merge_uint32_t(A, size, algorithm);
                    break;
                default:
                    break;
                }
            }
        }


        void DynamicSort::sort_IndexInt32(IndexInt32* A, uint32_t size, DynamicSortGather gather, DynamicSortAlgorithm algorithm) {
            PlatformAutoLock _autoLock(&mutex);

            auxBuffer.setSize(size * sizeof(IndexInt32));
            if (size < useMultithreadStartingAtCount) {

                switch (algorithm) {
                case DynamicSortAlgorithm_radix_counting:
                    radix_counting_sort_signed_index(A, size, (IndexInt32*)auxBuffer.data);
                    break;
                case DynamicSortAlgorithm_std:
                    std::sort(A, A + size, IndexInt32::comparator);
                    break;
                default:
                    break;
                }

            }
            else {
                switch (gather) {
                case DynamicSortGather_bucket:
                    bucket_IndexInt32(A, size, algorithm);
                    break;
                case DynamicSortGather_counting:
                    counting_IndexInt32(A, size, algorithm);
                    break;
                case DynamicSortGather_merge:
                    merge_IndexInt32(A, size, algorithm);
                    break;
                default:
                    break;
                }
            }
        }

        void DynamicSort::sort_IndexUInt32(IndexUInt32* A, uint32_t size, DynamicSortGather gather, DynamicSortAlgorithm algorithm) {
            PlatformAutoLock _autoLock(&mutex);

            auxBuffer.setSize(size * sizeof(IndexUInt32));
            if (size < useMultithreadStartingAtCount) {

                switch (algorithm) {
                case DynamicSortAlgorithm_radix_counting:
                    radix_counting_sort_unsigned_index(A, size, (IndexUInt32*)auxBuffer.data);
                    break;
                case DynamicSortAlgorithm_std:
                    std::sort(A, A + size, IndexUInt32::comparator);
                    break;
                default:
                    break;
                }

            }
            else {
                switch (gather) {
                case DynamicSortGather_bucket:
                    bucket_IndexUInt32(A, size, algorithm);
                    break;
                case DynamicSortGather_counting:
                    counting_IndexUInt32(A, size, algorithm);
                    break;
                case DynamicSortGather_merge:
                    merge_IndexUInt32(A, size, algorithm);
                    break;
                default:
                    break;
                }
            }
        }

    }
}

