/*
 *  Copyright 2018 Mike Reed
 */

#include "../include/GPath.h"
#include "../include/GMatrix.h"
#include "CurveApprox.h"

#pragma region Mike`s code
GPath::GPath() {}
GPath::~GPath() {}

GPath& GPath::operator=(const GPath& src) {
    if (this != &src) {
        fPts = src.fPts;
        fVbs = src.fVbs;
    }
    return *this;
}

void GPath::reset() {
    fPts.clear();
    fVbs.clear();
}

void GPath::dump() const {
    Iter iter(*this);
    GPoint pts[GPath::kMaxNextPoints];
    while (auto v = iter.next(pts)) {
        switch (*v) {
            case kMove:
                printf("M %g %g\n", pts[0].x, pts[0].y);
                break;
            case kLine:
                printf("L %g %g\n", pts[1].x, pts[1].y);
                break;
            case kQuad:
                printf("Q %g %g  %g %g\n", pts[1].x, pts[1].y, pts[2].x, pts[2].y);
                break;
            case kCubic:
                printf("C %g %g  %g %g  %g %g\n",
                       pts[1].x, pts[1].y,
                       pts[2].x, pts[2].y,
                       pts[3].x, pts[3].y);
                break;
        }
    }
}

void GPath::quadTo(GPoint p1, GPoint p2) {
    assert(fVbs.size() > 0);
    fPts.push_back(p1);
    fPts.push_back(p2);
    fVbs.push_back(kQuad);
}

void GPath::cubicTo(GPoint p1, GPoint p2, GPoint p3) {
    assert(fVbs.size() > 0);
    fPts.push_back(p1);
    fPts.push_back(p2);
    fPts.push_back(p3);
    fVbs.push_back(kCubic);
}

/////////////////////////////////////////////////////////////////

GPath::Iter::Iter(const GPath& path) {
    fCurrPt = path.fPts.data();
    fCurrVb = path.fVbs.data();
    fStopVb = fCurrVb + path.fVbs.size();
}

std::optional<GPath::Verb> GPath::Iter::next(GPoint pts[]) {
    assert(fCurrVb <= fStopVb);
    if (fCurrVb == fStopVb) {
        return {};
    }
    Verb v = *fCurrVb++;
    switch (v) {
        case kMove:
            pts[0] = *fCurrPt++;
            break;
        case kLine:
            pts[0] = fCurrPt[-1];
            pts[1] = *fCurrPt++;
            break;
        case kQuad:
            pts[0] = fCurrPt[-1];
            pts[1] = *fCurrPt++;
            pts[2] = *fCurrPt++;
            break;
        case kCubic:
            pts[0] = fCurrPt[-1];
            pts[1] = *fCurrPt++;
            pts[2] = *fCurrPt++;
            pts[3] = *fCurrPt++;
            break;
    }
    return v;
}

constexpr int kDoneVerb = -1;

GPath::Edger::Edger(const GPath& path) {
    fPrevMove = nullptr;
    fCurrPt = path.fPts.data();
    fCurrVb = path.fVbs.data();
    fStopVb = fCurrVb + path.fVbs.size();
    fPrevVerb = kDoneVerb;
}

std::optional<GPath::Verb> GPath::Edger::next(GPoint pts[]) {
    assert(fCurrVb <= fStopVb);
    bool do_return = false;
    while (fCurrVb < fStopVb) {
        switch (*fCurrVb++) {
            case kMove:
                if (fPrevVerb == kLine) {
                    pts[0] = fCurrPt[-1];
                    pts[1] = *fPrevMove;
                    do_return = true;
                }
                fPrevMove = fCurrPt++;
                fPrevVerb = kMove;
                break;
            case kLine:
                pts[0] = fCurrPt[-1];
                pts[1] = *fCurrPt++;
                fPrevVerb = kLine;
                return kLine;
            case kQuad:
                pts[0] = fCurrPt[-1];
                pts[1] = *fCurrPt++;
                pts[2] = *fCurrPt++;
                fPrevVerb = kQuad;
                return kQuad;
            case kCubic:
                pts[0] = fCurrPt[-1];
                pts[1] = *fCurrPt++;
                pts[2] = *fCurrPt++;
                pts[3] = *fCurrPt++;
                fPrevVerb = kCubic;
                return kCubic;
        }
        if (do_return) {
            return kLine;
        }
    }
    if (fPrevVerb >= kLine && fPrevVerb <= kCubic) {
        pts[0] = fCurrPt[-1];
        pts[1] = *fPrevMove;
        fPrevVerb = kDoneVerb;
        return kLine;
    } else {
        return {};
    }
}
#pragma endregion

#pragma region My code

void GPath::addPolygon(const GPoint pts[], int count){
    this->moveTo(pts[0]);
    for(int i = 1; i < count; i++){
        this->lineTo(pts[i]);
    }
}

void GPath::addRect(const GRect& rect, Direction dir){
    GPoint pts[4] = {{rect.left,    rect.top},
                     {rect.right,   rect.top},
                     {rect.right,   rect.bottom},
                     {rect.left,    rect.bottom}};
    
    // Swap if opposite orientation
    if(dir == kCCW_Direction){
        pts[1] = {rect.left,    rect.bottom};
        pts[3] = {rect.right,   rect.top};
    }

    this->addPolygon(pts, 4);
}

const GPoint unit_circle_cw[16] {{1.f, 0.414213562373f}, {0.707106781187f, 0.707106781187f}, {0.414213562373f, 1.f}, 
                          {0.f, 1.f}, {-0.414213562373f, 1.f},{-0.707106781187f, 0.707106781187f},{-1.f, 0.414213562373f},
                          {-1.f, 0.f}, {-1.f, -0.414213562373f}, {-0.707106781187f, -0.707106781187f}, {-0.414213562373f, -1.f},
                          {0.f, -1.f}, {0.414213562373f, -1.f},{0.707106781187f, -0.707106781187f},{1.f, -0.414213562373f}, {1.f, 0.f}};
const GPoint unit_circle_ccw[16]  {{1.f, -0.414213562373f}, {0.707106781187f, -0.707106781187f}, {0.414213562373f, -1.f},
                            {0.f, -1.f}, {-0.414213562373f, -1.f}, {-0.707106781187f, -0.707106781187f}, {-1.f, -0.414213562373f},
                            {-1.f, 0.f},  {-1.f, 0.414213562373f}, {-0.707106781187f, 0.707106781187f}, {-0.414213562373f, 1.f},
                            {0.f, 1.f}, {0.414213562373f, 1.f}, {0.707106781187f, 0.707106781187f}, {1.f, 0.414213562373f}, {1.f, 0.f}};

void GPath::addCircle(GPoint center, float radius, Direction dir){
    const GPoint (&ref_circle)[16] = (dir == kCW_Direction) ? unit_circle_cw : unit_circle_ccw;
    
    moveTo({center.x + radius, center.y});

    for(int i = 0; i < 16; i += 2){
        quadTo(center + ref_circle[i] * radius, center + ref_circle[i+1] * radius);
    }
}

int getQuadCritPoints(const GPoint controls[], GPoint output[]){
    int count = 0;

    float t = (controls[0].x - controls[1].x) / (controls[0].x - 2 * controls[1].x + controls[2].x);
    if(t <= 1.f && t >= 0.f){
        output[count] = CurveApproximator::evalQuadraticCurve(controls, t);
        ++count;
    }

    t = (controls[0].y - controls[1].y) / (controls[0].y - 2 * controls[1].y + controls[2].y);
    if(t <= 1.f && t >= 0.f){
        output[count] = CurveApproximator::evalQuadraticCurve(controls, t);
        ++count;
    }

    return count;
}

int getCubicCritPoints(const GPoint controls[], GPoint output[]){
    int count = 0;

    // Get derivative form coefficients
    GPoint a = 3 * (controls[3] - 3 * controls[2] + 3 * controls[1] - controls[0]);
    GPoint b = 6 * (controls[2] - 2 * controls[1] + controls[0]);
    GPoint c = 3 * (controls[1] - controls[0]);

    // Calculate for x
    float discrim_sq = b.x * b.x - 4 * a.x * c.x;
    float t;
    if(discrim_sq == 0.f){
        t = -2 * c.x / b.x;
        if(t <= 1.f && t >= 0.f){
            output[count] = CurveApproximator::evalCubicCurve(controls, t);
            ++count;
        }
    }
    else if(discrim_sq > 0.f){
        float q = -0.5f * (b.x + (std::signbit(b.x) ? -1 : 1) * std::sqrt(discrim_sq));

        t = q / a.x;
        if(t <= 1.f && t >= 0.f){
            output[count] = CurveApproximator::evalCubicCurve(controls, t);
            ++count;
        }

        t = c.x / q;
        if(t <= 1.f && t >= 0.f){
            output[count] = CurveApproximator::evalCubicCurve(controls, t);
            ++count;
        }
    }

    // Calculate for y
    discrim_sq = b.y * b.y - 4 * a.y * c.y;
    if(discrim_sq == 0.f){
        t = -2 * c.y / b.y;
        if(t <= 1.f && t >= 0.f){
            output[count] = CurveApproximator::evalCubicCurve(controls, t);
            ++count;
        }
    }
    else if(discrim_sq > 0.f){
        float q = -0.5f * (b.y + (std::signbit(b.y) ? -1 : 1) * std::sqrt(discrim_sq));

        t = q / a.y;
        if(t <= 1.f && t >= 0.f){
            output[count] = CurveApproximator::evalCubicCurve(controls, t);
            ++count;
        }

        t = c.y / q;
        if(t <= 1.f && t >= 0.f){
            output[count] = CurveApproximator::evalCubicCurve(controls, t);
            ++count;
        }
    }

    return count;
}

GRect GPath::bounds() const{
    if(fPts.empty()) { return {0.f, 0.f, 0.f, 0.f}; }

    float minX = fPts[0].x;
    float maxX = fPts[0].x;
    float minY = fPts[0].y;
    float maxY = fPts[0].y;

    GPoint critPoints[4];
    int numCrit = 0;

    GPoint lastPoint = fPts[0];
    GPoint edge[4];
    GPath::Edger edger(*this);
    std::optional<GPath::Verb> v;
    while ((v = edger.next(edge)).has_value()) {
        switch(v.value()){
        case GPath::kMove:
            minX = std::min(lastPoint.x, minX);
            maxX = std::max(lastPoint.x, maxX);
            minY = std::min(lastPoint.y, minY);
            maxY = std::max(lastPoint.y, maxY);
            break;
        case GPath::kLine:
            minX = std::min(edge[0].x, minX);
            maxX = std::max(edge[0].x, maxX);
            minY = std::min(edge[0].y, minY);
            maxY = std::max(edge[0].y, maxY);
            lastPoint = edge[1];
            break;
        case GPath::kQuad:
            minX = std::min(edge[0].x, minX);
            maxX = std::max(edge[0].x, maxX);
            minY = std::min(edge[0].y, minY);
            maxY = std::max(edge[0].y, maxY);
            lastPoint = edge[2];

            numCrit = getQuadCritPoints(edge, critPoints);
            for(int i = 0; i < numCrit; i++){
                minX = std::min(critPoints[i].x, minX);
                maxX = std::max(critPoints[i].x, maxX);
                minY = std::min(critPoints[i].y, minY);
                maxY = std::max(critPoints[i].y, maxY);
            }

            break;
        case GPath::kCubic:
            minX = std::min(edge[0].x, minX);
            maxX = std::max(edge[0].x, maxX);
            minY = std::min(edge[0].y, minY);
            maxY = std::max(edge[0].y, maxY);
            lastPoint = edge[3];

            numCrit = getCubicCritPoints(edge, critPoints);
            for(int i = 0; i < numCrit; i++){
                minX = std::min(critPoints[i].x, minX);
                maxX = std::max(critPoints[i].x, maxX);
                minY = std::min(critPoints[i].y, minY);
                maxY = std::max(critPoints[i].y, maxY);
            }

            break;
        }
    }

    minX = std::min(lastPoint.x, minX);
    maxX = std::max(lastPoint.x, maxX);
    minY = std::min(lastPoint.y, minY);
    maxY = std::max(lastPoint.y, maxY);

    /*
    for(int i = 1; i < fPts.size(); ++i){
        minX = std::min(minX, fPts[i].x);
        maxX = std::max(maxX, fPts[i].x);
        minY = std::min(minY, fPts[i].y);
        maxY = std::max(maxY, fPts[i].y);
    }
    */
    return {minX, minY, maxX, maxY};
}

void GPath::transform(const GMatrix& ctm){
    ctm.mapPoints(fPts.data(), fPts.size());
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t){
    float inv_t = 1 - t;
    dst[0] = src[0];
    dst[1] = inv_t * src[0] + t * src[1];
    dst[3] = inv_t * src[1] + t * src[2];
    dst[4] = src[2];
    dst[2] = inv_t * dst[1] + t * dst[3];
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t){
    float inv_t = 1 - t;
    dst[0] = src[0];
    dst[6] = src[3];

    dst[1] = inv_t * src[0] + t * src[1];
    dst[3] = inv_t * src[1] + t * src[2];
    dst[5] = inv_t * src[2] + t * src[3];

    dst[2] = inv_t * dst[1] + t * dst[3];
    dst[4] = inv_t * dst[3] + t * dst[5];

    dst[3] = inv_t * dst[2] + t * dst[4];
}
#pragma endregion