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

