#include "MeshBasedEntity.h"

MeshBasedEntity::MeshBasedEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat, Transform transform):
    GameEntity(mat,transform)
{

}

std::shared_ptr<Mesh> MeshBasedEntity::GetMesh()
{
    return mesh;
}
