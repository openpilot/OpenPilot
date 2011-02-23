
#if 0

///-----------------------------------------------------------------------------
/// MISC
///-----------------------------------------------------------------------------

const unsigned CORRECTION_SIZE = 4;

///-----------------------------------------------------------------------------
/// FILTER
///-----------------------------------------------------------------------------

// map size in # of states : 13 for robot plus 100 3D-landmarks
const unsigned MAP_SIZE = 500;

//
const double PIX_NOISE = 1.0;
const double PIX_NOISE_SIMUFACTOR = 0.5;

///-----------------------------------------------------------------------------
/// LANDMARKS
///-----------------------------------------------------------------------------

// lmk management
const double D_MIN = .5; /// inverse depth mean initialization
const double REPARAM_TH = 0.1; /// reparametrization threshold

///--------------------------------
/// landmark repartition

const unsigned GRID_HCELLS = 3;
const unsigned GRID_VCELLS = 3;
const unsigned GRID_MARGIN = 11;
const unsigned GRID_SEPAR = 20;

///--------------------------------
/// landmark updates

const double RELEVANCE_TH = 2.0; // in n_sigmas
const double MAHALANOBIS_TH = 3.0; // in n_sigmas
const unsigned N_UPDATES_TOTAL = 20;
const unsigned N_UPDATES_RANSAC = 15;
const unsigned N_INIT = 10;
const unsigned N_RECOMP_GAINS = 3;
const double RANSAC_LOW_INNOV = 1.0; // in pixels

#if RANSAC_FIRST
const unsigned RANSAC_NTRIES = 6;
#else
const unsigned RANSAC_NTRIES = 0;
#endif


///-----------------------------------------------------------------------------
/// RAW PROCESSING
///-----------------------------------------------------------------------------

///--------------------------------
/// DETECTION

const unsigned HARRIS_CONV_SIZE = 5;
const double HARRIS_TH = 15.0;
const double HARRIS_EDDGE = 1.4;

///--------------------------------
/// DESCRIPTOR

const unsigned DESC_SIZE = 31;
#if MULTIVIEW_DESCRIPTOR
const double DESC_SCALE_STEP = 2.0;
const double DESC_ANGLE_STEP = jmath::degToRad(10.0);
const DescriptorImagePointMultiView::PredictionType DESC_PREDICTION_TYPE = DescriptorImagePointMultiView::ptAffine;
#endif

///--------------------------------
/// MATCHING

const unsigned PATCH_SIZE = 13; // in pixels
const unsigned MAX_SEARCH_SIZE = 50000; // in number of pixels
const unsigned KILL_SEARCH_SIZE = 100000; // in number of pixels
#if MULTIVIEW_DESCRIPTOR
const double MATCH_TH = 0.95;
#else
const double MATCH_TH = 0.90;
#endif
const double MIN_SCORE = 0.85;
const double PARTIAL_POSITION = 0.25;





#include "kernel/keyValueFile.hpp"

#define KeyValueFile_getItem(k) keyValueFile.getItem(#k, k);
#define KeyValueFile_setItem(k) keyValueFile.setItem(#k, k);

#endif


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
	double DESC_SCALE_STEP; /// MultiviewDescriptor: min change of scale (ratio)
	double DESC_ANGLE_STEP; /// MultiviewDescriptor: min change of point of view (deg)
	int DESC_PREDICTION_TYPE; /// type of prediction from descriptor (0 = none, 1 = affine, 2 = homographic)

	unsigned PATCH_SIZE;       /// patch size used for matching
	unsigned MAX_SEARCH_SIZE;  /// if the search area is larger than this # of pixels, we bound it
	unsigned KILL_SEARCH_SIZE; /// if the search area is larger than this # of pixels, we vote for killing the landmark
	double MATCH_TH;           /// ZNCC score threshold
	double MIN_SCORE;          /// min ZNCC score under which we don't finish to compute the value of the score
	double PARTIAL_POSITION;   /// position in the patch where we test if we finish the correlation computation
	
 public:
	virtual void loadKeyValueFile(jafar::kernel::KeyValueFile const& keyValueFile)
	{
		KeyValueFile_getItem(CORRECTION_SIZE);
		
		KeyValueFile_getItem(MAP_SIZE);
		KeyValueFile_getItem(PIX_NOISE);
		KeyValueFile_getItem(PIX_NOISE_SIMUFACTOR);
		
		KeyValueFile_getItem(D_MIN);
		KeyValueFile_getItem(REPARAM_TH);
		
		KeyValueFile_getItem(GRID_HCELLS);
		KeyValueFile_getItem(GRID_VCELLS);
		KeyValueFile_getItem(GRID_MARGIN);
		KeyValueFile_getItem(GRID_SEPAR);
		
		KeyValueFile_getItem(RELEVANCE_TH);
		KeyValueFile_getItem(MAHALANOBIS_TH);
		KeyValueFile_getItem(N_UPDATES_TOTAL);
		KeyValueFile_getItem(N_UPDATES_RANSAC);
		KeyValueFile_getItem(N_INIT);
		KeyValueFile_getItem(N_RECOMP_GAINS);
		KeyValueFile_getItem(RANSAC_LOW_INNOV);
		
		KeyValueFile_getItem(RANSAC_NTRIES);
		
		KeyValueFile_getItem(HARRIS_CONV_SIZE);
		KeyValueFile_getItem(HARRIS_TH);
		KeyValueFile_getItem(HARRIS_EDDGE);
		
		KeyValueFile_getItem(DESC_SIZE);
		KeyValueFile_getItem(DESC_SCALE_STEP);
		KeyValueFile_getItem(DESC_ANGLE_STEP);
		KeyValueFile_getItem(DESC_PREDICTION_TYPE);
		
		KeyValueFile_getItem(PATCH_SIZE);
		KeyValueFile_getItem(MAX_SEARCH_SIZE);
		KeyValueFile_getItem(KILL_SEARCH_SIZE);
		KeyValueFile_getItem(MATCH_TH);
		KeyValueFile_getItem(MIN_SCORE);
		KeyValueFile_getItem(PARTIAL_POSITION);
	}
	
	virtual void saveKeyValueFile(jafar::kernel::KeyValueFile& keyValueFile)
	{
		KeyValueFile_setItem(CORRECTION_SIZE);
		
		KeyValueFile_setItem(MAP_SIZE);
		KeyValueFile_setItem(PIX_NOISE);
		KeyValueFile_setItem(PIX_NOISE_SIMUFACTOR);
		
		KeyValueFile_setItem(D_MIN);
		KeyValueFile_setItem(REPARAM_TH);
		
		KeyValueFile_setItem(GRID_HCELLS);
		KeyValueFile_setItem(GRID_VCELLS);
		KeyValueFile_setItem(GRID_MARGIN);
		KeyValueFile_setItem(GRID_SEPAR);
		
		KeyValueFile_setItem(RELEVANCE_TH);
		KeyValueFile_setItem(MAHALANOBIS_TH);
		KeyValueFile_setItem(N_UPDATES_TOTAL);
		KeyValueFile_setItem(N_UPDATES_RANSAC);
		KeyValueFile_setItem(N_INIT);
		KeyValueFile_setItem(N_RECOMP_GAINS);
		KeyValueFile_setItem(RANSAC_LOW_INNOV);
		
		KeyValueFile_setItem(RANSAC_NTRIES);
		
		KeyValueFile_setItem(HARRIS_CONV_SIZE);
		KeyValueFile_setItem(HARRIS_TH);
		KeyValueFile_setItem(HARRIS_EDDGE);
		
		KeyValueFile_setItem(DESC_SIZE);
		KeyValueFile_setItem(DESC_SCALE_STEP);
		KeyValueFile_setItem(DESC_ANGLE_STEP);
		KeyValueFile_setItem(DESC_PREDICTION_TYPE);
		
		KeyValueFile_setItem(PATCH_SIZE);
		KeyValueFile_setItem(MAX_SEARCH_SIZE);
		KeyValueFile_setItem(KILL_SEARCH_SIZE);
		KeyValueFile_setItem(MATCH_TH);
		KeyValueFile_setItem(MIN_SCORE);
		KeyValueFile_setItem(PARTIAL_POSITION);
	}
} configEstimation;


