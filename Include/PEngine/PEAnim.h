// PEAnim.h
#ifndef PEANIM_H
#define PEANIM_H

#include <string>
#include <vector>
#include <map>

namespace PEngine {
    class Scene;

    enum class AnimEasing { Linear, Sine, SmoothStep };

    struct PEAnimKeyframe {
        float time_ms;
        float pos[3];
        float rot[3];
        float scale[3];
        unsigned int textureID = 0;
        AnimEasing easing = AnimEasing::Linear;
    };

    struct PEAnimation {
        std::string target_object;
        std::string anim_name;
        float duration_ms;
        int type;
        std::vector<PEAnimKeyframe> keyframes;
        float current_time = 0.0f;
        bool is_playing = false;
        bool loop = false;
    };

    class Animator {
    public:
        std::map<std::string, PEAnimation> animations;

        void LoadAnimationPEANIM(const std::string& filepath, Scene* scene);
        void PlayAnimation(const std::string& anim_name, bool loop = false);
        void StopAnimation(const std::string& anim_name);
        void UpdateAnimations(Scene* scene, float deltaTime_ms);
    };
}

#endif