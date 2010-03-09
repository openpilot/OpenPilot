/**
 * sensorPinHole.hpp
 *
 *  Created on: 14/02/2010
 *      Author: jsola
 *
 * \ingroup rtslam
 */

#ifndef SENSORPINHOLE_HPP_
#define SENSORPINHOLE_HPP_

#include "rtslam/sensorAbstract.hpp"
#include "rtslam/gaussian.hpp"
#include "iostream"

namespace jafar {
	namespace rtslam {

		/**
		 * Class for pin hole parameters
		 * \ingroup rtslam
		 */
		class ParametersPinHole: public ParametersAbstract {
			public:
				jblas::vec4 intrinsic;
				jblas::vec distortion;
				jblas::vec correction;

				inline ParametersPinHole(const jblas::vec4 & _k, const jblas::vec & _d, const jblas::vec & _c) :
					intrinsic(_k), distortion(_d), correction(_c) {
				}

				inline ParametersPinHole(const jblas::vec4 & _k) :
					intrinsic(_k) {
				}

				inline ~ParametersPinHole(void) {
				}
		};

		/**
		 * Class for rectangular pixels image
		 * \ingroup rtslam
		 */
		class Image: public RawAbstract {
			public:
				jblas::mati pixels;
				size_t H_size;
				size_t V_size;

				inline Image(void) {
				}

				inline Image(size_t H, size_t V) :
					pixels(H, V), H_size(H), V_size(V) {
				}

				inline ~Image(void) {
				}

				inline size_t size1(void) {
					return pixels.size1();
				}

				inline size_t size2(void) {
					return pixels.size2();
				}

				inline void acquire(void) {
				}

		};

		/**
		 * Class for pin-hole cameras.
		 * This model accepts radial distortion model
		 * \ingroup rtslam
		 */
		class SensorPinHole: public SensorAbstract {
			public:
				ParametersPinHole parameters;
				Image image;

				/**
				 * Constructor from Gaussian pose and params
				 */
				inline SensorPinHole(Gaussian & _pose, const jblas::vec4 & k, const jblas::vec & d, const jblas::vec & c,
				    const size_t hsize, const size_t vsize) :
					SensorAbstract(_pose), parameters(k, d, c), image(hsize, vsize) {
					type = "Pin-hole-camera";
				}

				/**
				 * Constructor from mean pose and params
				 */
				inline SensorPinHole(const jblas::vec & _pose, const jblas::vec4 & k, const jblas::vec & d, const jblas::vec & c,
				    const size_t hsize, const size_t vsize) :
					SensorAbstract(_pose), parameters(k, d, c), image(hsize, vsize) {
					type = "Pin-hole-camera";
				}

				/**
				 * Construction from map, indirect array and params
				 */
				inline SensorPinHole(MapAbstract & map, const jblas::ind_array & ias, const jblas::vec4 & k,
				    const jblas::vec & d, const jblas::vec & c, const size_t hsize, const size_t vsize) :
					SensorAbstract(map, ias), parameters(k, d, c), image(hsize, vsize) {
					type = "Pin-hole-camera";
				}

			private:
		};

	}
}

#endif /* SENSORPINHOLE_HPP_ */
