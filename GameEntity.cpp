#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, Transform transform)
{
	this->mesh = mesh;
	this->transform = transform;
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

Transform* GameEntity::GetTransform()
{
	return &transform;
}
