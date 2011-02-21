

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
