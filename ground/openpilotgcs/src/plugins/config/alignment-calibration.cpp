#include "calibration.h"
#include <Eigen/Core>
#include <Eigen/Cholesky>
#include <Eigen/Geometry>

using namespace Eigen;

/** Calibrate the angular misalignment of one field sensor relative to another.
 *
 * @param rotationVector[out] The rotation vector that rotates sensor 1 such
 * that its principle axes are colinear with the axes of sensor 0.
 * @param samples0[in] A list of samples of the field observed by the reference
 * sensor.
 * @param reference0[in] The common value of the reference field in the inertial
 * reference frame.
 * @param samples1[in] The list of samples taken by the sensor to be aligned to
 * the reference.  The attitude of the sensor head as a whole must be identical
 * between samples0[i] and samples1[i] for all i.
 * @param reference1[in] The actual value of the second field in the inertial
 * reference frame.
 * @param n_samples The number of samples.
 */
void
calibration_misalignment(Vector3f& rotationVector,
		const Vector3f samples0[],
		const Vector3f& reference0,
		const Vector3f samples1[],
		const Vector3f& reference1,
		size_t n_samples)
{
	// Note that this implementation makes the assumption that the angular
	// misalignment is small.  Something based on QUEST would be needed to
	// account for errors larger than a few degrees.
	Matrix<double, Dynamic, 3> X(n_samples, 3);
	Matrix<double, Dynamic, 1> y(n_samples, 1);

	AngleAxisd reference(Quaterniond().setFromTwoVectors(
			reference0.cast<double>(), reference1.cast<double>()));
	for (size_t i = 0; i < n_samples; ++i) {
		AngleAxisd observation(Quaterniond().setFromTwoVectors(
			samples0[i].cast<double>(), samples1[i].cast<double>()));

		X.row(i) = observation.axis();
		y[i] = reference.angle() - observation.angle();
	}

	// Run linear least squares over the result.
	Vector3d result;
	(X.transpose() * X).ldlt().solve(X.transpose()*y, &result);
	rotationVector = result.cast<float>();
}
