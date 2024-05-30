#include "GradientShader.h"

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor cols[], int count, GTileMode tileMode){
    return std::unique_ptr<GShader>(new GradientShader(p0, p1, cols, count, tileMode));
}