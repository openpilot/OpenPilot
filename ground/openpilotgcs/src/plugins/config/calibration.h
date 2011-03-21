#ifndef AHRS_CALIBRATION_HPP
#define AHRS_CALIBRATION_HPP

#include <Eigen/Core>
#include <cstdlib>
using std::size_t;
using namespace Eigen;

void
calibration_misalignment(Vector3f& rotationVector,
		const Vector3f samples0[],
		const Vector3f& reference0,
		const Vector3f samples1[],
		const Vector3f& reference1,
		size_t n_samples);

Vector3f
twostep_bias_only(const Vector3f samples[], 
		size_t n_samples,
		const Vector3f& referenceField,
		const float noise);

void 
twostep_bias_scale(Vector3f& bias, 
		Vector3f& scale, 
		const Vector3f samples[], 
		const size_t n_samples,
		const Vector3f& referenceField,
		const float noise);

void 
twostep_bias_scale(Vector3f& bias, 
		Matrix3f& scale, 
		const Vector3f samples[], 
		const size_t n_samples,
		const Vector3f& referenceField,
		const float noise);

void
openpilot_bias_scale(Vector3f& bias,
		Vector3f& scale, 
		const Vector3f samples[], 
		const size_t n_samples,
		const Vector3f& referenceField);

void
gyroscope_calibration(Vector3f& bias,
		Matrix3f& accelSensitivity,
		Vector3f gyroSamples[],
		Vector3f accelSamples[],
		size_t n_samples);

#endif // !defined AHRS_CALIBRATION_HPP

