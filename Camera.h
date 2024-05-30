#ifndef Camera_DEFINED
#define Camera_DEFINED

#include "./include/vec.h"
#include "./include/matrix.h"

#define DEG2RAD 0.01745329f

class Camera{

public:
    Camera(const vec3 pos = vec3({0.f, 0.f, 3.f}),
           const vec3 target = vec3({0.f, 0.f, 0.f}),
           const vec3 up = vec3({0.f, 1.f, 0.f}),
           const float fov = 90.f,
           const float aspectRatio = 1.f,
           const float near = 0.1f,
           const float far = 100.f) : position(pos), fov_rad(fov), nearClip(near), farClip(far) {
        orient_mat = mat4();
        inv_pos = mat4::Translate(-pos);

        // construct vectors from matrix memory
        // i.e. modifying vectors modifies matrix, and vice versa
        p_right = vec3::fromArrMem(orient_mat._getMemoryView(0), p_right);
        p_up    = vec3::fromArrMem(orient_mat._getMemoryView(4), p_up);
        p_dir   = vec3::fromArrMem(orient_mat._getMemoryView(8), p_dir);

        // Calculate camera orientation
        p_dir.setValsTo(vec3::normalize(pos - target));
        p_right.setValsTo(vec3::normalize(vec3::cross(up, p_dir)));
        p_up.setValsTo(vec3::cross(p_dir, p_right));

        // Calculate projection matrix
        float right = tanf(fov * 0.5f * DEG2RAD) * near;
        float top = right / aspectRatio;
        projection = mat4{
            near / right, 0.f, 0.f, 0.f,
            0.f, near / top,   0.f, 0.f,
            0.f, 0.f, -(far + near) / (far - near), -1.f,
            0.f, 0.f, -2.f * far * near / (far - near), 0.f
        };
    }

    vec3 forward() const { return -p_dir; }
    vec3 backward() const { return p_dir; }
    vec3 up() const { return p_up; }
    vec3 down() const { return -p_up; }
    vec3 right() const { return p_right; }
    vec3 left() const { return -p_right; }
    const float near() const { return nearClip; }
    const float far() const { return farClip; }

    // void applyTransform(mat4 m) {};
    void Translate(float x, float y, float z) {
        position += {x, y, z};

        inv_pos[{0, 3}] -= x;
        inv_pos[{1, 3}] -= y;
        inv_pos[{2, 3}] -= z;
    };
    void Translate(vec3 v) { Translate(v.x(), v.y(), v.z()); };
    void Rotate(vec3 axis, float angle) {
        orient_mat *= mat4::Rotate(axis, angle);
    };

    mat4 getViewMatrix() const {
        return orient_mat * inv_pos;
    }
    mat4 getProjectionMatrix() const { return projection; }

    const vec3 getPos() const { return position; }

private:
    vec3 position;

    bool orthographic = false;
    float fov_rad; // FOV in radians
    float nearClip;
    float farClip;

    mat4 projection;

    mat4 orient_mat;
    mat4 inv_pos;

    // "proxy" vectors, share memory with orient_mat
    vec3 p_dir; // faces OPPOSITE of target, i.e. points out of camera's ass
    vec3 p_up;
    vec3 p_right;

};


#endif