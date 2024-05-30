#ifndef GBuffer_DEFINED
#define GBuffer_DEFINED

#include "include/vec.h"
#include <vector>
#include "include/Segment.h"
#include "MyCanvas.h"

using namespace std;

struct PixelData {
    float depth;
    vec3 position;
    vec3 normal;
    vec3 albedo;
    float specular;
};

class GBuffer {
public:
    GBuffer(const GISize dim) : _dim(dim) {
        B_depth = vector<vector<float>>(dim.height, vector<float>(dim.width, FLT_MAX));
        B_invdepth = vector<vector<float>>(dim.height, vector<float>(dim.width, 0.f));

        B_position = vector<vector<vec3>>(dim.height, vector<vec3>(dim.width));
        B_normal   = vector<vector<vec3>>(dim.height, vector<vec3>(dim.width));
        B_albedo   = vector<vector<vec3>>(dim.height, vector<vec3>(dim.width));
        B_specular = vector<vector<float>>(dim.height, vector<float>(dim.width));
    };

    void drawTri(int indices[3], const vector<vec2> &proj_verts, vec3 verts[3],
                 vec3 norms[3], const vector<GColor> &cols, float specular) {
        // Use Pineda's method or similar to rasterize triangle
        // Perform barycentric interpolation (see z-depth interpolation in scratch-a-pixel) to interpolate 
            // Note that this must be in camera space vertices, so we pass in 2d projection for rasterizing and 3d camera view for interpolating
        // Optimizations:
        /*
            Better triangle rasterizer (?)
        */

        vec2 proj_tri[3] = {proj_verts[indices[0]], proj_verts[indices[1]], proj_verts[indices[2]]};

        // Pineda's method (parallizable, but therefore slow on cpu)
        // Calculate bounding box for extents
        int minX = (int) round(min(proj_tri[0].x(), min(proj_tri[1].x(), proj_tri[2].x())));
        int maxX = (int) round(max(proj_tri[0].x(), max(proj_tri[1].x(), proj_tri[2].x())));
        int minY = (int) round(min(proj_tri[0].y(), min(proj_tri[1].y(), proj_tri[2].y())));
        int maxY = (int) round(max(proj_tri[0].y(), max(proj_tri[1].y(), proj_tri[2].y())));

        minX = max(0, minX);
        maxX = min(_dim.width, maxX);
        minY = max(0, minY);
        maxY = min(_dim.height, maxY);

        // Loop over pixels and perform edge checks for pixels using DDA for updates
        vec2 p = {minX + 0.5f, minY + 0.5f};
        float area = edgeFunction(proj_tri[0], proj_tri[1], proj_tri[2]);
        float inv_area = 1.f / area;
        float e0 = edgeFunction(proj_tri[1], proj_tri[2], p);
        float e1 = edgeFunction(proj_tri[2], proj_tri[0], p);
        float e2 = edgeFunction(proj_tri[0], proj_tri[1], p);

        // Since negative z is inwards, all z values relative to camera should be negative, so flip
        float inv_zs[3] = {-1.f / verts[0].z(), -1.f / verts[1].z(), -1.f / verts[2].z()};
        // GColor correctedCols[3] = {cols[indices[0]] * inv_zs[0],
        //                            cols[indices[1]] * inv_zs[1],
        //                            cols[indices[2]] * inv_zs[2]};
        GColor correctedCols[3] = {cols[indices[0]],
                                   cols[indices[1]],
                                   cols[indices[2]]};
        // vec3 correctedNorms[3] = {norms[0] * inv_zs[0],
        //                           norms[1] * inv_zs[1],
        //                           norms[2] * inv_zs[2]};

        for(int y = minY; y < maxY; ++y) {
            for(int x = minX; x < maxX; ++x) {
                // Naive, TODO: DDA
                p = {x + 0.5f, y + 0.5f};
                e0 = edgeFunction(proj_tri[1], proj_tri[2], p);
                e1 = edgeFunction(proj_tri[2], proj_tri[0], p);
                e2 = edgeFunction(proj_tri[0], proj_tri[1], p);

                if(e0 >= 0 && e1 >= 0 && e2 >= 0) {
                    // Draw pixel using baricentric interp
                    float w0 = e0 * inv_area;
                    float w1 = e1 * inv_area;
                    float w2 = e2 * inv_area;

                    float inv_z = w0 * inv_zs[0]
                                + w1 * inv_zs[1]
                                + w2 * inv_zs[2];

                    if(inv_z > B_invdepth[y][x]) {
                        // Render if closer to camera
                        B_invdepth[y][x] = inv_z;
                        B_depth[y][x] = 1.f / inv_z;


                        B_position[y][x] = triInterp(verts[0], verts[1], verts[2], w0, w1, w2);
                        //B_normal[y][x] = triInterp(correctedNorms[0], correctedNorms[1], correctedNorms[2], w0, w1, w2);
                        B_normal[y][x] = triInterp(norms[0], norms[1], norms[2], w0, w1, w2);
                        //TODO perspective correct norms
                        GColor c = triInterp(correctedCols[0], correctedCols[1], correctedCols[2], w0, w1, w2);
                        B_albedo[y][x] = {c.r, c.g, c.b};

                        //TODO specular rendering
                        B_specular[y][x] = specular;
                    }
                }
            }
        }



        // TODO: adapt scanrow method from MyCanvas
    }

    const PixelData getPixel(int x, int y) {
        return PixelData{
            B_depth[y][x],
            B_position[y][x],
            B_normal[y][x],
            B_albedo[y][x],
            B_specular[y][x]
        };
    }

    const vector<vector<float>> getDepthBuffer() const { return B_depth; }
    const vector<vector<float>> getInvDepthBuffer() const { return B_invdepth; }
    const vector<vector<vec3>> getPositionBuffer() const { return B_position; }
    const vector<vector<vec3>> getAlbedoBuffer() const { return B_albedo; }
    const vector<vector<vec3>> getNormalBuffer() const { return B_normal; }
    const vector<vector<float>> getSpecularBuffer() const { return B_specular; }

    const int width() const { return _dim.width; }
    const int height() const { return _dim.height; }

private:
    GISize _dim;

    // Buffers
    vector<vector<float>> B_depth;
    vector<vector<float>> B_invdepth;

    vector<vector<vec3>> B_position;
    vector<vector<vec3>> B_normal;
    vector<vector<vec3>> B_albedo;
    vector<vector<float>> B_specular;


    float edgeFunction(const vec2 &a, const vec2 &b, const vec2 &c) {
        return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
    }

    template<typename T>
    T triInterp(T v0, T v1, T v2, float w0, float w1, float w2) {
        return w0 * v0 + w1 * v1 + w2 * v2;
    }
};




#endif