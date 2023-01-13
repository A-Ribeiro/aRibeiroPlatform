#ifndef __RANDOM_INSTANTIABLE_THREAD_SAFE_H___
#define __RANDOM_INSTANTIABLE_THREAD_SAFE_H___

#include <aRibeiroCore/Random.h>
#include <aRibeiroPlatform/PlatformMutex.h>
#include <aRibeiroPlatform/PlatformAutoLock.h>

namespace aRibeiro {


	class RandomInstantiableThreadSafe {

		PlatformMutex mutex;
		RandomInstantiable random;

	public:

		/// \brief Set the initial generation seed of the randonlib.
		///
		/// The seed is initialized automatically using the system time as base.
		///
		/// If you set the seed to a specified value, the random sequence will always be the same.
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// Random::setSeed(time(NULL));
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		///
		ARIBEIRO_INLINE void setSeed(long int v) {
			PlatformAutoLock autoLock(&mutex);
			random.setSeed(v);
		}

		/// \brief Return double precision float value (double)
		///
		/// The returned value will be in range [0..1] ( including 0 and 1 ).
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// double value = Random::getDouble();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return range [0..1] ( including 0 and 1 )
		///
		ARIBEIRO_INLINE double getDouble() {
			PlatformAutoLock autoLock(&mutex);
			return random.getDouble();
		}

		/// \brief Return single precision float value (float)
		///
		/// The returned value will be in range [0..1] ( including 0 and 1 ).
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// float value = Random::getFloat();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return range [0..1] ( including 0 and 1 )
		///
		ARIBEIRO_INLINE float getFloat() {
			PlatformAutoLock autoLock(&mutex);
			return random.getFloat();
		}

		/// \brief Return random 2D vector (vec2)
		///
		/// All components will be in the range: [0..1] ( including 0 and 1 ).
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// vec2 value = Random::getVec2();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return random vec2, all components in range [0..1] ( including 0 and 1 )
		///
		ARIBEIRO_INLINE vec2 getVec2() {
			PlatformAutoLock autoLock(&mutex);
			return random.getVec2();
		}

		/// \brief Return random 3D vector (vec3)
		///
		/// All components will be in the range: [0..1] ( including 0 and 1 ).
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// vec3 value = Random::getVec3();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return random vec3, all components in range [0..1] ( including 0 and 1 )
		///
		ARIBEIRO_INLINE vec3 getVec3() {
			PlatformAutoLock autoLock(&mutex);
			return random.getVec3();
		}

		/// \brief Return random 3D vector with homogeneous coord (vec4)
		///
		/// All components will be in the range: [0..1] ( including 0 and 1 ).
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// vec4 value = Random::getVec4();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return random vec4, all components in range [0..1] ( including 0 and 1 )
		///
		ARIBEIRO_INLINE vec4 getVec4() {
			PlatformAutoLock autoLock(&mutex);
			return random.getVec4();
		}

		/// \brief Return random 4x4 matrix (mat4)
		///
		/// All components will be in the range: [0..1] ( including 0 and 1 ).
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// mat4 value = Random::getMat4();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return random mat4, all components in range [0..1] ( including 0 and 1 )
		///
		ARIBEIRO_INLINE mat4 getMat4() {
			PlatformAutoLock autoLock(&mutex);
			return random.getMat4();
		}

		/// \brief Return random quaternion (quat)
		///
		/// The quaternion is constructed from Euler angles.
		///
		/// All three Euler angles are in the range [0..360].
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// quat value = Random::getQuat();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return random valid rotation quaternion
		///
		ARIBEIRO_INLINE quat getQuat() {
			PlatformAutoLock autoLock(&mutex);
			return random.getQuat();
		}

		/// \brief Return random unit vector in a vec2 structure
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// vec2 value = Random::getVec2Direction();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return random unit vector in a vec2 structure
		///
		ARIBEIRO_INLINE vec2 getVec2Direction() {
			PlatformAutoLock autoLock(&mutex);
			return random.getVec2Direction();
		}

		/// \brief Return random unit vector in a vec3 structure
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// vec3 value = Random::getVec3Direction();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return random unit vector in a vec3 structure
		///
		ARIBEIRO_INLINE vec3 getVec3Direction() {
			PlatformAutoLock autoLock(&mutex);
			return random.getVec3Direction();
		}

		/// \brief Return random valid point representation (vec4 with w=1.0)
		///
		/// The x, y and z values are in range [0..1].
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// vec4 value = Random::getVec4Ptn();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return random valid point representation (vec4 with w=1.0)
		///
		ARIBEIRO_INLINE vec4 getVec4Ptn() {
			PlatformAutoLock autoLock(&mutex);
			return random.getVec4Ptn();
		}

		/// \brief Return random valid vector representation (vec4 with w=0.0)
		///
		/// The x, y and z values are in range [0..1].
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// vec4 value = Random::getVec4Vec();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return random valid vector representation (vec4 with w=0.0)
		///
		ARIBEIRO_INLINE vec4 getVec4Vec() {
			PlatformAutoLock autoLock(&mutex);
			return random.getVec4Vec();
		}

		/// \brief Return random unit vector in a vec4 (w = 0.0)
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// vec4 value = Random::getVec4VecDirection();
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \return random unit vector in a vec4 (w = 0.0)
		///
		ARIBEIRO_INLINE vec4 getVec4VecDirection() {
			PlatformAutoLock autoLock(&mutex);
			return random.getVec4VecDirection();
		}

		/// \brief Return integer range random
		///
		/// The min and max are included in random result.
		///
		/// Example:
		///
		/// \code
		/// #include <aRibeiroCore/aRibeiroCore.h>
		/// using namespace aRibeiro;
		///
		/// // valid index positions: [0..5]
		/// int array[6];
		///
		/// int random_index = Random::getRange(0,5);
		///
		/// array[random_index] = ...;
		/// \endcode
		///
		/// \author Alessandro Ribeiro
		/// \param min integer range min (included in random)
		/// \param max integer range max (included in random)
		/// \return random integer range random, include min and max.
		///
		ARIBEIRO_INLINE int getRange(int min, int max) {
			PlatformAutoLock autoLock(&mutex);
			return random.getRange(min, max);
		}

		ARIBEIRO_INLINE vec3 getVec3_uvw() {
			PlatformAutoLock autoLock(&mutex);
			return random.getVec3_uvw();
		}

		ARIBEIRO_INLINE vec3 getVec3PointInsideTriangle(const vec3& a, const vec3& b, const vec3& c) {
			PlatformAutoLock autoLock(&mutex);
			return random.getVec3PointInsideTriangle(a, b, c);
		}

	};


}

#endif

