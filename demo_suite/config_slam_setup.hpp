
#if 0
///-----------------------------------------------------------------------------
/// SENSOR
///-----------------------------------------------------------------------------

//-- Sensor Pose // x,y,z,roll,pitch,yaw
// camera alone
const double SENSOR_POSE_CONSTVEL[6] = {0, 0, 0, -90, 0, -90};
// camera and IMU sticked together
const double SENSOR_POSE_INERTIAL[6] = {0.04, 0.01, -0.035, 90, 0, 90};
//const double SENSOR_POSE_INERTIAL[6] = {0.0, 0.0, 0.0, 90, 0, 90};
// IMU on Mana on CPU, camera left
//const double SENSOR_POSE_INERTIAL[6] = {0.47, 0.153, 0.122, -90, 0, -90};
// IMU on Mana on platine, camera left
//const double SENSOR_POSE_INERTIAL[6] = {0, 0, 0, -90, 0, -90};


/*// flea2 with original obj
const std::string CAMERA_DEVICE = "0x00b09d01006fb38f";
const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 301.27013,   266.86136,   497.28243,   496.81116 };
const double DISTORTION[3] = { -0.23193,   0.11306, 0.0 }; //{-0.27965, 0.20059, -0.14215}; //{-0.27572, 0.28827};
*/

/*// flea2 with original obj after 2010/11/24
const std::string CAMERA_DEVICE = "0x00b09d01006fb38f";
const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 306.2969,   264.7741,   499.9177,   494.4829 };
const double DISTORTION[3] = { -0.2293129, 0.08793152, -0.01349877 };
*/

/*// flea2 with original obj after 2010/11/24, 320x240
const std::string CAMERA_DEVICE = "0x00b09d01006fb38f";
const unsigned IMG_WIDTH = 320;
const unsigned IMG_HEIGHT = 240;
const double INTRINSIC[4] = { 153.148, 132.387, 249.959, 247.241 };
const double DISTORTION[3] = { -0.2293129, 0.08793152, -0.01349877 };
*/


/*// flea2 with 4.5 obj after 2011/02/14
const std::string CAMERA_DEVICE = "0x00b09d010063565b";
const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 323.5790, 248.6014, 622.3523, 622.6011 };
const double DISTORTION[3] = { -0.3335163, 0.2082654, -0.09450253 };
*/

// flea2 with 4.5 obj after 2011/02/16
const std::string CAMERA_DEVICE = "0x00b09d010063565b";
const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 327.3508, 249.9982, 622.0489, 621.6159 };
const double DISTORTION[3] = { -0.3270245, 0.1944629, -0.09558001 };

/*// flea2 with Schneider lens
const std::string CAMERA_DEVICE = "0x00b09d01006fb38f";
const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 3.043313e+02, 2.407406e+02, 6.714993e+02, 6.714853e+02};
const double DISTORTION[3] = { -2.900038e-01, 2.388370e-01, -2.006549e-01 };
*/

/*// flea2 left on Mana
const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 3.220449e+02, 2.322685e+02, 6.723964e+02, 6.718552e+02};
const double DISTORTION[3] = {-2.889972e-01, 2.511945e-01, -2.147339e-01 };
*/

// Flea left on robuFast (half-scale)
//const unsigned IMG_WIDTH = 512;
//const unsigned IMG_HEIGHT = 384;
//const double INTRINSIC[4] = { 268.44175,   197.6454,  535.7405, 535.0865 };
//const double DISTORTION[3] = { -0.2927602, 0.250071, -0.2086941};


/*// flea1 montoldre october 2010
const std::string CAMERA_DEVICE = "";
const unsigned IMG_WIDTH = 512;
const unsigned IMG_HEIGHT = 384;
const double INTRINSIC[4] = { 268.44175, 197.6454, 535.7405, 535.0865};
//const double DISTORTION[3] = { -1.1710408, 4.0321136, -13.3564224 };
const double DISTORTION[3] = { -2.927602e-01, 2.520071e-01, -2.086941e-01 };
*/

/*// jmcodol's robot
const std::string CAMERA_DEVICE = "0x00b09d01006fb38f";
const unsigned IMG_WIDTH = 640;
const unsigned IMG_HEIGHT = 480;
const double INTRINSIC[4] = { 327.53722,   222.40418,   533.18050,   531.56182 };
//const double DISTORTION[2] = { 0.08577,   -0.22006, 0.0 };
const double DISTORTION[3] = { 0.0, 0.0};
*/

/*// saragosse sequence
const std::string CAMERA_DEVICE = "0x00b09d01006fb38f";
const unsigned IMG_WIDTH = 512;
const unsigned IMG_HEIGHT = 384;
const double INTRINSIC[4] = { 281.647017175628, 198.770772126498,   534.760390823972,   535.280428739968 };
const double DISTORTION[3] = { -0.27546592313146917, 0.12644899941674098, 0.036648747902512463 };
*/

/*// Vitana IR
	//const std::string CAMERA_DEVICE = "0x0001687300025581";
	const std::string CAMERA_DEVICE = "0x0001687300025853";
	const unsigned IMG_WIDTH = 160;
	const unsigned IMG_HEIGHT = 120;
	const double INTRINSIC[4] = { 86.54042,   63.63390,  216.52344,   217.71205 };
	const double DISTORTION[3] = { -0.31580,   0.40753,   0.00000 };
*/


///-----------------------------------------------------------------------------
/// SIMU SENSOR
///-----------------------------------------------------------------------------

const unsigned IMG_WIDTH_SIMU = 640;
const unsigned IMG_HEIGHT_SIMU = 480;
const double INTRINSIC_SIMU[4] = { 320.0,   240.0,   500.0,   500.0 };
const double DISTORTION_SIMU[3] = { -0.25,   0.10, 0.0 };

///-----------------------------------------------------------------------------
/// CONSTANT VELOCITY
///-----------------------------------------------------------------------------

const double UNCERT_VLIN = .5; // m/s
const double UNCERT_VANG = .5; // rad/s
const double PERT_VLIN = 2.0; // m/s per sqrt(s)
const double PERT_VANG = 2.0; // rad/s per sqrt(s)

///-----------------------------------------------------------------------------
/// INERTIAL
///-----------------------------------------------------------------------------

const std::string MTI_DEVICE = "/dev/ttyUSB0";

// inertial robot initial uncertainties and perturbations - in addition to constant velocity option UNCERT_VLIN.
const double UNCERT_GRAVITY = 0.01 * 9.81; // 1% m/s^2
const double UNCERT_ABIAS = 0.01*17.0; // 1% of full scale
const double UNCERT_WBIAS = 0.01*jmath::degToRad(300.0); // 1% of full scale
/*
There is a 1.4 factor for gyros because the experimental noise (0.006)
is slightly larger than computed noise (0.0055). For acceleros experimental
noise (0.009) is compatible with computed noise (0.0011)
 */
const double PERT_AERR = 1.0 * 0.002*sqrt(30.0); // m/s2, IMU acc error (MTI = 0.002*sqrt(30Hz) m/s2)
const double PERT_WERR = 1.4 * jmath::degToRad(0.05)*sqrt(40.0); // rad/s, IMU gyro error (MTI = 0.05*sqrt(40Hz) deg/s)
const double PERT_RANWALKACC = 0; // m/s^2 per sqrt(s), IMU a_bias random walk
const double PERT_RANWALKGYRO = 0; // rad/s^2 per sqrt(s), IMU w_bias random walk

const double IMU_TIMESTAMP_CORRECTION = 0.010;//0.007;


///-----------------------------------------------------------------------------
/// SIMU INERTIAL
///-----------------------------------------------------------------------------

const double SIMU_IMU_TIMESTAMP_CORRECTION = 0.000;
const double SIMU_IMU_FREQ = 120.0;
const double SIMU_IMU_GRAVITY = 9.81;
const double SIMU_IMU_GYR_BIAS = 0.0;
const double SIMU_IMU_GYR_BIAS_NOISESTD = 0.0;
const double SIMU_IMU_GYR_GAIN = 0.0;
const double SIMU_IMU_GYR_GAIN_NOISESTD = 0.0;
const double SIMU_IMU_RANDWALKGYR_FACTOR = 0.0;
const double SIMU_IMU_ACC_BIAS = 0.0;
const double SIMU_IMU_ACC_BIAS_NOISESTD = 0.0;
const double SIMU_IMU_ACC_GAIN = 0.0;
const double SIMU_IMU_ACC_GAIN_NOISESTD = 0.0;
const double SIMU_IMU_RANDWALKACC_FACTOR = 0.0;



#endif


#include "kernel/keyValueFile.hpp"

#define KeyValueFile_getItem(k) keyValueFile.getItem(#k, k);
#define KeyValueFile_setItem(k) keyValueFile.setItem(#k, k);



class ConfigSetup: public kernel::KeyValueFileSaveLoad
{
 public:
	/// SENSOR
	jblas::vec6 SENSOR_POSE_CONSTVEL; /// sensor pose in constant velocity (x,y,z,roll,pitch,yaw) (deg)
	jblas::vec6 SENSOR_POSE_INERTIAL; /// sensor pose in inertial (x,y,z,roll,pitch,yaw) (deg)

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
	double PERT_AERR;        /// noise stdev of accelerometers (% of ACCELERO_NOISE)
	double PERT_WERR;        /// noise stdev of gyrometers (% of GYRO_NOISE)
	double PERT_RANWALKACC;  /// IMU a_bias random walk (m/s2 per sqrt(s))
	double PERT_RANWALKGYRO; /// IMU w_bias random walk (rad/s per sqrt(s))

	double IMU_TIMESTAMP_CORRECTION; /// correction to add to the IMU timestamp for synchronization (s)

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
	
 public:
	virtual void loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile)
	{
		KeyValueFile_getItem(SENSOR_POSE_CONSTVEL);
		KeyValueFile_getItem(SENSOR_POSE_INERTIAL);
		
		KeyValueFile_getItem(CAMERA_DEVICE);
		KeyValueFile_getItem(IMG_WIDTH);
		KeyValueFile_getItem(IMG_HEIGHT);
		KeyValueFile_getItem(INTRINSIC);
		KeyValueFile_getItem(DISTORTION);
		
		KeyValueFile_getItem(IMG_WIDTH_SIMU);
		KeyValueFile_getItem(IMG_HEIGHT_SIMU);
		KeyValueFile_getItem(INTRINSIC_SIMU);
		KeyValueFile_getItem(DISTORTION_SIMU);
		
		KeyValueFile_getItem(UNCERT_VLIN);
		KeyValueFile_getItem(UNCERT_VANG);
		KeyValueFile_getItem(PERT_VLIN);
		KeyValueFile_getItem(PERT_VANG);
		
		KeyValueFile_getItem(MTI_DEVICE);
		KeyValueFile_getItem(ACCELERO_FULLSCALE);
		KeyValueFile_getItem(ACCELERO_NOISE);
		KeyValueFile_getItem(GYRO_FULLSCALE);
		KeyValueFile_getItem(GYRO_NOISE);
		
		KeyValueFile_getItem(UNCERT_GRAVITY);
		KeyValueFile_getItem(UNCERT_ABIAS);
		KeyValueFile_getItem(UNCERT_WBIAS);
		KeyValueFile_getItem(PERT_AERR);
		KeyValueFile_getItem(PERT_WERR);
		KeyValueFile_getItem(PERT_RANWALKACC);
		KeyValueFile_getItem(PERT_RANWALKGYRO);
		
		KeyValueFile_getItem(IMU_TIMESTAMP_CORRECTION);
		
		KeyValueFile_getItem(SIMU_IMU_TIMESTAMP_CORRECTION);
		KeyValueFile_getItem(SIMU_IMU_FREQ);
		KeyValueFile_getItem(SIMU_IMU_GRAVITY);
		KeyValueFile_getItem(SIMU_IMU_GYR_BIAS);
		KeyValueFile_getItem(SIMU_IMU_GYR_BIAS_NOISESTD);
		KeyValueFile_getItem(SIMU_IMU_GYR_GAIN);
		KeyValueFile_getItem(SIMU_IMU_GYR_GAIN_NOISESTD);
		KeyValueFile_getItem(SIMU_IMU_RANDWALKGYR_FACTOR);
		KeyValueFile_getItem(SIMU_IMU_ACC_BIAS);
		KeyValueFile_getItem(SIMU_IMU_ACC_BIAS_NOISESTD);
		KeyValueFile_getItem(SIMU_IMU_ACC_GAIN);
		KeyValueFile_getItem(SIMU_IMU_ACC_GAIN_NOISESTD);
		KeyValueFile_getItem(SIMU_IMU_RANDWALKACC_FACTOR);
	}
	
	virtual void saveKeyValueFile(jafar::kernel::KeyValueFile& keyValueFile)
	{
		KeyValueFile_setItem(SENSOR_POSE_CONSTVEL);
		KeyValueFile_setItem(SENSOR_POSE_INERTIAL);
		
		KeyValueFile_setItem(CAMERA_DEVICE);
		KeyValueFile_setItem(IMG_WIDTH);
		KeyValueFile_setItem(IMG_HEIGHT);
		KeyValueFile_setItem(INTRINSIC);
		KeyValueFile_setItem(DISTORTION);
		
		KeyValueFile_setItem(IMG_WIDTH_SIMU);
		KeyValueFile_setItem(IMG_HEIGHT_SIMU);
		KeyValueFile_setItem(INTRINSIC_SIMU);
		KeyValueFile_setItem(DISTORTION_SIMU);
		
		KeyValueFile_setItem(UNCERT_VLIN);
		KeyValueFile_setItem(UNCERT_VANG);
		KeyValueFile_setItem(PERT_VLIN);
		KeyValueFile_setItem(PERT_VANG);
		
		KeyValueFile_setItem(MTI_DEVICE);
		KeyValueFile_setItem(ACCELERO_FULLSCALE);
		KeyValueFile_setItem(ACCELERO_NOISE);
		KeyValueFile_setItem(GYRO_FULLSCALE);
		KeyValueFile_setItem(GYRO_NOISE);
		
		KeyValueFile_setItem(UNCERT_GRAVITY);
		KeyValueFile_setItem(UNCERT_ABIAS);
		KeyValueFile_setItem(UNCERT_WBIAS);
		KeyValueFile_setItem(PERT_AERR);
		KeyValueFile_setItem(PERT_WERR);
		KeyValueFile_setItem(PERT_RANWALKACC);
		KeyValueFile_setItem(PERT_RANWALKGYRO);
		
		KeyValueFile_setItem(IMU_TIMESTAMP_CORRECTION);
		
		KeyValueFile_setItem(SIMU_IMU_TIMESTAMP_CORRECTION);
		KeyValueFile_setItem(SIMU_IMU_FREQ);
		KeyValueFile_setItem(SIMU_IMU_GRAVITY);
		KeyValueFile_setItem(SIMU_IMU_GYR_BIAS);
		KeyValueFile_setItem(SIMU_IMU_GYR_BIAS_NOISESTD);
		KeyValueFile_setItem(SIMU_IMU_GYR_GAIN);
		KeyValueFile_setItem(SIMU_IMU_GYR_GAIN_NOISESTD);
		KeyValueFile_setItem(SIMU_IMU_RANDWALKGYR_FACTOR);
		KeyValueFile_setItem(SIMU_IMU_ACC_BIAS);
		KeyValueFile_setItem(SIMU_IMU_ACC_BIAS_NOISESTD);
		KeyValueFile_setItem(SIMU_IMU_ACC_GAIN);
		KeyValueFile_setItem(SIMU_IMU_ACC_GAIN_NOISESTD);
		KeyValueFile_setItem(SIMU_IMU_RANDWALKACC_FACTOR);
	}
} configSetup;


