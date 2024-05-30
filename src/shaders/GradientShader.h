/*
 *  Copyright 2024 Felix Zhu
 */

#ifndef GradientShader_DEFINED
#define GradientShader_DEFINED

#include "../../include/GShader.h"
#include "../../include/GMatrix.h"
#include "../../include/GBlend.h"
#include <vector>

class GradientShader : public GShader {
public:
    GradientShader(const GPoint a, const GPoint b, const GColor cols[], int count, GTileMode mode) : p0(a), p1(b), numCols((float)count), tileMode(mode) {

        GVector dir = b - a;

        for(int i = 0; i < count; i++){
            colors.push_back(cols[i]);
            hasTransparency = hasTransparency || cols[i].a < 1;
        }

        localMat = GMatrix(dir.x, -dir.y, a.x,
                           dir.y,  dir.x, a.y);
    }

    // Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque() { return !hasTransparency; }

    // The draw calls in GCanvas must call this with the CTM before any calls to shadeRow().
    bool setContext(const GMatrix& ctm){
        invContext = GMatrix::Concat(ctm, localMat).invert();
        return invContext.has_value();
    }

    /**
     *  Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
     *  corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
     *  can hold at least [count] entries.
     */
    void shadeRow(int pos_x, int pos_y, int count, GPixel row[]) {
        assert(this->invContext.has_value());

        const GVector step = invContext.value().e0();

        GPoint pos = invContext.value() * GPoint{pos_x + 0.5f, pos_y + 0.5f};

        float t;

        float x;
        int integ;
        float frac;
        GColor lerpCol;

        switch(tileMode){
            case GTileMode::kClamp:
                for(int i = 0; i < count; i++){
                    x = std::clamp(pos.x, 0.f, 1.f);

                    t = x * (numCols - 1.f);
                    integ = (int) std::floor(t);
                    frac = t - integ;

                    lerpCol = (1 - frac) * colors[integ] + frac * colors[integ + 1];

                    row[i] = toPremul(lerpCol);

                    pos += step;
                }
                break;
            case GTileMode::kRepeat:
                for(int i = 0; i < count; i++){
                    x = pos.x - std::floor(pos.x);

                    t = x * (numCols - 1.f);
                    integ = (int) std::floor(t);
                    frac = t - integ;

                    lerpCol = (1 - frac) * colors[integ] + frac * colors[integ + 1];

                    row[i] = toPremul(lerpCol);

                    pos += step;
                }
                break;
            case GTileMode::kMirror:
                for(int i = 0; i < count; i++){
                    x = pos.x * 0.5f;
                    x = x - std::floor(x);
                    if(x < 0.5f) x = x * 2.f;
                    else x = 2.f - 2.f*x;

                    t = x * (numCols - 1.f);
                    integ = (int) std::floor(t);
                    frac = t - integ;

                    lerpCol = (1 - frac) * colors[integ] + frac * colors[integ + 1];

                    row[i] = toPremul(lerpCol);

                    pos += step;
                }
                break;
        }
    }

private:
    const GPoint p0;
    const GPoint p1;
    const float numCols;
    std::vector<GColor> colors;
    bool hasTransparency = false;
    
    GMatrix localMat;
    std::optional<GMatrix> invContext;

    GTileMode tileMode;
};

#endif