#pragma once
#include "Physics.h"

namespace PEngine
{
    class Physics_Base : public Physics
    {
    public:
        Physics_Base(Scene* scene) : Physics(scene) {}

        void updatecolisions() override;
        void updatepositions(float deltaTime) override;
    };
}