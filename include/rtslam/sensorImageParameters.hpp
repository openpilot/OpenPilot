/*
 * sensorImageParameters.hpp
 *
 *     Project: jafar
 *  Created on: Jun 17, 2010
 *      Author: jsola
 * \ingroup rtslam
 */

#ifndef SENSORIMAGEPARAMETERS_HPP_
#define SENSORIMAGEPARAMETERS_HPP_

#include "rtslam/pinholeTools.hpp"
#include "jmath/jblas.hpp"

namespace jafar{
	namespace rtslam{
		using namespace jblas;

		/**
		 * Structure containing all non-extrinsic parameters of a camera
		 * \ingroup rtslam
		 */
		struct SensorImageParameters {

			public:

				// Image size
				unsigned int width;
				unsigned int height;

				// intrinsic and distortion params
				vec4 intrinsic;
				vec distortion;
				vec correction;

				// sensor precision in pixels
				double pixNoise;

				// non observable specification
				double distMin;

				// detection and matching params
				unsigned int patchSize;

				void setImgSize(const int _width, const int _height){
					width=_width;
					height=_height;
				}

				/**
				 * Pin-hole sensor setup.
				 * \param k the vector of intrinsic parameters <c>k = [u_0, v_0, a_u, a_v]</c>.
				 * \param d the radial distortion parameters vector <c>d = [d_2, d_4, ...] </c>.
				 * \param c the radial distortion correction parameters vector <c>c = [c_2, c_4, ...] </c>.
				 */
				void setIntrinsicCalibration(const vec4 _k, const vec & _d, size_t size_c){
					intrinsic = _k;
					distortion.resize(_d.size());
					distortion = _d;
					correction.resize(size_c);
					pinhole::computeCorrectionModel(intrinsic, distortion, correction);
				}

				void setMiscellaneous(double _pixNoise,
				    double d_min){
					pixNoise = _pixNoise;
					distMin = d_min;
				}


		} ;
	}
}



#endif /* SENSORIMAGEPARAMETERS_HPP_ */
