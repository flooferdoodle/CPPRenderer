#include "Object.h"
#include <unordered_map>
#include "include/CustomException.h"

void Object::addTri(const vec3(&v)[3], const vec4* cols, const vec2* uv) {
    int i = vertices.size();
    vertices.push_back(v[0]); vertices.push_back(v[1]); vertices.push_back(v[2]);
    indices.push_back(i); indices.push_back(i+1); indices.push_back(i+2);


    // Initialize colors if none yet, fill with default color (white)
    if(cols && !colors.has_value()) colors = std::vector<GColor>(i, {1.f, 1.f, 1.f, 1.f});
    
    // If colors exist but none passed, add default white
    if(!cols) {
        colors.value().push_back({1.f, 1.f, 1.f, 1.f});
        colors.value().push_back({1.f, 1.f, 1.f, 1.f});
        colors.value().push_back({1.f, 1.f, 1.f, 1.f});
    }
    else{
        colors.value().push_back(cols[0]);
        colors.value().push_back(cols[1]);
        colors.value().push_back(cols[2]);
    }

    // Initialize uvs if none yet
    if(uv && !uvs.has_value()) uvs = std::vector<GPoint>(i, {0.f, 0.f});
    
    // If uvs exist but none passed, add default uvs
    if(!uv) {
        uvs.value().push_back({0.f, 0.f});
        uvs.value().push_back({1.f, 0.f});
        uvs.value().push_back({1.f, 1.f});
    }
    else {
        uvs.value().push_back(uv[0]);
        uvs.value().push_back(uv[1]);
        uvs.value().push_back(uv[2]);
    }
}

#pragma region Primitives template data
const Object::PrimitiveTemplate Object::_Cube = {
    {
        {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f},

        /*
               0--------1
              /|       /|
             / |      / |
            3--------2  |
            |  4-----|--5
            | /      | /
            |/       |/
            7--------6
        
        */
    },

    {},

    {0,1,3, 1,2,3, // top
     3,2,7, 2,6,7, // front
     2,1,6, 1,5,6, // right
     1,0,4, 1,4,5, // back
     0,3,4, 3,7,4, // left
     7,6,4, 6,5,4  // bottom
    }
};


#define ICO_X .525731112119133606f
#define ICO_Z .850650808352039932f
#define ICO_N 0.f
const Object::PrimitiveTemplate Object::_Icosahedron = {
    {   // 0                       1                    2                    3
        {-ICO_X,ICO_N,ICO_Z}, {ICO_X,ICO_N,ICO_Z}, {-ICO_X,ICO_N,-ICO_Z}, {ICO_X,ICO_N,-ICO_Z},
        // 4                        5                   6                   7
        {ICO_N,ICO_Z,ICO_X}, {ICO_N,ICO_Z,-ICO_X}, {ICO_N,-ICO_Z,ICO_X}, {ICO_N,-ICO_Z,-ICO_X},
        // 8                        9                       10                  11
        {ICO_Z,ICO_X,ICO_N}, {-ICO_Z,ICO_X, ICO_N}, {ICO_Z,-ICO_X,ICO_N}, {-ICO_Z,-ICO_X, ICO_N}
    },

    {},

    {
        0,4,1,  0,9,4,  9,5,4,  4,5,8,  4,8,1, 
        8,10,1, 8,3,10, 5,3,8,  5,2,3,  2,7,3, 
        7,10,3, 7,6,10, 7,11,6, 11,0,6, 0,1,6, 
        6,1,10, 9,0,11, 9,11,2, 9,2,5,  7,2,11
    }
};

const Object::PrimitiveTemplate Object::_Plane = {
    {
        {-0.5f, 0.f, -0.5f},
        { 0.5f, 0.f, -0.5f}, 
        { 0.5f, 0.f,  0.5f}, 
        {-0.5f, 0.f,  0.5f}
    },
    
    {},

    {
        0, 1, 3,    1, 2, 3,
        0, 3, 1,    1, 3, 2
    }
};

#pragma endregion

class Edge {
public:
    int a, b;

    Edge(int _a, int _b) : a(_a), b(_b) {
        if(b < a) std::swap(a, b);
    }

    bool operator==(const Edge &other) const {
        return a == other.a && b == other.b;
    }
};
template<>
struct std::hash<Edge>
{
    std::size_t operator()(const Edge& e) const {
        return e.a << 16 + e.b;
    }
};

const Object Object::Icosphere(vec3 pos, vec3 scale, vec3 euler,
                               GColor col, float shininess, int subdivisions) {
    Object obj = _Icosahedron.GetPrimitive(pos, scale, euler, col, shininess);
    obj.smooth = true;

    // Add normals
    for(int i = 0; i < obj.vertexCount(); ++i) {
        obj.normals.push_back(obj.vertices[i]);
    }
    
    if(subdivisions < 1) return obj;

    if(subdivisions > 5) throw CustomException("Icosphere resolution exceeds hashing limit.");

    std::unordered_map<Edge, int> edge_mid{};
    Edge e{0, 0};
    int mids[3];

    for(int i = 0; i < subdivisions; ++i) {
        std::vector<int> new_tris{};
        new_tris.reserve(obj.indexCount() * 4);
        for(int n = 0; n < obj.indexCount(); n += 3) {
            for(int i = 0; i < 3; ++i) {
                e.a = obj.indices[n + i];
                e.b = obj.indices[n + (i + 1)%3];

                if(edge_mid.find(e) == edge_mid.end()){
                    edge_mid.insert({e, obj.vertexCount()});
                    vec3 newPoint = vec3::normalize(obj.vertices[e.a] + obj.vertices[e.b]);
                    obj.vertices.push_back(newPoint);
                    obj.normals.push_back(newPoint);
                }
                mids[i] = edge_mid[e];
            }

            new_tris.insert(new_tris.end(), {
                obj.indices[n    ], mids[0], mids[2],
                obj.indices[n + 1], mids[1], mids[0],
                obj.indices[n + 2], mids[2], mids[1],
                mids[0], mids[1], mids[2]
            });
        }
        obj.indices = std::move(new_tris);
        edge_mid.clear();
    }

    obj.colors = std::vector<GColor>(obj.vertexCount(), col);

    return obj;
}
