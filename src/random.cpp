/* $Id$ */

#include <cmath>

#include "kernel/jafarException.hpp"
#include "kernel/jafarDebug.hpp"

#include "jmath/random.hpp"

using namespace jblas;

using namespace jafar::jmath;

/*
 * class UniformDistribution
 */

UniformDistribution::UniformDistribution(double min_, double max_,
                                         long long unsigned int seed_) :
  rng(seed_),
  uniform(min_, max_),
  vg(rng, uniform) {}

UniformDistribution::~UniformDistribution() {}

double UniformDistribution::get() {
  return vg();
}

/*
 * class MultiDimUniformDistribution
 */

MultiDimUniformDistribution::MultiDimUniformDistribution(std::size_t size_, long long unsigned int seed_) :
    u(size_),
    uniformsVec(size_) 
{
  for (std::size_t i =0 ; i <size_ ; i++) {
    uniformsVec[i] = new  UniformDistribution(seed_+i);
  } 
}

MultiDimUniformDistribution::MultiDimUniformDistribution(const vec& min_, const vec& max_, 
                                                         long long unsigned int seed_) :
  u(min_.size()),
  uniformsVec(min_.size())
{
  JFR_PRECOND(min_.size() == max_.size(),
              "MultiDimUniformDistribution::MultiDimUniformDistribution: min_ and max_ size must be equal");

  for (std::size_t i =0 ; i <uniformsVec.size() ; i++) {
    uniformsVec[i] = new  UniformDistribution(min_(i), max_(i), seed_+i);
  }
}

MultiDimUniformDistribution::~MultiDimUniformDistribution() {
  for (std::size_t i =0 ; i <uniformsVec.size() ; i++) {
    delete uniformsVec[i];
  }
}

vec& MultiDimUniformDistribution::get() {
  for (std::size_t i=0 ; i <u.size() ; i++) {
    u(i) = uniformsVec[i]->get();
  }
  return u;
}

/*
 * class NormalDistribution
 */

NormalDistribution::NormalDistribution(double mean_, double sigma_, 
                                       long long unsigned int seed_) :
  rng(seed_),
  normal(mean_, sigma_),
  vg(rng, normal) 
{
  get();
}

NormalDistribution::~NormalDistribution() {}

double NormalDistribution::get() {
//   double u = vg();
//   if (u > normal.mean() + 3*normal.sigma() ) {
//     JFR_DEBUG("u=" << u);
//     u = normal.mean() + 3*normal.sigma();
//   }
//   if (u < normal.mean() - 3*normal.sigma() ) {
//     JFR_DEBUG("u=" << u);
//     u = normal.mean() - 3*normal.sigma();
//   }
  
//   return u;
  return vg();
}

/*
 * class MultiDimNormalDistribution
 */

MultiDimNormalDistribution::MultiDimNormalDistribution(std::size_t size_, long long unsigned int seed_) :
  u(size_), normalsVec(size_)
{
  for (std::size_t i=0 ; i <size_ ; i++) {
    normalsVec[i] = new  NormalDistribution(seed_+i);
  }
}

MultiDimNormalDistribution::MultiDimNormalDistribution(const vec& mean_, const vec& cov_, 
                                                       long long unsigned int seed_) :
  u(mean_.size()),
  normalsVec(mean_.size())
{
  JFR_PRECOND(cov_.size() == mean_.size(),
              "MultiDimNormalDistribution::MultiDimNormalDistribution: size of cov_ does not match");
  for (std::size_t i=0 ; i <normalsVec.size() ; i++) {
    normalsVec[i] = new  NormalDistribution(mean_(i), sqrt(cov_(i)), seed_+i);
  }
}

MultiDimNormalDistribution::~MultiDimNormalDistribution() {
  for (std::size_t i=0 ; i <normalsVec.size() ; i++) {
    delete normalsVec[i];
  }
}


vec& MultiDimNormalDistribution::get() {
  for (std::size_t i =0 ; i <u.size() ; i++) {
    u(i) = normalsVec[i]->get();
  }
  return u;
}
