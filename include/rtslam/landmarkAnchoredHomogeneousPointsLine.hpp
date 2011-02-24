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

        virtual size_t reparamSize() {return size();}

        virtual vec reparametrize_func(const vec & lmk) const {
          return lmk;
        }

        void reparametrize_func(const vec & lmk, vec & lnew, mat & LNEW_lmk) const {
          lnew = lmk;
          LNEW_lmk = identity_mat(size());
        }

        virtual bool needToDie(DecisionMethod dieMet = ANY);

        virtual bool needToReparametrize(){
          return true; // TODO why ?
        }
    }; // class LandmarkAnchoredHomogeneousPointsLine
  } // namespace rtslam
} // namespace jafar


#endif /* LANDMARKANCHOREDHOMOGENEOUSPOINTSLINE_HPP_ */
