#ifndef Object3D_DEFINED
#define Object3D_DEFINED

#include <vector>
#include <optional>
#include "include/vec.h"
#include "include/matrix.h"
#include "include/GColor.h"
#include "include/GPoint.h"

struct Object {
    vec3 pos{};
    vec3 euler{};
    vec3 scale{1.f, 1.f, 1.f};

    std::vector<vec3> vertices;
    std::vector<int> indices;
    std::vector<vec3> normals;
    
    // optional parameters
    std::optional<std::vector<GColor>> colors;
    std::optional<std::vector<GPoint>> uvs;

    // Material properties
    float shininess; // If shininess < 0, object is emitter
    const bool isEmitter() const { return shininess < 0.f; }

    // Rendering options
    bool smooth;

    Object() : vertices(), indices() {}
    Object(const vec3& _pos, const vec3& _scale, const vec3& _euler, const GColor& _col,
           const std::vector<vec3> &_vertices,
           const std::vector<int> &_indices,
           const std::vector<vec3> &_norms,
           float specular,
           bool _smooth) :
           pos(_pos), euler(_euler), scale(_scale),
           vertices(_vertices), indices(_indices), normals(_norms), 
           colors{std::vector<GColor>(_vertices.size(), _col)},
           shininess(specular), smooth(_smooth) {};

    int triCount() const { return indices.size() / 3; }
    int indexCount() const { return indices.size(); }
    int vertexCount() const { return vertices.size(); }

    const mat4 getTransform() const {
        return mat4::Translate(pos) * mat4::RotateEuler(euler) * mat4::Scale(scale);
    };

    const GColor* getColorArr() const {
        if(!colors.has_value()) return nullptr;
        return colors.value().data();
    }
    const GPoint* getUVsArr() const {
        if(!uvs.has_value()) return nullptr;
        return uvs.value().data();
    }

    bool verifyData() const {
        if(indices.size() % 3 != 0) return false;
        if(colors.has_value() && colors.value().size() != vertices.size()) return false;
        if(uvs.has_value() && uvs.value().size() != vertices.size()) return false;

        return true;
    }

    void addTri(const vec3(&v)[3], const vec4* cols = nullptr, const vec2* uv = nullptr);

    void addTriFan() {};

    // Primitives
    
    /// @brief Return a cube Object
    /// @param pos float or vec3
    /// @param scale float or vec3
    /// @param euler float or vec3
    /// @param col GColor
    /// @return Object with cube data
    const static Object Cube(vec3 pos    = {0.f, 0.f, 0.f},
                             vec3 scale  = {1.f, 1.f, 1.f},
                             vec3 euler  = {0.f, 0.f, 0.f},
                             GColor col  = {1.f, 1.f, 1.f, 1.f},
                             float shininess = 64.f) {
        return _Cube.GetPrimitive(pos, scale, euler, col, shininess);
    }

    /// @brief Return an icosahedron Object
    /// @param pos float or vec3
    /// @param scale float or vec3
    /// @param euler float or vec3
    /// @param col GColor
    /// @return Object with cube data
    const static Object Icosahedron(vec3 pos    = {0.f, 0.f, 0.f},
                                    vec3 scale  = {1.f, 1.f, 1.f},
                                    vec3 euler  = {0.f, 0.f, 0.f},
                                    GColor col  = {1.f, 1.f, 1.f, 1.f},
                                    float shininess = 64.f) {
        return _Icosahedron.GetPrimitive(pos, scale, euler, col, shininess);
    }

    /// @brief Return an icosphere Object
    /// @param pos float or vec3
    /// @param scale float or vec3
    /// @param euler float or vec3
    /// @param col GColor
    /// @param subdivisions number of times to subdivide tris
    /// @return 
    const static Object Icosphere(vec3 pos    = {0.f, 0.f, 0.f},
                                  vec3 scale  = {1.f, 1.f, 1.f},
                                  vec3 euler  = {0.f, 0.f, 0.f},
                                  GColor col  = {1.f, 1.f, 1.f, 1.f},
                                  float shininess = 64.f,
                                  int subdivisions = 1);

    /// @brief Return a plane Object
    /// @param pos float or vec3
    /// @param scale float or vec3
    /// @param euler float or vec3
    /// @param col GColor
    /// @return Object with plane data
    const static Object Plane(vec3 pos    = {0.f, 0.f, 0.f},
                              vec3 scale  = {1.f, 1.f, 1.f},
                              vec3 euler  = {0.f, 0.f, 0.f},
                              GColor col  = {1.f, 1.f, 1.f, 1.f},
                              float shininess = 64.f) {
        return _Plane.GetPrimitive(pos, scale, euler, col, shininess);
    }


private:
    /// @brief Template data for primitives. By default, primitives are shaded flat unless they contain normal info.
    struct PrimitiveTemplate {
        std::vector<vec3> vertices;
        std::vector<vec3> normals;
        std::vector<int> tris;

        /// @brief Create an object from primitive template. If primitive has normals, will use smooth shading. Otherwise, uses flat shading.
        /// @return 
        Object GetPrimitive(vec3 pos    = {0.f, 0.f, 0.f},
                            vec3 scale  = {1.f, 1.f, 1.f},
                            vec3 euler  = {0.f, 0.f, 0.f},
                            GColor col  = {1.f, 1.f, 1.f, 1.f},
                            float specular = 64) const {
            return Object(pos, scale, euler, col, vertices, tris, normals, specular, !normals.empty());
        }
    };
    
    const static PrimitiveTemplate _Cube;
    const static PrimitiveTemplate _Plane;

    const static PrimitiveTemplate _Icosahedron;
};


#endif