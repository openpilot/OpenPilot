
/**
 ******************************************************************************
 *
 * @file       MagOrAccelSensorCal.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      3 axis sensor cal from six measurements taken in a constant field.
 *             Call SixPointInConstFieldCal(FieldMagnitude, X, Y, Z, S, b)
 *             - FieldMagnitude is the constant field, e.g. 9.81 for accels
 *             - X, Y, Z are vectors of six measurements from different orientations
 *             - returns, S[3] and b[3], that are the scale and offsett for the axes
 *             - i.e. Measurementx = S[0]*Sensorx + b[0]
 *
 * @see        The GNU Public License (GPL) Version 3
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

#include <math.h>
#include "stdint.h"

//Function Prototypes
int16_t SixPointInConstFieldCal(double ConstMag, double x[6], double y[6],
				double z[6], double S[3], double b[3]);
int16_t LinearEquationsSolving(int16_t nDim, double *pfMatr,
			       double *pfVect, double *pfSolution);

int16_t SixPointInConstFieldCal(double ConstMag, double x[6], double y[6],
				double z[6], double S[3], double b[3])
{
	int16_t i;
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
		f[i] = x[i] * x[i] - x[i + 1] * x[i + 1];
	}

	// solve for c0=bx/Sx, c1=Sy^2/Sx^2; c2=Sy*by/Sx^2, c3=Sz^2/Sx^2, c4=Sz*bz/Sx^2
	if (!LinearEquationsSolving(5, (double *)A, f, c))
		return 0;

	// use one magnitude equation and c's to find Sx - doesn't matter which - all give the same answer
	xp = x[0];
	yp = y[0];
	zp = z[0];
	Sx = sqrt(ConstMag * ConstMag /
		  (xp * xp + 2 * c[0] * xp + c[0] * c[0] + c[1] * yp * yp +
		   2 * c[2] * yp + c[2] * c[2] / c[1] + c[3] * zp * zp +
		   2 * c[4] * zp + c[4] * c[4] / c[3]));

	S[0] = Sx;
	b[0] = Sx * c[0];
	S[1] = sqrt(c[1] * Sx * Sx);
	b[1] = c[2] * Sx * Sx / S[1];
	S[2] = sqrt(c[3] * Sx * Sx);
	b[2] = c[4] * Sx * Sx / S[2];

	return 1;
}

//*****************************************************************

int16_t LinearEquationsSolving(int16_t nDim, double *pfMatr,
			       double *pfVect, double *pfSolution)
{
	double fMaxElem;
	double fAcc;

	int16_t i, j, k, m;

	for (k = 0; k < (nDim - 1); k++)	// base row of matrix
	{
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
				pfMatr[k * nDim + i] =
				    pfMatr[m * nDim + i];
				pfMatr[m * nDim + i] = fAcc;
			}
			fAcc = pfVect[k];
			pfVect[k] = pfVect[m];
			pfVect[m] = fAcc;
		}

		if (pfMatr[k * nDim + k] == 0.)
			return 0;	// needs improvement !!!

		// triangulation of matrix with coefficients
		for (j = (k + 1); j < nDim; j++)	// current row of matrix
		{
			fAcc =
			    -pfMatr[j * nDim + k] / pfMatr[k * nDim + k];
			for (i = k; i < nDim; i++) {
				pfMatr[j * nDim + i] =
				    pfMatr[j * nDim + i] +
				    fAcc * pfMatr[k * nDim + i];
			}
			pfVect[j] = pfVect[j] + fAcc * pfVect[k];	// free member recalculation
		}
	}

	for (k = (nDim - 1); k >= 0; k--) {
		pfSolution[k] = pfVect[k];
		for (i = (k + 1); i < nDim; i++) {
			pfSolution[k] -=
			    (pfMatr[k * nDim + i] * pfSolution[i]);
		}
		pfSolution[k] = pfSolution[k] / pfMatr[k * nDim + k];
	}

	return 1;
}
