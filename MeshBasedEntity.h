#pragma once
#include "GameEntity.h"
#include <memory>

class MeshBasedEntity : public GameEntity
{
public:
	MeshBasedEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat, Transform transform);

	std::shared_ptr<Mesh> GetMesh();
	


private:
	std::shared_ptr<Mesh> mesh;

};

