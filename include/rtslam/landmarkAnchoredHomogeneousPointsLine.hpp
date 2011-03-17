/*
 * \file landmarkAnchoredHomogeneousPointsLine.hpp
 *
 *  Created on: Feb 14, 2011
 *      Author: bhautboi
 *      \ingroup rtslam
 */

#ifndef LANDMARKANCHOREDHOMOGENEOUSPOINTSLINE_HPP_
#define LANDMARKANCHOREDHOMOGENEOUSPOINTSLINE_HPP_

#include "boost/shared_ptr.hpp"
#include "rtslam/landmarkAbstract.hpp"
#include "rtslam/ahpTools.hpp"

/**
 * General namespace for Jafar environment.
 * \ingroup jafar
 */
namespace jafar {
  namespace rtslam {

    class LandmarkAnchoredHomogeneousPointsLine;
    typedef boost::shared_ptr<LandmarkAnchoredHomogeneousPointsLine> ahpl_ptr_t;



    /**
     * Class for Anchored homogeneous points lines
     * \ingroup rtslam
     */
    class LandmarkAnchoredHomogeneousPointsLine: public LandmarkAbstract {
      public:


        /**
         * Constructor from map
         */
        LandmarkAnchoredHomogeneousPointsLine(const map_ptr_t & mapPtr);

        /**
         * Constructor for simulated landmark.
         */
        LandmarkAnchoredHomogeneousPointsLine(const simulation_t dummy, const map_ptr_t & mapPtr);
        /**
         * Constructor by replacement: occupied the same filter state as a specified previous lmk. _icomp is the complementary memory, to be relaxed by the user.
         */
        LandmarkAnchoredHomogeneousPointsLine(const map_ptr_t & _mapPtr, const landmark_ptr_t _prevLmk,jblas::ind_array & _icomp);

        virtual ~LandmarkAnchoredHomogeneousPointsLine() {
//					cout << "Deleted landmark: " << id() << ": " << typeName() << endl;
        }

        virtual std::string typeName() const {
          return "Anchored-Homogeneous-Points-Line";
        }


        static size_t size(void) {
          return 11;
        }

        virtual size_t mySize() {return size();}

        virtual size_t reparamSize() {/*return size();*/ vec6 v; return v.size();} // TODO clean up

        virtual vec reparametrize_func(const vec & lmk) const {
          vec6 ret;
          vec7 lmk1;
          vec7 lmk2;
          subrange(lmk1,0,3) = subrange(lmk,0,3);
          subrange(lmk1,3,7) = subrange(lmk,3,7);
          subrange(lmk2,0,3) = subrange(lmk,0,3);
          subrange(lmk2,3,7) = subrange(lmk,7,11);
          subrange(ret,0,3) = lmkAHP::ahp2euc(lmk1);
          subrange(ret,3,6) = lmkAHP::ahp2euc(lmk2);
          return ret;
        }

        void reparametrize_func(const vec & lmk, vec & lnew, mat & LNEW_lmk) const {
          vec7 lmk1;
          vec7 lmk2;
          vec3 euc1;
          vec3 euc2;
          mat EUC1_lmk1(3,7);
          mat EUC2_lmk2(3,7);
          LNEW_lmk.clear();
          subrange(lmk1,0,3) = subrange(lmk,0,3);
          subrange(lmk1,3,7) = subrange(lmk,3,7);
          subrange(lmk2,0,3) = subrange(lmk,0,3);
          subrange(lmk2,3,7) = subrange(lmk,7,11);
          lmkAHP::ahp2euc(lmk1,euc1,EUC1_lmk1);
          lmkAHP::ahp2euc(lmk2,euc2,EUC2_lmk2);
          subrange(lnew,0,3) = euc1;
          subrange(lnew,3,6) = euc2;
          subrange(LNEW_lmk,0,3,0,7)  = EUC1_lmk1;
          subrange(LNEW_lmk,3,6,0,3)  = subrange(EUC2_lmk2,0,3,0,3);
          subrange(LNEW_lmk,3,6,7,11) = subrange(EUC2_lmk2,0,3,3,7);
        }

        virtual bool needToDie(DecisionMethod dieMet = ANY);

        virtual bool needToReparametrize(DecisionMethod met = ALL){
          return false; // TODO real reparametrization
        }
    }; // class LandmarkAnchoredHomogeneousPointsLine
  } // namespace rtslam
} // namespace jafar


#endif /* LANDMARKANCHOREDHOMOGENEOUSPOINTSLINE_HPP_ */
