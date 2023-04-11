#include "Camera.h"
#include "Input.h"
#include <iostream>

using namespace DirectX;

// Creates a camera at the specified position
Camera::Camera(float x, float y, float z, float moveSpeed, float mouseLookSpeed, float aspectRatio)
{
	this->movementSpeed = moveSpeed;
	this->mouseLookSpeed = mouseLookSpeed;
	transform.SetPosition(x, y, z);
	transform.SetRotation(0 ,-3.15 ,0);
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

// Nothing to really do
Camera::~Camera()
{ }


// Camera's update, which looks for key presses
void Camera::Update(float dt)
{
	// Current speed
	float speed = dt * movementSpeed;

	// Get the input manager instance
	Input& input = Input::GetInstance();

	// Speed up or down as necessary
	if (input.KeyDown(VK_SHIFT)) { speed *= 5; }
	if (input.KeyDown(VK_CONTROL)) { speed *= 0.1f; }

	// Movement
	if (input.KeyDown('W')) { transform.MoveRelative(0, 0, speed); }
	if (input.KeyDown('S')) { transform.MoveRelative(0, 0, -speed); }
	if (input.KeyDown('A')) { transform.MoveRelative(-speed, 0, 0); }
	if (input.KeyDown('D')) { transform.MoveRelative(speed, 0, 0); }
	if (input.KeyDown('X')) { transform.MoveAbsolute(0, -speed, 0); }
	if (input.KeyDown(' ')) { transform.MoveAbsolute(0, speed, 0); }
	if (input.KeyDown('P')) { std::cout<< "CPos " <<transform.GetPosition().x << " "
		<< transform.GetPosition().y << " "
	<< transform.GetPosition().z << std::endl;
	std::cout << "roll pitch yaw " << transform.GetPitchYawRoll().x << " "
		<< transform.GetPitchYawRoll().y << " "
		<< transform.GetPitchYawRoll().z << std::endl;
	std::cout << "Cup " << transform.GetPosition().x << " "
		<< transform.GetPosition().y << " "
		<< transform.GetPosition().z << std::endl;
	}

	// Handle mouse movement only when button is down
	if (input.MouseLeftDown())
	{
		// Calculate cursor change
		float xDiff = dt * mouseLookSpeed * input.GetMouseXDelta();
		float yDiff = dt * mouseLookSpeed * input.GetMouseYDelta();
		transform.Rotate(yDiff, xDiff, 0);
	}

	// Update the view every frame - could be optimized
	UpdateViewMatrix();

}

DirectX::XMVECTOR Camera::GetDirection()  noexcept
{
	XMFLOAT3 rot = transform.GetPitchYawRoll();
	XMFLOAT3 pos = transform.GetPosition();
	// camDirection = camPosition - camTarget
	const XMVECTOR forwardVector{ 0.0f, 0.0f, 1.0f, 0.0f };
	const auto lookVector = XMVector3Transform(forwardVector,
		XMMatrixRotationRollPitchYaw(rot.x, rot.y,0));
	const auto camPosition = XMLoadFloat3(&pos);
	const auto camTarget = XMVectorAdd(camPosition,
		lookVector);
	return XMVector3Normalize(XMVectorSubtract(camPosition,
		camTarget));
}



DirectX::XMVECTOR Camera::GetForward() const noexcept
{
	return XMVector3Cross(GetRight(),
		GetUp());
}
DirectX::XMVECTOR Camera::GetRight() const noexcept
{
	const XMVECTOR upVector{ 0.0f, 1.0f, 0.0f, 0.0f };
	return XMVector3Cross(upVector,
		direction);
}

DirectX::XMVECTOR Camera::GetUp() const noexcept
{
	return XMVector3Cross(direction,
		GetRight());
}
// Creates a new view matrix based on current position and orientation
void Camera::UpdateViewMatrix()
{
	// Rotate the standard "forward" matrix by our rotation
	// This gives us our "look direction"
	//XMFLOAT3 rot = transform.GetPitchYawRoll();
	//direction = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rot)));
	direction = Camera::GetDirection();
	XMFLOAT3 pos = transform.GetPosition();
	XMMATRIX view = XMMatrixLookToLH(
		XMLoadFloat3(&pos),
		direction,
		XMVectorSet(0, 1, 0, 0));

	XMStoreFloat4x4(&viewMatrix, view);
}

// Updates the projection matrix
void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	m_AspectRatio = aspectRatio;
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * XM_PI,		// Field of View Angle
		aspectRatio,		// Aspect ratio
		0.01f,				// Near clip plane distance
		100.0f);			// Far clip plane distance
	XMStoreFloat4x4(&projMatrix, P);
}

Transform* Camera::GetTransform()
{
	return &transform;
}
