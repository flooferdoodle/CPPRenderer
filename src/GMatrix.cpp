#include "../include/GMatrix.h"
#include <math.h>

GMatrix::GMatrix() : GMatrix(1.f, 0.f, 0.f, 0.f, 1.f, 0.f) {}

GMatrix GMatrix::Translate(float tx, float ty){
    return GMatrix(1.f, 0.f, tx,
                   0.f, 1.f, ty);
}
GMatrix GMatrix::Scale(float sx, float sy){
    return GMatrix(sx,  0.f, 0.f,
                   0.f, sy,  0.f);
}
GMatrix GMatrix::Rotate(float radians){
    float c = cosf(radians);
    float s = sinf(radians);
    return GMatrix(c, -s, 0.f,
                   s,  c, 0.f);
}

GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b){
    return GMatrix(a[0] * b[0] + a[2] * b[1],
                   a[0] * b[2] + a[2] * b[3],
                   a[0] * b[4] + a[2] * b[5] + a[4],
                   a[1] * b[0] + a[3] * b[1],
                   a[1] * b[2] + a[3] * b[3],
                   a[1] * b[4] + a[3] * b[5] + a[5]);
}

std::optional<GMatrix> GMatrix::invert() const{
    const float* mat = this->fMat;
    float det = mat[0] * mat[3] - mat[1] * mat[2];

    if(det == 0) return {};

    det = 1.f / det;

    return GMatrix( mat[3] * det,
                   -mat[2] * det,
                   (mat[2] * mat[5] - mat[3] * mat[4]) * det,
                   -mat[1] * det,
                    mat[0] * det,
                   (mat[1] * mat[4] - mat[0] * mat[5]) * det);
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const{
    const float* mat = this->fMat;
    for(int i = 0; i < count; i++){
        GPoint x;
        x.x = src[i].x * mat[0] + src[i].y * mat[2] + mat[4];
        x.y = src[i].x * mat[1] + src[i].y * mat[3] + mat[5];
        dst[i] = x;
    }
}