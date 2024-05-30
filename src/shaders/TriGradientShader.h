/*
 *  Copyright 2024 Felix Zhu
 */

#ifndef TriGradientShader_DEFINED
#define TriGradientShader_DEFINED

#include "../../include/GShader.h"
#include "../../include/GMatrix.h"
#include "../../include/GPoint.h"
#include "../../include/GBlend.h"
#include <vector>

class TriGradientShader : public GShader {
public:
    TriGradientShader(GPoint p0, GPoint p1, GPoint p2, GColor c0, GColor c1, GColor c2)
                     : P0(p0), P1(p1), P2(p2), C0(c0), C1(c1), C2(c2),
                       DC1(C1 - C0), DC2(C2 - C0), M(GMatrix(P1 - P0, P2 - P0, P0))
    {
        hasTransparency = (c0.a + c1.a + c2.a) < 3.f;
    }

    TriGradientShader() {}

    void reset(GPoint p0, GPoint p1, GPoint p2, GColor c0, GColor c1, GColor c2){
        P0 = p0; P1 = p1; P2 = p2;
        C0 = c0; C1 = c1; C2 = c2;
        DC1 = C1 - C0;
        DC2 = C2 - C0;
        M = GMatrix(P1 - P0, P2 - P0, P0);
        hasTransparency = (c0.a + c1.a + c2.a) < 3.f;
    }

    // Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() { return !hasTransparency; }

    // The draw calls in GCanvas must call this with the CTM before any calls to shadeRow().
    bool setContext(const GMatrix& ctm){
        invContext = GMatrix::Concat(ctm, M).invert();
        if(invContext.has_value()){
            DC = invContext.value()[0] * DC1 + invContext.value()[1] * DC2;
            return true;
        }
        return false;
    }

    void shadeRow(int pos_x, int pos_y, int count, GPixel row[]) {
        assert(this->invContext.has_value());

        GMatrix inv = invContext.value();

        GPoint P_ = inv * GPoint{pos_x + 0.5f, pos_y + 0.5f};
        GColor C = P_.x * DC1 + P_.y * DC2 + C0;

        for(int i = 0; i < count; i++){
            row[i] = toPremul(C);
            C += DC;
        }
    }

private:

    GPoint P0, P1, P2;
    GColor C0, C1, C2;
    GColor DC, DC1, DC2;

    bool hasTransparency;
    GMatrix M;
    std::optional<GMatrix> invContext;

};

#endif