#pragma once
#include "Mesh.h"
#include "Transform.h"
#include <memory>

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh, Transform transform);

	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();

private:
	std::shared_ptr<Mesh> mesh;
	Transform transform;
};

