#pragma once
#include <memory>
#include "Mesh.h"
#include "Transform.h"
#include "Material.h"
/*
	Game entity can be anything
    Mesh / SDF based entities

*/
class GameEntity
{
public:
	GameEntity(std::shared_ptr<Material> mat, Transform transform);

	Transform* GetTransform();
	std::shared_ptr<Material> GetMaterial();
private:
	Transform transform;
	std::shared_ptr<Material> material;
};

