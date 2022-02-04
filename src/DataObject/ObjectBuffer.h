#ifndef __object_buffer__H__
#define __object_buffer__H__

//#include <aRibeiroPlatform/aribeiro.h>
//#include "colorspace_ops.h"

#include <aRibeiroCore/common.h>

namespace aRibeiro {

    //
    // This Object Buffer have the default align to 32 bytes
    //   supporting 256 bits instructions ( All SSE and AVX/AVX2 ).
    //
	class ObjectBuffer {

		//private copy constructores, to avoid copy...
        ObjectBuffer(const ObjectBuffer& v){}
        void operator=(const ObjectBuffer& v){}

		bool constructed_from_external_buffer;
	public:
		uint8_t *data;
		size_t alloc_size;
		size_t size;
        size_t align;

		ObjectBuffer(uint8_t *data, size_t _size, int _align = 32) {
			data = data;
			size = _size;
            align = _align;
			alloc_size = 0;
			constructed_from_external_buffer = true;
		}

		ObjectBuffer() {
			data = NULL;
			size = 0;
			alloc_size = 0;
            align = 32;
			constructed_from_external_buffer = false;
		}

		virtual ~ObjectBuffer() {
			free();
		}

		ObjectBuffer* setSize(size_t _size, int _align = 32) {
			if (_size == size)
				return this;
			
			if (_size > alloc_size) {
				free();
				data = (uint8_t*)malloc_aligned(_size, _align);
				alloc_size = _size;
                align = _align;
				//constructed_from_external_buffer = false;
			}

			size = _size;

			return this;
		}

        ObjectBuffer* copy(const ObjectBuffer* src) {
            setSize(src->size, src->align);
            memcpy(data, src->data,size);

            return this;
        }

		ObjectBuffer* free() {
			if (!constructed_from_external_buffer && data != NULL) {
				free_aligned(data);
			}

			data = NULL;
			size = 0;
			alloc_size = 0;
			constructed_from_external_buffer = false;

			return this;
		}

	};
}

#endif

