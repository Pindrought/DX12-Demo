#pragma once
#include "pch.h"
#include "Object3D.h"

class Camera
{
public:
	enum class ProjectionType
	{
		Unknown,
		Perspective,
		Orthographic
	};

	ProjectionType GetProjectionType();

	Vector3 GetPosition() const { return m_Translation; }
	Matrix GetProjectionMatrix() const { return m_ProjectionMatrix; }
	Matrix GetViewMatrix() { RebuildViewMatrix();  return m_ViewMatrix; }
	Matrix GetViewProjectionMatrix() { RebuildViewMatrix(); return m_ViewProjectionMatrix; }


	void InitializePerspectiveRH(float aspectRatio, float nearZ, float farZ, float fovXDegrees = 90);
	void InitializeOrtho(float width, float height, float nearZ, float farZ);
	void InitializeOrthoOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ);

	void SetVerticalFOV(float fovRadiansY);
	void SetHorizontalFOV(float fovRadiansX);
	void SetPitchYawRoll(float pitch, float yaw, float roll);
	void SetTranslation(Vector3 translation);
private:
	ProjectionType m_ProjectionType = ProjectionType::Unknown;
	void RebuildViewMatrix();
	void RebuildPerspectiveMatrix();

	Matrix m_ViewMatrix;
	Matrix m_ProjectionMatrix;
	Matrix m_ViewProjectionMatrix;

	bool m_IsDirty = true;
	float m_NearZ = 0.1f;
	float m_FarZ = 1000.0f;
	float m_FovX = DirectX::XM_PIDIV2;
	float m_FovY = DirectX::XM_PIDIV2;
	float m_AspectRatio = 4.0f / 3.0f;
	Vector3 m_Translation = { 0, 0, 0 };
	float m_Pitch = 0;
	float m_Yaw = 0;
	float m_Roll = 0;
};