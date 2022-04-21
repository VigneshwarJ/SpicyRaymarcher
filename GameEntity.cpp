#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat, Transform transform)
{
	this->mesh = mesh;
	this->material = mat;
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

std::shared_ptr<Material> GameEntity::GetMaterial()
{
	return material;
}
