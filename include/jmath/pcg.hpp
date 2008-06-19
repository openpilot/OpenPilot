/** -*- c++ -*- \file pcg.hpp \brief preconditioned conjugate gradient solver */
/*
 -   begin                : 2005-08-24
 -   copyright            : (C) 2005 by Gunter Winkler, Konstantin Kutzkow
 -   email                : guwi17@gmx.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef _H_PCG_HPP_
#define _H_PCG_HPP_

#include <cassert>
#include <iostream>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include <boost/numeric/ublas/operation.hpp>

#include "jmath/precond.hpp"

namespace ublas = boost::numeric::ublas;


template <class MAT, class VEC, class Precond>
size_t pcg_solve(const MAT& A, VEC& x, const VEC& b, 
                 const Precond& B = IdentityPreconditioner<MAT>(),
                 const size_t maxiter = 10,
                 const double rTOL = 1e-6,
                 const double aTOL = 1e-14)
{
  typedef typename VEC::value_type  value_type;

  const size_t size = x.size();
  size_t iteration = 0;
  
  VEC resid(size);
  VEC g(size);
  VEC d1(size);
  value_type alpha, beta, norm, h1, h2, norm_0;
  
  resid = b - prod(A, x);
 
  B.apply(resid, g);
  
  norm = inner_prod(resid, resid);
  norm_0 = norm;
  ++iteration;

  B.apply(resid, d1);
  h2 = inner_prod(resid, d1);

  while ((iteration < maxiter) && (norm > (aTOL*aTOL)) && ((norm/norm_0) > (rTOL * rTOL)) ) {
    h1 = h2;
    
    axpy_prod(A, g, d1, true); 

    h2 = inner_prod(g, d1);
    
    alpha = h1/h2;
    
    x += alpha*g;
    resid -= alpha*d1; 
    
    B.apply(resid, d1);
    h2 = inner_prod(resid, d1);      
    
    beta = h2/h1; 
    g = beta*g + d1;
    
    norm = inner_prod(resid, resid);

    ++iteration;
  }
  return iteration;
}


#endif
