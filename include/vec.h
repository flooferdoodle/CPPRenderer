#ifndef vec_DEFINED
#define vec_DEFINED

#include <math.h>
#include <type_traits>
#include <algorithm>
#include <initializer_list>
#include <cstring>
#include "GColor.h"
#include "GPoint.h"
#include <array>

template<size_t D>
class Vector {
public:
    /// @brief Default constructor, initializes to 0
    Vector() : vals(new float[D]{}) {}

    /// @brief Constant constructor
    /// @param num fills vector with this value
    Vector(const float num) : vals(new float[D]{}) {
        for(int i = 0; i < D; ++i){
            vals[i] = num;
        }
    }

    /// @brief Initialize from array
    /// @param data
    Vector(const float(&data)[D]) : vals(new float[D]{}){
        std::copy(data, data + D, vals);
    }
    Vector(const std::array<float, D> data) : vals(new float[D]{}) {
        std::copy(data.begin(), data.end(), vals);
    }

    /// @brief Curly brace constructor. Has NO size checks.
    /// @param data should be NO BIGGER than size of vector
    Vector(std::initializer_list<float> data) : vals(new float[D]{}){
        int i = 0;
        for(float val : data){
            vals[i++] = val;
        }
        for(; i < D; ++i){
            vals[i] = 0.f;
        }
    }
    
    
    static Vector& fromArrMem(float data[D], Vector<D> &result) {
        result.vals = data;
        return result;
    }
    

    /// @brief Copy constructor
    /// @param v 
    Vector(const Vector<D>& v) : vals(new float[D]{}) {
        std::copy(v.vals, v.vals + D, vals);
    }

    float x() const { return vals[0]; }
    float y() const { return vals[1]; }
    float z() const { typename std::enable_if<(D > 2)>::type(); return vals[2]; }
    float w() const { typename std::enable_if<(D > 3)>::type(); return vals[3]; }
    float operator[](size_t i) const { return vals[i]; }
    float& operator[](size_t i) { return vals[i]; }

    float lengthsq() const {
        float sum = 0.f;
        for(int i = 0; i < D; ++i){
            sum += vals[i] * vals[i];
        }
        return sum;
    }
    float length() const { return sqrtf(lengthsq()); }
    static Vector normalize(const Vector& v){
        return (v / v.length());
    }
    Vector& normalize() {
        float s = 1.f / length();
        for(int i = 0; i < D; ++i){
            vals[i] *= s;
        }
        return *this;
    }


    // Equivalence
    bool operator==(Vector<D> v) const {
        bool same = true;
        for(int i = 0; i < D; ++i){
            same = vals[i] == v.vals[i];
            if(!same) return false;
        }
        return true;
    }
    bool operator!=(Vector<D> v) const {return !(*this == v); }

    // Assignment
    // TODO: this does NOT work
    Vector<D>& operator=(const Vector<D> &a) {
        std::copy(a.vals, a.vals + D, vals);
        return *this;
    }
    void setValsTo(const Vector<D> &a) {
        std::copy(a.vals, a.vals + D, vals);
    }
    
    // Arithmetic
    Vector<D> operator+(Vector<D> v) const {
        Vector<D> out;
        for(int i = 0; i < D; ++i){
            out[i] = vals[i] + v.vals[i];
        }
        return out;
    }
    Vector<D>& operator+=(Vector<D> v) {
        for(int i = 0; i < D; ++i){
            vals[i] += v.vals[i];
        }
        return *this;
    }

    Vector<D> operator-(Vector<D> v) const {
        Vector<D> out;
        for(int i = 0; i < D; ++i){
            out[i] = vals[i] - v.vals[i];
        }
        return out;
    }
    Vector<D>& operator-=(Vector<D> v) {
        for(int i = 0; i < D; ++i){
            vals[i] -= v.vals[i];
        }
        return *this;
    }
    Vector<D> operator-() const {
        Vector<D> out;
        for(int i = 0; i < D; ++i){
            out.vals[i] = -vals[i];
        }
        return out;
    }

    Vector<D> operator*(Vector<D> v) const {
        Vector<D> out;
        for(int i = 0; i < D; ++i){
            out.vals[i] = vals[i] * v.vals[i];
        }
        return out;
    }
    Vector<D>& operator*=(Vector<D> v) {
        for(int i = 0; i < D; ++i){
            vals[i] *= v.vals[i];
        }
        return *this;
    }
    friend Vector<D> operator*(Vector<D> v, float c) {
        Vector<D> out;
        for(int i = 0; i < D; ++i){
            out.vals[i] = v.vals[i] * c;
        }
        return out;
    }
    friend Vector<D> operator*(float c, Vector<D> v) {
        Vector<D> out;
        for(int i = 0; i < D; ++i){
            out.vals[i] = v.vals[i] * c;
        }
        return out;
    }

    Vector<D>& operator*=(float c) {
        for(int i = 0; i < D; ++i){
            vals[i] *= c;
        }
        return *this;
    }

    Vector<D> operator/(Vector<D> v) const {
        Vector<D> out;
        for(int i = 0; i < D; ++i){
            out.vals[i] = vals[i] / v.vals[i];
        }
        return out;
    }
    Vector<D>& operator/=(Vector<D> v) {
        for(int i = 0; i < D; ++i){
            vals[i] /= v.vals[i];
        }
        return *this;
    }
    friend Vector<D> operator/(Vector<D> v, float c) {
        c = 1.f / c;
        Vector<D> out;
        for(int i = 0; i < D; ++i){
            out.vals[i] = v.vals[i] * c;
        }
        return out;
    }
    friend Vector<D> operator/(float c, Vector<D> v) {
        c = 1.f / c;
        Vector<D> out;
        for(int i = 0; i < D; ++i){
            out.vals[i] = v.vals[i] * c;
        }
        return out;
    }
    Vector<D>& operator/=(float c) {
        c = 1.f / c;
        for(int i = 0; i < D; ++i){
            vals[i] *= c;
        }
        return *this;
    }

    // Vector operations
    static float dot(Vector<D> u, Vector<D> v){
        float sum = 0.f;
        for(int i = 0; i < D; ++i){
            sum += u.vals[i] * v.vals[i];
        }
        return sum;
    }
    float dot(Vector<D> v) const {
        float sum = 0.f;
        for(int i = 0; i < D; ++i){
            sum += vals[i] * v.vals[i];
        }
        return sum;
    }
    static Vector<D> reflect(Vector<D> incident, Vector<D> normal) {
        return incident - 2.f * dot(incident, normal) * normal;
    }

    template <size_t Dim = D>
    static typename std::enable_if<Dim == 2, float>::type
        cross(Vector<D> u, Vector<D> v) { // 2D cross
        return u.vals[0] * v.vals[1] - u.vals[1] * v.vals[0];
    }
    
    template <size_t Dim = D>
    typename std::enable_if<Dim == 2, float>::type
        cross(Vector<D> v) const { // 2D cross
        return vals[0] * v.vals[1] - vals[1] * v.vals[0];
    }

    template <size_t Dim = D>
    static typename std::enable_if<Dim == 3, Vector<D>>::type
        cross(Vector<D> u, Vector<D> v) { // 3D cross
        return Vector<D>({u[1] * v[2] - u[2] * v[1],
                          u[2] * v[0] - u[0] * v[2],
                          u[0] * v[1] - u[1] * v[0]});
    }
    template <size_t Dim = D>
    typename std::enable_if<Dim == 3, Vector<D>>::type
        cross(Vector<D> v) const { // 3D cross
        return Vector<D>({vals[1] * v[2] - vals[2] * v[1],
                          vals[2] * v[0] - vals[0] * v[2],
                          vals[0] * v[1] - vals[1] * v[0]});
    }

    // Implicit conversion to similar vector types
    template <size_t Dim = D>
    operator typename std::enable_if<Dim == 4, GColor>::type () const {
        return GColor{vals[0], vals[1], vals[2], vals[3]};
    }
    template <size_t Dim = D>
    operator typename std::enable_if<Dim == 3, GColor>::type () const {
        return GColor{vals[0], vals[1], vals[2], 1.f};
    }

    template <size_t Dim = D>
    operator typename std::enable_if<Dim == 2, GPoint>::type () const {
        return GPoint{vals[0], vals[1]};
    }
    

private:
    float* vals;
};

using vec4 = Vector<4>;
using vec3 = Vector<3>;
using vec2 = Vector<2>;

#endif