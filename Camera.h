#pragma once
#include <DirectXMath.h>
#include <Windows.h>

#include "Transform.h"

class Camera
{
public:
	Camera(float x, float y, float z, float moveSpeed, float mouseLookSpeed, float aspectRatio);
	~Camera();

	// Updating
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	// Getters
	DirectX::XMFLOAT4X4 GetView() { return viewMatrix; }
	DirectX::XMFLOAT4X4 GetProjection() { return projMatrix; }
	
	//https://stackoverflow.com/questions/21467225/how-do-you-determine-the-view-up-vector

	DirectX::XMVECTOR getForward() const noexcept;
	DirectX::XMVECTOR getDirection()  noexcept;
	DirectX::XMVECTOR Camera::getRight() const noexcept;


	DirectX::XMVECTOR Camera::getUp() const noexcept;


	Transform* GetTransform();

private:
	// Camera matrices
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;
	DirectX::XMVECTOR direction;
	//XMVECTOR up;

	Transform transform;

	float movementSpeed;
	float mouseLookSpeed;
};

