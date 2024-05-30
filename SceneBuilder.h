#ifndef SceneBuilder_DEFINED
#define SceneBuilder_DEFINED


#include <vector>
#include <string>
#include <unordered_map>
#include "Object.h"
#include "Light.h"
#include "Camera.h"
#include "include/CustomException.h"

#include "src/json.hpp"
using json = nlohmann::json;

using namespace std;


#pragma region SceneVerifier
struct DataVerify {
    vector<string> required;
    unordered_map<string, json> defaults;
};

class SceneVerifier {
public:
    static bool VerifyElement(string objKey, json& data);
    /// @brief Verifies scene objects and supplies defaults
    /// @param sceneData 
    /// @return True if scene verified, false or throws exception otherwise
    static bool Verify(json& sceneData);

private:
    typedef unordered_map<string, DataVerify> VerifierMap;
    static VerifierMap _verifierMap;

    // DataVerify for every kind of object that can exist in scene
    static DataVerify _Camera;
    static DataVerify _Object;
    static DataVerify _Icosphere;
    static DataVerify _PointLight;
};

#pragma endregion


struct Scene {
    Camera cam;
    vector<Object> objects{};
    vector<Light> lights{};
};


class SceneBuilder {
public:
    SceneBuilder(string filename = "") {
        if(filename == "") scene = {};
        else LoadScene(filename);
    }

    void AddObject(Object& obj) {
        scene.objects.push_back(obj);
    }

    void SaveScene() {throw CustomException("Not implemented."); }
    
    void LoadScene(string filename);

    const Scene getScene() const { return scene; }

private:
    Scene scene;

    typedef unordered_map<string, Object (*) (json)> ObjBuilderMap;
    typedef unordered_map<string, Light (*) (json)> LightBuilderMap;
    static ObjBuilderMap _ObjectBuilders;
    static LightBuilderMap _LightBuilders;
    static ObjBuilderMap _LightObjBuilders;

    // Builders
    Camera buildCamera(json cam_data); 

    Object buildObject(json obj_data);

    Light buildLight(json light_data);
    Object buildLightObj(json light_data);
};





#endif