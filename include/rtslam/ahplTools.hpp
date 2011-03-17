/**
 * \file ahplTools.hpp
 *
 * \date 28/02/2011
 * \author bhautboi
 *
 *
 *  This file defines the namespace landmarkAHPL in jafar::rtslam.
 *
 * \ingroup rtslam
 */

#include "jmath/misc.hpp"
#include "rtslam/quatTools.hpp"

#ifndef AHPLTOOLS_HPP
#define AHPLTOOLS_HPP

namespace jafar {
   namespace rtslam {
      /**
       * Namespace for operations on Anchored Homogeneous Points Lines.
       * \ingroup rtslam
       */
      namespace lmkAHPL {
         using namespace ublas;
         using namespace jblas;
         using namespace quaternion;


         /**
          * Split AHPL.
          * \param p0 the output anchor.
          * \param m1 the first output director vector
          * \param m2 the second output director vector
          * \param rho1 the first homogeneous parameter (inverse-distance)
          * \param rho2 the second homogeneous parameter (inverse-distance)
          */
         template<class AHPL, class P0, class M>
         void split(const AHPL & ahpl, P0 & p0, M & m1, M & m2, double & rho1, double & rho2) {
            p0 = project(ahpl, range(0, 3));
            m1 = project(ahpl, range(3, 6));
            rho1 = ahpl(6);
            m2 = project(ahpl, range(7, 10));
            rho2 = ahpl(10);
         }


         /**
          * Compose AHP.
          *
          * This is the opposite as split()
          * \param m1 the first output director vector
          * \param m2 the second output director vector
          * \param rho1 the first homogeneous parameter (inverse-distance)
          * \param rho2 the second homogeneous parameter (inverse-distance)
          */
         template<class P0, class M>
         vec11 compose(const P0 & p0, const M & m1, const M & m2, const double rho1, const double rho2) {
            vec11 ahpl;
            project(ahpl, range(0, 3)) = p0;
            project(ahpl, range(3, 6)) = m1;
            ahpl(6) = rho1;
            project(ahpl, range(7, 10)) = m2;
            ahpl(10) = rho2;
            return ahpl;
         }

         template<class VF, class Vlf>
         vec fromFrame(const VF & F, const Vlf & ahplf) {


            // split non-trivial chunks of landmark state
            vec3 p0f(subrange(ahplf, 0, 3));
            vec3 mf1(subrange(ahplf, 3, 6));
            vec3 mf2(subrange(ahplf, 7, 10));


            // transformed landmark in global frame
            vec ahpl(11);

            subrange(ahpl, 0, 3) = eucFromFrame(F, p0f);
            subrange(ahpl, 3, 6) = vecFromFrame(F, mf1);
            subrange(ahpl, 7, 10) = vecFromFrame(F, mf2);
            ahpl(6) = ahplf(6);
            ahpl(10) = ahplf(10);

            return ahpl;
         }

         template<class VF, class Vahplf, class Vahpl, class MAHPL_f, class MAHPL_ahplf>
         void fromFrame(const VF & F, const Vahplf & ahplf, Vahpl & ahpl, MAHPL_f & AHPL_f, MAHPL_ahplf & AHPL_ahplf) {
            // split non-trivial chunks of landmark state
            vec3 p0f(subrange(ahplf, 0, 3));
            vec3 mf1(subrange(ahplf, 3, 6));
            vec3 mf2(subrange(ahplf, 7, 10));
            // destination chunks
            vec3 p0;
            vec3 m1;
            vec3 m2;
            mat JAC_33(3, 3), JAC_37(3, 7);
            // Jacobians
            AHPL_f.clear();
            AHPL_ahplf.clear();
            // transform p0
            eucFromFrame(F, p0f, p0, JAC_37, JAC_33);
            subrange(ahpl, 0, 3) = p0;
            subrange(AHPL_f, 0, 3, 0, 7) = JAC_37;
            subrange(AHPL_ahplf, 0, 3, 0, 3) = JAC_33;
            // transform m1
            vecFromFrame(F, mf1, m1, JAC_37, JAC_33);
            subrange(ahpl, 3, 6) = m1;
            subrange(AHPL_f, 3, 6, 0, 7) = JAC_37;
            subrange(AHPL_ahplf, 3, 6, 3, 6) = JAC_33;
            // transform m2
            vecFromFrame(F, mf2, m2, JAC_37, JAC_33);
            subrange(ahpl, 7, 10) = m2;
            subrange(AHPL_f, 7, 10, 0, 7) = JAC_37;
            subrange(AHPL_ahplf, 7, 10, 7, 10) = JAC_33;
            // transform rho1
            ahpl(6) = ahplf(6);
            AHPL_ahplf(6, 6) = 1;
            // transform rho2
            ahpl(10) = ahplf(10);
            AHPL_ahplf(10, 10) = 1;
         }

         template<class VF, class Vahpl>
         vec toFrame(const VF & F, const Vahpl & ahpl) {
            // split non-trivial chunks of landmark state
            vec3 p0(subrange(ahpl, 0, 3));
            vec3 m1(subrange(ahpl, 3, 6));
            vec3 m2(subrange(ahpl, 7, 10));
            // transformed landmark in frame F
            vec ahplf(11);
            subrange(ahplf, 0, 3) = eucToFrame(F, p0);
            subrange(ahplf, 3, 6) = vecToFrame(F, m1);
            ahplf(6) = ahpl(6);
            subrange(ahplf, 7, 10) = vecToFrame(F, m2);
            ahplf(10) = ahpl(10);

            return ahplf;
         }

         template<class VF, class Vahpl, class Vahplf, class MAHPLF_f, class MAHPLF_ahp>
         void toFrame(const VF & F, const Vahpl & ahpl, Vahplf & ahplf, MAHPLF_f & AHPLF_f, MAHPLF_ahp & AHPLF_ahpl) {
            // split non-trivial chunks of landmark state
            vec3 p0(subrange(ahpl, 0, 3));
            vec3 m1(subrange(ahpl, 3, 6));
            vec3 m2(subrange(ahpl, 7, 10));
            // destination chunks
            vec3 p0f;
            vec3 mf1;
            vec3 mf2;
            mat JAC_33(3, 3), JAC_37(3, 7);
            // Jacobians
            AHPLF_f.clear();
            AHPLF_ahpl.clear();
            // transform p0
            eucToFrame(F, p0, p0f, JAC_37, JAC_33);
            subrange(ahplf, 0, 3) = p0f;
            subrange(AHPLF_f, 0, 3, 0, 7) = JAC_37;
            subrange(AHPLF_ahpl, 0, 3, 0, 3) = JAC_33;
            // transform m1
            vecToFrame(F, m1, mf1, JAC_37, JAC_33);
            subrange(ahplf, 3, 6) = mf1;
            subrange(AHPLF_f, 3, 6, 0, 7) = JAC_37;
            subrange(AHPLF_ahpl, 3, 6, 3, 6) = JAC_33;
            // transform m2
            vecToFrame(F, m2, mf2, JAC_37, JAC_33);
            subrange(ahplf, 7, 10) = mf2;
            subrange(AHPLF_f, 7, 10, 0, 7) = JAC_37;
            subrange(AHPLF_ahpl, 7, 10, 7, 10) = JAC_33;
            // transform rho1
            ahplf(6) = ahpl(6);
            AHPLF_ahpl(6, 6) = 1;
            // transform rho2
            ahplf(10) = ahpl(10);
            AHPLF_ahpl(10, 10) = 1;
         }


         /**
          * Reparametrize to Euclidean.
          * \param ahp the anchored homogeneous point to be reparametrized.
          * \return the Euclidean points.
          */
         template<class VA>
         vec6 ahpl2euc(const VA & ahpl) {
            vec6 ret;
            subrange(ret,0,3) = subrange(ahpl, 0, 3) + subrange(ahpl, 3, 6) / ahpl(6);
            subrange(ret,3,6) = subrange(ahpl, 0, 3) + subrange(ahpl, 7, 10) / ahpl(10);
            return ret;
         }


//         /**
//          * Reparametrize to Euclidean, with Jacobians.
//          * \param ahp the anchored homogeneous point to be reparametrized.
//          * \param euc the returned Euclidean point.
//          * \param EUC_ahp the Jacobian of the conversion.
//          */
//         template<class VA, class VE, class ME_a>
//         void ahp2euc(const VA & ahp, VE & euc, ME_a & EUC_ahp) {
//            euc.assign(ahp2euc(ahp));
//            // split non-trivial chunks of landmark state
//            vec3 m(subrange(ahp, 3, 6));
//            double rho = ahp(6);
//            identity_mat I(3);
//            subrange(EUC_ahp, 0, 3, 0, 3) = I;
//            subrange(EUC_ahp, 0, 3, 3, 6) = I / rho;
//            column(EUC_ahp, 6) = -m / rho / rho;
//         }


         /**
          * Bring landmark to bearing-only sensor frame (without range information).
          *
          * For a landmark and sensor frame
          * - ahpl = [p0 m1 rho1 m2 rho2]
          * - s = [t q1 q2],
          *
          * this function computes the chain (See Sola \e etal. PAMI 2010):
          * - R'(q) * ( m - (t - p0) * rho )
          *
          * which is a vector in sensor frame in the direction of the landmark. The range information is lost.
          *
          * \param s the sensor frame
          * \param ahpl the AHPL landmark
          * \return the bearing-only landmark in sensor frame
          */
         template<class VS, class VA, class VV>
         void toBearingOnlyFrame(const VS & s, const VA & ahpl, VV & v1, VV & v2) {
            vec3 p0, m1, m2;
            double rho1, rho2;
            split(ahpl, p0, m1, m2, rho1, rho2);
            vec3 t = subrange(s, 0, 3);
            vec4 q = subrange(s, 3, 7);
            vec3 d = m1 - (t - p0) * rho1;
            v1 = rotateInv(q, d);
            d = m2 - (t - p0) * rho2;
            v2 = rotateInv(q, d);
         }


         /**
          * Bring landmark to bearing-only sensor frame (give distance information).
          *
          * For a landmark and sensor frame
          * - ahp = [p0 m rho]
          * - s = [t q],
          *
          * this function computes the chain (See Sola \e etal. PAMI 2010):
          * - v = R'(q) * ( m - (t - p0) * rho )
          *
          * which is a vector in sensor frame in the direction of the landmark.
          *
          * The range information is recuperated in \a dist as the distance from sensor to landmark.
          *
          * \param s the sensor frame
          * \param ahp the AHP landmark
          * \param v the bearing-only landmark in sensor frame
          * \param dist the non-observable distance
          */
         template<class VS, class VA, class VV>
         void toBearingOnlyFrame(const VS & s, const VA & ahpl, VV & v1, VV & v2, double & dist1, double & dist2) {
            vec3 p0, m1, m2;
            double rho1, rho2;
            split(ahpl, p0, m1, m2, rho1, rho2);
            vec3 t = subrange(s, 0, 3);
            vec4 q = subrange(s, 3, 7);
            vec3 d = m1 - (t - p0) * rho1;
            v1 = rotateInv(q, d);
            dist1 = norm_2(v1) / rho1;
            d = m2 - (t - p0) * rho2;
            v2 = rotateInv(q, d);
            dist2 = norm_2(v2) / rho2;
         }


         /**
          * Bring landmark to bearing-only sensor frame (give distance information).
          *
          * For a landmark and sensor frame
          * - ahp = [p0 m rho]
          * - s = [t q],
          *
          * this function computes the chain (See Sola \e etal. PAMI 2010):
          * - R'(q) * ( m - (t - p0) * rho )
          *
          * which is a vector in sensor frame in the direction of the landmark.
          *
          * The range information is recuperated in \a dist as the distance from sensor to landmark.
          *
          * and returns the Jacobians wrt s and ahp.
          * \param s the sensor frame
          * \param ahp the AHP landmark
          * \param v the bearing-only landmark in sensor frame
          * \param V_s the Jacobian of \a v wrt \a s
          * \param V_ahp the Jacobian of \a v wrt \a ahp
          */
         template<class VS, class VA, class VV, class MV_s, class MV_a>
         void toBearingOnlyFrame(const VS & s, const VA & ahpl, VV & v1, VV & v2, double & dist1, double & dist2, MV_s & V1_s, MV_a & V1_ahpl, MV_s & V2_s, MV_a & V2_ahpl) {
            vec3 p0, m1, m2;
            double rho1, rho2;
            mat34 V1_q;
            mat33 V1_d1;
            mat34 V2_q;
            mat33 V2_d2;
            // value
            split(ahpl, p0, m1, m2, rho1, rho2);
            vec3 t = subrange(s, 0, 3);
            vec4 q = subrange(s, 3, 7);
            vec3 d = m1 - (t - p0) * rho1; //    before rotation
            rotateInv(q, d, v1, V1_q, V1_d1); //  obtain v1
            dist1 = norm_2(v1) / rho1; //        obtain non-observable distance
            d = m2 - (t - p0) * rho2; //         before rotation
            rotateInv(q, d, v2, V2_q, V2_d2); //  obtain v1
            dist2 = norm_2(v2) / rho2; //        obtain non-observable distance
            // Jacobians
            mat33 V1_p0;
            mat33 V2_p0;
            V1_p0 = V1_d1 * rho1;
            V2_p0 = V2_d2 * rho2;
            //				mat V_rho(3,1); // This comments only for Jacobian reference...
            //				V_m = V_d;
            //				V_rho = prod(V_d, (p0 - t));
            //				V_t = - V_d * rho;
            V1_s.clear();
            V1_ahpl.clear();
            subrange(V1_s, 0, 3, 0, 3) = -V1_p0; //        dv / dt
            subrange(V1_s, 0, 3, 3, 7) = V1_q; //          dv / dq
            subrange(V1_ahpl, 0, 3, 0, 3) = V1_p0; //      dv / dp0
            subrange(V1_ahpl, 0, 3, 3, 6) = V1_d1; //       dv / dm
            column(V1_ahpl, 6) = prod(V1_d1, (p0 - t)); //  dv / drho
            V2_s.clear();
            V2_ahpl.clear();
            subrange(V2_s, 0, 3, 0, 3) = -V2_p0; //        dv / dt
            subrange(V2_s, 0, 3, 3, 7) = V2_q; //          dv / dq
            subrange(V2_ahpl, 0, 3, 0, 3) = V2_p0; //      dv / dp0
            subrange(V2_ahpl, 0, 3, 7, 10) = V2_d2; //       dv / dm
            column(V2_ahpl, 10) = prod(V2_d2, (p0 - t)); //  dv / drho
         }


         /**
          * AHP landmark from bearing-only retro-projection.
          * This function is the inverse of toBearingOnlyFrame(). It builds the Anchored Homogeneous Point (AHP) landmark from a
          * sensor frame \a s, a retro-projected director vector \a v, and a inverse-distance proportional prior \a rho.
          *
          * It uses the formula (See Sola \e etal. PAMI 2010):
          * - AHP = [ t ;  R(q) * v ; rho * norm(v) ]
          *
          * \param s the sensor frame s = [t ; q]
          * \param v the retro-projected director vector in sensor frame
          * \param rho the prior, proportional to inverse-distance
          * \return the AHP landmark.
          */
         template<class VS, class VLS>
         vec11 fromBearingOnlyFrame(const VS & s, const VLS & v1, const VLS & v2, const double _rho1, const double _rho2) {
            vec3 t = subrange(s, 0, 3);
            vec4 q = subrange(s, 3, 7);
            vec11 ahpl;

            subrange(ahpl, 0, 3) = t;
            subrange(ahpl, 3, 6) = rotate(q, v1);
            ahpl(6) = _rho1 * norm_2(v1);
            subrange(ahpl, 7, 10) = rotate(q, v2);
            ahpl(10) = _rho2 * norm_2(v2);
            return ahpl;
         }


         /**
          * AHP landmark from bearing-only retro-projection, with Jacobians.
          * This function is the inverse of toBearingOnlyFrame(). It builds the Anchored Homogeneous Point (AHP) landmark from a
          * sensor frame \a s, a retro-projected director vector \a v, and a inverse-distance proportional prior \a rho.
          *
          * It uses the formula (See Sola \e etal. PAMI 2010):
          * - AHP = [ t ;  R(q) * v ; rho * norm(v) ]
          *
          * \param s the sensor frame
          * \param v the retro-projected director vector in sensor frame
          * \param rho the prior, proportional to inverse-distance
          * \param ahp the AHP landmark.
          * \param AHP_s the Jacobian wrt \a s
          * \param AHP_v the Jacobian wrt \a v
          * \param AHP_rho the Jacobian wrt \a rho
          */
         void fromBearingOnlyFrame(const vec7 & s, const vec3 & v1, const vec3 & v2, const double _rho1, const double _rho2,
            vec & ahpl, mat & AHPL_s, mat & AHPL_v1, mat & AHPL_v2, mat & AHPL_rho1, mat & AHPL_rho2);
/*
         template<class VS, class VLS, class VA, class MA_s, class MA_v, class MA_rho>
         void fromBearingOnlyFrame(const VS & s, const VLS & v1, const VLS & v2, const double _rho1, const double _rho2,
             VA & ahpl, MA_s & AHPL_s, MA_v & AHPL_v1, MA_v & AHPL_v2, MA_rho & AHPL_rho1, MA_rho & AHPL_rho2) {
            vec3 t = subrange(s, 0, 3);
            vec4 q = subrange(s, 3, 7);
            vec3 m1;
            vec3 m2;
            mat34 M1_q;
            mat33 M1_v1;
            mat34 M2_q;
            mat33 M2_v2;
            mat NV1_v1(1, 3);
            mat NV2_v2(1, 3);

            rotate(q, v1, m1, M1_q, M1_v1);
            rotate(q, v2, m2, M2_q, M2_v2);
            double nv1 = norm_2(v1);
            double nv2 = norm_2(v2);

            subrange(ahpl, 0, 3) = t; // p0  = t
            subrange(ahpl, 3, 6) = m1; // m   = R(q) * v
            ahpl(6) = _rho1 * nv1; //              rho = ||v|| * _rho
            ublasExtra::norm_2Jac<3>(v1, NV1_v1); // drho / dv = d||v||/dv * _rho
            subrange(ahpl, 7, 10) = m2; // m   = R(q) * v
            ahpl(10) = _rho2 * nv2; //              rho = ||v|| * _rho
            ublasExtra::norm_2Jac<3>(v2, NV2_v2); // drho / dv = d||v||/dv * _rho

            // Jacobians
            identity_mat I(3);
            AHPL_s.clear();
            AHPL_v1.clear();
            AHPL_v2.clear();
            AHPL_rho1.clear();
            AHPL_rho2.clear();
            subrange(AHPL_s, 0, 3, 0, 3) = I; //                dp0 / dt
            subrange(AHPL_s, 3, 6, 3, 7) = M1_q; //             dm / dq
            subrange(AHPL_s, 7, 10, 3, 7) = M2_q; //            dm / dq
            subrange(AHPL_v1, 3, 6, 0, 3) = M1_v1; //           dm / dv
            subrange(AHPL_v2, 7, 10,0, 3) = M2_v2; //           dm / dv
            subrange(AHPL_v1, 6, 7, 0, 3) = _rho1 * NV1_v1; //  drho / dv
            subrange(AHPL_v2, 7, 11,0, 3) = _rho2 * NV2_v2; //  drho / dv
            AHPL_rho1(6, 0) = nv1; //                           drho / drho
            AHPL_rho2(10,0) = nv2; //                           drho / drho
         }
*/
         template<class VS, class VA, class MA_s>
         double linearityScore(const VS & senpose, const VA & ahp, const MA_s & AHP){
/*
               vec3 euc = ahp2euc(ahp);

               vec3 hw = euc - subrange(senpose, 0, 3);

               double sigma_rho = sqrt(AHP(6,6));

               double sigma_d   = sigma_rho/(ahp(6)*ahp(6));
               double norm_hw   = norm_2(hw);
               double norm_m    = norm_2(subrange(ahp, 3, 6));
               double cos_a     = inner_prod(subrange(ahp, 3, 6) , hw) / (norm_hw*norm_m);

               double L = 4.0*sigma_d/norm_hw*jmath::abs(cos_a);

//					std::cout << "rho="<<ahp(6)<<", sr="<<sigma_rho<<", sd="<<sigma_d<<", nh="<<norm_hw<<", nm="<<norm_m<<", cos="<<jmath::abs(cos_a)<<std::endl;
//					std::cout << "linearity score: " << L << std::endl;

               return L; */ return 0.0;
         }

      } // namespace landmarkAHPL


   }
}

#endif // AHPLTOOLS_HPP
