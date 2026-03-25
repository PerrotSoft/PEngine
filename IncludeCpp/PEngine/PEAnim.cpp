// PEAnim.cpp
#include "../../Include/PEngine/PEAnim.h"
#include "../../Include/PEngine/PEngine.h"
#include <fstream>
#include <cmath>
#include <json.hpp>

using json = nlohmann::json;

namespace PEngine {

    void Animator::LoadAnimationPEANIM(const std::string& filepath, Scene* scene) {
        std::ifstream file(filepath);
        if (!file.is_open()) return;

        json j;
        file >> j;

        PEAnimation anim;
        anim.anim_name = j["anim_name"];
        anim.target_object = j["target_object"];
        anim.duration_ms = j["duration_ms"];
        anim.type = j["type"];

        for (auto& kf_json : j["keyframes"]) {
            PEAnimKeyframe kf;
            kf.time_ms = kf_json["time_ms"];
            kf.pos[0] = kf_json["pos"][0]; kf.pos[1] = kf_json["pos"][1]; kf.pos[2] = kf_json["pos"][2];
            kf.rot[0] = kf_json["rot"][0]; kf.rot[1] = kf_json["rot"][1]; kf.rot[2] = kf_json["rot"][2];
            kf.scale[0] = kf_json["scale"][0]; kf.scale[1] = kf_json["scale"][1]; kf.scale[2] = kf_json["scale"][2];

            if (kf_json.contains("texture")) {
                kf.textureID = scene->LoadTextureFromFile(kf_json["texture"]);
            }

            std::string easeType = kf_json.value("easing", "linear");
            if (easeType == "sine") kf.easing = AnimEasing::Sine;
            else if (easeType == "smoothstep") kf.easing = AnimEasing::SmoothStep;
            else kf.easing = AnimEasing::Linear;

            anim.keyframes.push_back(kf);
        }

        animations[anim.anim_name] = anim;
    }

    void Animator::PlayAnimation(const std::string& anim_name, bool loop) {
        if (animations.count(anim_name)) {
            animations[anim_name].is_playing = true;
            animations[anim_name].current_time = 0.0f;
            animations[anim_name].loop = loop;
        }
    }

    void Animator::StopAnimation(const std::string& anim_name) {
        if (animations.count(anim_name)) {
            animations[anim_name].is_playing = false;
        }
    }

    void Animator::UpdateAnimations(Scene* scene, float deltaTime_ms) {
        for (auto& pair : animations) {
            PEAnimation& anim = pair.second;
            if (!anim.is_playing || anim.keyframes.empty()) continue;

            anim.current_time += deltaTime_ms;

            if (anim.current_time >= anim.duration_ms) {
                if (anim.loop) {
                    anim.current_time = fmod(anim.current_time, anim.duration_ms);
                }
                else {
                    anim.is_playing = false;
                    anim.current_time = anim.duration_ms;
                }
            }

            PEAnimKeyframe* kf1 = &anim.keyframes.front();
            PEAnimKeyframe* kf2 = &anim.keyframes.back();

            for (size_t i = 0; i < anim.keyframes.size() - 1; ++i) {
                if (anim.current_time >= anim.keyframes[i].time_ms &&
                    anim.current_time <= anim.keyframes[i + 1].time_ms) {
                    kf1 = &anim.keyframes[i];
                    kf2 = &anim.keyframes[i + 1];
                    break;
                }
            }

            float t = 0.0f;
            if (kf2->time_ms > kf1->time_ms) {
                t = (anim.current_time - kf1->time_ms) / (kf2->time_ms - kf1->time_ms);
            }

            if (kf1->easing == AnimEasing::Sine) {
                t = sin(t * 1.57079632679f);
            }
            else if (kf1->easing == AnimEasing::SmoothStep) {
                t = t * t * (3.0f - 2.0f * t);
            }

            float res_pos[3], res_rot[3], res_sca[3];
            for (int i = 0; i < 3; i++) {
                res_pos[i] = kf1->pos[i] + (kf2->pos[i] - kf1->pos[i]) * t;
                res_rot[i] = kf1->rot[i] + (kf2->rot[i] - kf1->rot[i]) * t;
                res_sca[i] = kf1->scale[i] + (kf2->scale[i] - kf1->scale[i]) * t;
            }

            try {
                Scene::Object& obj = scene->SearchObject(anim.target_object.c_str());
                obj.pos = Vec::Vec3(res_pos[0], res_pos[1], res_pos[2]);
                obj.rotator = Vec::Vec3(res_rot[0], res_rot[1], res_rot[2]);
                obj.size = Vec::Vec3(res_sca[0], res_sca[1], res_sca[2]);

                if (anim.type == 1 || anim.type == 4 || anim.type == 2 || anim.type == 5) {
                    unsigned int cur_tex = kf1->textureID;
                    if (cur_tex != 0) {
                        if (obj.state == 1 || obj.state == 2) obj.material.diffuseMap = cur_tex;
                        else if (obj.state == 7) obj.image_ui.textureID = cur_tex;
                    }
                }
            }
            catch (...) {}
        }
    }
}