/**
 ******************************************************************************
 *
 * @file       legacy-calibration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#include <calibration.h>
#include <Eigen/LU>

/**
 * The basic calibration algorithm initially used in OpenPilot. This is a basic
 * 6-point calibration that doesn't attempt to account for any of the white noise
 * in the sensors.
 * The measurement equation is
 * B_k = D^-1 * (A_k * H_k - b)
 * Where B_k is the measurement of the field at time k
 * 	D is the diagonal matrix of scale factors
 * 	A_k is the attitude direction cosine matrix at time k
 *  H_k is the reference field at the kth sample
 *  b is the vector of scale factors.
 *
 * After computing the scale factor and bias, the optimized measurement is
 * \tilde{B}_k = D * (B_k + b)
 *
 * @param bias[out] The computed bias of the sensor
 * @param scale[out] The computed scale factor of the sensor
 * @param samples An array of sample data points.  Only the first 6 are
 * 	used.
 * @param n_samples The number of sample data points.  Must be at least 6.
 * @param referenceField The field being measured by the sensor.
 */
void
openpilot_bias_scale(Vector3f& bias,
		Vector3f& scale, 
		const Vector3f samples[], 
		const size_t n_samples,
		const Vector3f& referenceField)
{
	// TODO: Add error handling, and return error codes through the return
	// value.
	if (n_samples < 6) {
		bias = Vector3f::Zero();
		scale = Vector3f::Zero();
		return;
	}
	// mag = (S*x + b)**2
	// mag = (S**2 * x**2 + 2*S*b*x + b*b)
	// 0 = S**2 * (x1**2 - x2**2) + 2*S*B*(x1 - x2))
	// Fill in matrix A -
	// write six difference-in-magnitude equations of the form
	// Sx^2(x2^2-x1^2) + 2*Sx*bx*(x2-x1) + Sy^2(y2^2-y1^2) + 
	// 2*Sy*by*(y2-y1) + Sz^2(z2^2-z1^2) + 2*Sz*bz*(z2-z1) = 0
	//
	// or in other words
	// 2*Sx*bx*(x2-x1)/Sx^2  + Sy^2(y2^2-y1^2)/Sx^2  +
	// 2*Sy*by*(y2-y1)/Sx^2  + Sz^2(z2^2-z1^2)/Sx^2  + 2*Sz*bz*(z2-z1)/Sx^2  =
	// 	 (x1^2-x2^2)
	Matrix<float, 5, 5> A;
	Matrix<float, 5, 1> f;
	for (unsigned i=0; i<5; i++){
		A.row(i)[0] = 2.0 * (samples[i+1].x() - samples[i].x());
		A.row(i)[1] = samples[i+1].y()* samples[i+1].y() - samples[i].y()*samples[i].y();
		A.row(i)[2] = 2.0 * (samples[i+1].y() - samples[i].y());
		A.row(i)[3] = samples[i+1].z()*samples[i+1].z() - samples[i].z()*samples[i].z();
		A.row(i)[4] = 2.0 * (samples[i+1].z() - samples[i].z());
		f[i]    = samples[i].x()*samples[i].x() - samples[i+1].x()*samples[i+1].x();
	}
	Matrix<float, 5, 1> c;
	A.lu().solve(f, &c);


	// use one magnitude equation and c's to find Sx - doesn't matter which - all give the same answer
	float xp = samples[0].x();
    float yp = samples[0].y();
	float zp = samples[0].z();

	float Sx = sqrt(referenceField.squaredNorm() / 
			(xp*xp + 2*c[0]*xp + c[0]*c[0] + c[1]*yp*yp + 2*c[2]*yp + c[2]*c[2]/c[1] + c[3]*zp*zp + 2*c[4]*zp + c[4]*c[4]/c[3]));

	scale[0] = Sx;
	bias[0] = Sx*c[0];
	scale[1] = sqrt(c[1]*Sx*Sx);
	bias[1] = c[2]*Sx*Sx/scale[1];
	scale[2] = sqrt(c[3]*Sx*Sx);
	bias[2] = c[4]*Sx*Sx/scale[2];

	for (int i = 0; i < 3; ++i) {
		// Fixup difference between openpilot measurement model and twostep
		// version
		bias[i] = -bias[i] / scale[i];
	}
}


