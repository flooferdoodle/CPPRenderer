/*
 *  Copyright 2024 Felix Zhu
 */

#include "MyCanvas.h"
#include "include/Segment.h"
#include "include/GBlend.h"
#include "src/shaders/BitmapShader.h"
#include "src/shaders/TriGradientShader.h"
#include "src/shaders/TriTextureShader.h"
#include "src/shaders/MixedShader.h"
#include "include/GPath.h"
#include "src/CurveApprox.h"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <list>
#define INV_7 0.1428571429f

#pragma region Helper Functions

GBlendMode getOptimizedBlendMode(const GPaint& paint){
    bool hasShader = paint.getShader() != nullptr;

    GBlendMode blendMode = paint.getBlendMode();
    float alpha = paint.getAlpha();
    if(!hasShader){
        if(alpha == 0.f){
            switch(paint.getBlendMode()){
                case GBlendMode::kClear:
                case GBlendMode::kSrc:
                case GBlendMode::kSrcIn:
                case GBlendMode::kDstIn:
                case GBlendMode::kSrcOut:
                case GBlendMode::kDstATop:
                    blendMode = GBlendMode::kClear;
                    break;
                case GBlendMode::kDst:
                case GBlendMode::kSrcOver:
                case GBlendMode::kDstOver:
                case GBlendMode::kDstOut:
                case GBlendMode::kSrcATop:
                case GBlendMode::kXor:
                    return GBlendMode::kDst;
            }
        }
        else if(alpha == 1.f){
            switch(paint.getBlendMode()){
                case GBlendMode::kClear:
                case GBlendMode::kDstOut:
                    blendMode = GBlendMode::kClear;
                    break;
                case GBlendMode::kSrc:
                case GBlendMode::kSrcOver:
                    blendMode = GBlendMode::kSrc;
                    break;
                case GBlendMode::kDstOver:
                    blendMode = GBlendMode::kDstOver;
                    break;
                case GBlendMode::kSrcIn:
                case GBlendMode::kSrcATop:
                    blendMode = GBlendMode::kSrcIn;
                    break;
                case GBlendMode::kSrcOut:
                case GBlendMode::kXor:
                    blendMode = GBlendMode::kSrcOut;
                    break;
                case GBlendMode::kDstATop:
                    blendMode = GBlendMode::kDstATop;
                    break;
                
                case GBlendMode::kDstIn:
                case GBlendMode::kDst:
                    return GBlendMode::kDst;
            }
        }
    }

    else{
        if(paint.getShader()->isOpaque()){
            switch(paint.getBlendMode()){
                case GBlendMode::kClear:
                case GBlendMode::kDstOut:
                    blendMode = GBlendMode::kClear;
                    break;
                case GBlendMode::kSrc:
                case GBlendMode::kSrcOver:
                    blendMode = GBlendMode::kSrc;
                    break;
                case GBlendMode::kDstOver:
                    blendMode = GBlendMode::kDstOver;
                    break;
                case GBlendMode::kSrcIn:
                case GBlendMode::kSrcATop:
                    blendMode = GBlendMode::kSrcIn;
                    break;
                case GBlendMode::kSrcOut:
                case GBlendMode::kXor:
                    blendMode = GBlendMode::kSrcOut;
                    break;
                case GBlendMode::kDstATop:
                    blendMode = GBlendMode::kDstATop;
                    break;
                
                case GBlendMode::kDstIn:
                case GBlendMode::kDst:
                    return GBlendMode::kDst;
            }
        }
    }

    return blendMode;
}

#pragma endregion

#pragma region Class Functions

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device, device.width(), device.height()));
}

void MyCanvas::save(){
    this->saveStack.push(GMatrix(this->ctm));
}
void MyCanvas::restore(){
    assert(this->saveStack.size() > 0);
    this->ctm = saveStack.top();
    saveStack.pop();
}
void MyCanvas::concat(const GMatrix& matrix){
    ctm = GMatrix::Concat(ctm, matrix);
}

void MyCanvas::clear(const GColor& color) {
    GPixel premul = toPremul(color);
    GPixel* curr_pixel = fDevice.pixels();
    for(int i = 0; i < width * height; i++){
        *curr_pixel = premul;
        curr_pixel++;
    }
}

void MyCanvas::drawRect(const GRect& rect, const GPaint& paint) {
    GPoint arr[] = {{rect.left, rect.top},
                    {rect.right, rect.top},
                    {rect.right, rect.bottom},
                    {rect.left, rect.bottom}};
    this->drawConvexPolygon(arr, 4, paint);
}

template<typename F>
void MyCanvas::fillRow(int y, int l, int r, GPixel src, F blend, GPixel& prevDst, GPixel& prevBlend){
    for(; l < r; l++){
        GPixel* dst = fDevice.getAddr(l, y);

        if(*dst == prevDst){
            *dst = prevBlend;
            continue;
        }
        prevDst = *dst;
        prevBlend = blend(src, *dst);
        *dst = prevBlend;
    }
}

template<typename F>
void MyCanvas::fillRow(int y, int l, int r, GPixel src, F blend){
    for(; l < r; l++){
        GPixel* dst = fDevice.getAddr(l, y);
        *dst = blend(src, *dst);;
    }
}

template<typename F>
void MyCanvas::fillRow(int y, int l, int r, GPixel srcRow[], F blend){
    for(int i = 0; l + i < r; i++){
        GPixel* dst = fDevice.getAddr(l + i, y);
        *dst = blend(srcRow[i], *dst);
    }
}

std::vector<Segment> MyCanvas::createSegments(const GPoint a, const GPoint b){
    float yDiff = b.y - a.y;
    bool up = (yDiff < 0);

    float slope = (b.x - a.x) / yDiff;
    float xIntercept = a.x - slope * a.y;

    std::vector<Segment> segs;

    float minY = std::max(std::min(a.y, b.y), 0.f);
    float maxY = std::min(std::max(a.y, b.y), (float)height);

    if(slope == 0){ // vertical line
        segs.push_back(Segment(minY, maxY, std::clamp(a.x, 0.f, (float)width), up));
        return segs;
    }

    // Otherwise, clip if necessary

    // Sort by x
    GPoint left = (a.x < b.x) ? a : b;
    GPoint right = (a.x > b.x) ? a : b;

    // Vertical clipping
    if(left.y < 0){
        left.x = xIntercept;
        left.y = 0;
    }
    if(left.y > height){
        left.x = height * slope + xIntercept;
        left.y = height;
    }
    if(right.y < 0){
        right.x = xIntercept;
        right.y = 0;
    }
    if(right.y > height){
        right.x = height * slope + xIntercept;
        right.y = height;
    }

    // Horizontal clipping
    if(left.x < 0){
        if(right.x <= 0){ // clipped off left side
            segs.push_back(Segment(minY, maxY, 0, up));
            return segs;
        }

        // Otherwise, split segment
        float yInt = -xIntercept / slope;
        segs.push_back(Segment(std::min(left.y, yInt), std::max(left.y, yInt), 0, up));
        left.x = 0;
        left.y = yInt;
    }

    if(right.x > width){
        if(left.x >= width){ // clipped off right side
            segs.push_back(Segment(minY, maxY, width, up));
            return segs;
        }

        // Otherwise, split segment
        float yInt = (width - xIntercept) / slope;
        segs.push_back(Segment(std::min(right.y, yInt), std::max(right.y, yInt), width, up));
        right.x = width;
        right.y = yInt;
    }

    if(std::round(left.y) != std::round(right.y)){
        segs.push_back(Segment(std::min(left.y, right.y), std::max(left.y, right.y), slope, xIntercept, up));
    }


    return segs;
}


bool MyCanvas::canSkipEdge(const GPoint& a, const GPoint& b){
    return (std::round(a.y) == std::round(b.y) || 
           (a.y <= 0 && b.y <= 0) ||
           (a.y >= height && b.y >= height));
}

#pragma endregion

#pragma region Drawing Functions

void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                        int count, const int indices[], const GPaint& texturePaint){
    /*
    Step 1: loop through each triangle
    Step 2: Create an instance of shader to handle colors or tex or both
    Step 3: pass to drawConvexPolygon
    */
    GPoint tri[3] = {{0.f,0.f},{0.f,0.f},{0.f,0.f}};
    GPaint paint = GPaint();
    int n = 0;

    bool hasColor = colors != nullptr;
    bool hasTex = texs != nullptr && texturePaint.getShader() != nullptr;


    if(hasColor && hasTex) {
        TriGradientShader colShader = TriGradientShader();
        TriTextureShader texShader = TriTextureShader(texturePaint.getShader());
        GShader* shaders[2] = {&colShader, &texShader};
        MixedShader triShader = MixedShader(shaders, 2);
        paint.setShader(&triShader);
        for(int i = 0; i < count; ++i) {
            tri[0] = verts[indices[n]]; tri[1] = verts[indices[n+1]]; tri[2] = verts[indices[n+2]];
            colShader.reset(tri[0], tri[1], tri[2],
                colors[indices[n]], colors[indices[n+1]], colors[indices[n+2]]);
            texShader.reset(tri[0], tri[1], tri[2],
                texs[indices[n]], texs[indices[n+1]], texs[indices[n+2]]);

            drawConvexPolygon(tri, 3, paint);

            n += 3;
        }
    }
    else if(hasColor) {
        TriGradientShader triShader = TriGradientShader();
        paint.setShader(&triShader);
        for(int i = 0; i < count; ++i) {
            tri[0] = verts[indices[n]]; tri[1] = verts[indices[n+1]]; tri[2] = verts[indices[n+2]];
            triShader.reset(tri[0], tri[1], tri[2],
                colors[indices[n]], colors[indices[n+1]], colors[indices[n+2]]);
            
            drawConvexPolygon(tri, 3, paint);

            n += 3;
        }
    }
    else if(hasTex) {
        TriTextureShader triShader = TriTextureShader(texturePaint.getShader());
        paint.setShader(&triShader);
        for(int i = 0; i < count; ++i) {
            tri[0] = verts[indices[n]]; tri[1] = verts[indices[n+1]]; tri[2] = verts[indices[n+2]];
            triShader.reset(tri[0], tri[1], tri[2],
                texs[indices[n]], texs[indices[n+1]], texs[indices[n+2]]);
            
            drawConvexPolygon(tri, 3, paint);

            n += 3;
        }
    }

    
}

template<typename T>
void splitQuad(const T verts[4], const int n, const float inv_n, T vertices[]){
    T AB_step = (verts[1] - verts[0]) * inv_n;
    T DC_step = (verts[2] - verts[3]) * inv_n;
    T top = verts[0];
    T bot = verts[3];

    T v_step;
    T curr;
    int i = 0;
    for(int u = 0; u < n; ++u){
        curr = top;
        v_step = (bot - top) * inv_n;

        for(int v = 0; v < n; ++v){
            vertices[i] = curr;

            // update iterators
            curr += v_step;
            ++i;
        }

        top += AB_step; bot += DC_step;
    }
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                        int level, const GPaint& paint){
    if(level == 0) return drawMesh(verts, colors, texs, 2, new int[6]{0, 1, 3, 3, 1, 2}, paint);

    ++level;
    float inv_level = (1.f / level);
    int n = level + 1;

    // Split vertices
    std::vector<GPoint> vertices (n*n, {0.f, 0.f});
    splitQuad(verts, n, inv_level, vertices.data());

    // Calculate indexing
    std::vector<int> indices(level * level * 6, 0);
    int i = 0;
    int ind_i = 0;
    for(int u = 0; u < level; ++u){
        for(int v = 0; v < level; ++v){
            /* building tris for quad
                    i---n+i
                    |  /|
                    | / |
                    |/  |
                  i+1---n+i+1
            */

            indices[ind_i]   = i;
            indices[ind_i+1] = n + i;
            indices[ind_i+2] = i + 1;

            indices[ind_i+3] = indices[ind_i+2];
            indices[ind_i+4] = indices[ind_i+1];
            indices[ind_i+5] = indices[ind_i+1] + 1;

            assert(i < vertices.size());
            assert(n + i < vertices.size());
            
            ++i;
            ind_i += 6;
        }
        ++i;
    }

    std::vector<GColor> vert_cols;
    std::vector<GPoint> vert_texs;
    if(colors != nullptr){
        vert_cols = std::vector<GColor>(vertices.size(), {0.f, 0.f, 0.f, 0.f});
        splitQuad(colors, n, inv_level, vert_cols.data());
    }
    if(texs != nullptr && paint.getShader() != nullptr){
        vert_texs = std::vector<GPoint>(vertices.size(), {0.f, 0.f});
        splitQuad(texs, n, inv_level, vert_texs.data());
    }

    drawMesh(vertices.data(), vert_cols.data(), vert_texs.data(),
             level * level * 2, indices.data(), paint);
}


void MyCanvas::drawPath(const GPath& path, const GPaint& paint){
    bool hasShader = paint.getShader() != nullptr;

    // Get path and transform if ctm is not identity
    GPath copyPath = path;
    if(this->ctm != GMatrix()) { copyPath.transform(this->ctm); }

    // Get blendmode
    GBlendMode blendMode = getOptimizedBlendMode(paint);
    if(blendMode == GBlendMode::kDst) return;

    // Generate segments
    std::vector<Segment> segments;
    GPoint edge[4];
    GPath::Edger edger(copyPath);
    std::optional<GPath::Verb> v;
    while ((v = edger.next(edge)).has_value()) {
        std::vector<Segment> segs;

        switch(v.value()){
        case GPath::kMove:
            continue;
        case GPath::kLine:
            // Skip horizontal or vertically offscreen edges
            if(canSkipEdge(edge[0], edge[1])){ break; }

            // Create segments and clamp to canvas
            segs = createSegments(edge[0], edge[1]);
            break;

        case GPath::kQuad:
        {
            std::vector<GPoint> approxPath = CurveApproximator::convertQuadratic(edge);
            //segs.reserve(approxPath.size() * 2);

            for(int i = 0; i < approxPath.size() - 1; ++i){
                if(canSkipEdge(approxPath[i], approxPath[i+1])) continue;

                std::vector<Segment> newSegs = createSegments(approxPath[i], approxPath[i+1]);
                segs.insert(segs.end(), newSegs.begin(), newSegs.end());
            }

            break;
        }
        case GPath::kCubic:
        {
            std::vector<GPoint> approxPath = CurveApproximator::convertCubic(edge);
            //segs.reserve(approxPath.size() * 2);

            for(int i = 0; i < approxPath.size() - 1; ++i){
                if(canSkipEdge(approxPath[i], approxPath[i+1])) continue;

                std::vector<Segment> newSegs = createSegments(approxPath[i], approxPath[i+1]);
                segs.insert(segs.end(), newSegs.begin(), newSegs.end());
            }

            break;
        }
        }

        segs.shrink_to_fit();

        segments.insert(segments.end(), segs.begin(), segs.end());
    }

    if(segments.size() == 0) return;

    std::sort(segments.begin(), segments.end());


    // Get segments at the top y value
    std::list<Segment*> activeSegs;

    int nextSegIndex = 0;
    while(nextSegIndex < segments.size() && segments[nextSegIndex].minY == segments[0].minY){
        activeSegs.push_back(&segments[nextSegIndex]);
        nextSegIndex++;
    }

    // Allocate enough space for arbitrary number of intersections
    std::vector<SegmentIntersection> intersections(copyPath.countPoints());

    int numActiveSegs = activeSegs.size();
    int W = 0;
    int i = 0;
    std::optional<int> l = {};
    std::optional<int> r = {};

    // Iterate through y-values
    if(!hasShader){
        GPixel src = toPremul(paint);
        // Loop through pixel rows
        for(int y = segments[0].minY; !activeSegs.empty() || nextSegIndex < segments.size(); ++y){
            // Calculate intersections - O(n)
            i = 0;
            for(auto it = activeSegs.begin(); it != activeSegs.end(); ++it){
                intersections[i] = {(int)std::floor((*it)->horizontalIntersect(y + 0.5f) + 0.5f), (*it)->up};
                i++;
            }
            
            // Sort intersections - O(n log n)
            // TODO: O(n) check to see if already in sorted order
            std::sort(intersections.begin(), intersections.begin() + numActiveSegs);

            // Blit row - O(n + m)
            W = 0;
            l = {}; r = {};
            for(int i = 0; i < numActiveSegs; ++i){
                W += (intersections[i].up) ? 1 : -1;

                if(W != 0 && !l.has_value()){
                    l = intersections[i].x;
                }
                else if(W == 0 && l.has_value()){ // 2nd condition might be redundant
                    r = intersections[i].x;

                    // Blit row from l to r
                    switch(blendMode){
                    case GBlendMode::kClear:
                        fillRow(y, l.value(), r.value(), src, _blend_kClear);
                        break;
                    case GBlendMode::kSrc:
                        fillRow(y, l.value(), r.value(), src, _blend_kSrc);
                        break;
                    case GBlendMode::kSrcOver:
                        fillRow(y, l.value(), r.value(), src, _blend_kSrcOver);
                        break;
                    case GBlendMode::kDstOver:
                        fillRow(y, l.value(), r.value(), src, _blend_kDstOver);
                        break;
                    case GBlendMode::kSrcIn:
                        fillRow(y, l.value(), r.value(), src, _blend_kSrcIn);
                        break;
                    case GBlendMode::kDstIn:
                        fillRow(y, l.value(), r.value(), src, _blend_kDstIn);
                        break;
                    case GBlendMode::kSrcOut:
                        fillRow(y, l.value(), r.value(), src, _blend_kSrcOut);
                        break;
                    case GBlendMode::kDstOut:
                        fillRow(y, l.value(), r.value(), src, _blend_kDstOut);
                        break;
                    case GBlendMode::kSrcATop:
                        fillRow(y, l.value(), r.value(), src, _blend_kSrcATop);
                        break;
                    case GBlendMode::kDstATop:
                        fillRow(y, l.value(), r.value(), src, _blend_kDstATop);
                        break;
                    case GBlendMode::kXor:
                        fillRow(y, l.value(), r.value(), src, _blend_kXor);
                        break;
                }

                    l = {}; r = {};
                }
            }

            // Update active segments - O(n + k)
            activeSegs.remove_if( [y] (Segment* seg) { return y >= seg->maxY; } );
            // TODO: insert new segments in x-sorted order, and maybe have a way to swap order if reach intersections
            while(nextSegIndex < segments.size() && segments[nextSegIndex].minY <= y + 1){
                activeSegs.push_back(&segments[nextSegIndex]);
                nextSegIndex++;
            }
            numActiveSegs = activeSegs.size();
        }
    }
    else{
        GShader &shader = *(paint.getShader());
        bool hasContext = shader.setContext(ctm);

        assert(hasContext);

        // Loop through pixel rows
        for(int y = segments[0].minY; !activeSegs.empty() || nextSegIndex < segments.size(); ++y){
            // Calculate intersections - O(n)
            i = 0;
            for(auto it = activeSegs.begin(); it != activeSegs.end(); ++it){
                intersections[i] = {(int)std::floor((*it)->horizontalIntersect(y + 0.5f) + 0.5f), (*it)->up};
                i++;
            }
            
            // Sort intersections - O(n log n)
            // TODO: O(n) check to see if already in sorted order
            std::sort(intersections.begin(), intersections.begin() + numActiveSegs);

            // Blit row - O(n + m)
            W = 0;
            l = {}; r = {};
            for(int i = 0; i < numActiveSegs; ++i){
                W += (intersections[i].up) ? 1 : -1;

                if(W != 0 && !l.has_value()){
                    l = intersections[i].x;
                }
                else if(W == 0 && l.has_value()){ // 2nd condition might be redundant
                    r = intersections[i].x;

                    // Blit row from l to r

                    std::vector<GPixel> srcRow;
                    srcRow.reserve(r.value()-l.value());
                    shader.shadeRow(l.value(), y, r.value()-l.value(), &srcRow[0]);

                    switch(blendMode){
                    case GBlendMode::kClear:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kClear);
                        break;
                    case GBlendMode::kSrc:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kSrc);
                        break;
                    case GBlendMode::kSrcOver:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kSrcOver);
                        break;
                    case GBlendMode::kDstOver:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kDstOver);
                        break;
                    case GBlendMode::kSrcIn:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kSrcIn);
                        break;
                    case GBlendMode::kDstIn:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kDstIn);
                        break;
                    case GBlendMode::kSrcOut:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kSrcOut);
                        break;
                    case GBlendMode::kDstOut:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kDstOut);
                        break;
                    case GBlendMode::kSrcATop:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kSrcATop);
                        break;
                    case GBlendMode::kDstATop:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kDstATop);
                        break;
                    case GBlendMode::kXor:
                        fillRow(y, l.value(), r.value(), &srcRow[0], _blend_kXor);
                        break;
                    }

                    l = {}; r = {};
                }
            }

            // Update active segments - O(n + k)
            activeSegs.remove_if( [y] (Segment* seg) { return y >= seg->maxY; } );
            // TODO: insert new segments in x-sorted order, and maybe have a way to swap order if reach intersections
            while(nextSegIndex < segments.size() && segments[nextSegIndex].minY <= y + 1){
                activeSegs.push_back(&segments[nextSegIndex]);
                nextSegIndex++;
            }
            numActiveSegs = activeSegs.size();
        }
    }
}


void MyCanvas::drawConvexPolygon(const GPoint orig_points[], int count, const GPaint& paint){

    std::vector<GPoint> points(orig_points, orig_points + count);

    if(this->ctm != GMatrix()){ // Transform only if not identity
        ctm.mapPoints(&points[0], orig_points, count);
    }

    bool hasShader = paint.getShader() != nullptr;

    GBlendMode blendMode = getOptimizedBlendMode(paint);
    if(blendMode == GBlendMode::kDst) return;
    
    // Generate segments
    std::vector<Segment> segments;
    int next_i = 1;
    for(int i = 0; i < count; i++){
        next_i = (i+1)%count;
        // Check if line is approximately horizontal or entirely offscreen, in which case skip
        
        if(std::round(points[i].y) == std::round(points[next_i].y) || 
          (points[i].y <= 0 && points[next_i].y <= 0) ||
          (points[i].y >= height && points[next_i].y >= height)){
            continue;
        }

        std::vector<Segment> segs = createSegments(points[i], points[next_i]);
        segments.insert(segments.end(), segs.begin(), segs.end());
    }

    if(segments.size() == 0) return;

    std::sort(segments.begin(), segments.end());

    Segment* seg1 = &segments[0];
    Segment* seg2 = &segments[1];

    std::vector<Segment>::iterator lastUsed = segments.begin() + 1;

    int curr_y = segments[0].minY;

    // For pixels
    if(!hasShader){
        GPixel src = toPremul(paint);
        while(seg1 != nullptr && seg2 != nullptr){
            // Ensure seg2 is the shorter segment
            if(seg1->maxY < seg2->maxY){
                std::swap(seg1, seg2);
            }


            float x1 = seg1->horizontalIntersect(curr_y + 0.5f);
            float x2 = seg2->horizontalIntersect(curr_y + 0.5f);

            float dx1 = seg1->slope;
            float dx2 = seg2->slope;

            if(x2 < x1){
                std::swap(x1, x2);
                std::swap(dx1, dx2);
            }

            GPixel prevDst = 0x1;
            GPixel prevBlend;
            for(int y = curr_y; y <= seg2->maxY; y++){
                // Draw all rows within these segments

                //int l = std::clamp((int)std::floor(x1 + 0.5f), 0, fDevice.width() - 1);
                //int r = std::clamp((int)std::floor(x2 - 0.5f), 0, fDevice.width() - 1);
                int l = (int)std::floor(x1 + 0.5f);
                int r = (int)std::floor(x2 + 0.5f);


                switch(blendMode){
                    case GBlendMode::kClear:
                        fillRow(y, l, r, src, _blend_kClear, prevDst, prevBlend);
                        break;
                    case GBlendMode::kSrc:
                        fillRow(y, l, r, src, _blend_kSrc, prevDst, prevBlend);
                        break;
                    case GBlendMode::kDst:
                        //_drawConvexPolygon(points, count, paint, _blend_kDst);
                        break;
                    case GBlendMode::kSrcOver:
                        fillRow(y, l, r, src, _blend_kSrcOver, prevDst, prevBlend);
                        break;
                    case GBlendMode::kDstOver:
                        fillRow(y, l, r, src, _blend_kDstOver, prevDst, prevBlend);
                        break;
                    case GBlendMode::kSrcIn:
                        fillRow(y, l, r, src, _blend_kSrcIn, prevDst, prevBlend);
                        break;
                    case GBlendMode::kDstIn:
                        fillRow(y, l, r, src, _blend_kDstIn, prevDst, prevBlend);
                        break;
                    case GBlendMode::kSrcOut:
                        fillRow(y, l, r, src, _blend_kSrcOut, prevDst, prevBlend);
                        break;
                    case GBlendMode::kDstOut:
                        fillRow(y, l, r, src, _blend_kDstOut, prevDst, prevBlend);
                        break;
                    case GBlendMode::kSrcATop:
                        fillRow(y, l, r, src, _blend_kSrcATop, prevDst, prevBlend);
                        break;
                    case GBlendMode::kDstATop:
                        fillRow(y, l, r, src, _blend_kDstATop, prevDst, prevBlend);
                        break;
                    case GBlendMode::kXor:
                        fillRow(y, l, r, src, _blend_kXor, prevDst, prevBlend);
                        break;
                }

                x1 += dx1;
                x2 += dx2;
            }

            curr_y = seg2->maxY + 1;
            
            if(lastUsed == segments.end()){
                seg2 = nullptr;
                continue;
            }

            lastUsed++;
            seg2 = &*lastUsed;

            if(seg1->maxY < curr_y){
                if(lastUsed == segments.end()){
                    seg1 = nullptr;
                    continue;
                }

                lastUsed++;
                seg1 = &*lastUsed;
            }
        }
    }

    // For bitmap shader
    else{
        GShader &shader = *(paint.getShader());
        bool hasContext = shader.setContext(ctm);

        if(!hasContext) return;

        while(seg1 != nullptr && seg2 != nullptr){
            // Ensure seg2 is the shorter segment
            if(seg1->maxY < seg2->maxY){
                std::swap(seg1, seg2);
            }


            float x1 = seg1->horizontalIntersect(curr_y + 0.5f);
            float x2 = seg2->horizontalIntersect(curr_y + 0.5f);

            float dx1 = seg1->slope;
            float dx2 = seg2->slope;

            if(x2 < x1){
                std::swap(x1, x2);
                std::swap(dx1, dx2);
            }

            for(int y = curr_y; y <= seg2->maxY; y++){
                // Draw all rows within these segments

                int l = (int)std::floor(x1 + 0.5f);
                int r = (int)std::floor(x2 + 0.5f);

                std::vector<GPixel> srcRow(r-l, -1);
                shader.shadeRow(l, y, r-l, &srcRow[0]);

                switch(blendMode){
                    case GBlendMode::kClear:
                        fillRow(y, l, r, &srcRow[0], _blend_kClear);
                        break;
                    case GBlendMode::kSrc:
                        fillRow(y, l, r, &srcRow[0], _blend_kSrc);
                        break;
                    case GBlendMode::kDst:
                        //_drawConvexPolygon(points, count, paint, _blend_kDst);
                        break;
                    case GBlendMode::kSrcOver:
                        fillRow(y, l, r, &srcRow[0], _blend_kSrcOver);
                        break;
                    case GBlendMode::kDstOver:
                        fillRow(y, l, r, &srcRow[0], _blend_kDstOver);
                        break;
                    case GBlendMode::kSrcIn:
                        fillRow(y, l, r, &srcRow[0], _blend_kSrcIn);
                        break;
                    case GBlendMode::kDstIn:
                        fillRow(y, l, r, &srcRow[0], _blend_kDstIn);
                        break;
                    case GBlendMode::kSrcOut:
                        fillRow(y, l, r, &srcRow[0], _blend_kSrcOut);
                        break;
                    case GBlendMode::kDstOut:
                        fillRow(y, l, r, &srcRow[0], _blend_kDstOut);
                        break;
                    case GBlendMode::kSrcATop:
                        fillRow(y, l, r, &srcRow[0], _blend_kSrcATop);
                        break;
                    case GBlendMode::kDstATop:
                        fillRow(y, l, r, &srcRow[0], _blend_kDstATop);
                        break;
                    case GBlendMode::kXor:
                        fillRow(y, l, r, &srcRow[0], _blend_kXor);
                        break;
                }

                x1 += dx1;
                x2 += dx2;
            }

            curr_y = seg2->maxY + 1;
            
            if(lastUsed == segments.end()){
                seg2 = nullptr;
                continue;
            }

            lastUsed++;
            seg2 = &*lastUsed;

            if(seg1->maxY < curr_y){
                if(lastUsed == segments.end()){
                    seg1 = nullptr;
                    continue;
                }

                lastUsed++;
                seg1 = &*lastUsed;
            }
        }
    }
}

#pragma endregion

#pragma region Draw Something

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    return "";
}
#pragma endregion