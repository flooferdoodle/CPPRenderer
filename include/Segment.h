/*
 *  Copyright 2024 Felix Zhu
 */

#ifndef _g_segment_h
#define _g_segment_h

#include "GPoint.h"
#include "GBitmap.h"

struct Segment{
    int minY;
    int maxY;
    float b;
    float slope;
    bool up;

    /// @brief Special case vertical segment
    Segment(float top, float bot, float x, bool dir) : b(x), slope(0.f), up(dir) {
        minY = (int)std::round(top);
        maxY = (int)std::floor(bot - 0.5f);
    };

    Segment(float minY, float maxY, float m, float xInt, bool dir) :  b(xInt), slope(m), up(dir) {
        this->minY = (int)std::round(minY);
        this->maxY = (int)std::floor(maxY - 0.5f);
    }

    /// @brief Returns the x-value of the intersection point
    /// @param y 
    /// @return 
    inline float horizontalIntersect(float y){ return y * slope + b; }

    bool operator<(const Segment& other) const{
        if(minY == other.minY) return b < other.b;
        return minY < other.minY;
    }
};

struct SegmentIntersection{
    int x;
    bool up;

    bool operator<(const SegmentIntersection& other) const{
        return x < other.x;
    }
};

#endif