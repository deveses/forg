#include "forg_pch.h"

#include "math/Math.h"
#include <cfloat>
#include <cmath>
#include <numbers>

namespace forg::math {

const double Math::PI = std::numbers::pi;
const double Math::SQRT2_2 = std::numbers::sqrt2 / 2.0;
const double Math::RAD2DEG = 180.0 / std::numbers::pi;
const double Math::DEG2RAD = std::numbers::pi / 180.0;
const float Math::FloatMinValue = FLT_MIN;
const float Math::FloatMaxValue = FLT_MAX;

double Math::Sin(double d) { return std::sin(d); }

double Math::Cos(double d) { return std::cos(d); }

double Math::Tan(double d) { return std::tan(d); }

double Math::Atan(double d) { return std::atan(d); }

double Math::Atan2(double y, double x) { return std::atan2(y, x); }

double Math::Acos(double d) { return std::acos(d); }

bool Math::IsNaN(double d) { return std::isnan(d); }

double Math::Sqrt(double d) { return std::sqrt(d); }

double Math::Abs(double d) { return std::abs(d); }

double Math::Log(double d) { return std::log(d); }

double Math::Floor(double d) { return std::floor(d); }

double Math::Log10(double d) { return std::log10(d); }

double Math::Pow(double a, double b) { return std::pow(a, b); }

} // namespace forg::math
