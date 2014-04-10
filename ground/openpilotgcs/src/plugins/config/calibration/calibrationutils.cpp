/**
 ******************************************************************************
 *
 * @file       calibrationutils.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 *
 * @brief      Utilities for calibration. Ellipsoid and polynomial fit algorithms
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup
 * @{
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

#include "calibrationutils.h"
#include <math.h>
using namespace Eigen;
namespace OpenPilot {
float CalibrationUtils::ComputeSigma(Eigen::VectorXf *samplesY)
{
    Eigen::ArrayXd tmpd = samplesY->cast<double>().array();
    double mean = tmpd.mean();

    return (float)sqrt((tmpd - mean).square().mean());
}

/*
 * The following ellipsoid calibration code is based on RazorImu calibration samples that can be found here:
 * https://github.com/ptrbrtz/razor-9dof-ahrs/tree/master/Matlab/magnetometer_calibration
 */
bool CalibrationUtils::EllipsoidCalibration(Eigen::VectorXf *samplesX, Eigen::VectorXf *samplesY, Eigen::VectorXf *samplesZ, float nominalRange, EllipsoidCalibrationResult *result)
{
    Eigen::VectorXf radii;
    Eigen::Vector3f center;
    Eigen::MatrixXf evecs;

    EllipsoidFit(samplesX, samplesY, samplesZ, &center, &radii, &evecs);

    result->Scale.setZero();

    result->Scale << nominalRange / radii.coeff(0),
        nominalRange / radii.coeff(1),
        nominalRange / radii.coeff(2);

    Eigen::Matrix3f tmp;
    tmp << result->Scale.coeff(0), 0, 0,
        0, result->Scale.coeff(1), 0,
        0, 0, result->Scale.coeff(2);

    result->CalibrationMatrix = evecs * tmp * evecs.transpose();
    result->Bias.setZero();
    result->Bias << center.coeff(0), center.coeff(1), center.coeff(2);
    return true;
}

bool CalibrationUtils::PolynomialCalibration(VectorXf *samplesX, Eigen::VectorXf *samplesY, int degree, Eigen::Ref<Eigen::VectorXf> result, const double maxRelativeError)
{
    int samples = samplesX->rows();
    // perform internal calculation using doubles
    VectorXd doubleX = samplesX->cast<double>();
    VectorXd doubleY = samplesY->cast<double>();
    Eigen::MatrixXd x(samples, degree + 1);

    x.setOnes(samples, degree + 1);

    for (int i = 1; i < degree + 1; i++) {
        Eigen::MatrixXd tmp  = Eigen::MatrixXd(x.col(i - 1));
        Eigen::MatrixXd tmp2 = tmp.cwiseProduct(doubleX);

        x.col(i) = tmp2;
    }
    Eigen::MatrixXd xt   = x.transpose();

    Eigen::MatrixXd xtx  = xt * x;

    Eigen::VectorXd xty  = xt * doubleY;
    Eigen::VectorXd tmpx = xtx.fullPivHouseholderQr().solve(xty);
    result = tmpx.cast<float>();
    double relativeError = (xtx * tmpx - xty).norm() / xty.norm();
    return relativeError < maxRelativeError;
}

void CalibrationUtils::ComputePoly(VectorXf *samplesX, Eigen::VectorXf *polynomial, VectorXf *polyY)
{
    polyY->array().fill(polynomial->coeff(0));
    for (int i = 1; i < polynomial->rows(); i++) {
        polyY->array() += samplesX->array().pow(i) * polynomial->coeff(i);
    }
}

/* C++ Implementation of Yury Petrov's ellipsoid fit algorithm
 * Following is the origial code and its license from which this implementation is derived
 *
   Copyright (c) 2009, Yury Petrov
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

 * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.


   function [ center, radii, evecs, v ] = ellipsoid_fit( X, flag, equals )
   %
   % Fit an ellispoid/sphere to a set of xyz data points:
   %
   %   [center, radii, evecs, pars ] = ellipsoid_fit( X )
   %   [center, radii, evecs, pars ] = ellipsoid_fit( [x y z] );
   %   [center, radii, evecs, pars ] = ellipsoid_fit( X, 1 );
   %   [center, radii, evecs, pars ] = ellipsoid_fit( X, 2, 'xz' );
   %   [center, radii, evecs, pars ] = ellipsoid_fit( X, 3 );
   %
   % Parameters:
   % * X, [x y z]   - Cartesian data, n x 3 matrix or three n x 1 vectors
   % * flag         - 0 fits an arbitrary ellipsoid (default),
   %                - 1 fits an ellipsoid with its axes along [x y z] axes
   %                - 2 followed by, say, 'xy' fits as 1 but also x_rad = y_rad
   %                - 3 fits a sphere
   %
   % Output:
   % * center    -  ellispoid center coordinates [xc; yc; zc]
   % * ax        -  ellipsoid radii [a; b; c]
   % * evecs     -  ellipsoid radii directions as columns of the 3x3 matrix
   % * v         -  the 9 parameters describing the ellipsoid algebraically:
   %                Ax^2 + By^2 + Cz^2 + 2Dxy + 2Exz + 2Fyz + 2Gx + 2Hy + 2Iz = 1
   %
   % Author:
   % Yury Petrov, Northeastern University, Boston, MA
   %

   error( nargchk( 1, 3, nargin ) );  % check input arguments
   if nargin == 1
    flag = 0;  % default to a free ellipsoid
   end
   if flag == 2 && nargin == 2
    equals = 'xy';
   end

   if size( X, 2 ) ~= 3
    error( 'Input data must have three columns!' );
   else
    x = X( :, 1 );
    y = X( :, 2 );
    z = X( :, 3 );
   end

   % need nine or more data points
   if length( x ) < 9 && flag == 0
   error( 'Must have at least 9 points to fit a unique ellipsoid' );
   end
   if length( x ) < 6 && flag == 1
   error( 'Must have at least 6 points to fit a unique oriented ellipsoid' );
   end
   if length( x ) < 5 && flag == 2
   error( 'Must have at least 5 points to fit a unique oriented ellipsoid with two axes equal' );
   end
   if length( x ) < 3 && flag == 3
   error( 'Must have at least 4 points to fit a unique sphere' );
   end

   if flag == 0
    % fit ellipsoid in the form Ax^2 + By^2 + Cz^2 + 2Dxy + 2Exz + 2Fyz + 2Gx + 2Hy + 2Iz = 1
    D = [ x .* x, ...
          y .* y, ...
          z .* z, ...
      2 * x .* y, ...
      2 * x .* z, ...
      2 * y .* z, ...
      2 * x, ...
      2 * y, ...
      2 * z ];  % ndatapoints x 9 ellipsoid parameters
   elseif flag == 1
    % fit ellipsoid in the form Ax^2 + By^2 + Cz^2 + 2Gx + 2Hy + 2Iz = 1
    D = [ x .* x, ...
          y .* y, ...
          z .* z, ...
      2 * x, ...
      2 * y, ...
      2 * z ];  % ndatapoints x 6 ellipsoid parameters
   elseif flag == 2
    % fit ellipsoid in the form Ax^2 + By^2 + Cz^2 + 2Gx + 2Hy + 2Iz = 1,
    % where A = B or B = C or A = C
    if strcmp( equals, 'yz' ) || strcmp( equals, 'zy' )
        D = [ y .* y + z .* z, ...
            x .* x, ...
            2 * x, ...
            2 * y, ...
            2 * z ];
    elseif strcmp( equals, 'xz' ) || strcmp( equals, 'zx' )
        D = [ x .* x + z .* z, ...
            y .* y, ...
            2 * x, ...
            2 * y, ...
            2 * z ];
    else
        D = [ x .* x + y .* y, ...
            z .* z, ...
            2 * x, ...
            2 * y, ...
            2 * z ];
    end
   else
    % fit sphere in the form A(x^2 + y^2 + z^2) + 2Gx + 2Hy + 2Iz = 1
    D = [ x .* x + y .* y + z .* z, ...
      2 * x, ...
      2 * y, ...
      2 * z ];  % ndatapoints x 4 sphere parameters
   end

   % solve the normal system of equations
   v = ( D' * D ) \ ( D' * ones( size( x, 1 ), 1 ) );

   % find the ellipsoid parameters
   if flag == 0
    % form the algebraic form of the ellipsoid
    A = [ v(1) v(4) v(5) v(7); ...
          v(4) v(2) v(6) v(8); ...
          v(5) v(6) v(3) v(9); ...
          v(7) v(8) v(9) -1 ];
    % find the center of the ellipsoid
    center = -A( 1:3, 1:3 ) \ [ v(7); v(8); v(9) ];
    % form the corresponding translation matrix
    T = eye( 4 );
    T( 4, 1:3 ) = center';
    % translate to the center
    R = T * A * T';
    % solve the eigenproblem
    [ evecs evals ] = eig( R( 1:3, 1:3 ) / -R( 4, 4 ) );
    radii = sqrt( 1 ./ diag( evals ) );
   else
    if flag == 1
        v = [ v(1) v(2) v(3) 0 0 0 v(4) v(5) v(6) ];
    elseif flag == 2
        if strcmp( equals, 'xz' ) || strcmp( equals, 'zx' )
            v = [ v(1) v(2) v(1) 0 0 0 v(3) v(4) v(5) ];
        elseif strcmp( equals, 'yz' ) || strcmp( equals, 'zy' )
            v = [ v(2) v(1) v(1) 0 0 0 v(3) v(4) v(5) ];
        else % xy
            v = [ v(1) v(1) v(2) 0 0 0 v(3) v(4) v(5) ];
        end
    else
        v = [ v(1) v(1) v(1) 0 0 0 v(2) v(3) v(4) ];
    end
    center = ( -v( 7:9 ) ./ v( 1:3 ) )';
    gam = 1 + ( v(7)^2 / v(1) + v(8)^2 / v(2) + v(9)^2 / v(3) );
    radii = ( sqrt( gam ./ v( 1:3 ) ) )';
    evecs = eye( 3 );
   end

 */


void CalibrationUtils::EllipsoidFit(Eigen::VectorXf *samplesX, Eigen::VectorXf *samplesY, Eigen::VectorXf *samplesZ,
                                    Eigen::Vector3f *center,
                                    Eigen::VectorXf *radii,
                                    Eigen::MatrixXf *evecs)
{
    int numSamples = (*samplesX).rows();
    Eigen::MatrixXf D(numSamples, 9);

    D.setZero();
    D.col(0) = (*samplesX).cwiseProduct(*samplesX);
    D.col(1) = (*samplesY).cwiseProduct(*samplesY);
    D.col(2) = (*samplesZ).cwiseProduct(*samplesZ);
    D.col(3) = (*samplesX).cwiseProduct(*samplesY) * 2;
    D.col(4) = (*samplesX).cwiseProduct(*samplesZ) * 2;
    D.col(5) = (*samplesY).cwiseProduct(*samplesZ) * 2;
    D.col(6) = 2 * (*samplesX);
    D.col(7) = 2 * (*samplesY);
    D.col(8) = 2 * (*samplesZ);

    Eigen::VectorXf ones(numSamples);
    ones.setOnes(numSamples);

    Eigen::MatrixXf dt1 = (D.transpose() * D);
    Eigen::MatrixXf dt2 = (D.transpose() * ones);
    Eigen::VectorXf v   = dt1.inverse() * dt2;

    Eigen::Matrix4f A;
    A << v.coeff(0), v.coeff(3), v.coeff(4), v.coeff(6),
        v.coeff(3), v.coeff(1), v.coeff(5), v.coeff(7),
        v.coeff(4), v.coeff(5), v.coeff(2), v.coeff(8),
        v.coeff(6), v.coeff(7), v.coeff(8), -1;

    (*center) = -1 * A.block(0, 0, 3, 3).inverse() * v.segment(6, 3);

    Eigen::Matrix4f t     = Eigen::Matrix4f::Identity();
    t.block(3, 0, 1, 3) = center->transpose();

    Eigen::Matrix4f r     = t * A * t.transpose();

    Eigen::Matrix3f tmp2  = r.block(0, 0, 3, 3) * -1 / r.coeff(3, 3);
    Eigen::EigenSolver<Eigen::Matrix3f> es(tmp2);
    Eigen::VectorXf evals = es.eigenvalues().real();
    (*evecs) = es.eigenvectors().real();
    radii->setZero(3);

    (*radii) = (evals.segment(0, 3)).cwiseInverse().cwiseSqrt();
}

int CalibrationUtils::SixPointInConstFieldCal(double ConstMag, double x[6], double y[6], double z[6], double S[3], double b[3])
{
    int i;
    double A[5][5];
    double f[5], c[5];
    double xp, yp, zp, Sx;

    // Fill in matrix A -
    // write six difference-in-magnitude equations of the form
    // Sx^2(x2^2-x1^2) + 2*Sx*bx*(x2-x1) + Sy^2(y2^2-y1^2) + 2*Sy*by*(y2-y1) + Sz^2(z2^2-z1^2) + 2*Sz*bz*(z2-z1) = 0
    // or in other words
    // 2*Sx*bx*(x2-x1)/Sx^2  + Sy^2(y2^2-y1^2)/Sx^2  + 2*Sy*by*(y2-y1)/Sx^2  + Sz^2(z2^2-z1^2)/Sx^2  + 2*Sz*bz*(z2-z1)/Sx^2  = (x1^2-x2^2)
    for (i = 0; i < 5; i++) {
        A[i][0] = 2.0 * (x[i + 1] - x[i]);
        A[i][1] = y[i + 1] * y[i + 1] - y[i] * y[i];
        A[i][2] = 2.0 * (y[i + 1] - y[i]);
        A[i][3] = z[i + 1] * z[i + 1] - z[i] * z[i];
        A[i][4] = 2.0 * (z[i + 1] - z[i]);
        f[i]    = x[i] * x[i] - x[i + 1] * x[i + 1];
    }

    // solve for c0=bx/Sx, c1=Sy^2/Sx^2; c2=Sy*by/Sx^2, c3=Sz^2/Sx^2, c4=Sz*bz/Sx^2
    if (!LinearEquationsSolve(5, (double *)A, f, c)) {
        return 0;
    }

    // use one magnitude equation and c's to find Sx - doesn't matter which - all give the same answer
    xp   = x[0]; yp = y[0]; zp = z[0];
    Sx   = sqrt(ConstMag * ConstMag / (xp * xp + 2 * c[0] * xp + c[0] * c[0] + c[1] * yp * yp + 2 * c[2] * yp + c[2] * c[2] / c[1] + c[3] * zp * zp + 2 * c[4] * zp + c[4] * c[4] / c[3]));

    S[0] = Sx;
    b[0] = Sx * c[0];
    S[1] = sqrt(c[1] * Sx * Sx);
    b[1] = c[2] * Sx * Sx / S[1];
    S[2] = sqrt(c[3] * Sx * Sx);
    b[2] = c[4] * Sx * Sx / S[2];

    return 1;
}


int CalibrationUtils::LinearEquationsSolve(int nDim, double *pfMatr, double *pfVect, double *pfSolution)
{
    double fMaxElem;
    double fAcc;

    int i, j, k, m;

    for (k = 0; k < (nDim - 1); k++) { // base row of matrix
        // search of line with max element
        fMaxElem = fabs(pfMatr[k * nDim + k]);
        m = k;
        for (i = k + 1; i < nDim; i++) {
            if (fMaxElem < fabs(pfMatr[i * nDim + k])) {
                fMaxElem = pfMatr[i * nDim + k];
                m = i;
            }
        }

        // permutation of base line (index k) and max element line(index m)
        if (m != k) {
            for (i = k; i < nDim; i++) {
                fAcc = pfMatr[k * nDim + i];
                pfMatr[k * nDim + i] = pfMatr[m * nDim + i];
                pfMatr[m * nDim + i] = fAcc;
            }
            fAcc = pfVect[k];
            pfVect[k] = pfVect[m];
            pfVect[m] = fAcc;
        }

        if (pfMatr[k * nDim + k] == 0.) {
            return 0; // needs improvement !!!
        }
        // triangulation of matrix with coefficients
        for (j = (k + 1); j < nDim; j++) { // current row of matrix
            fAcc = -pfMatr[j * nDim + k] / pfMatr[k * nDim + k];
            for (i = k; i < nDim; i++) {
                pfMatr[j * nDim + i] = pfMatr[j * nDim + i] + fAcc * pfMatr[k * nDim + i];
            }
            pfVect[j] = pfVect[j] + fAcc * pfVect[k]; // free member recalculation
        }
    }

    for (k = (nDim - 1); k >= 0; k--) {
        pfSolution[k] = pfVect[k];
        for (i = (k + 1); i < nDim; i++) {
            pfSolution[k] -= (pfMatr[k * nDim + i] * pfSolution[i]);
        }
        pfSolution[k] = pfSolution[k] / pfMatr[k * nDim + k];
    }

    return 1;
}
}


