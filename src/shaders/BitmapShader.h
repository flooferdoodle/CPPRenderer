/*
 *  Copyright 2024 Felix Zhu
 */

#ifndef BitmapShader_DEFINED
#define BitmapShader_DEFINED

#include "../../include/GShader.h"
#include "../../include/GBitmap.h"
#include "../../include/GMatrix.h"

class BitmapShader : public GShader {
public:
    BitmapShader(const GBitmap& device, const GMatrix& localMatrix, GTileMode mode) : fDevice(device), localMat(localMatrix), tileMode(mode) {}

    // Return true iff all of the GPixels that may be returned by this shader will be opaque.
    bool isOpaque();

    // The draw calls in GCanvas must call this with the CTM before any calls to shadeRow().
    bool setContext(const GMatrix& ctm);

    /**
     *  Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
     *  corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
     *  can hold at least [count] entries.
     */
    void shadeRow(int x, int y, int count, GPixel row[]);

private:
    GBitmap fDevice;
    GMatrix localMat;
    std::optional<GMatrix> invContext;

    GTileMode tileMode;
};

#endif