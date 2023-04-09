#include "GameEntity.h"

GameEntity::GameEntity( std::shared_ptr<Material> mat, Transform transform)
{

	this->material = mat;
	this->transform = transform;
}


Transform* GameEntity::GetTransform()
{
	return &transform;
}

std::shared_ptr<Material> GameEntity::GetMaterial()
{
	return material;
}
