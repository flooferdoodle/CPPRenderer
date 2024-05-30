#ifndef matrix_DEFINED
#define matrix_DEFINED

#include "vec.h"
#include <algorithm>

struct idx_pair {
    size_t i {}, j {};
};

class mat3 {
public:
    
    /// @brief Default initializer to identity matrix
    mat3() : _m{1.f, 0.f, 0.f,
                  0.f, 1.f, 0.f,
                  0.f, 0.f, 1.f} {};

    /// @brief Construct matrix from 16 floats
    mat3(const float a00, const float a01, const float a02, 
         const float a10, const float a11, const float a12, 
         const float a20, const float a21, const float a22)
            : _m{a00, a01, a02, 
                   a10, a11, a12,
                   a20, a21, a22} {}

    /// @brief Array initializer
    /// @param data float[16] of matrix entries, in flattened form (stacked row-wise)
    mat3(const float(&data)[9]) : _m() {
        std::copy(data, data + 9, _m);
    }

    /// @brief Get memory access to underlying float[9] data
    /// @param idx 
    /// @return pointer to index
    float* _getMemoryView(const int idx) {
        return _m + idx;
    }

    float operator[](const idx_pair idx) const { // getter
        return _m[idx.i * 3 + idx.j];
    }
    float& operator[](const idx_pair idx){ // setter
        return _m[idx.i * 3 + idx.j];
    }

    /// @brief Row access operator
    /// @param idx 
    /// @return 
    vec3 operator[](const size_t idx) const {
        size_t i = idx * 3;
        return {_m[i], _m[i+1], _m[i+2]};
    }
    /// @brief Column access operator
    /// @param idx 
    /// @return 
    vec3 operator()(const size_t idx) const {
        return {_m[idx], _m[idx+3], _m[idx+6]};
    }
    vec3 getRow(const size_t idx) const { return this->operator[](idx); }
    vec3 getCol(const size_t idx) const { return this->operator()(idx); }

    bool operator==(mat3 m) const {
        bool flag = true;
        for(int i = 0; i < 9; ++i){
            flag = _m[i] == m._m[i];
            if(!flag) return false;
        }
        return true;
    }
    bool operator!=(mat3 m) const { return !(*this == m); }

    mat3& operator=(const mat3 m) {
        std::copy(m._m, m._m + 9, _m);
        return *this;
    }

    #pragma region Arithmetic Operations

    mat3 operator+(mat3 m) const {
        mat3 out;
        for(int i = 0; i < 9; ++i){
            out._m[i] = _m[i] + m._m[i];
        }
        return out;
    }
    mat3 operator-(mat3 m) const {
        mat3 out;
        for(int i = 0; i < 9; ++i){
            out._m[i] = _m[i] - m._m[i];
        }
        return out;
    }
    mat3& operator+=(mat3 m) {
        for(int i = 0; i < 9; ++i){
            _m[i] += m._m[i];
        }
        return *this;
    }
    mat3& operator-=(mat3 m) {
        for(int i = 0; i < 9; ++i){
            _m[i] -= m._m[i];
        }
        return *this;
    }
    mat3 operator-() const {
        mat3 m;
        for(int i = 0; i < 9; ++i){
            m._m[i] = -_m[i];
        }
        return m;
    }

    // Scalar mult
    friend mat3 operator*(mat3 m, float c) {
        mat3 out;
        for(int i = 0; i < 9; ++i){
            out._m[i] = m._m[i] * c;
        }
        return out;
    }
    friend mat3 operator*(float c, mat3 m) {
        return m * c;
    }
    friend mat3 operator/(mat3 m, float c) {
        return m * (1.f / c);
    }
    friend mat3 operator/(float c, mat3 m) {
        return m * (1.f / c);
    }

    // Vector mult
    friend vec3 operator*(mat3 m, vec3 v) {
        return vec3{vec3::dot(m[0], v),
                    vec3::dot(m[1], v),
                    vec3::dot(m[2], v)};
    }

    // Matrix mult
    mat3 operator*(mat3 m) const {
        return mat3(vec3::dot(this->operator[](0), m(0)),
                    vec3::dot(this->operator[](0), m(1)),
                    vec3::dot(this->operator[](0), m(2)),
                    vec3::dot(this->operator[](1), m(0)),
                    vec3::dot(this->operator[](1), m(1)),
                    vec3::dot(this->operator[](1), m(2)),
                    vec3::dot(this->operator[](2), m(0)),
                    vec3::dot(this->operator[](2), m(1)),
                    vec3::dot(this->operator[](2), m(2)));
    }
    mat3& operator*=(mat3 m) {
        float vals[9] =  {vec3::dot(this->operator[](0), m(0)),
                          vec3::dot(this->operator[](0), m(1)),
                          vec3::dot(this->operator[](0), m(2)),
                          vec3::dot(this->operator[](1), m(0)),
                          vec3::dot(this->operator[](1), m(1)),
                          vec3::dot(this->operator[](1), m(2)),
                          vec3::dot(this->operator[](2), m(0)),
                          vec3::dot(this->operator[](2), m(1)),
                          vec3::dot(this->operator[](2), m(2))};
        std::copy(vals, vals + 9, _m);
        return *this;
    }
    #pragma endregion

    mat3 invert() const {
        float a = (_m[4] * _m[8] - _m[7] * _m[5]);
        float b = (_m[3] * _m[8] - _m[5] * _m[6]);
        float c = (_m[3] * _m[7] - _m[4] * _m[6]);
        float det = _m[0] * a -
                    _m[1] * b -
                    _m[2] * c;
        float invdet = 1.f / det;

        return mat3{
            a * invdet,
            (_m[2] * _m[6] - _m[1] * _m[8]) * invdet,
            (_m[1] * _m[5] - _m[2] * _m[4]) * invdet,
            -b * invdet,
            (_m[0] * _m[8] - _m[2] * _m[6]) * invdet,
            (_m[3] * _m[2] - _m[0] * _m[5]) * invdet,
            c * invdet,
            (_m[6] * _m[1] - _m[0] * _m[7]) * invdet,
            (_m[0] * _m[4] - _m[3] * _m[1]) * invdet
        };
    }

    static mat3 invert(mat3 m) {
        return m.invert();
    }

    mat3 transpose() const {
        return {_m[0], _m[3], _m[6],
                _m[1], _m[4], _m[7],
                _m[2], _m[5], _m[8]};
    }
    /*
    0, 1, 2, 
    3, 4, 5,
    6, 7, 8
    
    */

private:
    float _m[9];

};

class mat4 {
public:
    
    /// @brief Default initializer to identity matrix
    mat4() : _m{1.f, 0.f, 0.f, 0.f,
                  0.f, 1.f, 0.f, 0.f,
                  0.f, 0.f, 1.f, 0.f,
                  0.f, 0.f, 0.f, 1.f} {};

    /// @brief Construct matrix from 16 floats
    mat4(const float a00, const float a01, const float a02, const float a03, 
         const float a10, const float a11, const float a12, const float a13, 
         const float a20, const float a21, const float a22, const float a23, 
         const float a30, const float a31, const float a32, const float a33)
            : _m{a00, a01, a02, a03,
                   a10, a11, a12, a13,
                   a20, a21, a22, a23,
                   a30, a31, a32, a33} {}

    /// @brief Array initializer
    /// @param data float[16] of matrix entries, in flattened form (stacked row-wise)
    mat4(const float(&data)[16]) : _m() {
        std::copy(data, data + 16, _m);
    }

    // Convenient initializers
    #pragma region Convenient initializers
    static mat4 Identity() {
        return mat4{1.f, 0.f, 0.f, 0.f,
                    0.f, 1.f, 0.f, 0.f,
                    0.f, 0.f, 1.f, 0.f,
                    0.f, 0.f, 0.f, 1.f};
    }
    static mat4 Scale(const vec3 s) {
        return mat4{s.x(), 0.f, 0.f, 0.f,
                    0.f, s.y(), 0.f, 0.f,
                    0.f, 0.f, s.z(), 0.f,
                    0.f, 0.f, 0.f,   1.f};
    }
    static mat4 Scale(const float x, const float y, const float z) {
        return mat4{  x, 0.f, 0.f, 0.f,
                    0.f,   y, 0.f, 0.f,
                    0.f, 0.f,   z, 0.f,
                    0.f, 0.f, 0.f, 1.f};
    }
    static mat4 Scale(const float c) {
        return mat4{  c, 0.f, 0.f, 0.f,
                    0.f,   c, 0.f, 0.f,
                    0.f, 0.f,   c, 0.f,
                    0.f, 0.f, 0.f, 1.f};
    }
    static mat4 Translate(const vec3 t) {
        return mat4{1.f, 0.f, 0.f, t.x(),
                    0.f, 1.f, 0.f, t.y(),
                    0.f, 0.f, 1.f, t.z(),
                    0.f, 0.f, 0.f, 1.f };
    }
    static mat4 Translate(const float x, const float y, const float z) {
        return mat4{1.f, 0.f, 0.f, x,
                    0.f, 1.f, 0.f, y,
                    0.f, 0.f, 1.f, z,
                    0.f, 0.f, 0.f, 1.f };
    }
    /// @brief Generates a rotation matrix
    /// @param axis unit vector
    /// @param angle in radians
    /// @return rotation matrix
    static mat4 Rotate(const vec3 axis, const float angle) {
        vec3 e = vec3::normalize(axis);
        float c = cosf(angle);
        float s = sinf(angle);
        float c_i = 1.f - c;
        return mat4{c + e.x() * e.x() * c_i,
                    e.x() * e.y() * c_i - e.z() * s,
                    e.x() * e.z() * c_i + e.y() * s, 0.f,
                    
                    e.y() * e.x() * c_i + e.z() * s,
                    c + e.y() * e.y() * c_i,
                    e.y() * e.z() * c_i - e.x() * s, 0.f,
                    
                    e.z() * e.x() * c_i - e.y() * s,
                    e.z() * e.y() * c_i + e.x() * s,
                    c + e.z() * e.z() * c_i, 0.f,
                    
                    0.f, 0.f, 0.f, 1.f};
    }

    static mat4 RotateX(const float angle) {
        float c = cosf(angle);
        float s = sinf(angle);
        return mat4{1.f, 0.f, 0.f, 0.f,
                    0.f,   c,  -s, 0.f,
                    0.f,   s,   c, 0.f,
                    0.f, 0.f, 0.f, 1.f};
    }
    static mat4 RotateY(const float angle) {
        float c = cosf(angle);
        float s = sinf(angle);
        return mat4{  c, 0.f,   s, 0.f,
                    0.f, 1.f, 0.f, 0.f,                   
                     -s, 0.f,   c, 0.f,
                    0.f, 0.f, 0.f, 1.f};
    }
    static mat4 RotateZ(const float angle) {
        float c = cosf(angle);
        float s = sinf(angle);
        return mat4{  c,  -s, 0.f, 0.f,
                      s,   c, 0.f, 0.f,
                    0.f, 0.f, 1.f, 0.f,
                    0.f, 0.f, 0.f, 1.f};
    }

    static mat4 RotateEuler(const vec3 v) {
        return RotateX(v.x()) * RotateY(v.y()) * RotateZ(v.z());
    }
    static mat4 RotateEuler(const float x, const float y, const float z) { return RotateEuler({x, y, z}); }

    #pragma endregion

    /// @brief Get memory access to underlying float[16] data
    /// @param idx 
    /// @return pointer to index
    float* _getMemoryView(const int idx) {
        return _m + idx;
    }

    float operator[](const idx_pair idx) const { // getter
        return _m[idx.i * 4 + idx.j];
    }
    float& operator[](const idx_pair idx){ // setter
        return _m[idx.i * 4 + idx.j];
    }

    /// @brief Row access operator
    /// @param idx 
    /// @return 
    vec4 operator[](const size_t idx) const {
        size_t i = idx * 4;
        return {_m[i], _m[i+1], _m[i+2], _m[i+3]};
    }
    /// @brief Column access operator
    /// @param idx 
    /// @return 
    vec4 operator()(const size_t idx) const {
        return {_m[idx], _m[idx+4], _m[idx+8], _m[idx+12]};
    }
    vec4 getRow(const size_t idx) const { return this->operator[](idx); }
    vec4 getCol(const size_t idx) const { return this->operator()(idx); }

    bool operator==(mat4 m) const {
        bool flag = true;
        for(int i = 0; i < 16; ++i){
            flag = _m[i] == m._m[i];
            if(!flag) return false;
        }
        return true;
    }
    bool operator!=(mat4 m) const { return !(*this == m); }

    mat4& operator=(const mat4 m) {
        std::copy(m._m, m._m + 16, _m);
        return *this;
    }

    #pragma region Arithmetic Operations

    mat4 operator+(mat4 m) const {
        mat4 out;
        for(int i = 0; i < 16; ++i){
            out._m[i] = _m[i] + m._m[i];
        }
        return out;
    }
    mat4 operator-(mat4 m) const {
        mat4 out;
        for(int i = 0; i < 16; ++i){
            out._m[i] = _m[i] - m._m[i];
        }
        return out;
    }
    mat4& operator+=(mat4 m) {
        for(int i = 0; i < 16; ++i){
            _m[i] += m._m[i];
        }
        return *this;
    }
    mat4& operator-=(mat4 m) {
        for(int i = 0; i < 16; ++i){
            _m[i] -= m._m[i];
        }
        return *this;
    }
    mat4 operator-() const {
        mat4 m;
        for(int i = 0; i < 16; ++i){
            m._m[i] = -_m[i];
        }
        return m;
    }

    // Scalar mult
    friend mat4 operator*(mat4 m, float c) {
        mat4 out;
        for(int i = 0; i < 16; ++i){
            out._m[i] = m._m[i] * c;
        }
        return out;
    }
    friend mat4 operator*(float c, mat4 m) {
        return m * c;
    }
    friend mat4 operator/(mat4 m, float c) {
        return m * (1.f / c);
    }
    friend mat4 operator/(float c, mat4 m) {
        return m * (1.f / c);
    }

    // Vector mult
    friend vec4 operator*(mat4 m, vec4 v) {
        return vec4{vec4::dot(m[0], v),
                    vec4::dot(m[1], v),
                    vec4::dot(m[2], v),
                    vec4::dot(m[3], v)};
    }
    friend vec3 operator*(mat4 m, vec3 v) {
        vec4 u = vec4{v.x(), v.y(), v.z(), 1.f};
        u = m * u;
        return vec3{u.x(), u.y(), u.z()} / std::abs(u.w());
    }

    // Matrix mult
    mat4 operator*(mat4 m) const {
        return mat4(vec4::dot(this->operator[](0), m(0)),
                    vec4::dot(this->operator[](0), m(1)),
                    vec4::dot(this->operator[](0), m(2)),
                    vec4::dot(this->operator[](0), m(3)),
                    vec4::dot(this->operator[](1), m(0)),
                    vec4::dot(this->operator[](1), m(1)),
                    vec4::dot(this->operator[](1), m(2)),
                    vec4::dot(this->operator[](1), m(3)),
                    vec4::dot(this->operator[](2), m(0)),
                    vec4::dot(this->operator[](2), m(1)),
                    vec4::dot(this->operator[](2), m(2)),
                    vec4::dot(this->operator[](2), m(3)),
                    vec4::dot(this->operator[](3), m(0)),
                    vec4::dot(this->operator[](3), m(1)),
                    vec4::dot(this->operator[](3), m(2)),
                    vec4::dot(this->operator[](3), m(3)));
    }
    mat4& operator*=(mat4 m) {
        float vals[16] = {vec4::dot(this->operator[](0), m(0)),
                          vec4::dot(this->operator[](0), m(1)),
                          vec4::dot(this->operator[](0), m(2)),
                          vec4::dot(this->operator[](0), m(3)),
                          vec4::dot(this->operator[](1), m(0)),
                          vec4::dot(this->operator[](1), m(1)),
                          vec4::dot(this->operator[](1), m(2)),
                          vec4::dot(this->operator[](1), m(3)),
                          vec4::dot(this->operator[](2), m(0)),
                          vec4::dot(this->operator[](2), m(1)),
                          vec4::dot(this->operator[](2), m(2)),
                          vec4::dot(this->operator[](2), m(3)),
                          vec4::dot(this->operator[](3), m(0)),
                          vec4::dot(this->operator[](3), m(1)),
                          vec4::dot(this->operator[](3), m(2)),
                          vec4::dot(this->operator[](3), m(3))};
        std::copy(vals, vals + 16, _m);
        return *this;
    }
    #pragma endregion

    mat4 invert() const {
        float A2323 = _m[10] * _m[15] - _m[11] * _m[14] ;
        float A1323 = _m[9] * _m[15] - _m[11] * _m[13] ;
        float A1223 = _m[9] * _m[14] - _m[10] * _m[13] ;
        float A0323 = _m[8] * _m[15] - _m[11] * _m[12] ;
        float A0223 = _m[8] * _m[14] - _m[10] * _m[12] ;
        float A0123 = _m[8] * _m[13] - _m[9] * _m[12] ;
        float A2313 = _m[6] * _m[15] - _m[7] * _m[14] ;
        float A1313 = _m[5] * _m[15] - _m[7] * _m[13] ;
        float A1213 = _m[5] * _m[14] - _m[6] * _m[13] ;
        float A2312 = _m[6] * _m[11] - _m[7] * _m[10] ;
        float A1312 = _m[5] * _m[11] - _m[7] * _m[9] ;
        float A1212 = _m[5] * _m[10] - _m[6] * _m[9] ;
        float A0313 = _m[4] * _m[15] - _m[7] * _m[12] ;
        float A0213 = _m[4] * _m[14] - _m[6] * _m[12] ;
        float A0312 = _m[4] * _m[11] - _m[7] * _m[8] ;
        float A0212 = _m[4] * _m[10] - _m[6] * _m[8] ;
        float A0113 = _m[4] * _m[13] - _m[5] * _m[12] ;
        float A0112 = _m[4] * _m[9] - _m[5] * _m[8] ;

        float det = _m[0] * ( _m[5] * A2323 - _m[6] * A1323 + _m[7] * A1223 ) 
            - _m[1] * ( _m[4] * A2323 - _m[6] * A0323 + _m[7] * A0223 ) 
            + _m[2] * ( _m[4] * A1323 - _m[5] * A0323 + _m[7] * A0123 ) 
            - _m[3] * ( _m[4] * A1223 - _m[5] * A0223 + _m[6] * A0123 ) ;
        det = 1.f / det;

        return mat4{
            det *   ( _m[5] * A2323 - _m[6] * A1323 + _m[7] * A1223 ),
            det * - ( _m[1] * A2323 - _m[2] * A1323 + _m[3] * A1223 ),
            det *   ( _m[1] * A2313 - _m[2] * A1313 + _m[3] * A1213 ),
            det * - ( _m[1] * A2312 - _m[2] * A1312 + _m[3] * A1212 ),
            det * - ( _m[4] * A2323 - _m[6] * A0323 + _m[7] * A0223 ),
            det *   ( _m[0] * A2323 - _m[2] * A0323 + _m[3] * A0223 ),
            det * - ( _m[0] * A2313 - _m[2] * A0313 + _m[3] * A0213 ),
            det *   ( _m[0] * A2312 - _m[2] * A0312 + _m[3] * A0212 ),
            det *   ( _m[4] * A1323 - _m[5] * A0323 + _m[7] * A0123 ),
            det * - ( _m[0] * A1323 - _m[1] * A0323 + _m[3] * A0123 ),
            det *   ( _m[0] * A1313 - _m[1] * A0313 + _m[3] * A0113 ),
            det * - ( _m[0] * A1312 - _m[1] * A0312 + _m[3] * A0112 ),
            det * - ( _m[4] * A1223 - _m[5] * A0223 + _m[6] * A0123 ),
            det *   ( _m[0] * A1223 - _m[1] * A0223 + _m[2] * A0123 ),
            det * - ( _m[0] * A1213 - _m[1] * A0213 + _m[2] * A0113 ),
            det *   ( _m[0] * A1212 - _m[1] * A0212 + _m[2] * A0112 ),
        };
    }

    static mat4 invert(mat4 m) {
        return m.invert();
    }

    mat4 transpose() const {
        return {_m[0], _m[4], _m[8], _m[12],
                _m[1], _m[5], _m[9], _m[13],
                _m[2], _m[6], _m[10], _m[14],
                _m[3], _m[7], _m[11], _m[15]};
    }
    /*
    0, 1, 2, 3
    4, 5, 6, 7
    8, 9, 10, 11,
    12, 13, 14, 15
    
    */

    static mat3 upperLeft(mat4 m) {
        return mat3{
            m._m[0], m._m[1], m._m[2],
            m._m[4], m._m[5], m._m[6],
            m._m[8], m._m[9], m._m[10]
        };
    }

private:
    float _m[16];

};



#endif