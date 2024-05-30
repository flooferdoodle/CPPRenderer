#ifndef Light_DEFINED
#define Light_DEFINED

#include "include/vec.h"

using namespace std;

struct Light { // Generic point light
    vec3 v{};
    vec3 col{1.f, 1.f, 1.f};
    float ambientStrength =  .1f;
    float specularStrength = 2.f;

    float effectiveDistance = 7.f;
    float K_c = 1.f;
    float K_l = .7f;
    float K_q = 1.8f;

    vec3 calcLight(const vec3& p, const vec3& camPos, const vec3& norm, const vec3& albedo, const float shininess){
        vec3 lightDir = (v - p);
        float d = lightDir.length();
        if(d > effectiveDistance) return {0.f, 0.f, 0.f};
        float attenuation = attenuate(d);
        lightDir /= d;

        float diff = max(vec3::dot(norm, lightDir), 0.f);

        vec3 viewDir = vec3::normalize(camPos - p);
        vec3 halfVec = vec3::normalize(lightDir + viewDir);
        //vec3 reflectDir = vec3::reflect(-lightDir, norm);
        //float spec = powf(max(vec3::dot(viewDir, reflectDir), 0.f), shininess);
        float spec = powf(max(vec3::dot(norm, halfVec), 0.f), shininess);

        return (ambientStrength + diff + spec) * attenuation * col * albedo;
    }

    float attenuate(float d) {
        return 1.f / (K_c + K_l * d + K_q * d * d);
    }
};

struct DirLight : Light {

};

#endif