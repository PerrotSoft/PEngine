#include "../../Include/Physics/Physics Base.h"
#include <cmath>

namespace PEngine
{
    void Physics_Base::updatecolisions()
    {
        for (auto& obj : Physics_objects)
        {
            if (obj.colision_type == 0) continue;

            for (auto& obj1 : Physics_objects)
            {
                if (obj.id == obj1.id) continue;
                if (obj.pos.x < obj1.pos.x + obj1.size.x &&
                    obj.pos.x + obj.size.x > obj1.pos.x &&
                    obj.pos.y < obj1.pos.y + obj1.size.y &&
                    obj.pos.y + obj.size.y > obj1.pos.y &&
                    obj.pos.z < obj1.pos.z + obj1.size.z &&
                    obj.pos.z + obj.size.z > obj1.pos.z)
                {
                    if (obj.colision_type == 2)
                    {
                        float diffX = (obj.pos.x + obj.size.x / 2) - (obj1.pos.x + obj1.size.x / 2);
                        float diffY = (obj.pos.y + obj.size.y / 2) - (obj1.pos.y + obj1.size.y / 2);

                        if (std::abs(diffX) > std::abs(diffY)) 
                            obj.velocity.x = -obj.velocity.x * 0.8f;
                        else 
                            obj.velocity.y = -obj.velocity.y * 0.8f;
                        if (obj.on_collision != nullptr)
                            obj.on_collision(obj1);
                    }
                    else if (obj.colision_type == 1)
                    {
                        if (obj.on_collision != nullptr)
                            obj.on_collision(obj1);
                    }
                }
            }
        }
    }

    void Physics_Base::updatepositions(float deltaTime)
    {
        for (auto& obj : Physics_objects)
        {
            obj.velocity.x *= (1.0f - obj.friction * deltaTime);
            obj.velocity.y *= (1.0f - obj.friction * deltaTime);
            obj.velocity.z *= (1.0f - obj.friction * deltaTime);
            obj.velocity.x += obj.speed_gravity.x * deltaTime;
            obj.velocity.y += obj.speed_gravity.y * deltaTime;
            obj.velocity.z += obj.speed_gravity.z * deltaTime;
            obj.pos.x += obj.velocity.x * deltaTime;
            obj.pos.y += obj.velocity.y * deltaTime;
            obj.pos.z += obj.velocity.z * deltaTime;

            if (g_scene && obj.linked_object_id != -1)
            {
                auto* visual = g_scene->GetObjectById(obj.linked_object_id);
                if (visual)
                    visual->pos = obj.pos;
            }
        }
    }
}