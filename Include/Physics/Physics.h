#pragma once
#include "..\PEngine\PEngine.h"
namespace PEngine
{
	class Physics {
	public:
		struct PhysicsObject
		{
			long long id;
			std::string name;
			Vec::Vec3 pos, size, rotator, velocity;
			float mass;

			uint8_t colision_type; // 0=No Collision, 1=Box, 2=BOX+rebound
			Vec::Vec3 speed_gravity;
			int friction;
			float elasticity;

			long long linked_object_id = -1;

			void (*on_collision)(PhysicsObject& other);
		};
		Physics(Scene* scene) : g_scene(scene) {}
		std::vector<PhysicsObject> Physics_objects;
		unsigned int next_physics_object_id = 1;
		PhysicsObject& AddPhysicsObject(const PhysicsObject& physics_object);
		PhysicsObject& SearchPhysicsObject(const char* name);
		void UpdatePhysics(float deltaTime);
	protected:
		Scene* g_scene = nullptr;
		virtual void updatecolisions();
		virtual void updatepositions(float deltaTime);
	};
}