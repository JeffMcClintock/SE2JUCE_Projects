#include <tuple>

std::tuple<double, double, double> CalculateCurve(const double level_, const double sampleRate, double rate, const double target, const double curveAmmount);
std::tuple<double, double, double> CalculateSteadyState(const double target);
