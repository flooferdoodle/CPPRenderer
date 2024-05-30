#include "SceneBuilder.h"
#include "include/CustomException.h"
#include <iostream>
#include <fstream>
#include <array>

#pragma region JSON Converters
template<size_t D>
Vector<D> jsonToVec(json arr) {
    assert(arr.type_name() == "array");
    assert(arr[0].is_number());

    return Vector<D>{arr.template get<std::array<float, D>>()};
}
float jsonToFloat(json val) {
    assert(val.is_number());
    return val.template get<float>();
}
int jsonToInt(json val) {
    assert(val.is_number_integer());
    return val.template get<int>();
}
string jsonToString(json str) {
    assert(str.is_string());
    return str.template get<string>();
}

vec3 jsonFloatOrVec3(json val) {
    if(val.is_array()) return jsonToVec<3>(val);
    // convert float to vec3
    assert(val.is_number());
    return vec3(jsonToFloat(val));
}
bool jsonToBool(json val) {
    assert(val.is_boolean());
    return val.template get<bool>();
}

#pragma endregion

#pragma region Object Builders
Camera SceneBuilder::buildCamera(json cam_data) {
    return Camera{
        jsonToVec<3>(cam_data["pos"]),
        jsonToVec<3>(cam_data["target"]),
        jsonToVec<3>(cam_data["up"]),
        jsonToFloat(cam_data["fov"]),
        jsonToFloat(cam_data["aspect"]),
        jsonToFloat(cam_data["nearClip"]),
        jsonToFloat(cam_data["farClip"])
    };
}

Object buildCube(json obj_data) {
    return Object::Cube(jsonToVec<3>(obj_data["pos"]),
                        jsonFloatOrVec3(obj_data["scale"]),
                        jsonToVec<3>(obj_data["euler"]),
                        jsonToVec<3>(obj_data["color"]),
                        jsonToFloat(obj_data["shininess"]));
}

Object buildIcosahedron(json obj_data) {
    return Object::Icosahedron(jsonToVec<3>(obj_data["pos"]),
                        jsonFloatOrVec3(obj_data["scale"]),
                        jsonToVec<3>(obj_data["euler"]),
                        jsonToVec<3>(obj_data["color"]),
                        jsonToFloat(obj_data["shininess"]));
}

Object buildIcosphere(json obj_data) {
    return Object::Icosphere(jsonToVec<3>(obj_data["pos"]),
                        jsonFloatOrVec3(obj_data["scale"]),
                        jsonToVec<3>(obj_data["euler"]),
                        jsonToVec<3>(obj_data["color"]),
                        jsonToFloat(obj_data["shininess"]),
                        jsonToInt(obj_data["subdivide"]));
}

Object buildPlane(json obj_data) {
    return Object::Plane(jsonToVec<3>(obj_data["pos"]),
                        jsonFloatOrVec3(obj_data["scale"]),
                        jsonToVec<3>(obj_data["euler"]),
                        jsonToVec<3>(obj_data["color"]),
                        jsonToFloat(obj_data["shininess"]));
}

SceneBuilder::ObjBuilderMap SceneBuilder::_ObjectBuilders = {
    {"cube", buildCube},
    {"icosahedron", buildIcosahedron},
    {"icosphere", buildIcosphere},
    {"plane", buildPlane}
};

Object SceneBuilder::buildObject(json obj_data) {
    string obj_type = jsonToString(obj_data["type"]);
    assert(_ObjectBuilders.find(obj_type) != _ObjectBuilders.end());

    return _ObjectBuilders[obj_type](obj_data);
}
#pragma endregion

#pragma region Light Builders

Light buildPointLight(json light_data) {
    vec3 atten = jsonToVec<3>(light_data["attenuation"]);
    vec3 col = jsonToVec<3>(light_data["color"]);
    float lightMax = max(col[0], max(col[1], col[2]));
    float effectiveRadius = (
        (-atten[1] +  std::sqrt(atten[1] * atten[1] - 4.f * atten[2] * (atten[0] - 51.2f * lightMax))) 
        / (2.f * atten[2])
    );
    return Light{jsonToVec<3>(light_data["pos"]),
                 jsonToVec<3>(light_data["color"]),
                 jsonToFloat(light_data["ambient"]),
                 jsonToFloat(light_data["specular"]),
                 effectiveRadius, atten[0], atten[1], atten[2]};
}

Object buildPointLightObj(json light_data) {
    return Object::Icosphere(jsonToVec<3>(light_data["pos"]),
                            jsonFloatOrVec3(light_data["d_size"]),
                            {0.f, 0.f, 0.f},
                            jsonToVec<3>(light_data["color"]),
                            -1.f, 2);
}

Light SceneBuilder::buildLight(json light_data) {
    string light_type = jsonToString(light_data["type"]);
    assert(_LightBuilders.find(light_type) != _LightBuilders.end());

    return _LightBuilders[light_type](light_data);
}

Object SceneBuilder::buildLightObj(json light_data) {
    string light_type = jsonToString(light_data["type"]);
    assert(_LightObjBuilders.find(light_type) != _LightObjBuilders.end());

    return _LightObjBuilders[light_type](light_data);
}


SceneBuilder::LightBuilderMap SceneBuilder::_LightBuilders = {
    {"point", buildPointLight}
};
SceneBuilder::ObjBuilderMap SceneBuilder::_LightObjBuilders = {
    {"point", buildPointLightObj}
};


#pragma endregion

void SceneBuilder::LoadScene(string filename) {
    ifstream file(filename);

    // Check file valid
    if(!file.is_open()){
        throw CustomException("Could not open file. Check filepath.");
    }

    //TODO: ensure file is json file
    json sceneData = json::parse(file);

    // Verify scene data
    if(!SceneVerifier::Verify(sceneData)){
        throw CustomException("Failed to load scene data.");
    }

    // Construct camera
    scene.cam = buildCamera(sceneData["cam"]);
    
    // Construct Objects
    for(auto& pair : sceneData["objects"].items()){
        scene.objects.push_back(buildObject(pair.value()));
    }

    // Construct Lights
    for(auto& pair : sceneData["lights"].items()){
        scene.lights.push_back(buildLight(pair.value()));
        scene.objects.push_back(buildLightObj(pair.value()));
    }
}

#pragma region SceneVerifier



// Required objects should always have default values for all parameters
#pragma region DataVerifiers

DataVerify SceneVerifier::_Camera = {
    // Required
    {},

    { // Default parameters
        {"pos", "[0.0, 0.0, 3.0]"_json},
        {"target", "[0.0, 0.0, 0.0]"_json},
        {"up", "[0.0, 1.0, 0.0]"_json},
        {"fov", "90.0"_json},
        {"aspect", "1.0"_json},
        {"nearClip", "0.1"_json},
        {"farClip", "100.0"_json}
    }
};

DataVerify SceneVerifier::_Object = {
    {"type", "pos"}, // Required

    { // Default parameters
        {"euler", "[0.0, 0.0, 0.0]"_json},
        {"scale", "1"_json},
        {"color", "[1.0, 1.0, 1.0]"_json},
        {"shininess", "64"_json}
    }
};

DataVerify SceneVerifier::_Icosphere = {
    {"type", "pos"}, // Required

    { // Default parameters
        {"euler", "[0.0, 0.0, 0.0]"_json},
        {"scale", "1"_json},
        {"color", "[1.0, 1.0, 1.0]"_json},
        {"shininess", "64"_json},
        {"subdivide", "1"_json}
    }
};

DataVerify SceneVerifier::_PointLight = {
    {"type", "pos"},

    {
        {"color", "[1.0, 1.0, 1.0]"_json},
        {"ambient", "0.1"_json},
        {"specular", "2"_json},
        // attenuation : [K_c, K_l, k_q]
        {"attenuation", "[1.0, 0.7, 1.8]"_json},
        {"d_size", "0.05"_json} // size of sphere representation
    }
};

SceneVerifier::VerifierMap SceneVerifier::_verifierMap = {
    {"cam", SceneVerifier::_Camera},
    {"object", SceneVerifier::_Object},
    {"cube", SceneVerifier::_Object},
    {"icosahedron", SceneVerifier::_Object},
    {"icosphere", SceneVerifier::_Icosphere},
    {"plane", SceneVerifier::_Object},
    {"point", SceneVerifier::_PointLight}
};
#pragma endregion



bool SceneVerifier::VerifyElement(string eleKey, json& data) {
    assert(_verifierMap.find(eleKey) != _verifierMap.end());
    if(_verifierMap.find(eleKey) == _verifierMap.end()) { 
        string e_msg = "Scene element of type \"" + eleKey + "\" is not valid.";
        throw CustomException(e_msg.data());
    }

    DataVerify dataVerify = _verifierMap[eleKey];

    for(string req : dataVerify.required) {
        if(!data.contains(req)){
            string e_msg = "Element \"" + eleKey + "\" is missing required property \"" + req + "\".";
            throw CustomException(e_msg.c_str());
        }
    }

    for(pair<string, json> opt : dataVerify.defaults) {
        if(!data.contains(opt.first)){
            data[opt.first] = opt.second;
        }
    }
    return true;
};

bool SceneVerifier::Verify(json& sceneData) {
    // Ensure required structures exist, or add if not present
    if(!sceneData.contains("cam")) sceneData["cam"] = "{}"_json;
    if(!sceneData.contains("objects")) sceneData["objects"] = "[]"_json;
    if(!sceneData.contains("lights")) sceneData["lights"] = "[]"_json;

    if(sceneData.size() > 3) cout << "Warning: Extra structures in scene file. Required structures are \"cam\", \"objects\", and \"lights\"." << endl;

    VerifyElement("cam", sceneData["cam"]);

    int i = sceneData["objects"].size() - 1;
    for (json::reverse_iterator it = sceneData["objects"].rbegin(); it != sceneData["objects"].rend(); ++it) {
        if(!VerifyElement(jsonToString((*it)["type"]), (*it)))
            return false;
        if((*it).contains("disable") && jsonToBool((*it)["disable"])){
            sceneData["objects"].erase(i);
        }
        --i;
    }

    i = sceneData["lights"].size() - 1;
    for (json::reverse_iterator it = sceneData["lights"].rbegin(); it != sceneData["lights"].rend(); ++it) {
        if(!VerifyElement(jsonToString((*it)["type"]), (*it)))
            return false;
        if((*it).contains("disable") && jsonToBool((*it)["disable"])){
            sceneData["lights"].erase(i);
        }
        --i;
    }

    return true;
}
#pragma endregion