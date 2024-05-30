/*
 *  Copyright 2024 Felix Zhu
 */

#ifndef TextureShader_DEFINED
#define TextureShader_DEFINED

#include "../../include/GShader.h"
#include "BitmapShader.h"
#include "../../include/GMatrix.h"

class TriTextureShader : public GShader {
public:
    TriTextureShader(GShader* shader) : bitmapShader(shader) { assert(shader != nullptr); }

    void reset(GPoint p0, GPoint p1, GPoint p2, GPoint t0, GPoint t1, GPoint t2) {
        GMatrix P = GMatrix(p1 - p0, p2 - p0, p0);
        GMatrix T = GMatrix(t1 - t0, t2 - t0, t0);
        auto T_inv = T.invert();
        assert(T_inv.has_value());
        transform = P * T_inv.value();
    }

    bool isOpaque() override { return bitmapShader->isOpaque(); }

    bool setContext(const GMatrix& ctm) override {
        return bitmapShader->setContext(ctm * transform);
    }

    void shadeRow(int pos_x, int pos_y, int count, GPixel row[]) override {
        bitmapShader->shadeRow(pos_x, pos_y, count, row);
    }

private:
    GMatrix transform;
    GShader* bitmapShader;
};

#endif