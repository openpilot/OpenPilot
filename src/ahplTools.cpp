#include "rtslam/ahplTools.hpp"

namespace jafar
{
   namespace rtslam
   {

   void lmkAHPL::fromBearingOnlyFrame(const vec7 & s, const vec3 & v1, const vec3 & v2, const double _rho1, const double _rho2,
      vec & ahpl, mat & AHPL_s, mat & AHPL_v1, mat & AHPL_v2, mat & AHPL_rho1, mat & AHPL_rho2) {
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
        subrange(AHPL_v2, 10, 11,0, 3) = _rho2 * NV2_v2; //  drho / dv
        AHPL_rho1(6, 0) = nv1; //                           drho / drho
        AHPL_rho2(10,0) = nv2; //                           drho / drho
   }
   }
}
