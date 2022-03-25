#include "AlgorithmsOpenMP.h"
#include <algorithm>

#include <aRibeiroPlatform/aRibeiroPlatform.h>

namespace aRibeiro {
    namespace Sorting {


        void hybrid_bucket_std_signed_OpenMP(int32_t* A, uint32_t size) {
            const int64_t bucket_count = 128;
            std::vector< int32_t >* bucket_list = new std::vector< int32_t >[bucket_count];

            int64_t min = INT32_MIN;
            int64_t max = INT32_MAX;
            int64_t delta = max - min + 1;

            for (int i = 0; i < size; i++) {
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

            #pragma omp parallel for
            for (int i = 0; i < bucket_count; i++) {
                std::vector< int32_t >& _bucket = bucket_list[i];
                if (_bucket.size() == 0)
                    continue;
                std::sort(_bucket.begin(), _bucket.end());
                memcpy(A + a_offset[i], &_bucket[0], sizeof(int32_t) * _bucket.size());
            }

            delete[]bucket_list;
        }

        void hybrid_bucket_radix_counting_signed_OpenMP(int32_t* A, uint32_t size, int32_t* tmp_array) {
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

            if (tmp_array == NULL) {
#pragma omp parallel for
                for (int i = 0; i < bucket_count; i++) {
                    std::vector< int32_t >& _bucket = bucket_list[i];
                    if (_bucket.size() == 0)
                        continue;
                    radix_counting_sort_signed(&_bucket[0], (uint32_t)_bucket.size(), NULL);
                    memcpy(A + a_offset[i], &_bucket[0], sizeof(int32_t) * _bucket.size());
                }
            }
            else {
#pragma omp parallel for
                for (int i = 0; i < bucket_count; i++) {
                    std::vector< int32_t >& _bucket = bucket_list[i];
                    if (_bucket.size() == 0)
                        continue;
                    radix_counting_sort_signed(&_bucket[0], (uint32_t)_bucket.size(), &tmp_array[a_offset[i]]);
                    memcpy(A + a_offset[i], &_bucket[0], sizeof(int32_t) * _bucket.size());
                }
            }
            delete[]bucket_list;
        }

        void hybrid_bucket_radix_counting_signed_index_OpenMP(IndexInt32* A, uint32_t size, IndexInt32* tmp_array) {
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

            if (tmp_array == NULL) {
#pragma omp parallel for
                for (int i = 0; i < bucket_count; i++) {
                    std::vector< IndexInt32 >& _bucket = bucket_list[i];
                    if (_bucket.size() == 0)
                        continue;
                    radix_counting_sort_signed_index(&_bucket[0], (uint32_t)_bucket.size(), NULL);
                    memcpy(A + a_offset[i], &_bucket[0], sizeof(IndexInt32) * _bucket.size());
                }
            }
            else {
#pragma omp parallel for
                for (int i = 0; i < bucket_count; i++) {
                    std::vector< IndexInt32 >& _bucket = bucket_list[i];
                    if (_bucket.size() == 0)
                        continue;
                    radix_counting_sort_signed_index(&_bucket[0], (uint32_t)_bucket.size(), &tmp_array[a_offset[i]]);
                    memcpy(A + a_offset[i], &_bucket[0], sizeof(IndexInt32) * _bucket.size());
                }
            }
            delete[]bucket_list;
        }

        void hybrid_counting_std_signed_OpenMP(int32_t* A, uint32_t size, int32_t* _tmp_array) {

            const int64_t bucket_count = 128;
            //std::vector< T > bucket_list[bucket_count];
            counter_type counting[bucket_count];
            counter_type offset[bucket_count];
            memset(counting, 0, sizeof(counter_type) * bucket_count);

            int64_t min = INT32_MIN;
            int64_t max = INT32_MAX;
            int64_t delta = max - min + 1;

            int32_t* aux;
            if (_tmp_array == NULL)
                aux = (int32_t*)malloc_aligned(size * sizeof(int32_t));
            else
                aux = _tmp_array;

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

#pragma omp parallel for
            for (int i = 0; i < bucket_count; i++) {
                int32_t element_count = counting[i] - offset[i];
                if (element_count == 0)
                    continue;
                std::sort(aux + offset[i], aux + counting[i]);
                memcpy(A + offset[i], aux + offset[i], element_count * sizeof(int32_t));
            }

            if (_tmp_array == NULL)
                free_aligned(aux);
        }

        void hybrid_counting_radix_counting_signed_OpenMP(int32_t* A, uint32_t size, int32_t* _tmp_array) {

            const int64_t bucket_count = 128;
            //std::vector< T > bucket_list[bucket_count];
            counter_type counting[bucket_count];
            counter_type offset[bucket_count];
            memset(counting, 0, sizeof(counter_type) * bucket_count);

            int64_t min = INT32_MIN;
            int64_t max = INT32_MAX;
            int64_t delta = max - min + 1;

            int32_t* aux;
            if (_tmp_array == NULL)
                aux = (int32_t*)malloc_aligned(size * sizeof(int32_t));
            else
                aux = _tmp_array;

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

            #pragma omp parallel for
            for (int i = 0; i < bucket_count; i++) {
                int32_t element_count = counting[i] - offset[i];
                if (element_count == 0)
                    continue;
                radix_counting_sort_signed(aux + offset[i], element_count, A + offset[i]);
                memcpy(A + offset[i], aux + offset[i], element_count * sizeof(int32_t));
            }

            if (_tmp_array == NULL)
                free_aligned(aux);
        }

        void hybrid_counting_radix_counting_signed_index_OpenMP(IndexInt32* A, uint32_t size, IndexInt32* _tmp_array) {

            const int64_t bucket_count = 128;
            //std::vector< T > bucket_list[bucket_count];
            counter_type counting[bucket_count];
            counter_type offset[bucket_count];
            memset(counting, 0, sizeof(counter_type) * bucket_count);

            int64_t min = INT32_MIN;
            int64_t max = INT32_MAX;
            int64_t delta = max - min + 1;

            IndexInt32* aux;
            if (_tmp_array == NULL)
                aux = (IndexInt32*)malloc_aligned(size * sizeof(IndexInt32));
            else
                aux = _tmp_array;

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

#pragma omp parallel for
            for (int i = 0; i < bucket_count; i++) {
                int32_t element_count = counting[i] - offset[i];
                if (element_count == 0)
                    continue;
                radix_counting_sort_signed_index(aux + offset[i], element_count, A + offset[i]);
                memcpy(A + offset[i], aux + offset[i], element_count * sizeof(IndexInt32));
            }

            if (_tmp_array == NULL)
                free_aligned(aux);
        }

        void hybrid_merge_std_signed_OpenMP(int32_t* _array, uint32_t size, int32_t* _pre_alloc_tmp) {
            int32_t* _aux;
            if (_pre_alloc_tmp == NULL)
                _aux = (int32_t*)malloc_aligned(sizeof(int32_t) * size);
            else
                _aux = _pre_alloc_tmp;

            int job_thread_size = size / PlatformThread::QueryNumberOfSystemThreads();// 1 << 16

            if (job_thread_size == 0)
                job_thread_size = 1;

            // sort blocks
#pragma omp parallel for
            for (int i = 0; i < size; i += job_thread_size) {
                int index_start = i;
                int index_end_exclusive = i + job_thread_size;
                if (index_end_exclusive > size)
                    index_end_exclusive = size;
                std::sort(_array + index_start, _array + index_end_exclusive);
            }

            // merge down the blocks
            int32_t* in = _array;
            int32_t* out = _aux;

            int element_count = job_thread_size;

            while (element_count < size) {

                //merge operation
#pragma omp parallel for
                for (int i = 0; i < size; i += (element_count << 1)) {

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

                        int32_t& _a = in[a_index];
                        int32_t& _b = in[b_index];

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

                //swap in/out
                int32_t* aux = in;
                in = out;
                out = aux;

                element_count = element_count << 1;
            }

            if (in != _array)
                memcpy(_array, in, sizeof(int32_t) * size);

            if (_pre_alloc_tmp == NULL)
                free_aligned(_aux);
        }

        void hybrid_merge_radix_counting_signed_OpenMP(int32_t* _array, uint32_t size, int32_t* _pre_alloc_tmp) {
            int32_t* _aux;
            if (_pre_alloc_tmp == NULL)
                _aux = (int32_t*)malloc_aligned(sizeof(int32_t) * size);
            else
                _aux = _pre_alloc_tmp;

            int job_thread_size = size / PlatformThread::QueryNumberOfSystemThreads();// 1 << 16

            if (job_thread_size == 0)
                job_thread_size = 1;

            // sort blocks
            #pragma omp parallel for
            for (int i = 0; i < size; i += job_thread_size) {
                int index_start = i;
                int index_end_exclusive = i + job_thread_size;
                if (index_end_exclusive > size)
                    index_end_exclusive = size;
                radix_counting_sort_signed(_array + index_start, index_end_exclusive - index_start, _aux + index_start);
            }

            // merge down the blocks
            int32_t* in = _array;
            int32_t* out = _aux;

            int element_count = job_thread_size;

            while (element_count < size) {

                //merge operation
                #pragma omp parallel for
                for (int i = 0; i < size; i += (element_count << 1) ) {
                    
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

                        int32_t &_a = in[a_index];
                        int32_t &_b = in[b_index];

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

                //swap in/out
                int32_t* aux = in;
                in = out;
                out = aux;

                element_count = element_count << 1;
            }

            if (in != _array)
                memcpy(_array, in, sizeof(int32_t) * size);

            if (_pre_alloc_tmp == NULL)
                free_aligned(_aux);
        }

        void hybrid_merge_radix_counting_signed_index_OpenMP(IndexInt32* _array, uint32_t size, IndexInt32* _pre_alloc_tmp) {
            IndexInt32* _aux;
            if (_pre_alloc_tmp == NULL)
                _aux = (IndexInt32*)malloc_aligned(sizeof(IndexInt32) * size);
            else
                _aux = _pre_alloc_tmp;

            int job_thread_size = size / PlatformThread::QueryNumberOfSystemThreads();// 1 << 16

            if (job_thread_size == 0)
                job_thread_size = 1;

            // sort blocks
#pragma omp parallel for
            for (int i = 0; i < size; i += job_thread_size) {
                int index_start = i;
                int index_end_exclusive = i + job_thread_size;
                if (index_end_exclusive > size)
                    index_end_exclusive = size;
                radix_counting_sort_signed_index(_array + index_start, index_end_exclusive - index_start, _aux + index_start);
            }

            // merge down the blocks
            IndexInt32* in = _array;
            IndexInt32* out = _aux;

            int element_count = job_thread_size;

            while (element_count < size) {

                //merge operation
#pragma omp parallel for
                for (int i = 0; i < size; i += (element_count << 1)) {

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

                //swap in/out
                IndexInt32* aux = in;
                in = out;
                out = aux;

                element_count = element_count << 1;
            }

            if (in != _array)
                memcpy(_array, in, sizeof(IndexInt32) * size);

            if (_pre_alloc_tmp == NULL)
                free_aligned(_aux);
        }

        

        void hybrid_bucket_std_unsigned_OpenMP(uint32_t* A, uint32_t size) {
            const int64_t bucket_count = 128;
            std::vector< uint32_t >* bucket_list = new std::vector< uint32_t >[bucket_count];

            int64_t min = 0;
            int64_t max = UINT32_MAX;
            int64_t delta = max - min + 1;

            for (int i = 0; i < size; i++) {
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

            #pragma omp parallel for
            for (int i = 0; i < bucket_count; i++) {
                std::vector< uint32_t >& _bucket = bucket_list[i];
                if (_bucket.size() == 0)
                    continue;
                std::sort(_bucket.begin(), _bucket.end());
                memcpy(A + a_offset[i], &_bucket[0], sizeof(uint32_t) * _bucket.size());
            }

            delete[]bucket_list;
        }

        void hybrid_bucket_radix_counting_unsigned_OpenMP(uint32_t* A, uint32_t size, uint32_t* tmp_array) {
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

            if (tmp_array == NULL) {
#pragma omp parallel for
                for (int i = 0; i < bucket_count; i++) {
                    std::vector< uint32_t >& _bucket = bucket_list[i];
                    if (_bucket.size() == 0)
                        continue;
                    radix_counting_sort_unsigned(&_bucket[0], (uint32_t)_bucket.size(), NULL);
                    memcpy(A + a_offset[i], &_bucket[0], sizeof(uint32_t) * _bucket.size());
                }
            }
            else {
#pragma omp parallel for
                for (int i = 0; i < bucket_count; i++) {
                    std::vector< uint32_t >& _bucket = bucket_list[i];
                    if (_bucket.size() == 0)
                        continue;
                    radix_counting_sort_unsigned(&_bucket[0], (uint32_t)_bucket.size(), &tmp_array[a_offset[i]]);
                    memcpy(A + a_offset[i], &_bucket[0], sizeof(uint32_t) * _bucket.size());
                }
            }
            delete[]bucket_list;
        }

        void hybrid_bucket_radix_counting_unsigned_index_OpenMP(IndexUInt32* A, uint32_t size, IndexUInt32* tmp_array) {
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

            if (tmp_array == NULL) {
#pragma omp parallel for
                for (int i = 0; i < bucket_count; i++) {
                    std::vector< IndexUInt32 >& _bucket = bucket_list[i];
                    if (_bucket.size() == 0)
                        continue;
                    radix_counting_sort_unsigned_index(&_bucket[0], (uint32_t)_bucket.size(), NULL);
                    memcpy(A + a_offset[i], &_bucket[0], sizeof(IndexUInt32) * _bucket.size());
                }
            }
            else {
#pragma omp parallel for
                for (int i = 0; i < bucket_count; i++) {
                    std::vector< IndexUInt32 >& _bucket = bucket_list[i];
                    if (_bucket.size() == 0)
                        continue;
                    radix_counting_sort_unsigned_index(&_bucket[0], (uint32_t)_bucket.size(), &tmp_array[a_offset[i]]);
                    memcpy(A + a_offset[i], &_bucket[0], sizeof(IndexUInt32) * _bucket.size());
                }
            }
            delete[]bucket_list;
        }

        void hybrid_counting_std_unsigned_OpenMP(uint32_t* A, uint32_t size, uint32_t* _tmp_array) {

            const int64_t bucket_count = 128;
            //std::vector< T > bucket_list[bucket_count];
            counter_type counting[bucket_count];
            counter_type offset[bucket_count];
            memset(counting, 0, sizeof(counter_type) * bucket_count);

            int64_t min = 0;
            int64_t max = UINT32_MAX;
            int64_t delta = max - min + 1;

            uint32_t* aux;
            if (_tmp_array == NULL)
                aux = (uint32_t*)malloc_aligned(size * sizeof(uint32_t));
            else
                aux = _tmp_array;

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

#pragma omp parallel for
            for (int i = 0; i < bucket_count; i++) {
                int32_t element_count = counting[i] - offset[i];
                if (element_count == 0)
                    continue;
                std::sort(aux + offset[i], aux + counting[i]);
                memcpy(A + offset[i], aux + offset[i], element_count * sizeof(uint32_t));
            }

            if (_tmp_array == NULL)
                free_aligned(aux);
        }

        void hybrid_counting_radix_counting_unsigned_OpenMP(uint32_t* A, uint32_t size, uint32_t* _tmp_array) {

            const int64_t bucket_count = 128;
            //std::vector< T > bucket_list[bucket_count];
            counter_type counting[bucket_count];
            counter_type offset[bucket_count];
            memset(counting, 0, sizeof(counter_type) * bucket_count);

            int64_t min = 0;
            int64_t max = UINT32_MAX;
            int64_t delta = max - min + 1;

            uint32_t* aux;
            if (_tmp_array == NULL)
                aux = (uint32_t*)malloc_aligned(size * sizeof(uint32_t));
            else
                aux = _tmp_array;

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

#pragma omp parallel for
            for (int i = 0; i < bucket_count; i++) {
                int32_t element_count = counting[i] - offset[i];
                if (element_count == 0)
                    continue;
                radix_counting_sort_unsigned(aux + offset[i], element_count, A + offset[i]);
                memcpy(A + offset[i], aux + offset[i], element_count * sizeof(uint32_t));
            }

            if (_tmp_array == NULL)
                free_aligned(aux);
        }

        void hybrid_counting_radix_counting_unsigned_index_OpenMP(IndexUInt32* A, uint32_t size, IndexUInt32* _tmp_array) {

            const int64_t bucket_count = 128;
            //std::vector< T > bucket_list[bucket_count];
            counter_type counting[bucket_count];
            counter_type offset[bucket_count];
            memset(counting, 0, sizeof(counter_type) * bucket_count);

            int64_t min = 0;
            int64_t max = UINT32_MAX;
            int64_t delta = max - min + 1;

            IndexUInt32* aux;
            if (_tmp_array == NULL)
                aux = (IndexUInt32*)malloc_aligned(size * sizeof(IndexUInt32));
            else
                aux = _tmp_array;

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

#pragma omp parallel for
            for (int i = 0; i < bucket_count; i++) {
                int32_t element_count = counting[i] - offset[i];
                if (element_count == 0)
                    continue;
                radix_counting_sort_unsigned_index(aux + offset[i], element_count, A + offset[i]);
                memcpy(A + offset[i], aux + offset[i], element_count * sizeof(IndexUInt32));
            }

            if (_tmp_array == NULL)
                free_aligned(aux);
        }


        void hybrid_merge_std_unsigned_OpenMP(uint32_t* _array, uint32_t size, uint32_t* _pre_alloc_tmp) {
            uint32_t* _aux;
            if (_pre_alloc_tmp == NULL)
                _aux = (uint32_t*)malloc_aligned(sizeof(uint32_t) * size);
            else
                _aux = _pre_alloc_tmp;

            int job_thread_size = size / PlatformThread::QueryNumberOfSystemThreads();// 1 << 16
            //job_thread_size /= 4;

            if (job_thread_size == 0)
                job_thread_size = 1;

            // sort blocks
#pragma omp parallel for
            for (int i = 0; i < size; i += job_thread_size) {
                int index_start = i;
                int index_end_exclusive = i + job_thread_size;
                if (index_end_exclusive > size)
                    index_end_exclusive = size;
                std::sort(_array + index_start, _array + index_end_exclusive);
            }

            // merge down the blocks
            uint32_t* in = _array;
            uint32_t* out = _aux;

            int element_count = job_thread_size;

            while (element_count < size) {

                //merge operation
#pragma omp parallel for
                for (int i = 0; i < size; i += (element_count << 1)) {

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

                        uint32_t& _a = in[a_index];
                        uint32_t& _b = in[b_index];

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

                //swap in/out
                uint32_t* aux = in;
                in = out;
                out = aux;

                element_count = element_count << 1;
            }

            if (in != _array)
                memcpy(_array, in, sizeof(int32_t) * size);

            if (_pre_alloc_tmp == NULL)
                free_aligned(_aux);
        }

        void hybrid_merge_radix_counting_unsigned_OpenMP(uint32_t* _array, uint32_t size, uint32_t* _pre_alloc_tmp) {
            uint32_t* _aux;
            if (_pre_alloc_tmp == NULL)
                _aux = (uint32_t*)malloc_aligned(sizeof(uint32_t) * size);
            else
                _aux = _pre_alloc_tmp;

            int job_thread_size = size / PlatformThread::QueryNumberOfSystemThreads();// 1 << 16
            //job_thread_size /= 4;

            if (job_thread_size == 0)
                job_thread_size = 1;

            // sort blocks
#pragma omp parallel for
            for (int i = 0; i < size; i += job_thread_size) {
                int index_start = i;
                int index_end_exclusive = i + job_thread_size;
                if (index_end_exclusive > size)
                    index_end_exclusive = size;
                radix_counting_sort_unsigned(_array + index_start, index_end_exclusive - index_start, _aux + index_start);
            }

            // merge down the blocks
            uint32_t* in = _array;
            uint32_t* out = _aux;

            int element_count = job_thread_size;

            while (element_count < size) {

                //merge operation
#pragma omp parallel for
                for (int i = 0; i < size; i += (element_count << 1)) {

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

                        uint32_t& _a = in[a_index];
                        uint32_t& _b = in[b_index];

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

                //swap in/out
                uint32_t* aux = in;
                in = out;
                out = aux;

                element_count = element_count << 1;
            }

            if (in != _array)
                memcpy(_array, in, sizeof(uint32_t) * size);

            if (_pre_alloc_tmp == NULL)
                free_aligned(_aux);
        }

        void hybrid_merge_radix_counting_unsigned_index_OpenMP(IndexUInt32* _array, uint32_t size, IndexUInt32* _pre_alloc_tmp) {
            IndexUInt32* _aux;
            if (_pre_alloc_tmp == NULL)
                _aux = (IndexUInt32*)malloc_aligned(sizeof(IndexUInt32) * size);
            else
                _aux = _pre_alloc_tmp;

            int job_thread_size = size / PlatformThread::QueryNumberOfSystemThreads();// 1 << 16
            //job_thread_size /= 4;

            if (job_thread_size == 0)
                job_thread_size = 1;

            // sort blocks
#pragma omp parallel for
            for (int i = 0; i < size; i += job_thread_size) {
                int index_start = i;
                int index_end_exclusive = i + job_thread_size;
                if (index_end_exclusive > size)
                    index_end_exclusive = size;
                radix_counting_sort_unsigned_index(_array + index_start, index_end_exclusive - index_start, _aux + index_start);
            }

            // merge down the blocks
            IndexUInt32* in = _array;
            IndexUInt32* out = _aux;

            int element_count = job_thread_size;

            while (element_count < size) {

                //merge operation
#pragma omp parallel for
                for (int i = 0; i < size; i += (element_count << 1)) {

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

                //swap in/out
                IndexUInt32* aux = in;
                in = out;
                out = aux;

                element_count = element_count << 1;
            }

            if (in != _array)
                memcpy(_array, in, sizeof(IndexUInt32) * size);

            if (_pre_alloc_tmp == NULL)
                free_aligned(_aux);
        }


    }
}
