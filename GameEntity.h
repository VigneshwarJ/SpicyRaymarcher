#pragma once
#include <memory>
#include "Mesh.h"
#include "Transform.h"
#include "Material.h"

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat, Transform transform);

	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();
	std::shared_ptr<Material> GetMaterial();


private:
	std::shared_ptr<Mesh> mesh;
	Transform transform;
	std::shared_ptr<Material> material;

};

