/**
 * \file pinholeTools.hpp
 *
 * \date 06/04/2010
 * \author jsola
 *
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

#ifndef PINHOLE_HPP_
#define PINHOLE_HPP_

#include "jmath/jblas.hpp"
#include "jmath/linearSolvers.hpp"
#include "jmath/ublasExtra.hpp"

#include "jmath/matlab.hpp"

namespace jafar {
	namespace rtslam {

		using namespace jblas;


		/**
		 * Namespace for operations related to the pin-hole model of a camera.
		 *
		 * The model incorporates radial distortion.
		 *
		 * \ingroup rtslam
		 */
		namespace pinhole {


			/**
			 * Pin-hole canonical projection.
			 * \return the projected point in the normalized 2D plane
			 * \param v a 3D point
			 */
			template<class V>
			vec2 projectPointToNormalizedPlane(const V & v) {

				vec2 up;
				up(0) = v(0) / v(2);
				up(1) = v(1) / v(2);
				return up;
			}

			/**
			 * Pin-hole canonical projection. Give distance.
			 * \param v a 3D point
			 * \param up the projected point in the normalized 2D plane
			 * \param dist the distance from the camera to the point
			 */
			template<class V, class P>
			void projectPointToNormalizedPlane(const V & v, P & up, double & dist) {

				up(0) = v(0) / v(2);
				up(1) = v(1) / v(2);
				dist = ublas::norm_2(v);
			}


			/**
			 * Pin-hole canonical projection, with jacobian
			 * \param v the 3D point to project
			 * \param up the projected 2D point
			 * \param UP_v the Jacibian of \a u wrt \a v
			 */
			template<class V, class U, class MU_v>
			void projectPointToNormalizedPlane(const V & v, U & up, MU_v & UP_v) {

				up = projectPointToNormalizedPlane(v);

				UP_v(0, 0) = 1.0 / v(2);
				UP_v(0, 1) = 0.0;
				UP_v(0, 2) = -v(0) / (v(2) * v(2));
				UP_v(1, 0) = 0.0;
				UP_v(1, 1) = 1.0 / v(2);
				UP_v(1, 2) = -v(1) / (v(2) * v(2));

			}

			/**
			 * Pin-hole canonical projection, distance and with jacobian
			 * \param v the 3D point to project
			 * \param up the projected 2D point
			 * \param dist the distance from the camera to the point
			 * \param UP_v the Jacibian of \a u wrt \a v
			 */
			template<class V, class U, class MU_v>
			void projectPointToNormalizedPlane(const V & v, U & up, double & dist, MU_v & UP_v) {

				projectPointToNormalizedPlane(v, up, dist);

				UP_v(0, 0) = 1.0 / v(2);
				UP_v(0, 1) = 0.0;
				UP_v(0, 2) = -v(0) / (v(2) * v(2));
				UP_v(1, 0) = 0.0;
				UP_v(1, 1) = 1.0 / v(2);
				UP_v(1, 2) = -v(1) / (v(2) * v(2));

			}

			/**
			 * Canonical back-projection.
			 * \param u the 2D point in the image plane
			 * \param depth point's depth orthogonal to image plane. Defaults to 1.0
			 * \return the 3D point
			 */
			template<class U>
			vec3 backprojectPointFromNormalizedPlane(const U & u, double depth = 1) {

				vec3 p;
				p(0) = depth * u(0);
				p(1) = depth * u(1);
				p(2) = depth;
				return p;
			}

			/**
			 * Canonical back-projection.
			 * \param u the 2D point in the image plane.
			 * \param depth point's depth orthogonal to image plane.
			 * \param p the 3D point.
			 * \param P_u Jacobian of p wrt u.
			 * \param P_depth Jacobian of p wrt depth.
			 */
			template<class U, class P, class MP_u, class MP_depth>
			void backprojectPointFromNormalizedPlane(const U & u, const double depth, P & p, MP_u & P_u, MP_depth & P_depth) {
				p = backprojectPointFromNormalizedPlane(u, depth);

				P_u(0, 0) = depth;
				P_u(0, 1) = 0.0;
				P_u(1, 0) = 0.0;
				P_u(1, 1) = depth;
				P_u(2, 0) = 0.0;
				P_u(2, 1) = 0.0;

				P_depth(0, 0) = u(0);
				P_depth(1, 0) = u(1);
				P_depth(2, 0) = 1.0;
			}

			/**
			 * Distortion factor for the model s = 1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...
			 * \param r2 the square of the radius to evaluate, r2 = r^2.
			 * \return the distortion factor so that rd = s*r
			 */
			template<class VD>
			double distortFactor(const VD & d, double r2){
				if (d.size() == 0) return 1.0;
				double s = 1.0;
				double r2i = 1.0;
				for (size_t i = 0; i < d.size(); i++) { //   here we are doing:
					r2i = r2i * r2; //                    r2i = r^(2*(i+1))
					s += d(i) * r2i; //                   s = 1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...
				}
				/*
					The model is not valid out of the image, and it can bring back landmarks very quickly after they got out.
					We should compute the point where the monotony changes by solving d/du u.f(u,v) = 0, but it would require a lot
					more computations to check the validity of the transform than to compute the distortion... and it should be done
					manually for every size of d (fortunately only 2 or 3 usually).
					So we just make a rough test on s, no camera should have a distortion greater than this, and anyway it will just
					prevent from observing some points on the borders of the image. In that case the landmark will seem to suddenly
					jump outside of the image.
				*/
				if (s < 0.6) s = 1.0;
				return s;
			}


			/**
			 * Radial distortion: ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + etc) * u
			 * \param d the distortion parameters vector
			 * \param up the point to distort
			 * \return the distorted point
			 */
			template<class VD, class VU>
			jblas::vec2 distortPoint(const VD & d, const VU & up) {
				size_t n = d.size();
				if (n == 0)
					return up;
				else {
					double r2 = up(0) * up(0) + up(1) * up(1); // this is the norm squared: r2 = ||u||^2
					return distortFactor(d, r2) * up;
//					double s = 1.0;
//					double r2i = 1.0;
//					for (size_t i = 0; i < n; i++) { //   here we are doing:
//						r2i = r2i * r2; //                    r2i = r^(2*(i+1))
//						s += d(i) * r2i; //                   s = 1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...
//					}
//					return s * up; //                     finally: ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...) * u;
				}
			}


			/**
			 * Radial distortion: ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + etc) * u, with jacobians
			 * \param d the radial distortion parameters vector
			 * \param up the point to distort
			 * \param ud the distorted point
			 * \param UD_up the Jacobian of \a ud wrt \a up
			 */
			template<class VD, class VUp, class VUd, class MUD_up>
			void distortPoint(const VD & d, const VUp & up, VUd & ud, MUD_up & UD_up) {
				size_t n = d.size();
				jblas::vec2 R2_up;
				jblas::vec2 S_up;

				if (n == 0) {
					ud = up;
					UD_up = jblas::identity_mat(2);
				}

				else {
					double r2 = up(0) * up(0) + up(1) * up(1); // this is the norm squared: r2 = ||u||^2
					double s = 1.0;
					double r2i = 1.0;
					double r2im1 = 1.0; //r2*(i-1)
					double S_r2 = 0.0;

					for (size_t i = 0; i < n; i++) { //.. here we are doing:
						r2i = r2i * r2; //................. r2i = r^(2*(i+1))
						s += d(i) * r2i; //................ s = 1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...

						S_r2 = S_r2 + (i + 1) * d(i) * r2im1; //jacobian of s wrt r2 : S_r2 = d_0 + 2 * d1 * r^2 + 3 * d_2 * r^4 +  ...
						r2im1 = r2im1 * r2;
					}

					if (s < 0.5) s = 1.0; // because the model is not valid too much out of the image, avoid to wrongly bring them back in the field of view
					ud = s * up; // finally ud = (1 + d_0 * r^2 + d_1 * r^4 + d_2 * r^6 + ...) * u;

					R2_up(0) = 2 * up(0);
					R2_up(1) = 2 * up(1);

					S_up(0) = R2_up(0) * S_r2;
					S_up(1) = R2_up(1) * S_r2;

					UD_up(0, 0) = S_up(0) * up(0) + s;
					UD_up(0, 1) = S_up(1) * up(0);
					UD_up(1, 0) = S_up(0) * up(1);
					UD_up(1, 1) = S_up(1) * up(1) + s;
				}

			}
			template<class VC, class VU>
			jblas::vec2 undistortPoint(const VC & c, const VU & ud) {
				size_t n = c.size();
				if (n == 0)
					return ud;
				else {
					double r2 = ud(0) * ud(0) + ud(1) * ud(1); // this is the norm squared: r2 = ||u||^2
					return distortFactor(c, r2) * ud;
//					double s = 1.0;
//					double r2i = 1.0;
//					for (size_t i = 0; i < n; i++) { //   here we are doing:
//						r2i = r2i * r2; //                    r2i = r^(2*(i+1))
//						s += c(i) * r2i; //                   s = 1 + c_0 * r^2 + c_1 * r^4 + c_2 * r^6 + ...
//					}
//					return s * ud; //                     finally: up = (1 + c_0 * r^2 + c_1 * r^4 + c_2 * r^6 + ...) * u;
				}
			}

			template<class VC, class VUd, class VUp, class MUP_ud>
			void undistortPoint(const VC & c, const VUd & ud, VUp & up, MUP_ud & UP_ud) {
				size_t n = c.size();
				jblas::vec2 R2_ud;
				jblas::vec2 S_ud;

				if (n == 0) {
					up = ud;
					UP_ud = jblas::identity_mat(2);
				}

				else {
					double r2 = ud(0) * ud(0) + ud(1) * ud(1); // this is the norm squared: r2 = ||u||^2
					double s = 1.0;
					double r2i = 1.0;
					double r2im1 = 1.0; //r2*(i-1)
					double S_r2 = 0.0;

					for (size_t i = 0; i < n; i++) { //.. here we are doing:
						r2i = r2i * r2; //................. r2i = r^(2*(i+1))
						s += c(i) * r2i; //................ s = 1 + c_0 * r^2 + c_1 * r^4 + c_2 * r^6 + ...

						S_r2 = S_r2 + (i + 1) * c(i) * r2im1; //jacobian of s wrt r2 : S_r2 = c_0 + 2 * d1 * r^2 + 3 * c_2 * r^4 +  ...
						r2im1 = r2im1 * r2;
					}

					up = s * ud; // finally up = (1 + c_0 * r^2 + c_1 * r^4 + c_2 * r^6 + ...) * u;

					R2_ud(0) = 2 * ud(0);
					R2_ud(1) = 2 * ud(1);

					S_ud(0) = R2_ud(0) * S_r2;
					S_ud(1) = R2_ud(1) * S_r2;

					UP_ud(0, 0) = S_ud(0) * ud(0) + s;
					UP_ud(0, 1) = S_ud(1) * ud(0);
					UP_ud(1, 0) = S_ud(0) * ud(1);
					UP_ud(1, 1) = S_ud(1) * ud(1) + s;
				}

			}


			/**
			 * Pixellization from k = [u_0, v_0, a_u, a_v]
			 * \param k the vector of intrinsic parameters, k = [u0, v0, au, av]
			 * \param ud the point to pixellize, adimensional
			 * \return the point in pixels coordinates
			 */
			template<class VK, class VU>
			jblas::vec2 pixellizePoint(const VK & k, const VU & ud) {
				double u_0 = k(0);
				double v_0 = k(1);
				double a_u = k(2);
				double a_v = k(3);
				vec2 u;
				u(0) = u_0 + a_u * ud(0);
				u(1) = v_0 + a_v * ud(1);
				return u;
			}


			/**
			 * Pixellization from k = [u_0, v_0, a_u, a_v] with jacobians
			 * \param k the vector of intrinsic parameters, k = [u0, v0, au, av]
			 * \param ud the point to pixellize, adimensional
			 * \param u the pixellized point
			 * \param U_ud the Jacobian of \a u wrt \a ud
			 */
			template<class VK, class VUd, class VU, class MU_ud>
			void pixellizePoint(const VK & k, const VUd & ud, VU & u, MU_ud & U_ud) {
				//double u_0 = k(0);
				//double v_0 = k(1);
				double a_u = k(2);
				double a_v = k(3);

				u = pixellizePoint(k, ud);

				U_ud(0, 0) = a_u;
				U_ud(0, 1) = 0;
				U_ud(1, 0) = 0;
				U_ud(1, 1) = a_v;
			}


			/**
			 * Depixellization from k = [u_0, v_0, a_u, a_v]
			 * \param k the vector of intrinsic parameters, k = [u0, v0, au, av]
			 * \param u the point to depixellize, in pixels
			 * \return the depixellized point, adimensional
			 */
			template<class VK, class VU>
			jblas::vec2 depixellizePoint(const VK & k, const VU & u) {
				double u_0 = k(0);
				double v_0 = k(1);
				double a_u = k(2);
				double a_v = k(3);
				vec2 ud;
				ud(0) = (u(0) - u_0) / a_u;
				ud(1) = (u(1) - v_0) / a_v;
				return ud;
			}


			/**
			 * Depixellization from k = [u_0, v_0, a_u, a_v], with Jacobians
			 * \param k the vector of intrinsic parameters, k = [u0, v0, au, av]
			 * \param u the point to depixellize, in pixels
			 * \param ud the depixellized point
			 * \param UD_u the Jacobian of \a ud wrt \a u
			 */
			template<class VK, class VUd, class VU, class MUD_u>
			void depixellizePoint(const VK & k, const VU & u, VUd & ud, MUD_u & UD_u) {
				//				double u_0 = k(0);
				//				double v_0 = k(1);
				double a_u = k(2);
				double a_v = k(3);

				ud = depixellizePoint(k, u);

				UD_u(0, 0) = 1.0 / a_u;
				UD_u(0, 1) = 0.0;
				UD_u(1, 0) = 0.0;
				UD_u(1, 1) = 1.0 / a_v;
			}


			/**
			 * Project a point into a pin-hole camera with radial distortion
			 * \param k the vector of intrinsic parameters, k = [u0, v0, au, av]
			 * \param d the radial distortion parameters vector
			 * \param v the 3D point to project, or the 3D director vector
			 * \return the projected and distorted point
			 */
			template<class VK, class VD, class V>
			vec2 projectPoint(const VK & k, const VD & d, const V & v) {
				return pixellizePoint(k, distortPoint(d, projectPointToNormalizedPlane(v)));
			}

			/**
			 * Project a point into a pin-hole camera with radial distortion.
			 * \param k the vector of intrinsic parameters, k = [u0, v0, au, av]
			 * \param d the radial distortion parameters vector
			 * \param v the 3D point to project, or the 3D director vector
			 * \param u the projected and distorted point
			 */
			template<class VK, class VD, class V, class U>
			void projectPoint(const VK & k, const VD & d, const V & v, U & u, double & dist) {
				vec2 up;
				projectPointToNormalizedPlane(v, up, dist);
				u = pixellizePoint(k, distortPoint(d, up));
			}


			/**
			 * Project a point into a pin-hole camera with radial distortion
			 * \param k the vector of intrinsic parameters, k = [u0, v0, au, av]
			 * \param d the radial distortion parameters vector
			 * \param v the 3D point to project, or the 3D director vector
			 * \param u the projected and distorted point
			 * \param U_v the Jacobian of \a u wrt \a v
			 */
			template<class VK, class VD, class V, class VU, class MU_v>
			void projectPoint(const VK & k, const VD & d, const V & v, VU & u, MU_v & U_v) {
				vec2 up, ud;
				mat23 UP_v;
				mat22 UD_up, U_ud;
				projectPointToNormalizedPlane(v, up, UP_v);
				distortPoint(d, up, ud, UD_up);
				pixellizePoint(k, ud, u, U_ud);

				mat23 U_v1;
				U_v1 = ublas::prod(UD_up, UP_v);
				U_v = ublas::prod(U_ud, U_v1);
			}

			/**
			 * Project a point into a pin-hole camera with radial distortion
			 * \param k the vector of intrinsic parameters, k = [u0, v0, au, av]
			 * \param d the radial distortion parameters vector
			 * \param v the 3D point to project, or the 3D director vector
			 * \param u the projected and distorted point
			 * \param dist the distance from the camera to the point
			 * \param U_v the Jacobian of \a u wrt \a v
			 */
			template<class VK, class VD, class V, class VU, class MU_v>
			void projectPoint(const VK & k, const VD & d, const V & v, VU & u, double & dist, MU_v & U_v) {
				vec2 up, ud;
				mat23 UP_v;
				mat22 UD_up, U_ud;
				projectPointToNormalizedPlane(v, up, dist, UP_v);
				distortPoint(d, up, ud, UD_up);
				pixellizePoint(k, ud, u, U_ud);

				mat23 U_v1;
				U_v1 = ublas::prod(UD_up, UP_v);
				U_v = ublas::prod(U_ud, U_v1);
			}


			/**
			 * Back-Project a point from a pin-hole camera with radial distortion
			 * \param k the vector of intrinsic parameters, k = [u0, v0, au, av]
			 * \param c the radial undistortion parameters vector
			 * \param u the 2D pixel
			 * \param depth the depth prior
			 * \return the back-projected 3D point
			 */
			template<class VK, class VC, class U>
			vec3 backprojectPoint(const VK & k, const VC & c, const U & u, const double depth = 1.0) {
				return backprojectPointFromNormalizedPlane(undistortPoint(c, depixellizePoint(k, u)), depth);
			}

			/**
			 * Back-Project a point from a pin-hole camera with radial distortion; give Jacobians
			 * \param k the vector of intrinsic parameters, k = [u0, v0, au, av]
			 * \param c the radial undistortion parameters vector
			 * \param u the 2D pixel
			 * \param depth the depth prior
			 * \param p the back-projected 3D point
			 * \param P_u Jacobian of p wrt u
			 * \param P_depth Jacobian of p wrt depth
			 */
			template<class VK, class VC, class U, class P, class MP_u, class MP_depth>
			void backProjectPoint(const VK & k, const VC & c, const U & u, double depth, P & p, MP_u & P_u, MP_depth & P_depth) {
				vec2 up, ud;
				mat32 P_up;
				mat22 UP_ud, UD_u;
				depixellizePoint(k, u, ud, UD_u);
				undistortPoint(c, ud, up, UP_ud);
				backprojectPointFromNormalizedPlane(up, depth, p, P_up, P_depth);

				P_u = ublas::prod(P_up, ublas::prod<mat>(UP_ud, UD_u));
			}


			/**
			 * Determine if a pixel is inside the region of interest
			 * \param pix the pixel to test
			 * \param roi the region of interest, in pixels
			 */
			template<class VPix>
			bool isInRoi(const VPix & pix, const int x, const int y, const int width, const int height) {
				return ((pix(0) >= x) && (pix(0) <= x + width - 1) && (pix(1) >= y) && (pix(1) <= y + height - 1));
			}

			/**
			 * Determine if a pixel is inside the image
			 * \param pix the pixel to test
			 * \param width the image width, in pixels
			 * \param height the image height, in pixels
			 */
			template<class VPix>
			bool isInImage(const VPix & pix, const int & width, const int & height) {
				return isInRoi(pix, 0, 0, width, height);
			}


			/**
			 * Compute distortion correction parameters.
			 *
			 * This method follows the one in Joan Sola's thesis [1], pag 46--49.
			 *
			 * \param size: the size of the desired correction vector.
			 */
			template<class Vk, class Vd, class Vc>
			void computeCorrectionModel(const Vk & k, const Vd & d, Vc & c) {
				size_t size = c.size();

				if (size != 0) {

					double r_max = sqrt(k(0)*k(0) / (k(2)*k(2)) + k(1)*k(1) / (k(3)*k(3)));
					double rd_max = 1.1 * r_max;

					size_t N_samples = 200; // number of samples
					double iN_samples = 1 / (double) N_samples;
					double rd_n, rc_2, rd_2;
					vec rd(N_samples+1), rc(N_samples+1);
					mat Rd(N_samples+1, size);


					for (size_t sample = 0; sample <= N_samples; sample++) {

						rc(sample) = sample * rd_max * iN_samples; // sample * rd_max / N_samples
						rc_2 = rc(sample) * rc(sample);
						rd(sample) = distortFactor(d, rc_2) * rc(sample);
						rd_2 = rd(sample) * rd(sample);

						rd_n = rd(sample); // start with rd

						for (size_t order = 0; order < size; order++) {
							rd_n *= rd_2; // increment:
							Rd(sample, order) = rd_n; // we have : rd^3, rd^5, rd^7, ...
						}
					}


					// solve Rd*c = (rc-rd) for c, with least-squares SVD method:
					// the result is c = pseudo_inv(Rd)*(rc-rd)
					//  with pseudo_inv(Rd) = (Rd'*Rd)^-1 * Rd'

					// this does not work:
					// jmath::LinearSolvers::solve_Cholesky(Rd, (rc - rd), c);

					// therefore we solve manually the pseudo-inverse:
					jblas::mat RdtRd(size,size);
					RdtRd = ublas::prod(ublas::trans(Rd), Rd);
					jblas::mat iRdtRd(size, size);
					jmath::ublasExtra::inv(RdtRd, iRdtRd);
					mat iRd = ublas::prod(iRdtRd, ublas::trans<jblas::mat>(Rd));

					c = ublas::prod(iRd,(rc-rd));
				}
			}

		} // namespace pinhole

	} // namespace rtslam
} // namespace jafar

#endif /* PINHOLE_HPP_ */
