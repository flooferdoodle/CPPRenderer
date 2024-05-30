#include "BitmapShader.h"
#include <math.h>

/**
 *  Return a subclass of GShader that draws the specified bitmap and the local matrix.
 *  Returns null if the subclass can not be created.
 */
std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& device, const GMatrix& localMatrix, GTileMode tileMode){
    return std::unique_ptr<GShader>(new BitmapShader(device, localMatrix, tileMode));
}

bool BitmapShader::isOpaque(){
    return this->fDevice.isOpaque();
}

bool BitmapShader::setContext(const GMatrix& ctm){
    invContext = GMatrix::Concat(ctm, localMat).invert();
    return invContext.has_value();
}

void BitmapShader::shadeRow(int pos_x, int pos_y, int count, GPixel row[]){
    assert(this->invContext.has_value());

    const GVector step = invContext.value().e0();
    GPoint pos = invContext.value() * GPoint{pos_x + 0.5f, pos_y + 0.5f};
    
    std::vector<GPoint> points(count, GPoint());

    int x, y;
    switch(tileMode){
        case GTileMode::kClamp:
            for (int i = 0; i < count; i++) {
                x = std::clamp((int) std::floor(pos.x), 0, fDevice.width() - 1);
                y = std::clamp((int) std::floor(pos.y), 0, fDevice.height() - 1);

                row[i] = *fDevice.getAddr(x, y);
                
                pos += step;
            }
            break;
        case GTileMode::kRepeat:
            for (int i = 0; i < count; i++) {
                x = ((int) std::floor(pos.x)) % fDevice.width();
                y = ((int) std::floor(pos.y)) % fDevice.height();

                if(x < 0) x += fDevice.width();
                if(y < 0) y += fDevice.height();

                row[i] = *fDevice.getAddr(x, y);
                
                pos += step;
            }
            break;
        case GTileMode::kMirror:
            for (int i = 0; i < count; i++) {
                x = ((int) std::floor(pos.x)) % (fDevice.width() * 2);
                y = ((int) std::floor(pos.y)) % (fDevice.height() * 2);

                if(x < 0) x += fDevice.width() * 2;
                if(y < 0) y += fDevice.height() * 2;

                if(x >= fDevice.width()) x = fDevice.width()*2 - x - 1;
                if(y >= fDevice.height()) y = fDevice.height()*2 - y - 1;

                row[i] = *fDevice.getAddr(x, y);
                
                pos += step;
            }
            break;
    }
}