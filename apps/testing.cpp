#include "../include/vec.h"
#include "../include/matrix.h"
#include <iostream>
#include <array>
#include <map>
#include <string>
#include "../SceneBuilder.h"
#include "../Object.h"

#include "../src/json.hpp"
using json = nlohmann::json;
using namespace std;

void printMat(const mat4& m){
    cout << m[{0, 0}] << ", " << m[{0, 1}] << ", " << m[{0, 2}] << ", " << m[{0, 3}] << endl;
    cout << m[{1, 0}] << ", " << m[{1, 1}] << ", " << m[{1, 2}] << ", " << m[{1, 3}] << endl;
    cout << m[{2, 0}] << ", " << m[{2, 1}] << ", " << m[{2, 2}] << ", " << m[{2, 3}] << endl;
    cout << m[{3, 0}] << ", " << m[{3, 1}] << ", " << m[{3, 2}] << ", " << m[{3, 3}] << endl;
}

void printVec(const vec2& v){
    cout << "(" << v.x() << ", " << v.y() << ")" << endl;
}
void printVec(const vec3& v){
    cout << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")" << endl;
}
void printVec(const vec4& v){
    cout << "(" << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << ")" << endl;
}


int main(int argc, char* argv[]) {
    json j = json::parse(R"(
        {
            "cam" : {
                "pos" : [0.1, 0.3, 0.2]
            },
            "arr" : [
                {
                    "type" : "str"
                },
                {
                    "type" : "str22"
                }
            ]
        }
    )");

    //cout << j["arr"].items() << endl;
    Object::Cube();

    //SceneBuilder builder("scene.json");

    /*
    float arr[3] = {1.f, 2.f, 3.f};

    vec3 v = vec3::fromArr(arr);

    printVec(v);
    
    arr[0] = 0.f;
    v[2] = 1.f;

    printVec(v);*/

    /*
    vec3 pos{1.f, 0.f, 0.f};

    mat4 m{};

    printMat(m);
    cout << "---------------" << endl;

    printMat(m + mat4::Identity());
    cout << "---------------" << endl;

    printMat(m * mat4::Scale(2.f) / 0.5f);
    cout << "---------------" << endl;

    printMat(m * mat4::Translate({0.3f, 1.2f, 5.0f}));
    cout << "---------------" << endl;

    printVec(( mat4::Translate({0.3f, 1.2f, 5.0f}) * pos));*/
    
    return 0;
}