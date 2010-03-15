/**
 * \file sensorPinHole.hpp
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

				inline ParametersPinHole(void) {
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
				 * Constructor from Gaussian pose.
				 * \param _pose the Gaussian pose.
				 */
				SensorPinHole(const Gaussian & _pose);

				/**
				 * Constructor from mean pose.
				 * \param _pose the pose 7-vector.
				 */
				SensorPinHole(const jblas::vec7 & _pose);

				/**
				 * Construction from map and indirect array.
				 * \param map the map.
				 * \param ias the indirect array pointing to the map states.
				 */
				SensorPinHole(MapAbstract & map, const jblas::ind_array & ias);

				/**
				 * Pin-hole sensor setup.
				 * \param k the vector of intrinsic parameters <c>k = [u_0, v_0, a_u, a_v]</c>.
				 * \param d the radial distortion parameters vector <c>d = [d_2, d_4, ...] </c>.
				 * \param c the radial distortion correction parameters vector <c>c = [c_2, c_4, ...] </c>.
				 * \param hsize the horizontal image size.
				 * \param vsize the vertical image size.
				 */
				void set_parameters(const jblas::vec4 & k, const jblas::vec & d, const jblas::vec & c, const size_t hsize,
				    const size_t vsize);

				static size_t size(void) {
					return 7;
				}



			private:
		};

	}
}

#endif /* SENSORPINHOLE_HPP_ */
