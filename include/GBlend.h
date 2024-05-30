/*
 *  Copyright 2024 Felix Zhu
 */

#ifndef GBlend_DEFINED
#define GBlend_DEFINED

#include "GPixel.h"
#include "GPaint.h"
#include "GColor.h"

//using blendFuncPtr = GPixel(*)(GPixel, GPixel);

//blendFuncPtr getBlendFunc(GBlendMode blendMode);

static inline GPixel toPremul(const GColor& color) {
    return GPixel_PackARGB((unsigned int) (color.a * 255.f + 0.5f),
                           (unsigned int) (color.r * color.a * 255.f + 0.5f),
                           (unsigned int) (color.g * color.a * 255.f + 0.5f),
                           (unsigned int) (color.b * color.a * 255.f + 0.5f) );
}

static inline GPixel toPremul(const GPaint& paint) {
    return toPremul(paint.getColor());
}

static inline unsigned int fastDiv255(unsigned int a){
    return (a + 128) * 257 >> 16;
}

static inline GPixel mult(int t, GPixel p){
    return GPixel_PackARGB(fastDiv255(t * GPixel_GetA(p)),
                           fastDiv255(t * GPixel_GetR(p)),
                           fastDiv255(t * GPixel_GetG(p)),
                           fastDiv255(t * GPixel_GetB(p)));
}

static inline GPixel _blend_kClear(GPixel src, GPixel dst){ return GPixel(0); }
static inline GPixel _blend_kSrc(GPixel src, GPixel dst){ return src; }
static inline GPixel _blend_kDst(GPixel src, GPixel dst){ return dst; }
static inline GPixel _blend_kSrcOver(GPixel src, GPixel dst){ return src + mult(255 - GPixel_GetA(src), dst); }
static inline GPixel _blend_kDstOver(GPixel src, GPixel dst){ return dst + mult(255 - GPixel_GetA(dst), src); }
static inline GPixel _blend_kSrcIn(GPixel src, GPixel dst){ return mult(GPixel_GetA(dst), src); }
static inline GPixel _blend_kDstIn(GPixel src, GPixel dst){ return mult(GPixel_GetA(src), dst); }
static inline GPixel _blend_kSrcOut(GPixel src, GPixel dst){ return mult(255 - GPixel_GetA(dst), src); }
static inline GPixel _blend_kDstOut(GPixel src, GPixel dst){ return mult(255 - GPixel_GetA(src), dst); }
static inline GPixel _blend_kSrcATop(GPixel src, GPixel dst){ return mult(GPixel_GetA(dst), src) + mult(255 - GPixel_GetA(src), dst); }
static inline GPixel _blend_kDstATop(GPixel src, GPixel dst){ return mult(GPixel_GetA(src), dst) + mult(255 - GPixel_GetA(dst), src); }
static inline GPixel _blend_kXor(GPixel src, GPixel dst){ return mult(255 - GPixel_GetA(src), dst) + mult(255 - GPixel_GetA(dst), src); }
static inline GPixel _blend_mult(GPixel src, GPixel dst){
    return GPixel_PackARGB(fastDiv255(GPixel_GetA(src) * GPixel_GetA(dst)),
                           fastDiv255(GPixel_GetR(src) * GPixel_GetR(dst)),
                           fastDiv255(GPixel_GetG(src) * GPixel_GetG(dst)),
                           fastDiv255(GPixel_GetB(src) * GPixel_GetB(dst)));
}


#endif
