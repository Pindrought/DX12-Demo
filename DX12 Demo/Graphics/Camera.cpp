#include "pch.h"
#include "Camera.h"

Camera::ProjectionType Camera::GetProjectionType()
{
	return m_ProjectionType;
}

void Camera::InitializePerspectiveRH(float aspectRatio, float nearZ, float farZ, float fovXDegrees)
{
	assert(nearZ > 0.0f);
	m_IsDirty = true;
	m_ProjectionType = ProjectionType::Perspective;
	m_NearZ = nearZ;
	m_FarZ = farZ;
	m_AspectRatio = aspectRatio;
	SetHorizontalFOV(DirectX::XMConvertToRadians(fovXDegrees));
	RebuildPerspectiveMatrix();
	RebuildViewMatrix();
}

void Camera::InitializeOrtho(float width, float height, float nearZ, float farZ)
{
	m_IsDirty = true;
	m_ProjectionType = ProjectionType::Orthographic;
	m_NearZ = nearZ;
	m_FarZ = farZ;
	m_ProjectionMatrix = XMMatrixOrthographicRH(width, height, nearZ, farZ);
	RebuildViewMatrix();
}

void Camera::InitializeOrthoOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ)
{
	m_IsDirty = true;
	m_ProjectionType = ProjectionType::Orthographic;
	m_NearZ = nearZ;
	m_FarZ = farZ;
	m_ProjectionMatrix = XMMatrixOrthographicOffCenterRH(left, right, bottom, top, nearZ, farZ);
	RebuildViewMatrix();
}

void Camera::SetVerticalFOV(float fovRadiansY)
{
	m_IsDirty = true;
	m_FovY = fovRadiansY;
	m_FovX = atan(tan(m_FovY / 2.0f) * m_AspectRatio) * 2;
	RebuildPerspectiveMatrix();
}

void Camera::SetHorizontalFOV(float fovRadiansX)
{
	m_IsDirty = true;
	m_FovX = fovRadiansX;
	m_FovY = atan(tan(m_FovX / 2.0f) * 1.0f / m_AspectRatio) * 2.0f;
	RebuildPerspectiveMatrix();
}

void Camera::SetPitchYawRoll(float pitch, float yaw, float roll)
{
	m_IsDirty = true;
	m_Pitch = pitch;
	m_Yaw = yaw;
	m_Roll = roll;
}

void Camera::SetTranslation(Vector3 translation)
{
	m_IsDirty = true;
	m_Translation = translation;
}

void Camera::RebuildViewMatrix()
{
	if (!m_IsDirty)
	{
		return;
	}
	// Use quaternion rotation — faster and avoids full matrix construction.
	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(m_Pitch, m_Yaw, m_Roll);

	// Rotate basis vectors (XMVector3Rotate is optimized for this).
	XMVECTOR forward = XMVector3Rotate(Vector3::Forward, quat);
	XMVECTOR up = XMVector3Rotate(Vector3::Up, quat);

	// Build view matrix directly.
	m_ViewMatrix = XMMatrixLookToRH(m_Translation, forward, up);
	m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;
	m_IsDirty = false;
}

void Camera::RebuildPerspectiveMatrix()
{
	assert(m_ProjectionType == ProjectionType::Perspective);
	m_ProjectionMatrix = XMMatrixPerspectiveFovRH(m_FovY, m_AspectRatio, m_NearZ, m_FarZ);
	m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;
}
