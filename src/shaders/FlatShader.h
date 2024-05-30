/*
 *  Copyright 2024 Felix Zhu
 */

#ifndef FlatShader_DEFINED
#define FlatShader_DEFINED

#include "../../include/GShader.h"
#include "../../include/vec.h"
#include "../../include/GColor.h"
#include "../../include/GBlend.h"
#include "TriTextureShader.h"
#include <cmath>

class DirectionalFlatShader : public GShader {
public:
    DirectionalFlatShader(vec3 lightDir, vec3 col = vec3{1.f, 1.f, 1.f}) : dir(vec3::normalize(lightDir)), lightCol(col) {}

    bool isOpaque() { return true; }

    bool setContext(const GMatrix& ctm) { return true; }

    void setLightDir(vec3 lightDir) { dir = vec3::normalize(lightDir); }
    void calcColor(vec3 norm, GColor triColor) {
        float diff = std::max(vec3::dot(norm, -dir), 0.f);
        diffuse = diff * lightCol;
        vec3 result = diffuse * vec3{triColor.r, triColor.g, triColor.b};
        shadeCol = toPremul({result.x(), result.y(), result.z(), 1.f});
    }

    void shadeRow(int x, int y, int count, GPixel row[]) {
        for(int i = 0; i < count; ++i){
            row[i] = shadeCol;
        }
    }

private:
    vec3 dir;
    vec3 lightCol;
    
    GPixel shadeCol;
    vec3 diffuse;
};

#endif