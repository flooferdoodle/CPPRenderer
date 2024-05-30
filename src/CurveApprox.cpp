/*
 *  Copyright 2024 Felix Zhu
 */

#include "CurveApprox.h"

const float CurveApproximator::INV_TOL = 4.f;

GPoint CurveApproximator::evalQuadraticCurve(const GPoint control[], float t){
    return (1 - t)*(1 - t) * control[0] + 2*(1 - t)*t * control[1] + t*t * control[2];
}

GPoint CurveApproximator::evalCubicCurve(const GPoint control[], float t){
    float inv_t = 1 - t;
    return inv_t*inv_t*inv_t * control[0] + 3*inv_t*inv_t*t * control[1] + 3*inv_t*t*t * control[2] + t*t*t * control[3];
}


std::vector<GPoint> CurveApproximator::convertQuadratic(const GPoint controls[]) {
    GVector E = (controls[0] - 2 * controls[1] + controls[2]) * 0.25f;
    int num_segs = (int) std::ceil(std::sqrt(E.length() * INV_TOL));

    std::vector<GPoint> path;
    //path.reserve(num_segs);

    float t = 0.f;
    float step = 1.f / (float) num_segs;
    for(int i = 0; i < num_segs; ++i){
        path.push_back(evalQuadraticCurve(controls, t));

        t += step;
    }

    path.push_back(controls[2]);

    return path;
}

std::vector<GPoint> CurveApproximator::convertCubic(const GPoint controls[]) {
    GVector E0 = controls[0] - 2 * controls[1] + controls[2];
    GVector E1 = controls[1] - 2 * controls[2] + controls[3];
    GVector E = {std::max(std::abs(E0.x), std::abs(E1.x)), std::max(std::abs(E0.y), std::abs(E1.y))};
    int num_segs = (int) std::ceil(std::sqrt(E.length() * INV_TOL * 0.75f));

    std::vector<GPoint> path;
    //path.reserve(num_segs);

    float t = 0.f;
    float step = 1.f / (float) num_segs;
    for(int i = 0; i < num_segs; ++i){
        path.push_back(evalCubicCurve(controls, t));

        t += step;
    }

    path.push_back(controls[3]);

    return path;
}

void CurveApproximator::splitQuadratic(std::vector<GPoint>& segs, const GPoint controls[], int depth, const int& max_depth){
    // order of segs maintained by DFS traversal order
    if(depth >= max_depth){
        segs.push_back(controls[0]);
        return;
    }

    // Calculate control points
    GPoint AB = (controls[0] + controls[1]) * 0.5f;
    GPoint BC = (controls[1] + controls[2]) * 0.5f;

    GPoint P = (AB + BC) * 0.5f;

    // Split curve
    GPoint splitCurve[] {controls[0], AB, P};
    splitQuadratic(segs, splitCurve, depth + 1, max_depth);

    splitCurve[0] = P; splitCurve[1] = BC; splitCurve[2] = controls[2];
    splitQuadratic(segs, splitCurve, depth + 1, max_depth);
}

//TODO: can make this generic for quadric curves too? or need a separate function?
//TODO: during split, use fast bounding with control points to check if curve is no longer inside canvas