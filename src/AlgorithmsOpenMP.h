#ifndef algorithms__openMP__h__
#define algorithms__openMP__h__

#include <aRibeiroCore/common.h>
#include <aRibeiroCore/Algorithms.h>

namespace aRibeiro {
    namespace Sorting {

        void hybrid_bucket_std_signed_OpenMP(int32_t* A, uint32_t size);
        void hybrid_bucket_radix_counting_signed_OpenMP(int32_t* A, uint32_t size, int32_t* tmp_array = NULL);
        void hybrid_bucket_radix_counting_signed_index_OpenMP(IndexInt32* A, uint32_t size, IndexInt32* tmp_array = NULL);
        
        void hybrid_counting_std_signed_OpenMP(int32_t* A, uint32_t size, int32_t* tmp_array = NULL);
        void hybrid_counting_radix_counting_signed_OpenMP(int32_t* A, uint32_t size, int32_t* tmp_array = NULL);
        void hybrid_counting_radix_counting_signed_index_OpenMP(IndexInt32* A, uint32_t size, IndexInt32* tmp_array = NULL);

        void hybrid_merge_std_signed_OpenMP(int32_t* _array, uint32_t size, int32_t* pre_alloc_tmp = NULL);
        void hybrid_merge_radix_counting_signed_OpenMP(int32_t* _array, uint32_t size, int32_t* pre_alloc_tmp = NULL);
        void hybrid_merge_radix_counting_signed_index_OpenMP(IndexInt32* _array, uint32_t size, IndexInt32* pre_alloc_tmp = NULL);

        void hybrid_bucket_std_unsigned_OpenMP(uint32_t* A, uint32_t size);
        void hybrid_bucket_radix_counting_unsigned_OpenMP(uint32_t* A, uint32_t size, uint32_t* tmp_array = NULL);
        void hybrid_bucket_radix_counting_unsigned_index_OpenMP(IndexUInt32* A, uint32_t size, IndexUInt32* tmp_array = NULL);

        void hybrid_counting_std_unsigned_OpenMP(uint32_t* A, uint32_t size, uint32_t* tmp_array = NULL);
        void hybrid_counting_radix_counting_unsigned_OpenMP(uint32_t* A, uint32_t size, uint32_t* tmp_array = NULL);
        void hybrid_counting_radix_counting_unsigned_index_OpenMP(IndexUInt32* A, uint32_t size, IndexUInt32* tmp_array = NULL);

        void hybrid_merge_std_unsigned_OpenMP(uint32_t* _array, uint32_t size, uint32_t* pre_alloc_tmp = NULL);
        void hybrid_merge_radix_counting_unsigned_OpenMP(uint32_t* _array, uint32_t size, uint32_t* pre_alloc_tmp = NULL);
        void hybrid_merge_radix_counting_unsigned_index_OpenMP(IndexUInt32* _array, uint32_t size, IndexUInt32* pre_alloc_tmp = NULL);

    }
}

#endif