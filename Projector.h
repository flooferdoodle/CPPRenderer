#ifndef Projector_DEFINED
#define Projector_DEFINED

#include "include/matrix.h"
#include "include/vec.h"
#include "Camera.h"
#include "Object.h"
#include <vector>
#include "MyCanvas.h"
#include "src/shaders/TriGradientShader.h"
#include "src/shaders/FlatShader.h"
#include "include/GPaint.h"
#include "SceneBuilder.h"
#include "GBuffer.h"

struct RenderStatistic {
    int numObjects = 0;
    int numTrisTotal = 0;
    int numTrisDrawn = 0;
    int numLights = 0;

    double ticksTaken = 0.0;
    double secondsTaken = 0.0;
};

class Projector {
public:

    Projector(GCanvas* canv, GISize dim, GBitmap* bmap)
        : _dim(dim), _buffer(GBuffer(dim)), _rendered(false), canvas(canv), bitmap(bmap) {}

    RenderStatistic RenderSceneTo(Scene scene, GCanvas& canvas, GISize dim) {
        _rendered = true;
        clock_t now = std::clock();

        _buffer = GBuffer(dim);
        RenderStatistic stats{};

        mat4 project_view = scene.cam.getProjectionMatrix() * scene.cam.getViewMatrix();
        for(const Object& obj : scene.objects) {
            ++stats.numObjects;

            mat4 obj_transform = obj.getTransform();
            mat4 obj_project = project_view * obj_transform;

            //mat3 normal_transform = mat4::upperLeft(obj_transform).invert().transpose();
            mat4 normal_transform = obj_transform.invert().transpose();
            normal_transform[12] = 0.f; normal_transform[13] = 0.f; normal_transform[14] = 0.f; 
            vec2 canv_dim = {(float) dim.width, (float) dim.height};

            // Project all vertices to 2D
            std::vector<vec2> proj_verts;
            proj_verts.reserve(obj.vertexCount());

            for(int i = 0; i < obj.vertexCount(); ++i) {
                vec3 proj = obj_project * obj.vertices[i];

                // recenter and rescale
                vec2 proj_centered = {proj.x() + 0.5f, proj.y() + 0.5f};
                proj_centered *= canv_dim;

                proj_verts.push_back(proj_centered);
            }

            // Backface Culling
            // CCW indicates we are looking at backside of tri, so we do "backface culling"
            std::vector<int> indices;
            std::vector<vec3> norms;
            std::vector<vec3> vertices;
            indices.reserve(obj.indexCount());
            norms.reserve(obj.indexCount());
            vertices.reserve(obj.indexCount());
            int count = 0;

            mat4 localToCam = scene.cam.getViewMatrix() * obj_transform;
            
            if(obj.smooth){
                for(int tri = 0; tri < obj.indexCount(); tri += 3) {
                    int a = obj.indices[tri];
                    int b = obj.indices[tri + 1];
                    int c = obj.indices[tri + 2];
                    vec2 ab = proj_verts[b] - proj_verts[a];
                    vec2 ac = proj_verts[c] - proj_verts[a];
                    float wind = ab.x() * ac.y() - ac.x() * ab.y();

                    if(signbit(wind)) { // If CW, keep tri
                        indices.push_back(a);
                        indices.push_back(b);
                        indices.push_back(c);

                        // TODO: perform this step during projection so that norms aren't double calculated
                        norms.push_back(vec3::normalize(normal_transform * obj.normals[a]));
                        norms.push_back(vec3::normalize(normal_transform * obj.normals[b]));
                        norms.push_back(vec3::normalize(normal_transform * obj.normals[c]));

                        vertices.push_back(localToCam * obj.vertices[a]);
                        vertices.push_back(localToCam * obj.vertices[b]);
                        vertices.push_back(localToCam * obj.vertices[c]);

                        ++count;
                    }

                    ++stats.numTrisTotal;
                }
            }
            else { // flat shading, calculate face norms
                for(int tri = 0; tri < obj.indexCount(); tri += 3) {
                    int a = obj.indices[tri];
                    int b = obj.indices[tri + 1];
                    int c = obj.indices[tri + 2];
                    vec2 ab = proj_verts[b] - proj_verts[a];
                    vec2 ac = proj_verts[c] - proj_verts[a];
                    float wind = ab.x() * ac.y() - ac.x() * ab.y();

                    if(signbit(wind)) { // If CW, keep tri
                        indices.push_back(a);
                        indices.push_back(b);
                        indices.push_back(c);
                        
                        vec3 norm = normal_transform * vec3::cross(obj.vertices[c] - obj.vertices[a], obj.vertices[b] - obj.vertices[a]);
                        norm.normalize();
                        norms.insert(norms.end(), {norm, norm, norm});

                        vertices.push_back(localToCam * obj.vertices[a]);
                        vertices.push_back(localToCam * obj.vertices[b]);
                        vertices.push_back(localToCam * obj.vertices[c]);
                        
                        ++count;
                    }

                    ++stats.numTrisTotal;
                }
            }

            // NOTE: the normals and camera vertices are calculated alongside indices, i.e. a single vertex has format
            // proj_verts[indices[n]] <-> norms[n] <-> vertices[n]

            // TODO: Render to GBuffer
            /*
                For current object's projected vertices, perform a simplified drawConvexPolygon optimized for tris
                At each given pixel, render to buffer only if current pos z-value is greater
            */
           
            // Rasterize triangle, and then scanrow its pixels, using above principle
            int n = 0;
            for(int i = 0; i < count; ++i) {
                _buffer.drawTri(&indices[n], proj_verts, &vertices[n], &norms[n], obj.colors.value(), obj.shininess);
                n += 3;
            }

            /*
            Step 1: loop through each triangle
            Step 2: Create an instance of shader to handle colors or tex or both
            Step 3: pass to drawConvexPolygon
            */
            /*
            GPoint tri[3] = {{0.f,0.f},{0.f,0.f},{0.f,0.f}};
            GPaint paint = GPaint();
            n = 0;
            DirectionalFlatShader shader({0.5f, 2.f, -2});
            const GColor* colArr = obj.getColorArr();

            paint.setShader(&shader);
            for(int i = 0; i < count; ++i) {
                tri[0] = proj_verts[indices[n]]; tri[1] = proj_verts[indices[n+1]]; tri[2] = proj_verts[indices[n+2]];

                shader.calcColor(norms[n], colArr[indices[n]]);
                
                canvas.drawConvexPolygon(tri, 3, paint);

                n += 3;
            }*/

            stats.numTrisDrawn += count;
        }

        // TODO: deferred rendering to screen
        stats.numLights = scene.lights.size();
        vec3 camPos = scene.cam.getPos();
        for(int y = 0; y < _dim.height; ++y) {
            for(int x = 0; x < _dim.width; ++x) {
                
                vec3 col{0.f, 0.f, 0.f};
                PixelData data = _buffer.getPixel(x, y);

                if(data.specular < 0.f) { // Rendering emitters
                    *(bitmap->getAddr(x, y)) = toPremul(data.albedo);
                    continue;
                }

                for(Light l : scene.lights) {
                    col += l.calcLight(data.position, camPos, data.normal, data.albedo, data.specular);
                }

                col[0] = std::clamp(col[0], 0.f, 1.f);
                col[1] = std::clamp(col[1], 0.f, 1.f);
                col[2] = std::clamp(col[2], 0.f, 1.f);

                *(bitmap->getAddr(x, y)) = toPremul(col);
            }
        }
        

        stats.ticksTaken = double(clock() - now);
        stats.secondsTaken = stats.ticksTaken / double(CLOCKS_PER_SEC);

        return stats;
    }

    enum BufferType {Depth, Inv_Depth, Position, Albedo, Normal, Specular};

    void ShowBuffer(BufferType type, GBitmap &bitmap, const Scene &scene) {
        if(!_rendered) throw CustomException("Nothing has been rendered yet.");
        if(bitmap.width() != _buffer.width() || bitmap.height() != _buffer.height())
            throw CustomException("Buffer and bitmap dimensions don't match."); 

        switch(type) {
            default:
            case BufferType::Depth:
                ShowDepthBuffer(bitmap, scene);
                break;
            case BufferType::Inv_Depth:
                ShowInvDepthBuffer(bitmap, scene);
                break;
            case BufferType::Normal:
                ShowNormalBuffer(bitmap);
                break;
            case BufferType::Albedo:
                ShowAlbedoBuffer(bitmap);
                break;
            case BufferType::Specular:
                ShowSpecularBuffer(bitmap);
                break;
            case BufferType::Position:
                ShowPositionBuffer(bitmap);
                break;
        }
    }

    const std::vector<std::vector<float>> getDepthBuffer() { if(!_rendered) throw CustomException("Nothing rendered."); return _buffer.getDepthBuffer(); }
    const std::vector<std::vector<float>> getInvDepthBuffer() { if(!_rendered) throw CustomException("Nothing rendered."); return _buffer.getInvDepthBuffer(); }
    const std::vector<std::vector<vec3>> getPositionBuffer() { if(!_rendered) throw CustomException("Nothing rendered."); return _buffer.getPositionBuffer(); }
    const std::vector<std::vector<vec3>> getAlbedoBuffer() { if(!_rendered) throw CustomException("Nothing rendered."); return _buffer.getAlbedoBuffer(); }
    const std::vector<std::vector<vec3>> getNormalBuffer() { if(!_rendered) throw CustomException("Nothing rendered."); return _buffer.getNormalBuffer(); }

private:
    GISize _dim;
    GBuffer _buffer;
    bool _rendered;

    GCanvas* canvas;
    GBitmap* bitmap;

    void ShowDepthBuffer(GBitmap &bitmap, const Scene &scene) {
        const std::vector<std::vector<float>> &buffer = _buffer.getDepthBuffer();
        float val;

        for(int y = 0; y < bitmap.height(); ++y) {
            for(int x = 0; x < bitmap.width(); ++x) {
                // Scale depth to near and far clipping
                val = 1.f - std::clamp(buffer[y][x], scene.cam.near(), scene.cam.far()) / (scene.cam.far() - scene.cam.near());
                // non-linear scale for better visualizing
                val = val * val;

                *(bitmap.getAddr(x, y)) = toPremul({val, val, val, 1.f});
            }
        }
    }

    void ShowInvDepthBuffer(GBitmap &bitmap, const Scene &scene) {
        const std::vector<std::vector<float>> &buffer = _buffer.getInvDepthBuffer();
        float val;
        float max = 1.f / scene.cam.near();
        float min = 1.f / scene.cam.far();

        for(int y = 0; y < bitmap.height(); ++y) {
            for(int x = 0; x < bitmap.width(); ++x) {
                // Scale depth to near and far clipping
                val = std::clamp(buffer[y][x], min, max) / (max - min);
                
                *(bitmap.getAddr(x, y)) = toPremul({val, val, val, 1.f});
            }
        }
    }

    void ShowPositionBuffer(GBitmap &bitmap) {
        const std::vector<std::vector<vec3>> &buffer = _buffer.getPositionBuffer();
        vec3 val;

        for(int y = 0; y < bitmap.height(); ++y) {
            for(int x = 0; x < bitmap.width(); ++x) {
                val = buffer[y][x];
                val[1] *= -1.f;
                
                //val[0] = abs(val[0]); val[1] = abs(val[1]); val[2] = abs(val[2]);
                val[0] = clamp(val[0], 0.f, 1.f); val[1] = clamp(val[1], 0.f, 1.f); val[2] = clamp(val[2], 0.f, 1.f);
                
                *(bitmap.getAddr(x, y)) = toPremul({val.x(), val.y(), val.z(), 1.f});
            }
        }
    }

    void ShowNormalBuffer(GBitmap &bitmap) {
        const std::vector<std::vector<vec3>> &buffer = _buffer.getNormalBuffer();
        vec3 val;

        for(int y = 0; y < bitmap.height(); ++y) {
            for(int x = 0; x < bitmap.width(); ++x) {
                val = buffer[y][x];
                
                val[0] = abs(val[0]); val[1] = abs(val[1]); val[2] = abs(val[2]);
                val[0] = clamp(val[0], 0.f, 1.f); val[1] = clamp(val[1], 0.f, 1.f); val[2] = clamp(val[2], 0.f, 1.f);
                
                *(bitmap.getAddr(x, y)) = toPremul({val.x(), val.y(), val.z(), 1.f});
            }
        }
    }

    void ShowAlbedoBuffer(GBitmap &bitmap) {
        const std::vector<std::vector<vec3>> &buffer = _buffer.getAlbedoBuffer();
        vec3 val;

        for(int y = 0; y < bitmap.height(); ++y) {
            for(int x = 0; x < bitmap.width(); ++x) {
                val = buffer[y][x];
                
                *(bitmap.getAddr(x, y)) = toPremul({val.x(), val.y(), val.z(), 1.f});
            }
        }
    }

    void ShowSpecularBuffer(GBitmap &bitmap) {
        const std::vector<std::vector<float>> &buffer = _buffer.getSpecularBuffer();
        float val;

        for(int y = 0; y < bitmap.height(); ++y) {
            for(int x = 0; x < bitmap.width(); ++x) {
                val = buffer[y][x] / 256.f;
                val = clamp(val, 0.f, 1.f);
                
                *(bitmap.getAddr(x, y)) = toPremul({val, val, val, 1.f});
            }
        }
    }

};

#endif