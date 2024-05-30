/*
 *  Copyright 2024 Felix Zhu
 */

#ifndef _g_curve_approx_h
#define _g_curve_approx_h

#include "../include/GPoint.h"

class CurveApproximator{
public:
    static const float INV_TOL;
    static std::vector<GPoint> convertQuadratic(const GPoint controls[]);
    static std::vector<GPoint> convertCubic(const GPoint controls[]);

    static GPoint evalQuadraticCurve(const GPoint control[], float t);
    static GPoint evalCubicCurve(const GPoint control[], float t);

private:
    static void splitQuadratic(std::vector<GPoint>& segs, const GPoint controls[], int depth, const int& max_depth);
};


#endif