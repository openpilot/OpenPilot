
#include <calibration.h>
#include <Eigen/Cholesky>
#include <Eigen/SVD>
#include <Eigen/QR>

/*
 * Compute basic calibration parameters for a three axis gyroscope.
 * The measurement equation is
 * gyro_k = accelSensitivity * \tilde{accel}_k + bias + omega
 *
 * 	where, omega is the true angular rate (assumed to be zero)
 * 		bias is the sensor bias
 * 		accelSensitivity is the 3x3 matrix of sensitivity scale factors
 * 		\tilde{accel}_k is the calibrated measurement of the accelerometer
 * 			at time k
 *
 * After calibration, the optimized gyro measurement is given by
 * \tilde{gyro}_k = gyro_k - bias - accelSensitivity * \tilde{accel}_k
 */
void
gyroscope_calibration(Vector3f& bias,
		Matrix3f& accelSensitivity,
		Vector3f gyroSamples[],
		Vector3f accelSamples[],
		size_t n_samples)
{
	// Assume the following measurement model:
	// y = H*x
	// where x is the vector of unknowns, and y is the measurement vector.
	// the vector of unknowns is
	// [a_x a_y a_z b_x]
	// The measurement vector is
	// [gyro_x]
	// and the measurement matrix H is
	// [accelSample_x accelSample_y accelSample_z 1]

	// Note that the individual solutions for x are given by
	// (H^T \times H)^-1 \times H^T y
	// Everything is identical except for y and x.  So, transform it
	// into block form X = (H^T \times H)^-1 \times H^T Y
	// where each column of X contains the partial solution for each
	// column of y.

	// Construct the matrix of accelerometer samples.  Intermediate results
	// are computed in "twice the precision that the source provides and the
	// result deserves" by Kahan's thumbrule to prevent numerical problems.
	Matrix<double, Dynamic, 4> H(n_samples, 4);
	// And the matrix of gyro samples.
	Matrix<double, Dynamic, 3> Y(n_samples, 3);
	for (unsigned i = 0; i < n_samples; ++i) {
		H.row(i) << accelSamples[i].transpose().cast<double>(), 1.0;
		Y.row(i) << gyroSamples[i].transpose().cast<double>();
	}

#if 1
	Matrix<double, 4, 3> result;
	// Use the cholesky-based Penrose pseudoinverse method.
	(H.transpose() * H).ldlt().solve(H.transpose()*Y, &result);

	// Transpose the result and return it to the caller.  Cast back to float
	// since there really isn't that much accuracy in the result.
	bias = result.row(3).transpose().cast<float>();
	accelSensitivity = result.block<3, 3>(0, 0).transpose().cast<float>();
#else
	// TODO: Compare this result with a total-least-squares model.
	size_t n = 4;
	Matrix<double, Dynamic, 7> C;
	C << H, Y;
	SVD<Matrix<double, Dynamic, 7> > usv(C);
	Matrix<double, 4, 3> V_ab = usv.matrixV().block<4, 3>(0, n);
	Matrix<double, Dynamic, 3> V_bb = usv.matrixV().corner(BottomRight, n_samples-4, 3);
	// X = -V_ab/V_bb;
	// X^T = (A * B^-1)^T
	// X^T = (B^-1^T * A^T)
	// X^T = (B^T^-1 * A^T)
	// V_bb is orthgonal but not orthonormal.  QR decomposition
	// should be very fast in this case.
	Matrix<double, 3, 4> result;
	V_bb.transpose().qr().solve(-V_ab.transpose(), &result);

	// Results only deserve single precision.
	bias = result.col(3).cast<float>();
	accelSensitivity = result.block<3, 3>(0, 0).cast<float>();

#endif
}
