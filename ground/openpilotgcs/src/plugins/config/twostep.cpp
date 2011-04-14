/**
 ******************************************************************************
 *
 * @file       twostep.cpp
 * @author     Jonathan Brandmeyer <jonathan.brandmeyer@gmail.com>
 * 	Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup Config plugin
 * @{
 * @brief Implements low-level calibration algorithms
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; Version 3 of the License, and no other
 * version.
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
#include <assertions.h>

#include <Eigen/Cholesky>
#include <Eigen/QR>
#include <Eigen/LU>
#include <cstdlib>

#undef PRINTF_DEBUGGING

#include <iostream>
#include <iomanip>

#if defined(__APPLE__) || defined(_WIN32)
  // Qt bug work around
  #ifndef isnan
	extern "C" int isnan(double);
  #endif
  #ifndef isinf
	extern "C" int isinf(double);
  #endif
#endif

/*
 * Alas, this implementation is a fairly direct copy of the contents of the
 * following papers.  You don't have a chance at understanding it without
 * reading these first.  Any bugs should be presumed to be JB's fault rather
 * than Alonso and Shuster until proven otherwise.
 *
 * Reference [1]: "A New Algorithm for Attitude-independent magnetometer
 * calibration", Robert Alonso and Malcolmn D. Shuster, Flight Mechanics/
 * Estimation Theory Symposium, NASA Goddard, 1994, pp 513-527
 *
 * Reference [2]: Roberto Alonso and Malcolm D. Shuster, "Complete Linear
 * Attitude-Independent Magnetometer Calibration", Journal of the Astronautical
 * Sciences, Vol 50, No 4, 2002, pp 477-490
 *
 */

namespace {
Vector3f center(const Vector3f samples[], 
				size_t n_samples)
{
	Vector3f summation(Vector3f::Zero());
	for (size_t i = 0; i < n_samples; ++i) {
		summation += samples[i];
	}
	return summation / float(n_samples);
}

void inv_fisher_information_matrix(Matrix3f& fisherInv,
		Matrix3f& fisher,
		const Vector3f samples[], 
		size_t n_samples,
		const float noise)
{
	MatrixXf column_samples(3, n_samples);
	for (size_t i = 0; i < n_samples; ++i) {
		column_samples.col(i) = samples[i];
	}
	fisher = 4 / noise * 
		column_samples * column_samples.transpose();
	// Compute the inverse by taking advantage of the results symmetricness
	fisher.ldlt().solve(Matrix3f::Identity(), &fisherInv);
	return;
}
// Eq 14 of the original reference.  Computes the gradiant of the cost function
// with respect to the bias offset.
// @param samples: The vector of measurement samples
// @param mags: The vector of sample delta magnitudes
// @param b: The estimate of the bias
// @param mu: The trace of the noise matrix (3*noise)
// @return the negative gradiant of the cost function with respect to the
// current value of the estimate, b
Vector3f
neg_dJdb(const Vector3f samples[], 
		const float mags[], 
		size_t n_samples,
		const Vector3f& b, 
		float mu,
		float noise)
{
	Vector3f summation(Vector3f::Zero());
	float b_norm2 = b.squaredNorm();
	
	for (size_t i = 0; i < n_samples; ++i) {
		summation += (mags[i] - 2*samples[i].dot(b) + b_norm2 - mu)
			*2* (samples[i] - b);
	}
	
	return (1.0 / noise)*summation;
}
} // !namespace (anon)

Vector3f twostep_bias_only(const Vector3f samples[], 
				size_t n_samples,
				const Vector3f& referenceField,
				const float noise)
{
	// \tilde{H}
	Vector3f centeredSamples[n_samples];
	// z_k
	float sampleDeltaMag[n_samples];
	// eq 7 and 8 applied to samples
	Vector3f avg = center(samples, n_samples);
	float refSquaredNorm = referenceField.squaredNorm();
	float sampleDeltaMagCenter = 0;
	for (size_t i = 0; i < n_samples; ++i)
	{
		// eq 9 applied to samples
		centeredSamples[i] = samples[i] - avg;
		// eqn 2a
		sampleDeltaMag[i] = samples[i].squaredNorm() - refSquaredNorm;
		sampleDeltaMagCenter += sampleDeltaMag[i];
	}
	sampleDeltaMagCenter /= n_samples;

	Matrix3f P_bb;
	Matrix3f P_bb_inv;
	// Due to eq 12b
	inv_fisher_information_matrix(P_bb, P_bb_inv, centeredSamples, n_samples, noise);
	// Compute centered magnitudes
	float sampleDeltaMagCentered[n_samples];
	for (size_t i = 0; i < n_samples; ++i) {
		sampleDeltaMagCentered[i] = sampleDeltaMag[i] - sampleDeltaMagCenter;
	}

	// From eq 12a	
	Vector3f estimate( Vector3f::Zero());
	for (size_t i = 0; i < n_samples; ++i) {
		estimate += sampleDeltaMagCentered[i] * centeredSamples[i];
	}
	estimate = P_bb * ((2 / noise) * estimate);

	// Newton-Raphson gradient descent to the optimal solution
	// eq 14a and 14b
	float mu = -3*noise;
	for (int i = 0; i < 6; ++i) {
		Vector3f neg_gradiant = neg_dJdb(samples, sampleDeltaMag, n_samples, 
				estimate, mu, noise);
		Matrix3f scale = P_bb_inv + 4/noise*(avg - estimate)*(avg - estimate).transpose();
		Vector3f neg_increment;
		scale.ldlt().solve(neg_gradiant, &neg_increment);
		// Note that the negative has been done twice
		estimate += neg_increment;
	}
	return estimate;
}

namespace {
// Private utility functions for the version of TWOSTEP that computes bias and
// scale factor vectors alone.

/** Compute the gradiant of the norm of b with respect to the estimate vector.  
 */
Matrix<double, 6, 1>
dnormb_dtheta(const Matrix<double, 6, 1>& theta)
{
	return (Matrix<double, 6, 1>() << 2*theta.coeff(0)/(1+theta.coeff(3)),
			2*theta.coeff(1)/(1+theta.coeff(4)),
			2*theta.coeff(2)/(1+theta.coeff(5)),
			-pow(theta.coeff(0), 2)/pow(1+theta.coeff(3), 2),
			-pow(theta.coeff(1), 2)/pow(1+theta.coeff(4), 2),
			-pow(theta.coeff(2), 2)/pow(1+theta.coeff(5), 2)).finished();
}

/**
 * Compute the gradiant of the cost function with respect to the optimal
 * estimate.
 */
Matrix<double, 6, 1>
dJdb(const Matrix<double, 6, 1>& centerL, 
		double centerMag,
		const Matrix<double, 6, 1>& theta, 
		const Matrix<double, 6, 1>& dbdtheta,
		double mu,
		double noise)
{
	// \frac{\delta}{\delta \theta'} |b(\theta')|^2
	Matrix<double, 6, 1> vectorPart = dbdtheta - centerL;

	// By equation 35
	double normbthetaSquared = 0;
	for (unsigned i = 0; i < 3; ++i) {
		normbthetaSquared += theta.coeff(i)*theta.coeff(i)/(1+theta.coeff(3+i));
	}
	double scalarPart = (centerMag - centerL.dot(theta) + normbthetaSquared
			- mu)/noise;
	return scalarPart * vectorPart;
}

/** Reconstruct the scale factor and bias offset vectors from the transformed
 * estimate vector.
 */
Matrix<double, 1, 6>
theta_to_sane(const Matrix<double, 6, 1>& theta)
{
	return (Matrix<double, 6, 1>() << 
		theta.coeff(0) / sqrt(1 + theta.coeff(3)),
		theta.coeff(1) / sqrt(1 + theta.coeff(4)),
		theta.coeff(2) / sqrt(1 + theta.coeff(5)),
		-1 + sqrt(1 + theta.coeff(3)),
		-1 + sqrt(1 + theta.coeff(4)),
		-1 + sqrt(1 + theta.coeff(5))).finished();
}

} // !namespace (anon)

/**
 * Compute the scale factor and bias associated with a vector observer
 * according to the equation:
 * B_k = (I_{3x3} + D)^{-1} \times (O^T A_k H_k + b + \epsilon_k)
 * where k is the sample index,
 *       B_k is the kth measurement
 *       I_{3x3} is a 3x3 identity matrix
 *       D is a 3x3 diagonal matrix of scale factors
 *       O is the orthogonal alignment matrix
 *       A_k is the attitude at the kth sample
 *       b is the bias in the global reference frame
 *       \epsilon_k is the noise at the kth sample
 * This implementation makes the assumption that \epsilon is a constant white,
 * gaussian noise source that is common to all k.  The misalignment matrix O
 * is not computed, and the off-diagonal elements of D, corresponding to the
 * misalignment scale factors are not either.
 *
 * @param bias[out] The computed bias, in the global frame
 * @param scale[out] The computed scale factor, in the sensor frame
 * @param samples[in] An array of measurement samples.
 * @param n_samples The number of samples in the array.
 * @param referenceField The magnetic field vector at this location. 
 * @param noise The one-sigma squared standard deviation of the observed noise
 * in the sensor.
 */
void twostep_bias_scale(Vector3f& bias, 
		Vector3f& scale, 
		const Vector3f samples[], 
		const size_t n_samples,
		const Vector3f& referenceField,
		const float noise)
{
	// Initial estimate for gradiant descent starts at eq 37a of ref 2.

	// Define L_k by eq 30 and 28 for k = 1 .. n_samples
	Matrix<double, Dynamic, 6> fullSamples(n_samples, 6);
	// \hbar{L} by eq 33, simplified by obesrving that the
	Matrix<double, 1, 6> centerSample = Matrix<double, 1, 6>::Zero();
	// Define the sample differences z_k by eq 23 a)
	double sampleDeltaMag[n_samples];
	// The center value \hbar{z}
	double sampleDeltaMagCenter = 0;
	double refSquaredNorm = referenceField.squaredNorm();

	for (size_t i = 0; i < n_samples; ++i) {
		fullSamples.row(i) << 2*samples[i].transpose().cast<double>(), 
			-(samples[i][0]*samples[i][0]),
			-(samples[i][1]*samples[i][1]),
			-(samples[i][2]*samples[i][2]);
		centerSample += fullSamples.row(i);

		sampleDeltaMag[i] = samples[i].squaredNorm() - refSquaredNorm;
		sampleDeltaMagCenter += sampleDeltaMag[i];
	}
	sampleDeltaMagCenter /= n_samples;
	centerSample /= n_samples;

	// True for all k.  
	// double mu = -3*noise;
	// The center value of mu, \bar{mu}
	// double centerMu = mu;
	// The center value of mu, \tilde{mu}
	// double centeredMu = 0;

	// Define \tilde{L}_k for k = 0 .. n_samples
	Matrix<double, Dynamic, 6> centeredSamples(n_samples, 6);
	// Define \tilde{z}_k for k = 0 .. n_samples
	double centeredMags[n_samples];
	// Compute the term under the summation of eq 37a
	Matrix<double, 6, 1> estimateSummation = Matrix<double, 6, 1>::Zero();
	for (size_t i = 0; i < n_samples; ++i) {
		centeredSamples.row(i) = fullSamples.row(i) - centerSample;
		centeredMags[i] = sampleDeltaMag[i] - sampleDeltaMagCenter;
		estimateSummation += centeredMags[i] * centeredSamples.row(i).transpose();
	}
	estimateSummation /= noise; // note: paper supplies 1/noise

	// By eq 37 b).  Note, paper supplies 1/noise here
	Matrix<double, 6, 6> P_theta_theta_inv = (1.0f/noise)*
			centeredSamples.transpose()*centeredSamples;
#ifdef PRINTF_DEBUGGING
	SelfAdjointEigenSolver<Matrix<double, 6, 6> > eig(P_theta_theta_inv);
	std::cout << "P_theta_theta_inverse: \n" << P_theta_theta_inv << "\n\n";
	std::cout << "P_\\tt^-1 eigenvalues: " << eig.eigenvalues().transpose()
			<< "\n";
	std::cout << "P_\\tt^-1 eigenvectors:\n" << eig.eigenvectors() << "\n";
#endif

	// The Fisher information matrix has a small eigenvalue, with a
	// corresponding eigenvector of about [1e-2 1e-2 1e-2 0.55, 0.55,
	// 0.55].  This means that there is relatively little information
	// about the common gain on the scale factor, which has a
	// corresponding effect on the bias, too. The fixup is performed by
	// the first iteration of the second stage of TWOSTEP, as documented in
	// [1].
	Matrix<double, 6, 1> estimate;
	// By eq 37 a), \tilde{Fisher}^-1
	P_theta_theta_inv.ldlt().solve(estimateSummation, &estimate);

#ifdef PRINTF_DEBUGGING
	Matrix<double, 6, 6> P_theta_theta;
	P_theta_theta_inv.ldlt().solve(Matrix<double, 6, 6>::Identity(), &P_theta_theta);
	SelfAdjointEigenSolver<Matrix<double, 6, 6> > eig2(P_theta_theta);
	std::cout << "eigenvalues: " << eig2.eigenvalues().transpose() << "\n";
	std::cout << "eigenvectors: \n" << eig2.eigenvectors() << "\n";
	std::cout << "estimate summation: \n" << estimateSummation.normalized()
		<< "\n";
#endif

	// estimate i+1 = estimate_i - Fisher^{-1}(at estimate_i)*gradiant(theta)
	// Fisher^{-1} = \tilde{Fisher}^-1 + \hbar{Fisher}^{-1}
	size_t count = 3;
	while (count --> 0) {
		// eq 40
		Matrix<double, 1, 6> db_dtheta = dnormb_dtheta(estimate);

		Matrix<double, 6, 1> dJ_dtheta = dJdb(centerSample, 
			sampleDeltaMagCenter,
			estimate,
			db_dtheta,
			-3*noise,
			noise/n_samples);


		// Eq 39 (with double-negation on term inside parens)
		db_dtheta = centerSample - db_dtheta;
	 	Matrix<double, 6, 6> scale = P_theta_theta_inv + 
			(double(n_samples)/noise)*db_dtheta.transpose() * db_dtheta;

		// eq 14b, mutatis mutandis (grumble, grumble)
		Matrix<double, 6, 1> increment;
		scale.ldlt().solve(dJ_dtheta, &increment);
		estimate -= increment;
	}
	
	// Transform the estimated parameters from [c | e] back into [b | d].
	for (size_t i = 0; i < 3; ++i) {
		scale.coeffRef(i) = -1 + sqrt(1 + estimate.coeff(3+i));
		bias.coeffRef(i) = estimate.coeff(i)/sqrt(1 + estimate.coeff(3+i));
	}
}

namespace {
// Private functions specific to the scale factor and orthogonal calibration 
// version of TWOSTEP

/** 
 * Reconstruct the symmetric E matrix from the last 6 elements of the estimate
 * vector
 */
Matrix3d 
E_theta(const Matrix<double, 9, 1>& theta)
{
	// By equation 49b
	return (Matrix3d() << 
			theta.coeff(3), theta.coeff(6), theta.coeff(7),
			theta.coeff(6), theta.coeff(4), theta.coeff(8),
			theta.coeff(7), theta.coeff(8), theta.coeff(5)).finished();
}

/**
 * Compute the gradient of the squared norm of b with respect to theta.  Note
 * that b itself is just a function of theta.  Therefore, this function
 * computes \frac{\delta,\delta\theta'}\left|b(\theta')\right|
 * From eq 55 of [2].
 */
Matrix<double, 9, 1>
dnormb_dtheta(const Matrix<double, 9, 1>& theta)
{
	// Re-form the matrix E from the vector of theta'
	Matrix3d E = E_theta(theta);
	// Compute (I + E)^-1 * c
	Vector3d IE_inv_c;
	E.ldlt().solve(theta.end<3>(), &IE_inv_c);

	return (Matrix<double, 9, 1>() << 2*IE_inv_c,
			-IE_inv_c.coeff(0)*IE_inv_c.coeff(0),
			-IE_inv_c.coeff(1)*IE_inv_c.coeff(1),
			-IE_inv_c.coeff(2)*IE_inv_c.coeff(2),

			-2*IE_inv_c.coeff(0)*IE_inv_c.coeff(1),
			-2*IE_inv_c.coeff(0)*IE_inv_c.coeff(2),
			-2*IE_inv_c.coeff(1)*IE_inv_c.coeff(2)).finished();
}

// The gradient of the cost function with respect to theta, at a particular
// value of theta.
// @param centerL: The center of the samples theta'
// @param centerMag: The center of the magnitude data
// @param theta: The estimate of the bias and scale factor
// @return the gradient of the cost function with respect to the
// current value of the estimate, theta'
Matrix<double, 9, 1>
dJ_dtheta(const Matrix<double, 9, 1>& centerL, 
		double centerMag,
		const Matrix<double, 9, 1>& theta, 
		const Matrix<double, 9, 1>& dbdtheta,
		double mu,
		double noise)
{
	// \frac{\delta}{\delta \theta'} |b(\theta')|^2
	Matrix<double, 9, 1> vectorPart = dbdtheta - centerL;

	// |b(\theta')|^2
	Matrix3d E = E_theta(theta);
	Vector3d rhs;
	(Matrix3d::Identity() + E).ldlt().solve(theta.start<3>(), &rhs);
	double normbthetaSquared = theta.start<3>().dot(rhs);

	double scalarPart = (centerMag - centerL.dot(theta) + normbthetaSquared
			- mu)/noise;
	return scalarPart * vectorPart;
}
} // !namespace (anonymous)

/**
 * Compute the scale factor and bias associated with a vector observer
 * according to the equation:
 * B_k = (I_{3x3} + D)^{-1} \times (O^T A_k H_k + b + \epsilon_k)
 * where k is the sample index,
 *       B_k is the kth measurement
 *       I_{3x3} is a 3x3 identity matrix
 *       D is a 3x3 symmetric matrix of scale factors
 *       O is the orthogonal alignment matrix
 *       A_k is the attitude at the kth sample
 *       b is the bias in the global reference frame
 *       \epsilon_k is the noise at the kth sample
 *
 * After computing the scale factor and bias matrices, the optimal estimate is
 * given by 
 * \tilde{B}_k = (I_{3x3} + D)B_k - b
 *
 * This implementation makes the assumption that \epsilon is a constant white,
 * gaussian noise source that is common to all k.  The misalignment matrix O
 * is not computed.
 *
 * @param bias[out] The computed bias, in the global frame
 * @param scale[out] The computed scale factor matrix, in the sensor frame.
 * @param samples[in] An array of measurement samples.
 * @param n_samples The number of samples in the array.
 * @param referenceField The magnetic field vector at this location. 
 * @param noise The one-sigma squared standard deviation of the observed noise
 * in the sensor.
 */
void twostep_bias_scale(Vector3f& bias, 
		Matrix3f& scale, 
		const Vector3f samples[], 
		const size_t n_samples,
		const Vector3f& referenceField,
		const float noise)
{
	// Define L_k by eq 51 for k = 1 .. n_samples
	Matrix<double, Dynamic, 9> fullSamples(n_samples, 9);
	// \hbar{L} by eq 52, simplified by observing that the common noise term
	// makes this a simple average.
	Matrix<double, 1, 9> centerSample = Matrix<double, 1, 9>::Zero();
	// Define the sample differences z_k by eq 23 a)
	double sampleDeltaMag[n_samples];
	// The center value \hbar{z}
	double sampleDeltaMagCenter = 0;
	// The squared norm of the reference vector
	double refSquaredNorm = referenceField.squaredNorm();

	for (size_t i = 0; i < n_samples; ++i) {
		fullSamples.row(i) << 2*samples[i].transpose().cast<double>(), 
			-(samples[i][0]*samples[i][0]),
			-(samples[i][1]*samples[i][1]),
			-(samples[i][2]*samples[i][2]),
			-2*(samples[i][0]*samples[i][1]),
			-2*(samples[i][0]*samples[i][2]),
			-2*(samples[i][1]*samples[i][2]);

		centerSample += fullSamples.row(i);

		sampleDeltaMag[i] = samples[i].squaredNorm() - refSquaredNorm;
		sampleDeltaMagCenter += sampleDeltaMag[i];
	}
	sampleDeltaMagCenter /= n_samples;
	centerSample /= n_samples;

	// Define \tilde{L}_k for k = 0 .. n_samples
	Matrix<double, Dynamic, 9> centeredSamples(n_samples, 9);
	// Define \tilde{z}_k for k = 0 .. n_samples
	double centeredMags[n_samples];
	// Compute the term under the summation of eq 57a
	Matrix<double, 9, 1> estimateSummation = Matrix<double, 9, 1>::Zero();
	for (size_t i = 0; i < n_samples; ++i) {
		centeredSamples.row(i) = fullSamples.row(i) - centerSample;
		centeredMags[i] = sampleDeltaMag[i] - sampleDeltaMagCenter;
		estimateSummation += centeredMags[i] * centeredSamples.row(i).transpose();
	}
	estimateSummation /= noise;

	// By eq 57b
	Matrix<double, 9, 9> P_theta_theta_inv = (1.0f/noise)*
		centeredSamples.transpose()*centeredSamples;

#ifdef PRINTF_DEBUGGING
	SelfAdjointEigenSolver<Matrix<double, 9, 9> > eig(P_theta_theta_inv);
	std::cout << "P_theta_theta_inverse: \n" << P_theta_theta_inv << "\n\n";
	std::cout << "P_\\tt^-1 eigenvalues: " << eig.eigenvalues().transpose() 
		<< "\n";
	std::cout << "P_\\tt^-1 eigenvectors:\n" << eig.eigenvectors() << "\n";
#endif

	// The current value of the estimate.  Initialized to \tilde{\theta}^*
	Matrix<double, 9, 1> estimate;
	// By eq 57a
	P_theta_theta_inv.ldlt().solve(estimateSummation, &estimate);

	// estimate i+1 = estimate_i - Fisher^{-1}(at estimate_i)*gradient(theta)
	// Fisher^{-1} = \tilde{Fisher}^-1 + \hbar{Fisher}^{-1}
	size_t count = 0;
	double eta = 10000;
	while (count++ < 200 && eta > 1e-8) {
		static bool warned = false;
		if (hasNaN(estimate)) {
			std::cout << "WARNING: found NaN at time " << count << "\n";
			warned = true;
		}
#if 0
		SelfAdjointEigenSolver<Matrix3d> eig_E(E_theta(estimate));
		Vector3d S = eig_E.eigenvalues();
		Vector3d W; W << -1 + sqrt(1 + S.coeff(0)),
				 -1 + sqrt(1 + S.coeff(1)),
				 -1 + sqrt(1 + S.coeff(2));
		scale = (eig_E.eigenvectors() * W.asDiagonal() * 
				eig_E.eigenvectors().transpose()) .cast<float>();

		(Matrix3f::Identity() + scale).ldlt().solve(
				estimate.start<3>().cast<float>(), &bias);
		std::cout << "\n\nestimated bias: " << bias.transpose()
			<< "\nestimated scale:\n" << scale;
#endif
		
		Matrix<double, 1, 9> db_dtheta = dnormb_dtheta(estimate);

		Matrix<double, 9, 1> dJ_dtheta = ::dJ_dtheta(centerSample, 
			sampleDeltaMagCenter,
			estimate,
			db_dtheta,
			-3*noise,
			noise/n_samples);

		// Eq 59, with reused storage on db_dtheta
		db_dtheta = centerSample - db_dtheta;
	 	Matrix<double, 9, 9> scale = P_theta_theta_inv + 
			(double(n_samples)/noise)*db_dtheta.transpose() * db_dtheta;

		// eq 14b, mutatis mutandis (grumble, grumble)
		Matrix<double, 9, 1> increment;
		scale.ldlt().solve(dJ_dtheta, &increment);
		estimate -= increment;
		eta = increment.dot(scale * increment);
		std::cout << "eta: " << eta << "\n";
	}
	std::cout << "terminated at eta = " << eta 
		<< " after " << count << " iterations\n";
	
	if (!isnan(eta) && !isinf(eta)) {
		// Transform the estimated parameters from [c | E] back into [b | D].
		// See eq 63-65
		SelfAdjointEigenSolver<Matrix3d> eig_E(E_theta(estimate));
		Vector3d S = eig_E.eigenvalues();
		Vector3d W; W << -1 + sqrt(1 + S.coeff(0)),
				 -1 + sqrt(1 + S.coeff(1)),
				 -1 + sqrt(1 + S.coeff(2));
		scale = (eig_E.eigenvectors() * W.asDiagonal() *
				eig_E.eigenvectors().transpose()) .cast<float>();

		(Matrix3f::Identity() + scale).ldlt().solve(
				estimate.start<3>().cast<float>(), &bias);
	}
	else {
		// return nonsense data.  The eigensolver can fall ingo
		// an infinite loop otherwise.
		// TODO: Add error code return
		scale = Matrix3f::Ones()*std::numeric_limits<float>::quiet_NaN();
		bias = Vector3f::Zero();
	}
}

