/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotLibraries OpenPilot System Libraries
 * @{
 * @file       rtslam.hpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include file of the task monitoring library
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef RTSLAMM_HPP
#define RTSLAMM_HPP

/** ############################################################################
 * #############################################################################
 * Includes
 * ###########################################################################*/

#include <iostream>
#include <boost/shared_ptr.hpp>
//#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <time.h>
#include <map>
#include <getopt.h>
#include "kernel/keyValueFile.hpp"

// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/timingTools.hpp"
#include "kernel/dataLog.hpp"
#include "kernel/threads.hpp"
#include "jmath/random.hpp"
#include "jmath/matlab.hpp"
#include "jmath/ublasExtra.hpp"
#include "jmath/angle.hpp"

#include "rtslam/rtSlam.hpp"
#include "rtslam/rawProcessors.hpp"
#include "rtslam/rawSegProcessors.hpp"
#include "rtslam/robotOdometry.hpp"
#include "rtslam/robotConstantVelocity.hpp"
#include "rtslam/robotInertial.hpp"
#include "rtslam/sensorPinhole.hpp"
#include "rtslam/sensorAbsloc.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "rtslam/landmarkAnchoredHomogeneousPointsLine.hpp"
//#include "rtslam/landmarkEuclideanPoint.hpp"
#include "rtslam/observationFactory.hpp"
#include "rtslam/observationMakers.hpp"
#include "rtslam/activeSearch.hpp"
#include "rtslam/activeSegmentSearch.hpp"
#include "rtslam/featureAbstract.hpp"
#include "rtslam/rawImage.hpp"
#include "rtslam/descriptorImagePoint.hpp"
#include "rtslam/descriptorSeg.hpp"
#include "rtslam/dataManagerOnePointRansac.hpp"
#include "rtslam/sensorManager.hpp"

#include "rtslam/hardwareSensorCameraFirewire.hpp"
#include "rtslam/hardwareSensorCameraUeye.hpp"
#include "rtslam/hardwareSensorCameraOpenCV.hpp"
#include "rtslam/hardwareSensorCameraOpenPilot.hpp"
#include "rtslam/hardwareEstimatorMti.hpp"
#include "rtslam/hardwareSensorGpsGenom.hpp"
#include "rtslam/hardwareSensorMocap.hpp"
#include "rtslam/hardwareSensorStateOpenPilot.hpp"
#include "rtslam/hardwareEstimatorOdo.hpp" 
#include "rtslam/hardwareSensorExternalLoc.hpp"

#include "rtslam/display_qt.hpp"
#include "rtslam/display_gdhe.hpp"

#include "rtslam/simuRawProcessors.hpp"
#include "rtslam/hardwareSensorAdhocSimulator.hpp"
#include "rtslam/hardwareEstimatorInertialAdhocSimulator.hpp"
#include "rtslam/exporterSocket.hpp"




extern "C" {
	#include "rtslam.h"		// c wrapper
};



using namespace jblas;
using namespace jafar;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace jafar::rtslam;
using namespace boost;



/** ############################################################################
 * #############################################################################
 * program parameters
 * ###########################################################################*/

enum { iDispQt = 0, iDispGdhe, iRenderAll, iReplay, iDump, iRandSeed, iPause, iVerbose, iMap, iRobot, iCamera, iTrigger, iGps, iSimu, iExport, nIntOpts };

enum { fFreq = 0, fShutter, fHeading, nFloatOpts };

enum { sDataPath = 0, sConfigSetup, sConfigEstimation, sLog, nStrOpts };

enum { bHelp = 0, bUsage, nBreakingOpts };

/// !!WARNING!! be careful that options are in the same order above and below







class RTSlam {


public:
    RTSlam(void);
	void init();
    void run();
    void exit(boost::thread *thread_main);
    void main();
    void display();

	void videoFrame(IplImage* image);
	void state(hardware::OpenPilotStateInformation * state);

protected:


	// stuff goes here
	int mode;
	time_t rseed;

    int intOpts[nIntOpts];
    static const int nFirstIntOpt = 0, nLastIntOpt = nIntOpts-1;

    double floatOpts[nFloatOpts];
    static const int nFirstFloatOpt = nIntOpts, nLastFloatOpt = nIntOpts+nFloatOpts-1;

    std::string strOpts[nStrOpts];
    static const int nFirstStrOpt = nIntOpts+nFloatOpts, nLastStrOpt = nIntOpts+nFloatOpts+nStrOpts-1;

    static const int nFirstBreakingOpt = nIntOpts+nFloatOpts+nStrOpts, nLastBreakingOpt = nIntOpts+nFloatOpts+nStrOpts+nBreakingOpts-1;

    /** ############################################################################
     * #############################################################################
     * Config data
     * ###########################################################################*/


    static const int slam_priority = -20; // needs to be started as root to be < 0
    static const int display_priority = 10;
    static const int display_period = 100; // ms
    static const unsigned N_FRAMES = 500000;

    /** ############################################################################
     * #############################################################################
     * Slam function
     * ###########################################################################*/
    
    /** pointer to camera sensor to get images fed from OP **/
    hardware::HardwareSensorCameraOpenPilot *openpilotcamera;
    hardware::HardwareSensorStateOpenPilot *openpilotstate;
    /** pointer to GPS sensor to get data fed from OP **/
    //hardware::HardwareSensorXXXOpenPilot * openpilotgps;
    /** pointer to odometry sensor to get data fed from OP **/
    //hardware::HardwareSensorXXXOpenPilot * openpilotins;

    world_ptr_t worldPtr;
    boost::scoped_ptr<kernel::DataLogger> dataLogger;
    sensor_manager_ptr_t sensorManager;
    boost::shared_ptr<ExporterAbstract> exporter;
    #ifdef HAVE_MODULE_QDISPLAY
    display::ViewerQt *viewerQt;
    #endif
    #ifdef HAVE_MODULE_GDHE
    display::ViewerGdhe *viewerGdhe;
    #endif
    kernel::VariableCondition<int> rawdata_condition;
    //(0);

    class ConfigSetup: public kernel::KeyValueFileSaveLoad
    {
     public:
        /// SENSOR
        jblas::vec6 SENSOR_POSE_CONSTVEL; /// camera pose in constant velocity (x,y,z,roll,pitch,yaw) (m,deg)
        jblas::vec6 SENSOR_POSE_INERTIAL; /// camera pose in inertial (x,y,z,roll,pitch,yaw) (m,deg)
        jblas::vec6 GPS_POSE; /// GPS pose (x,y,z,roll,pitch,yaw) (m,deg)
        jblas::vec2 GPS_VARIANCE; /// GPS pose (x,y,z,roll,pitch,yaw) (m,deg)
        jblas::vec6 ROBOT_POSE; /// the transformation between the slam robot (the main sensor, camera or imu) and the real robot = pose of the real robot in the slam robot frame, just like the other sensors

        unsigned CAMERA_TYPE;      /// camera type (0 = firewire, 1 = firewire format7, 2 = USB, 3 = UEYE)
        std::string CAMERA_DEVICE; /// camera device (firewire ID or device)
        unsigned IMG_WIDTH;        /// image width
        unsigned IMG_HEIGHT;       /// image height
        jblas::vec4 INTRINSIC;     /// intrisic calibration parameters (u0,v0,alphaU,alphaV)
        jblas::vec3 DISTORTION;    /// distortion calibration parameters

        /// SIMU SENSOR
        unsigned IMG_WIDTH_SIMU;
        unsigned IMG_HEIGHT_SIMU;
        jblas::vec4 INTRINSIC_SIMU;
        jblas::vec3 DISTORTION_SIMU;

        /// CONSTANT VELOCITY
        double UNCERT_VLIN; /// initial uncertainty stdev on linear velocity (m/s)
        double UNCERT_VANG; /// initial uncertainty stdev on angular velocity (rad/s)
        double PERT_VLIN;   /// perturbation on linear velocity, ie non-constantness (m/s per sqrt(s))
        double PERT_VANG;   /// perturbation on angular velocity, ie non-constantness (rad/s per sqrt(s))

        /// INERTIAL (also using UNCERT_VLIN)
        std::string MTI_DEVICE;    /// IMU device
        double ACCELERO_FULLSCALE; /// full scale of accelerometers (m/s2)  (MTI: 17)
        double ACCELERO_NOISE;     /// noise stdev of accelerometers (m/s2) (MTI: 0.002*sqrt(30) )
        double GYRO_FULLSCALE;     /// full scale of gyrometers (rad/s)     (MTI: rad(300) )
        double GYRO_NOISE;         /// noise stdev of gyrometers (rad/s)    (MTI: rad(0.05)*sqrt(40) )

        double UNCERT_GRAVITY;   /// initial gravity uncertainty (% of 9.81)
        double UNCERT_ABIAS;     /// initial accelerometer bias uncertainty (% of ACCELERO_FULLSCALE)
        double UNCERT_WBIAS;     /// initial gyrometer bias uncertainty (% of GYRO_FULLSCALE)
        double PERT_AERR;        /// noise stdev coeff of accelerometers, for testing purpose (% of ACCELERO_NOISE)
        double PERT_WERR;        /// noise stdev coeff of gyrometers, for testing purpose (% of GYRO_NOISE)
        double PERT_RANWALKACC;  /// IMU a_bias random walk (m/s2 per sqrt(s))
        double PERT_RANWALKGYRO; /// IMU w_bias random walk (rad/s per sqrt(s))

        double UNCERT_HEADING;   /// initial heading uncertainty
        double UNCERT_ATTITUDE;   /// initial attitude angles uncertainty

        double IMU_TIMESTAMP_CORRECTION; /// correction to add to the IMU timestamp for synchronization (s)
        double GPS_TIMESTAMP_CORRECTION; /// correction to add to the GPS timestamp for synchronization (s)

        /// Odometry noise variance to distance ratios
        double dxNDR;   /// Odometry noise in position increment (m per sqrt(m))
        double dvNDR;    /// Odometry noise in orientation increment (rad per sqrt(m))

        double POS_TIMESTAMP_CORRECTION; /// correction to add to the POS timestamp for synchronization (s)

        /// SIMU INERTIAL
        double SIMU_IMU_TIMESTAMP_CORRECTION;
        double SIMU_IMU_FREQ;
        double SIMU_IMU_GRAVITY;
        double SIMU_IMU_GYR_BIAS;
        double SIMU_IMU_GYR_BIAS_NOISESTD;
        double SIMU_IMU_GYR_GAIN;
        double SIMU_IMU_GYR_GAIN_NOISESTD;
        double SIMU_IMU_RANDWALKGYR_FACTOR;
        double SIMU_IMU_ACC_BIAS;
        double SIMU_IMU_ACC_BIAS_NOISESTD;
        double SIMU_IMU_ACC_GAIN;
        double SIMU_IMU_ACC_GAIN_NOISESTD;
        double SIMU_IMU_RANDWALKACC_FACTOR;

     private:
     	RTSlam* owner;
      void processKeyValueFile(jafar::kernel::KeyValueFile& keyValueFile, bool read);
     public:
     	ConfigSetup(RTSlam* myowner);
        virtual void loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile);
        virtual void saveKeyValueFile(jafar::kernel::KeyValueFile& keyValueFile);
    } configSetup;






    class ConfigEstimation: public kernel::KeyValueFileSaveLoad
    {
     public:
        /// MISC
        unsigned CORRECTION_SIZE; /// number of coefficients for the distortion correction polynomial

        /// FILTER
        unsigned MAP_SIZE; /// map size in # of states, robot + landmarks
        double PIX_NOISE;  /// measurement noise of a point
        double PIX_NOISE_SIMUFACTOR;

        /// LANDMARKS
        double D_MIN;      /// inverse depth mean initialization
        double REPARAM_TH; /// reparametrization threshold

        unsigned GRID_HCELLS;
        unsigned GRID_VCELLS;
        unsigned GRID_MARGIN;
        unsigned GRID_SEPAR;

        double RELEVANCE_TH;       /// (# of sigmas)
        double MAHALANOBIS_TH;     /// (# of sigmas)
        unsigned N_UPDATES_TOTAL;  /// max number of landmarks to update every frame
        unsigned N_UPDATES_RANSAC; /// max number of landmarks to update with ransac every frame
        unsigned N_INIT;           /// maximum number of landmarks to try to initialize every frame
        unsigned N_RECOMP_GAINS;   /// how many times information gain is recomputed to resort observations in active search
        double RANSAC_LOW_INNOV;   /// ransac low innovation threshold (pixels)

        unsigned RANSAC_NTRIES;    /// number of base observation used to initialize a ransac set

        /// RAW PROCESSING
        unsigned HARRIS_CONV_SIZE;
        double HARRIS_TH;
        double HARRIS_EDDGE;

        unsigned DESC_SIZE;     /// descriptor patch size (odd value)
        bool MULTIVIEW_DESCRIPTOR; /// whether use or not the multiview descriptor
        double DESC_SCALE_STEP; /// MultiviewDescriptor: min change of scale (ratio)
        double DESC_ANGLE_STEP; /// MultiviewDescriptor: min change of point of view (deg)
        int DESC_PREDICTION_TYPE; /// type of prediction from descriptor (0 = none, 1 = affine, 2 = homographic)

        unsigned PATCH_SIZE;       /// patch size used for matching
        unsigned MAX_SEARCH_SIZE;  /// if the search area is larger than this # of pixels, we bound it
        unsigned KILL_SEARCH_SIZE; /// if the search area is larger than this # of pixels, we vote for killing the landmark
        double MATCH_TH;           /// ZNCC score threshold
        double MIN_SCORE;          /// min ZNCC score under which we don't finish to compute the value of the score
        double PARTIAL_POSITION;   /// position in the patch where we test if we finish the correlation computation

     private:
      void processKeyValueFile(jafar::kernel::KeyValueFile& keyValueFile, bool read);
     public:
        virtual void loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile);
        virtual void saveKeyValueFile(jafar::kernel::KeyValueFile& keyValueFile);
    } configEstimation;


};

#endif // RTSLAM_HPP

/**
 * @}
 * @}
 */
