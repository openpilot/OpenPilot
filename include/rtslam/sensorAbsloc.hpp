/**
 * \file sensorAbsloc.hpp
 *
 * Header file for absolute localisation sensors (gps, motion capture...)
 *
 * \date 16/03/2011
 * \author croussil
 *
 * \ingroup rtslam
 */

#ifndef SENSORABSLOC_HPP_
#define SENSORABSLOC_HPP_

#include "jmath/jblas.hpp"
#include "jmath/ublasExtra.hpp"
#include "jmath/misc.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/quatTools.hpp"
#include "rtslam/sensorAbstract.hpp"
#include "rtslam/innovation.hpp"

namespace jafar {
	namespace rtslam {

		class SensorAbsloc;
		typedef boost::shared_ptr<SensorAbsloc> absloc_ptr_t;
		
		/**
		 * Class for absolute localization sensors (gps, motion capture...)
		 * For now we assume that we have at least one reading before images and
		 * that is is very precise. Improvements would be to start at 0,0,0
		 * with uncertainty 0 and estimate the initial position.
		 * \ingroup rtslam
		 */
		class SensorAbsloc: public SensorProprioAbstract {
			protected:
				jblas::ind_array ia_rs;
				Innovation *innovation;
				Measurement *measurement;
				Expectation *expectation;
				jblas::mat EXP_rs;
				jblas::mat INN_rs;
				jblas::mat EXP_q;
				int inns; /// innovation size
				int dats; /// data size
				bool absolute;
				bool first;

			private:
				size_t indexE_OriEuler, indexE_Pos, indexE_Bundleobs; /// indexes in expectation
				int indexD_Pos, indexD_OriEuler, indexD_Bundleobs; /// indexes in sensor's data
				bool full_cov;

			public:
				/**
					@param absolute do we estimate the absolute position as returned by the sensor,
					or do we estimate a relative position wrt the initial absolute position, and just
					convert to absolute position before exporting.
				*/
				SensorAbsloc(const robot_ptr_t & robPtr, const filtered_obj_t inFilter = UNFILTERED, bool absolute = false):
				  SensorProprioAbstract(robPtr, inFilter),
					ia_rs(ia_globalPose), innovation(NULL), measurement(NULL), expectation(NULL),
					inns(0), dats(0), absolute(absolute), first(true)
				{}
				~SensorAbsloc() { delete innovation; delete measurement; }
				virtual void setHardwareSensor(hardware::hardware_sensorprop_ptr_t hardwareSensorPtr_)
				{
					hardwareSensorPtr = hardwareSensorPtr_;
					// initialize jacobians and innovation sizes
					inns = hardwareSensorPtr->obsSize();
					dats = hardwareSensorPtr->dataSize();
					innovation = new Innovation(inns);
					measurement = new Measurement(inns);
					expectation = new Expectation(inns);
					EXP_rs.resize(inns, ia_rs.size(), false);
					INN_rs.resize(inns, ia_rs.size(), false);
					EXP_q.resize(3, 4, false);
				}
				
				
				virtual void init(unsigned id)
				{
					RawInfos infos;
					queryAvailableRaws(infos);

					// initialize type of data the hardware sensor is providing
					size_t indexE = 0; // current index when building expectation

					indexD_Pos = hardwareSensorPtr->getQuantity(hardware::HardwareSensorProprioAbstract::qPos);
					if (indexD_Pos >= 0) { indexE_Pos = indexE; indexE += hardwareSensorPtr->QuantityObsSizes[hardware::HardwareSensorProprioAbstract::qPos]; }

					indexD_OriEuler = hardwareSensorPtr->getQuantity(hardware::HardwareSensorProprioAbstract::qOriEuler);
					if (indexD_OriEuler >= 0) { indexE_OriEuler = indexE; indexE += hardwareSensorPtr->QuantityObsSizes[hardware::HardwareSensorProprioAbstract::qOriEuler]; }

					indexD_Bundleobs = hardwareSensorPtr->getQuantity(hardware::HardwareSensorProprioAbstract::qBundleobs);
					if (indexD_Bundleobs >= 0) { indexE_Bundleobs = indexE; indexE += hardwareSensorPtr->QuantityObsSizes[hardware::HardwareSensorProprioAbstract::qBundleobs]; }

					full_cov = hardwareSensorPtr->hasFullCov();

					// find minimum variance
					jblas::vec3 min_var; min_var(0) = min_var(1) = min_var(2) = 1e3;
					for(std::vector<RawInfo>::iterator it = infos.available.begin(); it != infos.available.end(); ++it)
					{
						hardwareSensorPtr->observeRaw((*it).id, reading);
						for(int i = 0; i < 3; ++i) if (reading.data(i+1+dats) < min_var(i)) min_var(i) = reading.data(i+1+dats);
						if ((*it).id == id) break;
					}

					// do the average using only readings with variance between min and 2*min
					jblas::vec3 average; average.clear();
					jblas::vec3 sum_coeffs; sum_coeffs.clear();
					for(std::vector<RawInfo>::iterator it = infos.available.begin(); it != infos.available.end(); ++it)
					{
						hardwareSensorPtr->observeRaw((*it).id, reading);
						for(int i = 0; i < 3; ++i)
							if (reading.data(i+1+dats) < 2*min_var(i))
								{ average(i) += reading.data(i+1)*reading.data(i+1+dats); sum_coeffs(i) += reading.data(i+1+dats); }
						if ((*it).id == id) break;
					}
					for(int i = 0; i < 3; ++i) average(i) /= sum_coeffs(i);

					// now initialize the robot state with this variance
					for(int i = 0; i < 3; ++i) { reading.data(i+1) = average(i); reading.data(i+1+dats) = min_var(i); }
				}
				

				virtual void process(unsigned id)
				{
					if (use_for_init)
						init(id);
					else
						hardwareSensorPtr->getRaw(id, reading);

					EXP_rs.clear();
					expectation->P() = jblas::zero_mat(inns);
					jblas::vec T = ublas::subrange(pose.x(), 0, 3);
					jblas::vec r = ublas::subrange(pose.x(), 3, 7);
					jblas::vec p = ublas::subrange(robotPtr()->pose.x(), 0, 3);
					jblas::vec q = ublas::subrange(robotPtr()->pose.x(), 3, 7);
					jblas::vec Tr = quaternion::rotate(q,T);

					// POSITION
					if (indexD_Pos >= 0)
					{
						// compute expectation->x and EXP_rs
						quaternion::rotate_by_dq(q, T, EXP_q);
						ublas::subrange(EXP_rs, indexE_Pos,indexE_Pos+3, 0,3) = jblas::identity_mat(3);
						ublas::subrange(EXP_rs, indexE_Pos,indexE_Pos+3, 3,7) = EXP_q;
						ublas::subrange(expectation->x(), indexE_Pos,indexE_Pos+3) = p + Tr;

						// fill measurement
						ublas::subrange(measurement->x(), indexE_Pos,indexE_Pos+3) = ublas::subrange(reading.data, indexD_Pos,indexD_Pos+3) - robotPtr()->origin_sensors;
						for(int i = 0; i < 3; ++i) measurement->P()(indexE_Pos+i,indexE_Pos+i) = jmath::sqr(reading.data(indexD_Pos+i+dats));

						// TODO gating ?
					}

					// ORIENTATION EULER
					if (indexD_OriEuler >= 0)
					{
						// compute expectation->x and EXP_rs
						jblas::mat QR_q(4,4), E_qr(3,4);
						jblas::vec3 e;
						jblas::vec4 qr = quaternion::qProd(q, r);
						quaternion::qProd_by_dq1(r, QR_q);
						quaternion::q2e(qr, e, E_qr);
						ublas::subrange(expectation->x(), indexE_OriEuler,indexE_OriEuler+3) = e;
						ublas::subrange(EXP_rs, indexE_OriEuler,indexE_OriEuler+3, 3,7) = ublas::prod(E_qr, QR_q);

						// fill measurement
						ublas::subrange(measurement->x(), indexE_OriEuler,indexE_OriEuler+3) = ublas::subrange(reading.data, indexD_OriEuler,indexD_OriEuler+3);
						for(int i = 0; i < 3; ++i) measurement->P()(indexE_OriEuler+i,indexE_OriEuler+i) = jmath::sqr(reading.data(indexD_OriEuler+i+dats));
					}

					// BUNDLE OBSERVATION
					if (indexD_Bundleobs >= 0)
					{
						// get data
						jblas::vec3 pos = ublas::subrange(reading.data, indexD_Bundleobs+0,indexD_Bundleobs+3);
						jblas::vec3 dir = ublas::subrange(reading.data, indexD_Bundleobs+3,indexD_Bundleobs+6);

						// compute expectation->x and EXP_rs and expectation->P
						jblas::mat33 U_p;
						jblas::vec3 u = pos-p;
						jmath::ublasExtra::normalizeJac(u, U_p);
						jmath::ublasExtra::normalize(u);
						ublas::subrange(expectation->x(), indexE_Bundleobs,indexE_Bundleobs+3) = u;
						ublas::subrange(EXP_rs, indexE_Bundleobs,indexE_Bundleobs+3, 0,3) = -U_p;

						jblas::sym_mat33 Pos; Pos.clear();
						if (full_cov)
						{
							Pos = ublasExtra::createSymMat(reading.data, dats+1, dats, 0, 3);
//							for(int i = 0; i < 3; ++i)
//							{
//								int k = dats+1 + (dats)*(dats+1)/2 - (dats-i)*(dats-i+1)/2;
//								for(int j = i; j < 3; ++j) Pos(i,j) = reading.data(k+(j-i));
//							}
						} else
						{
							for(int i = 0; i < 3; ++i) Pos(i,i) = reading.data(indexE_Bundleobs+i+dats);
						}
						ublas::subrange(expectation->P(), indexE_Bundleobs,indexE_Bundleobs+3, indexE_Bundleobs,indexE_Bundleobs+3) = prod_JPJt(Pos, U_p);

						// fill measurement
						ublas::subrange(measurement->x(), indexE_Bundleobs,indexE_Bundleobs+3) = dir;
						ublas::subrange(measurement->P(), indexE_Bundleobs,indexE_Bundleobs+3, indexE_Bundleobs,indexE_Bundleobs+3) =
							ublasExtra::createSymMat(reading.data, dats+1, dats, 3, 3);
//						for(int i = 0; i < 3; ++i)
//						{
//							int k = dats+1 + (dats)*(dats+1)/2 - (dats-(i+3))*(dats-(i+3)+1)/2;
//							for(int j = i; j < 3; ++j) measurement->P()(indexE_Bundleobs+i,indexE_Bundleobs+j) = reading.data(k+(j-i));
//						}
					}


					if (first)
					{
						first = false;
						// for first reading we force initialization

						if (indexD_OriEuler) // need to set ori uncertainty before pos uncertainty because of sensor lever arm
						{
							jblas::mat Q_qr(4,4), QR_e(4,3), Q_exp(4,3);
							jblas::vec qr, ri = quaternion::q2qc(r);
							quaternion::e2q(ublas::subrange(measurement->x(), indexE_OriEuler,indexE_OriEuler+3), qr, QR_e);
							q = quaternion::qProd(qr, ri);
							quaternion::qProd_by_dq1(ri, Q_qr);
							Q_exp = ublas::prod(Q_qr, QR_e);

							ublas::subrange(robotPtr()->pose.x(), 3, 7) = q;
							ublas::subrange(robotPtr()->pose.P(), 3,7, 3,7) = ublasExtra::prod_JPJt(
								ublas::subrange(measurement->P(), indexE_OriEuler,indexE_OriEuler+3, indexE_OriEuler,indexE_OriEuler+3), Q_exp);

							std::cout << "AbsLoc sets initial orientation q = " << ublas::subrange(robotPtr()->pose.x(), 3, 7) <<
								" e = " <<  quaternion::q2e(ublas::subrange(robotPtr()->pose.x(), 3, 7)) << std::endl;

						}

						if (indexD_Pos)
						{
							if (absolute)
							{
								robotPtr()->origin_sensors = jblas::zero_vec(3);
								ublas::subrange(robotPtr()->pose.x(), 0,3) = ublas::subrange(measurement->x(), indexE_Pos,indexE_Pos+3) - Tr;
								ublas::subrange(robotPtr()->pose.P(), 0,3, 0,3) = ublas::subrange(measurement->P(), indexE_Pos,indexE_Pos+3, indexE_Pos,indexE_Pos+3) +
									ublasExtra::prod_JPJt(ublas::subrange(robotPtr()->pose.P(), 3,7, 3,7), EXP_q);
							} else
							{
								robotPtr()->origin_sensors = ublas::subrange(measurement->x(), indexE_Pos,indexE_Pos+3) - Tr;
								ublas::subrange(robotPtr()->pose.x(), 0, 3) = jblas::zero_vec(3);
								ublas::subrange(robotPtr()->pose.P(), 0,3, 0,3) = ublas::subrange(measurement->P(), indexE_Pos,indexE_Pos+3, indexE_Pos,indexE_Pos+3) +
									ublasExtra::prod_JPJt(ublas::subrange(robotPtr()->pose.P(), 3,7, 3,7), EXP_q);
							}

							std::cout << "AbsLoc sets robot origin " << robotPtr()->origin_sensors <<
								" ; initial position " << ublas::subrange(robotPtr()->pose.x(), 0,3) <<
								" ; initial position var " << ublas::subrange(robotPtr()->pose.P(), 0,3, 0,3) << std::endl;
						}
					} else
					{
						// compute expectation->P and innovation
						expectation->P() += ublasExtra::prod_JPJt(ublas::project(robotPtr()->mapPtr()->filterPtr->P(), ia_rs, ia_rs), EXP_rs);
						innovation->x() = measurement->x() - expectation->x();
						innovation->P() = measurement->P() + expectation->P();
						INN_rs = -EXP_rs;

						map_ptr_t mapPtr = robotPtr()->mapPtr();
						ind_array ia_x = mapPtr->ia_used_states();
						mapPtr->filterPtr->correct(ia_x,*innovation,INN_rs,ia_rs);
					}

					if (use_for_init)
					{
						use_for_init = false;
						hardwareSensorPtr->getRaw(id, reading); // just to free
					}
					
					//hardwareSensorPtr->release();
				}

				
		};
}}

#endif
