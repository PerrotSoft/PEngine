#include "../../Include/Physics/Physics.h"
namespace PEngine
{
	Physics::PhysicsObject& Physics::AddPhysicsObject(const PhysicsObject& physics_object)
	{
		PhysicsObject new_object = physics_object;
		new_object.id = next_physics_object_id++;
		Physics_objects.push_back(new_object);
		return Physics_objects.back();
	}
	Physics::PhysicsObject& Physics::SearchPhysicsObject(const char* name)
	{
		for (auto& obj : Physics_objects)
		{
			if (obj.name == name)
			{
				return obj;
			}
		}
		throw std::runtime_error("Physics object not found: " + std::string(name));
	}
	void Physics::UpdatePhysics(float deltaTime)
	{
		updatecolisions();
		updatepositions(deltaTime);
	}
	void Physics::updatecolisions()
	{
	}
	void Physics::updatepositions(float deltaTime)
	{
	}
}