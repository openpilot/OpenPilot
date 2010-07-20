#ifndef JMATH_MISC_HPP
#define JMATH_MISC_HPP

#include <cmath>

namespace jafar {
namespace jmath {

#ifdef THALES_TAROT
#undef max
#undef min
#endif



template<typename T>
static inline T abs(const T x) { return (x>=0 ? x : -x); }

template<typename T>
static inline T sqr(const T x) { return x*x; }

template<typename T>
static inline T sum_sqr(const T x, const T y) { return sqr(x) + sqr(y); }
template<typename T>
static inline T sum_sqr(const T x, const T y, const T z) { return sqr(x) + sqr(y) + sqr(z); }

template <typename T> // use std::max instead
static inline T max(const T a, const T b) { return std::max(a,b); }
//static inline T max(const T a, const T b) { return (a > b ? a : b); }

template <typename T>
static inline T max(const T a, const T b, const T c) { return std::max(std::max(a,b),c); }

template <typename T> // use std::min instead
static inline T min(const T a, const T b) { return std::min(a,b); }
//static inline T min(const T a, const T b) { return (a < b ? a : b); }

template <typename T>
static inline T min(const T a, const T b, const T c) { return std::min(std::min(a,b),c); }

template <typename T>
static inline T sign(const T x) { return (x < 0 ? -1 : +1); }

template <typename T>
static inline T mod(const T x, const T n) { return x-n*std::floor(x/n); }
//static inline T mod(const T x, const T n) { return std::modf(x/n,NULL)*n; }
template <typename T>
static inline T mod2(const T x, const T a, const T b) { return mod(x-a,b-a)+a; }

template <typename T>
static inline T round(const T x) { return (x >= 0 ? std::floor(x+(T)0.5) : std::ceil(x-(T)0.5)); }

template <typename T>
static inline std::string toStr(T a) { std::ostringstream s; s << a; return s.str(); }

static inline std::string intToStr(int i) {
	std::ostringstream name;
	name << "" << i;
	return name.str();
}

}
}

#endif

