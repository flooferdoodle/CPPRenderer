/*
 *  Copyright 2024 Felix Zhu
 */

#ifndef _g_starter_canvas_h_
#define _g_starter_canvas_h_

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/Segment.h"

#include <stack>

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device, const int w, const int h) : fDevice(device), width(w), height(h), ctm(GMatrix()) {}

    void clear(const GColor& color) override;

    void drawRect(const GRect&, const GPaint&) override;
    void drawConvexPolygon(const GPoint[], int count, const GPaint&) override;
    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                  int count, const int indices[], const GPaint&) override;
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                  int level, const GPaint&) override;

    void drawPath(const GPath& path, const GPaint&) override;
    /// @brief Returns true if edge (a, b) is vertically offscreen or horizontal
    bool canSkipEdge(const GPoint& a, const GPoint& b);

    void save();
    void restore();
    void concat(const GMatrix& matrix);

    const int getWidth() const { return width; }
    const int getHeight() const { return height; }

    std::vector<Segment> createSegments(const GPoint a, const GPoint b);

private:
    GBitmap fDevice;
    const int width;
    const int height;

    GMatrix ctm;

    std::stack<GMatrix> saveStack;

    

    template<typename F>
    void fillRow(int y, int l, int r, GPixel src, F blend);

    template<typename F>
    void fillRow(int y, int l, int r, GPixel src, F blend, GPixel& prevDst, GPixel& prevBlend);

    template<typename F>
    void fillRow(int y, int l, int r, GPixel src[], F blend);
};

#endif