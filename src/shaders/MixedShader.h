/*
 *  Copyright 2024 Felix Zhu
 */

#ifndef MixedShader_DEFINED
#define MixedShader_DEFINED

#include "../../include/GShader.h"
#include "../../include/GBlend.h"
#include <vector>

class MixedShader : public GShader {
public:
    MixedShader(GShader* shaders[], const int count) : shader_arr(&shaders[0]), numShaders(count) {}

    bool isOpaque() {
        for(int i = 0; i < numShaders; ++i){
            if(!shader_arr[i]->isOpaque()) return false;
        }
        return true;
    }

    bool setContext(const GMatrix& ctm){
        bool flag = true;
        for(int i = 0; i < numShaders; ++i){
            flag = flag && shader_arr[i]->setContext(ctm);
        }
        return flag;
    }

    void shadeRow(int pos_x, int pos_y, int count, GPixel row[]) {
        // Initialize to max 
        GPixel white_pix = GPixel_PackARGB(255, 255, 255, 255);
        for(int i = 0; i < count; ++i){
            row[i] = white_pix;
        }
        
        std::vector<GPixel> tempRow(count, white_pix);
        for(int i = 0; i < numShaders; ++i){
            shader_arr[i]->shadeRow(pos_x, pos_y, count, tempRow.data());
            
            for(int i = 0; i < count; i++){
                row[i] = _blend_mult(row[i], tempRow[i]);
            }
        }
    }

private:
    GShader** shader_arr;
    const int numShaders;
};

#endif